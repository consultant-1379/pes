//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      mausinfo.h
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
//      Class for handling SWMAU information.
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

#ifndef MAUSINFO_H_
#define MAUSINFO_H_

#include <string>
#include <set>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace PES_CLH {

//========================================================================================
// Class MAUSInfo
//========================================================================================

class MAUSInfo
{
public:
   // Constructor
   MAUSInfo();

   // Destructor
   virtual ~MAUSInfo();

   void setPath(std::string path);

   std::string getPath() const;

   void setParentPath(std::string path);

   std::string getParentPath() const;

   bool epExist() const;

   bool find(const std::string& name);

   int getIndex() const;

   bool checkValueAll() const;

   // Static methods
   // Check if the system is running with SwMau
   static bool checkSwMauRunning();

   static std::string getEPName(int index);

   static const int EPMAX = 4;
   
private:
   static const std::string m_endpoints[EPMAX];
   int m_noEndpoints;                        // Number of endpoints
   std::string m_path;                       // Path to MAUS log directories
   std::string m_parentpath;                 // Path to parent MAUS log directories
   int m_index;                              // Index of endpoint in m_endpoints
   bool m_isvalall;                          // Check if value "all" is set
};

}

#endif // MAUSINFO_H_
