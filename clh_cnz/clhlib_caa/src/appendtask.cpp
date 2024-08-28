//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      appendtask.cpp
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
//      Class for the append type logs.
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
//      PA1    2013-03-04   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "appendtask.h"
#include "logger.h"
#include "exception.h"
#include "message.h"
#include "common.h"
#include "xmfilter.h"
#include "eventhandler.h"
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

using namespace std;
using namespace boost;

namespace PES_CLH {

const regex AppendTask::s_msgnopattern("Message \\d{10}");

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
AppendTask::AppendTask():
BaseTask(),
m_fs(),
m_seldivider(false),
m_filelist()
{
   m_data = new char[BaseParameters::s_maxfilesize];
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
AppendTask::~AppendTask()
{
   delete[] m_data;
   m_data = NULL;
}

//----------------------------------------------------------------------------------------
// Open log
//----------------------------------------------------------------------------------------
void AppendTask::open()
{
   m_logsize = 0;
   fs::path tempfile;
   const fs::path& logdir = getLogDir();
   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const fs::path& path = *iter;
      const string& file = path.filename().c_str();
      if (regex_match(file, getParameters().getLogFile()))
      {
         // Log subfile
         uintmax_t size = fs::file_size(path);
         try
         {
            fs::ifstream fs(path);
            checkIntegrity(fs, size);           // Check integrity of the log file

            m_logsize += size;                  // Update log size
            t_header header;
            fs.seekg(0, ios_base::beg);
            readData(fs, header);
            fs.seekg(header.m_prev);
            const Time& first = header.m_aptime;
            readData(fs, header);
            const Time& last = header.m_aptime;

            const Time& time = parseFileName(file);

            //HY85159 : Added DST check so that if any incorrect filenames are created during DST 
            //change , they will be corrected.
 
            if ( time.isDstTime() || (time != first))
            {
               // Time zone changed - rename the file
               const string& newfile = createFileName(first);
               const fs::path& newpath = logdir / newfile;

               if(newfile != file)
               {
            	   fs::rename(path, newpath);

            	   // Log event
            	   ostringstream s;
            	   s << *this << endl;
            	   s << "Time zone changed, log file renamed from " << path << " to " << newpath << ".";
            	   Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

               }
            }

            // Insert times in list
            m_filelist.push_back(make_pair(first, last));
         }
         catch (Exception& ex)
         {
            // Corrupt file - remove it
            fs::remove(path);

            // Log event
            ostringstream s;
            s << "Internal structure of log file '" << path << "' is inconsistent: "
              << ex.getMessage() << " File is removed." << endl;
            Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

            // Report to event handler
            EventHandler::send(ex.getErrCode(), s.str());
         }
      }
      else if (regex_match(file, getParameters().getTempFile()))
      {
         // Temporary log file found
         tempfile = path;
      }
      else
      {
         // Unknown file, remove it
         fs::remove(path);

         // Log event
         ostringstream s;
         s << *this << endl;
         s << "Unknown file " << path << " found - file removed.";
         Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
      }
   }

   sort(m_filelist.begin(), m_filelist.end());       // Sort file list

   m_isopen = true;

   // A temporary file was found - process it
   if (tempfile.empty() == false)
   {
      if (m_issetap2)
      {
         readMsgs(tempfile);
      }
      else
      {
         event(tempfile);
      }
   }
}

//----------------------------------------------------------------------------------------
//   Close log
//----------------------------------------------------------------------------------------
void AppendTask::close()
{
   m_filelist.clear();
   if (m_fs.is_open())
   {
      // Close present file
      m_fs.close();
   }
   m_isopen = false;
}

//----------------------------------------------------------------------------------------
// Check integrity of log file
//----------------------------------------------------------------------------------------
void AppendTask::checkIntegrity(fs::ifstream& fs, uintmax_t size) const
{
   t_header header;

   fs.seekg(0, ios_base::beg);
   readData(fs, header);
   size_t offset = header.m_prev;
   size_t next = size;
   do
   {
      fs.seekg(offset);
      readData(fs, header);
      size_t diff = next - offset;
      next = offset;
      offset = header.m_prev;

      Time(header.m_cptime);
      Time(header.m_aptime);
      size_t size = header.m_size;
      if (size + sizeof(t_header) != diff)
      {
         Exception ex(Exception::parameter(), WHERE__);
         ex << "Offset error.";
         throw ex;
      }
   }
   while (next != 0);
}

//----------------------------------------------------------------------------------------
// Parse a file name, extract time
//----------------------------------------------------------------------------------------
Time AppendTask::parseFileName(const string& file) const
{
   size_t pos = file.find_last_of('.');

   // Date and time
   string::const_iterator iter = file.begin() + pos - 22;
   return Time(
            string(iter, iter + 8),
            string(iter + 9, iter + 15),
            string(iter + 16, iter + 22)
            );
}

//----------------------------------------------------------------------------------------
// Construct a file name from time
//----------------------------------------------------------------------------------------
string AppendTask::createFileName(const Time& time) const
{
   return getParameters().getFilePrefix() + "_" + time.get(Time::e_long) +
          getParameters().getFileExt();
}

//----------------------------------------------------------------------------------------
//   Insert message in log
//----------------------------------------------------------------------------------------
void AppendTask::insert(const Time& cptime, const char* data, uintmax_t size, bool issel)
{
   if (m_isopen == false)
   {
      // Log is not opened
      Exception ex(Exception::internal(), WHERE__);
      ex << "Log '" << getParameters().getLogName() << "' is not opened.";
      throw ex;
   }

   // Check size
   uintmax_t eventsize = sizeof(t_header) + size;
   if (eventsize > BaseParameters::s_maxfilesize)
   {
      // Event message too big
      Exception ex(Exception::internal(), WHERE__);
      ex << *this << endl;
      ex << "Message size exceeds max allowed size.";
      throw ex;
   }

   uintmax_t maxsize = getParameters().getMaxsize();

   // Need for divide quota to 90% and 10% for Non-CPUB SEL
   if (m_seldivider)
   {
      if (m_issetap2)
      {
         maxsize = maxsize * 10 / 100;
      }
      else
      {
         maxsize = maxsize * 90 / 100;
      }
   }

   if (m_logsize + eventsize > maxsize)
   {
      maintainLogSize();               // Maintain size of the log
   }

   const Time& aptime = Time::now();
   const fs::path& logdir = getLogDir();
   if (m_filelist.empty())
   {
      // No file exists - create file
      const string& file = createFileName(aptime);
      const fs::path& path = logdir / file;
      createFile(path);
      m_filelist.push_back(make_pair(aptime, aptime));

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << *this << endl;
         s << "Log file " << path << " is created.";
         logger.event(WHERE__, s.str());
      }
   }
   else
   {
      // File(s) exist
      if (m_fs.is_open() == false)
      {
         // Open last file
         const Time& last = m_filelist.back().first;
         const string& file = createFileName(last);
         const fs::path& path = logdir / file;
         openFile(path);
      }
   }

