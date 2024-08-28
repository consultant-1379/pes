//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      engine.cpp
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
//      PA1    2013-03-01   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2013-04-02   UABTSO      CLH adapted to APG43 on Linux.
//      PA4    2013-08-01   XLOBUNG     CLH for AP2.
//      PA5    2014-06-18   XLOBUNG     Add MAUS logs
//      PA6    2017-05-31   XBIPPOL     HV81258
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

// Increase the FD size. Need to be put before <select.h>
#define FD_SETSIZE 2048

#include "engine.h"
#include <common.h>
#include <exception.h>
#include <loginfo.h>
#include <logger.h>
#include <eventhandler.h>
#include <acs_apgcc_paramhandling.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <sys/select.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <boost/bind.hpp>
#include <ACS_CS_API.h>
#include <mausinfo.h>

using namespace std;

namespace PES_CLH {

const string Engine::s_tesrv = "tesrv";
const string Engine::s_tracelog_cpa = "/var/cpftp/tracelog_cpa";
const string Engine::s_tracelog_cpb = "/var/cpftp/tracelog_cpb";
const string Engine::s_tesrv_cpa = "/data/cps/logs/tesrv/cpa";
const string Engine::s_tesrv_cpb = "/data/cps/logs/tesrv/cpb";

bool Engine::stoppoint = false;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Engine::Engine():
m_cptable(this, reset),
m_boardTable(this, reset),
m_cptasklist(),
m_seltasklist(),
m_seltask(),
m_selap2task(),
m_inotify(),
m_architecture(),
m_apbm(),
m_errorcount(0),
m_fds(),
m_endfd(-1),
m_inotifyfd(-1),
m_timerfd(-1),
m_trapfd(-1),
m_maxfd(0),
m_hascp2(false),
m_runningap(),
m_enableselap2(false),
m_ftpthread(),
m_transfertask(),
m_initiatedlogs(false),
m_cptablesubscribed(false),
m_hwctablesubscribed(false),
m_isAPZ21240_21250(false)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Engine::~Engine()
{
}

//----------------------------------------------------------------------------------------
// Read log parameters from the parameter handling table
//----------------------------------------------------------------------------------------
bool Engine::getLogParameters(
                  const string& logname,
                  uint32_t& maxsize,
                  Time& maxtime,
                  uint16_t& divider
                  )
{
   acs_apgcc_paramhandling par;
   char logpar[64];
   ACS_CC_ReturnType result;
   result = par.getParameter<64>(
                     "centralLogHandlerId=1",
                     "prv" + logname,
                     logpar
                     );
   if (result != ACS_CC_SUCCESS)
   {
      ostringstream s;
      s << "Failed to read parameter '" << logname << "'." << endl;
      s << par.getInternalLastErrorText() << endl;
      s << "Default values will be used.";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      // Report to event handler
      EventHandler::send(Exception::parameter().first, s.str());

      return false;
   }

   const char s_space[] = " \t";
   char* tstr = strdup(logpar);
   char* ptr = tstr;
   char* ptr1;

   uint32_t hours;
   uint16_t minutes(0);
   try
   {
      // Parse maxsize
      ptr += strspn(ptr, s_space);
      if (!isdigit(*ptr)) throw ptr;
      maxsize = strtoul(ptr, &ptr1, 10) * 1000;
      if (ptr == ptr1) throw ptr;
      ptr = ptr1 + strspn(ptr1, s_space);
      if (*ptr != ',') throw ptr;
      if (maxsize == UINT_MAX) throw ptr;

      // Parse maxtime
      ptr++;
      ptr += strspn(ptr, s_space);
      if (!isdigit(*ptr)) throw ptr;
      hours = static_cast<uint32_t>(strtoul(ptr, &ptr1, 10));
      if (ptr == ptr1) throw ptr;
      if (*ptr1 == ':')
      {
         ptr = ptr1 + 1;
         if (!isdigit(*ptr)) throw ptr;
         minutes = static_cast<uint16_t>(strtoul(ptr, &ptr1, 10));
         if (ptr + 2 != ptr1) throw ptr;
      }
      ptr = ptr1 + strspn(ptr1, s_space);
      if (*ptr != ',') throw ptr;

      // Parse divider value
      ptr++;
      ptr += strspn(ptr, s_space);
      if (!isdigit(*ptr)) throw ptr;
      divider = static_cast<uint16_t>(strtoul(ptr, &ptr1, 10));
      if (ptr == ptr1) throw ptr;
      ptr = ptr1 + strspn(ptr1, s_space);
      if (*ptr != 0) throw ptr;

      free(tstr);
   }
   catch (char*)
   {
      ostringstream s;
      s << "Error detected when parsing string for parameter '" << logname << "'."
        << endl;
      s << tstr << endl;
      size_t pos = ptr + 1 - tstr;
      s << setfill(' ') << setw(pos) << '^' << endl;
      s << "Default values will be used.";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      // Report to event handler
      EventHandler::send(Exception::parameter().first, s.str());

      free(tstr);

      return false;
   }

   // Validate maxsize
   if (maxsize < 3 * BaseParameters::s_maxfilesize)
   {
      ostringstream s;
      s << "Error detected when parsing string for parameter '" << logname << "'."
        << endl;
      s << "Max size value incorrect.";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      // Report to event handler
      EventHandler::send(Exception::parameter().first, s.str());

      return false;
   }

   // Validate maxtime
   if ((hours > 1000000) || (minutes > 59))
   {
      ostringstream s;
      s << "Error detected when parsing string for parameter '" << logname << "'."
        << endl;
      s << "Max time value incorrect.";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      // Report to event handler
      EventHandler::send(Exception::parameter().first, s.str());

      return false;
   }
   maxtime = hours * Time::s_hour + minutes * Time::s_minute;

   // Validate divider
   if (divider > 100)
   {
      ostringstream s;
      s << "Error detected when parsing string for parameter '" << logname << "'."
        << endl;
      s << "Divider value incorrect.";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      // Report to event handler
      EventHandler::send(Exception::parameter().first, s.str());

      return false;
   }

   return true;
}

//----------------------------------------------------------------------------------------
// Set log parameters read from the parameter handling table
//----------------------------------------------------------------------------------------
void Engine::setLogParameters()
{
   uint32_t maxsize;
   Time maxtime;
   uint16_t divider;
   bool ok;

   LogTable logtable;
   logtable.initialize();
   const LogTable::LIST& loglist = logtable.getList();

   // For all log types
   for (LogTable::LISTCITER iter = loglist.begin(); iter != loglist.end(); ++iter)
   {
      BaseParameters& parameters = iter->getParameters();
      if (!parameters.getLogFile().str().empty())
      {
         // Read log parameters
         ok = getLogParameters(parameters.getLogName(), maxsize, maxtime, divider);
         if (ok)
         {
            // Set log parameters
            parameters.setParameters(maxsize, maxtime, divider);
         }
      }
   }
}

//----------------------------------------------------------------------------------------
// Subscribe for APBM trap events
//----------------------------------------------------------------------------------------
void Engine::apbmSubscribe()
{
   int result;
   int bitmap = 0;
   switch (m_architecture)
   {
   case Common::SCB: bitmap = 0x3ffffff;  break;        // Slot 0-25: SCB-RP; 1-24: other boards
   case Common::SCX: bitmap = 0x17FFFFFF; break;        // Add slot for SCX and CMX
   case Common::DMX: bitmap = 0;          break;        // Not support SEL in DMX
   case Common::VIRTUALIZED: bitmap = 0;  break;        // Not yet implemented
   default: assert(!"Illegal node architecture");
   }
   result = m_apbm.subscribe_trap(bitmap, m_trapfd);
   if (result == 0)
   {
      FD_SET(m_trapfd, &m_fds);                    // APBM trap file descriptor
      m_maxfd = std::max(m_trapfd, m_maxfd);
   }
   else
   {
      ostringstream s;
      s << "Failed to subscribe for trap message events." << endl;
      s << "Fault code: " << result;

      // Subscription failed
      Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());

      // Set timer to retry again after 5 s
      itimerspec time = {{0, 0}, {5, 0}};
      result = timerfd_settime(m_timerfd, 0, &time, NULL);
      if (result != 0)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to set timer object.";
         ex.sysError();
         throw ex;
      }
   }
}

