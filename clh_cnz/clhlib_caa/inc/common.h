//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      common.h
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
//      Class for common routines.
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
//      PA1    2012-11-07   UABTSO      CLH adapted to APG43 on Linux.
//      R3A    2013-06-19   XVUNGUY     Add function to get AP node name
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace PES_CLH {

class Common
{
public:
   enum ArchitectureValue
   {
      SCB =          0,
      SCX =          1,
      DMX =          2,
      VIRTUALIZED =  3
   };

   enum ApNodeName
   {
       UNKNOW = -1,
       AP1 = 0,
       AP2 = 1
   };
   // Get software version
   static std::string getVersion(               // Returns software version information
         const std::string& swunit              // Software unit
         );

   // Create a process lock
   static bool createLock(
         const std::string& cmdname,            // Name of process executable
         bool isCmd,                            // True if it is command
         const std::string& opt                 // -t option
         );

   // Execute a shell command
   static int executeCommand(
         const std::string& command,            // Command string
         std::ostream& os                       // Stream containing output
         );

   // Create archive file
   static void archive(
         const fs::path& source,                // Source path
         const fs::path& dest                   // Destination path
         );

   // Get data disk path
   static fs::path getDataDiskPath(             // Returns data disk path
         const std::string& logicalName         // Logical disk path
         );

   // Get file management path
   static fs::path getFileMPath(                // Returns file management path
         const std::string& logicalName         // Logical disk path
         );

   // Create a symbolic link
   static void createSymLink(
         const fs::path& path                   // Path to create symbolic link for
         );

   // Create directory and a symbolic link to the directory
   static void createDirAndLink(
         const fs::path& path                   // Path to directory
         );

   // Create directories and a symbolic links to the directories
   static void createDirsAndLinks(
         const fs::path& path                   // Path to directory
         );

   // Get CP type
   static uint32_t getCPType();                 // Returns CP type

   // Get node architecture
   static ArchitectureValue getNodeArchitecture();
                                                // Returns node architecture

   // Copy file to Media
   static void copyToMedia(
         const fs::path& path                   // Source path
         );

   // Get external NBI path from absolute path
   static fs::path getExtNbiPath(               // Returns NBI path
         const fs::path& path                   // File path
         );

   // Poll emfinfo until copying is done
   static void emfpoll(
         const std::string& filename            // Filename to check
         );

   // Check if emfcopy is running
   static bool isEmfBusy(
         );

   // Get AP node name
   static ApNodeName getApNode(void);

   static void mount(const char *source, const char *target);

   static void uMount(const char *target);

   static bool s_tracelog;                      // True if trace logging is enabled

private:

   //   Parse emfinfo line
   static std::string emfparse(
         std::istream& is,                      // input stream
         const std::string& keyword             // Key word
         );

   // Read emfinfo result
   static void emfinfo(
         std::string& state,                    // State
         std::string& result,                   // result
         std::string& filename                  // File name
         );
};

}

#endif // COMMON_H_
