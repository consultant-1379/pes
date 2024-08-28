//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      true64task.cpp
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
//      PA1    2012-06-05   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "tru64task.h"
#include "logger.h"
#include "exception.h"
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Tru64Task::Tru64Task():
DirTask(),
m_subfilelist(),
m_currentdir()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Tru64Task::~Tru64Task()
{
}

//----------------------------------------------------------------------------------------
// Open log
//----------------------------------------------------------------------------------------
void Tru64Task::open()
{
   typedef std::map<string, fs::path> TLIST;
   typedef TLIST::const_iterator TLISTCITER;
   typedef tokenizer<boost::char_separator<char> > TOKENIZER;

   // Populate the list of regular expressions
   char_separator<char> sep("|");
   const string& str = getParameters().getTempFile().str();
   TOKENIZER tokens(str, sep);
   for (TOKENIZER::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
   {
      m_subfilelist[regex(*iter)] = false;
   }

   m_logsize = 0;
   TLIST tlist;

   // Calculate log size
   const fs::path& logdir = getLogDir();
   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const fs::path& path = *iter;
      const std::string& file = path.filename().c_str();
      if (regex_match(file, getParameters().getLogFile()))
      {
         fs::file_status stat(fs::symlink_status(path));
         if (fs::is_directory(stat))
         {
            // Calculate the size of the subfiles
            for (fs::directory_iterator siter(path); siter != end; ++siter)
            {
               const fs::path& spath = *siter;
               const std::string& file = spath.filename().c_str();
               if (regex_match(file, getParameters().getTempFile()))
               {
                  m_logsize += fs::file_size(spath);
               }
               else
               {
                  // Unknown file, delete it
                  fs::remove(spath);
               }
            }

            // Log file
            m_filelist.push_back(file);
         }
         else
         {
            // Not a directory, delete file
            fs::remove(path);
         }
      }
      else if (regex_match(file, getParameters().getTempFile()))
      {
         // Temporary log file found
         tlist[path.filename().c_str()] = path;
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

   sort(m_filelist.begin(), m_filelist.end(), compareByDate);       // Sort file list

   m_isopen = true;

   // If there are temporary files - process them
   for (TLISTCITER titer = tlist.begin(); titer != tlist.end(); ++titer)
   {
      event(titer->second);
   }
   tlist.clear();
}

//----------------------------------------------------------------------------------------
//   Close log
//----------------------------------------------------------------------------------------
void Tru64Task::close()
{
   m_filelist.clear();
   m_isopen = false;
}

//----------------------------------------------------------------------------------------
//   Insert file in log
//----------------------------------------------------------------------------------------
void Tru64Task::insert(const fs::path& path)
{
   if (m_isopen == false)
   {
      // Log is not opened
      Exception ex(Exception::internal(), WHERE__);
      ex << *this << endl;
      ex << "Log '" << getParameters().getLogName() << "' is not opened.";
      throw ex;
   }

   uintmax_t size = fs::file_size(path);

   // Check size
   if (size > getParameters().getMaxsize())
   {
      // File size too big - delete .tmp-file
      fs::remove(path);

      ostringstream s;
      s << *this << endl;
      s << "File size exceeds max allowed size, removed file " << path << ".";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      return;
   }

   if (m_logsize + size > getParameters().getMaxsize())
   {
      maintainLogSize();               // Maintain size of the log
   }

   // Subfile to process
   SUBFILELISTITER iter = m_subfilelist.begin();
   while (iter != m_subfilelist.end())
   {
      if (regex_match(path.filename().c_str(), iter->first))
      {
         if (iter->second)
         {
            // Prepare to create new log directory
            m_currentdir.clear();
         }
         break;
      }
       ++iter;
   }

   const fs::path& logdir = getLogDir();
   fs::path targetdir = logdir;
   if (m_currentdir.empty())
   {
      // Clear list of subfiles
      SUBFILELISTITER titer;
      for (titer = m_subfilelist.begin(); titer != m_subfilelist.end(); ++titer)
      {
         titer->second = false;
      }

      // Create a new log directory
      const Time& time = Time::now();
      ostringstream s;
      s << getParameters().getFilePrefix() << "_" << time.get();
      m_currentdir = s.str();
      targetdir /= m_currentdir;
      fs::create_directory(targetdir);

      // Logger information
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << *this << endl;
         s << "Created log file " << targetdir << ".";
         logger.event(WHERE__, s.str());
      }
   }
   else
   {
      targetdir /= m_currentdir;
   }

   iter->second = true;

   // Move log file
   fs::rename(path, targetdir / path.filename());

   Logger logger(LOG_LEVEL_INFO);
   if (logger)
   {
      ostringstream s;
      s << *this << endl;
      s << "File " << path << " was moved to target directory " << targetdir << ".";
      logger.event(WHERE__, s.str());
   }

   m_logsize += size;

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

}

