//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      parameters.h
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
//      PA1    2012-05-03   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-02-28   XLOBUNG     Add RP handling.
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "cpinfo.h"
#include "ltime.h"
#include "logger.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iomanip>

namespace fs = boost::filesystem;

namespace PES_CLH {

enum t_logtype
{
   e_error,
   e_event,
   e_syslog,
   e_binlog,
   e_corecpbb,
   e_corecpsb,
   e_corepcih,
   e_corecpub,
   e_crashcpbb,
   e_crashcpsb,
   e_crashpcih,
   e_crashcpub,
   e_evlogcpsb,
   e_evlogpcih,
   e_salinfocpsb,
   e_sel,
   e_ruf,
   e_consolsrm,
   e_consolbmc,
   e_consolmp,
   e_consolpcih,
   e_consolsyscon,
   e_xpulog,
   e_xpucore,
   e_trace,
   e_rp,
   e_mphca,
   e_mphcb,
   e_mwsr,
   e_mehl,
   e_mcpflagsa,
   e_mcpflagsb,
   e_minfr,
   e_mintfstsa,
   e_mintfstsb,
   e_msyscon,
   e_mcore,
   e_mevent
};

enum t_loggroup
{
   e_lgCP,
   e_lgXPU,
   e_lgSEL,
   e_lgRP,
   e_lgMAU
};

enum t_headertype
{
   e_noHeader,       // No header
   e_clhHeader,      // CLH header
   e_tesrvHeader,    // TESRV header
   e_selHeader       // SEL heander
};

//========================================================================================
// Class BaseParameters
//========================================================================================

class BaseParameters
{
public:
   BaseParameters() {}
   virtual ~BaseParameters() {}

   // Get path parameters
   static fs::path getApzLogsPath();
   static fs::path getCpsLogsPath();

   // Get log parameters
   virtual t_logtype getLogType() const = 0;
   virtual t_loggroup getLogGroup() const = 0;
   virtual std::string getLogName() const = 0;
   virtual std::string getShortName() const = 0;
   virtual std::string getCommonName() const = 0;
   virtual fs::path getPath() const = 0;
   virtual std::string getFilePrefix() const = 0;
   virtual std::string getFileExt() const = 0;
   virtual boost::regex getLogFile() const = 0;
   virtual boost::regex getTempFile() const = 0;
   virtual bool isValidApz(t_apzSystem apzsys) const = 0;
   virtual t_headertype getHeaderType() const = 0;
   virtual bool hasMsgNo() const = 0;
   virtual bool hasXmNo() const = 0;
   virtual uintmax_t getMaxsize() const = 0;
   virtual uint64_t getMaxtime() const = 0;
   virtual uint16_t getDivider() const = 0;
   virtual int getNoEndpoints() const = 0;

   // Set log parameters
   virtual void setParameters(
            uintmax_t maxsize,
            uint64_t maxtime,
            uint16_t divider
            ) = 0;

   static const uint32_t s_maxfilesize;

private:
   static fs::path s_apzlogspath;
   static fs::path s_cpslogspath;
};

//========================================================================================
// Class Parameters
//========================================================================================

template<t_logtype logtype>
class Parameters: public BaseParameters
{
public:
   Parameters():
   BaseParameters()
   {
   }

   virtual ~Parameters()
   {
   }

   // Get parameters
   t_logtype getLogType() const {return s_logtype;}
   t_loggroup getLogGroup() const {return s_loggroup;}
   std::string getLogName() const {return s_logname;}
   std::string getShortName() const {return s_shortname;}
   std::string getCommonName() const {return s_commonname;}
   fs::path getPath() const {return s_path;}
   std::string getFilePrefix() const {return s_fileprefix;}
   std::string getFileExt() const {return s_fileext;}
   boost::regex getLogFile() const {return s_logfile;}
   boost::regex getTempFile() const {return s_tempfile;}
   bool isValidApz(t_apzSystem apzsys) const
   {
      return (s_apzset & 1<<apzsys) || (apzsys == e_undefined);
   }
   t_headertype getHeaderType() const {return s_headertype;}
   bool hasMsgNo() const {return s_hasmsgno;}
   bool hasXmNo() const {return s_hasxmno;}
   uintmax_t getMaxsize() const {return s_maxsize;}
   uint64_t getMaxtime() const {return s_maxtime;}
   uint16_t getDivider() const {return s_divider;}

   // Set parameters
   void setParameters(
         uintmax_t maxsize,
         uint64_t maxtime,
         uint16_t divider
         )
   {
      Time ttime;

      s_maxsize = maxsize;
      s_maxtime = maxtime;
      s_divider = divider;

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         std::ostringstream s;
         s << "Parameters set for log type '" << s_logname << "':" << std::endl;
         s << "Max size: " << s_maxsize/1000 << " kbytes" << std::endl;
         ldiv_t d = ldiv(s_maxtime/Time::s_minute, 60);
         s << "Max time: " << d.quot << ":" << std::setfill('0') << std::setw(2)
           << d.rem << " hours" << std::endl;
         s << "Divider:  " << s_divider << " %";
         logger.event(WHERE__, s.str());
      }
   }

   int getNoEndpoints() const {return s_noEP;}

private:
   static const t_logtype s_logtype = logtype;
   static const t_loggroup s_loggroup;
   static const std::string s_logname;
   static const std::string s_shortname;
   static const std::string s_commonname;
   static const fs::path s_path;
   static const std::string s_fileprefix;
   static const std::string s_fileext;
   static const boost::regex s_logfile;
   static const boost::regex s_tempfile;
   static const uint16_t s_apzset;
   static const t_headertype s_headertype;
   static const bool s_hasmsgno;
   static const bool s_hasxmno;

   static uintmax_t s_maxsize;      // Max size in bytes
   static uint64_t s_maxtime;       // Max time in microseconds
   static uint16_t s_divider;       // Divider value in %

   static int s_noEP;               // Number of endpoints
};

}

#endif // PARAMETERS_H_
