//----------------------------------------------------------------------------------------
//
//  FILE
//      clhtran.cpp
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
//      Console commands clhtran and xputran.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1425  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2012-11-27   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <cmdparser.h>
#include <cpinfo.h>
#include <loginfo.h>
#include <logtask.h>
#include <ltime.h>
#include <exception.h>
#include <common.h>
#include <ACS_APGCC_Util.H>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <list>
#include <signal.h>
#include <mausinfo.h>

namespace fs = boost::filesystem;

using namespace std;
using namespace PES_CLH;

enum t_cmdtype
{
   e_clhtran,
   e_xputran,
   e_tesrvtran
};

t_cmdtype cmdtype(static_cast<t_cmdtype>(-1));         // Command type

//----------------------------------------------------------------------------------------
// Signal handler
//----------------------------------------------------------------------------------------
void sighandler(int)
{
}

//----------------------------------------------------------------------------------------
// Read events for each log type
//----------------------------------------------------------------------------------------
void transferForEachLogType(
         const BaseTask* logtaskp,
         const Period& period,
         const Filter& filter)
{
   try
   {
      logtaskp->transferLogs(period, filter);
   }
   catch (StartGreatStopTimeException& ex)
   {
      cout << "Warning: " << *logtaskp  << " First time (" << ex.getStartTime()
           << ") is greater than Last time (" << ex.getStopTime() << ")." << endl;
   }
}

