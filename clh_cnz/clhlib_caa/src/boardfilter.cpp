//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      boardfilter.cpp
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
//      PA1    2011-09-08   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "boardfilter.h"
#include "exception.h"
#include <boost/lexical_cast.hpp>

using namespace std;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructors  
//----------------------------------------------------------------------------------------
BoardFilter::BoardFilter():
Filter(),
m_magazine(~uint32_t()),
m_slot(~uint16_t())
{
}

BoardFilter::BoardFilter(const Magazine& magazine, const Slot& slot):
Filter(),
m_magazine(magazine), 
m_slot(slot)
{
}

//----------------------------------------------------------------------------------------
// Destructor     
//----------------------------------------------------------------------------------------
BoardFilter::~BoardFilter()
{
}

//----------------------------------------------------------------------------------------
// Check if filter is empty
//----------------------------------------------------------------------------------------
bool BoardFilter::empty() const
{
   return m_magazine == ~uint32_t();
}

//----------------------------------------------------------------------------------------
// Test based on subrack id and slot number     
//----------------------------------------------------------------------------------------
bool BoardFilter::test(const string& str) const
{
   if (m_magazine == ~uint32_t()) return true;

   const string s_trap("Trap OID: ");
   const string s_subrack("Subrack id: ");   
   const string s_slot("Slot number: ");

   string value;
   size_t pos(0);
   try
   {
      value = getItem(s_trap, str, pos);
      //boost::lexical_cast<uint16_t>(value);

      // Parse subrack id
      value = getItem(s_subrack, str, pos);
      Magazine magazine(value);

      // Parse slot number
      value = getItem(s_slot, str, pos);
      Slot slot(value);

      return (magazine == m_magazine) && (slot == m_slot);
   }
   catch (exception&)
   {
      return false;
   }
}

//----------------------------------------------------------------------------------------
// Get item from stream    
//----------------------------------------------------------------------------------------
string BoardFilter::getItem(
                        const string& label, 
                        const string& str, 
                        size_t& pos
                        ) const
{
   if (str.find(label, pos) != pos)
   {   
      throw exception();
   }
   size_t size = label.size();   
   size_t pos1 = pos + size;
   pos = str.find_first_of('\n', pos1);   
   return str.substr(pos1, pos++ - pos1);
}

}
