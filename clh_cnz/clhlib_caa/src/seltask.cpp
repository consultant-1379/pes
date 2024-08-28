  //#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      seltask.cpp
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
//      Class for the system event logs (SEL).
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
//      PA1    2012-09-24   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "seltask.h"
#include <acs_apbm_api.h>
#include "common.h"
#include "logger.h"
#include <sstream>



using namespace std;
using namespace boost;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
SelTask::SelTask():
AppendTask(),
m_parameters()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
SelTask::~SelTask()
{
}

//----------------------------------------------------------------------------------------
// Act on an Error event
//----------------------------------------------------------------------------------------
void SelTask::event(const fs::path&)
{
}

//----------------------------------------------------------------------------------------
// Insert event message
//----------------------------------------------------------------------------------------
void SelTask::insertSEL(
         int oid,
         const Magazine& magazine,
         const Slot& slot,
         const std::string& data
         )
{
   ostringstream s;
   s << "Trap OID: " ;
   if(oid == acs_apbm_trapmessage::SENSOR_STATE_CHANGE)
	  s << "SENSOR_STATE_CHANGE" << endl;
   else if (oid == acs_apbm_trapmessage::SEL_ENTRY)
	  s << "SEL_ENTRY" << endl;
   else /* acs_apbm_trapmessage::BOARD_PRESENCE */
	  s << "BOARD_PRESENCE" << endl;
	
   s << "Subrack id: " << magazine << endl;
   s << "Slot number: " << slot << endl;
   s << data;
   const string& str = s.str().c_str();
   AppendTask::insert(Time(), str.c_str(), str.size());

   Logger logger(LOG_LEVEL_DEBUG);
   if (logger)
   {
      std::ostringstream s;
      s << "Subrack id: " << magazine << "   Slot number: " << slot << "   ";
      s << *this << std::endl;
      s << "Appended 1 event.";
      logger.event(WHERE__, s.str());
   }
}

//----------------------------------------------------------------------------------------
// Get log parameters
//----------------------------------------------------------------------------------------
const BaseParameters& SelTask::getParameters() const
{
   return m_parameters;
}

//----------------------------------------------------------------------------------------
// Get log directory
//----------------------------------------------------------------------------------------
fs::path SelTask::getLogDir() const
{
   if (m_issetap2)
   {
      return BaseParameters::getApzLogsPath() / "cphw_ap2/sel";
   }

   return BaseParameters::getApzLogsPath() / m_parameters.getPath();
}

//----------------------------------------------------------------------------------------
// Get log directory
//----------------------------------------------------------------------------------------
fs::path SelTask::getLogDirAP2() const
{
   return BaseParameters::getApzLogsPath() / "cphw_ap2/sel";
}

//----------------------------------------------------------------------------------------
// Create log directory
//----------------------------------------------------------------------------------------
void SelTask::createLogDir() const
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
// Stream textual information about the log task
//----------------------------------------------------------------------------------------
void SelTask::stream(std::ostream& s) const
{
   s << "Log type: " << m_parameters.getLogName();
}

