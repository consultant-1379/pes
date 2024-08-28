//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      sells.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012, 2013. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Console command sells.
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
//      PA2    2013-08-01   XLOBUNG     CLH on AP2.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <cmdparser.h>
#include <seltask.h>
#include <boardfilter.h>
#include <ltime.h>
#include <exception.h>
#include <common.h>
#include <ACS_APGCC_Util.H>
#include <string>
#include <iostream>
#include <memory>

namespace fs = boost::filesystem;

using namespace std;
using namespace PES_CLH;

//----------------------------------------------------------------------------------------
//   Usage
//----------------------------------------------------------------------------------------
void usage()
{
   cout << "Usage: sells [-r subrack -s slot]" << endl
        << "       [-a start_time][-e start_date][-b stop_time][-f stop_date]" << endl;
}

//----------------------------------------------------------------------------------------
//   Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   uint16_t retCode(0);

   try
   {
      //Check if command is invoked on AP1
      if (Common::AP1 != Common::getApNode())
      {
          Exception ex(Exception::illApSide(), WHERE__);
          throw ex;

      }

      // Check that we are running on the active node
      bool ok = ACS_APGCC::is_active_node();
      if (ok == false)             // Stand-by
      {
         throw Exception(Exception::illNodeState(), WHERE__);
      }

      // Declare command options
      CmdParser::Optarg optSubRack("r");
      CmdParser::Optarg optSlot("s");
      CmdParser::Optarg optStartTime("a");
      CmdParser::Optarg optStopTime("b");
      CmdParser::Optarg optStartDate("e");
      CmdParser::Optarg optStopDate("f");

      // Parse command
      CmdParser cmdparser(argc, argv);

      cmdparser.fetchOpt(optSubRack);
      cmdparser.fetchOpt(optSlot);
      cmdparser.fetchOpt(optStartTime);
      cmdparser.fetchOpt(optStopTime);
      cmdparser.fetchOpt(optStartDate);
      cmdparser.fetchOpt(optStopDate);

      // End of command check
      cmdparser.check();

      // Analyze parameters
      auto_ptr<Filter> filterp(new BoardFilter());
      if (optSubRack.found() && optSlot.found())
      {
         filterp.reset(new BoardFilter(optSubRack.getArg(), optSlot.getArg()));
      }
      else if (optSubRack.found() || optSlot.found())
      {
         throw Exception(Exception::usage(), WHERE__);
      }

      // Set time period
      Period period(
               optStartDate.getArg(),
               optStartTime.getArg(),
               optStopDate.getArg(),
               optStopTime.getArg()
               );

      // Check CP system, one-CP or multiple-CP
      if (CPTable::isMultiCPSystem() == false)
      {
         CPInfo cpinfo;
         // Get APZ system
         t_apzSystem apzsys = cpinfo.getAPZSystem();
         if (apzsys == e_classic)
         {
            throw Exception(Exception::illCommand(), WHERE__);
         }
      }
      
      // Check node architecture
      Common::ArchitectureValue arch = Common::getNodeArchitecture();
      if (arch == Common::DMX)
      {
         Exception ex(Exception::notSupportInBSP(), WHERE__);
         throw ex;
      }

      // Print events
      cout << endl;
      SelTask seltaskap1;
      SelTask seltaskap2;
      seltaskap2.setAP2();
      // Check if SEL logs exist
      const Period& tperiodap1 = seltaskap1.readSELEvents(period, *filterp.get(), printLogEvent, std::cout, true);
      const Period& tperiodap2 = seltaskap2.readSELEvents(period, *filterp.get(), printLogEvent, std::cout, true);
      
      if (!tperiodap1.empty() && !tperiodap2.empty())
      {
          try
          {
              cout << "AP1:" << endl << endl;
              seltaskap1.readSELEvents(period, *filterp.get(), printLogEvent, std::cout, false);
              cout << "AP2:" << endl << endl;
              seltaskap2.readSELEvents(period, *filterp.get(), printLogEvent, std::cout, false);
          }
          catch (StartGreatStopTimeException& ex)
          {
              cout << "Warning: " << "First time (" << ex.getStartTime()
                   << ") is greater than Last time (" << ex.getStopTime() << ")." << endl;
          }
      }
      else
      {
         if (!tperiodap1.empty())
         {
             try
             {
                seltaskap1.readSELEvents(period, *filterp.get(), printLogEvent, std::cout, false);
             }
             catch (StartGreatStopTimeException& ex)
             {
                cout << "Warning: " << "First time (" << ex.getStartTime()
                     << ") is greater than Last time (" << ex.getStopTime() << ")." << endl;
             }
         }
         else
         {
            if (!tperiodap2.empty())
            {
                try
                {
                   seltaskap2.readSELEvents(period, *filterp.get(), printLogEvent, std::cout, false);
                }
                catch (StartGreatStopTimeException& ex)
                {
                   cout << "Warning: " << "First time (" << ex.getStartTime()
                        << ") is greater than Last time (" << ex.getStopTime() << ")." << endl;
                }
            }
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
         usage();
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
