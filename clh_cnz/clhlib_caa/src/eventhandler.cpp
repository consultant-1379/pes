//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      eventhandler.cpp
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
//      PA1    2012-05-04   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <sstream>
#include <cassert>
#include <stdint.h>

#include "eventhandler.h"
#include "logger.h"

using namespace std;

namespace PES_CLH {

acs_aeh_evreport EventHandler::s_evreport;
string EventHandler::s_processname;
const uint32_t EventHandler::s_clhevent = 32000;

//----------------------------------------------------------------------------------------
// Initialize the event handler with process name
//----------------------------------------------------------------------------------------
void EventHandler::init(const string& processname)
{
   s_processname = processname;
}

//----------------------------------------------------------------------------------------
// Report an event to ACS event handler
//----------------------------------------------------------------------------------------
void EventHandler::send(uint32_t error, const string& text)
{
   ostringstream s;
   s << s_processname << ":" << getpid();
   uint32_t eventNumber;
   string problemText;

   if (error == Exception::parameter().first)
   {
      // Parameter error
      eventNumber = s_clhevent + 1;
      problemText = text;
   }
   else if (error == Exception::system().first)
   {
      // System error
      eventNumber = s_clhevent + 2;
      problemText = text;
   }
   else if (error == Exception::internal().first)
   {
      // Internal error
      eventNumber = s_clhevent + 3;
      problemText = text;
   }
   else
   {
      // Other errors are handled as internal errors
      eventNumber = s_clhevent + 3;
      problemText = Exception::internal().second + text;
   }

   ACS_AEH_ReturnType ret;
   const char cause[] = "Central Log Handler fault";

   ret = s_evreport.sendEventMessage(
                           s.str().c_str(),      // Process name
                           eventNumber,           // Specific problem
                           "EVENT",              // Severity
                           cause,                // Cause
                           "APZ",                // Object class of reference
                           "",                   // Object of reference
                           "",                   // Problem data
                           problemText.c_str()   // Problem text
                           );

   if (ret == ACS_AEH_error)
   {
      ACS_AEH_ErrorType error = s_evreport.getError();
      string errormsg;
      switch (error)
      {
         case ACS_AEH_syntaxError:
            errormsg = "Syntax error in event message.";
            break;

         case ACS_AEH_eventDeliveryFailure:
            errormsg = "Failed to deliver event.";
            break;

         default:
            errormsg = "Unknown error.";
      }

      Logger::event(LOG_LEVEL_WARN, WHERE__, errormsg);
   }
}

}