   m_fs.seekg(0, ios_base::end);         // Move get pointer to end of file
   streamoff filesize = m_fs.tellg();   // File size
   if (filesize + eventsize > BaseParameters::s_maxfilesize)
   {
      // File reached max size
      m_fs.close();                     // Close current file

      // Create new file
      const string& file = createFileName(aptime);
      const fs::path& path = logdir / file;
      createFile(path);
      m_filelist.push_back(make_pair(aptime, aptime));
      filesize = 0;

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << *this << endl;
         s << "Log file reached max size, new file " << path << " is created.";
         logger.event(WHERE__, s.str());
      }
   }
   else
   {
      m_filelist.back().second = aptime;
   }

   // Append message to the log file
   t_header header;
   header.m_prev = 0; 
   header.m_cptime = cptime;
   header.m_aptime = aptime;

   // In case of SEL events in sel.tmp file
   if (issel)
   {
      // In sel.tmp the timestamp is AP time, not CP time.
      header.m_cptime = Time();
      header.m_aptime = cptime;
   }
   header.m_size = size;

   if (filesize == 0)
   {
      writeData(m_fs, header);         // Write header
      m_fs.write(data, size);            // Write message
   }
   else
   {
      m_fs.seekg(ios_base::beg);         // Move get pointer to beginning of file
      size_t curlast;
      readData(m_fs, curlast);         // Get pointer to the last record in file
      m_fs.seekp(0, ios_base::end);      // Move put pointer to end of file
      streamoff newlast;
      newlast = m_fs.tellp();            // Get new last pointer 
      header.m_prev = curlast;         // Move pointer to previous record
      writeData(m_fs, header);         // Write header
      m_fs.write(data, size);            // Write message
      m_fs.seekp(0, ios_base::beg);      // Move put pointer to beginning of file
      writeData(m_fs, newlast);         // Write new last pointer
      m_fs.seekp(0, ios_base::end);      // Move put pointer to end of file
   }
   m_fs.flush();

   m_logsize += eventsize;
}

