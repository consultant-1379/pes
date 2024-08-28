//----------------------------------------------------------------------------------------
//
//  FILE
//      pes_clhapi.cpp
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
//      Class for for listing and transferring CPS trace logs.
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

#include "pes_clhapi.h"
#include "pes_clhapi_impl.h"
#include <string>
#include <string.h>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Pes_clhapi::Pes_clhapi():
m_apiptr(0)
{
   m_apiptr = new PES_CLH::Api_impl;
}

//----------------------------------------------------------------------------------------
//   Destructor
//----------------------------------------------------------------------------------------
Pes_clhapi::~Pes_clhapi()
{
   delete m_apiptr;
}

//----------------------------------------------------------------------------------------
// Read trace log, receive events in the callback, the call is blocked until all data
// has been received 
//----------------------------------------------------------------------------------------
int Pes_clhapi::readLog_p(
                  const char* cpname,
                  const char* cpside,
                  char* &startdate,
                  char* &starttime,
                  char* &stopdate,
                  char* &stoptime,
                  t_eventcb eventcb
                  ) const
{
   return m_apiptr->readLog(
                     cpname,
                     cpside,
                     startdate,
                     starttime,
                     stopdate,
                     stoptime,
                     eventcb
                     );
}

//----------------------------------------------------------------------------------------
// Write logs to a destination
//----------------------------------------------------------------------------------------
int Pes_clhapi::transferLogs_p(
                  const char* cpname,
                  const char* startdate,
                  const char* starttime,
                  const char* stopdate,
                  const char* stoptime,
                  desttype_t desttype
                  ) const
{
   return m_apiptr->transferLogs(
                     cpname,
                     startdate,
                     starttime,
                     stopdate,
                     stoptime,
                     desttype
                     );
}

//----------------------------------------------------------------------------------------
// Get error text
//----------------------------------------------------------------------------------------
const char* Pes_clhapi::getErrorText_p() const
{
   return m_apiptr->getErrorText();
}
