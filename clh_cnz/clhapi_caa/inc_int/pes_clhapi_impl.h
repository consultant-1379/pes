//----------------------------------------------------------------------------------------
//
//  FILE
//      pes_clhapi_impl.h
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
//      Implementation class for for listing and transferring CPS trace logs.
//
//  ERROR HANDLING
//      -
//
//  DOCUMENT NO
//      190 89-CAA 109 1426  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2012-10-05   UABTSO      New API created for listing and transferring
//                                      CPS trace logs.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------

#ifndef PES_CLHAPI_IMPL_H_
#define PES_CLHAPI_IMPL_H_

#include "pes_clhapi.h"
#include <ltime.h>
#include <cpinfo.h>
#include <boost/filesystem.hpp>
#include <string>

namespace fs = boost::filesystem;

namespace PES_CLH {

class Api_impl
{
public:

   // Constructor
   Api_impl();

   // Destructor
   ~Api_impl();

   // Read trace log, receive events in the callback, the call is blocked until all data
   // has been received 
   int readLog(
         const std::string& cpname,       // CP name, for a one CP system an empty string is provided
         const std::string& cpside,       // CP side, "A" or "B" for a one CP system or SPX and an
                                          // empty string for a single sided CP system. The deprecated
                                          // "CPA" and "CPB" are kept for backward compatibility.
         char* &startdate,                // Start date, returns date for first log greater than start date.
         char* &starttime,                // Start time, returns time for first log greater than start time.
         char* &stopdate,                 // Stop date, returns date for first log less than stop date.
         char* &stoptime,                 // Stop time, returns time for first log less than stop time.
         Pes_clhapi::t_eventcb eventcb    // Call back method.
         );

   // Write logs to a destination
   int transferLogs(
         const std::string& cpname,       // CP name, in a one CP system an empty string is provided.
         const char* startdate,           // Start date
         const char* starttime,           // Start time
         const char* stopdate,            // Stop date
         const char* stoptime,            // Stop time
         desttype_t desttype              // Destination type.
         );

   // Get error text
   const char* getErrorText() const;

private:

   // Get CP side
   t_cpSide getCpSide(
         const std::string& str
         ) const;

   // List events
   void listEvents(
         const CPInfo& cpinfo,
         t_cpSide cpside,
         const Period& period
         ) const;

   void transferLogs(
         const CPInfo& cpinfo,
         const Period& period
         ) const;

   void transferLogs(
         const Period& period
         ) const;

   static void getLogEvent(
         const Time& cptime,
         const Time& aptime,
         size_t size,
         const char* buf,
         std::ostream& os
         );

   std::string m_errortext;

   static const char s_success[];
   static Pes_clhapi::t_eventcb s_eventcb;
};

}

#endif // PES_CLHAPI_IMPL_H_

