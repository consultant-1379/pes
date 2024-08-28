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
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      XDT/DEK/XVUNGUY
//
//  REVISION HISTORY
//      Rev.      Date         Prepared    Description
//      ----      ----         --------    -----------
//      PA1       2013-03-01   XVUNGUY     CLH adapted to APG43 on Linux.
//      PA2       2013-08-21   XLOBUNG     Add comments.
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

namespace PES_CLH{
//============================================
// class RPLogItem
//============================================
class RPLogItem
{
public:
   // Constructor
   RPLogItem(
         const string logname,            // Log name
         const string mag,                // Magazine
         const string slot,               // Slot
         const string rpno,               // RP number
         const string start_time,         // Start time
         const string filename            // Filename
   );
   
   // Destructor
   ~RPLogItem(void);
   
   // Get log name
   const string getLogName() const;
   
   // Get magazine
   const string getMag() const;
   
   // Set slot number
   const string getSlot() const;
   
   // Get RP number
   const string getRPNo() const;
   
   // Set start time
   void setStartTime(const string& time);
   
   // Get start time
   const string getStartTime() const;
   
   // Set stop time
   void setStopTime(const string& time);
   
   // Get stop time
   const string getStopTime() const;
   
   // Get filename
   const string getFileName() const;

private:
   string m_logname;                // Log name
   string m_mag;                    // Magazine
   string m_slot;                   // Slot
   string m_rpno;                   // RP number
   string m_startTime;              // Start time
   string m_stopTime;               // Stop time
   string m_fileName;               // Filename
};

}
#endif // RP_LOG_ITEM_H_