//----------------------------------------------------------------------------------------
// Transfer log files for a CP or blade
//----------------------------------------------------------------------------------------
void transferLogs(
      const CPInfo& cpinfo,
      const LogTable::LIST& loglist,
      const Period& period,
      const MAUSInfo& mausinfo
      )
{
   bool epopt = mausinfo.epExist();
   t_apzSystem apzsys = cpinfo.getAPZSystem();
   const string& cpname = cpinfo.getName();
   if (apzsys == e_undefined)
   {
      cout << "APZ system value is not defined for " << cpname.c_str() << ".";
      return;
   }
   CPID cpid = cpinfo.getCPID();

   // Iterate all CP sides
   char maxside = (cpid < ACS_CS_API_HWC_NS::SysType_CP) ? e_cpa: e_cpb;
   for (int cpside = e_cpa; cpside <= maxside; cpside++)
   {
      t_cpSide tcpside = static_cast<t_cpSide>(cpside);

      // Iterate all log types
      for (LogTable::LISTCITER iter = loglist.begin(); iter != loglist.end(); iter++)
      {
         const LogInfo& loginfo = *iter;
         const BaseParameters& parameters = loginfo.getParameters();
         if (parameters.isValidApz(apzsys))
         {
            t_logtype logtype = parameters.getLogType();
            t_loggroup loggroup = parameters.getLogGroup();
            if (loggroup != e_lgMAU)
            {
               if (!epopt)
               {
                  const BaseTask* logtaskp = createTask(logtype, cpinfo, tcpside);
                  const Filter& filter = iter->getXmFilter();
                  transferForEachLogType(logtaskp, period, filter);
                  delete logtaskp;
               }
            }
            else
            {
               const fs::path& apzpath = BaseParameters::getApzLogsPath();
               bool cont = true;
               int noeploop = 0;
               while (cont)
               {
                  MAUSInfo tmpmaus;
                  int noep = parameters.getNoEndpoints();
                  fs::path fullpath = apzpath;
                  // Append CP name if MCP
                  fullpath /= cpinfo.getName();
                  fs::path parentpath = "";
                  bool isLogTypeValid = true;           // Flag to check if the logs cannot be printed.
                  // Active MAUS logs
                  if (noep == 1)
                  {
                     fullpath /= parameters.getPath();
                     parentpath /= "mau";
                     if (epopt)
                     {
                        isLogTypeValid = false;
                     }
                     cont = false;
                  }
                  // Endpoint MAUS logs
                  else
                  {
                     if (epopt)
                     {
                        string epName = "";
                        // Loop all endpoints
                        fs::path eppath;
                        if (mausinfo.checkValueAll())
                        {
                           eppath = "mau/maus/" + MAUSInfo::getEPName(noeploop);
                           noeploop ++;
                     
                           if (noeploop == MAUSInfo::EPMAX)
                           {
                              cont = false;
                           }
                        }
                        else
                        {
                           eppath = "mau/maus/" + MAUSInfo::getEPName(mausinfo.getIndex());
                           cont = false;
                        }

                        parentpath /= eppath;
                        fullpath /= eppath / parameters.getPath();
                  
                        if (epName == "apa" || epName == "apb")
                        {
                           if (logtype == e_msyscon || logtype == e_mcore)
                           {
                              isLogTypeValid = false;
                           }
                        }
                     }
                     else
                     {
                        isLogTypeValid = false;
                        cont = false;
                     }
                  }
                  
                  if (isLogTypeValid)
                  {
                     tmpmaus.setParentPath(parentpath.string());
                     tmpmaus.setPath(fullpath.string());
                     const BaseTask* logtaskp = createTask(logtype, cpinfo, tmpmaus);
                     const Filter& filter = iter->getXmFilter();
                     transferForEachLogType(logtaskp, period, filter);
                     delete logtaskp;
                  }
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------
// One CP system:   Transfer log files
// Multi CP system: Transfer log files for all CP:s
//----------------------------------------------------------------------------------------
void transferLogs(
      const LogTable::LIST& loglist,
      const Period& period,
      const MAUSInfo& mausinfo
      )
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
         transferLogs(*iter, loglist, period, mausinfo);
      }
   }
   else
   {
      // One CP system
      transferLogs(cptable.get(), loglist, period, mausinfo);
   }
}

//----------------------------------------------------------------------------------------
//   Usage
//----------------------------------------------------------------------------------------
void usage(t_cmdtype cmdtype, bool multicp, bool usemaus)
{
   const string& cpNameOption = multicp? "[-cp cp_name] ": "";
   const string& mausOption = usemaus? "[-m maus_endpoint]":"";

   switch (cmdtype)
   {
   case e_clhtran:
      cout << "Usage: clhtran -t transfertype " << cpNameOption << endl
           << "               [-a start_time][-e start_date]" << endl
           << "               [-b stop_time][-f stop_date]" << mausOption
           << "[log...]" << endl;
      break;

   case e_xputran:
      cout << "Usage: xputran -t transfertype " << cpNameOption << endl
           << "               [-a start_time][-e start_date]" << endl
           << "               [-b stop_time][-f stop_date][log...]" << endl;
      break;

   case e_tesrvtran:
      cout << "Usage: tesrvtran -t transfertype " << cpNameOption << endl
           << "                 [-a start_time][-e start_date]" << endl
           << "                 [-b stop_time][-f stop_date]" << endl;
      break;

   default:
      assert("!Illegal command type");
   }
}

//----------------------------------------------------------------------------------------
//   Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   const string& path = argv[0];
   size_t pos = path.find_last_of('/') + 1;
   const string& cmdname = path.substr(pos);
   uint16_t retCode(0);
   bool multicp(false);
   bool usemaus(false);
   string cp;
   CPTable cptable;
   CPInfo cpinfo;
   t_apzSystem apzsys = e_undefined;
   bool isBlade = false;
   MAUSInfo mausinfo;
   t_mauType mauType = e_mauundefined;

   try
   {
      //Check if command is invoked on AP1
      if (Common::AP1 != Common::getApNode())
      {
         Exception ex(Exception::illApSide(), WHERE__);
         throw ex;
      }

       // Check that we are running on the active node
      bool result = ACS_APGCC::is_active_node();
      if (result == false)             // Stand-by
      {
         throw Exception(Exception::illNodeState(), WHERE__);
      }

      // Check CP system, one-CP or multiple-CP
      multicp = CPTable::isMultiCPSystem();

      // Check if MAUS is running
      usemaus = MAUSInfo::checkSwMauRunning();

      if      (cmdname == "clhtran")   cmdtype = e_clhtran;
      else if (cmdname == "xputran")   cmdtype = e_xputran;
      else if (cmdname == "tesrvtran") cmdtype = e_tesrvtran;
      else throw Exception(Exception::illCommand(), WHERE__);

      LogTable logtable;

      // Declare command options
      CmdParser::Optarg optCpName("cp");
      CmdParser::Optarg optStartTime("a");
      CmdParser::Optarg optStopTime("b");
      CmdParser::Optarg optStartDate("e");
      CmdParser::Optarg optStopDate("f");
      CmdParser::Optarg optTransType("t");
      CmdParser::Optarg optMausEP("m");

      // Parse command
      CmdParser cmdparser(argc, argv);

      list<string> namelist;

      cmdparser.fetchOpt(optCpName);
      if (cmdtype == e_clhtran)
      {
         // Check MAU Type associated with a CP
         if (multicp)
         {
            // Multi CP system
            if (optCpName.found())
            {
               // CP information
               cp = boost::to_upper_copy(optCpName.getArg());
               CPTable::const_iterator iter = cptable.find(cp);
               if (iter != cptable.end())
               {
                  CPInfo tmp_cpinfo = *iter;
                  CPID cpid = cpinfo.getCPID();
                  // CP or BC?
                  if (cpid >= ACS_CS_API_HWC_NS::SysType_CP)
                  {
                     mauType = tmp_cpinfo.getMAUType();
                  }
               }
            }
         }
         else
         {
            CPInfo tmp_cpinfo = cptable.get();
            mauType = tmp_cpinfo.getMAUType();
         }

         if (mauType == e_maus)
         {
            usemaus = true;
         }
      }

      cmdparser.fetchOpt(optStartTime);
      cmdparser.fetchOpt(optStopTime);
      cmdparser.fetchOpt(optStartDate);
      cmdparser.fetchOpt(optStopDate);
      cmdparser.fetchOpt(optTransType);
      if (usemaus)
      {
         cmdparser.fetchOpt(optMausEP);
      }

      // Log types
      string logname;
      switch (cmdtype)
      {
         case e_clhtran:
            logtable.initialize(LogTable::s_error);
            logtable.initialize(LogTable::s_event);
            logtable.initialize(LogTable::s_syslog);
            logtable.initialize(LogTable::s_binlog);
            logtable.initialize(LogTable::s_corecpbb);
            logtable.initialize(LogTable::s_corecpsb);
            logtable.initialize(LogTable::s_corepcih);
            logtable.initialize(LogTable::s_corecpub);
            logtable.initialize(LogTable::s_crashcpbb);
            logtable.initialize(LogTable::s_crashcpsb);
            logtable.initialize(LogTable::s_crashpcih);
            logtable.initialize(LogTable::s_crashcpub);
            logtable.initialize(LogTable::s_evlogcpsb);
            logtable.initialize(LogTable::s_evlogpcih);
            logtable.initialize(LogTable::s_salinfocpsb);
            logtable.initialize(LogTable::s_sel);
            logtable.initialize(LogTable::s_consolsrm);
            logtable.initialize(LogTable::s_consolbmc);
            logtable.initialize(LogTable::s_consolmp);
            logtable.initialize(LogTable::s_consolpcih);
            logtable.initialize(LogTable::s_consolsyscon);
            logtable.initialize(LogTable::s_xpulog);
            logtable.initialize(LogTable::s_xpucore);
            if (usemaus)
            {
               logtable.initialize(LogTable::s_mphca);
               logtable.initialize(LogTable::s_mphcb);
               logtable.initialize(LogTable::s_mwsr);
               logtable.initialize(LogTable::s_mehl);
               logtable.initialize(LogTable::s_mcpflagsa);
               logtable.initialize(LogTable::s_mcpflagsb);
               logtable.initialize(LogTable::s_minfr);
               logtable.initialize(LogTable::s_mintfstsa);
               logtable.initialize(LogTable::s_mintfstsb);
               logtable.initialize(LogTable::s_mevent);
            }
            while (cmdparser.fetchPar(logname))
            {
               namelist.push_back(boost::to_upper_copy(logname));
            }
            break;

         case e_xputran:
            logtable.initialize(LogTable::s_xpulog);
            logtable.initialize(LogTable::s_xpucore);
            while (cmdparser.fetchPar(logname))
            {
               namelist.push_back(boost::to_upper_copy(logname));
            }
            break;

         case e_tesrvtran:
            logtable.initialize(LogTable::s_trace);
            namelist.push_back("TRACE");
            break;

         default:
            assert(!"Illegal command type.");
      }

      // End of command check
      cmdparser.check();

      string destination;
      if (optTransType.found())
      {
         destination = boost::to_lower_copy(optTransType.getArg());

         // Transfer type
         if (destination != "file" && destination != "media")
         {
            throw Exception(Exception::illTransType(destination), WHERE__);
         }
         else if ((destination == "media") && (Common::getNodeArchitecture()== Common::VIRTUALIZED))
         {
            throw Exception(Exception::illTransType(destination), WHERE__);
         }
      }
      else
      {
         throw Exception(Exception::usage(), WHERE__);
      }

      // Analyze parameters
      if (multicp)
      {
         // Multi CP system
         if (optCpName.found())
         {
            // CP information
            cp = boost::to_upper_copy(optCpName.getArg());
            CPTable::const_iterator iter = cptable.find(cp);
            if (iter == cptable.end())
            {
               throw Exception(Exception::cpNotDefined(cp), WHERE__);
            }

            cpinfo = *iter;
            apzsys = cpinfo.getAPZSystem();

            if (apzsys == e_undefined)
            {
               throw Exception(Exception::apzSystemNotDefined(cp), WHERE__);
            }

            CPID cpid = cpinfo.getCPID();

            // CP or BC?
            if (cpid < ACS_CS_API_HWC_NS::SysType_CP)
            {
               isBlade = true;
            }
         }
      }
      else
      {
         if (optCpName.found())
         {
            throw Exception(Exception::illOption("-" + optCpName.getOpt()), WHERE__);
         }

         cpinfo = cptable.get();
         apzsys = cpinfo.getAPZSystem();

         if (apzsys == e_undefined)
         {
            throw Exception(Exception::apzSystemNotDefined("CP"), WHERE__);
         }
      }

      // Set time period
      Period period(
               optStartDate.getArg(),
               optStartTime.getArg(),
               optStopDate.getArg(),
               optStopTime.getArg()
               );

      if (apzsys == e_classic)
      {
         throw Exception(Exception::illCommand(), WHERE__);
      }

      // Set up log for endpoint
      if (cmdtype == e_clhtran)
      {
         // If an endpoint is specified
         if (optMausEP.found())
         {
            string ep = boost::to_lower_copy(optMausEP.getArg());
            
            if (!mausinfo.find(ep))
            {
               // throw exception since the ep can not be found in the table.
               throw Exception(Exception::illEndpoint(boost::to_upper_copy(ep)), WHERE__);
            }

            if (isBlade)
            {
               // throw exception since there is no MAUS log for blade
               throw Exception(Exception::illToUseWithSingleSidedCP(), WHERE__);
            }

            if (mausinfo.checkValueAll())
            {
                logtable.initialize(LogTable::s_msyscon);
                logtable.initialize(LogTable::s_mcore);
            }
            else
            {
               if (ep == "cpa" || ep == "cpb")
               {
                  logtable.initialize(LogTable::s_msyscon);
                  logtable.initialize(LogTable::s_mcore);
               }
            }
         }
         else
         {
            if (!namelist.empty())
            {
               std::list<std::string>::const_iterator iter;
               for (iter = namelist.begin(); iter != namelist.end(); ++iter)
               {
                  const string& logname = *iter;

                  if (logname == "MSYSCON")
                  {
                     throw Exception(Exception::mausEPMissing(logname), WHERE__);
                  }
                  else
                  {
                     if (logname == "MCORE")
                     {
                        throw Exception(Exception::mausEPMissing(logname), WHERE__);
                     }
                  }
               }
            }
         }
      }

      // Analyze list of logs
      LogTable::LIST loglist;
      if (namelist.empty())
      {
          loglist = logtable.getList();
      }
      else
      {
          loglist = logtable.getList(namelist, apzsys);
      }

      // Throw exception for MAUS logs
      if (cmdtype == e_clhtran)
      {
         for (LogTable::LISTCITER iter = loglist.begin(); iter != loglist.end(); iter++)
         {
            const LogInfo& loginfo = *iter;
            const BaseParameters& parameters = loginfo.getParameters();
            t_loggroup loggroup = parameters.getLogGroup();
            int noEPInMausLog = parameters.getNoEndpoints();

            if (loggroup == e_lgMAU)
            {
               if (!namelist.empty())
               {
                  if (isBlade)
                  {
                     // throw exception since there is no MAUS log for blade
                     throw Exception(Exception::illToUseWithSingleSidedCP(), WHERE__);
                  }
               
                  if (mausinfo.epExist() && noEPInMausLog == 1)
                  {
                     // Throw exception since active log can not be used with an endpoint
                     const string& logname = parameters.getCommonName();
                     throw Exception(Exception::illLogTypeForMAUSEP(logname), WHERE__);
                  }

                  if (!mausinfo.epExist() && noEPInMausLog > 1)
                  {
                     // Throw exception since endpoint log can not be used without an endpoint
                     const string& logname = parameters.getCommonName();
                     throw Exception(Exception::mausEPMissing(logname), WHERE__);
                  }
               }
            }
            else
            {
               if (!namelist.empty())
               {
                  if (mausinfo.epExist() && noEPInMausLog == 0)
                  {
                     // Throw exception since active log can not be used with an endpoint
                     const string& logname = parameters.getCommonName();
                     throw Exception(Exception::illLogTypeForMAUSEP(logname), WHERE__);
                  }
               }
            }
         }
      }

      // Check that no other instance of this executable is running
      result = Common::createLock(cmdname, true, destination);
      if (result == false)
      {
         if (destination == "file")
         {
            Exception ex(Exception::cmdBusyToTransferToFile(), WHERE__);
            throw ex;
         }
         else
         {
            Exception ex(Exception::cmdBusyToTransferToMedia(), WHERE__);
            throw ex;
         }
      }

      // Check if emf is busy with transferring another file
      if (destination == "media")
      {
         if (Common::isEmfBusy())
         {
            Exception ex(Exception::cmdBusyToTransferToMedia(), WHERE__);
            throw ex;
         }
      }

      // Command execution
      fs::path supportpath;
      fs::path temppath;
      fs::path archpath;

      // Save current working directory
      fs::path curdir = fs::current_path();

      try
      {
         // Prepare temporary archive
         supportpath = Common::getFileMPath("supportData");
         //temppath = supportpath / "temp";
         const string& clhtmp = "clh_" + destination;
         temppath = supportpath / clhtmp;

         ostringstream s;
         s << "clh_files_" << Time::now();
         string archname = s.str();
         archpath = temppath / archname;

         fs::create_directories(archpath);

         // Change current working directory to archive path
         fs::current_path(archpath);

         // Execute command

         if (optCpName.found())
         {
            // Transfer files for specific CP or blade
            transferLogs(cpinfo, loglist, period, mausinfo);
         }
         else
         {
            // In a multi CP system: Transfer log files for all CP:s and blades
            // In a single CP system: Transfer log files
            transferLogs(loglist, period, mausinfo);
         }

         const fs::path& archive = archpath / "archive.zip";

         if (fs::exists(archive))
         {
            const string& archfile = archname + ".zip";
            if (destination == "file")
            {
               // Move archive to ftp volume
               fs::rename(archive, supportpath / archfile);

                 // Transfer finished, remove temporary files
               fs::remove_all(temppath);
            }
            else if (destination == "media")
            {
               // Copy archive to DVD
               const fs::path& tarchive = archpath / archfile;
               fs::rename(archive, tarchive);
               fs::path nbipath = Common::getExtNbiPath(tarchive);
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
            }
            else
            {
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
      catch (...)
      {
         // Cleanup after error

         // Restore current working directory
         fs::current_path(curdir);

         // Remove temporary archive
         fs::remove_all(temppath);

         throw;
      }

      cout << endl;
   }
   catch (Exception& ex)
   {
      cerr << ex << endl;
      retCode = ex.getErrCode();
      if (retCode == Exception::usage().first)
      {
         // If incorrect usage, print command format
         cerr << endl;
         usage(cmdtype, multicp, usemaus);
      }
      cerr << endl;
   }
   catch (exception& e)
   {
      // Boost exception
      Exception ex(Exception::system(), WHERE__);
      ex << e.what() << ".";
      cerr << ex << endl << endl;
      retCode = ex.getErrCode();
   }

   return retCode;
}
