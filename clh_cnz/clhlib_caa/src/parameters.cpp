//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      parametes.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013, 2014. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for handling log parameters.
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
//      PA1    2013-01-24   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "parameters.h"
#include "common.h"
#include "ltime.h"
#include <string.h>

using namespace std;
using namespace boost;

namespace PES_CLH {

const string s_datepattern = "_\\d{8}_\\d{6}";
const string s_extdatepattern = "_\\d{8}_\\d{6}_\\d{6}";
const string s_xmpattern =   "_\\d{4}";
const uint16_t s_apzCpub = 1<<e_apz21255 | 1<<e_apz21260 | 1<<e_apz21401 | 1<<e_apz21403 | 1<<e_apz21410;
const uint16_t s_apzAll = 1<<e_apz21240 | 1<<e_apz21250 | s_apzCpub | 1<<e_apz21270 | 1<<e_apz21420;
const uint16_t s_apzForRP = 1<<e_apz21250 | 1<<e_apz21255 | 1<<e_apz21260;
const uint16_t s_apzMaus = 1<<e_apz21255 | 1<<e_apz21260 | 1<<e_apz21270;

//========================================================================================
// Class BaseParameters
//========================================================================================

const uint32_t BaseParameters::s_maxfilesize = 100000;
fs::path BaseParameters::s_apzlogspath;
fs::path BaseParameters::s_cpslogspath;

//-----------------------------------------------------------------------------------------
// Get APZ logs path
//-----------------------------------------------------------------------------------------
fs::path BaseParameters::getApzLogsPath()
{
   if (s_apzlogspath.empty())
   {
      const std::string& logicalName = CPTable::isMultiCPSystem()? "APZ_LOGS": "APZ_DATA";
      s_apzlogspath = Common::getDataDiskPath(logicalName);
   }
   return s_apzlogspath;
}

//-----------------------------------------------------------------------------------------
// Get CPS logs path
//-----------------------------------------------------------------------------------------
fs::path BaseParameters::getCpsLogsPath()
{
   if (s_cpslogspath.empty())
   {
      s_cpslogspath = Common::getDataDiskPath("CPS_LOGS");

   }
   return s_cpslogspath;
}

//========================================================================================
// Class Parameters<e_error>
//========================================================================================

// Parameters
template<> const string Parameters<e_error>::s_logname = "ERROR";
template<> const t_loggroup Parameters<e_error>::s_loggroup = e_lgCP;
template<> const string Parameters<e_error>::s_shortname = s_logname;
template<> const string Parameters<e_error>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_error>::s_path = "pes/error";
template<> const string Parameters<e_error>::s_fileprefix = "ErrorLog";
template<> const string Parameters<e_error>::s_fileext = ".log";
template<> const regex Parameters<e_error>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_error>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_error>::s_apzset = s_apzAll;
template<> const t_headertype Parameters<e_error>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_error>::s_hasmsgno = true;
template<> const bool Parameters<e_error>::s_hasxmno = false;
template<> uintmax_t Parameters<e_error>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_error>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_error>::s_divider = 50;
template<> int Parameters<e_error>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_event>
//========================================================================================

// Parameters
template<> const string Parameters<e_event>::s_logname = "EVENT";
template<> const t_loggroup Parameters<e_event>::s_loggroup = e_lgCP;
template<> const string Parameters<e_event>::s_shortname = s_logname;
template<> const string Parameters<e_event>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_event>::s_path = "pes/event";
template<> const string Parameters<e_event>::s_fileprefix = "EventLog";
template<> const string Parameters<e_event>::s_fileext = ".log";
template<> const regex Parameters<e_event>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_event>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_event>::s_apzset = s_apzAll;
template<> const t_headertype Parameters<e_event>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_event>::s_hasmsgno = true;
template<> const bool Parameters<e_event>::s_hasxmno = false;
template<> uintmax_t Parameters<e_event>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_event>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_event>::s_divider = 50;
template<> int Parameters<e_event>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_syslog>
//========================================================================================

// Parameters
template<> const string Parameters<e_syslog>::s_logname = "SYSLOG";
template<> const t_loggroup Parameters<e_syslog>::s_loggroup = e_lgCP;
template<> const string Parameters<e_syslog>::s_shortname = "SYS";
template<> const string Parameters<e_syslog>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_syslog>::s_path = "cphw/syslog";
template<> const string Parameters<e_syslog>::s_fileprefix = "Syslog";
template<> const string Parameters<e_syslog>::s_fileext = ".log";
template<> const regex Parameters<e_syslog>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_syslog>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_syslog>::s_apzset = 1<<e_apz21240 | s_apzCpub| 1<<e_apz21270 | 1<<e_apz21420;
template<> const t_headertype Parameters<e_syslog>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_syslog>::s_hasmsgno = false;
template<> const bool Parameters<e_syslog>::s_hasxmno = false;
template<> uintmax_t Parameters<e_syslog>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_syslog>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_syslog>::s_divider = 50;
template<> int Parameters<e_syslog>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_binlog>
//========================================================================================

// Parameters
template<> const string Parameters<e_binlog>::s_logname = "BINLOG";
template<> const t_loggroup Parameters<e_binlog>::s_loggroup = e_lgCP;
template<> const string Parameters<e_binlog>::s_shortname = "BIN";
template<> const string Parameters<e_binlog>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_binlog>::s_path = "cphw/binlog";
template<> const string Parameters<e_binlog>::s_fileprefix = "Binlog";
template<> const string Parameters<e_binlog>::s_fileext = ".log";
template<> const regex Parameters<e_binlog>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_binlog>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_binlog>::s_apzset = 1<<e_apz21240;
template<> const t_headertype Parameters<e_binlog>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_binlog>::s_hasmsgno = false;
template<> const bool Parameters<e_binlog>::s_hasxmno = false;
template<> uintmax_t Parameters<e_binlog>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_binlog>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_binlog>::s_divider = 50;
template<> int Parameters<e_binlog>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_corecpbb>
//========================================================================================

// Parameters
template<> const string Parameters<e_corecpbb>::s_logname = "CORECPBB";
template<> const t_loggroup Parameters<e_corecpbb>::s_loggroup = e_lgCP;
template<> const string Parameters<e_corecpbb>::s_shortname = "CORE";
template<> const string Parameters<e_corecpbb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_corecpbb>::s_path = "cphw/core";
template<> const string Parameters<e_corecpbb>::s_fileprefix = "Apz_vmCore";
template<> const string Parameters<e_corecpbb>::s_fileext = ".log";
template<> const regex Parameters<e_corecpbb>::s_logfile(
      s_fileprefix + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_corecpbb>::s_tempfile(
      s_fileprefix + "\\.tmp"
      );
template<> const uint16_t Parameters<e_corecpbb>::s_apzset = 1<<e_apz21240;
template<> const t_headertype Parameters<e_corecpbb>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_corecpbb>::s_hasmsgno = false;
template<> const bool Parameters<e_corecpbb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_corecpbb>::s_maxsize = 500000000;
template<> uint64_t Parameters<e_corecpbb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_corecpbb>::s_divider = 50;
template<> int Parameters<e_corecpbb>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_corecpsb>
//========================================================================================

// Parameters
template<> const string Parameters<e_corecpsb>::s_logname = "CORECPSB";
template<> const t_loggroup Parameters<e_corecpsb>::s_loggroup = e_lgCP;
template<> const string Parameters<e_corecpsb>::s_shortname = "CORE";
template<> const string Parameters<e_corecpsb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_corecpsb>::s_path = "cphw/core/cpsb";
template<> const string Parameters<e_corecpsb>::s_fileprefix = "Apz_vmCore";
template<> const string Parameters<e_corecpsb>::s_fileext = ".log";
template<> const regex Parameters<e_corecpsb>::s_logfile(
      s_fileprefix + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_corecpsb>::s_tempfile(
      s_fileprefix + "\\.tmp"
      );
template<> const uint16_t Parameters<e_corecpsb>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_corecpsb>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_corecpsb>::s_hasmsgno = false;
template<> const bool Parameters<e_corecpsb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_corecpsb>::s_maxsize = 500000000;
template<> uint64_t Parameters<e_corecpsb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_corecpsb>::s_divider = 50;
template<> int Parameters<e_corecpsb>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_corepcih>
//========================================================================================

// Parameters
template<> const string Parameters<e_corepcih>::s_logname = "COREPCIH";
template<> const t_loggroup Parameters<e_corepcih>::s_loggroup = e_lgCP;
template<> const string Parameters<e_corepcih>::s_shortname = "CORE";
template<> const string Parameters<e_corepcih>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_corepcih>::s_path = "cphw/core/pcih";
template<> const string Parameters<e_corepcih>::s_fileprefix =
      "(Apz_vmCore|pcihsrv_tick|pcihsrv_stat)";
template<> const string Parameters<e_corepcih>::s_fileext = ".log";
template<> const regex Parameters<e_corepcih>::s_logfile(
      s_fileprefix + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_corepcih>::s_tempfile(
      s_fileprefix + "\\.tmp"
      );
template<> const uint16_t Parameters<e_corepcih>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_corepcih>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_corepcih>::s_hasmsgno = false;
template<> const bool Parameters<e_corepcih>::s_hasxmno = false;
template<> uintmax_t Parameters<e_corepcih>::s_maxsize = 500000000;
template<> uint64_t Parameters<e_corepcih>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_corepcih>::s_divider = 50;
template<> int Parameters<e_corepcih>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_corecpub>
//========================================================================================

// Parameters
template<> const string Parameters<e_corecpub>::s_logname = "CORECPU";
template<> const t_loggroup Parameters<e_corecpub>::s_loggroup = e_lgCP;
template<> const string Parameters<e_corecpub>::s_shortname = "CORE";
template<> const string Parameters<e_corecpub>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_corecpub>::s_path = "cphw/core";
template<> const string Parameters<e_corecpub>::s_fileprefix = "Apz_vmCore";
template<> const string Parameters<e_corecpub>::s_fileext = ".log";
template<> const regex Parameters<e_corecpub>::s_logfile(
      s_fileprefix + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_corecpub>::s_tempfile(
      s_fileprefix + "\\.tmp"
      );
template<> const uint16_t Parameters<e_corecpub>::s_apzset = s_apzCpub | 1<<e_apz21270 | 1<<e_apz21420;
template<> const t_headertype Parameters<e_corecpub>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_corecpub>::s_hasmsgno = false;
template<> const bool Parameters<e_corecpub>::s_hasxmno = false;
template<> uintmax_t Parameters<e_corecpub>::s_maxsize = 500000000;
template<> uint64_t Parameters<e_corecpub>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_corecpub>::s_divider = 50;
template<> int Parameters<e_corecpub>::s_noEP = 0;

//========================================================================================
//	Class Parameters<e_crashcpbb>
//========================================================================================

// Parameters
template<> const string Parameters<e_crashcpbb>::s_logname = "CRASHCPBB";
template<> const t_loggroup Parameters<e_crashcpbb>::s_loggroup = e_lgCP;
template<> const string Parameters<e_crashcpbb>::s_shortname = "CRASH";
template<> const string Parameters<e_crashcpbb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_crashcpbb>::s_path = "cphw/crash";
template<> const string Parameters<e_crashcpbb>::s_fileprefix = "Tru64_Crash";
template<> const string Parameters<e_crashcpbb>::s_fileext = "";
template<> const regex Parameters<e_crashcpbb>::s_logfile(s_fileprefix + s_datepattern);
template<> const regex Parameters<e_crashcpbb>::s_tempfile(
      "bounds|msgbuf\\.savecore|evm\\.buf|vmz?core(\\.\\d)?"
      );
template<> const uint16_t Parameters<e_crashcpbb>::s_apzset = 1<<e_apz21240;
template<> const t_headertype Parameters<e_crashcpbb>::s_headertype = e_noHeader;
template<> const bool Parameters<e_crashcpbb>::s_hasmsgno = false;
template<> const bool Parameters<e_crashcpbb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_crashcpbb>::s_maxsize = 1000000000;
template<> uint64_t Parameters<e_crashcpbb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_crashcpbb>::s_divider = 50;
template<> int Parameters<e_crashcpbb>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_crashcpsb>
//========================================================================================

// Parameters
template<> const string Parameters<e_crashcpsb>::s_logname = "CRASHCPSB";
template<> const t_loggroup Parameters<e_crashcpsb>::s_loggroup = e_lgCP;
template<> const string Parameters<e_crashcpsb>::s_shortname = "CRASH";
template<> const string Parameters<e_crashcpsb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_crashcpsb>::s_path = "cphw/crash/cpsb";
template<> const string Parameters<e_crashcpsb>::s_fileprefix = "IA64_Crash";
template<> const string Parameters<e_crashcpsb>::s_fileext = "";
template<> const regex Parameters<e_crashcpsb>::s_logfile(s_fileprefix + s_datepattern);
template<> const regex Parameters<e_crashcpsb>::s_tempfile("cpsb[ab]_\\d{12}");
template<> const uint16_t Parameters<e_crashcpsb>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_crashcpsb>::s_headertype = e_noHeader;
template<> const bool Parameters<e_crashcpsb>::s_hasmsgno = false;
template<> const bool Parameters<e_crashcpsb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_crashcpsb>::s_maxsize = 1000000000;
template<> uint64_t Parameters<e_crashcpsb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_crashcpsb>::s_divider = 50;
template<> int Parameters<e_crashcpsb>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_crashpcih>
//========================================================================================

// Parameters
template<> const string Parameters<e_crashpcih>::s_logname = "CRASHPCIH";
template<> const t_loggroup Parameters<e_crashpcih>::s_loggroup = e_lgCP;
template<> const string Parameters<e_crashpcih>::s_shortname = "CRASH";
template<> const string Parameters<e_crashpcih>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_crashpcih>::s_path = "cphw/crash/pcih";
template<> const string Parameters<e_crashpcih>::s_fileprefix = "IA32_Crash";
template<> const string Parameters<e_crashpcih>::s_fileext = "";
template<> const regex Parameters<e_crashpcih>::s_logfile(s_fileprefix + s_datepattern);
template<> const regex Parameters<e_crashpcih>::s_tempfile("pcih[ab]_\\d{12}");
template<> const uint16_t Parameters<e_crashpcih>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_crashpcih>::s_headertype = e_noHeader;
template<> const bool Parameters<e_crashpcih>::s_hasmsgno = false;
template<> const bool Parameters<e_crashpcih>::s_hasxmno = false;
template<> uintmax_t Parameters<e_crashpcih>::s_maxsize = 1000000000;
template<> uint64_t Parameters<e_crashpcih>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_crashpcih>::s_divider = 50;
template<> int Parameters<e_crashpcih>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_crashcpub>
//========================================================================================

// Parameters
template<> const string Parameters<e_crashcpub>::s_logname = "CRASHCPU";
template<> const t_loggroup Parameters<e_crashcpub>::s_loggroup = e_lgCP;
template<> const string Parameters<e_crashcpub>::s_shortname = "CRASH";
template<> const string Parameters<e_crashcpub>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_crashcpub>::s_path = "cphw/crash";
template<> const string Parameters<e_crashcpub>::s_fileprefix = "x86_64_Crash";
template<> const string Parameters<e_crashcpub>::s_fileext = ".gz";
template<> const regex Parameters<e_crashcpub>::s_logfile(
      s_fileprefix + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_crashcpub>::s_tempfile(
      "dump_file" + s_datepattern + "\\" + s_fileext
      );
template<> const uint16_t Parameters<e_crashcpub>::s_apzset = s_apzCpub | 1<<e_apz21270 | 1<<e_apz21420;
template<> const t_headertype Parameters<e_crashcpub>::s_headertype = e_noHeader;
template<> const bool Parameters<e_crashcpub>::s_hasmsgno = false;
template<> const bool Parameters<e_crashcpub>::s_hasxmno = false;
template<> uintmax_t Parameters<e_crashcpub>::s_maxsize = 1000000000;
template<> uint64_t Parameters<e_crashcpub>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_crashcpub>::s_divider = 50;
template<> int Parameters<e_crashcpub>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_evlogcpsb>
//========================================================================================

// Parameters
template<> const string Parameters<e_evlogcpsb>::s_logname = "EVLOGCPSB";
template<> const t_loggroup Parameters<e_evlogcpsb>::s_loggroup = e_lgCP;
template<> const string Parameters<e_evlogcpsb>::s_shortname = "EVLOG";
template<> const string Parameters<e_evlogcpsb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_evlogcpsb>::s_path = "cphw/evlog/cpsb";
template<> const string Parameters<e_evlogcpsb>::s_fileprefix = "Evlog";
template<> const string Parameters<e_evlogcpsb>::s_fileext = ".log";
template<> const regex Parameters<e_evlogcpsb>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_evlogcpsb>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_evlogcpsb>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_evlogcpsb>::s_headertype = e_noHeader;
template<> const bool Parameters<e_evlogcpsb>::s_hasmsgno = false;
template<> const bool Parameters<e_evlogcpsb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_evlogcpsb>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_evlogcpsb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_evlogcpsb>::s_divider = 50;
template<> int Parameters<e_evlogcpsb>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_evlogpcih>
//========================================================================================

// Parameters
template<> const string Parameters<e_evlogpcih>::s_logname = "EVLOGPCIH";
template<> const t_loggroup Parameters<e_evlogpcih>::s_loggroup = e_lgCP;
template<> const string Parameters<e_evlogpcih>::s_shortname = "EVLOG";
template<> const string Parameters<e_evlogpcih>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_evlogpcih>::s_path = "cphw/evlog/pcih";
template<> const string Parameters<e_evlogpcih>::s_fileprefix = "Evlog";
template<> const string Parameters<e_evlogpcih>::s_fileext = ".log";
template<> const regex Parameters<e_evlogpcih>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_evlogpcih>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_evlogpcih>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_evlogpcih>::s_headertype = e_noHeader;
template<> const bool Parameters<e_evlogpcih>::s_hasmsgno = false;
template<> const bool Parameters<e_evlogpcih>::s_hasxmno = false;
template<> uintmax_t Parameters<e_evlogpcih>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_evlogpcih>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_evlogpcih>::s_divider = 50;
template<> int Parameters<e_evlogpcih>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_salinfocpsb>
//========================================================================================

// Parameters
template<> const string Parameters<e_salinfocpsb>::s_logname = "SALINFOCPSB";
template<> const t_loggroup Parameters<e_salinfocpsb>::s_loggroup = e_lgCP;
template<> const string Parameters<e_salinfocpsb>::s_shortname = "SALINFO";
template<> const string Parameters<e_salinfocpsb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_salinfocpsb>::s_path = "cphw/salinfo/cpsb";
template<> const string Parameters<e_salinfocpsb>::s_fileprefix = "salinfo";
template<> const string Parameters<e_salinfocpsb>::s_fileext = "";
template<> const regex Parameters<e_salinfocpsb>::s_logfile(s_fileprefix + s_datepattern);
template<> const regex Parameters<e_salinfocpsb>::s_tempfile("cpsb[ab]_\\d{12}");
template<> const uint16_t Parameters<e_salinfocpsb>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_salinfocpsb>::s_headertype = e_noHeader;
template<> const bool Parameters<e_salinfocpsb>::s_hasmsgno = false;
template<> const bool Parameters<e_salinfocpsb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_salinfocpsb>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_salinfocpsb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_salinfocpsb>::s_divider = 50;
template<> int Parameters<e_salinfocpsb>::s_noEP = 0;

//========================================================================================
//	Class Parameters<e_sel>
//========================================================================================

// Parameters
template<> const string Parameters<e_sel>::s_logname = "SEL";
template<> const t_loggroup Parameters<e_sel>::s_loggroup = e_lgSEL;
template<> const string Parameters<e_sel>::s_shortname = "SEL";
template<> const string Parameters<e_sel>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_sel>::s_path = "cphw/sel";
template<> const string Parameters<e_sel>::s_fileprefix = "sel";
template<> const string Parameters<e_sel>::s_fileext = ".log";
template<> const regex Parameters<e_sel>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_sel>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_sel>::s_apzset = 1<<e_apz21255 | 1<<e_apz21260;
template<> const t_headertype Parameters<e_sel>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_sel>::s_hasmsgno = false;
template<> const bool Parameters<e_sel>::s_hasxmno = false;
template<> uintmax_t Parameters<e_sel>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_sel>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_sel>::s_divider = 50;
template<> int Parameters<e_sel>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_ruf>
//========================================================================================

// Parameters
template<> const string Parameters<e_ruf>::s_logname = "RUF";
template<> const t_loggroup Parameters<e_ruf>::s_loggroup = e_lgCP;
template<> const string Parameters<e_ruf>::s_shortname = "RUF";
template<> const string Parameters<e_ruf>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_ruf>::s_path = "cphw/ruf";
template<> const string Parameters<e_ruf>::s_fileprefix = "fwrhlogfile";
template<> const string Parameters<e_ruf>::s_fileext = "";
template<> const regex Parameters<e_ruf>::s_logfile("");
template<> const regex Parameters<e_ruf>::s_tempfile(s_fileprefix);
template<> const uint16_t Parameters<e_ruf>::s_apzset = s_apzCpub;
template<> const t_headertype Parameters<e_ruf>::s_headertype = e_noHeader;
template<> const bool Parameters<e_ruf>::s_hasmsgno = false;
template<> const bool Parameters<e_ruf>::s_hasxmno = false;
template<> uintmax_t Parameters<e_ruf>::s_maxsize = 0;
template<> uint64_t Parameters<e_ruf>::s_maxtime = 0;
template<> uint16_t Parameters<e_ruf>::s_divider = 0;
template<> int Parameters<e_ruf>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_consolsrm>
//========================================================================================

// Parameters
template<> const string Parameters<e_consolsrm>::s_logname = "CONSOLSRM";
template<> const t_loggroup Parameters<e_consolsrm>::s_loggroup = e_lgCP;
template<> const string Parameters<e_consolsrm>::s_shortname = "SRM";
template<> const string Parameters<e_consolsrm>::s_commonname = "CONSOL";
template<> const fs::path Parameters<e_consolsrm>::s_path = "cpt/srm";
template<> const string Parameters<e_consolsrm>::s_fileprefix = "srm";
template<> const string Parameters<e_consolsrm>::s_fileext = ".log";
template<> const regex Parameters<e_consolsrm>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_consolsrm>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_consolsrm>::s_apzset = 1<<e_apz21240;
template<> const t_headertype Parameters<e_consolsrm>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_consolsrm>::s_hasmsgno = false;
template<> const bool Parameters<e_consolsrm>::s_hasxmno = false;
template<> uintmax_t Parameters<e_consolsrm>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_consolsrm>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_consolsrm>::s_divider = 50;
template<> int Parameters<e_consolsrm>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_consolbmc>
//========================================================================================

// Parameters
template<> const string Parameters<e_consolbmc>::s_logname = "CONSOLBMC";
template<> const t_loggroup Parameters<e_consolbmc>::s_loggroup = e_lgCP;
template<> const string Parameters<e_consolbmc>::s_shortname = "BMC";
template<> const string Parameters<e_consolbmc>::s_commonname = "CONSOL";
template<> const fs::path Parameters<e_consolbmc>::s_path = "cpt/bmc";
template<> const string Parameters<e_consolbmc>::s_fileprefix = "bmc";
template<> const string Parameters<e_consolbmc>::s_fileext = ".log";
template<> const regex Parameters<e_consolbmc>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_consolbmc>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_consolbmc>::s_apzset = 1<<e_apz21240 | 1<<e_apz21250;
template<> const t_headertype Parameters<e_consolbmc>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_consolbmc>::s_hasmsgno = false;
template<> const bool Parameters<e_consolbmc>::s_hasxmno = false;
template<> uintmax_t Parameters<e_consolbmc>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_consolbmc>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_consolbmc>::s_divider = 50;
template<> int Parameters<e_consolbmc>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_consolmp>
//========================================================================================

// Parameters
template<> const string Parameters<e_consolmp>::s_logname = "CONSOLMP";
template<> const t_loggroup Parameters<e_consolmp>::s_loggroup = e_lgCP;
template<> const string Parameters<e_consolmp>::s_shortname = "MP";
template<> const string Parameters<e_consolmp>::s_commonname = "CONSOL";
template<> const fs::path Parameters<e_consolmp>::s_path = "cpt/mp";
template<> const string Parameters<e_consolmp>::s_fileprefix = "mp";
template<> const string Parameters<e_consolmp>::s_fileext = ".log";
template<> const regex Parameters<e_consolmp>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_consolmp>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_consolmp>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_consolmp>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_consolmp>::s_hasmsgno = false;
template<> const bool Parameters<e_consolmp>::s_hasxmno = false;
template<> uintmax_t Parameters<e_consolmp>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_consolmp>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_consolmp>::s_divider = 50;
template<> int Parameters<e_consolmp>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_consolpcih>
//========================================================================================

// Parameters
template<> const string Parameters<e_consolpcih>::s_logname = "CONSOLPCIH";
template<> const t_loggroup Parameters<e_consolpcih>::s_loggroup = e_lgCP;
template<> const string Parameters<e_consolpcih>::s_shortname = "PCIH";
template<> const string Parameters<e_consolpcih>::s_commonname = "CONSOL";
template<> const fs::path Parameters<e_consolpcih>::s_path = "cpt/pcih";
template<> const string Parameters<e_consolpcih>::s_fileprefix = "pcih";
template<> const string Parameters<e_consolpcih>::s_fileext = ".log";
template<> const regex Parameters<e_consolpcih>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_consolpcih>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_consolpcih>::s_apzset = 1<<e_apz21250;
template<> const t_headertype Parameters<e_consolpcih>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_consolpcih>::s_hasmsgno = false;
template<> const bool Parameters<e_consolpcih>::s_hasxmno = false;
template<> uintmax_t Parameters<e_consolpcih>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_consolpcih>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_consolpcih>::s_divider = 50;
template<> int Parameters<e_consolpcih>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_consolsyscon>
//========================================================================================

// Parameters
template<> const string Parameters<e_consolsyscon>::s_logname = "CONSOLSYSCON";
template<> const t_loggroup Parameters<e_consolsyscon>::s_loggroup = e_lgCP;
template<> const string Parameters<e_consolsyscon>::s_shortname = "SYSCON";
template<> const string Parameters<e_consolsyscon>::s_commonname = "CONSOL";
template<> const fs::path Parameters<e_consolsyscon>::s_path = "cpt/syscon";
template<> const string Parameters<e_consolsyscon>::s_fileprefix = "syscon";
template<> const string Parameters<e_consolsyscon>::s_fileext = ".log";
template<> const regex Parameters<e_consolsyscon>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_consolsyscon>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_consolsyscon>::s_apzset = s_apzCpub | 1<<e_apz21420;
template<> const t_headertype Parameters<e_consolsyscon>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_consolsyscon>::s_hasmsgno = false;
template<> const bool Parameters<e_consolsyscon>::s_hasxmno = false;
template<> uintmax_t Parameters<e_consolsyscon>::s_maxsize = 200000000;
template<> uint64_t Parameters<e_consolsyscon>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_consolsyscon>::s_divider = 50;
template<> int Parameters<e_consolsyscon>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_xpulog>
//========================================================================================

// Parameters
template<> const string Parameters<e_xpulog>::s_logname = "XPULOG";
template<> const t_loggroup Parameters<e_xpulog>::s_loggroup = e_lgXPU;
template<> const string Parameters<e_xpulog>::s_shortname = s_logname;
template<> const string Parameters<e_xpulog>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_xpulog>::s_path = "xpu/log";
template<> const string Parameters<e_xpulog>::s_fileprefix = "XMLog";
template<> const string Parameters<e_xpulog>::s_fileext = ".log";
template<> const regex Parameters<e_xpulog>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_xpulog>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_xpulog>::s_apzset = 1<<e_apz21403 | 1<<e_apz21410 | 1<<e_apz21420;
template<> const t_headertype Parameters<e_xpulog>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_xpulog>::s_hasmsgno = false;
template<> const bool Parameters<e_xpulog>::s_hasxmno = true;
template<> uintmax_t Parameters<e_xpulog>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_xpulog>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_xpulog>::s_divider = 50;
template<> int Parameters<e_xpulog>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_xpucore>
//========================================================================================

// Parameters
template<> const string Parameters<e_xpucore>::s_logname = "XPUCORE";
template<> const t_loggroup Parameters<e_xpucore>::s_loggroup = e_lgXPU;
template<> const string Parameters<e_xpucore>::s_shortname = s_logname;
template<> const string Parameters<e_xpucore>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_xpucore>::s_path = "xpu/core";
template<> const string Parameters<e_xpucore>::s_fileprefix = "XMCore";
template<> const string Parameters<e_xpucore>::s_fileext = ".log";
template<> const regex Parameters<e_xpucore>::s_logfile(
      s_fileprefix + s_xmpattern + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_xpucore>::s_tempfile(
      s_fileprefix + s_xmpattern + "\\.tmp"
      );
template<> const uint16_t Parameters<e_xpucore>::s_apzset = 1<<e_apz21403 | 1<<e_apz21410 | 1<<e_apz21420;
template<> const t_headertype Parameters<e_xpucore>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_xpucore>::s_hasmsgno = false;
template<> const bool Parameters<e_xpucore>::s_hasxmno = true;
template<> uintmax_t Parameters<e_xpucore>::s_maxsize = 500000000;
template<> uint64_t Parameters<e_xpucore>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_xpucore>::s_divider = 50;
template<> int Parameters<e_xpucore>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_trace>
//========================================================================================

// Parameters
template<> const string Parameters<e_trace>::s_logname = "TRACE";
template<> const t_loggroup Parameters<e_trace>::s_loggroup = e_lgCP;
template<> const string Parameters<e_trace>::s_shortname = s_logname;
template<> const string Parameters<e_trace>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_trace>::s_path = "cps/trace";
template<> const string Parameters<e_trace>::s_fileprefix = "TraceLog";
template<> const string Parameters<e_trace>::s_fileext = ".log";
template<> const regex Parameters<e_trace>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_trace>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_trace>::s_apzset = s_apzAll;
template<> const t_headertype Parameters<e_trace>::s_headertype = e_tesrvHeader;
template<> const bool Parameters<e_trace>::s_hasmsgno = false;
template<> const bool Parameters<e_trace>::s_hasxmno = false;
template<> uintmax_t Parameters<e_trace>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_trace>::s_maxtime = 0;
template<> uint16_t Parameters<e_trace>::s_divider = 0;
template<> int Parameters<e_trace>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_rp>
//========================================================================================

// Parameters
template<> const string Parameters<e_rp>::s_logname = "RPDUMPLOG";
template<> const t_loggroup Parameters<e_rp>::s_loggroup = e_lgRP;
template<> const string Parameters<e_rp>::s_shortname = "RP";
template<> const string Parameters<e_rp>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_rp>::s_path = "rp";
template<> const string Parameters<e_rp>::s_fileprefix = "RP_";
template<> const string Parameters<e_rp>::s_fileext = "(.txt|.bin)";
template<> const regex Parameters<e_rp>::s_logfile(
      s_fileprefix + "\\d{1,4}_\\d{1,3}_\\d{1,2}_\\d{8}_\\d{6}_([a-zA-Z0-9]+)._\\d{8}_\\d{6}" + s_fileext
      );
template<> const regex Parameters<e_rp>::s_tempfile(
      s_fileprefix + "\\d{1,4}_\\d{1,3}_\\d{1,2}_\\d{8}_\\d{6}_([a-zA-Z0-9]+)._(txt|bin)\\.tmp"
      );
template<> const uint16_t Parameters<e_rp>::s_apzset = s_apzForRP;
template<> const t_headertype Parameters<e_rp>::s_headertype = e_noHeader;
template<> const bool Parameters<e_rp>::s_hasmsgno = false;
template<> const bool Parameters<e_rp>::s_hasxmno = false;
template<> uintmax_t Parameters<e_rp>::s_maxsize = 1000000000;
template<> uint64_t Parameters<e_rp>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_rp>::s_divider = 50;
template<> int Parameters<e_rp>::s_noEP = 0;

//========================================================================================
// Class Parameters<e_mphca>
//========================================================================================

// Parameters
template<> const string Parameters<e_mphca>::s_logname = "PHCA";
template<> const t_loggroup Parameters<e_mphca>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mphca>::s_shortname = "MPHCA";
template<> const string Parameters<e_mphca>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mphca>::s_path = "mau/phc/phca";
template<> const string Parameters<e_mphca>::s_fileprefix = "Phc";
template<> const string Parameters<e_mphca>::s_fileext = ".log";
template<> const regex Parameters<e_mphca>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mphca>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mphca>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mphca>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mphca>::s_hasmsgno = false;
template<> const bool Parameters<e_mphca>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mphca>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mphca>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mphca>::s_divider = 50;
template<> int Parameters<e_mphca>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mphcb>
//========================================================================================

// Parameters
template<> const string Parameters<e_mphcb>::s_logname = "PHCB";
template<> const t_loggroup Parameters<e_mphcb>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mphcb>::s_shortname = "MPHCB";
template<> const string Parameters<e_mphcb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mphcb>::s_path = "mau/phc/phcb";
template<> const string Parameters<e_mphcb>::s_fileprefix = "Phc";
template<> const string Parameters<e_mphcb>::s_fileext = ".log";
template<> const regex Parameters<e_mphcb>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mphcb>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mphcb>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mphcb>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mphcb>::s_hasmsgno = false;
template<> const bool Parameters<e_mphcb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mphcb>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mphcb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mphcb>::s_divider = 50;
template<> int Parameters<e_mphcb>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mwsr>
//========================================================================================

// Parameters
template<> const string Parameters<e_mwsr>::s_logname = "WSR";
template<> const t_loggroup Parameters<e_mwsr>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mwsr>::s_shortname = "MWSR";
template<> const string Parameters<e_mwsr>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mwsr>::s_path = "mau/wsr";
template<> const string Parameters<e_mwsr>::s_fileprefix = "Wsr";
template<> const string Parameters<e_mwsr>::s_fileext = ".log";
template<> const regex Parameters<e_mwsr>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mwsr>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mwsr>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mwsr>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mwsr>::s_hasmsgno = false;
template<> const bool Parameters<e_mwsr>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mwsr>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mwsr>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mwsr>::s_divider = 50;
template<> int Parameters<e_mwsr>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mehl>
//========================================================================================

// Parameters
template<> const string Parameters<e_mehl>::s_logname = "EHL";
template<> const t_loggroup Parameters<e_mehl>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mehl>::s_shortname = "MEHL";
template<> const string Parameters<e_mehl>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mehl>::s_path = "mau/ehl";
template<> const string Parameters<e_mehl>::s_fileprefix = "Ehl";
template<> const string Parameters<e_mehl>::s_fileext = ".log";
template<> const regex Parameters<e_mehl>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mehl>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mehl>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mehl>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mehl>::s_hasmsgno = false;
template<> const bool Parameters<e_mehl>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mehl>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mehl>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mehl>::s_divider = 50;
template<> int Parameters<e_mehl>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mcpflagsa>
//========================================================================================

// Parameters
template<> const string Parameters<e_mcpflagsa>::s_logname = "CPFLAGSA";
template<> const t_loggroup Parameters<e_mcpflagsa>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mcpflagsa>::s_shortname = "MCPFLAGSA";
template<> const string Parameters<e_mcpflagsa>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mcpflagsa>::s_path = "mau/cpflags/cpflagsa";
template<> const string Parameters<e_mcpflagsa>::s_fileprefix = "Cpflags";
template<> const string Parameters<e_mcpflagsa>::s_fileext = ".log";
template<> const regex Parameters<e_mcpflagsa>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mcpflagsa>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mcpflagsa>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mcpflagsa>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mcpflagsa>::s_hasmsgno = false;
template<> const bool Parameters<e_mcpflagsa>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mcpflagsa>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mcpflagsa>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mcpflagsa>::s_divider = 50;
template<> int Parameters<e_mcpflagsa>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mcpflagsb>
//========================================================================================

// Parameters
template<> const string Parameters<e_mcpflagsb>::s_logname = "CPFLAGSB";
template<> const t_loggroup Parameters<e_mcpflagsb>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mcpflagsb>::s_shortname = "MCPFLAGSB";
template<> const string Parameters<e_mcpflagsb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mcpflagsb>::s_path = "mau/cpflags/cpflagsb";
template<> const string Parameters<e_mcpflagsb>::s_fileprefix = "Cpflags";
template<> const string Parameters<e_mcpflagsb>::s_fileext = ".log";
template<> const regex Parameters<e_mcpflagsb>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mcpflagsb>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mcpflagsb>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mcpflagsb>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mcpflagsb>::s_hasmsgno = false;
template<> const bool Parameters<e_mcpflagsb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mcpflagsb>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mcpflagsb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mcpflagsb>::s_divider = 50;
template<> int Parameters<e_mcpflagsb>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_minfr>
//========================================================================================

// Parameters
template<> const string Parameters<e_minfr>::s_logname = "INFR";
template<> const t_loggroup Parameters<e_minfr>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_minfr>::s_shortname = "MINFR";
template<> const string Parameters<e_minfr>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_minfr>::s_path = "mau/infr";
template<> const string Parameters<e_minfr>::s_fileprefix = "Infr";
template<> const string Parameters<e_minfr>::s_fileext = ".log";
template<> const regex Parameters<e_minfr>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_minfr>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_minfr>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_minfr>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_minfr>::s_hasmsgno = false;
template<> const bool Parameters<e_minfr>::s_hasxmno = false;
template<> uintmax_t Parameters<e_minfr>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_minfr>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_minfr>::s_divider = 50;
template<> int Parameters<e_minfr>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mintfstsa>
//========================================================================================

// Parameters
template<> const string Parameters<e_mintfstsa>::s_logname = "INTFSTSA";
template<> const t_loggroup Parameters<e_mintfstsa>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mintfstsa>::s_shortname = "MINTFSTSA";
template<> const string Parameters<e_mintfstsa>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mintfstsa>::s_path = "mau/intfsts/intfstsa";
template<> const string Parameters<e_mintfstsa>::s_fileprefix = "Intfsts";
template<> const string Parameters<e_mintfstsa>::s_fileext = ".log";
template<> const regex Parameters<e_mintfstsa>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mintfstsa>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mintfstsa>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mintfstsa>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mintfstsa>::s_hasmsgno = false;
template<> const bool Parameters<e_mintfstsa>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mintfstsa>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mintfstsa>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mintfstsa>::s_divider = 50;
template<> int Parameters<e_mintfstsa>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_mintfstsb>
//========================================================================================

// Parameters
template<> const string Parameters<e_mintfstsb>::s_logname = "INTFSTSB";
template<> const t_loggroup Parameters<e_mintfstsb>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mintfstsb>::s_shortname = "MINTFSTSB";
template<> const string Parameters<e_mintfstsb>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mintfstsb>::s_path = "mau/intfsts/intfstsb";
template<> const string Parameters<e_mintfstsb>::s_fileprefix = "Intfsts";
template<> const string Parameters<e_mintfstsb>::s_fileext = ".log";
template<> const regex Parameters<e_mintfstsb>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mintfstsb>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mintfstsb>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mintfstsb>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mintfstsb>::s_hasmsgno = false;
template<> const bool Parameters<e_mintfstsb>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mintfstsb>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mintfstsb>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mintfstsb>::s_divider = 50;
template<> int Parameters<e_mintfstsb>::s_noEP = 1;

//========================================================================================
// Class Parameters<e_msyscon>
//========================================================================================

// Parameters
template<> const string Parameters<e_msyscon>::s_logname = "MAUSSYSCON";
template<> const t_loggroup Parameters<e_msyscon>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_msyscon>::s_shortname = "MSYSCON";
template<> const string Parameters<e_msyscon>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_msyscon>::s_path = "syscon";
template<> const string Parameters<e_msyscon>::s_fileprefix = "Syscon";
template<> const string Parameters<e_msyscon>::s_fileext = ".log";
template<> const regex Parameters<e_msyscon>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_msyscon>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_msyscon>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_msyscon>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_msyscon>::s_hasmsgno = false;
template<> const bool Parameters<e_msyscon>::s_hasxmno = false;
template<> uintmax_t Parameters<e_msyscon>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_msyscon>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_msyscon>::s_divider = 50;
template<> int Parameters<e_msyscon>::s_noEP = 2;

//========================================================================================
// Class Parameters<e_mcore>
//========================================================================================

// Parameters
template<> const string Parameters<e_mcore>::s_logname = "MAUSCORE";
template<> const t_loggroup Parameters<e_mcore>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mcore>::s_shortname = "MCORE";
template<> const string Parameters<e_mcore>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mcore>::s_path = "core";
template<> const string Parameters<e_mcore>::s_fileprefix = "Core";
template<> const string Parameters<e_mcore>::s_fileext = ".log";
template<> const regex Parameters<e_mcore>::s_logfile(
      s_fileprefix + s_datepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mcore>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mcore>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mcore>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mcore>::s_hasmsgno = false;
template<> const bool Parameters<e_mcore>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mcore>::s_maxsize = 100000000;
template<> uint64_t Parameters<e_mcore>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mcore>::s_divider = 50;
template<> int Parameters<e_mcore>::s_noEP = 2;

//========================================================================================
// Class Parameters<e_mevent>
//========================================================================================

// Parameters
template<> const string Parameters<e_mevent>::s_logname = "MAUSEVENT";
template<> const t_loggroup Parameters<e_mevent>::s_loggroup = e_lgMAU;
template<> const string Parameters<e_mevent>::s_shortname = "MEVENT";
template<> const string Parameters<e_mevent>::s_commonname = s_shortname;
template<> const fs::path Parameters<e_mevent>::s_path = "event";
template<> const string Parameters<e_mevent>::s_fileprefix = "Event";
template<> const string Parameters<e_mevent>::s_fileext = ".log";
template<> const regex Parameters<e_mevent>::s_logfile(
      s_fileprefix + s_extdatepattern + "\\" + s_fileext
      );
template<> const regex Parameters<e_mevent>::s_tempfile(
      s_fileprefix + "\\.tmp[_]?"
      );
template<> const uint16_t Parameters<e_mevent>::s_apzset = s_apzMaus;
template<> const t_headertype Parameters<e_mevent>::s_headertype = e_clhHeader;
template<> const bool Parameters<e_mevent>::s_hasmsgno = false;
template<> const bool Parameters<e_mevent>::s_hasxmno = false;
template<> uintmax_t Parameters<e_mevent>::s_maxsize = 10000000;
template<> uint64_t Parameters<e_mevent>::s_maxtime = 336 * Time::s_hour;
template<> uint16_t Parameters<e_mevent>::s_divider = 50;
template<> int Parameters<e_mevent>::s_noEP = 4;

}

