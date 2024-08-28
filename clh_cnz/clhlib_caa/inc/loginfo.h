//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      loginfo.h
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
//      PA1    2012-05-03   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef LOGINFO_H_
#define LOGINFO_H_

#include "parameters.h"
#include "xmfilter.h"
#include <string>
#include <list>

namespace PES_CLH {

//========================================================================================
// Class LogInfo
//========================================================================================

class LogInfo
{
public:
   LogInfo();

   LogInfo(
         BaseParameters& parameters       // Log task
         );

   ~LogInfo();

   bool operator==(
         const std::string& logname       // log name
         ) const;

   bool operator<(
         const std::string& logname       // log name
         ) const;

   // Get log parameters
   BaseParameters& getParameters() const;
                                          // Returns the log parameters

   // Set XM filter
   void setXmFilter(
         const XmFilter& xmfilter         // XM list
         );

   // Get XM filter
   XmFilter getXmFilter() const;          // Retuns the XM filter


private:
   BaseParameters* m_parametersp;
   XmFilter m_xmfilter;
};

//========================================================================================
// Class LogTable
//========================================================================================

class LogTable
{
public:
   typedef std::list<LogInfo> LIST;
   typedef LIST::iterator LISTITER;
   typedef LIST::const_iterator LISTCITER;

   LogTable();
   ~LogTable();

   void initialize(
         t_apzSystem apzsys = e_undefined
         );

   void initialize(
         BaseParameters& parameters,               // Log task
         t_apzSystem apzsys = e_undefined
         );

   LIST getList(
         t_apzSystem apzsys = e_undefined
         ) const;

   LIST getList(
         const std::list<std::string>& namelist,
         t_apzSystem apzsys = e_undefined
         ) const;


   static Parameters<e_error>        s_error;
   static Parameters<e_event>        s_event;
   static Parameters<e_syslog>       s_syslog;
   static Parameters<e_binlog>       s_binlog;
   static Parameters<e_corecpbb>     s_corecpbb;
   static Parameters<e_corecpsb>     s_corecpsb;
   static Parameters<e_corepcih>     s_corepcih;
   static Parameters<e_corecpub>     s_corecpub;
   static Parameters<e_crashcpbb>    s_crashcpbb;
   static Parameters<e_crashcpsb>    s_crashcpsb;
   static Parameters<e_crashpcih>    s_crashpcih;
   static Parameters<e_crashcpub>    s_crashcpub;
   static Parameters<e_evlogcpsb>    s_evlogcpsb;
   static Parameters<e_evlogpcih>    s_evlogpcih;
   static Parameters<e_salinfocpsb>  s_salinfocpsb;
   static Parameters<e_sel>          s_sel;
   static Parameters<e_ruf>          s_ruf;
   static Parameters<e_consolsrm>    s_consolsrm;
   static Parameters<e_consolbmc>    s_consolbmc;
   static Parameters<e_consolmp>     s_consolmp;
   static Parameters<e_consolpcih>   s_consolpcih;
   static Parameters<e_consolsyscon> s_consolsyscon;
   static Parameters<e_xpulog>       s_xpulog;
   static Parameters<e_xpucore>      s_xpucore;
   static Parameters<e_trace>        s_trace;
   static Parameters<e_rp>           s_rp;
   static Parameters<e_mphca>        s_mphca;
   static Parameters<e_mphcb>        s_mphcb;
   static Parameters<e_mwsr>         s_mwsr;
   static Parameters<e_mehl>         s_mehl;
   static Parameters<e_mcpflagsa>    s_mcpflagsa;
   static Parameters<e_mcpflagsb>    s_mcpflagsb;
   static Parameters<e_minfr>        s_minfr;
   static Parameters<e_mintfstsa>    s_mintfstsa;
   static Parameters<e_mintfstsb>    s_mintfstsb;
   static Parameters<e_msyscon>      s_msyscon;
   static Parameters<e_mcore>        s_mcore;
   static Parameters<e_mevent>       s_mevent;

private:
   LIST m_loglist;
   bool m_hasxmno;
};

}

#endif // LOGINFO_H_
