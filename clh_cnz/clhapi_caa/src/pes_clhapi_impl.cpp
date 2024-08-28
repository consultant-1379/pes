//----------------------------------------------------------------------------------------
//
//  FILE
//      pes_clhapi_impl.cpp
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
//      Implementation class for for listing and transferring CPS trace logs.
//
//  ERROR HANDLING
//      -
//
//  DOCUMENT NO
//      190 89-CAA 109 1426  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-01-29   UABTSO      New API created for listing and transferring
//                                      CPS trace logs.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------

#include "pes_clhapi_impl.h"
#include <common.h>
#include <exception.h>
#include <logtask.h>
#include <boost/algorithm/string.hpp>
#include <signal.h>

using namespace std;

namespace PES_CLH {

const char Api_impl::s_success[] = "Successful execution.";
Pes_clhapi::t_eventcb Api_impl::s_eventcb = NULL;

//----------------------------------------------------------------------------------------
// Signal handler
//----------------------------------------------------------------------------------------
void sighandler(int)
{
}

//----------------------------------------------------------------------------------------
//	Constructor
//----------------------------------------------------------------------------------------
Api_impl::Api_impl() :
m_errortext()
{
}

//----------------------------------------------------------------------------------------
//	Destructor
//----------------------------------------------------------------------------------------
Api_impl::~Api_impl()
{
}

//----------------------------------------------------------------------------------------
// Read log, receive events in the callback, the call is blocked until all data
// has been received 
//----------------------------------------------------------------------------------------
int Api_impl::readLog(
                  const std::string& cpname,
                  const std::string& cpside,
                  char* &startdate,
                  char* &starttime,
                  char* &stopdate,
                  char* &stoptime,
                  Pes_clhapi::t_eventcb eventcb
                  )
{
   try
   {
      // Analyze CP name and side
      string cp;
      CPTable cptable;
      CPInfo cpinfo;
      t_cpSide tcpside = e_noside;

      if (CPTable::isMultiCPSystem())
      {
         // Multi CP system
         if (cpname.empty())
         {
            Exception ex(Exception::parameter(), WHERE__);
            ex << "CP name is required for a multi CP system";
            throw ex;
         }

         // CP information
         cp = boost::to_lower_copy(cpname);
         CPTable::const_iterator iter = cptable.find(cp);
         if (iter == cptable.end())
         {
            throw Exception(Exception::cpNotDefined(cp), WHERE__);
         }

         cpinfo = *iter;
         CPID cpid = cpinfo.getCPID();

         // CP or BC?
         if (cpid < ACS_CS_API_HWC_NS::SysType_CP)
         {
            // BC has no CP sides
            if (cpside.empty() == false)
            {
               throw Exception(Exception::cpSideNotAllowed(), WHERE__);
            }
            tcpside = e_cpa;
         }
         else
         {
            // Dual CP system - check CP side
            if (cpside.empty() == false)
            {
               tcpside = getCpSide(cpside);
            }
            else
            {
               Exception ex(Exception::parameter(), WHERE__);
               ex << "CP side is missing.";
               throw ex;
            }
         }
      }
      else
      {
         if (cpname.empty() == false)
         {
            Exception ex(Exception::parameter(), WHERE__);
            ex << "CP name is not allowed for a single CP system.";
            throw ex;
         }

         // Check CP side
         if (cpside.empty() == false)
         {
            tcpside = getCpSide(cpside);
         }
         else
         {
            Exception ex(Exception::parameter(), WHERE__);
            ex << "CP side is missing.";
            throw ex;
         }
      }

      // Analyze start and stop times
      Period period(startdate, starttime, stopdate, stoptime);

      // Create a trace task
      const BaseTask* logtaskp = createTask(e_trace, cpinfo, tcpside);

      // Command execution
      Period tperiod;
      if (eventcb == 0)
      {
         tperiod = logtaskp->readEvents(period, NoFilter());
      }
      else
      {
         s_eventcb = eventcb;
         tperiod = logtaskp->readEvents(period, NoFilter(), getLogEvent);
      }

      delete logtaskp;

      if (tperiod.empty() == false)
      {
         string tstartdate;
         string tstarttime;
         string tstopdate;
         string tstoptime;

         tperiod.first().get(tstartdate, tstarttime);
         tperiod.last().get(tstopdate, tstoptime);

         free(startdate);
         free(starttime);
         free(stopdate);
         free(stoptime);

         startdate = strdup(tstartdate.c_str());
         starttime = strdup(tstarttime.c_str());
         stopdate = strdup(tstopdate.c_str());
         stoptime = strdup(tstoptime.c_str());
      }
      else
      {
         startdate[0] = 0;
         starttime[0] = 0;
         stopdate[0] = 0;
         stoptime[0] = 0;
      }
   }
   catch (Exception& ex)
   {
      m_errortext = ex.getMessage();
      return ex.getErrCode();
   }

   m_errortext = s_success;
   return 0;
}

//----------------------------------------------------------------------------------------
// Write logs to a destination
//----------------------------------------------------------------------------------------
int Api_impl::transferLogs(
                  const std::string& cpname,
                  const char* startdate,
                  const char* starttime,
                  const char* stopdate,
                  const char* stoptime,
                  desttype_t desttype
                  )
{
   CPTable cptable;
   CPInfo cpinfo;
   Period period;

   try
   {
      // Analyze CP name
      if (CPTable::isMultiCPSystem())
      {
         // Multi CP system
         if (cpname.empty() == false)
         {
            // CP information
            const string& cp = boost::to_lower_copy(cpname);
            CPTable::const_iterator iter = cptable.find(cp);
            if (iter == cptable.end())
            {
               throw Exception(Exception::cpNotDefined(cp), WHERE__);
            }
            cpinfo = *iter;
         }
      }
      else
      {
         if (cpname.empty() == false)
         {
            Exception ex(Exception::parameter(), WHERE__);
            ex << "CP name is not allowed for a single CP system";
            throw ex;
         }
      }

      // Analyze start and stop times
      period = Period(startdate, starttime, stopdate, stoptime);

      // Analyze destination
      switch (desttype)
      {
         case e_file:
         case e_media:
            break;

         default:
            assert(!"Illegal destination type.");
      }
   }
   catch (Exception& ex)
   {
      m_errortext = ex.getMessage();
      return ex.getErrCode();
   }

   // Command execution
   fs::path supportpath;
   fs::path temppath;
   fs::path archpath;

   // Save current working directory
   fs::path curdir = fs::current_path();

   try
   {
      // Check if emf is busy with transferring another file
      if (desttype == e_media)
      {
         if (Common::isEmfBusy())
         {
            Exception ex(Exception::cmdBusyToTransferToMedia(), WHERE__);
            throw ex;
         }
      }

      // Prepare temporary archive
      supportpath = Common::getFileMPath("supportData");
      //temppath = supportpath / "temp";
      if (desttype == e_file)
      {
         temppath = supportpath / "clh_file";
      }
      else
      {
         temppath = supportpath / "clh_media";
      }

      ostringstream s;
      s << "tesrv_files_" << Time::now();
      string archname = s.str();
      archpath = temppath / archname;

      fs::create_directories(archpath);


      // Change current working directory to archive path
      fs::current_path(archpath);

      // Execute command
      if (cpname.empty() == false)
      {
         // Transfer files for specific CP or blade
         transferLogs(cpinfo, period);
      }
      else
      {
         // In a multi CP system: Transfer log files for all CP:s and blades
         // In a single CP system: Transfer log files
         transferLogs(period);
      }

      const fs::path& archive = archpath / "archive.zip";

      if (fs::exists(archive))
      {
         const string& archfile = archname + ".zip";

         switch (desttype)
         {
         case e_file:
         {
            // Move archive to ftp volume
            fs::rename(archive, supportpath / archfile);

            // Transfer finished, remove temporary files
            fs::remove_all(temppath);
            break;
         }

         case e_media:
         {
            // Copy archive to DVD
            const fs::path& tarchive = archpath / archfile;
            fs::rename(archive, tarchive);
            const fs::path& nbipath = Common::getExtNbiPath(tarchive);
            Common::copyToMedia(nbipath);

            // Run in background
            int ret = daemon(1, 0);
            if (ret != 0)
            {
               Exception ex(Exception::system(), WHERE__);
               ex << "Failed to daemonize process.";
               ex.sysError();
               throw ex;
            }
            // Execute in background
            Common::emfpoll(archfile);

            // Copying finished, remove temporary files
            fs::remove_all(temppath);

            // Exit process when finished
            exit(0);
            break;
         }

         default:
            assert(!"Illegal destination.");
         }
      }
      else
      {
         // Remove temporary archive
         fs::remove_all(temppath);

         // No files to transfer
         cout << "No file to compress." << endl;
      }

      // Restore current working directory
      fs::current_path(curdir);
   }
   catch (Exception& ex)
   {
      // Restore current working directory
      fs::current_path(curdir);

      // Remove temporary archive
      fs::remove_all(temppath);

      // Return code
      m_errortext = ex.getMessage();
      return ex.getErrCode();
   }
   catch (exception& e)
   {
      // Boost exception
      // Remove temporary archive
      fs::remove_all(temppath);

      // Restore current working directory
      fs::current_path(curdir);

      // Return code
      Exception ex(Exception::system(), WHERE__);
      ex << e.what() << ".";
      m_errortext = ex.getMessage();
      return ex.getErrCode();
   }

   m_errortext = s_success;
   return 0;
}

//----------------------------------------------------------------------------------------
// Get error text
//----------------------------------------------------------------------------------------
const char* Api_impl::getErrorText() const
{
   return m_errortext.c_str();
}

//----------------------------------------------------------------------------------------
// Transfer log files for a CP or blade
//----------------------------------------------------------------------------------------
void Api_impl::transferLogs(
      const CPInfo& cpinfo,
      const Period& period
      ) const
{
   CPID cpid = cpinfo.getCPID();

   // Iterate all CP sides
   char maxside = (cpid < ACS_CS_API_HWC_NS::SysType_CP) ? e_cpa: e_cpb;
   for (int cpside = e_cpa; cpside <= maxside; cpside++)
   {
      t_cpSide tcpside = static_cast<t_cpSide>(cpside);
      const BaseTask* logtaskp = createTask(e_trace, cpinfo, tcpside);
      logtaskp->transferLogs(period, NoFilter());
      delete logtaskp;
   }
}

//----------------------------------------------------------------------------------------
// One CP system:   Transfer log files
// Multi CP system: Transfer log files for all CP:s
//----------------------------------------------------------------------------------------
void Api_impl::transferLogs(const Period& period) const
{
   CPTable cptable;
   if (CPTable::isMultiCPSystem())
   {
      // Multi CP system

      // Iterate all CP:s
      for (CPTable::const_iterator iter = cptable.begin();
           iter != cptable.end();
           ++iter)
      {
         transferLogs(*iter, period);
      }
   }
   else
   {
      // One CP system
      transferLogs(cptable.get(), period);
   }
}

//----------------------------------------------------------------------------------------
// Get CP side
//----------------------------------------------------------------------------------------
t_cpSide Api_impl::getCpSide(const string& str) const
{
   const string& side = boost::to_upper_copy(str);
   if      (side.empty()) return e_noside;
   else if (side == "A" || side == "CPA") return e_cpa;
   else if (side == "B" || side == "CPB") return e_cpb;
   else throw Exception(Exception::illCpSide(side), WHERE__);
}

//----------------------------------------------------------------------------------------
// Get a log event
//----------------------------------------------------------------------------------------
void Api_impl::getLogEvent(
         const Time& cptime,
         const Time& aptime,
         size_t size,
         const char* buf,
         ostream&
         )
{
   s_eventcb(cptime.get(Time::e_long).c_str(), aptime.get().c_str(), size, buf);
}

}
