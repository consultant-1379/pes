//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      logtask.cpp
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
//      Class for handling all log tasks.
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
//      PA1    2012-09-27   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "logtask.h"
#include "xmfilter.h"
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;
using namespace boost;

namespace PES_CLH {

//========================================================================================
// Class Error
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Error::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Event
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Event::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Syslog
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Syslog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Binlog
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Binlog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Corecpbb
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Corecpbb::event(const fs::path& path)
{
   // Process the file
   Time time;
   uintmax_t size;
   
   readHeader(path, time, size);
   insert(path, getParameters().getFilePrefix(), time, size);
}

//========================================================================================
// Class Corecpsb
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Corecpsb::event(const fs::path& path)
{
   // Process the file
   Time time;
   uintmax_t size;
   
   readHeader(path, time, size);
   insert(path, getParameters().getFilePrefix(), time, size);
}

//========================================================================================
// Class Corepcih
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Corepcih::event(const fs::path& path)
{
   // Process the file
   Time time;
   uintmax_t size;
   
   readHeader(path, time, size);
   insert(path, path.stem().c_str(), time, size);
}

//========================================================================================
// Class Corecpub
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Corecpub::event(const fs::path& path)
{
   // Process the file
   Time time;
   uintmax_t size;

   readHeader(path, time, size);
   insert(path, getParameters().getFilePrefix(), time, size);
}

//========================================================================================
// Class Crashcpbb
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Crashcpbb::event(const fs::path& path)
{
   // Process the files
   insert(path);
}

//========================================================================================
// Class Crashcpsb
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Crashcpsb::event(const fs::path& path)
{
   // Process the directory
   m_threadgroup.create_thread(boost::bind(&Crashcpsb::insert, this, path, 6));
}

//========================================================================================
// Class Crashpcih
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Crashpcih::event(const fs::path& path)
{
   // Process the directory
   m_threadgroup.create_thread(boost::bind(&Crashpcih::insert, this, path, 6));
}

//========================================================================================
// Class Crashcpub
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Crashcpub::event(const fs::path& path)
{
   // Process the file
   uintmax_t size = fs::file_size(path);
   insert(path, getParameters().getFilePrefix(), Time::now(), size);
}

//========================================================================================
// Class Evlogcpsb
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Evlogcpsb::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Evlogpcih
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Evlogpcih::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Salinfocpsb
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Salinfocpsb::event(const fs::path& path)
{
   // Process the directory
   m_threadgroup.create_thread(boost::bind(&Salinfocpsb::insert, this, path, 6));
}

//========================================================================================
// Class Sel
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Sel::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Ruf
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Ruf::event(const fs::path&)
{
   // Ruf logs are not handled by CLH
}

//========================================================================================
// Class Consolsrm
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Consolsrm::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Consolbmc
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Consolbmc::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Consolmp
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Consolmp::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Consolpcih
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Consolpcih::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Consolsyscon
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Consolsyscon::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Xpulog
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Xpulog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Xpucore
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Xpucore::event(const fs::path& path)
{
   // Get XM number
   const string& stem = path.stem().c_str();
   size_t pos = stem.size();
   string str = stem.substr(pos - 4, 4);
   uint16_t xmno = boost::lexical_cast<uint16_t>(str);
   if (xmno > 0 && xmno <= XmFilter::s_maxmno)
   {
      // Process the file
      Time time;
      uintmax_t size;

      readHeader(path, time, size);
      const string& fileprefix = getParameters().getFilePrefix() + "_" + str;
      insert(path, fileprefix, time, size);
   }
   else
   {
      // Log event
      ostringstream s;
      s << *this << endl;
      s << "File " << path << " is not a valid Xpucore log - ignored.";
      Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

      // Delete file
      fs::remove(path);
   }
}

//========================================================================================
// Class Trace
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void Trace::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================

//========================================================================================
// Class RPLog
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void RPLog::event(const fs::path& path)
{
   // Process the file
   insert(path);
}

//========================================================================================

//========================================================================================
// Class PHC A
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void PhcaLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class PHC B
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void PhcbLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class WSR
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void WsrLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class EHL
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void EhlLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Cpflags A
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void CpflagsaLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Cpflags B
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void CpflagsbLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class INFR
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void InfrLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Intfsts A
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void IntfstsaLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class Intfsts B
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void IntfstsbLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class MAU Syscon
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void SysconMauLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class MAU Event
//========================================================================================
//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void EventMauLog::event(const fs::path& path)
{
   // Process the file
   readMsgs(path);
}

//========================================================================================
// Class MAU Core
//========================================================================================

//----------------------------------------------------------------------------------------
// Act on an event
//----------------------------------------------------------------------------------------
template<> void CoreMau::event(const fs::path& path)
{
   // Process the file
   Time time;
   uintmax_t size;

   readHeader(path, time, size);
   insert(path, getParameters().getFilePrefix(), time, size);
}

//----------------------------------------------------------------------------------------
// Create log task
//----------------------------------------------------------------------------------------
BaseTask* createTask(t_logtype logtype, const CPInfo& cpinfo, t_cpSide cpside)
{
   BaseTask* logtaskp;
   switch (logtype)
   {
   case e_error:        logtaskp = new Error(cpinfo, cpside);        break;
   case e_event:        logtaskp = new Event(cpinfo, cpside);        break;
   case e_syslog:       logtaskp = new Syslog(cpinfo, cpside);       break;
   case e_binlog:       logtaskp = new Binlog(cpinfo, cpside);       break;
   case e_corecpbb:     logtaskp = new Corecpbb(cpinfo, cpside);     break;
   case e_corecpsb:     logtaskp = new Corecpsb(cpinfo, cpside);     break;
   case e_corepcih:     logtaskp = new Corepcih(cpinfo, cpside);     break;
   case e_corecpub:     logtaskp = new Corecpub(cpinfo, cpside);     break;
   case e_crashcpbb:    logtaskp = new Crashcpbb(cpinfo, cpside);    break;
   case e_crashcpsb:    logtaskp = new Crashcpsb(cpinfo, cpside);    break;
   case e_crashpcih:    logtaskp = new Crashpcih(cpinfo, cpside);    break;
   case e_crashcpub:    logtaskp = new Crashcpub(cpinfo, cpside);    break;
   case e_evlogcpsb:    logtaskp = new Evlogcpsb(cpinfo, cpside);    break;
   case e_evlogpcih:    logtaskp = new Evlogpcih(cpinfo, cpside);    break;
   case e_salinfocpsb:  logtaskp = new Salinfocpsb(cpinfo, cpside);  break;
   case e_sel:          logtaskp = new Sel(cpinfo, cpside);          break;
   case e_ruf:          logtaskp = new Ruf(cpinfo, cpside);          break;
   case e_consolsrm:    logtaskp = new Consolsrm(cpinfo, cpside);    break;
   case e_consolbmc:    logtaskp = new Consolbmc(cpinfo, cpside);    break;
   case e_consolmp:     logtaskp = new Consolmp(cpinfo, cpside);     break;
   case e_consolpcih:   logtaskp = new Consolpcih(cpinfo, cpside);   break;
   case e_consolsyscon: logtaskp = new Consolsyscon(cpinfo, cpside); break;
   case e_xpulog:       logtaskp = new Xpulog(cpinfo, cpside);       break;
   case e_xpucore:      logtaskp = new Xpucore(cpinfo, cpside);      break;
   case e_trace:        logtaskp = new Trace(cpinfo, cpside);        break;
   default:             assert(!"Illegal log type");
   }

   return logtaskp;
}

//========================================================================================

//----------------------------------------------------------------------------------------
// Create log task
//----------------------------------------------------------------------------------------
BaseTask* createTask(t_logtype logtype)
{
   BaseTask* logtaskp;
   switch (logtype)
   {
   case e_rp:           logtaskp = new RPLog();                      break;
   case e_sel:          logtaskp = new Sel();                        break;
   default:             assert(!"Illegal log type");
   }

   return logtaskp;
}

//========================================================================================

//----------------------------------------------------------------------------------------
// Create log task
//----------------------------------------------------------------------------------------
BaseTask* createTask(t_logtype logtype, const CPInfo& cpinfo, const MAUSInfo& swmauinfo)
{
   BaseTask* logtaskp;
   switch (logtype)
   {
   case e_mphca:        logtaskp = new PhcaLog(cpinfo, swmauinfo);           break;
   case e_mphcb:        logtaskp = new PhcbLog(cpinfo, swmauinfo);           break;
   case e_mwsr:         logtaskp = new WsrLog(cpinfo, swmauinfo);            break;
   case e_mehl:         logtaskp = new EhlLog(cpinfo, swmauinfo);            break;
   case e_mcpflagsa:    logtaskp = new CpflagsaLog(cpinfo, swmauinfo);       break;
   case e_mcpflagsb:    logtaskp = new CpflagsbLog(cpinfo, swmauinfo);       break;
   case e_minfr:        logtaskp = new InfrLog(cpinfo, swmauinfo);           break;
   case e_mintfstsa:    logtaskp = new IntfstsaLog(cpinfo, swmauinfo);       break;
   case e_mintfstsb:    logtaskp = new IntfstsbLog(cpinfo, swmauinfo);       break;
   case e_msyscon:      logtaskp = new SysconMauLog(cpinfo, swmauinfo);      break;
   case e_mcore:        logtaskp = new CoreMau(cpinfo, swmauinfo);           break;
   case e_mevent:       logtaskp = new EventMauLog(cpinfo, swmauinfo);       break;
   default:             assert(!"Illegal log type");
   }

   return logtaskp;
}

}
