//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      common.cpp
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
//      Class for common routines.
//      
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2012-11-27   UABTSO      CLH adapted to APG43 on Linux.
//      R3A    2013-06-19   XVUNGUY     Add function to get AP node name
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "common.h"
#include "cpinfo.h"
#include "exception.h"
#include "logger.h"
#include <acs_apgcc_paramhandling.h>
#include <ACS_APGCC_CommonLib.h>
#include <ACS_DSD_Client.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/mount.h>

using namespace std;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Get software version
//----------------------------------------------------------------------------------------
string Common::getVersion(const string& swunit)
{
   ostringstream s;
   s << "Central Log Handler (CLH), " << swunit << "." << endl;
   s << "(c) Copyright Ericsson AB 2012. All rights reserved." << endl;
   s << "Software version is ";
   ostringstream t;
   int result = Common::executeCommand("rpm -q PES_CLHBIN", t);   // Software version query
   s << (result? "unknown\n": t.str());
   s << "Build date is " __DATE__ << "  " << __TIME__ << ".";
   return s.str();
}

//----------------------------------------------------------------------------------------
// Create a lock file
//----------------------------------------------------------------------------------------
bool Common::createLock(const string& cmdname, bool isCmd, const string& opt)
{
   bool found(false);

   // Get pid file
   fs::path pidpath = (isCmd == true)? ("/var/run"):("/var/run/apg");

   if  (isCmd)
   {
      pidpath /= cmdname + "." + opt + ".pid";
   }
   else
   {
      pidpath /= cmdname + ".pid";
   }
   if (fs::exists(pidpath))
   {
      // pid-file exists - read pid
      string pid;
      fs::ifstream pidfs(pidpath);
      getline(pidfs, pid, '\0');

      // Check if process exists in the proc filesystem
      fs::path path = "/proc";
      path /= pid;
      path /= "cmdline";
      if (fs::exists(path))
      {
         // Process exists
         string cmdline;
         fs::ifstream pathfs(path);
         if (pathfs.is_open() == false)
         {
            Exception ex(Exception::internal(), WHERE__);
            ex << "Failed to open file " << path << ".";
            throw ex;
         }
         getline(pathfs, cmdline, '\0');
         const string& tcmdname = fs::path(cmdline).filename().c_str();
         found = (tcmdname == cmdname); // Is it same command name (executable)?
      }
   }

   if (found)
   {
      return false;
   }
   else
   {
      // Create pid-file
      int pid = getpid();
      fs::ofstream pidfs(pidpath, ios::trunc);
      if (pidfs.is_open() == false)
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << "Failed to open pid file.";
         throw ex;
      }
      pidfs << pid;
      return true;
   }
}

//----------------------------------------------------------------------------------------
// Execute a console command
//----------------------------------------------------------------------------------------
int Common::executeCommand(const string& command, ostream& os)
{
   char cbuf[512];

   FILE* fp = popen((command + " 2>&1").c_str(), "r");
   if (fp == 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to launch command '" << command << "'.";
      ex.sysError();
      throw ex;
   }

   bool interrupt(true);
   while (fgets(cbuf, 511, fp) != NULL)
   {
      interrupt = false;
      os << cbuf;
   }
   if (interrupt == true)
   {
      os << "Command interrupted by operator.";
   }

   int pstat = pclose(fp);

   return WIFEXITED(pstat)? WEXITSTATUS(pstat): -1;
}

//----------------------------------------------------------------------------------------
// Insert file in archive
//----------------------------------------------------------------------------------------
void Common::archive(const fs::path& source, const fs::path& dest)
{
   stringstream command;

   command << "zip -mr " << "\"" << dest << "\" \"" << source << "\"";

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

//----------------------------------------------------------------------------------------
//   Get data disk path path
//----------------------------------------------------------------------------------------
fs::path Common::getDataDiskPath(const string& logicalName)
{
   const int size = 64;
   char path[size] = "";
   int len = size - 1;

   ACS_APGCC_CommonLib commonlib;
   ACS_APGCC_DNFPath_ReturnTypeT ret;
   ret = commonlib.GetDataDiskPath(logicalName.c_str(), path, len);
   if (ret != ACS_APGCC_DNFPATH_SUCCESS)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get data disk path for logical name '"
         << logicalName << "'." << endl;
      switch (ret)
      {
         case ACS_APGCC_STRING_BUFFER_SMALL:
            ex << "Buffer too small.";
            break;

         case ACS_APGCC_FAULT_LOGICAL_NAME:
            ex << "Illegal logical name.";
            break;

         case ACS_APGCC_DNFPATH_FAILURE:
            ex << "Failed to get DNF path.";
            break;

         default:
            ex << "Unexpected return code.";
      }
      throw ex;
   }

   return path;
}

