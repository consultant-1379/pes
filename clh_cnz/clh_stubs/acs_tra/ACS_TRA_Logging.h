/*
 * .NAME: ACS_TRA_Logging.h
 *
 * .LIBRARY 3C++
 * .PAGENAME
 * .HEADER
 * .LEFT_FOOTER Ericsson Telecom AB
 * .INCLUDE
 *
 * .COPYRIGHT
 *  COPYRIGHT Ericsson Utvecklings AB, Sweden 1995-2010.
 *  All rights reserved.
 *
 *  The Copyright to the computer program(s) herein is the property of
 *  Ericsson Telecom AB, Sweden.
 *  The program(s) may be used and/or copied only with the written
 *  permission from Ericsson Telecom AB or in accordance with
 *  the terms and conditions stipulated in the agreement/contract under
 *  which the program(s) have been supplied.
 *
 * .DESCRIPTION
 *	This file contains the class ACS_TRA_Loggingall methods and
 *  function to permit each application to log its information in
 *  a file on a disk.
 *
 * .ERROR HANDLING
 *
 *
 * DOCUMENT NO
 *	190 89-CAA 109 0024
 *
 * AUTHOR
 * 	2010-05-26 by XYV  XLUCPET
 *
 * REVISION
 *
 * CHANGES
 *
 * RELEASE REVISION HISTORY
 *
 * REV NO		DATE		NAME 		DESCRIPTION
 * PA1	        20100504	XLUCPET		Impact due to IP:
 *	  									2/159 41-8/FCP 121 1642
 *	  									"APG50 Tracing and MKTR"
 * PA2 	        20101104	XGIOPAP		Implementation of comment
 */

#include <string>
#include <fstream>

#ifndef ACS_TRA_LOG_H_
#define ACS_TRA_LOG_H_
#define NUMBER_OF_SECOND 1


// Log level for trace log
enum ACS_TRA_LogLevel
{
	LOG_LEVEL_TRACE,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL
};

// Type of return error
enum ACS_TRA_LogResult
{
	TRA_LOG_OK,
	TRA_LOG_ERROR,
	TRA_LOG_ERROR_CONF_FILE,
	TRA_LOG_ERROR_LOG_LEVEL
};

/*
 * Name: ACS_TRA_Logging
 * Description: This class contains methods and attribute
 * to permit each application to log its information on a file
 */
class ACS_TRA_Logging
{
public:
	ACS_TRA_Logging();

	virtual ~ACS_TRA_Logging();

	ACS_TRA_LogResult Open(const char*);

	void Close();

	ACS_TRA_LogResult Write(const char*, ACS_TRA_LogLevel Log_Level = LOG_LEVEL_INFO);

	ACS_TRA_LogResult getLastError(void) const;

	const char* getLastErrorText(void) const;

private:
	std::fstream m_fs;

	static const char s_logpath[];
};

#endif /* ACS_TRA_LOG_H_ */
