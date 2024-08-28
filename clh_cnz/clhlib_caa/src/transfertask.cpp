//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      transfertask.h
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
//      Class for transfer task.
//      
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      XDT/DEK XLOBUNG (Long Nguyen)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-08-01   XLOBUNG     CLH for AP2.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>


#include <exception.h>
#include <logger.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/time.h>

#include <ACS_CS_API.h>

#include "transfertask.h"
#include "ftptask.h"
#include "seltask.h"
#include "ltime.h"


using namespace std;
using namespace boost;

namespace PES_CLH {

const boost::regex TransferTask::m_retmpfile("sel.tmp");
const boost::regex TransferTask::m_relogfile("sel_\\d{8}_\\d{6}.tmp");
const boost::regex TransferTask::m_noncpubpath("/data/apz/logs/cphw/sel");

//============================================================================
// Constructor
//============================================================================
TransferTask::TransferTask():
m_inotify(),
m_fds(),
m_endEvent(-1),
m_inotifyfd(-1),
m_timerfd(-1),
m_maxfd(0),
m_pathList(),
m_monitoredPaths(),
m_filesTransferring(),
m_needtransfer(false)
{
}

//============================================================================
// Destructor
//============================================================================
TransferTask::~TransferTask()
{
   //Trace
   Logger logger(LOG_LEVEL_INFO);
   if (logger)
   {
      ostringstream s;
      s << "Enter TransferTask() Destructor.";
      logger.event(WHERE__, s.str());
   }
}

//============================================================================
// Set paths to be monitored
//============================================================================
bool TransferTask::setPathsToMonitor(vector<string> list)
{
   // Clear
   m_pathList.clear();
   m_filesTransferring.clear();
   m_monitoredPaths.clear();
   
   //Trace
   vector<string>::iterator iter = list.begin();
   while (iter != list.end())
   {
      const string& path = *iter;
      if (fs::exists(path))
      {
         m_pathList.push_back(path);
      }
      iter++;
   }
   return true;
}

//============================================================================
// Set timer for retry in case of failure
//============================================================================
void TransferTask::setTimer(int second)
{
   // Set timer to retry again after second s
   itimerspec time = {{0, 0}, {second, 0}};
   int result = timerfd_settime(m_timerfd, 0, &time, NULL);
   if (result != 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to set timer object.";
      ex.sysError();
      throw ex;
   }
   
   //Trace
   Logger logger(LOG_LEVEL_INFO);
   if (logger)
   {
      ostringstream s;
      s << "Set timer for ftp thread: ";
      s << second << "s.";
      logger.event(WHERE__, s.str());
   }
}

//============================================================================
// FTPHandler
// Send file to AP1
void TransferTask::FTPHandler()
{
   eventfd_t runstate;
   
   do {
      try {
         // Trace log
         ostringstream s;
         s << "Transfer Task:" << endl;
         s << "Enter FTPHandler().";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
      
         // Get AP1 IPs
         getAP1Interfaces();
      
         // Init
         Init();
      
         m_maxfd = 0;
      
         // Clear the set of file descriptors
         FD_ZERO(&m_fds);
         
         // Initiate notification for shutdown
         m_endEvent = eventfd(0, 0);
         if (m_endEvent == -1)
         {
            Exception ex(Exception::system(), WHERE__);
            ex << "Failed to create event notification.";
            ex.sysError();
            throw ex;
         }
      
         FD_SET(m_endEvent, &m_fds);                        // File descriptor for shutdown
         m_maxfd = m_endEvent;
         
         // Initiate file notifications
         m_inotifyfd = m_inotify.open(false);
         FD_SET(m_inotifyfd, &m_fds);                    // Inotify file descriptor
         m_maxfd = max(m_inotifyfd, m_maxfd);
         
         
         // Monitor paths
         FILELIST::iterator iter = m_pathList.begin();
         while (iter != m_pathList.end())
         {
            const string& path = *iter;
            int hwatch = m_inotify.addWatch(path.c_str(), IN_MOVED_TO);
            m_monitoredPaths[hwatch] = path;
            iter ++;
            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Monitoring path: " << path.c_str();
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
         
         // Create timer object
         m_timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
         if (m_timerfd == -1)
         {
            Exception ex(Exception::system(), WHERE__);
            ex << "Failed to create timer object.";
            ex.sysError();
            throw ex;
         }
      
         FD_SET(m_timerfd, &m_fds);                    // Timer file descriptor
         m_maxfd = max(m_timerfd, m_maxfd);
      
         setTimer(300);
      
         do
         {
            runstate = e_continue;
            fd_set fds = m_fds;
      
            // Wait for an event
            int ret = select(m_maxfd + 1, &fds, NULL, NULL, NULL);
            if (ret == -1)
            {
               if (errno == EINTR)
               {
                  // Signal received
                  FD_ZERO(&fds);
                  FD_SET(m_endEvent, &fds);
               }
               else
               {
                  Exception ex(Exception::system(), WHERE__);
                  ex << "Failed to read a notification event.";
                  ex.sysError();
                  throw ex;
               }
            }
      
            if (FD_ISSET(m_inotifyfd, &fds))
            {
               // Import to list
               importNewFileToList();
      
               if (m_needtransfer)
               {
                  transferFiles();
                  m_needtransfer = false;
               }
            }
      
            if (FD_ISSET(m_timerfd, &fds))
            {
               ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Transfer each 5 minutes";
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
               uint64_t exp;
               read(m_timerfd, &exp, sizeof(uint64_t));
      
               // Transfer again after 5m (300s)
               renameTmpFiles();
               transferFiles();
               setTimer(300);
            }
            
            if (FD_ISSET(m_endEvent, &fds))
            {
               eventfd_read(m_endEvent, &runstate);
               ostringstream s;
               s << "Transfer Task:" << endl;
               s << "EndEvent";
               Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
            }
         } while (runstate == e_continue);
      } 
      catch (Exception& ex)
      {
         runstate = e_restart;
         Logger::event(ex);
      }
      catch (std::exception& ex)
      {
         runstate = e_restart;
         ostringstream s;
         s << "Ending exception: " << ex.what() << endl;
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
      }
      
      try {
         m_monitoredPaths.clear();
         
         // Close file notifications
         m_inotify.close();
         m_inotifyfd = -1;
         
         if (m_timerfd != -1)
         {
            // Close timer object
            close(m_timerfd);
            m_timerfd = -1;
         }
         
         // Close shutdown event notification
         close(m_endEvent);
         m_endEvent = -1;
         
         ostringstream s;
         s << "Transfer Task:" << endl;
         s << "End FTPHandler().";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
      }
      catch (std::exception& ex)
      {
         ostringstream s;
         s << "Ending exception: " << ex.what() << endl;
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
      }
   } while (runstate == e_restart);
}

//============================================================================
// A new file appears
//============================================================================
void TransferTask::importNewFileToList()
{
   try
   {
      int hwatch;
      fs::path logdir;

      Inotify::Event event;
      while (m_inotify.getEvent(event))            // Get a file event
      {
         hwatch = event.getWd();
         MONITORPATHS::iterator iter = m_monitoredPaths.find(hwatch);
         if (iter == m_monitoredPaths.end())
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << "Failed to find notification event in log table.";
            throw ex;
         }
         const string& path = iter->second;
         logdir = path;
         // Change file watch
         int thwatch = m_inotify.addWatch(logdir.c_str(), IN_DELETE_SELF);
         if (thwatch != hwatch)
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << logdir.c_str() << endl;
            ex << "Unexpected watch descriptor.";
            throw ex;
         }
         const string& file = event.getName();
         const string& fullpath = path + "/" + file;

         if (regex_match(file, m_relogfile))
         {
             m_filesTransferring.push_back(fullpath);
             m_needtransfer = true;
             ostringstream s;
             s << "Transfer Task:" << endl;
             s << "Add to transferring list: " << fullpath.c_str() << endl;
             Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
         else if (regex_match(file, m_retmpfile))
         {
         }
         else
         {
            // Unknown file, remove it
            fs::remove(fullpath);
            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Removed unknown file: " << fullpath.c_str();
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
         
         // Enable file watch
         thwatch = m_inotify.addWatch(logdir.c_str(), IN_MOVED_TO);
         if (thwatch != hwatch)
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << logdir.c_str() << endl;
            ex << "Unexpected watch descriptor.";
            throw ex;
         }
      }
   }
   catch (std::exception& e)
   {
      // Boost exception
      Exception ex(Exception::system(), WHERE__);
      ex << e.what();
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
//             ipAddressToString()
//
//       Converts an IP address on the form of a 32-bit integer
//       to text string format.
//----------------------------------------------------------------------------------------
string TransferTask::ipAddressToString(uint32_t ipNum)
{
   ostringstream s;
   for (int i = 3; i >= 0; i--)
   {
      s << ((ipNum >> i*8) & 0xFF);
      if (i > 0)
      {
         s << ".";
      }
   }
   return s.str();
}

//============================================================================
// Transfer files
//============================================================================
void TransferTask::transferFiles()
{
   if (m_filesTransferring.size() == 0)
   {
      ostringstream s;
      s << "Transfer Task:" << endl;
      s << "No file to be transferred.";
      Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
      return;
   }
   
   //
   FILELIST::iterator iter = m_filesTransferring.begin();
   while (iter != m_filesTransferring.end())
   {
      //ftp_client_main("192.168.169.2", "anonymous", "", "/apz/logs/cp1", "sel.xxx");
     // File Path
     const string& filepath = *iter;
     const fs::path& fullpath = filepath;
     const fs::path& dirpath = fullpath.parent_path();
     const string& tmpurl = dirpath.c_str();
     string url = tmpurl;
     bool removed = false;

     if (regex_match(tmpurl, m_noncpubpath))
     {
        const fs::path& apzpath = BaseParameters::getApzLogsPath();
        const fs::path& tmp = apzpath / "cphw_ap2/sel";
        url = tmp.c_str();
     }

     // Remove the /data
     url = url.substr(5);

      ostringstream s;
      s << "Transfer Task:" << endl;
      s << "Transferring " << filepath.c_str() << endl;
      s << "To URL: " << url.c_str() << endl;
      Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());

      for (vector<string>::iterator iterIP = m_ap1Interfaces.begin(); iterIP != m_ap1Interfaces.end(); iterIP++)
      {
         const string& ipaddress = *iterIP;
         bool result = ftpTransfer(ipaddress, "anonymous", "", url, filepath);
         if (result)
         {
            // Put it to the first 
            if (iterIP != m_ap1Interfaces.begin())
            {
               iter_swap(iterIP, m_ap1Interfaces.begin());
            }

            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Transfered: " << filepath.c_str() << endl;
            s << "To: " << url.c_str() << endl;
            s << "IP: " << ipaddress.c_str() << endl;
            s << "Removed: " << filepath.c_str();
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
 
            // Delete file
            fs::remove(filepath);
            // Remove out of list
            m_filesTransferring.erase(iter);
            removed = true;
            break;
         }
      }

      if (!removed)
      {
         iter++;
      }

      // No file found
      if (m_filesTransferring.size() == 0)
      {
         return;
      }
   }
}

//============================================================================
// Get IP AP1 addresses
//============================================================================
void TransferTask::getAP1Interfaces()
{
   Logger logger(LOG_LEVEL_INFO);
   // Clear all current IP if any
   m_ap1Interfaces.clear();
   // Get IP addresses for AP1 from CS
   ACS_CS_API_NS::CS_API_Result result;
   // Get the APID (system ID) for the front APG (AP1)
   APID ap1ID;
   result = ACS_CS_API_NetworkElement::getFrontAPG(ap1ID);
   
   if (result == ACS_CS_API_NS::Result_Success)
   {
      ACS_CS_API_HWC* m_hwcInstance;
      // Get HWC table information
      m_hwcInstance = ACS_CS_API::createHWCInstance();
      // Create search instance
      ACS_CS_API_BoardSearch* const boardSearch =
         ACS_CS_API_HWC::createBoardSearchInstance();
         
      if (m_hwcInstance && boardSearch)
      {
         // Limit the board search to boards with AP1 sys ID 
         // and FBN APUB (to avoid getting disks and dvds and stuff)
         boardSearch->setSysId(ap1ID);
         boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_APUB);
         ACS_CS_API_IdList boardList;
         result = m_hwcInstance->getBoardIds(boardList, boardSearch);
         
         if (result == ACS_CS_API_NS::Result_Success)
         {
            for (unsigned int i = 0; i < boardList.size(); i++)
            {
               BoardID boardID = boardList[i];
               
               // IP address for the board
               uint32_t ip;
               
               // Get IP address for planeA
               result = m_hwcInstance->getIPEthA(ip, boardID);
               if (result == ACS_CS_API_NS::Result_Success)
               {
                  //OK, then push to vector
                  m_ap1Interfaces.push_back(ipAddressToString(ip));
                  if (logger)
                  {
                     ostringstream s;
                     s << "Got IPEthA for front APG from CS:" << ipAddressToString(ip);
                     logger.event(WHERE__, s.str());
                  }
               }
               else
               {
                  if (logger)
                  {
                     ostringstream s;
                     s << "Error getting IPEthA for front APG from CS.";
                     logger.event(WHERE__, s.str());
                  }
               }
               
               // Get IP address for planeB
               result = m_hwcInstance->getIPEthB(ip, boardID);
               if (result == ACS_CS_API_NS::Result_Success)
               {
                  //OK, then push to vector
                  m_ap1Interfaces.push_back(ipAddressToString(ip));
                  if (logger)
                  {
                     ostringstream s;
                     s << "Got IPEthB for front APG from CS:" << ipAddressToString(ip);
                     logger.event(WHERE__, s.str());
                  }
               }
               else
               {
                  if (logger)
                  {
                     ostringstream s;
                     s << "Error getting IPEthB for front APG from CS.";
                     logger.event(WHERE__, s.str());
                  }
               }
            }
         }
         else
         {
            if (logger)
            {
               ostringstream s;
               s << "Error getting board ID list from CS.";
               logger.event(WHERE__, s.str());
            }
         }
         
         // Delete search instance
         ACS_CS_API::deleteHWCInstance(m_hwcInstance);
         m_hwcInstance = NULL;
         ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
      }
      else
      {
         if (logger)
         {
            ostringstream s;
            s << "Error getting m_hwcInstance / boardSearch objects from CS.";
            logger.event(WHERE__, s.str());
         }
      }
   }
   else
   {
      if (logger)
      {
         ostringstream s;
         s << "Error getting ID for front APG from CS.";
         logger.event(WHERE__, s.str());
      }
   }
   
   // Add some hardcoded hostnames or IP addresses for AP1 in case getting data
   // from CS failed
   if (m_ap1Interfaces.size() < 4) // Should contain IPEthA and IPEthB for AP side A and B
   {
      // << "EV: Problem getting interfaces for AP1 from CS, will use hard-coded addresses.\n";
      if (logger)
      {
         ostringstream s;
         s << "Problem getting interfaces for AP1 from CS, will use hard-coded addresses.";
         logger.event(WHERE__, s.str());
      }
      m_ap1Interfaces.clear();
      m_ap1Interfaces.push_back("192.168.169.1");
      m_ap1Interfaces.push_back("192.168.169.2");
      m_ap1Interfaces.push_back("192.168.170.1");
      m_ap1Interfaces.push_back("192.168.170.2");
   }
}

//============================================================================
// FTP Function
//============================================================================
bool TransferTask::ftpTransfer(
         string const& server,
         string const& userid,
         string const& password,
         string const& url,
         string const& filename)
{
   bool stored = false;
   try
   {
      boost::asio::io_service ioservice;
      FtpClient ftpclient(
         ioservice, server, userid, password, url, filename);
      boost::thread newthread(
         boost::bind(&boost::asio::io_service::run, &ioservice));
      newthread.join();
      stored = ftpclient.isStored();
      ftpclient.close();
   }
   catch (std::exception& a_error)
   {
      return false;
   }
   return stored;
}

//============================================================================
// Init
//============================================================================
void TransferTask::Init()
{
   Time currentTime = Time::now();
   time_t timeadded = 1; // 1s
   m_filesTransferring.clear();
   for (vector<string>::iterator iter = m_pathList.begin(); iter != m_pathList.end(); iter++)
   {
      // Scan path
      const fs::path& logdir = *iter;
      fs::directory_iterator end;
      for (fs::directory_iterator iterf(logdir); iterf != end; ++iterf)
      {
         const fs::path& path = *iterf;
         const string& file = path.filename().c_str();
         if (regex_match(file, m_relogfile))
         {
            m_filesTransferring.push_back(path.c_str());
            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Put: " << path.c_str() << " to transferring list.";
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
         else if (regex_match(file, m_retmpfile))
         {
            // Temporary log file found
            //tempfile = path;
            SelTask seltask;
            const string& newname = seltask.createTmpFileName(currentTime);
            const fs::path& dir = path.parent_path();
            const fs::path& newpath = dir / newname;
            fs::rename(path, newpath);
            m_filesTransferring.push_back(newpath.c_str());

            // Add 1s to prevent the duplicated filename
            currentTime += timeadded;

            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Renames sel.tmp to " << newpath.c_str() << endl;
            s << "and add to transferring list.";
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
         else
         {
            // Unknown file, remove it
            fs::remove(path);
            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Removed unknown file: " << path.c_str();
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
      }
   }
   sort(
         m_filesTransferring.begin(),
         m_filesTransferring.end(),
         CompareSELFilesbyTime()
   );
}

//============================================================================
// Rename and import to list
//============================================================================
void TransferTask::renameTmpFiles()
{
   Time currentTime = Time::now();
   time_t timeadded = 1; // 1s
   for (vector<string>::iterator iter = m_pathList.begin(); iter != m_pathList.end(); iter++)
   {
      // Scan path
      const fs::path& logdir = *iter;
      fs::directory_iterator end;
      for (fs::directory_iterator iterf(logdir); iterf != end; ++iterf)
      {
         const fs::path& path = *iterf;
         const string& file = path.filename().c_str();
         if (regex_match(file, m_retmpfile))
         {
            // Temporary log file found
            //tempfile = path;
            SelTask seltask;
            const string& newname = seltask.createTmpFileName(currentTime);
            const fs::path& dir = path.parent_path();
            const fs::path& newpath = dir / newname;
            fs::rename(path, newpath);

            // Add 1s to prevent the duplicated filename
            currentTime += timeadded;

            ostringstream s;
            s << "Transfer Task:" << endl;
            s << "Renamed sel.tmp to " << newpath.c_str();
            Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
         }
      }
   }
}

//============================================================================
// Compare SEL files by Time
//============================================================================
bool TransferTask::CompareSELFilesbyTime::operator()(
      const std::string& first,
      const std::string& second
      )
{
   Time firsttime;
   Time secondtime;
   const fs::path& firstpath = first;
   const string& fn1 = firstpath.filename().c_str();
   const fs::path& secondpath = second;
   const string& fn2 = secondpath.filename().c_str();
   
   // Parse filename: sel_yyyymmdd_hhmmss.tmp
   firsttime.set(fn1.substr(4, 8), fn1.substr(13, 6), true);
   secondtime.set(fn2.substr(4, 8), fn2.substr(13, 6), true);
   
   return (firsttime <= secondtime);
}

//============================================================================
// Stop FTP thread
//============================================================================
void TransferTask::stopThread()
{
   int ret = eventfd_write(m_endEvent, e_shutdown);
   if (ret != 0)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to write to event file descriptor.";
      throw ex;
   }
}

}
