//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      common.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2015. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class contains common methods used by cfet.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 0xxx  PA1
//
//  AUTHOR
//      EAB/FLE/DF UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      A      2010-03-16   UABTSO      CFET was migrated to APG50.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <fstream>
#include <vector>
#include <cassert>
#include <ACS_APGCC_CommonLib.h>
#include <stdlib.h>
#include <ACS_CS_API.h>
#include "exception.h"

class Common
{
public:
    typedef std::vector<std::string> VECTOR;
    typedef VECTOR::const_iterator VECTORCITER;

    enum t_cptype {e_onecp, e_multicp};

    // Convert characters in string to upper case
    static std::string toUpper(                       // Returns converted string
            const std::string& str                    // String to convert
            );
    
    // Convert characters in string to lower case
    static std::string toLower(                       // Returns converted string
            const std::string& str                    // String to convert
            );

    // Translate string to integer value
    static int64_t strToInt(                          // Returns integer value
            const std::string& val                    // String to convert
            );
 
    // Check if a file exists
    static bool fileExists(                           // Returns true if found, false otherwise
            const std::string& fileName               // File name
            );
 
    // Create a hard link
    static void createHardLink(
            const std::string& linkName,              // Link name
            const std::string& fileName               // File name
            );

    // Delete a file
    static void deleteFile(
            const std::string& fileName               // File name
            );

    // Delete a file
    static void deleteFile(
            const std::string& fileName,              // File name
            Exception& exVal
            );

   // Remove debug file
   static void removeDebugFile();

   // Get the port of current login shell 
   static bool getCurrentPortNo(int& port);
   
   // Check if a TS shell
   static bool isTroubleShootingShell();
    
   // Move a file
    static void moveFile(
            const std::string& source,                // Source file name
            const std::string& dest,                  // Destination file name
            bool failIfExists = true                  // If true, move fails if file exists
            );

    // Copy a file
    static void copyFile(
            const std::string& source,                // Source file name
            const std::string& dest,                  // Destination file name
            bool failIfExists = true                  // If true, copy fails if file exists
            );

    // Copy a file
    static void copyFile(
            const std::string& source,                // Source file name
            const std::string& dest,                  // Destination file name
            Exception& exVal
            );

    // Copy files to a directory
    static void copyFiles(
            const std::string& source,                // Source file name pattern
            const std::string& dest,                  // Destination directory
            bool failIfExists = true                  // If true, copy fails if file exists
            );
    
    // Create a directory
    static void createDir(
            const std::string& dir                    // Directory name

            );

    // Remove a directory
    static void removeDirectory(
            const std::string& path                   // Directory to delete
            );

    // Delete a directory recursively
    static void deleteDirTree(
            const std::string& path                   // Directory to delete
            );

    // Set attributes to normal for files recursively
    static void setAttrsRec(
            const std::string& path                   // Directory to set attributes for 
            );

    // Create a symbolic link
    static void createSymbolicLink(
            const std::string& link,                  // Symbolic link
            const std::string& dir                    // Destination directory
            );

    // Delete a symbolic link
    static void deleteSymbolicLink(
            const std::string& link                   // Symbolic link
            );

    // Execute a console command

        static int executeCommand(
          const std::string& command,                 // Command string
          std::ostream& os                            // Stream containing output
          );    

    // Create temporary directory
    static std::string createTempDir();               // Returns path to temporary directory

   // Test the integrity of a zip file before actual unzip
    static void testunzip(
            const std::string& source,                // File to test unzip on
         const int level                              // Level that zipfile was unzipped at
         );

    // Unzip a file
    static void unzip(
            const std::string& source,                // File to unzip
            const std::string& dest                   // Destination path for unzipped files
            );

    // Set APZ data path
    static void setApzDataPath();

    // Get APG version
    //static ACS_APGCC_HwVer_ReturnTypeT getApgVersion();     // Return value:
                                                            //    APG version:
                                                            //    1    APG40C/2
                                                            //    2    APG40C/4
                                                            //    3    APG43

    // Get APZ data path
    static const std::string getApzDataPath();    // Returns path to APZ data

    // Create symbolic links for directory structute
    static void createSymlinks();
    static void createLink(const string&);

    // Check if MAUS
    static bool isMAUS(CPID cpId);
    
    // Check if vAPZ
    static bool isVAPZ();

    // Check CP name
    static bool isCpName(const string& name);

    // Invoke bootOpt script
    static int _execlp(const char* fileName);

    // Get bootOpt script path
    static string bootOptsTool();


private:
    static std::string s_apzpath;

};

#endif
