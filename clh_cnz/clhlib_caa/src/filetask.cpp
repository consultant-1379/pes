//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      filetask.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012, 2014. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for the file type logs.
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
//      PA2    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "filetask.h"
#include "logger.h"
#include "exception.h"
#include "message.h"
#include "common.h"
#include "xmfilter.h"
#include "eventhandler.h"
#include <boost/lexical_cast.hpp>
#include <fcntl.h>
#include <boost/numeric/conversion/cast.hpp>

using namespace std;
using namespace boost;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
FileTask::FileTask():
BaseTask(),
m_filelist()
{
}

//----------------------------------------------------------------------------------------
//	Destructor
//----------------------------------------------------------------------------------------
FileTask::~FileTask()
{	
}

//----------------------------------------------------------------------------------------
//	Open log
//----------------------------------------------------------------------------------------
void FileTask::open()
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
         m_logsize += fs::file_size(path);
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
//   Close log
//----------------------------------------------------------------------------------------
void FileTask::close()
{
   m_filelist.clear();
   m_isopen = false;
}

//----------------------------------------------------------------------------------------
// Parse a file name, extract time (and XM number)
//----------------------------------------------------------------------------------------
FileTask::PAIR FileTask::parseFileName(const string& file)
{
   Time time;
   uint16_t xmno(0);
   size_t pos = file.find_last_of('.');

   // Date and time
   string::const_iterator iter = file.begin() + pos - 15;
   time.set(string(iter, iter + 8), string(iter + 9, iter + 15));

   int count = 0;
   for (iter -= 5; *iter != '_'; ++iter) count += isdigit(*iter)? 1: 0;
   if (count == 4)
   {
      // XM number
      xmno = lexical_cast<uint16_t>(string(iter - 4, iter));
      iter += 5;
   }

   return make_pair(time, xmno);
}

//----------------------------------------------------------------------------------------
// Compare file names by date
//----------------------------------------------------------------------------------------
bool FileTask::compareByDate(const string& file1, const string& file2)
{
   const PAIR& p1 = parseFileName(file1);
   const PAIR& p2 = parseFileName(file2);
   return p1.first < p2.first;
}

//----------------------------------------------------------------------------------------
//   Insert file in log
//----------------------------------------------------------------------------------------
void FileTask::insert(
                  const fs::path& path,
                  const string& prefix,
                  const Time& time,
                  uintmax_t size
                  )
{
   if (m_isopen == false)
   {
      // Log is not opened
      Exception ex(Exception::internal(), WHERE__);
      ex << *this << endl;
      ex << "Log '" << getParameters().getLogName() << "' is not opened.";
      throw ex;
   }

   // Check size
   if (size > getParameters().getMaxsize())
   {
      // File size too big  - delete .tmp-file
      fs::remove(path);

      ostringstream s;
      s << *this << endl;
      s << "File size exceeds max allowed size, removed file " << path << ".";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      return;
   }

   while (m_logsize + size > getParameters().getMaxsize())
   {
      // Maintain size of the log
      if (!maintainLogSize())
      {
         // break out of loop if no file can be deleted due to corruptions
         break;
      }
   }

   // Rename log file
   ostringstream s;
   s << prefix << "_" << time.get() << getParameters().getFileExt();
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
      // File already exists - delete .tmp-file
      fs::remove(path);

      ostringstream s;
      s << *this << endl;
      s << "Rename failed, destination file " << targetpath << " already exists.";
      Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
   }

   Logger integrity(LOG_LEVEL_DEBUG);
   if (integrity)
   {
      // Integrity check
      size_t logsize = calculateLogSize();
      if (logsize != m_logsize)
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << *this << endl;
         ex << "Calculated size " << m_logsize << " does not match real size "
            << logsize << " for log '" << getParameters().getLogName() << "'.";
         throw ex;
      }
   }
}

//----------------------------------------------------------------------------------------
//   Read header
//----------------------------------------------------------------------------------------
void FileTask::readHeader(const fs::path& path, Time& time, uintmax_t& size)
{
   fs::fstream fs;
   fs.open(path, ios_base::in);
   if (fs.is_open() == false)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << *this << endl;
      ex << "Failed to open file " << path << ".";
      ex.sysError();
      throw ex;
   }

   const int s_bufsize = 128;
   char line[s_bufsize];
   uintmax_t datasize;

   fs.getline(line, s_bufsize - 1);
   if (line == CLHMessage::s_headerTag)
   {
      // Analyze header
      try
      {
         // Get time
         fs.getline(line, s_bufsize - 1);

         // Local time received from CP, convert it to UTC time
         time = Time(lexical_cast<uint64_t>(line), false);
         fs >> ws;

         // Get size
         fs.getline(line, s_bufsize - 1);
         datasize = lexical_cast<uint64_t>(line);
         fs >> ws;
      }
      catch (std::exception&)
      {
         ostringstream s;
         s << *this << endl;
         s << "CLH header corrupt for file " << path << ".";
         Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

         time = Time::now();
         datasize = 0;
      }
   }
   else
   {
      ostringstream s;
      s << *this << endl;
      s << "CLH header is missing for file " << path << ".";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      time = Time::now();
      datasize = 0;
   }

   fs.close();

   size = fs::file_size(path);
   if (datasize > 0)
   {
      if (datasize + 40 != size)   // Header size is 40 bytes
      {
         ostringstream s;
         s << *this << endl;
         s << "File size does not match size in header information for file" << endl;
         s << path << ".";
         Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
      }
   }
}

