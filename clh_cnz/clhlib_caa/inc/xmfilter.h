//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      xmfilter.h
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
//      Class for filtering logs based on XM number.
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
//      PA1    2012-09-27   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef XMFILTER_H_
#define XMFILTER_H_

#include "filter.h"
#include <string>
#include <list>

namespace PES_CLH {

class XmFilter: public Filter
{
public:
   // Constructors
   XmFilter(const std::string& str);
   XmFilter();

   // Destructor
   ~XmFilter();

   // Check if filter is empty
   bool empty() const;              // Returns true if empty, false otherwise

   // Test filter match
   bool test(                       // Return true if matched, false otherwise
         uint16_t xmno              // XM number to be matched towards the filter
         ) const;

   // Test filter match
   bool test(                       // Return true if matched, false otherwise
         const std::string& str     // String containing XM number to be matched
                                    // towards the filter
         ) const;

   static const uint16_t s_maxmno;

private:
   typedef std::pair<uint16_t, uint16_t> PAIR;
   typedef std::list<PAIR> XMLIST;
   typedef XMLIST::const_iterator XMLISTCITER;

   uint16_t getXmno();

   std::string m_str;
   std::string::const_iterator m_iter;
   XMLIST m_xmlist;
   bool m_kernel;
   bool m_parentx;
};

}

#endif // XMFILTER_H_