//----------------------------------------------------------------------------------------
// Get file management path
//----------------------------------------------------------------------------------------
fs::path Common::getFileMPath(const string& logicalName)
{
   const int size = 64;
   char path[size] = "";
   int len = size - 1;

   ACS_APGCC_CommonLib commonlib;
   ACS_APGCC_DNFPath_ReturnTypeT ret;
   ret = commonlib.GetFileMPath(logicalName.c_str(), path, len);
   if (ret != ACS_APGCC_DNFPATH_SUCCESS)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get file management path for logical name '"
         << logicalName << "'." << endl;
      switch (ret)
      {
         case ACS_APGCC_STRING_BUFFER_SMALL:
            ex << "Buffer too small.";
            break;

         case ACS_APGCC_FAULT_LOGICAL_NAME:
            ex << "Illegal logical name.";
            break;

         case ACS_APGCC_DNFPATH_FAILURE:
            ex << "Failed to get DNF path.";
            break;

         default:
            ex << "Unexpected return code.";
      }
      throw ex;
   }

   return path;
}

//----------------------------------------------------------------------------------------
// Create a symbolic link
//----------------------------------------------------------------------------------------
void Common::createSymLink(const fs::path& path)
{
   const fs::path& subdir = path.filename();
   string subname = subdir.c_str();
   subname[0] = toupper(subname[0]);
   const fs::path& symlink = path.parent_path() / subname;
   if (fs::exists(symlink) == false)
   {
      fs::create_symlink(path.filename(), symlink);

      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << "Symbolic link " << symlink << " --> " << path.filename()
           << " was created.";
         logger.event(WHERE__, s.str());
      }
   }
}

//----------------------------------------------------------------------------------------
// Create directory and a symbolic link to the directory
//----------------------------------------------------------------------------------------
void Common::createDirAndLink(const fs::path& path)
{
   // Create directory
   if (fs::exists(path) == false)
   {
      fs::create_directory(path);

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << "Directory " << path << " was created.";
         logger.event(WHERE__, s.str());
      }
   }

   int ret = chmod(path.c_str(), 02777);      // This is completely insane !!!
   if (ret != 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to set permissions for directory " << path << ".";
      throw ex;
   }

   // Create symbolic link to directory
   const fs::path& file = path.filename();
   const fs::path symfile = boost::to_upper_copy(file.string());
   const fs::path& symlink = path.parent_path() / symfile;
   if (fs::exists(symlink) == false)
   {
      fs::create_symlink(path.filename(), symlink);

      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << "Symbolic link " << symlink << " --> " << path.filename()
           << " was created.";
         logger.event(WHERE__, s.str());
      }
   }
}

//----------------------------------------------------------------------------------------
// Create directories and a symbolic links to the directory
//----------------------------------------------------------------------------------------
void Common::createDirsAndLinks(const fs::path& path)
{
   fs::path::const_iterator iter;
   fs::path dir;
   int count(0);
   for (iter = path.begin(); iter != path.end(); ++iter)
   {
      dir /= *iter;
      if (count > 1)
      {
         Common::createDirAndLink(dir);
      }
      count++;
   }
}

//----------------------------------------------------------------------------------------
// Get CP type
//----------------------------------------------------------------------------------------
uint32_t Common::getCPType()
{
   ACS_CC_ReturnType result;

   OmHandler immHandle;
   result = immHandle.Init();
   if (result != ACS_CC_SUCCESS)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to read CP type." << endl;
      ex << "Could not initialize interaction with IMM.";
      throw ex;
   }

   std::vector<std::string> dnList;
   result = immHandle.getClassInstances("AxeFunctions", dnList);
   if (result != ACS_CC_SUCCESS)
   {
      immHandle.Finalize();
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to read CP type." << endl;
      ex << "Could not retrieve list of DN:s.";
      throw ex;
   }

   immHandle.Finalize();

   acs_apgcc_paramhandling par;
   int cpType(-1);
   result = par.getParameter(dnList[0], "apzProtocolType", &cpType);
   if (result != ACS_CC_SUCCESS)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to read CP type." << endl;
      ex << par.getInternalLastErrorText();
      throw ex;
   }

   return cpType;
}

//----------------------------------------------------------------------------------------
// Get node architecture
//----------------------------------------------------------------------------------------
Common::ArchitectureValue Common::getNodeArchitecture()
{
   ACS_CC_ReturnType result;

   OmHandler immHandle;
   result = immHandle.Init();
   if (result != ACS_CC_SUCCESS)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to read architecture type." << endl;
      ex << "Could not initialize interaction with IMM.";
      throw ex;
   }

   std::vector<std::string> dnList;
   result = immHandle.getClassInstances("AxeFunctions", dnList);
   if (result != ACS_CC_SUCCESS)
   {
      immHandle.Finalize();
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to read architecture type." << endl;
      ex << "Could not retrieve list of DN:s.";
      throw ex;
   }

   immHandle.Finalize();

   acs_apgcc_paramhandling par;
   int architecture(-1);
   result = par.getParameter(dnList[0], "apgShelfArchitecture", &architecture);
   if (result != ACS_CC_SUCCESS)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to read architecture type." << endl;
      ex << par.getInternalLastErrorText();
      throw ex;
   }

   return static_cast<ArchitectureValue>(architecture);
}

