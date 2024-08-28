//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      appendtask.h
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
//      Class for the append type logs.
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
//      PA1    2012-03-01   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef APPENDTASK_H_
#define APPENDTASK_H_

#include "basetask.h"
#include <boost/filesystem/fstream.hpp>
#include <iostream>

namespace fs = boost::filesystem;

namespace PES_CLH {

void printLogEvent(
      const Time& cptime,
      const Time& aptime,
      size_t size,
      const char* buf,
      std::ostream& os
      );

class AppendTask: public BaseTask
{
public:

   // Constructor
   AppendTask();

   // Destructor
   virtual ~AppendTask();

   // Open log
   void open();

   // Close log
   void close();

   // Check integrity of log file
   void checkIntegrity(
         fs::ifstream& fs,             // File stream
         uintmax_t size                // File size
         ) const;

   // Insert event message
   void insert(
         const Time& cptime,           // CP time
         const char* data,             // Data message
         uintmax_t size,               // Data size
         bool issel = false            // Is Sel log
         );

   // Read events
   Period readEvents(                  // Returns time for first and last event
         const Period& period,         // Time period
         const Filter& filter,         // Search filter
         t_eventcb eventcb = 0,        // Event callback function
         std::ostream& os = std::cout  // Outstream
         ) const;

   // List time for first and last event
   Period listEvents(                  // Returns time for first and last event
         const Period& period          // Time period
         ) const;

   // Transfer log files for a CP or blade
   void transferLogs(
         const Period& period,         // Time period
         const Filter& filter          // Search filter
         ) const;

   // Read all messages from a temporary log file
   void readMsgs(
         const fs::path& file
           );

   // Set Flag for handling SEL log quota
   void enableSELDivider();

protected:
   typedef std::pair<Time, Time> PAIR;
   typedef std::deque<PAIR> FILELIST;
   typedef FILELIST::iterator FILELISTITER;
   typedef FILELIST::const_iterator FILELISTCITER;
   typedef FILELIST::reverse_iterator FILELISTRITER;
   typedef FILELIST::const_reverse_iterator FILELISTCRITER;

   // Parse a log subfile name
   Time parseFileName(                 // Returns the time
         const std::string& file       // Subfile name
         ) const;

   // Create file
   void createFile(
         const fs::path& path          // File to create
         );

   // Open file
   void openFile(
         const fs::path& path          // File t open
         );

   fs::fstream m_fs;                   // File stream object

   bool m_seldivider;                  // Used for SEL log

private:
   // Disable default copy constructor
   AppendTask(const AppendTask&);

   // Disable default assignment operator
   AppendTask& operator=(const AppendTask&);

protected:
#pragma pack(push)                     // Push current alignment to stack
#pragma pack(4)                        // Set alignment to 4 bytes boundary

   struct t_header
   {
      size_t m_prev;                   // Address to previous record
      int64_t m_cptime;                // CP time
      int64_t m_aptime;                // AP time
      size_t m_size;                   // Size of log event
   };

#pragma pack(pop)                      // Restore original alignment from stack
   // Create a log subfile name based on time
   std::string createFileName(         // Returns the file name
          const Time& time             // Time
          ) const;

   // Parse XM number
   bool parseXmNo(                     // Returns true if XM number was found,
                                       // false otherwise
         const char* data,             // Data string to parse
         uint16_t& xmno                // XM number returned
         ) const;

   // Get divider value
   FILELISTITER getDivIter();

   void maintainLogSize();

   template<typename T>
   static void writeData(
         std::ostream& fs,
         const T& t
         )
   {
      fs.write(reinterpret_cast<const char*>(&t), sizeof(T));
   }

   template<typename T>
   static void readData(
         std::istream& fs,
         T& t
         )
   {
      fs.read(reinterpret_cast<char*>(&t), sizeof(T));
   }

   void dumpHeader(
         t_header header
         ) const;   // Test ***


   FILELIST m_filelist;         // Log file list
   char* m_data;               // Data buffer
   static const boost::regex s_msgnopattern;
};

}

#endif // APPENDTASK_H_
