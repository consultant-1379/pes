//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      message.cpp
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
//      Class for parsing log events.
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
//      PA1    2012-02-29   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <iterator>
#include <boost/lexical_cast.hpp>
#include <string.h>
#include <stdint.h>

#include "message.h"
#include "logger.h"
#include "ltime.h"
#include "parameters.h"

using namespace std;

namespace PES_CLH {

//========================================================================================
// Class Message
//========================================================================================

const char Message::s_datamissing[] = "*** CENTRAL LOG HANDLER: DATA IS MISSING! ***";

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Message::Message() :
m_time(),
m_size(0),
m_isptr(0)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Message::~Message()
{
}

//----------------------------------------------------------------------------------------
// Get time for a log message
//----------------------------------------------------------------------------------------
int64_t Message::getTime() const
{
   return m_time;
}

//----------------------------------------------------------------------------------------
// Get size for a log message
//----------------------------------------------------------------------------------------
uintmax_t Message::getSize() const
{
   return m_size;
}

//----------------------------------------------------------------------------------------
// Get log data
//----------------------------------------------------------------------------------------
const char* Message::getData(char* data) const
{
   if (m_isptr)
   {
      ios::streamoff pos = m_isptr->tellg();
      m_isptr->seekg(pos - m_size);
      m_isptr->read(data, m_size);
      m_isptr->seekg(pos);
   }
   else
   {
      strcpy(data, s_datamissing);
   }

   return data;
}

//----------------------------------------------------------------------------------------
// Send log messages to outstream 
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Message& message)
{
   s << "Time: " << message.m_time << endl;
   s << "Size: " << message.m_size << endl;
//   s << "Data: " << message.m_data << endl;
   return s;
}

//========================================================================================
//   Class CLHMessage
//========================================================================================

const string CLHMessage::s_headerTag = "CLH-hdr";

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
CLHMessage::CLHMessage() :
Message()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
CLHMessage::~CLHMessage()
{
}

//----------------------------------------------------------------------------------------
// Read CLH log messages from temporary file
//----------------------------------------------------------------------------------------
istream& operator>>(istream& is, CLHMessage& message)
{
   ios::streamoff pos = is.tellg();
   string str;

   istream_iterator<string> end_iter;
   for (istream_iterator<string> iter(is); iter != end_iter; iter++)
   {
      // Find header
      if (iter->find(CLHMessage::s_headerTag) != string::npos)
      {
         is.ignore();          // Ignore '\n'

         ios::streamoff tpos = is.tellg();
         size_t skipped = tpos - pos - 8;
         if (skipped > 0)
         {
            ostringstream s;
            s << "Skipped " << skipped
              << " bytes in log message before header was found.";
            Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

            message.m_time = Time::now();
            uintmax_t size = strlen(Message::s_datamissing);
            message.m_isptr = 0;
            message.m_size = size;
            is.seekg(tpos - 8);
            return is;
         }

         try
         {
            // Get time
            is >> str;
            message.m_time = boost::lexical_cast<int64_t>(str);
            is.seekg(tpos += 18);

            // Get size
            is >> str;
            uintmax_t size =  boost::lexical_cast<uintmax_t>(str);
            is.seekg(tpos += 14);

            // Get data
            if (size <= BaseParameters::s_maxfilesize)
            {
               message.m_size = size;
            }
            else
            {
               // Message size exceeded, message will be truncated
               message.m_size = BaseParameters::s_maxfilesize;

               ostringstream s;
               s << "Message size exceeds max allowed size - truncated.";
               Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
            }
            message.m_isptr = &is;
            pos = tpos + size;
            is.seekg(pos);
            return is;
         }
         catch (exception&)
         {
            is.seekg(pos += 8);

            ostringstream s;
            s << "CLH header corrupt.";
            Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
         }
      }
   }
   return is;
}

//========================================================================================
//   Class TESRVMessage
//========================================================================================

const string TESRVMessage::s_headerTag = "TRACELOG-HDR-V1";

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
TESRVMessage::TESRVMessage() :
Message()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
TESRVMessage::~TESRVMessage()
{
}

//----------------------------------------------------------------------------------------
// Read TESRV log messages from temporary file
//----------------------------------------------------------------------------------------
istream& operator>>(istream& is, TESRVMessage& message)
{
   ios::streamoff pos = is.tellg();
   string str;

   istream_iterator<string> end_iter;
   for (istream_iterator<string> iter(is); iter != end_iter; iter++)
   {
      // Find header
      if (iter->find(TESRVMessage::s_headerTag) != string::npos)
      {
         is.ignore();          // Ignore '\n'

         ios::streamoff tpos = is.tellg();
         size_t skipped = tpos - pos - 16;
         if (skipped > 0)
         {
            ostringstream s;
            s << "Skipped " << skipped
              << " bytes in log message before header was found.";
            Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());

            message.m_time = Time::now();
            uintmax_t size = strlen(Message::s_datamissing);
            message.m_isptr = 0;
            message.m_size = size;
            is.seekg(tpos - 16);
            return is;
         }

         try
         {
            // Get time
            is.read(reinterpret_cast<char*>(&message.m_time), sizeof(int64_t));
            is.seekg(tpos += 8);

            // Get size
            uintmax_t size;
            is.read(reinterpret_cast<char*>(&size), sizeof(uintmax_t));
            is.seekg(tpos += 9);

            // Get data
            if (size <= BaseParameters::s_maxfilesize)
            {
               message.m_size = size;
            }
            else
            {
               // Message size exceeded, message will be truncated
               message.m_size = BaseParameters::s_maxfilesize;

               ostringstream s;
               s << "Message size exceeds max allowed size - truncated.";
               Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
            }
            message.m_isptr = &is;
            pos = tpos + size;
            is.seekg(pos);
            return is;
         }
         catch (exception&)
         {
            is.seekg(pos += 16);

            ostringstream s;
            s << "TESRV header corrupt.";
            Logger::event(LOG_LEVEL_WARN, WHERE__, s.str());
         }
      }
   }
   return is;
}

}

