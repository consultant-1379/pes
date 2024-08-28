//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      eventhandler.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2011. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for ACS event handling.
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
//      PA1    2011-09-08   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

#include <acs_aeh_evreport.h>
#include <string>

namespace PES_CLH {

class EventHandler
{
public:

   // Initialize the event handler
   static void init(
         const std::string& processname         // Process name
         );

   // Report an event to ACS event handler
   static void send(
         uint32_t error,                        // Error code
         const std::string& text                // Text
         );

private:

   static acs_aeh_evreport s_evreport;
   static std::string s_processname;
   static const uint32_t s_clhevent;
};

}

#endif // EVENTHANDLER_H_