//----------------------------------------------------------------------------------------
// Initiate all logs
//----------------------------------------------------------------------------------------
void Engine::initiateLogs()
{
   // Initiate all log entries
   const fs::path& apzpath = BaseParameters::getApzLogsPath();
   const fs::path& cpspath = BaseParameters::getCpsLogsPath();

   // Create log entries
   bool sellogging = false;
   bool rplogging = false;

   if (m_cptable.isMultiCPSystem() == false)
   {
      // One CP system
      Logger::event(LOG_LEVEL_INFO, WHERE__, "A one CP system was detected.");

      // Create a list of all log entries
      const CPInfo &cpinfo = m_cptable.get();
      try
      {
         create(cpinfo);
      }
      catch (Exception& ex)
      {
         // Failed to create
         Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      }

      try
      {
         t_apzSystem apzsystem = cpinfo.getAPZSystem();
         switch (apzsystem)
         {
         case e_classic:
            sellogging = false;
            break;

         case e_apz21240:
           sellogging = false;
           m_isAPZ21240_21250 = true;
           break;

         case e_apz21250:
            sellogging = false;
            rplogging = true;
            m_isAPZ21240_21250 = true;
            break;

         case e_apz21255:
         case e_apz21260:
            sellogging = true;
            rplogging = true;
          break;

         default:
            Exception ex(Exception::internal(), WHERE__);
            ex << "Unexpected APZ type. No SEL and RP log.";
            Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
            break;
         }
      }
      catch (Exception& ex)
      {
         Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      }
   }
   else
   {
      try
      {
         // Multi CP system
         Logger::event(LOG_LEVEL_INFO, WHERE__, "A multi CP system was detected.");

         // Remove CLH directories that are not used
         cleanupLogDir(apzpath);

         // Remove TESRV directories that are not used
         cleanupLogDir(cpspath);

         for (CPTable::const_iterator iter = m_cptable.begin();
              iter != m_cptable.end();
              ++iter)
         {
            const CPInfo& cpinfo = *iter;
            const string& cpname = cpinfo.getName();
            fs::path cpdir;
            fs::path sympath;

            // Create CP directory for APZ
            cpdir = apzpath / cpname;
            Common::createDirAndLink(cpdir);

            // Create CP directory for CPS
            cpdir = cpspath / cpname;
            Common::createDirAndLink(cpdir);

            // Create a list of all log entries
            try
            {
               create(cpinfo);
            }
            catch (Exception& ex)
            {
               // Failed to create
               Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
            }
         }
      }
      catch (Exception& ex)
      {
         Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      }
      sellogging = true;
   }

   if (sellogging)
   {
      // Initiate System Event Logs (SEL)
      try
      {
         // Get node architecture
         m_architecture = Common::getNodeArchitecture();
         Logger logger(LOG_LEVEL_INFO);
         switch (m_architecture)
         {
         case Common::SCB:
            if (logger)
            {
               ostringstream s;
               s << "SCB architecture was detected.";
               logger.event(WHERE__, s.str());
            }
            // Only SCB-RPs need to be hanlded
            m_enableselap2 = true;
            break;

         case Common::SCX:
            if (logger)
            {
               ostringstream s;
               s << "SCX architecture was detected.";
               logger.event(WHERE__, s.str());
            }

            // Create list of eGEM2 subracks
            for (BoardTable::const_iterator iter = m_boardTable.begin();
                 iter != m_boardTable.end();
                 ++iter)
            {
               const Magazine& magazine = iter->getMagazine();
               m_subracklist.insert(SUBRACKMAP::value_type(magazine[0], magazine));
            }
            break;

         case Common::DMX:      // Not support SEL
            if (logger)
            {
               ostringstream s;
               s << "DMX architecture was detected.";
               logger.event(WHERE__, s.str());
            }
            break;
            
         case Common::VIRTUALIZED:      // Not support SEL
            if (logger)
            {
               ostringstream s;
               s << "VIRTUALIZED architecture was detected.";
               logger.event(WHERE__, s.str());
            }
            break;

         default:
            assert(!"Illegal node architecture");
         }

         // Subscribe for trap message events
         if ((m_runningap == Common::AP1) || (m_runningap == Common::AP2 && m_enableselap2))
         {
            apbmSubscribe();
         }

         // Create directory for the (non CPUB) System Event Log (SEL)
         const fs::path& cphwdir = m_seltask.getLogDir();
         Common::createDirsAndLinks(cphwdir);

         if (m_runningap == Common::AP1 && m_hascp2 && m_enableselap2)
         {
            // Initiate for handling SEL tmp file from AP2
            // Monitor the /cphw_ap2/sel
            m_selap2task.setAP2();
            const fs::path& cphwap2dir = m_selap2task.getLogDirAP2();
            Common::createDirsAndLinks(cphwap2dir);

            // Change quota to 90% and 10% for non-cpub sel for AP1 and AP2 respectively
            Logger logger(LOG_LEVEL_INFO);
            if (logger)
            {
               std::ostringstream s;
               s << "Log type: SEL AP1" << std::endl;
               s << "Max size changed to: " << m_seltask.getParameters().getMaxsize() * 90 / 100000
                     << " kbytes" << std::endl;
               s << "Log type: SEL AP2" << std::endl;
               s << "Max size changed to: " << m_selap2task.getParameters().getMaxsize() * 10 / 100000
                                 << " kbytes" << std::endl;
               logger.event(WHERE__, s.str());
            }
            // Set flag to change the sel divider to 90% and 10% ratio.
            m_seltask.enableSELDivider();
            m_selap2task.enableSELDivider();
            m_selap2task.open();
            // Create a monitor task for this directory
            BaseTask* const selap2 = createTask(e_sel);
            selap2->setNonCPUB(true);
            uint32_t mask = IN_CLOSE_WRITE;
            int hwatch = m_inotify.addWatch(cphwap2dir.c_str(), mask);

            // Insert in list of CP logs
            m_cptasklist[hwatch] = selap2;

            if (logger)
            {
               ostringstream s;
               s << "Directory " << cphwap2dir << " are being monitored.";
               logger.event(WHERE__, s.str());
            }
         }

         // Should be opened after changing log quota
         if (m_runningap == Common::AP2)
         {
            m_seltask.openSELAP2();
         }
         else
         {
            m_seltask.open();
         }
      }
      catch (Exception& ex)
      {
         // Failed to create
         Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      }
   }

   if (rplogging)
   {
         // Log event
         Logger logger(LOG_LEVEL_INFO);
         if (logger)
         {
            ostringstream s;
            s << "RP is supported.";
            logger.event(WHERE__, s.str());
         }

         // Create directory for RP Log
         BaseTask* const rptaskp = createTask(e_rp);
         const fs::path& rpdir = rptaskp->getLogDir();
         Common::createDirsAndLinks(rpdir);

         rptaskp->open();
         uint32_t mask = IN_CREATE | IN_CLOSE_WRITE | IN_MOVED_TO;
         int hwatch = m_inotify.addWatch(rpdir.c_str(), mask);

        // Insert in list of CP logs
        m_cptasklist[hwatch] = rptaskp;
   }
   
   // Subscribe if any
   subscribeCPTableChanges();
   subscribeHWCTableChanges();

   if(m_isAPZ21240_21250)
   {
      Common::mount(s_tesrv_cpa.c_str(), s_tracelog_cpa.c_str());
      Common::mount(s_tesrv_cpb.c_str(), s_tracelog_cpb.c_str());
   }

   // Subscribe for CP Table and HWC Table changes
   subscribeCPTableChanges();
   subscribeHWCTableChanges();

   Logger::event(LOG_LEVEL_INFO, WHERE__, "All logs initiated.");
}

