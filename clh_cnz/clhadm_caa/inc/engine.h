//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      engine.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2017. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for the CLH log engine.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1416  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-01-08   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2013-04-02   UABTSO      CLH adapted to APG43 on Linux.
//      PA4    2014-06-18   XLOBUNG     Add MAUS logs
//      PA5    2017-05-31   XBIPPOL     HV81258
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef ENGINE_H_
#define ENGINE_H_

#include <logtask.h>
#include <seltask.h>
#include <rptask.h>
#include <inotify.h>
#include <cpinfo.h>
#include <acs_apbm_api.h>
#include <boost/filesystem.hpp>
#include <string>
#include <map>
#include <common.h>

#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <transfertask.h>

namespace fs = boost::filesystem;

namespace PES_CLH {

class BaseTask;

class Engine
{
public:
   typedef std::map<int, BaseTask*> CPTASKLIST;
   typedef CPTASKLIST::iterator CPTASKLISTITER;
   typedef CPTASKLIST::const_iterator CPTASKLISTCITER;

   typedef std::pair<CPID, t_cpSide> CPKEY;
   typedef std::map<CPKEY, Sel*> SELTASKLIST;
   typedef SELTASKLIST::iterator SELTASKLISTITER;
   typedef SELTASKLIST::const_iterator SELTASKLISTCITER;

   enum t_runstate
   {
      e_continue,
      e_restart,
      e_shutdown,
      e_fault,
      e_fatal
   };

   // Constructor
   Engine();

   // Destructor
   ~Engine();

   // Read log parameters from the parameter handling table
   bool getLogParameters(
         const std::string& logname,         // Log name
         uint32_t& maxsize,                  // Max size
         PES_CLH::Time& maxtime,             // Max time
         uint16_t& divider                   // Divider value
         );

   // Set log parameters read from the parameter handling table
   void setLogParameters();

   // Initiate notification
   void initiateNotification();

   // Initiate all logs
   void initiateLogs();

   // Terminate all logs
   void terminateLogs();

   // Terminate notification
   void terminateNotification();

   // Resets the events on all the fd's except for event fd m_endfd
   void handleEintrError();

   // Execute the CLH log engine
   void execute();

   // Handle a CP log event
   void handleCPEvent();

   // Handle a SEL event
   void handleSELEvent();

   // Reset request due to change in tables
   void reset(
         t_runstate state                     // Run state
         );

   // Reset to run state
   static void reset(
         void* instptr                        // Instance pointer
         );

   // Set environment for running
   void setEnvironment(void);

   // Handle Transfer thread (only run on AP2)
   void handleTransferThread();
   
   // Set stop point
   static void setStopPoint(bool stop);
   
   // Check if stop point is set
   static bool checkStopPoint();

private:
   typedef std::map<int, Magazine> SUBRACKMAP;
   typedef SUBRACKMAP::const_iterator SUBRACKMAPCITER;

   // Disable default copy constructor
   Engine(const Engine&);

   // Disable default assignment operator
   Engine& operator=(const Engine&);

   // Subscribe for APBM trap events
   void apbmSubscribe();

   // Create all log instances belonging to a CP identity
   void create(
         const CPInfo& cpinfo
         );

   // Insert entry in the log table
   void insert(
         BaseTask* logtask
         );

   // Remove log directories that not are used
   void cleanupLogDir(
         const fs::path& logdir
         ) const;

   // Create directory and soft link for trace log
   void createTraceDir(
         const BaseTask& logtask
         ) const;

   // Check if APZ and CQS directories exist and DSD is OK
   bool checkReadyForRunning();

   // Check set Timer for APZ and CQS dir and DSD
   void setTimer();

   // Check for getting running AP
   bool checkRunningAP();

   // Stop FTP thread
   void stopFTPThread();
   
   // The function handles subscribing CP Table change notification.
   void subscribeCPTableChanges();
   
   // The function handles un-subscribing CP Table change notification.
   void unsubscribeCPTableChanges();
   
   // The function handles subscribing HWC Table change notification.
   void subscribeHWCTableChanges();
   
   // The function handles un-subscribing HWC Table change notification.
   void unsubscribeHWCTableChanges();

   CPTable m_cptable;                        // CP table
   BoardTable m_boardTable;                  // Board table
   CPTASKLIST m_cptasklist;                  // CP Log task list
   SELTASKLIST m_seltasklist;                // SEL task list
   SelTask m_seltask;                        // System event log (SEL) task object
   SelTask m_selap2task;                     // Handle the non-cpub SEL log transferred from AP2.
   Inotify m_inotify;                        // File notification
   Common::ArchitectureValue m_architecture; // Node architecture
   SUBRACKMAP m_subracklist;                 // List of eGEM2 subracks
   acs_apbm_api m_apbm;                      // APBM instance
   int m_errorcount;                         // Error counter
   fd_set m_fds;                             // File descriptor set
   int m_endfd;                              // Shutdown file descriptor
   int m_inotifyfd;                          // Inotify file descriptor
   int m_timerfd;                            // Timer file descriptor
   acs_apbm::trap_handle_t m_trapfd;         // APBM trap handle
   int m_maxfd;                              // The largest file descriptor;
   bool m_hascp2;                            // Check if CP2 exists
   Common::ApNodeName m_runningap;           // Running on AP
   bool m_enableselap2;                      // Set if SEL AP2 needs to be handled
   boost::thread m_ftpthread;                // Ftp thread (AP2 only)
   TransferTask m_transfertask;              // Transfer task (AP2 only)
   bool m_initiatedlogs;                     // Check if all logs are initiated
   bool m_cptablesubscribed;                 // Check if CP table is subscribed
   bool m_hwctablesubscribed;                // Check if HWC table is subscribed

   static const std::string s_tesrv;
   static const std::string s_tracelog_cpa;
   static const std::string s_tracelog_cpb;
   static const std::string s_tesrv_cpa;
   static const std::string s_tesrv_cpb;
   static bool stoppoint;                    // Check if CLH needs to be stopped in startup phase
   bool m_isAPZ21240_21250;
};

}

#endif // ENGINE_H_
