//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rptask.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013. All rights reserved.
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
//      XDT/DEK (Long Nguyen)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-02-28   XLOBUNG     New version.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "rptask.h"
#include "logger.h"
#include "exception.h"
#include "message.h"
#include "common.h"
#include <boost/lexical_cast.hpp>
#include <fcntl.h>
#include <boost/numeric/conversion/cast.hpp>

using namespace std;
using namespace boost;

namespace PES_CLH {
//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
RPType::RPType():
m_filelist(),
m_currentsize(0),
m_logdir(),
m_totalquota(0),
m_maxtime(0),
m_divider(0),
m_logname()
{
}

//----------------------------------------------------------------------------------------
//   Destructor
//----------------------------------------------------------------------------------------
RPType::~RPType()
{
}

//----------------------------------------------------------------------------------------
//   Close log
//----------------------------------------------------------------------------------------
void RPType::close()
{
    m_filelist.clear();
}

//----------------------------------------------------------------------------------------
//   Add file into list
//----------------------------------------------------------------------------------------
void RPType::addFilename(const std::string& file)
{
    //to do
    m_filelist.push_back(file);
}

//----------------------------------------------------------------------------------------
//   Set parameters
//----------------------------------------------------------------------------------------
void RPType::setParameters(
         const std::string name,
         uintmax_t maxsize,
         uint64_t maxtime,
         uint16_t divider
         )
{
   m_logname = name;
   m_totalquota = maxsize;
   m_maxtime = maxtime;
   m_divider = divider;
}

//----------------------------------------------------------------------------------------
//   Add file size
//----------------------------------------------------------------------------------------
void RPType::addFileSize(uintmax_t size)
{
   m_currentsize += size;
}

//----------------------------------------------------------------------------------------
//   Add file size
//----------------------------------------------------------------------------------------
void RPType::sortList()
{
   sort(m_filelist.begin(), m_filelist.end(), compareByDate);  // Sort dump file list
}

//----------------------------------------------------------------------------------------
// Parse a file name, extract time
//----------------------------------------------------------------------------------------
Time RPType::parseFileName(const string& file)
{
   Time time;
   size_t pos = file.find_last_of('.');

   // Date and time
   string::const_iterator iter = file.begin() + pos - 15;
   time.set(string(iter, iter + 8), string(iter + 9, iter + 15));

   return time;
}

//----------------------------------------------------------------------------------------
// Compare file names by date
//----------------------------------------------------------------------------------------
bool RPType::compareByDate(const string& file1, const string& file2)
{
   const Time p1 = parseFileName(file1);
   const Time p2 = parseFileName(file2);
   return p1 < p2;
}

//----------------------------------------------------------------------------------------
//   Get divider iterator
//----------------------------------------------------------------------------------------
RPType::FILELISTITER RPType::getDivIter()
{
   size_t size = m_filelist.size();

   if (size == 0)
   {
       return m_filelist.end();
   }
   else
   {
      div_t d = div(size * m_divider, 100);
      int divsize = d.quot + (d.rem? 1: 0);

      return m_filelist.begin() + divsize;
   }
}

//----------------------------------------------------------------------------------------
// Maintain the log so it does not exceed the maximum size
//----------------------------------------------------------------------------------------
void RPType::maintainLogSize()
{
   // Log reached max size - delete a log file
   const fs::path& logdir = m_logdir;

   FILELISTITER iter = m_filelist.begin();      // Oldest subfile
   const Time time = parseFileName(*iter);    // Time for oldest subfile
   int64_t diff = Time::now() - time;
   uint64_t difft = 0;                          // Convert to uint64_t
   bool maxtime(true);

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

   if (difft < m_maxtime)
   {
      // Max time is not reached - delete log file according to the divider
      iter = getDivIter();
      maxtime = false;
   }

   if (iter != m_filelist.end())
   {
      const fs::path& path = logdir / *iter;
      m_currentsize -= fs::file_size(path);
      fs::remove(path);

      m_filelist.erase(iter);

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
   }
}

//----------------------------------------------------------------------------------------
//   Insert file in log
//----------------------------------------------------------------------------------------
void RPType::insert(
                  const fs::path& path
                  )
{
   // Check size
   uintmax_t size = fs::file_size(path);

   if (size > m_totalquota)
   {
      // File size too big  - delete .tmp-file
      fs::remove(path);

      ostringstream s;
      s << *this << endl;
      s << "File size exceeds max allowed size, removed file " << path << ".";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      return;
   }

   if (m_currentsize + size > m_totalquota)
   {
      maintainLogSize();                    // Maintain size of the log
   }

   // Parse log filename
   const string& file = path.filename().c_str();
   size_t pos = file.find_last_of('.');
   string::const_iterator iter = file.begin() + pos - 3;
   const string& prefix = string(file.begin(), iter);
   const string& extension = string(iter, iter + 3);

   // Rename log file
   Time time = Time::now();               //Current AP time
   ostringstream s;
   s << prefix << time.get() << "." << extension;
   string targetfile = s.str();

   const fs::path& targetpath = m_logdir / targetfile;

   if (fs::exists(targetpath) == false)
   {
      fs::rename(path, targetpath);
      m_filelist.push_back(targetfile);
      m_currentsize += size;

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
}

//----------------------------------------------------------------------------------------
//   Set log directory
//----------------------------------------------------------------------------------------
void RPType::setLogDir(fs::path path)
{
   m_logdir = path;
}

//----------------------------------------------------------------------------------------
//   Get total file size
//----------------------------------------------------------------------------------------
uintmax_t RPType::getTotalFileSize()
{
   return m_currentsize;
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& s, const RPType& rptype)
{
   rptype.stream(s);
   return s;
}

//----------------------------------------------------------------------------------------
// Stream textual information about the log task
//----------------------------------------------------------------------------------------
void RPType::stream(std::ostream& s) const
{
   s << "Log type: " << m_logname;
}

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
RPTask::RPTask():
BaseTask(),
m_rpdump(),
m_rplog()
{
}

//----------------------------------------------------------------------------------------
//   Destructor
//----------------------------------------------------------------------------------------
RPTask::~RPTask()
{
}

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
void RPTask::event(const fs::path&)
{
}

//----------------------------------------------------------------------------------------
//   Open log
//----------------------------------------------------------------------------------------
void RPTask::open()
{
   typedef std::set<fs::path> TLIST;
   typedef TLIST::const_iterator TLISTCITER;
   uintmax_t maxsize = getParameters().getMaxsize();
   uint64_t maxtime = getParameters().getMaxtime();
   uint16_t divider = getParameters().getDivider();
   fs::path logdir = getLogDir();
   fs::directory_iterator end;
   TLIST tlist;

   // Set parameters for RP dump/log
   m_rpdump.setParameters("RPDUMP", maxsize * 90 / 100, maxtime, divider);
   m_rplog.setParameters("RPLOG", maxsize * 10 / 100, maxtime, divider);

   // Set log directory
   m_rpdump.setLogDir(logdir);
   m_rplog.setLogDir(logdir);

   // Calculate log size
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const fs::path& path = *iter;
      const string& file = path.filename().c_str();
      if (regex_match(file, getParameters().getLogFile()))
      {
         size_t pos = file.find_last_of('.');
         string::const_iterator iter = file.begin() + pos + 1;
         const string& extension = string(iter, iter + 3);
         uintmax_t size = fs::file_size(path);

         if (extension.compare("bin") == 0)
         {
             m_rpdump.addFilename(file);
             m_rpdump.addFileSize(size);
         }
         else if (extension.compare("txt") == 0)
         {
             m_rplog.addFilename(file);
             m_rplog.addFileSize(size);
         }
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

   m_rpdump.sortList();    // Sort dump file list
   m_rplog.sortList();      // Sort log file list

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
void RPTask::close()
{
   m_rpdump.close();
   m_rplog.close();
   m_isopen = false;
}

//----------------------------------------------------------------------------------------
// Get log directory
//----------------------------------------------------------------------------------------
fs::path RPTask::getLogDir() const
{
   return BaseParameters::getApzLogsPath() / m_parameters.getPath();
}

//----------------------------------------------------------------------------------------
// Stream textual information about the log task
//----------------------------------------------------------------------------------------
void RPTask::stream(std::ostream& s) const
{
   s << "Log type: " << m_parameters.getLogName();
}

//----------------------------------------------------------------------------------------
// Create log directory
//----------------------------------------------------------------------------------------
void RPTask::createLogDir() const
{
   // Create log directory if it does not exist
   const fs::path& logdir = getLogDir();
   if (fs::exists(logdir) == false)
   {
      fs::create_directories(logdir);
      
      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         std::ostringstream s;
         s << *this << std::endl;
         s << "Directory " << logdir << " was created.";
         logger.event(WHERE__, s.str());
      }
   }
}

//----------------------------------------------------------------------------------------
// Get log parameters
//----------------------------------------------------------------------------------------
const BaseParameters& RPTask::getParameters() const
{
    return m_parameters;
}

//----------------------------------------------------------------------------------------
//   Insert file in log
//----------------------------------------------------------------------------------------
void RPTask::insert(
                  const fs::path& path
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

   // Parse log filename
   const string& file = path.filename().c_str();
   size_t pos = file.find_last_of('.');
   string::const_iterator iter = file.begin() + pos - 3;
   const string& extension = string(iter, iter + 3);

   if (extension.compare("bin") == 0)
   {
      m_rpdump.insert(path);
   }
   else
   {
      m_rplog.insert(path);
   }

   Logger integrity(LOG_LEVEL_DEBUG);
   if (integrity)
   {
      //Integrity check
      size_t logsize = calculateLogSize();
      uintmax_t storedlogsize = m_rpdump.getTotalFileSize() + m_rplog.getTotalFileSize();
      if (logsize != storedlogsize)
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << *this << endl;
         ex << "Calculated size " << storedlogsize << " does not match real size "
         << logsize << " for log '" << getParameters().getLogName() << "'.";
         throw ex;
      }
   }
}

//----------------------------------------------------------------------------------------
// Read event log (Not implemented for file type logs)
//----------------------------------------------------------------------------------------
Period RPTask::readEvents(
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
Period RPTask::listEvents(const Period&) const
{
   return Period();
}

//----------------------------------------------------------------------------------------
// Transfer log files for a CP or blade
//----------------------------------------------------------------------------------------
void RPTask::transferLogs(const Period&, const Filter&) const
{
}

}
