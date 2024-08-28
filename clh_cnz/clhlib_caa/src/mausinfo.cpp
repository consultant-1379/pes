//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      mausinfo.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2014. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for handling MAU information.
//      
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      XDT/DEX XLOBUNG (Long Nguyen)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2014-03-06   XLOBUNG     MAUS logging.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "mausinfo.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace PES_CLH;

namespace PES_CLH {

const string MAUSInfo::m_endpoints[] = {"cpa", "cpb", "apa", "apb"};

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
MAUSInfo::MAUSInfo():
      m_noEndpoints(0),
      m_path(""),
      m_parentpath(""),
      m_index(-1),
      m_isvalall(false)
{
}


//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
MAUSInfo::~MAUSInfo()
{
}

//----------------------------------------------------------------------------------------
// Set Path
//----------------------------------------------------------------------------------------
void MAUSInfo::setPath(string path)
{
   m_path = path;
}

//----------------------------------------------------------------------------------------
// Get Path
//----------------------------------------------------------------------------------------
string MAUSInfo::getPath() const
{
   return m_path;
}

//----------------------------------------------------------------------------------------
// Set Parent Path
//----------------------------------------------------------------------------------------
void MAUSInfo::setParentPath(string path)
{
   m_parentpath = path;
}

//----------------------------------------------------------------------------------------
// Get Parent Path
//----------------------------------------------------------------------------------------
string MAUSInfo::getParentPath() const
{
   return m_parentpath;
}

//----------------------------------------------------------------------------------------
// Check if an endpoint exists
//----------------------------------------------------------------------------------------
bool MAUSInfo::epExist() const
{
   return (m_index >= 0 || m_isvalall);
}

//----------------------------------------------------------------------------------------
// Check if value "all" exists
//----------------------------------------------------------------------------------------
bool MAUSInfo::checkValueAll() const
{
   return m_isvalall;
}

//----------------------------------------------------------------------------------------
// Find endpoint
//----------------------------------------------------------------------------------------
bool MAUSInfo::find(const string& name)
{
   for (int i = 0; i < EPMAX; i++)
   {
      if (name == m_endpoints[i])
      {
         m_index = i;
         return true;
      }
   }
   
   if (name == "all")
   {
      m_isvalall = true;
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------------------
// Get Index
//----------------------------------------------------------------------------------------
int MAUSInfo::getIndex() const
{
   return m_index;
}


//----------------------------------------------------------------------------------------
// Check if the system is running with SwMau
//----------------------------------------------------------------------------------------
bool MAUSInfo::checkSwMauRunning()
{
   // This is to simulate the case which SWMAU is running
   // A file is created at /data/apz/
   // Filename: SWMAU.info

   const string& filepath = "/data/apz/SWMAU.info";
   ifstream fs(filepath.c_str());

   if (fs.is_open())
   {
      fs.close();
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------------------
// Get EP name
//----------------------------------------------------------------------------------------
string MAUSInfo::getEPName(int index)
{
   return m_endpoints[index];
}

}
