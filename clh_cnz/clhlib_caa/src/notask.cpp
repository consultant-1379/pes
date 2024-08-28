//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      notask.cpp
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
//      PA1    2011-08-25   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "notask.h"

using namespace std;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
NoTask::NoTask():
BaseTask()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
NoTask::~NoTask()
{	
}

//----------------------------------------------------------------------------------------
// Open log
//----------------------------------------------------------------------------------------
void NoTask::open()
{
}

//----------------------------------------------------------------------------------------
// Close log
//----------------------------------------------------------------------------------------
void NoTask::close()
{
}

//----------------------------------------------------------------------------------------
// Read event log (Not implemented for RUF type logs)
//----------------------------------------------------------------------------------------
Period NoTask::readEvents(
       const Period&,
       const Filter&,
       t_eventcb,
       std::ostream&
       ) const
{
   return Period();
}

//----------------------------------------------------------------------------------------
// List time for first and last event
//----------------------------------------------------------------------------------------
Period NoTask::listEvents(const Period&) const
{
   return Period();
}

//----------------------------------------------------------------------------------------
// Transfer log files for a CP or blade
//----------------------------------------------------------------------------------------
void NoTask::transferLogs(const Period&, const Filter&) const
{
}

}
