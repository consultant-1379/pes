//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      logtask.h
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
//      Class for handling all log tasks
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
//      PA1    2012-05-03   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef LOGTASK_H_
#define LOGTASK_H_

#include "parameters.h"
#include "common.h"
#include "cpinfo.h"
#include "appendtask.h"
#include "filetask.h"
#include "dirtask.h"
#include "tru64task.h"
#include "notask.h"
#include "logger.h"
#include "rptask.h"
#include "seltask.h"
#include "mausinfo.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace PES_CLH {

template<t_logtype logtype, typename T>
class LogTask: public T
{
public:

   // Constructors
   LogTask():
   T(),
   m_parameters(),
   m_cpinfo(),
   m_cpside(),
   m_nocpside(true),
   m_swmauinfo()
   {
   }

   LogTask(
           const CPInfo& cpinfo,
           t_cpSide cpside
           ):
   T(),
   m_parameters(),
   m_cpinfo(cpinfo),
   m_cpside(cpside),
   m_nocpside(false),
   m_swmauinfo()
   {
   }

   // Constructors
   LogTask(
         const CPInfo& cpinfo,
         const MAUSInfo& swmauinfo):
   T(),
   m_parameters(),
   m_cpinfo(cpinfo),
   m_cpside(),
   m_nocpside(true),
   m_swmauinfo(swmauinfo)
   {
   }

   // Destructor
   ~LogTask()
   {
   }

   // Act on log event
   void event(
         const fs::path& path
         );

   // Get log parameters
   const BaseParameters& getParameters() const {return m_parameters;}

   // Get parent log directory
   fs::path getParentDir() const;

   // Get log directory
   fs::path getLogDir() const;

   // Create log directory
   void createLogDir() const;

private:
   // Stream textual information about the log entry
   void stream(std::ostream& s) const;

   const Parameters<logtype> m_parameters;
   CPInfo m_cpinfo;              // CP information
   t_cpSide m_cpside;            // CP side
   bool m_nocpside;              // No CP side
   MAUSInfo m_swmauinfo;        // SW MAU Info
};

//----------------------------------------------------------------------------------------
// Get parent log directory
//----------------------------------------------------------------------------------------
template<t_logtype logtype, typename T>
fs::path LogTask<logtype, T>::getParentDir() const
{
   fs::path cpdir = m_cpinfo.getName();

   if (m_parameters.getLogGroup() == e_lgMAU)
   {
      cpdir /= m_swmauinfo.getParentPath();
      return cpdir;
   }

   switch (m_cpside)
   {
      case e_cpa: cpdir /= "cpa"; break;
      case e_cpb: cpdir /= "cpb"; break;
      default:    assert(!"Illegal CP side");
   }
   return cpdir;
}

//----------------------------------------------------------------------------------------
// Get log directory
//----------------------------------------------------------------------------------------
template<t_logtype logtype, typename T>
fs::path LogTask<logtype, T>::getLogDir() const
{
   const fs::path& logpath = BaseParameters::getApzLogsPath();

   if (m_nocpside)
   {
      if (m_parameters.getLogGroup() == e_lgMAU)
      {
         fs::path mauspath = m_swmauinfo.getPath();
         return mauspath;
      }
      return logpath / m_parameters.getPath();
   }

   return logpath / getParentDir() / m_parameters.getPath();
}

//----------------------------------------------------------------------------------------
// Create log directory
//----------------------------------------------------------------------------------------
template<t_logtype logtype, typename T>
void LogTask<logtype, T>::createLogDir() const
{
   const fs::path& logdir = getLogDir();
   Common::createDirsAndLinks(logdir);
}

