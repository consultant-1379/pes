//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      logger.cpp
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
//      PA1    2011-10-14   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "logger.h"
#include "ltime.h"
#include <iostream>

using namespace std;

namespace PES_CLH {

ACS_TRA_Logging Logger::s_logging;
ACS_TRA_LogLevel Logger::s_minlevel = LOG_LEVEL_WARN;
bool Logger::s_console = false;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Logger::Logger(ACS_TRA_LogLevel level):
m_level(level)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Logger::~Logger()
{
}

//----------------------------------------------------------------------------------------
// Boolean operator
//----------------------------------------------------------------------------------------
Logger::operator bool() const
{
   return m_level >= s_minlevel;
}

//----------------------------------------------------------------------------------------
// Open log file
//----------------------------------------------------------------------------------------
void Logger::open(const string& name, ACS_TRA_LogLevel minlevel, bool console)
{
   s_minlevel = minlevel;
   s_console = console;

   ACS_TRA_LogResult result = s_logging.Open(name.c_str());
   if (result != TRA_LOG_OK)
   {
      cerr << "Failed to open log file." << endl;
   }
}

//----------------------------------------------------------------------------------------
// Close log file
//----------------------------------------------------------------------------------------
void Logger::close()
{
   s_logging.Close();
}

//----------------------------------------------------------------------------------------
// Create log event
//----------------------------------------------------------------------------------------
void Logger::event(
         const string& file,
         const string& function,
         long line,
         const string& message
         )
{
   size_t pos = file.find_last_of('/');

   ostringstream s;
   s << "========================================================================"
     << endl;
   s << "Level:    " << m_level << endl;
   s << "Time:     " << Time::e_pretty << Time::now() << endl;
   s << "File:     " << file.substr(pos + 1) << endl;
   s << "Function: " << function << endl;
   s << "Line:     " << line << endl;
   s << "Message:  \\" << endl;
   s << message;

   // Write to log
   s_logging.Write(s.str().c_str(), m_level);
   if (s_console)
   {
      // Print to console
      cout << s.str() << endl;
   }
}

//----------------------------------------------------------------------------------------
// Create log event, static version
//----------------------------------------------------------------------------------------
void Logger::event(
               ACS_TRA_LogLevel level,
               const string& file,
               const string& function,
               long line,
               const string& message
               )
{
   if (level >= s_minlevel)
   {
      Logger logger(level);
      logger.event(file, function, line, message);
   }
}

//----------------------------------------------------------------------------------------
// Create log event from exception
//----------------------------------------------------------------------------------------
void Logger::event(ACS_TRA_LogLevel level, const Exception& ex)
{
   event(
         level,
         ex.getFile(),
         ex.getFunction(),
         ex.getLine(),
         ex.getMessage()
         );
}

void Logger::event(const Exception& ex)
{
   event(LOG_LEVEL_ERROR, ex);
}

//----------------------------------------------------------------------------------------
// Get min log level
//----------------------------------------------------------------------------------------
ACS_TRA_LogLevel Logger::getMinLevel()
{
   return s_minlevel;
}

//----------------------------------------------------------------------------------------
// Log level outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, ACS_TRA_LogLevel level)
{
   switch (level)
   {
      case LOG_LEVEL_TRACE:  s << "TRACE"; break;
      case LOG_LEVEL_DEBUG:  s << "DEBUG"; break;
      case LOG_LEVEL_INFO:   s << "INFO";  break;
      case LOG_LEVEL_WARN:   s << "WARN";  break;
      case LOG_LEVEL_ERROR:  s << "ERROR"; break;
      case LOG_LEVEL_FATAL:  s << "FATAL"; break;
      default:               s << "****";
   }

   return s;
}

}

