//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dirtask.h
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
//      Class for the directory type logs.
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

#ifndef DIRTASK_H_
#define DIRTASK_H_

#include "basetask.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/thread.hpp>
#include <deque>

namespace fs = boost::filesystem;

namespace PES_CLH {

class DirTask: public BaseTask
{
public:
   // Constructor
   DirTask();

   // Destructor
   virtual ~DirTask();

   // Open log
   void open();

   // Close log
   void close();

   // Insert event log
   void insert(
         const fs::path& path,          // Log file
         int delay = 0                  // Time to wait for subfiles
         );

   // Read events
   Period readEvents(
         const Period& period,          // Time period
         const Filter& filter,          // Search filter
         t_eventcb eventcb = 0,         // Event callback function
         std::ostream& os = std::cout   // Outstream
         ) const;

   // List time for first and last event
   Period listEvents(
         const Period& period           // Time period
         ) const;

   // Transfer log files for a CP or blade
   void transferLogs(                   // Returns time for first and last event
         const Period& period,          // Time period
         const Filter&                  // Search filter
         ) const;

protected:
   typedef std::deque<std::string> FILELIST;
   typedef FILELIST::iterator FILELISTITER;
   typedef FILELIST::const_iterator FILELISTCITER;

   // Parse a log subfile name
   static Time parseFileName(           // Returns the time
         const std::string& file        // Subfile name
         );

   static bool compareByDate(
         const std::string& file1,
         const std::string& file2
         );

   FILELISTITER getDivIter();

   void maintainLogSize();


   FILELIST m_filelist;                 // Log file list
   boost::thread_group m_threadgroup;   // Thread group
   boost::mutex m_mutex;               // Mutex

private:
   // Disable default copy constructor
   DirTask(const DirTask&);

   // Disable default assignment operator
   DirTask& operator=(const DirTask&);
};

}

#endif // DIRTASK_H_
