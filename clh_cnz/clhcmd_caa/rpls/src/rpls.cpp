//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rpls.cpp
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
//      Console command rpls.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1425  PA1
//
//  AUTHOR
//      XDT/DEK/XVUNGUY
//
//  REVISION HISTORY
//      Rev.     Date         Prepared    Description
//      PA1      2012-10-16   XVUNGUY     CLH adapted to APG43 on Linux.
//      PA2      2013-08-21   XLOBUNG     Add comments.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>
#include <cmdparser.h>
#include <rptask.h>
#include <ltime.h>
#include <exception.h>
#include <common.h>
#include <ACS_APGCC_Util.H>
#include <string>
#include <iostream>
#include <memory>
#include "rploglist.h"
#include <boost/lexical_cast.hpp>
using namespace std;
using namespace PES_CLH;
//----------------------------------------------------------------------------------------
//	Usage
//----------------------------------------------------------------------------------------
void usage()
{
    cout << "Usage: rpls -d|-g [-rp rpno]" << endl
            << "       [-a start_time][-e start_date][-b stop_time][-f stop_date]" << endl;
}
void doRplsCmd(const string& rpNum, const Time& staTime, const Time& stoTime, bool isOptDir)
{
    RPLogList rplist;
    rplist.listRpLogs(rpNum, staTime, stoTime, isOptDir);
    rplist.print(isOptDir);
}
//----------------------------------------------------------------------------------------
//	Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{

    try
    {
        //Check if command is invoked on AP1
        if (Common::AP1 != Common::getApNode())
        {
            Exception ex(Exception::illApSide(), WHERE__);
            throw ex;

        }

        // Check CP system, one-CP or multiple-CP
        if (CPTable::isMultiCPSystem())
        {
            throw Exception(Exception::illCommand(), WHERE__);
        }
        else
        {
            CPInfo cpinfo;
            // Get APZ system
            t_apzSystem apzsys = cpinfo.getAPZSystem();
            if (apzsys == e_classic)
            {
                throw Exception(Exception::illCommand(), WHERE__);
            }
        }
        // Check that we are running on the active node
        bool ok = ACS_APGCC::is_active_node();
        if (ok == false) 			   // Stand-by
        {
            throw Exception(Exception::illNodeState(), WHERE__);
        }

        // Declare command options
        CmdParser::Opt dir("d");
        CmdParser::Opt lst("g");
        CmdParser::Optarg optRPNo("rp");
        CmdParser::Optarg optStartTime("a");
        CmdParser::Optarg optStopTime("b");
        CmdParser::Optarg optStartDate("e");
        CmdParser::Optarg optStopDate("f");
        // Parse command
        CmdParser cmdparser(argc, argv);
        cmdparser.fetchOpt(optRPNo);
        cmdparser.fetchOpt(optStartTime);
        cmdparser.fetchOpt(optStopTime);
        cmdparser.fetchOpt(optStartDate);
        cmdparser.fetchOpt(optStopDate);
        bool isOptDir = cmdparser.fetchOpt(dir);
        if (!cmdparser.fetchOpt(lst) && !isOptDir)
        {
            throw Exception(Exception::usage(), WHERE__);
        }
        // End of command check
        cmdparser.check();
        if (optRPNo.getArg() != "")
        {
            uint16_t rpno;
            try
            {
                rpno = static_cast<uint16_t>(boost::lexical_cast<uint16_t>(optRPNo.getArg()));
            }
            catch (exception&)
            {
                throw Exception(Exception::illRPno(rpno), WHERE__);
            }
            if (rpno > 1023)
            {
                throw Exception(Exception::illRPno(rpno), WHERE__);
            }
        }
        // Set time period
        Period period(
                optStartDate.getArg(),
                optStartTime.getArg(),
                optStopDate.getArg(),
                optStopTime.getArg()
        );
        // Print events
        doRplsCmd(optRPNo.getArg(), period.first(), period.last(), isOptDir);
        cout << endl;
    }
    catch (Exception& ex)
    {
        cerr << ex << endl;
        uint16_t retCode = ex.getErrCode();
        if (retCode == Exception::usage().first)
        {
            // If incorrect usage, print command format
            cerr << endl;
            usage();
        }
        cerr << endl;
        return retCode;
    }
    catch (exception& e)
    {
        // Boost exception
        Exception ex(Exception::system(), WHERE__);
        ex << e.what() << ".";
        cerr << ex << endl << endl;
        return ex.getErrCode();
    }
    return 0;
}
