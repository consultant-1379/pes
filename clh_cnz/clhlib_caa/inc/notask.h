//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      notask.h
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
//      Class for an empty log task.
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

#ifndef NOTASK_H_
#define NOTASK_H_

#include "basetask.h"
#include <iostream>

namespace PES_CLH {

class NoTask: public BaseTask
{
public:

   // Constructor
   NoTask();

   // Destructor
   virtual ~NoTask();

   // Open log
   void open();

   // Close log
   void close();

   // Read event log
   Period readEvents(
         const Period& period,
         const Filter& xmfilter,
         t_eventcb eventcb = 0,
         std::ostream& os = std::cout
         ) const;

   // List time for first and last event
   Period listEvents(
         const Period& period
         ) const;

   // Transfer log files for a CP or blade
   void transferLogs(
         const Period& period,
         const Filter& xmfilter
         ) const;

private:

};

}

#endif // NOTASK_H_
