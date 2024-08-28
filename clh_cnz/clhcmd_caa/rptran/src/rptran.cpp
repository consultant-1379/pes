//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rptran.cpp
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
//      Console command for rptran.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424
//
//  AUTHOR
//      XDT/DEK/XVUNGUY
//
//  REVISION HISTORY
//      Rev.      Date         Prepared    Description
//      ----      ----         --------    -----------
//      R2A04     2013-03-06   XVUNGUY     CLH adapted to APG43 on Linux.
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
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <signal.h>
#include "rploglist.h"
namespace fs = boost::filesystem;
using namespace std;
using namespace PES_CLH;

//----------------------------------------------------------------------------------------
// Signal handler
//----------------------------------------------------------------------------------------
void sighandler(int)
{
}
//----------------------------------------------------------------------------------------
// Usage
//----------------------------------------------------------------------------------------
void usage()
{
    cout << "Usage: rptran -t {file | media} [-rp rpno]" << endl
            << "       [-a start_time][-e start_date][-b stop_time][-f stop_date]" << endl;
}
void doRptranCmd(const string& rpNum, const Time& staTime, const Time& stoTime, fs::path& desPath)
{
   fs::path rpFolder("RP/");
   const fs::path& rpPath = BaseParameters::getApzLogsPath() / rpFolder;
   //const string& rpPath("/data/apz/logs/RP/");
   RPLogList rplist;
   list<RPLogItem> rpLogItems;
   list<RPLogItem>::iterator it;
   bool fileExist(false);
   rplist.listRpLogs(rpNum, staTime, stoTime, false);
   rpLogItems = rplist.getRpLogList();
   it = rpLogItems.begin();
   while (it != rpLogItems.end())
   {
      fileExist = true;
      const string& filename = it->getFileName().c_str();
      //const string& srcStr = rpPath + filename;
      const fs::path& srcPath = rpPath / filename;
      //fs::path srcPath(srcStr.c_str());
      fs::copy_file(srcPath, desPath/filename);
      it++;
   }
   if (fileExist)
   {
      stringstream command;
      command << "zip -mr archive .";
      // Execute zip command
      ostringstream os;
      int error = Common::executeCommand(command.str(), os);
      if (error != 0)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Command execution failed for command '" << command.str() << "'." << endl;
         ex << os.str();
         throw ex;
      }
   }
}
//----------------------------------------------------------------------------------------
//	Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   const string& path = argv[0];
   size_t pos = path.find_last_of('/') + 1;
   const string& cmdname = path.substr(pos);

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
      if (ok == false)             // Stand-by
      {
         throw Exception(Exception::illNodeState(), WHERE__);
      }

      // Declare command options
      CmdParser::Optarg optTransType("t");
      CmdParser::Optarg optRPNo("rp");
      CmdParser::Optarg optStartTime("a");
      CmdParser::Optarg optStopTime("b");
      CmdParser::Optarg optStartDate("e");
      CmdParser::Optarg optStopDate("f");
      // Parse command
      CmdParser cmdparser(argc, argv);
      cmdparser.fetchOpt(optTransType);
      cmdparser.fetchOpt(optRPNo);
      cmdparser.fetchOpt(optStartTime);
      cmdparser.fetchOpt(optStopTime);
      cmdparser.fetchOpt(optStartDate);
      cmdparser.fetchOpt(optStopDate);
      // End of command check
      cmdparser.check();
      // Analyze parameters
      string destination;
      if (optTransType.found())
      {
         destination = boost::to_lower_copy(optTransType.getArg());
         // Transfer type
         if (destination != "file" && destination != "media")
         {
             throw Exception(Exception::illTransType(destination), WHERE__);
         }
      }
      else
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

      // Check that no other instance of this executable is running
      ok = Common::createLock(cmdname, true, destination);
      if (ok == false)
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
         s << "rp_files_" << Time::now();
         const string& archname = s.str();
         archpath = temppath / archname;
         fs::path archPathRef(archpath);
         fs::create_directories(archpath);
         // Change current working directory to archive path
         fs::current_path(archpath);
         // Execute command
         doRptranCmd(optRPNo.getArg(), period.first(), period.last(), archPathRef);
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
            else
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
         //fs::remove_all(temppath);
         throw;
      }
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
