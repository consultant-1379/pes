//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      filetask.h
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
//      Class for the file type logs.
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
//      PA1    2012-09-18   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef FILETASK_H_
#define FILETASK_H_

#include "basetask.h"
#include <boost/filesystem/fstream.hpp>
#include <deque>

namespace fs = boost::filesystem;

namespace PES_CLH {

class FileTask: public BaseTask
{
public:
   // Constructor
   FileTask();

   // Destructor
   virtual ~FileTask();

   // Open log
   void open();

   // Close log
   void close();

   // Insert event log
   void insert(
         const fs::path& source,         // Log file
         const std::string& prefix,      // File prefix
         const Time& time,               // CP time
         uintmax_t size                  // Data size
         );

   // Read log header
   void readHeader(
         const fs::path& file,           // Log file
         Time& time,                     // CP time
         uintmax_t& size                 // Data size
         );

   // Read events
   Period readEvents(                    // Returns time for first and last event
         const Period& period,           // Time period
         const Filter& filter,           // Search filter
         t_eventcb eventcb = 0,          // Event callback function
         std::ostream& os = std::cout    // Outstream
         ) const;

   // List time for first and last event
   Period listEvents(                    // Returns time for first and last event
         const Period& period            // Time period
         ) const;

   // Transfer log files for a CP or blade
   void transferLogs(
         const Period& period,           // Time period
         const Filter& filter            // Search filter
         ) const;

protected:
   typedef std::pair<Time, uint16_t> PAIR;
   typedef std::deque<std::string> FILELIST;
   typedef FILELIST::iterator FILELISTITER;
   typedef FILELIST::const_iterator FILELISTCITER;

   // Parse a log subfile name
   static PAIR parseFileName(
         const std::string& file
         );

private:

   static bool compareByDate(
         const std::string& file1,
         const std::string& file2
         );

   void copyFile(
         const fs::path& source,
         const fs::path& target,
         ssize_t offset
         ) const;

   FILELISTITER getDivIter();

   bool maintainLogSize();


   FILELIST m_filelist;                  // Log file list

private:
   // Disable default copy constructor
   FileTask(const FileTask&);

   // Disable default assignment operator
   FileTask& operator=(const FileTask&);
};

}

#endif // FILETASK_H_
