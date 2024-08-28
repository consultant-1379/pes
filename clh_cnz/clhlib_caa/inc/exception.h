//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      exception.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012, 2014. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for exception handling.
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
//      PA1    2012-08-30   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-06-19   XVUNGUY     Add error codes - 119 + 120
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <iostream>
#include <sstream>
#include <string>
#include <stdint.h>
#include "ltime.h"

#define WHERE__ __FILE__, __PRETTY_FUNCTION__, __LINE__

namespace PES_CLH {

class Exception
{
   friend std::ostream& operator<<(std::ostream& s, const Exception& ex);

public:
   typedef std::pair<uint16_t, std::string> t_error;

   // Constructors
   Exception(
         const t_error& error,
         const std::string& file,
         const std::string& function,
         long line
         );

   // Copy constructor
   Exception(const Exception& ex);

   // Assignment operator
   Exception& operator=(const Exception& ex);

   // Get file
   std::string getFile() const;

   // Get function
   std::string getFunction() const;

   // Get line
   long getLine() const;

   // Get error message
   std::string getMessage() const;

   // Get error code
   uint16_t getErrCode() const {return m_errcode;}

   // Stream error information to the Exception object
   template<typename T>
   std::stringstream& operator<<(const T& t)
   {
      m_s << t;
      return m_s;
   }

   // Win32 error text, specified
   void sysError(int errnum);

   // Win32 error text, last received
   void sysError();

   // Error messages
   static t_error general();
   static t_error usage();
   static t_error unexpOption(const std::string& opt);
   static t_error unexpToken(const std::string& token);
   static t_error argExpected(const std::string& opt);
   static t_error illNodeState();
   static t_error illTimeInter();
   static t_error illDate(const std::string& date);
   static t_error illTime(const std::string& time);
   static t_error illCpSide(const std::string& cpside);
   static t_error cpSideNotAllowed();
   static t_error illTransType(const std::string& transtype);
   static t_error illLogType(const std::string& logtype);
   static t_error illLogTypeForApz(const std::string& logtype);
   static t_error logTypeDupl(const std::string& logtype);
   static t_error mediaFailed();
   static t_error noFileToCompress();
   static t_error illSubrack(const std::string& subrack);
   static t_error illSlot(const std::string& slot);
   static t_error boardNotDefined(const std::string& magazine, uint32_t slot);
   static t_error illXmno(uint16_t xmno);
   static t_error unexpXmno(const std::string& logtype);
   static t_error errXmlist();
   static t_error illRPno(uint16_t rpno);
   static t_error illLogLevel(const std::string& level);
   static t_error parameter();
   static t_error system();
   static t_error internal();
   static t_error illCommand();
   static t_error illOption(const std::string& opt);
   static t_error cpNotDefined(const std::string& cpname);
   static t_error illApSide(void);
   static t_error cmdBusyToTransferToMedia(void);
   static t_error cmdBusyToTransferToFile(void);
   static t_error apzSystemNotDefined(const std::string& cpname);
   static t_error notSupportInBSP(void);
   static t_error cpSideNotAllowedToUseWithMAUS(void);
   static t_error mausEPMissing(const std::string& logtype);
   static t_error illLogTypeForMAUSEP(const std::string& logtype);
   static t_error illToUseWithSingleSidedCP(void);
   static t_error mausLogsNotSupported(const std::string& logtype);
   static t_error illEndpoint(const std::string& ep);

protected:
   uint16_t m_errcode;              // Error code
   std::stringstream m_s;           // Error message
   std::string m_file;              // File
   std::string m_function;          // Function
   long m_line;                     // Line
};

class StartGreatStopTimeException: public Exception
{
public:
   // Constructors
   StartGreatStopTimeException(
                        const Time& startTime,
                        const Time& stopTime,
                        const std::string& file,
                        const std::string& function,
                        long line
                        );

   Time getStartTime() const
   {
      return m_startTime;
   }

   Time getStopTime() const
   {
      return m_stopTime;
   }

private:
   Time m_startTime;
   Time m_stopTime;
};

}

#endif // EXCEPTION_H_
