//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      exception.cpp
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
//      -
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
//      PA2    2013-06-19   XVUNGUY     Add error codes - 119 + 120
//      PA3    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "exception.h"
#include <errno.h>
#include <string.h>

using namespace std;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Exception::Exception(
         const t_error& error,
         const string& file,
         const string& function,
         long line
         ) :
m_errcode(error.first),
m_s(),
m_file(file),
m_function(function),
m_line(line)
{
   m_s << error.second;
}

//----------------------------------------------------------------------------------------
// Copy constructor
//----------------------------------------------------------------------------------------
Exception::Exception(const Exception& ex) :
m_errcode(ex.m_errcode),
m_s(),
m_file(ex.m_file),
m_function(ex.m_function),
m_line(ex.m_line)
{
   m_s << ex.m_s.str();
}

//----------------------------------------------------------------------------------------
// Assignment operator
//----------------------------------------------------------------------------------------
Exception& 
Exception::operator=(const Exception& ex) 
{
   if (this != &ex)
   {
      m_errcode = ex.m_errcode;
      m_s << ex.m_s.str();
      m_file = ex.m_file;
      m_function = ex.m_function;
      m_line = ex.m_line;
   }
   return *this;
}

//----------------------------------------------------------------------------------------
// Get file
//----------------------------------------------------------------------------------------
string Exception::getFile() const
{
   return m_file;
}

//----------------------------------------------------------------------------------------
// Get function
//----------------------------------------------------------------------------------------
string Exception::getFunction() const
{
   return m_function;
}

//----------------------------------------------------------------------------------------
// Get line
//----------------------------------------------------------------------------------------
long Exception::getLine() const
{
   return m_line;
}

//----------------------------------------------------------------------------------------
// Get error message
//----------------------------------------------------------------------------------------
 std::string Exception::getMessage() const
 {
    return m_s.str();
 }

//----------------------------------------------------------------------------------------
// System error text, specified
//----------------------------------------------------------------------------------------
void 
Exception::sysError(int errnum)
{
   m_s << endl;
   m_s << strerror(errnum);
}

//----------------------------------------------------------------------------------------
// System error text, last received
//----------------------------------------------------------------------------------------
void 
Exception::sysError()
{
   sysError(errno);
}

//----------------------------------------------------------------------------------------
// Error messages
//----------------------------------------------------------------------------------------
Exception::t_error Exception::general()
{
   ostringstream s;
   s << "Error when executing.";
   return t_error(1, s.str());
}

Exception::t_error Exception::usage()
{
   ostringstream s;
   s << "Incorrect usage: Command format not correct.";
   return t_error(2, s.str());
}

Exception::t_error Exception::unexpOption(const string& opt)
{
   ostringstream s;
   s << "Incorrect usage: Unexpected option '" << opt << "' found in command.";
   return t_error(2, s.str());
}

Exception::t_error Exception::unexpToken(const string& token)
{
   ostringstream s;
   s << "Incorrect usage: Unexpected token '" << token << "' found in command.";
   return t_error(2, s.str());
}

Exception::t_error Exception::argExpected(const string& opt)
{
   ostringstream s;
   s << "Incorrect usage: Argument was expected for option '" << opt << "'.";
   return t_error(2, s.str());
}

Exception::t_error Exception::illNodeState()
{
   ostringstream s;
   s << "This command must be executed in the active node.";
   return t_error(4, s.str());
}

Exception::t_error Exception::illTimeInter()
{
   ostringstream s;
   s << "Start time is greater than stop time.";
   return t_error(19, s.str());
}

Exception::t_error Exception::illDate(const string& date)
{
   ostringstream s;
   s << "Date value '" << date << "' is not valid.";
   return t_error(20, s.str());
}

Exception::t_error Exception::illTime(const string& time)
{
   ostringstream s;
   s << "Time value '" << time << "' is not valid.";
   return t_error(21, s.str());
}

Exception::t_error Exception::illCpSide(const string& cpside)
{
   ostringstream s;
   s << "CP side '" << cpside << "' is not valid.";
   return t_error(22, s.str());
}

Exception::t_error Exception::cpSideNotAllowed()
{
   ostringstream s;
   s << "CP side is not allowed for a blade cluster CP.";
   return t_error(23, s.str());
}

Exception::t_error Exception::illTransType(const string& transtype)
{
   ostringstream s;
   s << "Transfer type '" << transtype << "' is not valid.";
   return t_error(24, s.str());
}

Exception::t_error Exception::illLogType(const string& logtype)
{
   ostringstream s;
   s << "Log type '" << logtype << "' is not valid.";
   return t_error(26, s.str());
}

Exception::t_error Exception::illLogTypeForApz(const string& logtype)
{
   ostringstream s;
   s << "Log type '" << logtype << "' is not valid for this APZ type.";
   return t_error(27, s.str());
}

Exception::t_error Exception::logTypeDupl(const string& logtype)
{
   ostringstream s;
   s << "Log type '" << logtype << "' is specified more than once.";
   return t_error(28, s.str());
}

Exception::t_error Exception::mediaFailed()
{
   ostringstream s;
   s << "An error has occurred during the file transfer to media." << endl;
   s << "See attribute resultOfOperation in MO ExternalMediaM for more details.";
   return t_error(33, s.str());
}

