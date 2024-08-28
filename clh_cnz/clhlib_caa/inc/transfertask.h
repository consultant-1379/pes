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

#ifndef FTPTASK_H_
#define FTPTASK_H_

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include <iomanip>

#include "inotify.h"

namespace fs = boost::filesystem;

namespace PES_CLH {

//========================================================================================
// Class TransferTask
//========================================================================================

class TransferTask
{
public:
   // Constructor
   TransferTask();
   
   // Destructor
   ~TransferTask();
public:
   // FTP handler
   void FTPHandler();
   
   // Initiate
   void Init();
   
   // Set paths to be monitored
   bool setPathsToMonitor(std::vector<std::string> list);
   
   // Stop FTP thread
   void stopThread();

private:
   typedef std::vector<std::string>          FILELIST;
   typedef std::map<int, std::string>        MONITORPATHS;
   
   enum t_ftprunstate
   {
      e_continue,
      e_restart,
      e_shutdown
   };

   // Compare SEL files by Time
   class CompareSELFilesbyTime
   {
   public:
      // Compare operator
      bool operator()(const std::string& first, const std::string& second);
   };
   
      // Set timer for retry in case of failure
   void setTimer(int second);

   // A new file appears
   void importNewFileToList();

   // Transfer
   void transferFiles();
   
   // Rename and import to list
   void renameTmpFiles();

   // Convert IP address to string
   std::string ipAddressToString(uint32_t ipNum);
   
   // Get AP1 IP addresses
   void getAP1Interfaces();
   
   // FTP transfer function
   bool ftpTransfer(
         std::string const& server,
         std::string const& userid,
         std::string const& password,
         std::string const& url,
         std::string const& filename);

   Inotify m_inotify;                        // File notification
   fd_set m_fds;                             // File descriptor set
   int m_endEvent;                           // Shutdown file description
   int m_inotifyfd;                          // Inotify file descriptor
   int m_timerfd;                            // Timer file descriptor
   int m_maxfd;                              // The largest file descriptor
   FILELIST m_pathList;                      // Paths to be monitored
   MONITORPATHS m_monitoredPaths;            // Monitored paths
   FILELIST m_filesTransferring;             // Files need to be transferred
   bool m_needtransfer;                      // True when files are ready for transferring
   std::vector<std::string> m_ap1Interfaces; // Ip addresses of AP1
   static const boost::regex m_retmpfile;    // Regular Expression for tmp file
   static const boost::regex m_relogfile;    // Regular expression for log tmp file
   static const boost::regex m_noncpubpath;  // Regular expression for non-cpub path
};

}

#endif
