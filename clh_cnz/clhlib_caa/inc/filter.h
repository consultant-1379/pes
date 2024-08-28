//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      filter.h
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
//      Base class for log filters.
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

#ifndef FILTER_H_
#define FILTER_H_

#include <iostream>

namespace PES_CLH {

//========================================================================================
// Class Filter
//========================================================================================

class Filter
{
public:
   // Constructor
   Filter()
   {
   };

   // Destructor
   virtual ~Filter() {};

   // Check if filter is empty
   virtual bool empty() const = 0;  // Returns true if empty, false otherwise

   // Test filter match
   virtual bool test(               // Return true if matched, false otherwise
         const std::string& str     // String to be matched towards the filter
         ) const = 0;

private:
};

class NoFilter: public Filter
{
public:
   // Constructor
   NoFilter()
   {
   };

   // Destructor
   virtual ~NoFilter() {};

   // Check if filter is empty
   bool empty() const               // Returns true
   {
      return true;
   }

   // Test filter match
   bool test(                      // Returns true
         const std::string&        // Not used
         ) const
   {
      return true;
   }

private:
};

}

#endif // FILTER_H_
