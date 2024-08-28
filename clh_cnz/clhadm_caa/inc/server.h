//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      server.h
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
//      This is the CLH server.
//      
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1416  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-01-08   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef SERVER_H_
#define SERVER_H_

#include <string>

// Signal handler
void sighandler(
      int signo
      );

// Signal handler fro AMF service
void sighandlerAmf(
      int signo
      );

// Get software version
std::string getVersion();

// AMF service registration
void regAMFService(
      const std::string& service          // Service name
      );

// Command usage
void usage(
      const std::string& cmdname,
      bool verbose = false
      );

#endif // SERVER_H_