//----------------------------------------------------------------------------------------
// Terminate all logs
//----------------------------------------------------------------------------------------
void Engine::terminateLogs()
{
   // Close all logs and disable file watches
   for (CPTASKLISTCITER iter = m_cptasklist.begin();
        iter != m_cptasklist.end();
        ++iter)
   {
      BaseTask* const logtaskp = iter->second;
      fs::path logdir;// = logtaskp->getLogDir();
      t_logtype logtype = logtaskp->getParameters().getLogType();

      if (logtaskp->isNonCPUB())
      {
         logdir = m_selap2task.getLogDirAP2();
      }
      else
      {
         logdir = logtaskp->getLogDir();
      }

      if (logtype != e_sel)
      {
         logtaskp->close();                              // Close CP log
         delete logtaskp;
      }
      else
      {
         if (logtaskp->isNonCPUB())
         {
            logtaskp->close();                           // Close CP log
            delete logtaskp;
         }
      }
   }
   m_cptasklist.clear();

   // Terminate System Event Logs (SEL)
   for (SELTASKLISTCITER iter = m_seltasklist.begin();
        iter != m_seltasklist.end();
        ++iter)
   {
      Sel* const seltaskp = iter->second;
      seltaskp->close();                           // Close SEL log
      delete seltaskp;
   }
   m_seltasklist.clear();

   // Close the (non CPUB) System Event Log (SEL)
   m_seltask.close();

   if (m_runningap == Common::AP1 && m_hascp2 && m_enableselap2)
   {
      m_selap2task.close();
   }

   // Unsubscribe for CP Table and HWC Table changes
   unsubscribeCPTableChanges();
   unsubscribeHWCTableChanges();

   if(m_isAPZ21240_21250)
   {
      Common::uMount(s_tracelog_cpa.c_str());
      Common::uMount(s_tracelog_cpb.c_str());
   }

   Logger::event(LOG_LEVEL_INFO, WHERE__, "All logs terminated.");
}

