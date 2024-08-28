//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      tru64task.h
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
//      Class for the True64 type logs.
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
//      PA1    2012-09-20   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef TRU64TASK_H_
#define TRU64TASK_H_

#include "dirtask.h"
#include <map>

namespace fs = boost::filesystem;

namespace PES_CLH {

class Tru64Task: public DirTask
{
public:
   // Constructor
   Tru64Task();

   // Destructor
   virtual ~Tru64Task();

   // Open log
   void open();

   // Close log
   void close();

   // Insert event log
   void insert(
         const fs::path& path           // Log file
         );

private:
   typedef std::map<boost::regex, bool> SUBFILELIST;
   typedef SUBFILELIST::iterator SUBFILELISTITER;

   SUBFILELIST m_subfilelist;          // List of subfiles
   std::string m_currentdir;           // Current log directory
};

}

#endif // TRU64TASK_H_
