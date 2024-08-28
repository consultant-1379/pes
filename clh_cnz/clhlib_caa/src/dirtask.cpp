//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dirtask.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for the directory type logs.
//      
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2012-06-04   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "dirtask.h"
#include "logger.h"
#include "exception.h"
#include "common.h"
#include "inotify.h"
#include "eventhandler.h"
#include <boost/tokenizer.hpp>
#include <boost/numeric/conversion/cast.hpp>

using namespace std;
using namespace boost;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
DirTask::DirTask():
BaseTask(),
m_filelist(),
m_threadgroup(),
m_mutex()
{
}

//----------------------------------------------------------------------------------------
//	Destructor
//----------------------------------------------------------------------------------------
DirTask::~DirTask()
{
}

//----------------------------------------------------------------------------------------
//	Open log
//----------------------------------------------------------------------------------------
void DirTask::open()
{
   typedef std::set<fs::path> TLIST;
   typedef TLIST::const_iterator TLISTCITER;

   m_logsize = 0;
   TLIST tlist;

   // Calculate log size
   const fs::path& logdir = getLogDir();
   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const fs::path& path = *iter;
      const string& file = path.filename().c_str();
      if (regex_match(file, getParameters().getLogFile()))
      {
         // Log file
         for (fs::directory_iterator siter(path); siter != end; ++siter)
         {
            const fs::path& subfile = *siter;
            fs::file_status stat(fs::symlink_status(subfile));
            if (fs::is_regular_file(stat))
            {
               m_logsize += fs::file_size(subfile);
            }
         }
         m_filelist.push_back(file);
      }
      else if (regex_match(file, getParameters().getTempFile()))
      {
         // Temporary log file found
         tlist.insert(path);
      }
      else
      {
         // Unknown file, delete it
         fs::remove(path);

         // Log event
         ostringstream s;
         s << *this << endl;
         s << "Unknown file " << path << " found - file removed.";
         Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
      }
   }

   sort(m_filelist.begin(), m_filelist.end(), compareByDate);    // Sort file list

   m_isopen = true;

   // If there are temporary files - process them
   for (TLISTCITER titer = tlist.begin(); titer != tlist.end(); ++titer)
   {
      event(*titer);
   }
   tlist.clear();
}

//----------------------------------------------------------------------------------------
// Close log
//----------------------------------------------------------------------------------------
void DirTask::close()
{
   m_threadgroup.join_all();
   m_filelist.clear();
   m_isopen = false;
}

//----------------------------------------------------------------------------------------
// Parse a file name, extract time
//----------------------------------------------------------------------------------------
Time DirTask::parseFileName(const string& file)
{
   size_t pos = file.size();

   // Date and time
   string::const_iterator iter = file.begin() + pos - 15;
   return Time(
            string(iter, iter + 8),
            string(iter + 9, iter + 15)
            );
}

//----------------------------------------------------------------------------------------
// Compare file names by date
//----------------------------------------------------------------------------------------
bool DirTask::compareByDate(const string& file1, const string& file2)
{
   const Time& time1 = parseFileName(file1);
   const Time& time2 = parseFileName(file2);
   return time1 < time2;
}

//----------------------------------------------------------------------------------------
// Insert file in log
//----------------------------------------------------------------------------------------
void DirTask::insert(const fs::path& path, int delay)
{
   boost::mutex::scoped_lock lock(m_mutex);

   if (m_isopen == false)
   {
      // Log is not opened
      Exception ex(Exception::internal(), WHERE__);
      ex << *this << endl;
      ex << "Log '" << getParameters().getLogName() << "' is not opened.";
      throw ex;
   }

   Inotify inotify;
   int inotifyfd = inotify.open(false);
   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(inotifyfd, &fds);

   // Wait for subfiles, timeout after 'delay' seconds
   boost::this_thread::sleep(boost::posix_time::seconds(delay));

   // Iterate the subfiles
   size_t size(0);
   fs::directory_iterator end;
   for (fs::directory_iterator siter(path); siter != end; ++siter)
   {
      const fs::path& subfile = *siter;

      // Get file size
      fs::file_status stat(fs::symlink_status(subfile.c_str()));
      if (fs::is_regular_file(stat))
      {
         size += fs::file_size(subfile.c_str());
      }
   }

   if (m_logsize + size > getParameters().getMaxsize())
   {
      maintainLogSize();               // Maintain size of the log
   }

   // Rename log file
   ostringstream s;
   s << getParameters().getFilePrefix() << "_" << Time::now() << getParameters().getFileExt();
   string targetfile = s.str();

   const fs::path& targetpath = getLogDir() / targetfile;
   if (fs::exists(targetpath) == false)
   {
      fs::rename(path, targetpath);
      m_filelist.push_back(targetfile);

      m_logsize += size;

      // Logger information
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << *this << endl;
         s << "Created log file " << targetpath << ".";
         logger.event(WHERE__, s.str());
      }
   }
   else
   {
      // File already exists - delete temporary file
      fs::remove(path);

      ostringstream s;
      s << *this << endl;
      s << "Rename failed, destination file " << targetpath << " already exists.";
      Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
   }

   // Close the notification
   inotify.close();
}

