//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cfeted.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2020, 2013. All rights reserved.
//
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This is the main file for cfeted.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 0xxx  PA1
//
//  AUTHOR
//      EAB/FLE/DF UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      A      2010-03-16   UABTSO      CFET was migrated to APG50.
//             2010-11-04   XSCVERE     CFET was updated with Boost File system Functions
//      R2A03  2013-02-26   XVUNGUY     CFET was updated with supported Blade Memory configuration
//      -      2015-01-21   EGIAAVA     CFET was adapted for SLES12 and new BOOST LIB 1_54
//      -      2017-05-17   XMAHIMA     CFET was updated for SW MAU(MAUS) Improvement feature
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>
#include <exception.h>
#include <common.h>
#include <hwcinfo.h>
#include <dumps.h>
#include <dialogue.h>
//#include <ACS_ExceptionHandler.h>
//#include <ACS_PRC_Process.h>
//#include <ACS_AEH_signalHandler.h>
#include <string>
#include <iostream>
#include <bitset>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <unistd.h>

#include <mauconfig.h>
#include <boost/lexical_cast.hpp>

#include <cphwconfig.h>

using namespace std;
namespace fs = boost::filesystem;

const char s_AUT[]  = "AUT";    // Automatic upgrade
const char s_CPHW[] = "HW";     // CPHW dump
const char s_PES[]  = "PE";     // PlexEngine dump
const char s_BAK[]  = "BAK";    // Backwards"
const char s_CUR[]  = "CUR";    // Current
const char s_NEW[]  = "NEW";    // New
const char s_ALL[]  = "ALL";    // All
const char s_MAU[] =  "MAU";	// MAU


const char s_INCORRECTUSAGE[] = "Incorrect usage.";
const string& s_COMMANDNAME = "cfeted";