//----------------------------------------------------------------------------------------
// Create new log file
//----------------------------------------------------------------------------------------
void AppendTask::createFile(const fs::path& path)
{
   if (fs::exists(path) == true)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << *this << endl;
      ex << "File " << path << " already exists.";
      throw ex;
   }

   // Create file
   m_fs.open(path, ios_base::out | ios_base::binary);
   if ((m_fs.is_open() == false) || (m_fs.fail() == true))
   {
      Exception ex(Exception::system(), WHERE__);
      ex << *this << endl;
      ex << "Failed to create file " << path << ".";
      ex.sysError();
      throw ex;
   }

   // Close file
   m_fs.close();

   openFile(path);
}

//----------------------------------------------------------------------------------------
// Open file
//----------------------------------------------------------------------------------------
void AppendTask::openFile(const fs::path& path)
{
   m_fs.open(path, ios_base::in | ios_base::out | ios_base::binary);
   if ((m_fs.is_open() == false) || (m_fs.fail() == true))
   {
      Exception ex(Exception::system(), WHERE__);
      ex << *this << endl;
      ex << "Failed to open file " << path << ".";
      ex.sysError();
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Read messages from a temporary log file
//----------------------------------------------------------------------------------------
void AppendTask::readMsgs(const fs::path& path)
{
   fs::path tpath(path);
   tpath.replace_extension(".tmp_");
   if (path == tpath)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << *this << endl;
      ex << "Ignore notification for *.tmp_ file: " << path << ".";
      ex.sysError();
      Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      return;
   }

   if (fs::exists(tpath))
   {
      // Incomplete message file found
      fs::ifstream ifs(path, ios_base::binary);
      if (ifs.is_open() == false)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << *this << endl;
         ex << "Failed to open file " << path << ".";
         ex.sysError();
         Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      }
      else
      {
         fs::ofstream ofs(tpath, ios_base::binary | ios_base::app);
         if (ofs.is_open() == false)
         {
            Exception ex(Exception::system(), WHERE__);
            ex << *this << endl;
            ex << "Failed to open file " << tpath << ".";
            ex.sysError();
            Logger::event(LOG_LEVEL_ERROR, WHERE__, ex.getMessage());
         }
         else
         {
            // Remove tmp-file
            fs::remove(path);
            // Append tmp-file to incomplete message
            ofs << ifs.rdbuf();
            ofs.close();
      
            Logger logger(LOG_LEVEL_INFO);
            if (logger)
            {
               ostringstream s;
               s << *this << endl;
               s << "Log messages appended to incomplete retained message.";
               logger.event(WHERE__, s.str());
            }
         }
      }
   }
   else
   {
      try
      {
         fs::rename(path, tpath);
      }
      catch (std::exception& exp)
      {
         // This should really not happen, still it happens
         ostringstream s;
         s << *this << endl;
         s << "Failed to rename file " << path << " to " << tpath << "." << endl;
         s << exp.what() << endl;
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
         // Failed and do nothing
         return;
      }
   }

   fs::fstream ifs;
   ifs.open(tpath, ios_base::in);
   if (ifs.is_open() == false)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << *this << endl;
      ex << "Failed to open file " << tpath << ".";
      ex.sysError();
      Logger::event(LOG_LEVEL_ERROR, WHERE__, ex.getMessage());
      
      // If file is corrupted, remove it.
      if (fs::exists(tpath))
      {
         // Remove tmp-file
         fs::remove(tpath);
      }

      // Failed and do nothing
      return;
   }

   uintmax_t size = fs::file_size(tpath); // Get file size
   ios::streamoff pos(0);
   int counter(0);
   bool isselheader = false;

   switch (getParameters().getHeaderType())
   {
   case e_selHeader:
      isselheader = true;
   case e_clhHeader:
   {
      istream_iterator<CLHMessage> end_iter;
      for (istream_iterator<CLHMessage> iter(ifs); iter != end_iter; ++iter)
      {
         pos = ifs.tellg();

         try
         {
            const char* data = iter->getData(m_data);

            // Analyze data message
            if (getParameters().hasXmNo())
            {
               // XM number expected
               uint16_t xmno;
               if (parseXmNo(data, xmno) == false)
               {
                  ostringstream s;
                  s << *this << endl;
                  s << "XM number missing or faulty.";
                  Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
               }
            }
            if (getParameters().hasMsgNo())
            {
               // Message number expected
               size_t pos = regex_match(data, s_msgnopattern);
               if (pos != 0)
               {
                  ostringstream s;
                  s << *this << endl;
                  s << "Message number missing or faulty.";
                  Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
               }
            }
            counter++;
            
            // Insert message in log file
            // Local time received from CP, convert it to UTC time
            insert(Time(iter->getTime(), true), data, iter->getSize(), isselheader);
         }
         catch (Exception& ex)
         {
            ex << " Event ignored.";
            Logger::event(LOG_LEVEL_WARN, ex);
         }
      }
   }
   break;

   case e_tesrvHeader:
   {
      istream_iterator<TESRVMessage> end_iter;
      for (istream_iterator<TESRVMessage> iter(ifs); iter != end_iter; ++iter)
      {
         pos = ifs.tellg();
         const char* data = iter->getData(m_data);
         counter++;

         // Insert message in log file
         // Local time received from CP, convert it to UTC time
         insert(Time(iter->getTime(), true), data, iter->getSize());
      }
   }
   break;

   case e_noHeader:
   {
      char* data = m_data;

      try 
      {
         filebuf* pbuf = ifs.rdbuf();
         pbuf->sgetn(data, size);
         pos = ifs.tellg();
         counter = 1;
      }
      catch (...)
      {
         // Close file hanlder
         ifs.close();
         // Delete file
         fs::remove(tpath);
         
         // Write to log
         Exception ex(Exception::system(), WHERE__);
         ex << "Error while handling no header tmp files." << endl;
         Logger::event(ex);
         
         return;
      }

      // Insert message in log file
      insert(Time::now(), data, size);
   }
   break;

   default:
      assert(!"Illegal header type.");
   }
   ifs.close();

   ios::streampos rem = size - pos;
   if (rem)
   {
      // Last message was incomplete - move it to beginning of the file
      fs::fstream fs(tpath, ios::binary | ios::in | ios::out);
      if (fs.is_open())
      {
         try {
            char* data = m_data;
            fs.seekg(-rem, ios::end);
            fs.read(data, rem);
            fs.seekp(0, ios::beg);
            fs.write(data, rem);
            fs.close();

            // Truncate the file
            truncate(tpath.c_str(), rem);
      
            Logger logger(LOG_LEVEL_INFO);
            if (logger)
            {
               ostringstream s;
               s << *this << endl;
               s << "Incomplete message was retained.";
               logger.event(WHERE__, s.str());
            }
         }
         catch (std::exception&)
         {
            Logger::event(LOG_LEVEL_ERROR, WHERE__, "Failed in handling incompleted messages.");
            fs.close();
            fs::remove(tpath);
         }
      }
      else
      {
         Exception ex(Exception::system(), WHERE__);
         ex << *this << endl;
         ex << "Failed to open file " << tpath << ".";
         ex.sysError();
         throw ex;
      }

   }
   else
   {
      // Delete file
      fs::remove(tpath);
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

   // Logger information
   Logger logger(LOG_LEVEL_DEBUG);
   if (logger)
   {
      ostringstream s;
      s << *this << endl;
      s << "Appended " << counter << " event(s).";
      logger.event(WHERE__, s.str());
   }
}

