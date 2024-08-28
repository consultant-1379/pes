//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      clhls.cpp
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
//      Console commands clhls and xpuls.
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
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <list>
#include <mausinfo.h>

namespace fs = boost::filesystem;

using namespace std;
using namespace PES_CLH;

enum t_cmdtype
{
   e_clhls,
   e_xpuls,
   e_tesrvls
};

t_cmdtype cmdtype(static_cast<t_cmdtype>(-1));         // Command type

//----------------------------------------------------------------------------------------
// Read events for each log type
//----------------------------------------------------------------------------------------
void readEventsForEachLogType(
         const BaseTask* logtaskp,
         const Period& period,
         const Filter& filter)
{
   try
   {
      // Print events
      logtaskp->readEvents(
                       period,
                       filter,
                       printLogEvent
                       );
   }
   catch (StartGreatStopTimeException& ex)
   {
      cout << "Warning: " << *logtaskp  << " First time (" << ex.getStartTime()
           << ") is greater than Last time (" << ex.getStopTime() << ")." << endl;
   }
}
//----------------------------------------------------------------------------------------
// Read events
//----------------------------------------------------------------------------------------
void readEvents(
         const CPInfo& cpinfo,
         t_cpSide cpside,
         const LogTable::LIST& loglist,
         const Period& period,
         const MAUSInfo& mausinfo
         )
{
   bool epopt = mausinfo.epExist();
   for (LogTable::LISTCITER iter = loglist.begin(); iter != loglist.end(); iter++)
   {
      const LogInfo& loginfo = *iter;
      const BaseParameters& parameters = loginfo.getParameters();
      const Filter& filter = loginfo.getXmFilter();
      t_logtype logtype = parameters.getLogType();
      t_loggroup loggroup = parameters.getLogGroup();

      if (loggroup != e_lgMAU)
      {
         if (!epopt)
         {
            for (int i = e_cpa; i <= e_cpb; i++)
            {
               if (cpside & i)
               {
                  t_cpSide tcpside = static_cast<t_cpSide>(i);
                  const BaseTask* logtaskp = createTask(logtype, cpinfo, tcpside);
                  readEventsForEachLogType(logtaskp, period, filter);
                  delete logtaskp;
               }
            }
         }
      }
      else
      {
         if (cpside == e_bothsides)
         {
            const fs::path& apzpath = BaseParameters::getApzLogsPath();
            MAUSInfo tmpmaus;
            int noep = parameters.getNoEndpoints();
            fs::path fullpath = apzpath;
            bool isLogTypeValid = true;           // Flag to check if the logs cannot be printed.
            // Append CP name if MCP
            fullpath /= cpinfo.getName();
            
            // Active MAUS logs
            if (noep == 1)
            {
               fullpath /= parameters.getPath();
               if (epopt)
               {
                  isLogTypeValid = false;
               }
            }
            // Endpoint MAUS logs
            else
            {
               if (epopt)
               {
                  string epName = "";
                  epName = MAUSInfo::getEPName(mausinfo.getIndex());
                  fullpath = fullpath / "mau/maus" / epName / parameters.getPath();
                  
                  if (epName == "apa" || epName == "apb")
                  {
                     if (logtype == e_msyscon)
                     {
                        isLogTypeValid = false;
                     }
                  }
               }
               else
               {
                  isLogTypeValid = false;
               }
            }
            
            if (isLogTypeValid)
            {
               tmpmaus.setPath(fullpath.string());
               const BaseTask* logtaskp = createTask(logtype, cpinfo, tmpmaus);
               readEventsForEachLogType(logtaskp, period, filter);
               delete logtaskp;
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------
// List time for first and last event
//----------------------------------------------------------------------------------------
void listEvents(
         const CPInfo& cpinfo,
         const LogTable::LIST& loglist,
         const Period& period,
         const MAUSInfo& mausinfo
         )
{
   bool epopt = mausinfo.epExist();
   const int tab1(16);
   const int tab2(10);

   cout.setf(ios::left, ios::adjustfield);
   cout << "Log type        CP side   First time        Last time" << endl;

   CPID cpid = cpinfo.getCPID();

   for (LogTable::LISTCITER iter = loglist.begin(); iter != loglist.end(); iter++)
   {
      const BaseParameters& parameters= iter->getParameters();
      t_logtype logtype = parameters.getLogType();
      t_loggroup loggroup = parameters.getLogGroup();

      if (loggroup != e_lgMAU)
      {
         if (!epopt)
         {
            // For all CP sides
            char maxside = (cpid < ACS_CS_API_HWC_NS::SysType_CP) ? e_cpa: e_cpb;
            for (int cpside = e_cpa; cpside <= maxside; cpside++)
            {
               t_cpSide tcpside = static_cast<t_cpSide>(cpside);
               const BaseTask* logtaskp = createTask(logtype, cpinfo, tcpside);
               stringstream ssPeriod;

               try
               {
                  Period tperiod = logtaskp->listEvents(period);
                  if (tperiod.empty() == false)
                  {
                     ssPeriod << tperiod.first() << "   " << tperiod.last();
                  }
               }
               catch (StartGreatStopTimeException& ex)
               {
                  ssPeriod << ex.getStartTime() << "   " << ex.getStopTime() << endl
                           << setw(tab1) << ' ' << "Warning: First time is greater than Last time.";
               }

               delete logtaskp;

               switch (tcpside)
               {
                  case e_cpa:
                  {
                     const string& commonname = parameters.getCommonName();
                     size_t size = commonname.size();
                     string extname = parameters.getLogName();
                     extname.erase(0, size);
                     string name = commonname;
                     if (extname.empty() == false)
                     {
                        name += "[" + extname + "]";
                     }
                     cout << setw(tab1) << name << setw(tab2) << "A" << ssPeriod.str() << endl;
                  }
                  break;

                  case e_cpb:
                  {
                     cout << setw(tab1) << "" << setw(tab2) << "B" << ssPeriod.str() << endl;
                  }
                  break;

                  default:
                     assert(!"Illegal CP side");
               }
            }
         }
      }
      else
      {
         const fs::path& apzpath = BaseParameters::getApzLogsPath();
         MAUSInfo activemaus;
         int noep = parameters.getNoEndpoints();
         fs::path fullpath = apzpath;
         bool flag = true;           // Flag to check if the logs cannot be printed.
         // Append CP name if MCP
         fullpath /= cpinfo.getName();
         
         // Active MAUS logs
         if (noep == 1)
         {
            fullpath /= parameters.getPath();
            if (epopt)
            {
               flag = false;
            }
         }
         // Endpoint MAUS logs
         else
         {
            if (epopt)
            {
               const string& epName = MAUSInfo::getEPName(mausinfo.getIndex());
               fullpath = fullpath / "mau/maus" / epName / parameters.getPath();
         
               if (epName == "apa" || epName == "apb")
               {
                  if (logtype == e_msyscon || logtype == e_mcore)
                  {
                     flag = false;
                  }
               }
            }
            else
            {
               flag = false;
            }
         }
         
         if (flag)
         {
            activemaus.setPath(fullpath.string());
            const BaseTask* logtaskp = createTask(logtype, cpinfo, activemaus);
            stringstream ssPeriod;
         
            try
            {
               Period tperiod = logtaskp->listEvents(period);
               if (tperiod.empty() == false)
               {
                  ssPeriod << tperiod.first() << "   " << tperiod.last();
               }
            }
            catch (StartGreatStopTimeException& ex)
            {
              ssPeriod << ex.getStartTime() << "   " << ex.getStopTime() << endl
                       << setw(tab1) << ' ' << "Warning: First time is greater than Last time.";
            }
         
            delete logtaskp;
         
            string commonname = parameters.getLogName();
            if (noep > 1)
            {
               commonname.erase(0, 4);    // Remove "MAUS" text
            }
            commonname += "[MAUS]";
            cout << setw(tab1) << commonname << setw(tab2) << " " << ssPeriod.str() << endl;
         }
      }
   }
   cout << endl;
   cout.unsetf(ios::left); // Restore format
}

//----------------------------------------------------------------------------------------
// Get CP side
//----------------------------------------------------------------------------------------
t_cpSide getCpSide(const string& str)
{
   const string& side = boost::to_upper_copy(str);
   if      (side == "A" || side == "CPA") return e_cpa;
   else if (side == "B" || side == "CPB") return e_cpb;
   else throw Exception(Exception::illCpSide(side), WHERE__);
}

//----------------------------------------------------------------------------------------
//   Usage
//----------------------------------------------------------------------------------------
void usage(t_cmdtype cmdtype, bool multicp, bool usemaus)
{
   const string& cpNameOption = multicp? "-cp cp_name ": "";
   const string& mausOption = usemaus? "[-m maus_endpoint]":"";

   switch (cmdtype)
   {
   case e_clhls:
      cout << "Usage: clhls " << cpNameOption
           << "[-s cp_side][-a start_time][-e start_date]" << endl;
      cout << "             [-b stop_time][-f stop_date]" << mausOption;
      cout << "[log ...]" << endl;
      cout << "       clhls -d " << cpNameOption
           << "[-e start_date][-f stop_date]" << mausOption << endl;
      break;

   case e_xpuls:
      cout << "Usage: xpuls " << cpNameOption
           << "[-s cp_side][-a start_time][-e start_date]" << endl;
      cout << "             [-b stop_time][-f stop_date]";
      cout << "[log[xmlist...]]" << endl;
      cout << "       xpuls -d " << cpNameOption
           << "[-e start_date][-f stop_date]" << endl;
      break;

   case e_tesrvls:
      cout << "Usage: tesrvls " << cpNameOption
           << "[-s cp_side][-a start_time][-e start_date]" << endl;
      cout << "               [-b stop_time][-f stop_date]" << endl;
      cout << "       tesrvls -d " << cpNameOption
           << "[-e start_date][-f stop_date]" << endl;
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
   t_cpSide cpside = e_noside;
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
      //usemaus = MAUSInfo::checkSwMauRunning();

      if      (cmdname == "clhls")    cmdtype = e_clhls;
      else if (cmdname == "xpuls")    cmdtype = e_xpuls;
      else if (cmdname == "tesrvls") cmdtype = e_tesrvls;
      else throw Exception(Exception::illCommand(), WHERE__);

      LogTable logtable;

      // Declare command options
      CmdParser::Optarg optCpName("cp");
      CmdParser::Optarg optCpSide("s");
      CmdParser::Optarg optStartTime("a");
      CmdParser::Optarg optStopTime("b");
      CmdParser::Optarg optStartDate("e");
      CmdParser::Optarg optStopDate("f");
      CmdParser::Opt dir("d");
      CmdParser::Optarg optMausEP("m");

      // Parse command
      CmdParser cmdparser(argc, argv);

      list<string> namelist;

      cmdparser.fetchOpt(optCpName);
      if (cmdtype == e_clhls)
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

      if (cmdparser.fetchOpt(dir) == false)
      {
         cmdparser.fetchOpt(optCpSide);
         cmdparser.fetchOpt(optStartTime);
         cmdparser.fetchOpt(optStopTime);
         cmdparser.fetchOpt(optStartDate);
         cmdparser.fetchOpt(optStopDate);
         if (usemaus)
         {
            cmdparser.fetchOpt(optMausEP);
         }

         // Log types
         string logname;
         switch (cmdtype)
         {
            case e_clhls:
               logtable.initialize(LogTable::s_error);
               logtable.initialize(LogTable::s_event);
               logtable.initialize(LogTable::s_syslog);
//             logtable.initialize(LogTable::s_binlog);      // Not printable?
//             logtable.initialize(LogTable::s_evlogcpsb);   //      -"-
//             logtable.initialize(LogTable::s_evlogpcih);   //        -"-
               logtable.initialize(LogTable::s_sel);
               logtable.initialize(LogTable::s_consolsrm);
               logtable.initialize(LogTable::s_consolbmc);
               logtable.initialize(LogTable::s_consolmp);
               logtable.initialize(LogTable::s_consolpcih);
               logtable.initialize(LogTable::s_consolsyscon);
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

            case e_xpuls:
               logtable.initialize(LogTable::s_xpulog);
               while (cmdparser.fetchPar(logname))
               {
                  namelist.push_back(boost::to_upper_copy(logname));
               }
               break;

            case e_tesrvls:
               logtable.initialize(LogTable::s_trace);
               namelist.push_back("TRACE");
               break;

            default:
               assert(!"Illegal command type");
         }
      }
      else
      {
         cmdparser.fetchOpt(optStartDate);
         cmdparser.fetchOpt(optStopDate);
         if (usemaus)
         {
            cmdparser.fetchOpt(optMausEP);
         }

         // Log types
         string logname;
         switch (cmdtype)
         {
         case e_clhls:
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
            break;

         case e_xpuls:
            logtable.initialize(LogTable::s_xpulog);
            logtable.initialize(LogTable::s_xpucore);
            break;

         case e_tesrvls:
            logtable.initialize(LogTable::s_trace);
            break;

         default:
            assert(!"Illegal command type");
         }
      }

      // End of command check
      cmdparser.check();

      // Analyze parameters
      if (multicp)
      {
         // Multi CP system
         if (optCpName.found() == false)
         {
            throw Exception(Exception::usage(), WHERE__);
         }

         // CP information
         cp = boost::to_upper_copy(optCpName.getArg());
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
            if (optCpSide.found())
            {
               throw Exception(Exception::cpSideNotAllowed(), WHERE__);
            }
            cpside = e_cpa;
            isBlade = true;
         }
         else
         {
            // Single CP system - check CP side
            if (optCpSide.found())
            {
               cpside = getCpSide(optCpSide.getArg());
            }
            else
            {
               cpside = e_bothsides;
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

         //   Check CP side
         if (optCpSide.found())
         {
            cpside = getCpSide(optCpSide.getArg());
         }
         else
         {
            cpside = e_bothsides;
         }
      }

      // Set time period
      Period period(
               optStartDate.getArg(),
               optStartTime.getArg(),
               optStopDate.getArg(),
               optStopTime.getArg()
               );

      t_apzSystem apzsys = cpinfo.getAPZSystem();
      if (apzsys == e_classic)
      {
         throw Exception(Exception::illCommand(), WHERE__);
      }

      if (apzsys == e_undefined)
      {
         throw Exception(Exception::apzSystemNotDefined(cp), WHERE__);
      }

      // Set up log for endpoint
      if (cmdtype == e_clhls)
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
               if (dir.found())
               {
                  logtable.initialize(LogTable::s_mcore);
               }

               // CP side is not allowed with -m all
               if (cpside != e_bothsides)
               {
                  // throw exception due to the side are not be used with MAUS log types.
                  throw Exception(Exception::cpSideNotAllowedToUseWithMAUS(), WHERE__);
               }
            }
            else
            {
               if (ep == "cpa" || ep == "cpb")
               {
                  // Initialize Syscon log
                  logtable.initialize(LogTable::s_msyscon);

                  if (dir.found())
                  {
                     logtable.initialize(LogTable::s_mcore);
                  }
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
                     if (logname == "MCORE" && dir.found())
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
          loglist = logtable.getList(apzsys);
      }
      else
      {
          loglist = logtable.getList(namelist, apzsys);
      }

      // Throw exceptions for MAUS logs
      if (cmdtype == e_clhls)
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

                  if (cpside != e_bothsides)
                  {
                     // throw exception due to the side are not be used with MAUS log types.
                     throw Exception(Exception::cpSideNotAllowedToUseWithMAUS(), WHERE__);
                  }

                  if (mausinfo.epExist() && noEPInMausLog <= 1)
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

      // Execute command
      cout << endl;
      if (dir.found())
      {
         if (mausinfo.checkValueAll())
         {
            for (int idx = 0; idx < MAUSInfo::EPMAX; idx++)
            {
                MAUSInfo minfo;
                const string& epName = MAUSInfo::getEPName(idx);
                minfo.find(epName);
                cout << boost::to_upper_copy(epName) << " ENDPOINT LOGS:" << endl;
                listEvents(cpinfo, loglist, period, minfo);
            }
         }
         else
         {
            // List first and last event
            listEvents(cpinfo, loglist, period, mausinfo);
         }
      }
      else
      {
         if (mausinfo.checkValueAll())
         {
            for (int idx = 0; idx < MAUSInfo::EPMAX; idx++)
            {
               MAUSInfo minfo;
               const string& epName = MAUSInfo::getEPName(idx);
               minfo.find(epName);
               readEvents(cpinfo, cpside, loglist, period, minfo);
            }
         }
         else
         {
             // Read events
             readEvents(cpinfo, cpside, loglist, period, mausinfo);
         }
      }
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