//----------------------------------------------------------------------------------------
// Get divider iterator
//----------------------------------------------------------------------------------------
DirTask::FILELISTITER DirTask::getDivIter()
{
   size_t size = m_filelist.size();
   if (size == 0)
   {
      return m_filelist.end();
   }
   else
   {
      div_t d = div(size * getParameters().getDivider(), 100);
      int divsize = d.quot + (d.rem? 1: 0);

      return m_filelist.begin() + divsize;
   }
}

//----------------------------------------------------------------------------------------
// Maintain the log so it does not exceed the maximum size
//----------------------------------------------------------------------------------------
void DirTask::maintainLogSize()
{
   // Log reached max size - delete a log file
   const fs::path& logdir = getLogDir();

   FILELISTITER iter = m_filelist.begin();      // Oldest subfile
   const Time& first = parseFileName(*iter);    // Time for oldest subfile
   int64_t diff = Time::now() - first;
   uint64_t difft = 0;                          // Convert to uint64_t
   bool maxtime(true);
   bool is_deleted(false);

   try 
   {
      difft = boost::numeric_cast<uint64_t>(diff);
   }
   catch (std::exception& e)
   {
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         logger.event(WHERE__, e.what());
      }
      difft = 0;
   }

   if (difft < getParameters().getMaxtime())
   {
      // Max time is not reached - delete log file according to the divider
      iter = getDivIter();
      maxtime = false;
   }

   while (!is_deleted && iter != m_filelist.end())
   {
      const fs::path& path = logdir / *iter;
      fs::directory_iterator end;
      uintmax_t filesize = 0;
      for (fs::directory_iterator siter(path); siter != end; ++siter)
      {
         const fs::path& subpath = *siter;
         fs::file_status stat(fs::symlink_status(subpath));
         if (fs::is_regular_file(stat))
         {
            //m_logsize -= fs::file_size(subpath);
            filesize += fs::file_size(subpath);
         }
      }

      try
      {
         fs::remove_all(path);
         is_deleted = true;
      }
      catch (fs::filesystem_error e)
      {
         // Move to the next file
         iter++;
         is_deleted = false;
         // Log event
         ostringstream s;
         s << *this << endl;
         s << "Log reached max size, ";
         s << "but " << path << " can not be deleted (" << e.code() << "). Will move to next file.";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
      }

      if (is_deleted)
      {
         // Remove file out of the list
         m_filelist.erase(iter);
         // Subtract the file size
         m_logsize -= filesize;
         // Log event
         Logger logger(LOG_LEVEL_INFO);
         if (logger)
         {
            ostringstream s;
            s << *this << endl;
            s << "Log reached max size, ";
            if (maxtime == true)
            {
               s << "deleted oldest directory " << path << ".";
            }
            else
            {
               s << "deleted directory " << path << ", according to divider value.";
            }
            logger.event(WHERE__, s.str());
         }
         break;
      }
   }

   // Can not delete any files. Raise an AP event
   if (!is_deleted)
   {
      // Report to event handler
      ostringstream s;
      s << *this << endl;
      s << "Error detected while maintaining the quota.";
      EventHandler::send(Exception::system().first, s.str());
   }
}

//----------------------------------------------------------------------------------------
// Read event log (Not implemented for directory type logs)
//----------------------------------------------------------------------------------------
Period DirTask::readEvents(
      const Period&,
      const Filter&,
      t_eventcb,
      std::ostream&
      ) const
{
   return Period();
}

//----------------------------------------------------------------------------------------
// List time for first and last event
//----------------------------------------------------------------------------------------
Period DirTask::listEvents(const Period& period) const
{
   Time start(Time::s_maxtime);
   Time stop(Time::s_mintime);

   // List the log files
   const fs::path& logdir = getLogDir();
   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const string& filename = fs::path(*iter).filename().c_str();
      if (regex_match(filename, getParameters().getLogFile()))
      {
         const Time& time = parseFileName(filename);

         if (time >= period.first())
         {
            if (time < start) start = time;
         }
         if (time < period.last())
         {
            if (time > stop) stop = time;
         }
      }
   }

   // Start & stop time not found
   if (start == Time::s_maxtime && stop == Time::s_mintime)
   {
     return Period(Time(), Time());
   }

   // Start greater than stop time
   if  (start > stop)
   {
      throw StartGreatStopTimeException(start, stop, WHERE__);
   }

   return Period(start, stop);
}

//----------------------------------------------------------------------------------------
// Transfer log files for a CP or blade
//----------------------------------------------------------------------------------------
void DirTask::transferLogs(const Period& period, const Filter&) const
{
   // Iterate the log files
   const fs::path& logdir = getLogDir();
   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const string& filename = fs::path(*iter).filename().c_str();
      if (regex_match(filename, getParameters().getLogFile()))
      {
         const Time& time = parseFileName(filename);

         if (time >= period.first() && time <= period.last())
         {
            // Insert file in archive
            const fs::path& dir = getParentDir();
            fs::create_directories(dir);

            const fs::path& destfile = dir / filename;
            fs::create_directory(destfile);
            for (fs::directory_iterator siter(*iter); siter != end; ++siter)
            {
               const fs::path& subpath = *siter;
               const string& subfilename = fs::path(subpath).filename().c_str();
               fs::file_status stat(fs::symlink_status(subpath));
               if (fs::is_regular_file(stat))
               {
                  fs::copy_file(*siter, destfile / subfilename);
               }
            }

            Common::archive(destfile, "archive");      // Insert in zip-file
         }
      }
   }
}

}