//----------------------------------------------------------------------------------------
// Parse XM number
//----------------------------------------------------------------------------------------
bool AppendTask::parseXmNo(const char* data, uint16_t& xmno) const
{
   const string str(data, 8);
   if (str.substr(0, 3) == "XM ")
   {
      size_t pos = str.find(':');
      if (pos == string::npos) return false;
      try
      {
         xmno = boost::lexical_cast<uint16_t>(str.substr(3, pos - 3));
      }
      catch (std::exception&)
      {
         return false;
      }
      return (xmno >= 1) && (xmno <= XmFilter::s_maxmno);
   }
   else if (str.find("KERNEL:") == 0)
   {
      return true;
   }
   else if (str.find("PARENTX:") == 0)
   {
      return true;
   }
   else
   {
      return false;
   }
}

//----------------------------------------------------------------------------------------
//   Get divider iterator
//----------------------------------------------------------------------------------------
AppendTask::FILELISTITER AppendTask::getDivIter()
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
void AppendTask::maintainLogSize()
{
   // Log reached max size - delete a log subfile
   const fs::path& logdir = getLogDir();

   FILELISTITER iter = m_filelist.begin();   // Oldest subfile
   const Time& stop = iter->second;          // Stop time for oldest subfile
   int64_t diff = Time::now() - stop;
   uint64_t difft = 0;                       // Convert to uint64_t
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
      // Max time is not reached - delete subfile according to the divider
      iter = getDivIter();
      maxtime = false;
   }

   while (!is_deleted && iter != m_filelist.end())
   {
      Time time = iter->first;
      FILELISTITER iter_next = iter + 1;
      //iter = m_filelist.erase(iter);
      if (iter_next == m_filelist.end())
      {
         // This is the last file, make sure it gets closed
         if (m_fs.is_open())
         {
            m_fs.close();
         }
      }

      const string& file = createFileName(time);
      const fs::path& path = logdir / file;
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
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
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
// Read event log
//----------------------------------------------------------------------------------------
Period AppendTask::readEvents(
            const Period& period,
            const Filter& filter,
            t_eventcb eventcb,
            ostream& os
            ) const
{
   set<fs::path> list;

   // Create a list with the log subfiles
   const fs::path& logdir = getLogDir();

   // Check log dir exist
   if (fs::exists(logdir) == false)
   {
      return Period(Time(), Time());
   }

   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const fs::path& path = *iter;
      if (regex_match(path.filename().c_str(), getParameters().getLogFile()))
      {
         list.insert(path);
      }
   }

   Time start;
   Time stop;

   // Print events in reverse order from the log files
   for (set<fs::path>::const_reverse_iterator riter = list.rbegin();
        riter != list.rend();
        ++riter)
   {
      const fs::path& path = *riter;
      fs::ifstream fs(path, ios_base::binary);
      if (fs.is_open() == false)
      {
         // Oops, file was deleted maybe due to maintenance purpose
         // Continue with the next file then
         continue;
      }

      t_header header;
      readData(fs, header);
      const Time& first(header.m_aptime);
      size_t offset = header.m_prev;
      fs.seekg(offset);
      readData(fs, header);
      const Time& last(header.m_aptime);

      if (period.first() > last) break;                  // Stop time reached
      if (period.last() >= first)
      {
         // Print events
         fs.seekg(offset);
         size_t next;

         do
         {
            next = offset;
            readData(fs, header);
            const Time& cptime(header.m_cptime);
            const Time& aptime(header.m_aptime);
            if (period.first() > aptime) break;       // Stop time reached
            if (period.last() >= aptime)
            {
               bool found = true;
               if (eventcb)
               {
                  size_t size = header.m_size;
                  boost::scoped_array<char> sarray(new char[size]);
                  char* const buf = sarray.get();
                  fs.read(buf, size);

                  // Filter the printout
                  found = filter.test(buf);
                  if (found)
                  {
                     eventcb(cptime, aptime, size, buf, os);
                  }
               }
               if (found)
               {
                  if (stop.empty())
                  {
                     stop = aptime;
                  }
                  start = aptime;
               }
            }
            offset = header.m_prev;
            fs.seekg(offset);
         }
         while (next);
      }
   }

   // Start & stop time not found
   if (start.empty() || stop.empty())
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
// List time for first and last event
//----------------------------------------------------------------------------------------
Period AppendTask::listEvents(const Period& period) const
{
   return readEvents(period, NoFilter());
}

//----------------------------------------------------------------------------------------
// Transfer log files for a CP or blade
//----------------------------------------------------------------------------------------
void AppendTask::transferLogs(const Period& period, const Filter& filter) const
{
   const fs::path& tempfile = "temp";
   fs::ofstream fs(tempfile, ios_base::binary);
   if (fs.is_open() == false)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << *this << endl;
      ex << "Failed to create file " << tempfile << ".";
      ex.sysError();
      throw ex;
   }

   Time start;
   Time stop;

   try
   {
      // Transfer append logs
      const Period& tperiod = readEvents(
                                      period,
                                      filter,
                                      printLogEvent,
                                      fs
                                      );

      if (tperiod.empty() == false)
      {
         start = tperiod.first();
         stop = tperiod.last();
      }
   }
   catch (StartGreatStopTimeException& ex)
   {
      start = ex.getStartTime();
      stop = ex.getStopTime();
   }

   fs.close();

   if (start.empty() || stop.empty())
   {
      // Nothing to transfer
      fs::remove(tempfile);
   }
   else
   {
      // Insert file in archive
      const fs::path& dir = getParentDir();
      fs::create_directories(dir);

      ostringstream s;
      s << getParameters().getFilePrefix() << "_" << start << "__" << stop << ".log";
      const fs::path& destfile = dir / s.str();
      fs::rename(tempfile, destfile);

      Common::archive(destfile, "archive");

      // Start time greater than stop time
      if (start > stop)
      {
         throw StartGreatStopTimeException(start, stop, WHERE__);
      }
   }
}