//----------------------------------------------------------------------------------------
// Copy to Media
//----------------------------------------------------------------------------------------
void Common::copyToMedia(const fs::path& path)
{
   const string& cmd = "emfcopy -m TODVD " + path.string();

   ostringstream os;

   // Execute command
   int exitCode = Common::executeCommand(cmd, os);
   if (exitCode != 0)
   {
      Exception ex(Exception::mediaFailed(), WHERE__);
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Get external NBI path from absolute path
//----------------------------------------------------------------------------------------
fs::path Common::getExtNbiPath(const fs::path& path)
{
   fs::path internal_root = Common::getFileMPath("internalRoot");
   size_t size = internal_root.string().size();
   size_t pos = path.string().find(internal_root.string());
   if (pos != 0)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Path " << path << " is not an NBI path.";
      throw ex;
   }

   return path.string().substr(size);
}

//----------------------------------------------------------------------------------------
//   Parse emfinfo line
//----------------------------------------------------------------------------------------
string Common::emfparse(istream& is, const string& keyword)
{
   string str;
   do
   {
      getline(is, str);
      if (is.eof())
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << "End of file reached.";
         throw ex;
      }
   }
   while (str.find(keyword) == string::npos);
   size_t pos = str.find_last_of(' ');
   return str.substr(pos + 1);
}

//----------------------------------------------------------------------------------------
// Read emfinfo result
//----------------------------------------------------------------------------------------
void Common::emfinfo(string& state, string& result, string& filename)
{
   stringstream s;
   int ret = Common::executeCommand("emfinfo", s);
   if (ret == 0)
   {
      // Get state
      state = emfparse(s, "State");

      // Get result
      result = emfparse(s, "Result");

      // Get file name
      do
      {
         string str;
         getline(s, str);
         if (str.empty() == false)
         {
            filename = str;
         }
      }
      while (!s.eof());
      size_t pos;
      pos = filename.find_first_not_of(' ');
      filename.erase(0, pos);
      pos = filename.find_first_of(' ', pos);
      filename.erase(pos);
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << s.str();
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Poll emfinfo until copying is done
//----------------------------------------------------------------------------------------
void Common::emfpoll(const string& filename)
{
   // Read emfinfo until copying has finished
   int waittime = 2;
   bool ready(false);
   do
   {
      string state;
      string result;
      string tfilename;

      // Read emfinfo
      emfinfo(state, result, tfilename);
      if (state == "PROCESSING")
      {
         // Processing - wait ...
         sleep(waittime);
      }
      else if (state == "IDLE")
      {
         // Is it correct file?
         if (tfilename == filename)
         {
            // File copy ready
            ready = true;
         }
         else
         {
            // Not started yet - wait...
            sleep(waittime);
         }
      }
      else
      {
         // Unexpected state - wait ...
         sleep(waittime);
      }
   }
   while (ready == false);
}

//----------------------------------------------------------------------------------------
// Check if emfcopy is running
//----------------------------------------------------------------------------------------
bool Common::isEmfBusy()
{
   string state;
   string result;
   string filename;

   // Read emfinfo
   try
   {
      emfinfo(state, result, filename);
   }
   catch (Exception& ex)
   {
      return false;
   }

   if (state == "PROCESSING")
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------------------
// Get AP node name
//----------------------------------------------------------------------------------------
Common::ApNodeName Common::getApNode(void)
{
   ApNodeName apName;
   ACS_DSD_Client dsdClient;
   ACS_DSD_Node ownNode;
   int ret = dsdClient.get_local_node(ownNode);
   if (ret == 0)
   {
      switch (ownNode.system_id)
      {
      case 2001:
      {
         apName = AP1;
         break;
      }
      case 2002:
      {
         apName = AP2;
         break;
      }
      default:
      {
         apName = UNKNOW;
         break;
      }
      }
   }
   else
   {
      apName = UNKNOW;
   }
   return apName;
}

//----------------------------------------------------------------------------------------
// Mount a folder to another folder
//----------------------------------------------------------------------------------------
void Common::mount(const char *source, const char *target)
{
   int retCode = ::mount(source, target, "ext3", MS_SYNCHRONOUS|MS_BIND, NULL);
   if(retCode != 0)
   {
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         ostringstream s;
         s << "Failed to mount source: " << source << " --> target: " << target;
         logger.event(WHERE__, s.str());
      }
   }
   else
   {
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << "Successfully to mount source: " << source << " --> target: " << target;
         logger.event(WHERE__, s.str());
      }
   }
}

//----------------------------------------------------------------------------------------
// Unmount a folder
//----------------------------------------------------------------------------------------
void Common::uMount(const char *target)
{
   int retCode = ::umount(target);
   if(retCode != 0)
   {
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         ostringstream s;
         s << "Failed to unmount: "<< target;
         logger.event(WHERE__, s.str());
      }
   }
   else
   {
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << "Successfully to unmount: "<< target;
         logger.event(WHERE__, s.str());
      }
   }


}

}//End namespace