Exception::t_error Exception::noFileToCompress()
{
   ostringstream s;
   s << "No file to compress.";
   return t_error(35, s.str());
}

Exception::t_error Exception::illSubrack(const string& subrack)
{
   ostringstream s;
   s << "Subrack '" << subrack << "' is invalid.";
   return t_error(36, s.str());
}

Exception::t_error Exception::illSlot(const string& slot)
{
   ostringstream s;
   s << "Slot number '" << slot << "' is invalid.";
   return t_error(37, s.str());
}

Exception::t_error Exception::boardNotDefined(const string& magazine, uint32_t slot)
{
   ostringstream s;
   s << "Board with subrack '" << magazine << "' and slot " << slot << " is not defined.";
   return t_error(38, s.str());
}

Exception::t_error Exception::illXmno(uint16_t xmno)
{
   ostringstream s;
   s << "XM number " << xmno << " is not valid.";
   return t_error(45, s.str());
}

Exception::t_error Exception::unexpXmno(const string& logtype)
{
   ostringstream s;
   s << "XM numbers are not allowed for log type '" << logtype << "'.";
   return t_error(46, s.str());
}

Exception::t_error Exception::errXmlist()
{
   ostringstream s;
   s << "Error found when parsing list of XM numbers.";
   return t_error(47, s.str());
}

Exception::t_error Exception::illRPno(uint16_t rpno)
{
   ostringstream s;
   s << "RP number " << rpno << " is not valid.";
   return t_error(48, s.str());
}

Exception::t_error Exception::illLogLevel(const string& level)
{
   ostringstream s;
   s << "Illegal log level: " << level << ".";
   return t_error(60, s.str());
}

Exception::t_error Exception::parameter()
{
   ostringstream s;
   s << "Parameter error: ";
   return t_error(100, s.str());
}

Exception::t_error Exception::system()
{
   ostringstream s;
   s << "System error: ";
   return t_error(101, s.str());
}

Exception::t_error Exception::internal()
{
   ostringstream s;
   s << "Internal error: ";
   return t_error(102, s.str());
}

Exception::t_error Exception::illCommand()
{
   ostringstream s;
   s << "Illegal command in this system configuration.";
   return t_error(115, s.str());
}

Exception::t_error Exception::illOption(const string& opt)
{
   ostringstream s;
   s << "Illegal option '" << opt << "' in this system configuration.";
   return t_error(116, s.str());
}

Exception::t_error Exception::cpNotDefined(const string& cpname)
{
   ostringstream s;
   s << "CP '" << cpname << "' is not defined.";
   return t_error(118, s.str());
}

Exception::t_error Exception::illApSide(void)
{
   ostringstream s;
   s << "This command must be executed from IO APG.";
   return t_error(119, s.str());
}

Exception::t_error Exception::cmdBusyToTransferToMedia(void)
{
   ostringstream s;
   s << "A media transfer is already ongoing." << endl;
   s << "See attribute resultOfOperation in MO ExternalMediaM for more details.";
   return t_error(120, s.str());
}

Exception::t_error Exception::cmdBusyToTransferToFile(void)
{
   ostringstream s;
   s << "A file transfer is already ongoing.";
   return t_error(121, s.str());
}

Exception::t_error Exception::apzSystemNotDefined(const string& cpname)
{
   ostringstream s;
   s << "APZ system value is not defined for " << cpname << ".";
   return t_error(122, s.str());
}

Exception::t_error Exception::notSupportInBSP(void)
{
   ostringstream s;
   s << "SEL is not supported on this shelf architecture.";
   return t_error(123, s.str());
}

Exception::t_error Exception::cpSideNotAllowedToUseWithMAUS(void)
{
   ostringstream s;
   s << "CP side is not allowed to use for MAUS log types.";
   return t_error(124, s.str());
}

Exception::t_error Exception::mausEPMissing(const string& logtype)
{
   ostringstream s;
   s << "The MAUS endpoint is missing for log type '" << logtype << "'.";
   return t_error(125, s.str());
}

Exception::t_error Exception::illLogTypeForMAUSEP(const string& logtype)
{
   ostringstream s;
   s << "Log type '" << logtype << "' is not valid for an MAUS endpoint.";
   return t_error(126, s.str());
}

Exception::t_error Exception::illToUseWithSingleSidedCP(void)
{
   ostringstream s;
   s << "MAUS log types are only applicable for Dual sided CP.";
   return t_error(127, s.str());
}

Exception::t_error Exception::illEndpoint(const string& ep)
{
   ostringstream s;
   s << "MAUS endpoint '" << ep << "' is not valid.";
   return t_error(128, s.str());
}

//----------------------------------------------------------------------------------------
//   Outstream operator
//----------------------------------------------------------------------------------------
ostream& 
operator<<(ostream& s, const Exception& ex)
{
   s << ex.m_s.str();
   return s;
}

//----------------------------------------------------------------------------------------
//   StartGreatStopTimeException
//----------------------------------------------------------------------------------------
StartGreatStopTimeException::StartGreatStopTimeException(
                                                const Time& startTime,
                                                const Time& stopTime,
                                                const string& file,
                                                const string& function,
                                                long line
                                                ):
Exception(Exception::illTimeInter(), file, function, line),
m_startTime(startTime),
m_stopTime(stopTime)
{
   // Do nothing
}

}
