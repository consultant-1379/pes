//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      seltran.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Console command seltran.
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
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <signal.h>

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
//	Usage
//----------------------------------------------------------------------------------------
void usage()
{
    cout << "Usage: seltran -t {file | media} [-r subrack -s slot]" << endl
            << "       [-a start_time][-e start_date][-b stop_time][-f stop_date]" << endl;
}

//----------------------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   uint16_t retCode(0);

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
      CmdParser::Optarg optTransType("t");

      // Parse command
      CmdParser cmdparser(argc, argv);

      cmdparser.fetchOpt(optSubRack);
      cmdparser.fetchOpt(optSlot);
      cmdparser.fetchOpt(optStartTime);
      cmdparser.fetchOpt(optStopTime);
      cmdparser.fetchOpt(optStartDate);
      cmdparser.fetchOpt(optStopDate);
      cmdparser.fetchOpt(optTransType);

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
      
      // Check node architecture
      Common::ArchitectureValue arch = Common::getNodeArchitecture();
      if (arch == Common::DMX)
      {
         Exception ex(Exception::notSupportInBSP(), WHERE__);
         throw ex;
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
         s << "sel_files_" << Time::now();
         string archname = s.str();
         archpath = temppath / archname;
         
         fs::create_directories(archpath);
         
         // Change current working directory to archive path
         fs::current_path(archpath);
         
         // Execute command
         try
         {
            SelTask seltaskap1;
            const string& fnap1 = seltaskap1.transferSELLogs(period, *filterp.get());

            SelTask seltaskap2;
            seltaskap2.setAP2();
            const string& fnap2 = seltaskap2.transferSELLogs(period, *filterp.get());

            if (fnap1 != "" && fnap2 != "")
            {
               const fs::path& ap1path = fnap1;
               const fs::path& ap2path = fnap2;
               Common::archive(ap1path, "archive");
               Common::archive(ap2path, "archive");
            }
            else
            {
               string strtmp = "";
               if (fnap1 != "")
               {
                  strtmp = fnap1;
               }
               else
               {
                  if (fnap2 != "")
                  {
                     strtmp = fnap2;
                  }
               }

               if (strtmp != "")
               {
                  const fs::path& appath = strtmp;
                  const fs::path& apparent = appath.parent_path();
                  const string& filename = appath.filename().c_str();
                  const string& newfilename = filename.substr(0, 36) + ".log";
                  const fs::path& newpath = apparent / newfilename;
                  fs::rename(appath, newpath);
                  Common::archive(newpath, "archive");
               }
            }
         }
         catch (StartGreatStopTimeException& ex)
         {
             cout << "Warning: " << "First time (" << ex.getStartTime()
                  << ") is greater than Last time (" << ex.getStopTime() << ")." << endl;
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
