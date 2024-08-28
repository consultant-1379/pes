//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      loginfo.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012, 2013, 2014. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for handling list of log types.
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
//      PA1    2012-10-01   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "loginfo.h"
#include "exception.h"

using namespace std;

namespace PES_CLH {

//========================================================================================
// Class LogInfo
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//---------------------------------------------------------------------------------------- 
LogInfo::LogInfo(): 
m_parametersp(0),
m_xmfilter()
{
}

LogInfo::LogInfo(BaseParameters& parameters):
m_parametersp(&parameters),
m_xmfilter()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------------------- 
LogInfo::~LogInfo()
{
}

//----------------------------------------------------------------------------------------
// Equality operator
//---------------------------------------------------------------------------------------- 
bool LogInfo::operator==(const string& logname) const
{
   return m_parametersp->getLogName() == logname;
}

//----------------------------------------------------------------------------------------
// Less than operator
//---------------------------------------------------------------------------------------- 
bool LogInfo::operator<(const string& logname) const
{
   return m_parametersp->getLogName() < logname;
}

//----------------------------------------------------------------------------------------
// Get log parameters
//----------------------------------------------------------------------------------------
BaseParameters& LogInfo::getParameters() const
{
   return *m_parametersp;
}

//----------------------------------------------------------------------------------------
// Set the XM filter
//---------------------------------------------------------------------------------------- 
void LogInfo::setXmFilter(const XmFilter& xmfilter)
{
   m_xmfilter = xmfilter;
}

//----------------------------------------------------------------------------------------
// Get the XM filter
//----------------------------------------------------------------------------------------
XmFilter LogInfo::getXmFilter() const
{
   return m_xmfilter;
}


//========================================================================================
// Class LogTable
//========================================================================================

Parameters<e_error>        LogTable::s_error;
Parameters<e_event>        LogTable::s_event;
Parameters<e_syslog>       LogTable::s_syslog;
Parameters<e_binlog>       LogTable::s_binlog;
Parameters<e_corecpbb>     LogTable::s_corecpbb;
Parameters<e_corecpsb>     LogTable::s_corecpsb;
Parameters<e_corepcih>     LogTable::s_corepcih;
Parameters<e_corecpub>     LogTable::s_corecpub;
Parameters<e_crashcpbb>    LogTable::s_crashcpbb;
Parameters<e_crashcpsb>    LogTable::s_crashcpsb;
Parameters<e_crashpcih>    LogTable::s_crashpcih;
Parameters<e_crashcpub>    LogTable::s_crashcpub;
Parameters<e_evlogcpsb>    LogTable::s_evlogcpsb;
Parameters<e_evlogpcih>    LogTable::s_evlogpcih;
Parameters<e_salinfocpsb>  LogTable::s_salinfocpsb;
Parameters<e_sel>          LogTable::s_sel;
Parameters<e_ruf>          LogTable::s_ruf;
Parameters<e_consolsrm>    LogTable::s_consolsrm;
Parameters<e_consolbmc>    LogTable::s_consolbmc;
Parameters<e_consolmp>     LogTable::s_consolmp;
Parameters<e_consolpcih>   LogTable::s_consolpcih;
Parameters<e_consolsyscon> LogTable::s_consolsyscon;
Parameters<e_xpulog>       LogTable::s_xpulog;
Parameters<e_xpucore>      LogTable::s_xpucore;
Parameters<e_trace>        LogTable::s_trace;
Parameters<e_rp>           LogTable::s_rp;
Parameters<e_mphca>        LogTable::s_mphca;
Parameters<e_mphcb>        LogTable::s_mphcb;
Parameters<e_mwsr>         LogTable::s_mwsr;
Parameters<e_mehl>         LogTable::s_mehl;
Parameters<e_mcpflagsa>    LogTable::s_mcpflagsa;
Parameters<e_mcpflagsb>    LogTable::s_mcpflagsb;
Parameters<e_minfr>        LogTable::s_minfr;
Parameters<e_mintfstsa>    LogTable::s_mintfstsa;
Parameters<e_mintfstsb>    LogTable::s_mintfstsb;
Parameters<e_msyscon>      LogTable::s_msyscon;
Parameters<e_mcore>        LogTable::s_mcore;
Parameters<e_mevent>       LogTable::s_mevent;

//----------------------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------------------- 
LogTable::LogTable():
m_loglist(),
m_hasxmno(false)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------------------- 
LogTable::~LogTable()
{
}

//----------------------------------------------------------------------------------------
// Initialize list of log types for a certain APZ system. If no APZ system is specified,
// the entire list is initialized
//----------------------------------------------------------------------------------------
void LogTable::initialize(t_apzSystem apzsys)
{
   initialize(s_error,        apzsys);
   initialize(s_event,        apzsys);
   initialize(s_syslog,       apzsys);
   initialize(s_binlog,       apzsys);
   initialize(s_corecpbb,     apzsys);
   initialize(s_corecpsb,     apzsys);
   initialize(s_corepcih,     apzsys);
   initialize(s_corecpub,     apzsys);
   initialize(s_crashcpbb,    apzsys);
   initialize(s_crashcpsb,    apzsys);
   initialize(s_crashpcih,    apzsys);
   initialize(s_crashcpub,    apzsys);
   initialize(s_evlogcpsb,    apzsys);
   initialize(s_evlogpcih,    apzsys);
   initialize(s_salinfocpsb,  apzsys);
   initialize(s_sel,          apzsys);
   initialize(s_ruf,          apzsys);
   initialize(s_consolsrm,    apzsys);
   initialize(s_consolbmc,    apzsys);
   initialize(s_consolmp,     apzsys);
   initialize(s_consolpcih,   apzsys);
   initialize(s_consolsyscon, apzsys);
   initialize(s_xpulog,       apzsys);
   initialize(s_xpucore,      apzsys);
   initialize(s_trace,        apzsys);
   initialize(s_rp,           apzsys);
   initialize(s_mphca,        apzsys);
   initialize(s_mphcb,        apzsys);
   initialize(s_mwsr,         apzsys);
   initialize(s_mehl,         apzsys);
   initialize(s_mcpflagsa,    apzsys);
   initialize(s_mcpflagsb,    apzsys);
   initialize(s_minfr,        apzsys);
   initialize(s_mintfstsa,    apzsys);
   initialize(s_mintfstsb,    apzsys);
   initialize(s_msyscon,      apzsys);
   initialize(s_mcore,        apzsys);
   initialize(s_mevent,       apzsys);
}

//----------------------------------------------------------------------------------------
// Initialize a log type entry in the log table (if APZ system is valid)
//----------------------------------------------------------------------------------------
void LogTable::initialize(BaseParameters& parameters, t_apzSystem apzsys)
{
   if (parameters.isValidApz(apzsys))
   {
      m_hasxmno |= parameters.hasXmNo();
      m_loglist.push_back(LogInfo(parameters));
   }
}

//----------------------------------------------------------------------------------------
// Get log parameters for all log types valid for an APZ type
//---------------------------------------------------------------------------------------- 
LogTable::LIST LogTable::getList(t_apzSystem apzsys) const
{
   LIST loglist;
   for (LISTCITER iter = m_loglist.begin(); iter != m_loglist.end(); ++iter)
   {
      const LogInfo& loginfo = *iter;
      const BaseParameters& parameters = loginfo.getParameters();
      if (parameters.isValidApz(apzsys))
      {
         loglist.push_back(loginfo);
      }
   }
   return loglist;
}

//----------------------------------------------------------------------------------------
// Get log parameters for a list of log types valid for an APZ type
//----------------------------------------------------------------------------------------
LogTable::LIST LogTable::getList(const list<string>& namelist, t_apzSystem apzsys) const
{
   LIST loglist;
   Exception::t_error error;
   enum findflag_t {e_none, e_short, e_common} findflag;

   std::list<std::string>::const_iterator iter;
   for (iter = namelist.begin(); iter != namelist.end(); ++iter)
   {
      size_t pos = iter->find(':');
      const string& logname = (m_hasxmno)? iter->substr(0, pos): *iter;
      bool empty = true;
      bool foundapz = false;
      for (LISTCITER liter = m_loglist.begin(); liter != m_loglist.end(); ++liter)
      {
         findflag = e_none;
         LogInfo loginfo = *liter;
         const BaseParameters& parameters = loginfo.getParameters();
         if (logname == parameters.getShortName() &&
               parameters.getLogGroup() != e_lgMAU)
         {
            // Found short name
            empty = false;
            if (parameters.isValidApz(apzsys))
            {
               if (pos != string::npos)
               {
                  // XM-list found
                  if (parameters.hasXmNo() == false)
                  {
                     // XM-list is not allowed for this log type
                     throw Exception(Exception::unexpXmno(logname), WHERE__);
                  }

                  // Insert XM number
                  const string& xmstr = iter->substr(pos + 1);
                  loginfo.setXmFilter(xmstr);
               }

               findflag = e_short;
               foundapz = true;
            }
         }
         else if (logname == parameters.getCommonName())
         {
            // Found common name
            empty = false;
            if (pos != string::npos)
            {
               // Common name cannot have XPU list
               if (m_hasxmno)
               {
                  // XM-list is not allowed for this log type
                  throw Exception(Exception::unexpXmno(logname), WHERE__);
               }
               else
               {
                  // Illegal log type
                  Exception ex(Exception::illLogType(logname), WHERE__);
                  throw ex;
               }
            }

            if (parameters.isValidApz(apzsys))
            {
               findflag = e_common;
               foundapz = true;
            }
         }

         if (findflag != e_none)
         {
            LISTCITER listiter = std::find(loglist.begin(), loglist.end(), parameters.getLogName());
            if (listiter == loglist.end())
            {
               loglist.push_back(loginfo);
            }
            else
            {
               throw Exception(Exception::logTypeDupl(parameters.getShortName()), WHERE__);
            }
         }
         if (findflag == e_short) break;
      }

      if (empty)
      {
         throw Exception(Exception::illLogType(logname), WHERE__);
      }
      if (!foundapz)
      {
         throw Exception(Exception::illLogTypeForApz(logname), WHERE__);
      }
   }

   return loglist;
}

}
