//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rploglist.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013. All rights reserved.
//
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for describing list of RP log file.
//      Most of the codes are based on rploglist.h on APG43.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424
//
//  AUTHOR
//      XDT/DEK/XVUNGUY
//
//  REVISION HISTORY
//      Rev.      Date         Prepared    Description
//      ----      ----         --------    -----------
//      R2A04     2013-03-01   XVUNGUY     CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef RP_LOG_LIST_H_
#define RP_LOG_LIST_H_

#include <list>
#include <vector>
#include "ltime.h"
#include "rplogitem.h"

using namespace std;

namespace PES_CLH
{
//============================================
// class RPLogList
//============================================
class RPLogList
{

public:
   // Constructor
   RPLogList(void);

   // Destructor
   ~RPLogList(void);

   // Print
   void print(bool isOptDir);

   // List RP logs
   void listRpLogs(
         const std::string& rpNum,
         const Time& staTime,
         const Time& stoTime,
         bool isOptDir
         );

   // Get RP log list
   std::list<RPLogItem>& getRpLogList(void);

private:
   // Add one RP log file to the list
   void add(const RPLogItem& item, bool isOptDir);

   // Display format with "-d" option
   void printOptD();

   // Display format with "-g" option
   void printOptF();

   // Get all RP log files stored in /data/apz/logs/RP/ path
   void listFiles(std::vector<std::string>& database);

private:
   list<RPLogItem> m_rpLogItems;          // RP log items

};

}

#endif // RP_LOG_LIST_H_

