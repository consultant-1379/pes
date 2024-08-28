//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      tesrvls.cpp
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
//      Console command tesrvls.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 ???  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-01-28   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "pes_clhapi.h"
#include <cmdparser.h>
#include <cpinfo.h>
#include <exception.h>
#include <common.h>
#include <ACS_APGCC_Util.H>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace PES_CLH;

//----------------------------------------------------------------------------------------
// Read events
//----------------------------------------------------------------------------------------
void printEvent(const char* cptime, const char* aptime, size_t size, const char* evmsg)
{
   // Print a log event to the console
   cout << cptime << "  AP time: " << aptime << endl;
   cout.write(evmsg, size);
   cout << endl;
}

//----------------------------------------------------------------------------------------
//	Usage
//----------------------------------------------------------------------------------------
void usage(bool multicp)
{
   const string& cpNameOption = multicp? "-cp cp_name ": "";
   cout << "Usage: tesrvls " << cpNameOption
        << "[-s cp_side][-a start_time][-e start_date]" << endl;
   cout << "               [-b stop_time][-f stop_date]" << endl;
   cout << "       tesrvls -d " << cpNameOption
        << "[-e start_date][-f stop_date]" << endl;
}

//----------------------------------------------------------------------------------------
//	Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   uint16_t retCode(0);
   bool multicp(false);
   try
   {
      // Check that we are running on the active node
      bool ok = ACS_APGCC::is_active_node();
      if (ok == false) // Stand-by
      {
         throw Exception(Exception::illNodeState(), WHERE__);
      }

      // Check CP system, one-CP or multiple-CP
      multicp = CPTable::isMultiCPSystem();

      // Declare command options
      CmdParser::Optarg optCpName("cp");
      CmdParser::Optarg optCpSide("s");
      CmdParser::Optarg optStartTime("a");
      CmdParser::Optarg optStopTime("b");
      CmdParser::Optarg optStartDate("e");
      CmdParser::Optarg optStopDate("f");
      CmdParser::Opt optDir("d");

      // Parse command
      CmdParser cmdparser(argc, argv);

      list<string> namelist;

      if (cmdparser.fetchOpt(optDir) == false)
      {
         cmdparser.fetchOpt(optCpName);
         cmdparser.fetchOpt(optCpSide);
         cmdparser.fetchOpt(optStartTime);
         cmdparser.fetchOpt(optStopTime);
         cmdparser.fetchOpt(optStartDate);
         cmdparser.fetchOpt(optStopDate);
      }
      else
      {
         cmdparser.fetchOpt(optCpName);
         cmdparser.fetchOpt(optStartDate);
         cmdparser.fetchOpt(optStopDate);
      }

      // End of command check
      cmdparser.check();

      Pes_clhapi::t_eventcb eventcb = 0;
      if (optDir.found() == false)
      {
         eventcb = printEvent;
      }

      string startDate = optStartDate.getArg();
      string startTime = optStartTime.getArg();
      string stopDate = optStopDate.getArg();
      string stopTime = optStopTime.getArg();

      // Excecue command
      Pes_clhapi clhapi;
      int retCode = clhapi.readLog(optCpName.getArg(), // Name of CP or blade
            optCpSide.getArg(),     // CP side
            startDate,              // Start date “YYYYMMDD”
            startTime,              // Start time “HHmm”
            stopDate,               // Stop date “YYYYMMDD”
            stopTime,               // Stop time “HHmm”
            eventcb                 // Callback function
      );
      if (retCode != 0)
      {
         cerr << clhapi.getErrorText() << endl;
         return retCode;
      }
      if (optDir.found() == true)
      {
         // Print first and last event
         cout << setfill(' ');
         cout << std::left;

         cout << setw(9) << optCpSide.getArg() << setw(8) << startDate << "_" << setw(8)
              << startTime << setw(8) << stopDate << "_" << setw(8) << stopTime
              << endl;
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
         usage(multicp);
      }
      cerr << endl;
   }

   return retCode;
}
