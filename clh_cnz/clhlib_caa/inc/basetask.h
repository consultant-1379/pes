//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      basetask.h
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
//      Base class for the task classes.
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
//      PA1    2012-05-08   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef BASETASK_H_
#define BASETASK_H_

#include "parameters.h"
#include "cpinfo.h"
#include "ltime.h"
#include "filter.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iostream>

namespace fs = boost::filesystem;

namespace PES_CLH {

class BaseTask
{
   friend std::ostream& operator<<(std::ostream& s, const BaseTask& logtask);

public:

   typedef void (*t_eventcb)(
         const Time& cptime,
         const Time& aptime,
         size_t size,
         const char* buf,
         std::ostream& s
         );

   // Constructor
   BaseTask();

   // Destructor
   virtual ~BaseTask();

   // Open log
   virtual void open() = 0;

   // Close log
   virtual void close() = 0;

   // Act on log event
   virtual void event(
         const fs::path& path
         ) = 0;

   // Read events from log files
   virtual Period readEvents(
           const Period& period,
           const Filter& filter,
         t_eventcb eventcb = 0,
         std::ostream& os = std::cout
         ) const = 0;

   // List time for first and last event
   virtual Period listEvents(
           const Period& period
           ) const = 0;

   // Transfer log files for a CP or blade
   virtual void transferLogs(
         const Period& period,
         const Filter& filter
         ) const = 0;

   // Get log parameters
   virtual const BaseParameters& getParameters() const = 0;

   // Get log parent directory
   virtual fs::path getParentDir() const = 0;

   // Get log directory
   virtual fs::path getLogDir() const = 0;

   // Create log directory
   virtual void createLogDir() const = 0;

   // Set value if this is the hanlder for Non-CPUB
   void setNonCPUB(bool noncpub);

   // Check if this is the handler for Non-CPUB
   bool isNonCPUB() const;

   // Special use for AP2
   void setAP2();

   // Open for SEL AP2
   void openSELAP2();

protected:
   const BaseTask& operator=(const BaseTask& log);

   // Stream textual information about the log entry
   virtual void stream(std::ostream& s) const = 0;

   // Caculate log size
   size_t calculateLogSize() const;


   uintmax_t m_logsize;               // Size of log
   bool m_isopen;                     // True if log is opened
   bool m_isnoncpub;                  // Used for SEL log
   bool m_issetap2;                   // Used for SEL log

private:
   static const std::string s_datepattern;
   static const std::string s_extdatepattern ;
   static const std::string s_xmpattern;
   static const uint16_t s_apzCpub;
   static const uint16_t s_apzAll;
};

}

#endif // BASETASK_H_
