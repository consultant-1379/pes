//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rplogitem.h
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
//      Class for describing each RP log file.
//      Most of the codes are based on rplogitem.h on APG43.
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

#ifndef RP_LOG_ITEM_H_
#define RP_LOG_ITEM_H_
#include <string>

using namespace std;

namespace PES_CLH
{
//============================================
// class RPLogItem
//============================================
class RPLogItem
{
public:
   // Constructor
   RPLogItem(
         const string logname,         // Log name
         const string mag,             // Magazine
         const string slot,            // Slot number
         const string rpno,            // RP number
         const string start_time,      // Start time
         const string filename         // Filename
   );

   // Destructor
   ~RPLogItem(void);

   // Get log name
   const string getLogName() const;

   // Get Magazine
   const string getMag() const;

   // Get Slot
   const string getSlot() const;

   // Get RP number
   const string getRPNo() const;

   // Set start time
   void setStartTime(const string& time);

   // Get Start time
   const string getStartTime() const;

   // Set Stop time
   void setStopTime(const string& time);

   // Get Stop time
   const string getStopTime() const;

   // Get filename
   const string getFileName() const;

private:
   string m_logname;                // Log name
   string m_mag;                    // Magazine
   string m_slot;                   // Slot number
   string m_rpno;                   // RP number
   string m_startTime;              // Start time
   string m_stopTime;               // Stop time
   string m_fileName;               // Filename
};

}

#endif // RP_LOG_ITEM_H_

