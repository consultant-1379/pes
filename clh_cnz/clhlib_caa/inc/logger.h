//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      logger.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2011. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for handling trace logs.
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
//      PA1    2011-10-17   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef LOGGER_H_
#define LOGGER_H_

#include "exception.h"
#include <ACS_TRA_Logging.h>
#include <string>

const ACS_TRA_LogLevel LOG_LEVEL_OFF = static_cast<ACS_TRA_LogLevel>(LOG_LEVEL_FATAL + 1);

namespace PES_CLH {

class Logger
{
public:
   Logger(ACS_TRA_LogLevel level);

   ~Logger();

   operator bool() const;

   static void open(
         const std::string& name,
         ACS_TRA_LogLevel minlevel,
         bool console
         );

   static void close();

   void event(
         const std::string& file,
         const std::string& function,
         long line,
         const std::string& message
         );

   static void event(
         ACS_TRA_LogLevel,
         const std::string& file,
         const std::string& function,
         long line,
         const std::string& message
         );

   static void event(ACS_TRA_LogLevel level, const Exception& ex);

   static void event(const Exception& ex);

   static ACS_TRA_LogLevel getMinLevel();

private:
   ACS_TRA_LogLevel m_level;

   static ACS_TRA_Logging s_logging;
   static ACS_TRA_LogLevel s_minlevel;
   static bool s_console;
};

std::ostream& operator<<(std::ostream& s, ACS_TRA_LogLevel level);

}

#endif // LOGGER_H_