//----------------------------------------------------------------------------------------
// Save event to file
//----------------------------------------------------------------------------------------
void SelTask::saveToFile(const fs::path& path,
                        const std::string& data,      // Data message
                        uintmax_t size                // Data size
               )
{
   if (m_isopen == false)
   {
      // Log is not opened
      Exception ex(Exception::internal(), WHERE__);
      ex << "Log '" << getParameters().getLogName() << "' is not opened.";
      throw ex;
   }

   // Check size
   uintmax_t eventsize = 40 + size;
   if (eventsize > BaseParameters::s_maxfilesize)
   {
      // Event message too big
      Exception ex(Exception::internal(), WHERE__);
      ex << *this << endl;
      ex << "Message size exceeds max allowed size.";
      throw ex;
   }

   const string& filename = m_parameters.getFilePrefix() + ".tmp";

   // Path to file
   const fs::path& filepath = path / filename;

   fs::fstream ifs;

   // Check if sel.tmp does not exist, create it
   if (fs::exists(filepath) == false)
   {
      //Create new file
      createFile(filepath);

      // Log event
       Logger logger(LOG_LEVEL_INFO);
       if (logger)
       {
         ostringstream s;
         s << *this << endl;
         s << "Log file " << filepath << " is created.";
         logger.event(WHERE__, s.str());
       }
   }
   else
   {
      // Open the existing file
      if (m_fs.is_open() == false)
      {
         // Open file
         openFile(filepath);
      }
   }

      m_fs.seekg(0, ios_base::end);         // Move get pointer to end of file
      streamoff filesize = m_fs.tellg();   // File size
      if (filesize + eventsize > 1000)
      {
         // File reached max size
         m_fs.close();                     // Close current file

         // Rename sel.tmp to sel_xxx.tmp and delete if the file list exceeds
         // TO DO
         const string& newname = createTmpFileName(Time::now());
         const fs::path& newpath = path / newname;
         //const fs::path& newpath = "/data/apz/data/sel_000.tmp";
         fs::rename(filepath, newpath);
         //m_fileListToFtpAP2.push_back(newpath);

         // Create new sel.tmp
         createFile(filepath);
      }

      //string selEvent = data + "\n\n";
      string selEvent = data + "\n\n";
      string header = "CLH-hdr\n";   // 8 bytes
      const Time& aptime = Time::now();
      const string& apstrtime = aptime.get(Time::e_tvsec);
      char tmpBufTime[20];
      sprintf(tmpBufTime, "%s000000\n ", apstrtime.c_str());
      header += std::string(tmpBufTime);
      char tmpBufSize[15];
   sprintf(tmpBufSize, "%u\n", static_cast<unsigned int>(selEvent.length()));
   header += std::string(tmpBufSize);

   size_t len = header.length();

   for (size_t i = 0; i < (40 - len); i++)
   {
      header += " ";
   }

   selEvent = header + selEvent;
   uintmax_t sizeEvent = selEvent.length();

      if (filesize == 0)
   {
         m_fs.write(selEvent.c_str(), sizeEvent);            // Write message
   }
   else
   {
      m_fs.seekp(0, ios_base::end);      // Move put pointer to end of file
      m_fs.write(selEvent.c_str(), sizeEvent);            // Write message
   }
      m_fs.flush();
      m_fs.close();
}

//----------------------------------------------------------------------------------------
// Construct a tmp file name from time
//----------------------------------------------------------------------------------------
string SelTask::createTmpFileName(const Time& time)
{
   return m_parameters.getFilePrefix() + "_" + time.get(Time::e_plain) + ".tmp";
}

//----------------------------------------------------------------------------------------
// Read SEL event log
//----------------------------------------------------------------------------------------
Period SelTask::readSELEvents(
            const Period& period,
            const Filter& filter,
            t_eventcb eventcb,
            ostream& os,
            bool checkonly
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
      bool nextfile = false;
      const fs::path& path = *riter;
      fs::ifstream fs(path, ios_base::binary);
      if (fs.is_open() == false)
      {
         nextfile = true;
      }

      if (!nextfile)
      {
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
                        if (checkonly)
                        {
                           stop = aptime;
                           return Period(stop, stop);
                        }
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
// Transfer SEL log files
//----------------------------------------------------------------------------------------
string SelTask::transferSELLogs(
                     const Period& period,
                     const Filter& filter)
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
   const Period& tperiod = readSELEvents(
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
      string result = "";
      return result;
   }
   else
   {
      // Insert file in archive
      const fs::path& dir = getParentDir();
      fs::create_directories(dir);

      ostringstream s;
      // sel_yyyymmdd_hhmmss__yyyymmdd_hhmmss.log
      s << getParameters().getFilePrefix() << "_" << start << "__" << stop;
      
      if (m_issetap2)
      {
         s << "_ap2";
      }
      else
      {
         s << "_ap1";
      }
      s << ".log";

      const fs::path& destfile = dir / s.str();
      fs::rename(tempfile, destfile);
      string result = destfile.c_str();

      // Start time greater than stop time
      if (start > stop)
      {
         throw StartGreatStopTimeException(start, stop, WHERE__);
      }

      return result;
   }
}


}