//---------------------------------------------------------------------------------------
// Check if CFETED process is already running
//---------------------------------------------------------------------------------------
bool createLock(const string& cmdname)
{
    bool found(false);
    try
    {
        // Get pid file
        fs::path pidpath = "/var/run";
        pidpath /= cmdname + ".pid";
        if (fs::exists(pidpath)) {
            // pid-file exists - read pid
            string pid;
            fs::ifstream pidfs(pidpath);
            getline(pidfs, pid, '\0');
            // Check if process exists in the proc filesystem
            fs::path path = "/proc";
            path /= pid;
            path /= "cmdline";
            if (fs::exists(path)) {
                // Process exists
                string cmdline;
                fs::ifstream pathfs(path);
                getline(pathfs, cmdline, '\0');
                const string& tcmdname = fs::path(cmdline).filename().string();
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
            pidfs << pid;
            return true;
        }
    }
    catch(...)
    {
        Exception ex(Exception::e_internal_29);
        ex << "file system error.";
        throw ex;
    }
}
//----------------------------------------------------------------------------------------
// Check CP side
//----------------------------------------------------------------------------------------
char
checkCpSide(const string& side)
{
    const string& tside = Common::toUpper(side);
    if (tside == "A" || tside == "CPA")
    {
        return 'A';
    }
    else if (tside == "B" || tside == "CPB")
    {
        return 'B';
    }
    else if (tside.empty())
    {
        return 0;
    }
    else
    {
        Exception ex(Exception::e_illegalcpside_7);
        ex << "Illegal CP side '" << tside << "'.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Check dump type
//----------------------------------------------------------------------------------------
DumpsBase::t_conftype
checkDumpType(const string& type)
{
    const string& ttype = Common::toUpper(type);
    if (ttype == s_CPHW)
    {
        return DumpsBase::e_cphw;
    }
    else if (ttype == s_PES)
    {
        return DumpsBase::e_pes;
    }
    else
    {
        Exception ex(Exception::e_illegaldumptype_8);
        ex << "Illegal dump type '" << ttype << "'.";
        throw ex;
    }
}
//----------------------------------------------------------------------------------------
// Check dump version
//----------------------------------------------------------------------------------------
DumpsBase::t_dumpversion
checkDumpVersion(const string& version)
{
    const string& tversion = Common::toUpper(version);
    if (tversion == s_BAK)
    {
        return DumpsBase::e_bak;
    }
    else if (tversion == s_CUR)
    {
        return DumpsBase::e_cur;
    }
    else if (tversion == s_NEW)
    {
        return DumpsBase::e_new;
    }
    else if (tversion == s_ALL)
    {
        return DumpsBase::e_all;
    }
    else
    {
        Exception ex(Exception::e_illegaldumpversion_9);
        ex << "Illegal dump version '" << tversion << "'.";
        throw ex;
    }
}
//----------------------------------------------------------------------------------------
// Command usage, (when cfeted is run on Multi CP system)
//----------------------------------------------------------------------------------------
void
usage1(bool verbose = false)
{
    cerr << endl;
    if (verbose == false)
    {
#if 0
        cerr << "Usage:" << endl;
        cerr << "cfeted -d dump package" << endl;
        cerr << "cfeted {-n hw | -n pe } {-cp cpname | -i cpid} -s cpside" << endl;
        cerr << "cfeted {-b hw | -b pe } {-cp cpname | -i cpid} -s cpside" << endl;
        cerr << "cfeted -c {-cp cpname | -i cpid} [-s cpside]" << endl;
        cerr << "cfeted -v version {-cp cpname | -i cpid} [-s cpside]" << endl;
        cerr << "cfeted -r {-cp cpname | -i cpid} [-s cpside]" << endl;
        cerr << "cfeted -x -cp cpname" << endl;
        cerr << "cfeted -cp cpname -n mau -s apside" << endl;
        cerr << "cfeted -cp cpname -b mau -s apside" << endl;
        cerr << "cfeted -m -cp cpname -v version" << endl;
        cerr << "cfeted -m -cp cpname -p config" << endl;
        cerr << "cfeted -m -cp cpname -l" << endl;
        cerr << "cfeted -h" << endl;
#endif

        cerr << "Usage:" << endl;
        cerr << "cfeted -d dump package" << endl;
        cerr << "cfeted [-t aut] -n dump -cp cpname [-s cpside]" << endl;
        cerr << "cfeted [-t aut] -n dump -i cpid [-s cpside]" << endl;
        cerr << "cfeted [-t aut] -b dump -cp cpname [-s cpside]" << endl;
        cerr << "cfeted [-t aut] -b dump -i cpid [-s cpside]" << endl;
	cerr << "cfeted -bootOpt 'BOOT_OPTIONS' -cp cpname [-s cpside]" << endl;
        cerr << "cfeted -bootOpt 'BOOT_OPTIONS' -i cpid [-s cpside]" << endl;
	cerr << "cfeted -bootOptList -cp cpname [-s cpside]" << endl;
	cerr << "cfeted -bootOptList -i cpid [-s cpside]" << endl;
        cerr << "cfeted -c -cp cpname [-s cpside]" << endl;
        cerr << "cfeted -c -i cpid -s [cpside]" << endl;
        cerr << "cfeted -c -l -cp cpname -s [cpside]" << endl;
        cerr << "cfeted -c -p config -cp cpname -s [cpside]" << endl;
        cerr << "cfeted -c -v version -cp cpname -s [cpside]" << endl;
        cerr << "cfeted -x -cp cpname" << endl;
        cerr << "cfeted -v version -cp cpname [-s cpside]" << endl;
        cerr << "cfeted -v version -i cpid [-s cpside]" << endl;
        cerr << "cfeted -r -cp cpname [-s cpside]" << endl;
        cerr << "cfeted -r -i cpid [-s cpside]" << endl;
        cerr << "cfeted -m -l" << endl;
        cerr << "cfeted -m -cp cpname -p config" << endl;
        cerr << "cfeted -m -cp cpname -v version" << endl;
        cerr << "cfeted -cp cpname -n dump1 -s apside" << endl;
        cerr << "cfeted -cp cpname -b dump1 -s apside" << endl;
        cerr << "cfeted -h" << endl;

        //cerr << "cfeted " << endl;
        //cerr << "cfeted " << endl;
    }
    else
    {
        cerr << "-----------------------------------------------------------" << endl;
        cerr << "              Configuration Editing Tool" << endl;
        cerr << endl;
        cerr << " (C) Copyright Ericsson AB 2007, 2015. All Rights Reserved. " << endl;
        cerr << "-----------------------------------------------------------" << endl;
        cerr << endl;
        cerr << "Install PES and CPHW dump package:" << endl;
        cerr << endl;
        cerr << "  cfeted -d dump package" << endl;
        cerr << endl;
        cerr << "      -d  dump        Dump type:" << endl;
        cerr << "                      pe   PlexEngine dump" << endl;
        cerr << "                      hw   CPHW dump" << endl;
        cerr << "      package         Package file" << endl;
        cerr << endl;
        cerr << "Configuration of PES and CPHW dumps:" << endl << endl;
        cerr << "  cfeted [-t aut] -n dump -cp cpname [-s cpside]" << endl;
        cerr << "  cfeted [-t aut] -n dump -i cpid [-s cpside]" << endl;
        cerr << "  cfeted [-t aut] -b dump -cp cpname [-s cpside]" << endl;
        cerr << "  cfeted [-t aut] -b dump -i cpid [-s cpside]" << endl;
        cerr << endl;
        cerr << "      -n  dump        Configure a new dump:" << endl;
        cerr << "                      pe   PlexEngine dump" << endl;
        cerr << "                      hw   CPHW dump" << endl;
        cerr << "      -b  dump        Restore a backup dump:" << endl;
        cerr << "                      pe   PlexEngine dump" << endl;
        cerr << "                      hw   CPHW dump" << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -i  cpid        CP identity for a multiple CP system." << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;
        cerr << "Configuration of Security mitigations:" << endl << endl;
        cerr << "  cfeted -bootOpt 'BOOT_OPTIONS' -cp cpname [-s cpside]" << endl;
        cerr << "  cfeted -bootOpt 'BOOT_OPTIONS' -i cpid [-s cpside]" << endl;
        cerr << "  cfeted -bootOptList -cp cpname [-s cpside]" << endl;
        cerr << "  cfeted -bootOptList -i cpid [-s cpside]" << endl;
	cerr << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -i  cpid        CP identity for a multiple CP system." << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << "      'BOOT_OPTIONS'  List all strings to enable/disable each mitigation seperated by space" << endl;
        cerr << endl;
        cerr << "Memory configuration for PES (a dialogue to make a selection is shown):" << endl;
        cerr << endl;
        cerr << "  cfeted -c -cp cpname [-s cpside]" << endl;
        cerr << "  cfeted -c -i cpid [-s cpside]" << endl;
        cerr << "      -c              Memory configuration of PES." << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -i  cpid        CP identity for a multiple CP system." << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;
        cerr << "Memory configuration for XPU:" << endl;
        cerr << "  cfeted -x -cp cpname" << endl;
        cerr << "  cfeted -x -i cpid" << endl;
        cerr << "      -x              Memory configuration for XPU." << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -i  cpid        CP identity for a multiple CP system." << endl;
        cerr << endl;
        cerr << "Print PES and CPHW dump configurations:" << endl;
        cerr << endl;
        cerr << "  cfeted -v version -cp cpname  [-s cpside]" << endl;
        cerr << "  cfeted -v version -i cpid [-s cpside]" << endl;
        cerr << endl;
        cerr << "      -v  version     Version to print:" << endl;
        cerr << "                      bak  Backup version" << endl;
        cerr << "                      cur  Current version" << endl;
        cerr << "                      new  New version" << endl;
        cerr << "                      all  All versions" << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -i  cpid        CP identity for a multiple CP system." << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;
        cerr << "Remove configuration:" << endl;
        cerr << endl;
        cerr << "  cfeted -r {-cp cpname | -i cpid} [-s cpside]" << endl;
        cerr << "      -r              Remove configuration." << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -i  cpid        CP identity for a multiple CP system." << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;

        // Help for SW MAU
        cerr << "Print and Configuration of MAU dump:" << endl;
        cerr << endl;
        cerr << "  cfeted -cp cpname -n dump1 -s apside" << endl;
        cerr << "  cfeted -cp cpname -b dump1 -s apside" << endl;
        cerr << "      -n dump1        Configure a new MAU software dump1." << endl;
        cerr << "                      mau    MAU software dump" << endl;
        cerr << "      -b dump1        Restore a backup of MAU software dump1." << endl;
        cerr << "                      mau    MAU software dump" << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -s apside       AP side:" << endl;
        cerr << "                      apa    AP side A" << endl;
        cerr << "                      apb    AP side B" << endl;
        cerr << endl;

        cerr << "  cfeted -m -cp cpname -v version" << endl;
        cerr << "  cfeted -m -cp cpname -p config" << endl;
        cerr << "  cfeted -m -l" << endl;
        cerr << "      -v version       Version to print:" << endl;
        cerr << "                       bak  Backup version" << endl;
        cerr << "                       cur  Current version" << endl;
        cerr << "                       new  New version" << endl;
        cerr << "                       all  All versions" << endl;
        cerr << "      -p config        New configuration number to configure" << endl;
        cerr << "      -l               List configuration numbers" << endl;
        cerr << endl;

        cerr << "Print and Configuration of CP MAU dump:" << endl;
        cerr << "  cfeted -c -v version -cp cpname -s [cpside]" << endl;
        cerr << "  cfeted -c -p config -cp cpname -s [cpside]" << endl;
        cerr << "  cfeted -c -l -cp cpname -s [cpside]" << endl;
        cerr << "      -v version       Version to print:" << endl;
        cerr << "                       bak  Backup version" << endl;
        cerr << "                       cur  Current version" << endl;
        cerr << "      -p config        New configuration number to configure" << endl;
        cerr << "      -l               List configuration numbers" << endl;
        cerr << "      -cp cpname       CP name for a multiple CP system." << endl;
        cerr << "      -s  cpside       CP side:" << endl;
        cerr << "                       a    CP side A" << endl;
        cerr << "                       b    CP side B" << endl;
        cerr << endl;

        cerr << "Detailed help for this command:" << endl;
        cerr << endl;
        cerr << "  cfeted -h" << endl;
    }
}
//----------------------------------------------------------------------------------------
// Command usage, (when cfeted is run on One CP system)
//----------------------------------------------------------------------------------------
void
usage2(bool verbose = false)
{
    cerr << endl;
    if (verbose == false)
    {
#if 0
        cerr << "Usage:" << endl;
        cerr << "cfeted -d dump package" << endl;
        cerr << "cfeted {-n hw | -n pe} -s cpside" << endl;
        cerr << "cfeted {-b hw | -b pe} -s cpside" << endl;
        cerr << "cfeted -c -s cpside" << endl;
        cerr << "cfeted -v version [-s cpside]" << endl;
        cerr << "cfeted -n mau -s apside" << endl;
        cerr << "cfeted -b mau -s apside" << endl;
        cerr << "cfeted -m -v version" << endl;
        cerr << "cfeted -m -cfg config" << endl;
        cerr << "cfeted -m -l" << endl;
        cerr << "cfeted -h" << endl;
#endif

        cerr << "Usage:" << endl;
        cerr << "cfeted -d dump package" << endl;
        cerr << "cfeted [-t aut] -n dump -s cpside" << endl;
        cerr << "cfeted [-t aut] -b dump -s cpside" << endl;
	cerr << "cfeted -bootOpt 'BOOT_OPTIONS' -s cpside" << endl;
	cerr << "cfeted -bootOptList -s cpside" << endl;
        cerr << "cfeted -c -s cpside" << endl;
        cerr << "cfeted -c -l -s [cpside]" << endl;
        cerr << "cfeted -c -p config -s [cpside]" << endl;
        cerr << "cfeted -c -v version -s [cpside]" << endl;
        cerr << "cfeted -v version [-s cpside]" << endl;
        cerr << "cfeted -m -l" << endl;
        cerr << "cfeted -m -p config" << endl;
        cerr << "cfeted -m -v version" << endl;
        cerr << "cfeted -n dump1 -s apside" << endl;
        cerr << "cfeted -b dump1 -s apside" << endl;
        cerr << "cfeted -h" << endl;

        //cerr << "cfeted " << endl;
        //cerr << "cfeted " << endl
    }
    else
    {
        cerr << "-----------------------------------------------------------" << endl;
        cerr << "              Configuration Editing Tool" << endl;
        cerr << endl;
        cerr << " (C) Copyright Ericsson AB 2007, 2015. All Rights Reserved. " << endl;
        cerr << "-----------------------------------------------------------" << endl;
        cerr << endl;
        cerr << "Install PES and CPHW dump package:" << endl;
        cerr << endl;
        cerr << "  cfeted -d dump package" << endl;
        cerr << endl;
        cerr << "      -d  dump        Dump type:" << endl;
        cerr << "                      pe   PlexEngine dump" << endl;
        cerr << "                      hw   CPHW dump" << endl;
        cerr << "      package         Package file" << endl;
        cerr << endl;
        cerr << "Configuration of dumps:" << endl << endl;
        cerr << "  cfeted -n dump -s cpside" << endl;
        cerr << "  cfeted -b dump -s cpside" << endl;
        cerr << endl;
        cerr << "      -n  dump        Configure a new dump:" << endl;
        cerr << "                      pe   PlexEngine dump" << endl;
        cerr << "                      hw   CPHW dump" << endl;
        cerr << "      -b  dump        Restore a backup dump:" << endl;
        cerr << "                      pe   PlexEngine dump" << endl;
        cerr << "                      hw   CPHW dump" << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;
        cerr << "Configuration of Security mitigations:" << endl << endl;
        cerr << "  cfeted -bootOpt 'BOOT_OPTIONS' -s cpside" << endl;
        cerr << "  cfeted -bootOptList -s cpside" << endl;
        cerr << endl;
        cerr << "cfeted -bootOpt" << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << "      'BOOT_OPTIONS'  List all strings to enable/disable each mitigation seperated by space" << endl;
        cerr << endl;
        cerr << "Memory configuration for PES (a dialogue to make a selection is shown):" << endl;
        cerr << endl;
        cerr << "  cfeted -c -s cpside" << endl;
        cerr << "      -c              Memory configuration." << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;
        cerr << "Print PES and CPHW dump configurations:" << endl;
        cerr << endl;
        cerr << "  cfeted -v version [-s cpside]" << endl;
        cerr << endl;
        cerr << "      -v  version     Version to print:" << endl;
        cerr << "                      bak  Backup version" << endl;
        cerr << "                      cur  Current version" << endl;
        cerr << "                      new  New version" << endl;
        cerr << "                      all  All versions" << endl;
        cerr << "      -s  cpside      CP side:" << endl;
        cerr << "                      a    CP side A" << endl;
        cerr << "                      b    CP side B" << endl;
        cerr << endl;

        // Help for SW MAU
        cerr << "Print and Configuration of MAU dump:" << endl;
        cerr << endl;
        cerr << "  cfeted -n dump1 -s apside" << endl;
        cerr << "  cfeted -b dump1 -s apside" << endl;
        cerr << "      -n dump1        Configure a new MAU software dump1." << endl;
        cerr << "                      mau    MAU software dump" << endl;
        cerr << "      -b dump1        Restore a backup MAU software dump1." << endl;
        cerr << "                      mau    MAU software dump" << endl;
        cerr << "      -cp cpname      CP name for a multiple CP system." << endl;
        cerr << "      -s apside       AP side:" << endl;
        cerr << "                      apa    AP side A" << endl;
        cerr << "                      apb    AP side B" << endl;
        cerr << endl;

        cerr << "  cfeted -m  -v version" << endl;
        cerr << "  cfeted -m  -p config " << endl;
        cerr << "  cfeted -m -l" << endl;
        cerr << "      -v version       Version to print:" << endl;
        cerr << "                       bak  Backup version" << endl;
        cerr << "                       cur  Current version" << endl;
        cerr << "                       new  New version" << endl;
        cerr << "                       all  All versions" << endl;
        cerr << "      -p config        New configuration number to configure" << endl;
        cerr << "      -l               List configuration numbers" << endl;
        cerr << endl;

        cerr << "Print and Configuration of CP MAU dump:" << endl;
        cerr << "  cfeted -c -v version -s [cpside]" << endl;
        cerr << "  cfeted -c -p config -s [cpside]" << endl;
        cerr << "  cfeted -c -l -s [cpside]" << endl;
        cerr << "      -v version       Version to print:" << endl;
        cerr << "                       bak  Backup version" << endl;
        cerr << "                       cur  Current version" << endl;
        cerr << "      -p config        New configuration number to configure" << endl;
        cerr << "      -l               List configuration numbers" << endl;
        cerr << "      -s  cpside       CP side:" << endl;
        cerr << "                       a    CP side A" << endl;
        cerr << "                       b    CP side B" << endl;
        cerr << endl;

        cerr << "Detailed help for this command:" << endl;
        cerr << endl;
        cerr << "  cfeted -h" << endl;
    }
}
//----------------------------------------------------------------------------------------
// Command usage.
//----------------------------------------------------------------------------------------
void
usage(bool verbose = false)
{
    if (CPInfo::isMultiCPSystem() == true)
    {
        // We are running on a Multi CP system...
        usage1(verbose);
    }
    else
    {
        // We are running on a One CP system...
        usage2(verbose);
    }
}
//----------------------------------------------------------------------------------------
// Command usage printout hinting for -h help flag
// to get most details about the command usage
//----------------------------------------------------------------------------------------
void usage_helpflag()
{
    cerr << "For more usage details, use 'cfeted -h'." << endl;
}

//----------------------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------------------
int
main(int argc, const char* argv[])
{
    // Follow AP design rule for crash dump config and handling.
    // AP_SetCleanupAndCrashRoutine(s_COMMANDNAME.c_str(), NULL);
    // Setting default signal handler
    //ACS_AEH_setSignalExceptionHandler(s_COMMANDNAME.c_str(), PRC);
    // Setting process priority
    //AP_InitProcess(s_COMMANDNAME.c_str(), AP_COMMAND);
    // cout << endl; // Commented away redundant newline...
    // Set APZ data path
	try
    {
		Common::setApzDataPath();
        // Check if data directory exists
        const string& datapath = Common::getApzDataPath();
        if (Common::fileExists(datapath) == false)
        {
            Exception ex(Exception::e_system_28);
            ex << "Directory '" << datapath << "' does not exist.";
            throw ex;
        }
        //Check if CFETED process is already running, if not create pid file
        fs::path cmdline = argv[0];
        const string& cmdname = cmdline.filename().string();
        bool ok = createLock(cmdname);
        if (ok == false)
        {
            Exception ex(Exception::e_alreadyexec_3);
            ex << "Command is already executing.";
            throw ex;
        }
    }
    catch (Exception& ex)
    {
        cerr << ex << endl;
        return ex.errorCode();
    }

	typedef bitset<32> BITSET;
    BITSET options;
    enum
    {
        e_install,          // Option -d
        e_conftype,         // Option -t
        e_memory,           // Option -c
        e_remove,           // Option -r
        e_version,          // Option -v
        e_cpname,           // Option -cp
        e_cpid,             // Option -i
        e_cpside,           // Option -s
        e_upgrade,          // Option -n
        e_backup,           // Option -b
        e_blademem,         // Option -x
        e_help,             // Option -h
        e_lab,              // Option -lab
        e_mau,              // Option -m
        e_list,             // Option -l
        e_cfg,              // Option -p
        e_upgrade_mau,      // Option -n mau
        e_backup_mau,       // Option -b mau
       	e_modify_patches,   // Option -bootOpt
        e_list_patches      // Option -bootOptList
    };
    string conftype;
    string side;
    string cpname;
    CPID cpid(0);
    string dumptype = "";
    string version;
    BITSET allowed;
    int i = 1;
    string mauconfig;
    string patches;
    string cpid_str;
    string cparg;
    try
    {
        //cout<< "i value before starting of while:" << i <<endl;
        while (i < argc && argv[i][0] == '-')
        {
            //cout << "Iteration:"<<i<<endl;
            //cout << "Argument:"<<argv[i]<<endl;
            (void) allowed.reset();
            const string& option = argv[i];
            if (option == "-d")
            {
                if (options.none())
                {
                    i++;
                    if (i == argc || argv[i][0] == '-')
                    {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_install);
                    dumptype = argv[i];
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-lab") // Manipulate auto_exec file
            {
               (void) allowed.set(e_upgrade);
               (void) allowed.set(e_conftype);
               (void) allowed.set(e_cpname);
               (void) allowed.set(e_cpid);
               (void) allowed.set(e_cpside);
               (void) allowed.set(e_backup);
               if ((options & ~allowed).none())
               {
                  (void) options.set(e_lab);
               }
               else
               {
                  cerr << s_INCORRECTUSAGE << endl;
                  usage();
                  return Exception::e_usage_2;
               }
            }
            else if (option == "-t") // Automatic option, deprecated
            {
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_upgrade);
                (void) allowed.set(e_backup);
                if ((options & ~allowed).none())
                {
                    i++;
                    if (i == argc || argv[i][0] == '-')
                    {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_conftype);
                    conftype = Common::toUpper(argv[i]);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-c") // Memory configuration option
            {
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_list);
                (void) allowed.set(e_cfg);
                (void) allowed.set(e_version);
                if ((options & ~allowed).none()) {
                    (void) options.set(e_memory);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-v") // List versions option
            {
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_mau);
                (void) allowed.set(e_memory);
                if ((options & ~allowed).none()) {
                    i++;
                    if (i == argc || argv[i][0] == '-') {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_version);
                    version = argv[i];
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-cp") // CP name option
            {
                (void) allowed.set(e_conftype);
                (void) allowed.set(e_memory);
                (void) allowed.set(e_version);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_upgrade);
                (void) allowed.set(e_backup);
                (void) allowed.set(e_remove);
                (void) allowed.set(e_blademem);
                (void) allowed.set(e_mau);
                (void) allowed.set(e_list);
                (void) allowed.set(e_cfg);
                (void) allowed.set(e_upgrade_mau);
                (void) allowed.set(e_backup_mau);
		(void) allowed.set(e_modify_patches);
                (void) allowed.set(e_list_patches);
                if ((options & ~allowed).none())
                {
                    i++;
                    if (i == argc || argv[i][0] == '-')
                    {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_cpname);
                    cpname = argv[i];
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-i") // CP identity option
            {
                (void) allowed.set(e_conftype);
                (void) allowed.set(e_memory);
                (void) allowed.set(e_version);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_upgrade);
                (void) allowed.set(e_backup);
                (void) allowed.set(e_remove);
		(void) allowed.set(e_modify_patches);
                (void) allowed.set(e_list_patches);
                if ((options & ~allowed).none()) {
                    i++;
                    if (i == argc || argv[i][0] == '-')
                    {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_cpid);
			cpid_str = argv[i];
                    try
                    {
                        cpid = (CPID) Common::strToInt(argv[i]);
                    }
                    // Split into two exception codes... catch (Exception& ex)
                    catch (int val)
                    {
                        if (val == 1)
                        {
                            Exception ex1(Exception::e_incorr_argfor_i_int_11);
                            ex1 << "Incorrect argument for option '" << option
                                    << "' - ";
                            ex1 << "Integer was expected." << endl;
                            cerr << ex1 << endl;
                            return Exception::e_incorr_argfor_i_int_11;
                        }
                        else if (val == 2)
                        {
                            Exception ex2(Exception::e_incorr_argfor_i_ovf_12);
                            ex2 << "Incorrect argument for option '" << option
                                    << "' - ";
                            ex2 << "Overflow occurred.";
                            cerr << ex2 << endl;
                            return Exception::e_incorr_argfor_i_ovf_12;
                        }
                        else
                        {
                            Exception ex3(Exception::e_internal_29);
                            ex3 << "Unexpected exception value." << endl;
                            cerr << ex3 << endl;
                            return Exception::e_internal_29;
                            //throw Exception(Exception::e_internal_29,"Unexpected exception value.");
                        }
                    }
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-s") // CP and AP side option
            {
                (void) allowed.set(e_conftype);
                (void) allowed.set(e_memory);
                (void) allowed.set(e_version);
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_upgrade);
                (void) allowed.set(e_backup);
                (void) allowed.set(e_remove);
                (void) allowed.set(e_mau);
                (void) allowed.set(e_upgrade_mau);
                (void) allowed.set(e_backup_mau);
                (void) allowed.set(e_list);
                (void) allowed.set(e_cfg);
		(void) allowed.set(e_modify_patches);
                (void) allowed.set(e_list_patches);
                if ((options & ~allowed).none()) {
                    i++;
                    if (i == argc || argv[i][0] == '-')
                    {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_cpside);
                    side = argv[i];
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-n") // New configuration option
            {
                (void) allowed.set(e_conftype);
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_lab);

                if ((options & ~allowed).none())
                {
                	++i;
                	if (i == argc || argv[i][0] == '-')
                	{
                		cerr << "Option '" << option
                				<< "' requires an argument." << endl;
                		cerr << s_INCORRECTUSAGE << endl;
                		usage();
                		return Exception::e_usage_2;
                	}
                	dumptype = argv[i];
                	//const string dtype = Common::toUpper(dumptype);
                	string dtype = boost::to_upper_copy(dumptype);
                	if (dtype == s_MAU)
                	{
                		(void) options.set(e_upgrade_mau);
                	}
                	else
                	{
                		(void) options.set(e_upgrade);
                	}
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-b") // Backup configuration option
            {
                (void) allowed.set(e_conftype);
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_lab);
                if ((options & ~allowed).none())
                {
                	++i;
                	if (i == argc || argv[i][0] == '-')
                	{
                		cerr << "Option '" << option
                				<< "' requires an argument." << endl;
                		cerr << s_INCORRECTUSAGE << endl;
                		usage();
                		return Exception::e_usage_2;
                	}
                	dumptype = argv[i];

                	//const string dtype = Common::toUpper(dumptype);
                	string dtype = boost::to_upper_copy(dumptype);
                	if (dtype == s_MAU)
                	{
                		(void) options.set(e_backup_mau);
                	}
                	else
                	{
                		(void) options.set(e_backup);
                	}
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-r") // Remove configuration option
            {
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                (void) allowed.set(e_cpside);
                if ((options & ~allowed).none())
                {
                    (void) options.set(e_remove);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-x") // Blade memory configuration option
            {
                (void) allowed.set(e_cpname);
                if ((options & ~allowed).none())
                {
                    (void) options.set(e_blademem);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-h") // Help
            {
                if (options.none())
                {
                    (void) options.set(e_help);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-m") // MAU
            {
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_version);
                (void) allowed.set(e_cfg);
                (void) allowed.set(e_list);
                if ((options & ~allowed).none()) {
                    (void) options.set(e_mau);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-p") // CONFIG
            {
                (void) allowed.set(e_mau);
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_memory);
                (void) allowed.set(e_cpside);
                if ((options & ~allowed).none())
                {
                    i++;
                    if (i == argc || argv[i][0] == '-')
                    {
                        cerr << "Option '" << option
                                << "' requires an argument." << endl;
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    (void) options.set(e_cfg);
                    mauconfig = argv[i];
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-l") // CONFIG
            {
                (void) allowed.set(e_mau);
                (void) allowed.set(e_memory);
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_cpname);
		(void) allowed.set(e_modify_patches);
                if ((options & ~allowed).none()) {
                    (void) options.set(e_list);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-bootOpt") // Modify Mitigations
            {
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_list);
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                if ((options & ~allowed).none())
                {
                  ++i;
                  if (i == argc || argv[i][0] == '-')
		              {
                    cerr << "Option '" << option
                         << "' requires an argument." << endl;
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                  }
                  patches = argv[i];
                  if (patches[0] == '\'')
                  { 
                    string tmp = argv[i];
                    while (++i < argc){
                      if (argv[i][0] != '-'){
                        tmp.append(" ");
                        tmp.append(argv[i]);
                      }else{
                        --i; break;
                      }
                    }
                    patches = tmp;
                  }
                  (void) options.set(e_modify_patches);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else if (option == "-bootOptList") // List Mitigations
            {
                (void) allowed.set(e_cpside);
                (void) allowed.set(e_cpname);
                (void) allowed.set(e_cpid);
                if ((options & ~allowed).none()) {
                    (void) options.set(e_list_patches);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else
            {
                cerr << "Illegal option '" << argv[i] << "'." << endl;
                usage();
                return Exception::e_usage_2;
            }
            ++i;
        } // End parameter parsing

        // Commmand execution
        // ------------------

        if (CPInfo::isMultiCPSystem() == true)
        {
            // Multi CP system
        	// ---------------
            if (options.test(e_install))
            {
                // Install package
                if (argc != i + 1) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                switch (checkDumpType(dumptype))
                {
                case DumpsBase::e_cphw:
                    Dumps<Common::e_multicp>::installCPHW(argv[i]);
                    cout << "CPHW dump was installed." << endl;
                    break;
                case DumpsBase::e_pes:
                    Dumps<Common::e_multicp>::installPES(argv[i]);
                    cout << "PES dump was installed." << endl;
                    break;
                default:
                    // Should never get here since checkDumpType has checked it already...
                    throw Exception(Exception::e_internal_29,
                            "Illegal dump type.");
                }

            }
            else if(options.test(e_modify_patches))
            {
		// Apply patches
                // HWC information
                HWCInfo hwcinfo;
                bool found=false;
                bool isBlade=false;

                if (options.test(e_cpid) && !options.test(e_cpname))
                {
                    // Set CP identity
                    found = hwcinfo.setId(cpid);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP identity not found.";
                        throw ex;
                    }
                }
                else if (options.test(e_cpname) && !options.test(e_cpid))
                {
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP name not found.";
                        throw ex;
                    }
                }
                else if (!options.test(e_cpid) && !options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP name or identity must be specified for a multiple CP system.";
                    throw ex;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }


                if(options.test(e_cpid))
                  cparg = cpid_str;
                else if(options.test(e_cpname))
                  cparg = cpname;

               // If side not given, check if it's a double sided CP,
               // or single sided CP and call different print methods.
                    CPID cpId = hwcinfo.getCpId();
                    unsigned short sysType = 1000 * (cpId / 1000);
                    switch (sysType) {
                    case ACS_CS_API_HWC_NS::SysType_BC:
                        // Single sided machine,
                        // then call printConfigurations()
                        // Set CP side to 0 since this is a single side CP.
                        found = hwcinfo.setSide(0);
                        if (found == false) {
                            Exception ex(Exception::e_cpboardnotfound_4);
                            ex << "CP board not found.";
                            throw ex;
                        }
                        isBlade=true;
                        break;
                    case ACS_CS_API_HWC_NS::SysType_CP:
                        isBlade=false;
                        break;
                    default:
                        Exception ex(Exception::e_internal_29);
                        ex << "Illegal CP identity " << cpId << ".";
                        throw ex;
                    } // end of switch



                if (!options.test(e_cpside))
                {
  		    if(isBlade) 
                       DumpsBase::modifyPatches(cparg, patches);
                     else 
                      {
                        // Double sided machine,
                       Exception ex(Exception::e_illopt_sysconf_116);
                       ex << "CP side must be specified for a Dual sided CP";
                       throw ex;
                      }
                }
                else
                {
                    // This is the original code for printing one side, i.e.
                    // -s <side> was given so use print method for one side only...

                   if(isBlade)
                   {
                       // Double sided machine,
                       Exception ex(Exception::e_illopt_sysconf_116);
                       ex << "CP side is not allowed for a single sided CP.";
                       throw ex;
                   }

                    char cpside = checkCpSide(side);
                    // Set CP side
                    found = hwcinfo.setSide(cpside);
                    if (found == false) {
                        Exception ex(Exception::e_cpboardnotfound_4);
                        ex << "CP board not found.";
                        throw ex;
                    }
		    DumpsBase::modifyPatches(cparg, side, patches);
                }

            }
            else if(options.test(e_list_patches))
            {
		// list patches 
		if(!options.test(e_cpname) && !options.test(e_cpid))
		{
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP name or identity must be specified for a multiple CP system.";
                    throw ex;
		}
		
		if(options.test(e_cpid))
		    cparg = cpid_str;
                else if(options.test(e_cpname))
                    cparg = cpname;

                // HWC information
                HWCInfo hwcinfo;
                bool found;
                if (options.test(e_cpid) && !options.test(e_cpname))
                {
                    // Set CP identity
                    found = hwcinfo.setId(cpid);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP identity not found.";
                        throw ex;
                    }
                }
                else if (options.test(e_cpname) && !options.test(e_cpid))
                {
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP name not found.";
                        throw ex;
                    }
                }
                else if (!options.test(e_cpid) && !options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP name or identity must be specified for a multiple CP system.";
                    throw ex;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }


               if(!options.test(e_cpside))
                {
                    // If side not given, check if it's a double sided CP,
                    // or single sided CP and call different print methods.
                   CPID cpId = hwcinfo.getCpId();
                   unsigned short sysType = 1000 * (cpId / 1000);
                    switch (sysType) {
                    case ACS_CS_API_HWC_NS::SysType_BC:
                        // Single sided machine,
                        // then call printConfigurations()
                        // Set CP side to 0 since this is a single side CP.
                        found = hwcinfo.setSide(0);
                        if (found == false) {
                            Exception ex(Exception::e_cpboardnotfound_4);
                            ex << "CP board not found.";
                            throw ex;
                        }
                        DumpsBase::listPatches(cparg);
                        break;
                    case ACS_CS_API_HWC_NS::SysType_CP:
                        // Double sided machine,
                        side = "a";
                        DumpsBase::listPatches(cparg, side);
                        side = "b";
                        DumpsBase::listPatches(cparg, side);
                        break;
                    default:
                        Exception ex(Exception::e_internal_29);
                        ex << "Illegal CP identity " << cpId << ".";
                        throw ex;
                    } // end of switch
                }
                else
                {
                   // -s option is not allowed in BC 
                   CPID cpId = hwcinfo.getCpId();
                   unsigned short sysType = 1000 * (cpId / 1000);
                   if( options.test(e_cpside) && sysType == ACS_CS_API_HWC_NS::SysType_BC )
                   {
                     Exception ex(Exception::e_illopt_sysconf_116);
                     ex << "CP side is not allowed for a single sided CP.";
                     throw ex;
                   }

                   char cpside = checkCpSide(side);
                   DumpsBase::listPatches(cparg, side);
                }
            }

            else if ((options.test(e_upgrade) || options.test(e_backup)))
            {
                // Configure dump
                if (argc != i)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                if (options.test(e_conftype))
                {
                    if (conftype != s_AUT)
                    {
                        cerr << "Illegal configuration type '" << conftype
                                << "'." << endl;
                        cerr << "Warning: Option '-t' is deprecated." << endl;
                        usage();
                        Exception ex(Exception::e_usage_2);
                        throw ex;
                    }
                }
                // Dump version
                DumpsBase::t_dumpversion dumpver;
                if (options.test(e_backup))
                {
                    dumpver = DumpsBase::e_bak;
                }
                else if (options.test(e_upgrade))
                {
                    dumpver = DumpsBase::e_new;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                // HWC information
                HWCInfo hwcinfo;
                bool found;
                if (options.test(e_cpid) && !options.test(e_cpname))
                {
                    // Set CP identity
                    found = hwcinfo.setId(cpid);
                    if (found == false)
                    {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP identity not found.";
                        throw ex;
                    }
                }
                else if (options.test(e_cpname) && !options.test(e_cpid))
                {
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP name not found.";
                        throw ex;
                    }
                }
                else if (!options.test(e_cpid) && !options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP name or identity must be specified for a multiple CP system.";
                    throw ex;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                char cpside = checkCpSide(side);
                // Set CP side
                found = hwcinfo.setSide(cpside);
                if (found == false)
                {
                    Exception ex(Exception::e_cpboardnotfound_4);
                    ex << "CP board not found.";
                    throw ex;
                }
                // Dump type
                switch (checkDumpType(dumptype))
                {
                case DumpsBase::e_cphw:
                    Dumps<Common::e_multicp>::configCPHW(hwcinfo, dumpver);
                    break;
                case DumpsBase::e_pes:
                    Dumps<Common::e_multicp>::configPES(hwcinfo, dumpver);
                    break;
                default:
                    throw Exception(Exception::e_internal_29,
                            "Illegal dump type.");
                }
                // Fix TR HR57367
                if(!options.test(e_lab))
                {
                   Common::removeDebugFile();
                }

            }
            else if (options.test(e_remove))
            {
                if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                // HWC information
                HWCInfo hwcinfo;
                bool found;
                if (options.test(e_cpid) && !options.test(e_cpname))
                {
                    // Set CP identity
                    found = hwcinfo.setId(cpid);
                    // Get default CP name
                    cpname = CPInfo::getDefaultCPName(cpid);
                }
                else if (options.test(e_cpname) && !options.test(e_cpid))
                {
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                }
                else if (!options.test(e_cpid) && !options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP name or identity must be specified for a multiple CP system.";
                    throw ex;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                char cpside = checkCpSide(side);
                if (found)
                {
                    cpname = Common::toUpper(cpname);
                    // CP exists, check if CP side exists
                    found = hwcinfo.setSide(cpside);
                    if (found == true)
                    {
                        bool
                        ok =
                                Dialogue::affirm(
                                        "Warning: You are attempting to remove the configuration for an "
                                        "existing\nCP side or blade, in that case you will not be able "
                                        "to boot this CP side\nor blade, do you want to continue anyway?\n");
                        cout << endl;
                        if (ok == false) {
                            return Exception::e_ok_0;
                        }
                    }
                }
                // Remove configuration
                Dumps<Common::e_multicp>::removeConfig(cpname, cpside);
            }
            else if (options.test(e_memory))
            {
            	bool forcpmau=false;
            	if(options.test(e_cfg) || options.test(e_list) || options.test(e_version))
            	{
            		forcpmau = true;
            	}
            	if(forcpmau)
            	{
                    bool done = false;
                    HWCInfo hwcinfo;       // HWC information
                    bool found;
                    int cpid;
                    // Print command format
                    if (argc != i)
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }

                    if (!options.test(e_cpname))
                    {
                        Exception ex(Exception::e_illopt_sysconf_116);
                        ex << "CP name must be specified for a multiple CP system.";
                        throw ex;
                    }

                    if (!options.test(e_cpside))
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }

                    const string& _cpname = boost::to_upper_copy(cpname);
                    if ((_cpname != "CP1") && (_cpname != "CP2"))
                    {
                    	cerr << _cpname << " is not supported." << endl;
                    	usage();
                    	return Exception::e_usage_2;
                    }
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                    if (found == false) {
                    	Exception ex(Exception::e_noconf_cp_or_side_16);
                    	ex << _cpname << " name not found.";
                    	throw ex;
                    }

                    // This is old code, "-l -m" does not need "-cp cpname"
                    if (_cpname == "CP1")
                    {
                    	cpid = 1;
                    }
                    else
                    {
                    	cpid = 2;
                    }

                    // Check if MAUS is running
                    if (!Common::isMAUS(1001) && !Common::isMAUS(1002))
                    {
                        Exception ex(Exception::e_illopt_sysconf_116);
                        ex << "MAU configuration is only allowed for a CP running with MAUS.";
                        throw ex;
                    }

                       char cpside = checkCpSide(side);
                       // Set CP side
                       found = hwcinfo.setSide(cpside);
                       if (found == false)
                       {
                         Exception ex(Exception::e_cpboardnotfound_4);
                         ex << "CP board not found.";
                         throw ex;
                       }

                    Mauconfig maus(cpid);
                    // List MAU items
                    if (options.test(e_list))
                    {
                    	done = true;
                        maus.listCPMAUConfiguration(cpside,true);
                    }

                    // Choose a configuration
                    if (options.test(e_cfg))
                    {
                    	 done = true;
                    	 int cfgnum = -1;
                    	 try
                    	 {
                    	     cfgnum = boost::lexical_cast<int>(mauconfig);
                    	 }
                    	 catch (std::exception&)
                    	 {
                    	     cfgnum = -1;
                    	 }
                    	 maus.selectCPMAUConfiguration(cfgnum,cpside,true);
                    }

                    // Print version
                    if (options.test(e_version))
                    {
                    	done = true;
                    	version = Common::toUpper(version);
                    	if(version == s_CUR || version == s_BAK)
                    	{
                    		maus.printCPMAUVersion(version,cpside,true);
                    	}
                    	else if(version == s_NEW){
                    		Exception ex(Exception::e_illegaldumpversion_9);
                    		ex << "Not Supported.";
                    		throw ex;
                    	}
                    	else
                    	{
                    		Exception ex(Exception::e_illegaldumpversion_9);
                    		ex << "Illegal dump version '" << version << "'.";
                    		throw ex;
                    	}

                    }

                    if (!done)
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }

            	}
            	else
            	{
                   // Memory configuration
                   if (argc != i)
                   {
                       cerr << s_INCORRECTUSAGE << endl;
                       usage();
                       return Exception::e_usage_2;
                   }
                   // HWC information
                   HWCInfo hwcinfo;
                   bool found;
                   if (options.test(e_cpid) && !options.test(e_cpname))
                   {
                      // Set CP identity
                      found = hwcinfo.setId(cpid);
                      if (found == false) {
                          Exception ex(Exception::e_cp_not_defined_118);
                          ex << "CP identity not found.";
                          throw ex;
                      }
                   }
                   else if (options.test(e_cpname) && !options.test(e_cpid))
                   {
                      // Set CP name
                      found = hwcinfo.setName(cpname);
                      if (found == false) {
                          Exception ex(Exception::e_cp_not_defined_118);
                          ex << "CP name not found.";
                          throw ex;
                      }
                   }
                   else if (!options.test(e_cpid) && !options.test(e_cpname))
                   {
                       Exception ex(Exception::e_illopt_sysconf_116);
                       ex << "CP name or identity must be specified for a multiple CP system.";
                       throw ex;
                   }
                   else
                   {
                      cerr << s_INCORRECTUSAGE << endl;
                      usage();
                      return Exception::e_usage_2;
                   }
                   char cpside = checkCpSide(side);
                   // Set CP side
                   found = hwcinfo.setSide(cpside);
                   if (found == false) {
                       Exception ex(Exception::e_cpboardnotfound_4);
                       ex << "CP board not found.";
                       throw ex;
                   }
                   Dumps<Common::e_multicp>::memoryConfig(hwcinfo);
            	}
            }
            else if (options.test(e_version) && !options.test(e_mau) && !options.test(e_memory))
            {
                // Print configurations
                if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                // HWC information
                HWCInfo hwcinfo;
                bool found;
                if (options.test(e_cpid) && !options.test(e_cpname))
                {
                    // Set CP identity
                    found = hwcinfo.setId(cpid);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP identity not found.";
                        throw ex;
                    }
                }
                else if (options.test(e_cpname) && !options.test(e_cpid))
                {
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                    if (found == false) {
                        Exception ex(Exception::e_cp_not_defined_118);
                        ex << "CP name not found.";
                        throw ex;
                    }
                }
                else if (!options.test(e_cpid) && !options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP name or identity must be specified for a multiple CP system.";
                    throw ex;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                if (!options.test(e_cpside))
                {
                    // If side not given, check if it's a double sided CP,
                    // or single sided CP and call different print methods.
                    CPID cpId = hwcinfo.getCpId();
                    unsigned short sysType = 1000 * (cpId / 1000);
                    switch (sysType) {
                    case ACS_CS_API_HWC_NS::SysType_BC:
                        // Single sided machine,
                        // then call printConfigurations()
                        // Set CP side to 0 since this is a single side CP.
                        found = hwcinfo.setSide(0);
                        if (found == false) {
                            Exception ex(Exception::e_cpboardnotfound_4);
                            ex << "CP board not found.";
                            throw ex;
                        }
                        Dumps<Common::e_multicp>::printConfigurations(hwcinfo,
                                checkDumpVersion(version));
                        break;
                    case ACS_CS_API_HWC_NS::SysType_CP:
                        // Double sided machine,
                        // then call printDoubleSidedConfigurations()
                        Dumps<Common::e_multicp>::printDoubleSidedConfigurations(
                                hwcinfo, checkDumpVersion(version));
                        break;
                    default:
                        Exception ex(Exception::e_internal_29);
                        ex << "Illegal CP identity " << cpId << ".";
                        throw ex;
                    } // end of switch
                }
                else
                {
                    // This is the original code for printing one side, i.e.
                    // -s <side> was given so use print method for one side only...
                    char cpside = checkCpSide(side);
                    // Set CP side
                    found = hwcinfo.setSide(cpside);
                    if (found == false) {
                        Exception ex(Exception::e_cpboardnotfound_4);
                        ex << "CP board not found.";
                        throw ex;
                    }
                    Dumps<Common::e_multicp>::printConfigurations(hwcinfo,
                            checkDumpVersion(version));
                }
            } // end of if options.test(e_version)
            else if (options.test(e_blademem))
            {
                // Blade memory configuration
                if (argc != i)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                // HWC information
                HWCInfo hwcinfo;
                bool found;
                const string& _cpname = boost::to_upper_copy(cpname);
                if (options.test(e_cpname))
                {
                    if ((_cpname == "CP1") || (_cpname == "CP2"))
                    {
                        cerr << cpname << " is not supported. Valid range is bc0 - bc63." << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    // Set CP name
                    found = hwcinfo.setName(cpname);
                    if (found == false) {
                        Exception ex(Exception::e_noconf_cp_or_side_16);
                        ex << cpname << " name not found.";
                        throw ex;
                    }

                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }

                Dumps<Common::e_multicp>::bladeMemoryConfig(hwcinfo);
            }
            else if (options.test(e_help))
            {
                // Help
                if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                usage(true);
            }
            else if (options.test(e_mau) || options.test(e_upgrade_mau) || options.test(e_backup_mau))
            {
                bool done = false;
                HWCInfo hwcinfo;       // HWC information
                bool found;
                int cpid;
                int apside;
                // Print command format
                if (argc != i)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }

                if (!options.test(e_cpname) && !options.test(e_list))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex << "CP name must be specified for a multiple CP system.";
                    throw ex;
                }

                const string& _cpname = boost::to_upper_copy(cpname);
                if ((_cpname != "CP1") && (_cpname != "CP2") && !options.test(e_list))
                {
                	cerr << cpname << " is not supported." << endl;
                	usage();
                	return Exception::e_usage_2;
                }
                // Set CP name
                found = hwcinfo.setName(cpname);
                if (found == false && !options.test(e_list)) {
                	Exception ex(Exception::e_noconf_cp_or_side_16);
                	ex << cpname << " name not found.";
                	throw ex;
                }

                // This is old code, "-l -m" does not need "-cp cpname"
                if (_cpname == "CP1" || options.test(e_list))
                {
                	cpid = 1;
                }
                else
                {
                	cpid = 2;
                }

                // Check if MAUS is running
                //if (!Common::isMAUS(1000 + cpid))
                if (!Common::isMAUS(1001) && !Common::isMAUS(1002))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex << "MAU configuration is only allowed for a CP running with MAUS.";
                    throw ex;
                }

                // List MAU items
                if (options.test(e_list))
                {
                    if (options.test(e_cpname))
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
                    done = true;
                    Mauconfig maus(cpid);
                    maus.listMAUConfiguration();
                }

                // Choose a configuration
                if (options.test(e_cfg))
                {
                    done = true;
                    int cfgnum = -1;
                    Mauconfig maus(cpid);
                    try
                    {
                       cfgnum = boost::lexical_cast<int>(mauconfig);
                    }
                    catch (std::exception&)
                    {
                       cfgnum = -1;
                    }

                     maus.selectMAUConfiguration(cfgnum);
                }

                // Print version
                if (options.test(e_version))
                {
                    if (options.test(e_cpside))
                    {
                     	cerr << s_INCORRECTUSAGE << endl;
                    	usage();
                    	return Exception::e_usage_2;
                    }

                    done = true;
                    DumpsBase::t_dumpversion type = checkDumpVersion(version);
                    Mauconfig maus(cpid);
                    switch (type)
                    {
                        case DumpsBase::e_bak:
                           maus.displayBakSetting();
                           break;
                        case DumpsBase::e_cur:
                           maus.displayCurSetting();
                           break;
                        case DumpsBase::e_new:
                           maus.displayNewSetting();
                           break;
                        case DumpsBase::e_all:
                           maus.displayAllSetting();
                           break;
                    }
                }

                // Install new or back to previous version
                if (options.test(e_upgrade_mau) || options.test(e_backup_mau))
                {
                    if (!options.test(e_cpside))
                    {
                     	cerr << s_INCORRECTUSAGE << endl;
                    	usage();
                    	return Exception::e_usage_2;
                    }

                    done = true;
                    const string& _apside = boost::to_upper_copy(side);
                    if ((_apside != "APA") && (_apside != "APB"))
                    {
                    	//cerr << side << " is not supported." << endl;
                    	//usage();
                    	cerr << "Illegal AP side '" << side << "'" << endl;
                    	return Exception::e_usage_2;
                    }
                    apside = _apside == "APA" ? 0 : 1;

                    Mauconfig maus(cpid);
                    if (options.test(e_upgrade_mau))
                    {
                        maus.installNewConfiguration(apside);
                    }
                    else
                    {
                        maus.installBakConfiguration(apside);
                    }
                }

                if (!done)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else
            {
                cerr << s_INCORRECTUSAGE << endl;
                usage();
                return Exception::e_usage_2;
            }
        }
        else
        {
            // One CP system
        	// -------------
            if (options.test(e_install))
            {
                // Install package
                if (argc != i + 1) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                switch (checkDumpType(dumptype))
                {
                case DumpsBase::e_cphw:
                    Dumps<Common::e_onecp>::installCPHW(argv[i]);
                    cout << "CPHW dump was installed." << endl;
                    break;
                case DumpsBase::e_pes:
                    Dumps<Common::e_onecp>::installPES(argv[i]);
                    cout << "PES dump was installed." << endl;
                    break;
                default:
                    throw Exception(Exception::e_internal_29,
                            "Illegal dump type.");
                }
            }
            else if(options.test(e_modify_patches))
            {
		// apply patches in Single CP 
                 if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }


                 if (options.test(e_cpname))
                 {
                      Exception ex(Exception::e_illopt_sysconf_116);
                      ex << "CP name must not be specified for a one CP system.";
                      throw ex;
                 }

                 if (options.test(e_cpid))
                 {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP identity must not be specified for a one CP system.";
                    throw ex;
                 }


                if(!options.test(e_cpside))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP side must be specified for a Dual sided CP";
                    throw ex;

                }
                checkCpSide(side);
                DumpsBase::modifyPatches(side, patches);
            }
            else if(options.test(e_list_patches))
            {

                  if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }

		// list patches in single CP 
                 if (options.test(e_cpname))
                 {
                      Exception ex(Exception::e_illopt_sysconf_116);
                      ex << "CP name must not be specified for a one CP system.";
                      throw ex;
                 }

                 if (options.test(e_cpid))
                 {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP identity must not be specified for a one CP system.";
                    throw ex;
                 }

                if(!options.test(e_cpside))
                {
                 side = "a";
		DumpsBase::listPatches(side);
                 side = "b";
		DumpsBase::listPatches(side);
                }
                else
                {
                 checkCpSide(side);
		 DumpsBase::listPatches(side); 
                }
            }

            else if ((options.test(e_upgrade) || options.test(e_backup)))
            {
                // Configure dump
                if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                if (options.test(e_conftype))
                {
                    if (conftype != s_AUT)
                    {
                        cerr << "Illegal configuration type '" << conftype
                                << "'." << endl;
                        cerr << "Warning: Option '-t' is deprecated." << endl;
                        usage();
                        Exception ex(Exception::e_usage_2);
                        throw ex;
                    }
                }
                // Dump version
                DumpsBase::t_dumpversion dumpver;
                if (options.test(e_backup))
                {
                    dumpver = DumpsBase::e_bak;
                } else if (options.test(e_upgrade))
                {
                    dumpver = DumpsBase::e_new;
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                if (options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex << "CP name must not be specified for a one CP system.";
                    throw ex;
                }
                if (options.test(e_cpid))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP identity must not be specified for a one CP system.";
                    throw ex;
                }
                // CP side
                char cpside;
                if (options.test(e_cpside))
                {
                       cpside = checkCpSide(side);
                }
                else
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                // Dump type
                switch (checkDumpType(dumptype))
                {
                case DumpsBase::e_cphw:
                    Dumps<Common::e_onecp>::configCPHW(cpside, dumpver);
                    break;
                case DumpsBase::e_pes:
                    Dumps<Common::e_onecp>::configPES(cpside, dumpver);
                    break;
                default:
                    throw Exception(Exception::e_internal_29,
                            "Illegal dump type.");
                }
                // Fix TR HR57367
                if(!options.test(e_lab))
                {
                   Common::removeDebugFile();
                }

            }
            else if (options.test(e_remove))
            {
                Exception ex(Exception::e_illopt_sysconf_116);
                ex << "Configuration cannot be removed for a one CP system.";
                throw ex;
            }
            else if (options.test(e_memory))
            {
            	bool forcpmau=false;
            	if(options.test(e_cfg) || options.test(e_list) || options.test(e_version))
            	{
            		forcpmau = true;
            	}
            	if(forcpmau)
            	{
                    bool done = false;
                    bool found;
                    int cpid = 1;
                    // Print command format
                    if (argc != i)
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }

                    if (options.test(e_cpname))
                    {
                        Exception ex(Exception::e_illopt_sysconf_116);
                        ex << "CP name must not be specified for a one CP system.";
                        throw ex;
                    }

                    if (!options.test(e_cpside))
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }

                    // Check if MAUS is running
                    if (!Common::isMAUS(1001))
                    {
                        Exception ex(Exception::e_illopt_sysconf_116);
                        ex << "MAU configuration is only allowed for a CP running with MAUS.";
                        throw ex;
                    }
                       char cpside = checkCpSide(side);
                       string cpboard = boost::lexical_cast<string>(cpside);
                       if (cpboard.empty())
                       {
                         Exception ex(Exception::e_cpboardnotfound_4);
                         ex << "CP board not found.";
                         throw ex;
                       }

                    Mauconfig maus(cpid);
                    // List MAU items
                    if (options.test(e_list))
                    {
                    	done = true;
                        maus.listCPMAUConfiguration(cpside,false);
                    }

                    // Choose a configuration
                    if (options.test(e_cfg))
                    {
                    	 done = true;
                    	 int cfgnum = -1;
                    	 try
                    	 {
                    	     cfgnum = boost::lexical_cast<int>(mauconfig);
                    	 }
                    	 catch (std::exception&)
                    	 {
                    	     cfgnum = -1;
                    	 }
                    	 maus.selectCPMAUConfiguration(cfgnum,cpside,false);
                    }

                    // Print version
                    if (options.test(e_version))
                    {
                    	done = true;
                    	version = Common::toUpper(version);
                    	if(version == s_CUR || version == s_BAK)
                    	{
                    		maus.printCPMAUVersion(version,cpside,false);
                    	}
                    	else if(version == s_NEW){
                    		Exception ex(Exception::e_illegaldumpversion_9);
                    		ex << "Not Supported.";
                    		throw ex;
                    	}
                    	else
                    	{
                    		Exception ex(Exception::e_illegaldumpversion_9);
                    		ex << "Illegal dump version '" << version << "'.";
                    		throw ex;
                    	}

                    }
                    if (!done)
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }
            	}
            	else
            	{
                   // Memory configuration
                   if (argc != i)
                   {
                       cerr << s_INCORRECTUSAGE << endl;
                       usage();
                       return Exception::e_usage_2;
                   }
                   if (options.test(e_cpname))
                   {
                       Exception ex(Exception::e_illopt_sysconf_116);
                       ex << "CP name must not be specified for a one CP system.";
                       throw ex;
                   }
                   if (options.test(e_cpid))
                   {
                       Exception ex(Exception::e_illopt_sysconf_116);
                       ex
                       << "CP identity must not be specified for a one CP system.";
                       throw ex;
                   }
                   // CP side
                   char cpside;
                   if (options.test(e_cpside))
                   {
                       cpside = checkCpSide(side);
                   }
                   else
                   {
                       cerr << s_INCORRECTUSAGE << endl;
                       usage();
                       return Exception::e_usage_2;
                   }
                   Dumps<Common::e_onecp>::memoryConfig(cpside);
            	}
            }
            else if (options.test(e_version) && !options.test(e_mau) && !options.test(e_memory))
            {
                // Print configurations
                if (argc != i)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                if (options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex << "CP name must not be specified for a one CP system.";
                    throw ex;
                }
                if (options.test(e_cpid))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex
                    << "CP identity must not be specified for a one CP system.";
                    throw ex;
                }
                //       Dumps<Common::e_onecp>::printConfigurations(checkDumpVersion(version));
                char cpside;
                if (options.test(e_cpside)) //if CPside is mentioned
                {
                    cpside = checkCpSide(side);
                    Dumps<Common::e_onecp>::printConfigurations(
                            checkDumpVersion(version), cpside);
                }
                else
                {
                    cpside = '\0';
                    Dumps<Common::e_onecp>::printConfigurations(
                            checkDumpVersion(version), cpside);
                }
            }
            else if (options.test(e_blademem))
            {
                // Blade memory configuration
                Exception ex(Exception::e_illopt_sysconf_116);
                ex
                << "The command just supports on Multi CP system.";
                throw ex;
            }
            else if (options.test(e_help))
            {
                // Help
                if (argc != i) {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
                usage(true);
            }
            else if (options.test(e_mau) || options.test(e_upgrade_mau) || options.test(e_backup_mau))
            {
            	bool done = false;
                int cpid = 1;
                int apside;
                // Print command format
                if (argc != i)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }

                if (options.test(e_cpname))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex << "CP name must not be specified for a one CP system.";
                    throw ex;
                }

                // Check if MAUS is running
                if (!Common::isMAUS(1001))
                {
                    Exception ex(Exception::e_illopt_sysconf_116);
                    ex << "MAU configuration is only allowed for a CP running with MAUS.";
                    throw ex;
                }

                // List MAU items
                if (options.test(e_list))
                {
                    done = true;
                    Mauconfig maus(cpid);
                    maus.listMAUConfiguration();
                }

                // Choose a configuration
                if (options.test(e_cfg))
                {
                    done = true;
                    int cfgnum = -1;
                    Mauconfig maus(cpid);
                    try
                    {
                       cfgnum = boost::lexical_cast<int>(mauconfig);
                    }
                    catch (std::exception&)
                    {
                       cfgnum = -1;
                    }

                     maus.selectMAUConfiguration(cfgnum);
                }

                // Print version
                if (options.test(e_version))
                {
                    if (options.test(e_cpside))
                    {
                     	cerr << s_INCORRECTUSAGE << endl;
                    	usage();
                    	return Exception::e_usage_2;
                    }

                    done = true;
                    DumpsBase::t_dumpversion type = checkDumpVersion(version);
                    Mauconfig maus(cpid);
                    switch (type)
                    {
                        case DumpsBase::e_bak:
                           maus.displayBakSetting();
                           break;
                        case DumpsBase::e_cur:
                           maus.displayCurSetting();
                           break;
                        case DumpsBase::e_new:
                           maus.displayNewSetting();
                           break;
                        case DumpsBase::e_all:
                           maus.displayAllSetting();
                           break;
                    }
                }

                // Install new or back to previous version
                if (options.test(e_upgrade_mau) || options.test(e_backup_mau))
                {
                    if (!options.test(e_cpside))
                    {
                        cerr << s_INCORRECTUSAGE << endl;
                        usage();
                        return Exception::e_usage_2;
                    }

                    done = true;
                    const string& _apside = boost::to_upper_copy(side);
                    if ((_apside != "APA") && (_apside != "APB"))
                    {
                    	//cerr << side << " is not supported." << endl;
                    	//usage();
                    	cerr << "Illegal AP side '" << side << "'" << endl;
                    	return Exception::e_usage_2;
                    }
                    apside = _apside == "APA" ? 0 : 1;

                    Mauconfig maus(cpid);
                    if (options.test(e_upgrade_mau))
                    {
                        maus.installNewConfiguration(apside);
                    }
                    else
                    {
                        maus.installBakConfiguration(apside);
                    }
                }

                if (!done)
                {
                    cerr << s_INCORRECTUSAGE << endl;
                    usage();
                    return Exception::e_usage_2;
                }
            }
            else
            {
                cerr << s_INCORRECTUSAGE << endl;
                usage();
                return Exception::e_usage_2;
            }
        }
    }
    catch (Exception& ex)
    {
        cerr << ex << endl;
        return ex.errorCode();
    }
    catch (...)
    {
        Exception ex(Exception::e_system_28);
        ex << "CS API is missing.";
        cerr << ex << endl;
        return ex.errorCode();
    }
    return Exception::e_ok_0;
}
