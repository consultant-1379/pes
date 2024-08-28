//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      seltask.h
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
//      Class for the system event logs (SEL).
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

#ifndef SELTASK_H_
#define SELTASK_H_

#include "parameters.h"
#include "appendtask.h"
#include "boardinfo.h"
#include "logger.h"
#include "common.h"
#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include <iomanip>

namespace PES_CLH {

class SelTask: public AppendTask
{
public:

   // Constructor
   SelTask();

   // Destructor
   virtual ~SelTask();

   // Act on log event
   void event(
         const fs::path& path
         );

   // Insert event message
   void insertSEL(
         int oid,                      // OID
         const Magazine& magazine,     // Magazine id
         const Slot& slot,             // Slot position
         const std::string& data       // Data message
         );

   // Get log parameters
   const BaseParameters& getParameters() const;

   // Get parent log path
   fs::path getParentDir() const {return "";}

   // Get a log directory
   fs::path getLogDir() const;

   // Get a log directory to stor AP2 Non-CPUB SEL log
   fs::path getLogDirAP2() const;

   // Create a log directory
   void createLogDir() const;

   // Save event to file
   void saveToFile(const fs::path& path,
         const std::string& data,       // Data message
         uintmax_t size                 // Data size
         );

   // Create a tmp subfile name based on time
   std::string createTmpFileName(       // Returns the file name
         const Time& time               // Time
         );

   // Read SEL events
   Period readSELEvents(                // Returns time for first and last event
         const Period& period,          // Time period
         const Filter& filter,          // Search filter
         t_eventcb eventcb = 0,         // Event callback function
         std::ostream& os = std::cout,  // Outstream
         bool checkonly = false         // Only for checking
         ) const;

   // Transfer SEL log files
   std::string transferSELLogs(
         const Period& period,          // Time period
         const Filter& filter          // Search filter
         );

private:
   // Stream textual information about the log entry
   void stream(std::ostream& s) const;

   Parameters<e_sel> m_parameters;
};

}

#endif // SELTASK_H_