//----------------------------------------------------------------------------------------
// Print a log event to the stream
//----------------------------------------------------------------------------------------
void printLogEvent(
         const Time& cptime,
         const Time& aptime,
         size_t size,
         const char* buf,
         ostream& os
         )
{
   if (cptime.empty() == false)
   {
      os << Time::e_long << cptime << "  ";
   }
   os << "AP time: " << aptime << endl;

   // Trim output so it always ends with two new lines
   int i;
   for (i = size - 1; i >= 0; i--)
   {
      if (!isspace(buf[i])) break;
   }
   os.write(buf, i + 1);
   os << endl << endl;
}

//----------------------------------------------------------------------------------------
// Dump header (for test)
//----------------------------------------------------------------------------------------
void AppendTask::dumpHeader(t_header header) const
{
   cout << "Prev. ptr: " << header.m_prev << endl;
   cout << "CP Time:   " << Time::e_pretty << Time(header.m_cptime) << endl;
   cout << "AP Time:   " << Time::e_pretty << Time(header.m_aptime) << endl;
   cout << "Size:      " << header.m_size << endl;
   cout << endl;
}

//----------------------------------------------------------------------------------------
// Set Flag for handling SEL log quota
//----------------------------------------------------------------------------------------
void AppendTask::enableSELDivider()
{
   m_seldivider = true;
}

}
