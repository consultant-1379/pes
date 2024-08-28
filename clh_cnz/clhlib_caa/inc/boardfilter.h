//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      boardfilter.h
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
//      Class for filtering logs based on magazine id and slot position.
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

#ifndef BOARDFILTER_H_
#define BOARDFILTER_H_

#include "filter.h"
#include "boardinfo.h"
#include <string>

namespace PES_CLH {

class BoardFilter: public Filter
{
public:
   // Constructors
   BoardFilter();
   BoardFilter(const Magazine& magazine, const Slot& slot);

   // Destructor
   virtual ~BoardFilter();

   // Check if filter is empty
   bool empty() const;               // Returns true if empty, false otherwise

   // Test filter match
   bool test(                        // Return true if matched, false otherwise
         const std::string& str      // String to be matched towards the filter
         ) const;

private:
   std::string getItem(              // Returns item to be parsed
         const std::string& label,   // Label to be parsed
         const std::string& str, 
         size_t& pos
         ) const;

   Magazine m_magazine;
   Slot m_slot;
};

}

#endif // BOARDFILTER_H_