//----------------------------------------------------------------------------------------
// Stream textual information about the log task
//----------------------------------------------------------------------------------------
template<t_logtype logtype, typename T>
void LogTask<logtype, T>::stream(std::ostream& s) const
{
   if (!m_nocpside)
   {
      const std::string& cpname = m_cpinfo.getName();
      if (cpname.empty() == false)
      {
         s << "CP: " << boost::to_upper_copy(cpname) << "   ";
      }
      s << "CP-side: ";
      switch (m_cpside)
      {
         case e_cpa: s << "A"; break;
         case e_cpb: s << "B"; break;
         default:    assert(!"Illegal CP side");
      }
      s << "   ";
   }
   s << "Log type: " << m_parameters.getLogName();
}

//----------------------------------------------------------------------------------------
// Type definitions for all log tasks
//----------------------------------------------------------------------------------------

typedef LogTask<e_error,        AppendTask> Error;
typedef LogTask<e_event,        AppendTask> Event;
typedef LogTask<e_syslog,       AppendTask> Syslog;
typedef LogTask<e_binlog,       AppendTask> Binlog;
typedef LogTask<e_corecpbb,     FileTask>   Corecpbb;
typedef LogTask<e_corecpsb,     FileTask>   Corecpsb;
typedef LogTask<e_corepcih,     FileTask>   Corepcih;
typedef LogTask<e_corecpub,     FileTask>   Corecpub;
typedef LogTask<e_crashcpbb,    Tru64Task>  Crashcpbb;
typedef LogTask<e_crashcpsb,    DirTask>    Crashcpsb;
typedef LogTask<e_crashpcih,    DirTask>    Crashpcih;
typedef LogTask<e_crashcpub,    FileTask>   Crashcpub;
typedef LogTask<e_evlogcpsb,    AppendTask> Evlogcpsb;
typedef LogTask<e_evlogpcih,    AppendTask> Evlogpcih;
typedef LogTask<e_salinfocpsb,  DirTask>    Salinfocpsb;
typedef LogTask<e_sel,          SelTask>    Sel;
typedef LogTask<e_ruf,          NoTask>     Ruf;
typedef LogTask<e_consolsrm,    AppendTask> Consolsrm;
typedef LogTask<e_consolbmc,    AppendTask> Consolbmc;
typedef LogTask<e_consolmp,     AppendTask> Consolmp;
typedef LogTask<e_consolpcih,   AppendTask> Consolpcih;
typedef LogTask<e_consolsyscon, AppendTask> Consolsyscon;
typedef LogTask<e_xpulog,       AppendTask> Xpulog;
typedef LogTask<e_xpucore,      FileTask>   Xpucore;
typedef LogTask<e_trace,        AppendTask> Trace;
typedef LogTask<e_rp,           RPTask>     RPLog;
typedef LogTask<e_mphca,        AppendTask> PhcaLog;
typedef LogTask<e_mphcb,        AppendTask> PhcbLog;
typedef LogTask<e_mwsr,         AppendTask> WsrLog;
typedef LogTask<e_mehl,         AppendTask> EhlLog;
typedef LogTask<e_mcpflagsa,    AppendTask> CpflagsaLog;
typedef LogTask<e_mcpflagsb,    AppendTask> CpflagsbLog;
typedef LogTask<e_minfr,        AppendTask> InfrLog;
typedef LogTask<e_mintfstsa,    AppendTask> IntfstsaLog;
typedef LogTask<e_mintfstsb,    AppendTask> IntfstsbLog;
typedef LogTask<e_msyscon,      AppendTask> SysconMauLog;
typedef LogTask<e_mcore,        FileTask>   CoreMau;
typedef LogTask<e_mevent,       AppendTask> EventMauLog;

// Create a log task
BaseTask* createTask(
      t_logtype logtype,
      const CPInfo& cpinfo,
      t_cpSide cpside
      );
// Create a log task
BaseTask* createTask(
      t_logtype logtype
      );

// Create a log task
BaseTask* createTask(
      t_logtype logtype,
      const CPInfo& cpinfo,
      const MAUSInfo& swmauinfo
      );
}

#endif // LOGTASK_H_
