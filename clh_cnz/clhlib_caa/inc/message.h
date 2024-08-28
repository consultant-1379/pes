//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      message.h
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
//      PA1    2012-03-05   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>
#include <iostream>

namespace PES_CLH {

//========================================================================================
// Class Message
//========================================================================================

class Message
{
   friend std::ostream& operator<<(std::ostream& s, const Message& message);

public:
   Message();
   virtual ~Message();

   int64_t getTime() const;
   uintmax_t getSize() const;
   const char* getData(char* data) const;

protected:
   int64_t m_time;
   uintmax_t m_size;
   std::istream* m_isptr;

   static const char s_datamissing[];
};

//========================================================================================
// Class CLHMessage
//========================================================================================

class CLHMessage: public Message
{
   friend std::istream& operator>>(
         std::istream& s,
         CLHMessage& message
         );

public:
   CLHMessage();
   ~CLHMessage();

   static const std::string s_headerTag;
};

//========================================================================================
// Class TESRVMessage
//========================================================================================

class TESRVMessage: public Message
{
   friend std::istream& operator>>(
         std::istream& s,
         TESRVMessage& message
         );

public:
   TESRVMessage();
   ~TESRVMessage();

   static const std::string s_headerTag;
};

}

#endif // MESSAGE_H_