//----------------------------------------------------------------------------------------
// Execute the log engine
//----------------------------------------------------------------------------------------
void Engine::execute()
{
   try
   {
      // Set log parameters
      setLogParameters();
   }catch (std::exception& exp)
   {
      // Can not read IMM parameters
      ostringstream s;
      s << "Can not read IMM parameter table.";
      s << "The default values will be used.";
      Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
   }

   // Here is the main loop for processing the logs
   int errorlevel(0);
   eventfd_t runstate;
   bool needretry = false;

   do
   {
      runstate = e_continue;

      // Reset error counter
      m_errorcount = 0;

      try
      {
         // Set info needed for handling AP2
         setEnvironment();

         // Initiate notification
         initiateNotification();

         // Check that APZ logs and CQS logs path directory is present and DSD is OK
         needretry = false;
         m_initiatedlogs = false;
         if (checkReadyForRunning())
         {
           // Initiate all logs
           initiateLogs();
           m_initiatedlogs = true;
         }
         else
         {
             needretry = true;

         }

         ostringstream s;
         s << "Additional Info:" << endl;
         s << "AP2:" << ((m_runningap == Common::AP2)?"Yes -- ":"No -- ");
         s << "SEL: " << ((m_enableselap2)?"Yes":"No");
         s << endl;
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());

         // Check if CLH is running on AP2
         if (m_runningap == Common::AP2 && m_enableselap2 && !needretry)
         {
            m_ftpthread = boost::thread(&Engine::handleTransferThread, this);
         }

         do
         {

            fd_set fds = m_fds;

            // Wait for an event
            int ret = select(m_maxfd + 1, &fds, NULL, NULL, NULL);
            if (ret == -1)
            {
               if (errno == EINTR)
               {
                  // Signal received
                  handleEintrError();
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

               // Handle CP log event
               handleCPEvent();
            }

            if (FD_ISSET(m_timerfd, &fds))
            {


               uint64_t exp;
               read(m_timerfd, &exp, sizeof(uint64_t));

               if (needretry)
               {
                  checkReadyForRunning();
                  reset(e_restart);
               }
               else
               {
                  // Subscribe for APBM trap event
                  if ((m_runningap == Common::AP1) ||
                        (m_runningap == Common::AP2 && m_enableselap2))
                  {
                     apbmSubscribe();
                  }
              }
            }

            if (FD_ISSET(m_trapfd, &fds))
            {
               // Handle SEL event
               handleSELEvent();
            }

            // Too many errors in handling CP Logs and SEL logs
            if (m_errorcount > 30)
            {
               // Internal restart
               ostringstream s;
               s << "Consecutive errors. Restarting internally.";
               Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());
               
               // Restart internal
               runstate = e_restart;
            }

            if (FD_ISSET(m_endfd, &fds))
            {
               eventfd_read(m_endfd, &runstate);
               Logger logger(LOG_LEVEL_INFO);
               switch (runstate)
               {
               case e_restart:
                    // Handle restart event
                  if (logger)
                  {
                     ostringstream s;
                     s << "Restart was requested.";
                     logger.event(WHERE__, s.str());
                  }
                  break;

               case e_fault:
                  // Handle fault injection
                  if (logger)
                  {
                     Exception ex(Exception::internal(), WHERE__);
                     ex << "Fault injection was requested.";
                     throw ex;
                  }
                  break;

               case e_shutdown:
                  // Handle shutdown event
                  if (logger)
                  {
                     ostringstream s;
                     s << "Shutdown was requested.";
                     logger.event(WHERE__, s.str());
                  }
                  break;

               default:
                  runstate = e_restart;
                  break;
               }
            }

            if (errorlevel > 0) errorlevel--;
         }
         while (runstate == e_continue);

      }
      catch (Exception& ex)
      {
         // Log error
         Logger::event(ex);

         // Report to event handler
         EventHandler::send(ex.getErrCode(), ex.getMessage());

         errorlevel += 5;
         if (errorlevel < 20)
         {
            runstate = e_restart;
         }
         else
         {
            // Too many errors
            runstate = e_fatal;
         }
      }
      catch (std::exception& exp)
      {
         runstate = e_fatal;
         Exception ex(Exception::system(), WHERE__);
         ex << exp.what() << endl;
         Logger::event(ex);
         
         // Report to event handler
         EventHandler::send(ex.getErrCode(), ex.getMessage());
         
         errorlevel += 5;
         if (errorlevel < 10)
         {
            runstate = e_restart;
         }
         else
         {
            // Too many errors
            runstate = e_fatal;
         }
      }
      catch (...)
      {
         runstate = e_restart;
         ostringstream s;
         s << "Exception while handling logs.";
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
      }

      try
      {
         // Terminiate transfer thread
         if (m_runningap == Common::AP2 && m_enableselap2 && !needretry)
         {
            stopFTPThread();
         }

         if (m_initiatedlogs)
         {
            // Terminate all logs
            terminateLogs();
         }

         // Terminate notification
         terminateNotification();

         if (runstate == e_restart)
         {

            // Reset HW and CP Table
            m_cptable.reset();
            m_boardTable.reset();
         }
      }
      catch (Exception& ex)
      {
         Logger::event(ex);
      }
      catch (std::exception& ex)
      {
         ostringstream s;
         s << "Ending exception: " << ex.what() << endl;
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
      }
      catch (...)
      {
         ostringstream s;
         s << "Exception while terminating logs.";
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
      }
      
      // Unsubscribe if any
      unsubscribeCPTableChanges();
      unsubscribeHWCTableChanges();
   }
   while (runstate == e_restart);

   if (runstate == e_fatal)
   {
      // Repeated errors, unrecoverable fault
      Exception ex(Exception::internal(), WHERE__);
      ex << "Repeated errors, bailing out.";

      // Report to event handler
      EventHandler::send(ex.getErrCode(), ex.getMessage());

      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Create all log instances belonging to a CP identity
//----------------------------------------------------------------------------------------
void Engine::create(const CPInfo& cpinfo)
{
   CPID cpid = cpinfo.getCPID();
   t_mauType mauType = e_mauundefined;
   const string& cpname = cpinfo.getName();
   t_apzSystem apzsystem = cpinfo.getAPZSystem();

   LogTable logtable;
   logtable.initialize(apzsystem);
   const LogTable::LIST& loglist = logtable.getList();

   const fs::path& apzpath = BaseParameters::getApzLogsPath();
   const fs::path& cpspath = BaseParameters::getCpsLogsPath();

   // Read MAU Type
   if (cpid >= ACS_CS_API_HWC_NS::SysType_CP)
   {
      mauType = cpinfo.getMAUType();
   }
   
   // Log event
   Logger logger(LOG_LEVEL_INFO);
   if (logger)
   {
      ostringstream s;
      s << "APZ system ";
      if (cpname.empty() == false)
      {
         s << "for CP '" << boost::to_upper_copy(cpname) << "' ";
      }
      s << "is '" << apzsystem << "'." << endl;
      s << "MAU Type: ";
      switch (mauType)
      {
         case e_maus: s << "MAUS."; break;
         case e_maub: s << "MAUB."; break;
         default: s << "UNDEF."; break;
      }
      logger.event(WHERE__, s.str());
   }

   // For all CP sides
   char maxside = (cpid < ACS_CS_API_HWC_NS::SysType_CP) ? e_cpa: e_cpb;
   for (int tcpside = e_cpa; tcpside <= maxside; tcpside++)
   {
      t_cpSide cpside = static_cast<t_cpSide>(tcpside);
      string side;
      switch (cpside)
      {
         case e_cpa: side = "cpa"; break;
         case e_cpb: side = "cpb"; break;
         default:    assert(!"Illegal CP side");
      }

      fs::path cpdir;

      // Create CP side directory for APZ
      cpdir = apzpath / cpname / side;
      Common::createDirsAndLinks(cpdir);

      // Create CP side directory for CPS
      cpdir = cpspath / cpname / s_tesrv / side;
      Common::createDirsAndLinks(cpdir);

      // For all valid log types
      for (LogTable::LISTCITER iter = loglist.begin(); iter != loglist.end(); ++iter)
      {
         const LogInfo& loginfo = *iter;
         const BaseParameters& parameters = loginfo.getParameters();
         t_logtype logtype = parameters.getLogType();
         t_loggroup loggroup = parameters.getLogGroup();

         // Check if stop point is set
         if (Engine::checkStopPoint())
         {
            // Exit the method
            return;
         }

         if (loggroup != e_lgMAU)
         {
            if (logtype != e_rp)
            {
               BaseTask* const logtaskp = createTask(logtype, cpinfo, cpside);
               if (logtype == e_sel)
               {
                  logtaskp->createLogDir();                 // Create log directory
                  if (m_runningap == Common::AP1)
                  {
                     logtaskp->open();                      // Open log
                  }
                  else
                  {
                     logtaskp->openSELAP2();
                  }

                  // Insert in list of CP SEL logs
                  CPKEY cpkey(cpid, cpside);
                  Sel* const seltaskp = dynamic_cast<Sel*>(logtaskp);
                  m_seltasklist.insert(SELTASKLIST::value_type(cpkey, seltaskp));

                  if (m_hascp2)
                  {
                     if (m_runningap == Common::AP1)
                     {
                        // Monitor sel files for CP2
                        const string& cp2name = "cp2";
                        if (cpname.compare(cp2name) == 0)
                        {
                           const fs::path& logdir = logtaskp->getLogDir();
                           // Log event
                           Logger logger(LOG_LEVEL_INFO);
                           if (logger)
                           {
                              ostringstream s;
                              s << "Directory " << logdir << " are being monitored.";
                              logger.event(WHERE__, s.str());
                           }
                           uint32_t mask;
                           mask = IN_CLOSE_WRITE;
                           int hwatcht = m_inotify.addWatch(logdir.c_str(), mask);

                           // Insert in list of CP logs
                           m_cptasklist[hwatcht] = logtaskp;
                        }
                     }
                  }
               }
               else
               {
                  if (logtype == e_trace)
                  {
                     // Trace log
                     createTraceDir(*logtaskp);
                  }
                  else
                  {
                     logtaskp->createLogDir();           // Create log directory
                  }

                  // Add a file watch
                  if (m_runningap == Common::AP1)
                  {
                     logtaskp->open();                      // Open log
                     const fs::path& logdir = logtaskp->getLogDir();
                     uint32_t mask;
                     if (dynamic_cast<DirTask*>(logtaskp))
                     {
                        mask = IN_CREATE | IN_ISDIR;
                     }
                     else
                     {
                        if (logtype != e_xpucore)
                        {
                           mask = IN_CLOSE_WRITE;
                        }
                        else
                        {
                           mask = IN_MOVED_TO;
                        }
                     }
                     int hwatch = m_inotify.addWatch(logdir.c_str(), mask);

                     // Insert in list of CP logs
                     m_cptasklist[hwatch] = logtaskp;
                  }
               }
            }
         }
         else
         {
            if (mauType == e_maus && tcpside == e_cpb)
            {
               int maxep = parameters.getNoEndpoints();

               // Loop all endpoints
               for (int i = 0; i < maxep; i++)
               {
                  MAUSInfo swmau;
                  // Set MAUSInfo
                  fs::path fullpath = apzpath / cpname;

                  switch (maxep)
                  {
                  case 1:
                     fullpath = fullpath / parameters.getPath();
                     break;
                  case 2:
                  case 4:
                     fullpath = fullpath / "mau/maus" / MAUSInfo::getEPName(i) / parameters.getPath();
                     break;
                  }

                  swmau.setPath(fullpath.string());
                  BaseTask* const logtaskp = createTask(logtype, cpinfo, swmau);
                  const fs::path& logdir = logtaskp->getLogDir();
                  logtaskp->createLogDir();
                  logtaskp->open();                      // Open log
                  uint32_t mask;
                  if (logtype != e_mcore)
                  {
                     mask = IN_CLOSE_WRITE;
                  }
                  else
                  {
                     mask = IN_MOVED_TO;
                  }
                  int hwatch = m_inotify.addWatch(logdir.c_str(), mask);

                  // Insert in list of CP logs
                  m_cptasklist[hwatch] = logtaskp;
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------
// Handle a CP log event
//----------------------------------------------------------------------------------------
void Engine::handleCPEvent()
{
   try
   {
      BaseTask* logtaskp = 0;
      int hwatch;
      fs::path logdir;

      Inotify::Event event;
      while (m_inotify.getEvent(event))            // Get a file event
      {
         hwatch = event.getWd();

         // Find event in log table
         CPTASKLISTITER iter = m_cptasklist.find(hwatch);
         if (iter == m_cptasklist.end())
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << "Failed to find notification event in log table.";
            throw ex;
         }
         logtaskp = iter->second;
         t_logtype logtype = logtaskp->getParameters().getLogType();
         // Get log dir in the case of sel log for non-cpub
         if (logtype == e_sel && logtaskp->isNonCPUB())
         {
            logdir = m_selap2task.getLogDirAP2();
         }
         else
         {
            logdir = logtaskp->getLogDir();
         }

         // Disable file watch
         int thwatch = m_inotify.addWatch(logdir.c_str(), IN_DELETE_SELF);
         if (thwatch != hwatch)
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << *logtaskp << endl;
            ex << "Unexpected watch descriptor.";
            throw ex;
         }
         const string& file = event.getName();
         const fs::path& path = logdir / file;
         bool isDone = false;

         if (logtype == e_sel)
         {

         if (logtaskp->isNonCPUB())
         {
            Logger logger(LOG_LEVEL_INFO);
            if (logger)
            {
               ostringstream s;
               s << "New SEL file: " << path << "...";
               logger.event(WHERE__, s.str());
            }

            if (boost::regex_match(file, logtaskp->getParameters().getTempFile()))
            {
               m_selap2task.readMsgs(path);
               isDone = true;
            }
         }
         else
         {
            Logger logger(LOG_LEVEL_INFO);
            if (logger)
            {
               ostringstream s;
               s << "New SEL file: " << path << "...";
               logger.event(WHERE__, s.str());
            }
         }
         }
         //end
         if (boost::regex_match(file, logtaskp->getParameters().getTempFile()))
         {
            if (!isDone)
            {
               // A valid temp. log file - process it
               logtaskp->event(path);
            }
         }
         else
         {
            bool isRemoved = true;
            if (logtype == e_rp)
            {
               // Check format of .rpfile
               const boost::regex rptmpfile(
                     "RP_\\d{1,4}_\\d{1,3}_\\d{1,2}_\\d{8}_\\d{6}_([a-zA-Z0-9]+)._(txt|bin)\\.rpfile");
               if (boost::regex_match(file, rptmpfile))
               {
                   isRemoved = false;

                   // Log event
                   ostringstream s;
                   s << *logtaskp << endl;
                   s << "File " << path << " is kept.";
                   Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
               }
            }
            if (logtype == e_sel)
            {
               const boost::regex sellog = logtaskp->getParameters().getLogFile();

               if (boost::regex_match(file, sellog))
               {
                  isRemoved = false;
               }
            }

            if (isRemoved)
            {
               // Not a valid log file - delete it
               fs::remove(path);
               
               // Log event
               ostringstream s;
               s << *logtaskp << endl;
               s << "File " << path << " deleted. Not a valid log file.";
               Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
            }
         }

         // Enable file watch
         uint32_t mask;
         if (dynamic_cast<DirTask*>(logtaskp))
         {
            mask = IN_CREATE | IN_ISDIR;
         }
         else
         {
            mask = IN_CLOSE_WRITE;
            if (logtype == e_rp)
            {
               mask = IN_CREATE | IN_CLOSE_WRITE | IN_MOVED_TO;
            }
            else
            {
               if (logtype == e_xpucore || logtype == e_mcore)
               {
                  mask = IN_MOVED_TO;
               }
               else
               {
                  mask = IN_CLOSE_WRITE;
               }
            }
         }
         thwatch = m_inotify.addWatch(logdir.c_str(), mask);
         if (thwatch != hwatch)
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << *logtaskp << endl;
            ex << "Unexpected watch descriptor.";
            throw ex;
         }
      }
   }
   catch (Exception& ex)
   {
      m_errorcount++;
      Logger::event(ex);
   }
   catch (std::exception& e)
   {
      m_errorcount++;
      // Boost exception
      Exception ex(Exception::system(), WHERE__);
      ex << e.what();
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Handle a SEL event
//----------------------------------------------------------------------------------------
void Engine::handleSELEvent()
{
   try
   {
      int result;

      // Get trap message
      acs_apbm_trapmessage trapMsg;
      result = m_apbm.get_trap(m_trapfd, trapMsg);
      if (result != 0)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to get trap message, fault code is " << result << ".";
         throw ex;
      }

      // Get information about specific board
      int oid = trapMsg.OID();
      switch (oid)
      {
         case acs_apbm_trapmessage::SENSOR_STATE_CHANGE:
         case acs_apbm_trapmessage::SEL_ENTRY:
         case acs_apbm_trapmessage::BOARD_PRESENCE:
         {
            const vector<int>& values = trapMsg.values();
            if (values.size() >= 2)
            {
               // Log event
               Logger logger(LOG_LEVEL_TRACE);
               if (logger)
               {
                  ostringstream s;
                  s << "Trap message was received." << endl;
                  s << "OID:    " << oid << endl;
                  s << "Values: ";
                  vector<int>::const_iterator iter;
                  for (iter = values.begin(); iter != values.end(); ++iter)
                  {
                     s << *iter << " ";
                  }
                  logger.event(WHERE__, s.str());
               }

               Magazine magazine;
               switch (m_architecture)
               {
               case Common::SCB:      // SCB architecture
               {

                  uint32_t address =
                        (values[0] & 0xf) | (values[0] & 0xf0)<<4 | (values[0] & 0xf00)<<16;
                  magazine = address;
               }
               break;

               case Common::SCX:    // SCX architecture
               {
                  SUBRACKMAPCITER iter = m_subracklist.find(values[0]);
                  if (iter != m_subracklist.end())
                  {
                     magazine = iter->second;
                  }
                  else
                  {
                     Exception ex(Exception::system(), WHERE__);
                     ex << "Failed to find eGEM2 subrack.";
                     throw ex;
                  };
               }
               break;

               case Common::DMX:               // DMX architecture
                  // Not support SEL
                  return;
                  break;
               
               case Common::VIRTUALIZED:      // VIRTUALIZED architecture
                  // Not support SEL
                  return;
                  break;

               default:
                  assert(!"Illegal node architecture");
               }
               Slot slot(values[1]);

               uint32_t length = trapMsg.message_length();
               const char* buf = trapMsg.message();
               ostringstream s;
               vector<int>::const_iterator value_iter;
               for (value_iter = values.begin(); value_iter != values.end(); ++value_iter)
               {
                  s << setw(2) << setfill('0') << hex << uppercase << *value_iter << " ";
               }

               if(oid == acs_apbm_trapmessage::SENSOR_STATE_CHANGE)
                  length = 3;
               else if (oid == acs_apbm_trapmessage::BOARD_PRESENCE)
                  length = 0;
			   
               for (size_t i = 0; i < length; i++)
               {
                  s << setw(2) << setfill('0') << hex << uppercase << (buf[i] & 0xFFu);;
               }
               const string& data = s.str();

               // Find board
               BoardTable::const_iterator iter = m_boardTable.find(magazine, slot);
               if (iter == m_boardTable.end())
               {
                  ostringstream s;
                  s << "Failed to find board info for subrack '"
                    << magazine << "' and slot " << slot << ".";
                  Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
                  return;
               }

               const BoardInfo& boardinfo = *iter;

               // Get HWC Functional Board Name (FBN) Identifier
               uint16_t fbn = boardinfo.getFBN();
               switch (fbn)
               {
                  case ACS_CS_API_HWC_NS::FBN_CPUB:
                  {
                     uint16_t systype = boardinfo.getSysType();
                     switch (systype)
                     {
                        case ACS_CS_API_HWC_NS::SysType_CP:
                        {
                           uint16_t cpid = boardinfo.getSysId();  // CP id
                           t_cpSide cpside;                       // CP side
                           switch (boardinfo.getSide())
                           {
                              case ACS_CS_API_HWC_NS::Side_A: cpside = e_cpa; break;
                              case ACS_CS_API_HWC_NS::Side_B: cpside = e_cpb; break;
                              default:                        assert(!"Illegal CP side");
                           }

                           // Insert a CPUB SEL event
                           if (CPTable::isMultiCPSystem())
                           {
                              CPTable::const_iterator iter = m_cptable.find(cpid);
                              if (iter == m_cptable.end())
                              {
                                 s.unsetf(ios_base::basefield | ios_base::hex | ios_base::uppercase);

                                 Exception ex(Exception::system(), WHERE__);
                                 ex << "Failed to find CP information for CP id " << cpid << ".";
                                 throw ex;
                              }
                           }

                           CPKEY cpkey(cpid, cpside);
                           SELTASKLISTCITER iter = m_seltasklist.find(cpkey);
                           if (iter == m_seltasklist.end())
                           {
                              Exception ex(Exception::internal(), WHERE__);
                              ex << "Failed to find CP id and side in SEL log table.";
                              throw ex;
                           }

                           ostringstream s;
                           s << "Trap OID: " ;
                           if(oid == acs_apbm_trapmessage::SENSOR_STATE_CHANGE)
                              s << "SENSOR_STATE_CHANGE" << endl;
                           else if (oid == acs_apbm_trapmessage::SEL_ENTRY)
                              s << "SEL_ENTRY" << endl;
                           else /* acs_apbm_trapmessage::BOARD_PRESENCE */
                              s << "BOARD_PRESENCE" << endl;
							  
                           s << data;
                           const string& str = s.str().c_str();
                           Sel* const seltaskp = iter->second;

                           if (m_runningap == Common::AP1)
                           {
                              // Store to SEL log
                              seltaskp->insert(Time(), str.c_str(), str.size());
                           }

                           if (m_runningap == Common::AP2)
                           {
                              // Store to tmp file
                              const fs::path& path = seltaskp->getLogDir();
                              seltaskp->saveToFile(path, str.c_str(), str.size());
                           }

                           // Logger information
                           Logger logger(LOG_LEVEL_DEBUG);
                           if (logger)
                           {
                              ostringstream s;
                              s << *seltaskp << endl;
                              s << "Appended 1 event.";
                              logger.event(WHERE__, s.str());
                           }
                        }
                        break;

                        default: ;
                     }
                  }

                  case ACS_CS_API_HWC_NS::FBN_SCBRP:
                  case ACS_CS_API_HWC_NS::FBN_RPBIS:
                  case ACS_CS_API_HWC_NS::FBN_MAUB:
                  case ACS_CS_API_HWC_NS::FBN_APUB:
                  case ACS_CS_API_HWC_NS::FBN_Disk:
                  case ACS_CS_API_HWC_NS::FBN_DVD:
                  case ACS_CS_API_HWC_NS::FBN_GEA:
                  case ACS_CS_API_HWC_NS::FBN_SCXB:
                  case ACS_CS_API_HWC_NS::FBN_IPTB:
                  case ACS_CS_API_HWC_NS::FBN_EPB1:
                  case ACS_CS_API_HWC_NS::FBN_EvoET:
                  case ACS_CS_API_HWC_NS::FBN_CMXB:

                     // Insert a SEL event
                    if (m_runningap == Common::AP1)
                    {
                       // Add to SEL log
                       m_seltask.insertSEL(oid, magazine, slot, data);
                    }

                    if (m_runningap == Common::AP2)
                    {
                       // Store to tmp file
                       const fs::path& path = m_seltask.getLogDir();
                       // Event
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
                       m_seltask.saveToFile(path, str.c_str(), str.size());
                    }
                     break;

                  default:
                     Exception ex(Exception::system(), WHERE__);
                     ex << "Unknown FBN identifier (" << fbn
                        << ") returned for board info for subrack '"
                        << magazine << "' and slot " << slot << ".";
                     throw ex;
               }
            }
            else
            {
               Exception ex(Exception::system(), WHERE__);
               ex << "Invalid trap message.";
               throw ex;
            }
         }
         break;

         case acs_apbm_trapmessage::NIC:
         case acs_apbm_trapmessage::RAID:
         case acs_apbm_trapmessage::APBM_READY:
         case acs_apbm_trapmessage::DISKCONN:
         {
            ostringstream s;
            s << "Unexpected trap message OID: " << oid << ".";
            Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
            return;
         }
            break;

         default:
            Exception ex(Exception::system(), WHERE__);
            ex << "Invalid trap message OID: " << oid << ".";
            throw ex;
      }
   }
   catch (Exception& ex)
   {
      m_errorcount++;
      Logger::event(ex);
   }
   catch (std::exception& e)
   {
      m_errorcount++;
      // Boost exception
      Exception ex(Exception::system(), WHERE__);
      ex << e.what();
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Remove log directories that not are used
//----------------------------------------------------------------------------------------
void Engine::cleanupLogDir(const fs::path& logdir) const
{
   const fs::path& apzpath = BaseParameters::getApzLogsPath();
   const string& cphwdir = m_seltask.getLogDir().parent_path().filename().c_str();
   const string& cphwap2dir = m_selap2task.getLogDirAP2().parent_path().filename().c_str();

   try
   {
      fs::directory_iterator end;
      for (fs::directory_iterator piter(logdir); piter != end; ++piter)
      {
         const fs::path& path = *piter;
         const string& name = path.filename().c_str();
         CPTable::const_iterator iter = m_cptable.find(name);
         bool found = (iter != m_cptable.end());
         fs::file_status stat(symlink_status(path));
         if (fs::is_directory(stat))
         {
            // This is a directory
            if (found == true)
            {
               found = (name == iter->getName());
            }
            else if (logdir == apzpath)
            {
               found = (name == cphwdir);
               if (!found)
               {
                  found = (name == cphwap2dir);
               }
            }
            if (found == false)
            {
               // Not a CP directory - remove it recursively
               fs::remove_all(path);

               // Log event
               Logger logger(LOG_LEVEL_INFO);
               if (logger)
               {
                  ostringstream s;
                  s << "Directory '" << name << "' was removed.";
                  logger.event(WHERE__, s.str());
               }
            }
         }
         else if (fs::is_symlink(stat))
         {
            // This is a symbolic link
            if (found == true)
            {
               found = (name == boost::to_upper_copy(iter->getName()));
            }
            else if (logdir == apzpath)
            {
               found = (name == boost::to_upper_copy(cphwdir));
               if (!found)
               {
                     found = (name == boost::to_upper_copy(cphwap2dir));
               }
            }
            if (found == false)
            {
               // Not a CP symbolic link - remove it
               fs::remove(path);

               // Log event
               Logger logger(LOG_LEVEL_INFO);
               if (logger)
               {
                  ostringstream s;
                  s << "Symbolic link '" << name << "' was removed.";
                  logger.event(WHERE__, s.str());
               }
            }
         }
         else
         {
            // This is a file - remove it
            fs::remove(path);

            // Log event
            Logger logger(LOG_LEVEL_INFO);
            if (logger)
            {
               ostringstream s;
               s << "File '" << name << "' was removed.";
               logger.event(WHERE__, s.str());
            }
         }
      }

      // Remove MAU directories if they are not used
      for (CPTable::const_iterator iter = m_cptable.begin();
              iter != m_cptable.end();
              ++iter)
      {
         const CPInfo& cpinfo = *iter;
         CPID cpid = cpinfo.getCPID();
         if (cpid >= 1000)
         {
            if (cpinfo.getMAUType() != e_maus)
            {
               const string& cpname = cpinfo.getName();
               const fs::path& pathdir = apzpath / cpname / "/mau";
               const fs::path& pathsym = apzpath / cpname / "/MAU";
               
               // Check existing
               if (exists(pathdir))
               {
                  fs::remove_all(pathdir);
                  fs::remove(pathsym);
               }
            }
         }
         
      }
   }
   catch (exception& e)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << e.what();
      Logger::event(ex);
   }
}

//----------------------------------------------------------------------------------------
// Create directory and symbolic link for trace log
//----------------------------------------------------------------------------------------
void Engine::createTraceDir(const BaseTask& logtask) const
{
   // Create trace log directory
   fs::path logdir = BaseParameters::getCpsLogsPath();
   const fs::path& pardir = logtask.getParentDir();
   logdir /= pardir.parent_path() / s_tesrv / pardir.filename();

   fs::path symlink = logtask.getLogDir();

   // Create CLH log directory if it does not exist
   fs::path sympath = symlink.parent_path();
   if (fs::exists(sympath) == false)
   {
      fs::create_directories(sympath);

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << logtask << endl;
         s << "Directory " << sympath << " was created.";
         logger.event(WHERE__, s.str());
      }
   }

   // Create a symbolic link to trace directory
   if (fs::exists(symlink) == false)
   {
      fs::create_symlink(logdir, symlink);

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << logtask << endl;
         s << "Symbolic link " << symlink << " --> " << logdir << " was created.";
         logger.event(WHERE__, s.str());
      }
   }
}

//----------------------------------------------------------------------------------------
// Reset request due to change in tables
//----------------------------------------------------------------------------------------
void Engine::reset(t_runstate runstate)
{
   //HW77717 To check if m_endfd is already closed
   if(m_endfd!=-1) 
   {
   	int ret = eventfd_write(m_endfd, runstate);
   	if (ret != 0)
   	{
      		Exception ex(Exception::system(), WHERE__);
      		ex << "Failed to write to event file descriptor.";
      		ex.sysError();
      		throw ex;
   	}
   }
}

//----------------------------------------------------------------------------------------
// Call reset function
//----------------------------------------------------------------------------------------
void Engine::reset(void* instptr)
{
   Engine* engine = static_cast<Engine*>(instptr);
   engine->reset(e_restart);
}

//----------------------------------------------------------------------------------------
// Set some information
//----------------------------------------------------------------------------------------
void Engine::setEnvironment()
{
   try
   {
      if (m_cptable.isMultiCPSystem())
      {
        for (CPTable::const_iterator iter = m_cptable.begin();
              iter != m_cptable.end();
              ++iter)
        {
            const CPInfo& cpinfo = *iter;
            const string& cpname = cpinfo.getName();
            const string& cp2_name = "cp2";

            if (boost::iequals(cpname, cp2_name))
            {
               // Set if CP2 exists
               m_hascp2 = true;
               break;
            }
        }
        m_runningap = Common::UNKNOW;
      }
   }
   catch (Exception& ex)
   {
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Handle Transfer thread
//----------------------------------------------------------------------------------------
void Engine::handleTransferThread()
{
   try
   {
      if (m_runningap == Common::AP2)
      {
         Logger logger(LOG_LEVEL_INFO);
         if (logger)
         {
             ostringstream s;
             s << "Enter handleFTPThread().";
             logger.event(WHERE__, s.str());
         }

         const fs::path& apzpath = BaseParameters::getApzLogsPath();
         // Set path
         vector<string> paths;
         // SEL Non-cpub path
         fs::path tmppath = apzpath / "cphw/sel";
         paths.push_back(tmppath.c_str());
         // CP2 side A
         tmppath = apzpath / "cp2/cpa/cphw/sel";
         paths.push_back(tmppath.c_str());
         // CP2 side B
         tmppath = apzpath / "cp2/cpb/cphw/sel";
         paths.push_back(tmppath.c_str());
         m_transfertask.setPathsToMonitor(paths);

         // Handler
         m_transfertask.FTPHandler();

         if (logger)
         {
             ostringstream s;
             s << "Exit handleFTPThread().";
             logger.event(WHERE__, s.str());
         }
      }
   }
   catch (Exception& ex)
   {
      // Log error
      Logger::event(ex);
   }
   catch (std::exception& ex)
   {
      ostringstream s;
      s << "Running exception: " << ex.what() << endl;
      Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());
   }
}

//----------------------------------------------------------------------------------------
// Check if APZ and CQS directories exist and DSD is OK
//----------------------------------------------------------------------------------------
bool Engine::checkReadyForRunning()
{
   try
   {
      bool resultapz = false;
      bool resultcqs = false;
      bool resultdsd = false;
      // Check that APZ logs path directory is present
      const fs::path& apzpath = BaseParameters::getApzLogsPath();
      resultapz = fs::exists(apzpath);
      if (resultapz == false)
      {
         Logger logger(LOG_LEVEL_WARN);
         if (logger)
         {
            ostringstream s;
            s << "APZ logs path is missing.";
            logger.event(WHERE__, s.str());
         }
      }

      // Check that CPS logs path directory is present
      const fs::path& cpspath = BaseParameters::getCpsLogsPath();
      resultcqs = fs::exists(cpspath);
      if (resultcqs == false)
      {
         Logger logger(LOG_LEVEL_WARN);
         if (logger)
         {
            ostringstream s;
            s << "CPS logs path is missing.";
            logger.event(WHERE__, s.str());
         }
      }

      resultdsd = checkRunningAP();

      if (resultapz && resultcqs && resultdsd)
      {
         Logger logger(LOG_LEVEL_INFO);
         if (logger)
         {
            ostringstream s;
            s << "CLH is ready to run.";
            logger.event(WHERE__, s.str());
         }


         // Create symbolic link for APZ
         Common::createSymLink(apzpath);

         // Create symbolic link for CPS
         Common::createSymLink(cpspath);

         return true;
      }
      else
      {
         setTimer();

      }
   }
   catch (Exception& ex)
   {
      throw ex;
   }

   return false;
}

//---------------------------------------------------------------------------------------------
// Clear the FD event bits of the file descriptors when error EINTR occurred.
// Since the m_endfd events control the engine behavior, these events shall remain unchanged.
// The CLH running state is set in the sighandler function server.cpp for the below signals: 
// SIGINT, SIGTERM, SIGUSR1, SIGUSR2
//---------------------------------------------------------------------------------------------
void Engine::handleEintrError()
{
   FD_CLR(m_trapfd, &m_fds); 
   FD_CLR(m_inotifyfd, &m_fds); 
   FD_CLR(m_timerfd, &m_fds);  
}

//----------------------------------------------------------------------------------------
// Initiate notification
//----------------------------------------------------------------------------------------
void Engine::initiateNotification()
{
   m_maxfd = 0;

   // Clear the set of file descriptors
   FD_ZERO(&m_fds);

   // Initiate notification for shutdown
   m_endfd = eventfd(0, 0);
   if (m_endfd == -1)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to create event notification.";
      ex.sysError();
      throw ex;
   }

   FD_SET(m_endfd, &m_fds);                        // File descriptor for shutdown
   m_maxfd = m_endfd;

   // Initiate file notifications for CP logs
   m_inotifyfd = m_inotify.open(false);
   FD_SET(m_inotifyfd, &m_fds);                    // Inotify file descriptor
   m_maxfd = std::max(m_inotifyfd, m_maxfd);

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
   m_maxfd = std::max(m_timerfd, m_maxfd);

   Logger::event(LOG_LEVEL_INFO, WHERE__, "All notifications initiated.");
}

//----------------------------------------------------------------------------------------
// Stop FTP thread
//----------------------------------------------------------------------------------------
void Engine::stopFTPThread()
{
   m_transfertask.stopThread();
   m_ftpthread.join();
}

//----------------------------------------------------------------------------------------
// Check set Timer for APZ and CQS dir and DSD
//----------------------------------------------------------------------------------------
void Engine::setTimer()
{
    // Set timer to retry again after 60 s
    itimerspec time = {{0, 0}, {60, 0}};
    int result = timerfd_settime(m_timerfd, 0, &time, NULL);
    if (result != 0)
    {
       Exception ex(Exception::system(), WHERE__);
       ex << "Failed to set timer object.";
       ex.sysError();
       throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Check for getting running AP
//----------------------------------------------------------------------------------------
bool Engine::checkRunningAP()
{
    // Get the current running AP

   //m_runningap = Common::getApNode();
   m_runningap = Common::getApNode();
   if (m_runningap == Common::UNKNOW)
   {
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         ostringstream s;
         s << "Can not get AP node name from DSD.";
         logger.event(WHERE__, s.str());
      }
      return false;
   }

   try
   {
      if (m_cptable.isMultiCPSystem())
      {
        for (CPTable::const_iterator iter = m_cptable.begin();
              iter != m_cptable.end();
              ++iter)
        {
            const CPInfo& cpinfo = *iter;
            t_apzSystem apzsystem = cpinfo.getAPZSystem();
            if(apzsystem == e_undefined)
            {

              Logger logger(LOG_LEVEL_WARN);
              if (logger)
              {
                 ostringstream s;
                 s << "Can not get APZSystem from ACS_CS_API.";
                 logger.event(WHERE__, s.str());
              }
             return false;
            }
        }
      }
   }
   catch (Exception& ex)
   {
      Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
      return false;
   } 


  return true;
}

//----------------------------------------------------------------------------------------
// Terminate notification
//----------------------------------------------------------------------------------------
void Engine::terminateNotification()
{
   // Close shutdown event notification
   close(m_endfd);
   m_endfd = -1;
   
   // Close file notifications for CP logs
   m_inotify.close();
   m_inotifyfd = -1;

   // Close timer for APBM subscription
   if (m_timerfd != -1)
   {
      // Close timer object
      close(m_timerfd);
      m_timerfd = -1;
   }

   if (m_trapfd != -1)
   {
      // Unsubscribe for trap message events
      int result = m_apbm.unsubscribe_trap(m_trapfd);
      if (result != 0)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to unsubscribe for trap message events.";
         throw ex;
      }
      m_trapfd = -1;
   }

   Logger::event(LOG_LEVEL_INFO, WHERE__, "All notifications terminated.");
}

//----------------------------------------------------------------------------------------
// The function handles subscribing CP Table change notification
//----------------------------------------------------------------------------------------
void Engine::subscribeCPTableChanges()
{
   try
   {
      if (CPTable::isMultiCPSystem())
      {
         // Subscribe for changes in the CP table
         ACS_CS_API_SubscriptionMgr* mgr = ACS_CS_API_SubscriptionMgr::getInstance();

         if (mgr)
         {
            ACS_CS_API_NS::CS_API_Result result = mgr->subscribeCpTableChanges(m_cptable);

            if (ACS_CS_API_NS::Result_Success != result)
            {
               Logger::event(LOG_LEVEL_ERROR, WHERE__, "Failed to subscribe for changes in the CP table.");
            }
            else
            {
               Logger::event(LOG_LEVEL_INFO, WHERE__, "Subscribed CP Table successfully.");
            }
         }
      }
   }
   catch (Exception& ex)
   {
      Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
   }
   
   m_cptablesubscribed = true;
}

//----------------------------------------------------------------------------------------
// The function handles un-subscribing CP Table change notification
//----------------------------------------------------------------------------------------
void Engine::unsubscribeCPTableChanges()
{
   try
   {
      if (m_cptablesubscribed)
      {
         // Subscribe for changes in the CP table
         ACS_CS_API_SubscriptionMgr* mgr = ACS_CS_API_SubscriptionMgr::getInstance();

         if (mgr)
         {
            ACS_CS_API_NS::CS_API_Result result = mgr->unsubscribeCpTableChanges(m_cptable);
            
            if (ACS_CS_API_NS::Result_Success != result)
            {
               Logger::event(LOG_LEVEL_ERROR, WHERE__, "Failed to unsubscribe for changes in the CP table.");
            }
            else
            {
               Logger::event(LOG_LEVEL_INFO, WHERE__, "Unsubscribed CP Table successfully.");
            }
         }
      }
   }
   catch (Exception& ex)
   {
      Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
   }
   
   m_cptablesubscribed = false;
}

//----------------------------------------------------------------------------------------
// The function handles subscribing HWC Table change notification
//----------------------------------------------------------------------------------------
void Engine::subscribeHWCTableChanges()
{
   try
   {
      if (!m_hwctablesubscribed)
      {
         // Subscribe for changes in the HWC table
         ACS_CS_API_SubscriptionMgr* mgr = ACS_CS_API_SubscriptionMgr::getInstance();

         if (mgr)
         {
            ACS_CS_API_NS::CS_API_Result result = mgr->subscribeHWCTableChanges(m_boardTable);
            
            if (ACS_CS_API_NS::Result_Success != result)
            {
               Logger::event(LOG_LEVEL_ERROR, WHERE__, "Failed to subscribe for changes in the HWC table.");
            }
            else
            {
               Logger::event(LOG_LEVEL_INFO, WHERE__, "Subscribed HWC table successfully.");
            }
         }
      }
   }
   catch (Exception& ex)
   {
      Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
   }
   
   m_hwctablesubscribed = true;
}

//----------------------------------------------------------------------------------------
// The function handles unsubscribing HWC Table change notification
//----------------------------------------------------------------------------------------
void Engine::unsubscribeHWCTableChanges()
{
   try
   {
      if (m_hwctablesubscribed)
      {
         // Subscribe for changes in the HWC table
         ACS_CS_API_SubscriptionMgr* mgr = ACS_CS_API_SubscriptionMgr::getInstance();

         if (mgr)
         {
            ACS_CS_API_NS::CS_API_Result result = mgr->unsubscribeHWCTableChanges(m_boardTable);
            
            if (ACS_CS_API_NS::Result_Success != result)
            {
               Logger::event(LOG_LEVEL_ERROR, WHERE__, "Failed to unsubscribe for changes in the HWC table.");
            }
            else
            {
               Logger::event(LOG_LEVEL_INFO, WHERE__, "Unsubscribed HWC table successfully.");
            }
         }
      }
   }
   catch (Exception& ex)
   {
      Logger::event(LOG_LEVEL_WARN, WHERE__, ex.getMessage());
   }
   
   m_hwctablesubscribed = false;
}

//----------------------------------------------------------------------------------------
// Set stop point
//----------------------------------------------------------------------------------------
void Engine::setStopPoint(bool stop)
{
   stoppoint = stop;
}

//----------------------------------------------------------------------------------------
// Check if stop point is set
//----------------------------------------------------------------------------------------
bool Engine::checkStopPoint()
{
   return stoppoint;
}

}
