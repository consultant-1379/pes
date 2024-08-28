//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      tesrvtran.cpp
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
//      Console command tesrvtran.
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

#include <pes_clhapi.h>
#include <cmdparser.h>
#include <cpinfo.h>
#include <exception.h>
#include <common.h>
#include <ACS_APGCC_Util.H>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>

using namespace std;
using namespace PES_CLH;

//----------------------------------------------------------------------------------------
//	Usage
//----------------------------------------------------------------------------------------
void usage(bool multicp)
{
   const string& cpNameOption = multicp? "[-cp cp_name] ": "";
   cout << "Usage: tesrvtran -t {file | dvd} " << cpNameOption << endl
        << "                 [-a start_time][-e start_date]" << endl
        << "                 [-b stop_time][-f stop_date]" << endl;
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
		if (ok == false) 			   // Stand-by
		{
			throw Exception(Exception::illNodeState(), WHERE__);
		}

		// Check CP system, one-CP or multiple-CP
		multicp = CPTable::isMultiCPSystem();

      // Declare command options
		CmdParser::Optarg optCpName("cp");
		CmdParser::Optarg optStartTime("a");
		CmdParser::Optarg optStopTime("b");
		CmdParser::Optarg optStartDate("e");
		CmdParser::Optarg optStopDate("f");
		CmdParser::Optarg optTransType("t");

		// Parse command
		CmdParser cmdparser(argc, argv);

		list<string> namelist;

      cmdparser.fetchOpt(optCpName);
      cmdparser.fetchOpt(optStartTime);
      cmdparser.fetchOpt(optStopTime);
      cmdparser.fetchOpt(optStartDate);
      cmdparser.fetchOpt(optStopDate);
      cmdparser.fetchOpt(optTransType);

      // End of command check
      cmdparser.check();

      string destination;
      desttype_t desttype;
      if (optTransType.found())
      {
         destination = boost::to_upper_copy(optTransType.getArg());

         // Transfer type
         if (destination == "FILE")
         {
            desttype = e_file;
         }
         else if (destination != "DVD")
         {
            desttype = e_dvd;
         }
         else
         {
            throw Exception(Exception::illTransType(destination), WHERE__);
         }
      }
      else
      {
         throw Exception(Exception::usage(), WHERE__);
      }

		// Transfer log files
		Pes_clhapi clhapi;
		retCode = clhapi.transferLogs(
		      optCpName.getArg(),
            optStartTime.getArg(),
            optStopTime.getArg(),
            optStartDate.getArg(),
            optStopDate.getArg(),
            desttype
            );

		 if (retCode != 0)
		 {
		    cerr << clhapi.getErrorText() << endl;
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