//----------------------------------------------------------------------------------------
//   Get divider iterator
//----------------------------------------------------------------------------------------
FileTask::FILELISTITER FileTask::getDivIter()
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
bool FileTask::maintainLogSize()
{
   // Log reached max size - delete a log file
   const fs::path& logdir = getLogDir();

   FILELISTITER iter = m_filelist.begin();      // Oldest subfile
   const PAIR& p = parseFileName(*iter);
   const Time& first = p.first;                 // Time for oldest subfile
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
      //m_logsize -= fs::file_size(path);
      uintmax_t filesize = fs::file_size(path);

      try
      {
         fs::remove(path);
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
               s << "deleted oldest file " << path << ".";
            }
            else
            {
               s << "deleted file " << path << ", according to divider value.";
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
   
   return is_deleted;
}

//----------------------------------------------------------------------------------------
// Read event log (Not implemented for file type logs)
//----------------------------------------------------------------------------------------
Period FileTask::readEvents(
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
Period FileTask::listEvents(const Period& period) const
{
   Time start(Time::s_maxtime);
   Time stop(Time::s_mintime);

   // List the log files
   const fs::path& logdir = getLogDir();

   // Check log dir exist
   if (fs::exists(logdir) == false)
   {
      return Period(Time(), Time());
   }

   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const string& filename = fs::path(*iter).filename().c_str();
      if (regex_match(filename, getParameters().getLogFile()))
      {
         const PAIR& pair = parseFileName(filename);
         const Time& time = pair.first;

         if (time >= period.first() && time < period.last())
         {
            if (time < start) start = time;
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
void FileTask::transferLogs(const Period& period, const Filter& filter) const
{
   size_t offset;
   switch (getParameters().getHeaderType())
   {
      case     e_noHeader:  offset =  0; break;        // No header
      case     e_clhHeader: offset = 40; break;        // CLH header
      default: assert(!"Unexpected header type");
   }

   // Iterate the log files
   const fs::path& logdir = getLogDir();
   // Check log dir exist
   if (fs::exists(logdir) == false)
   {
      return;
   }

   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const string& filename = fs::path(*iter).filename().c_str();
      if (regex_match(filename, getParameters().getLogFile()))
      {
         const PAIR& pair = parseFileName(filename);
         const Time& time = pair.first;

         if (time >= period.first() && time <= period.last())
         {
            bool match(true);
            if (filter.empty() == false)
            {
               // Filter supplied, match events towards list of XM numbers
               const XmFilter& xmfilter = dynamic_cast<const XmFilter&>(filter);
               match = xmfilter.test(pair.second);
            }
            if (match)
            {
               // Insert file in archive
               const fs::path& dir = getParentDir();
               fs::create_directories(dir);

               const fs::path& destfile = dir / filename;
               copyFile(*iter, destfile, offset);

               Common::archive(destfile, "archive");     // Insert in zip-file
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------
// Copy a file (with offset)
//----------------------------------------------------------------------------------------
void FileTask::copyFile(const fs::path& source, const fs::path& target, ssize_t offset) const
{
   // Open source file
   int input = ::open(source.c_str(), O_RDONLY);
   if (input == -1)
   {
      Exception ex(Exception::parameter(), WHERE__);
      ex << *this << endl;
      ex << "Failed to open file " << source << ".";
      ex.sysError();
      throw ex;
   }

   // Open target file
   int output = ::open(target.c_str(), O_WRONLY | O_CREAT, 0666);
   if (output == -1)
   {
      Exception ex(Exception::parameter(), WHERE__);
      ex << *this << endl;
      ex << "Failed to open file " << target << ".";
      ex.sysError();
      throw ex;
   }

   // Set offset of source file
   off_t off = lseek(input, offset, SEEK_SET);
   if (off == -1)
   {
      Exception ex(Exception::parameter(), WHERE__);
      ex << *this << endl;
      ex << "Failed to set offset for file " << source << ".";
      ex.sysError();
      throw ex;
   }

   const int bufsize(4096);
   char buf[bufsize] = "";

   ssize_t size;
   do
   {
      size = read(input, buf, bufsize);
      if (size > 0)
      {
         size = write(output, buf, size);
      }
   }
   while (size > 0);

   if (size == -1)
   {
      ::close(output);
      ::close(input);

      Exception ex(Exception::parameter(), WHERE__);
      ex << *this << endl;
      ex << "Failed to copy file " << source << " to " << target << ".";
      ex.sysError();
      throw ex;
   }

   ::close(output);
   ::close(input);
}

}

