//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      xmfilter.cpp
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
//      Class for handling XM numbers.
//      
//  ERROR HANDLING
//      Class for filtering logs based on XM number.
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
//      PA1    2012-09-25   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <boost/lexical_cast.hpp>
#include <stdint.h>

#include "xmfilter.h"
#include "exception.h"

using namespace std;

namespace PES_CLH {

const uint16_t XmFilter::s_maxmno = 4095;

//----------------------------------------------------------------------------------------
// Constructors   
//----------------------------------------------------------------------------------------
XmFilter::XmFilter():
Filter(),
m_str(),
m_iter(),
m_xmlist(),
m_kernel(false),
m_parentx(false)
{
}

XmFilter::XmFilter(const string& str):
Filter(),
m_str(str),
m_iter(m_str.begin()),
m_xmlist(),
m_kernel(false),
m_parentx(false)
{
   enum t_state
   {
      e_begin,
      e_number1,
      e_number2,
      e_dash,
      e_comma,
      e_kernel,
      e_parentx,
      e_end
   };

   PAIR interval;
   t_state state = e_begin;

   do
   {
      switch (state)
      {
      case e_begin: 
      case e_comma: 
         if (m_iter == m_str.end())
         {
            throw Exception(Exception::errXmlist(), WHERE__);
         }
         switch (*m_iter)
         {
         case 'K':
            m_kernel = true;
            ++m_iter;
            state = e_kernel;
            break;

         case 'P':
            m_parentx = true;
            ++m_iter;
            state = e_parentx;
            break;

         default:
            interval.first = getXmno();
            state = e_number1;
            break;
         }
         break;

      case e_number1:
         if (m_iter == m_str.end())
         {
            interval.second = interval.first;
            m_xmlist.push_back(interval);
            state = e_end;
            break;
         }
         switch (*m_iter)
         {
         case '-':
            ++m_iter;
            state = e_dash;
            break;

         case ',':
            ++m_iter;
            interval.second = interval.first;
            m_xmlist.push_back(interval);
            state = e_comma;
            break;

         default:
            throw Exception(Exception::errXmlist(), WHERE__);
         }
         break;

      case e_number2: 
         if (m_iter == m_str.end())
         {
            m_xmlist.push_back(interval);
            state = e_end;
            break;
         }
         switch (*m_iter)
         {
         case ',':
            ++m_iter;
            m_xmlist.push_back(interval);
            state = e_comma;
            break;

         default:
            throw Exception(Exception::errXmlist(), WHERE__);
         }
         break;

      case e_dash:
         interval.second = getXmno();
         state = e_number2;
         break;

      case e_kernel:
      case e_parentx:
         if (m_iter == m_str.end())
         {
            state = e_end;
            break;
         }
         switch (*m_iter)
         {
         case ',':
            ++m_iter;
            state = e_comma;
            break;

         default:
            throw Exception(Exception::errXmlist(), WHERE__);
         }
         break;

      default:   
         throw Exception(Exception::errXmlist(), WHERE__);
      }
   }
   while (state != e_end);
}

//----------------------------------------------------------------------------------------
//   Destructor     
//----------------------------------------------------------------------------------------
XmFilter::~XmFilter()
{
}

//----------------------------------------------------------------------------------------
// Check if filter is empty
//----------------------------------------------------------------------------------------
bool XmFilter::empty() const
{
   return m_xmlist.empty();
}

//----------------------------------------------------------------------------------------
// Test filter match
//----------------------------------------------------------------------------------------
bool XmFilter::test(uint16_t xmno) const
{
   for (XMLISTCITER iter = m_xmlist.begin(); iter != m_xmlist.end(); ++iter)
   {
      const PAIR& interval = *iter;
      if ((xmno >= interval.first) && (xmno <= interval.second))
      {
         return true;
      }
   }
   return false;
}

//----------------------------------------------------------------------------------------
// Test filter match
//----------------------------------------------------------------------------------------
bool XmFilter::test(const std::string& str) const
{
   if (m_str.empty()) return true;

   uint16_t xmno;
   if (str.substr(0, 3) == "XM ")
   {
      size_t pos = str.find(':');
      if (pos == string::npos) return false;
      try
      {
         xmno = boost::lexical_cast<uint16_t>(str.substr(3, pos - 3));
      }
      catch (exception&)
      {
                     throw Exception(Exception::illXmno(xmno), WHERE__);
      }
      if (xmno < 1 || xmno > s_maxmno) return false;
   }
   else if (str.find("KERNEL:") == 0)
   {
      return m_kernel? true: false;
   }
   else if (str.find("PARENTX:") == 0)
   {
      return m_parentx? true: false;
   }
   else
   {
      return false;
   }

   return test(xmno);
}

//----------------------------------------------------------------------------------------
//   Get XM number from string 
//----------------------------------------------------------------------------------------
uint16_t XmFilter::getXmno()
{
   if (m_iter == m_str.end())
   {
      throw Exception(Exception::errXmlist(), WHERE__);
   }
   string::const_iterator titer = m_iter;
   while (titer != m_str.end())
   {
      if (isdigit(*titer))
      {
         ++titer;
      }
      else break;
   }
   if (m_iter == titer)
   {
      throw Exception(Exception::errXmlist(), WHERE__);
   }

   uint16_t xmno = static_cast<uint16_t>(boost::lexical_cast<uint16_t>(string(m_iter, titer)));
   if (xmno == 0 || xmno > s_maxmno)
   {
      throw Exception(Exception::illXmno(xmno), WHERE__);
   }
   m_iter = titer;
   return xmno;
}

}
