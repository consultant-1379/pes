//==============================================================================
//
// NAME
//      ACS_PRC_Process.h
//
//  COPYRIGHT Ericsson Utvecklings AB, Sweden 1999.
//  All rights reserved.
//
//  The Copyright to the computer program(s) herein 
//  is the property of Ericsson Utvecklings AB, Sweden.
//  The program(s) may be used and/or copied only with 
//  the written permission from Ericsson Utvecklings AB or in 
//  accordance with the terms and conditions stipulated in the 
//  agreement/contract under which the program(s) have been 
//  supplied.
//
// .DESCRIPTION 
// 	This program contains a method to set the priority of 
//	processes in the Adjunct Processor.
//
// DOCUMENT NO
//	190 89-CAA 109 0340 Ux
//
// AUTHOR 
// 	1999-08-25 by UAB/I/GM Thomas Olsson
//
// SEE ALSO 
// 	-
//
//==============================================================================

#ifndef ACS_PRC_PROCESS_H
#define ACS_PRC_PROCESS_H

#ifdef _PRCAPIBUILD
#define PRCAPIExport __declspec (dllexport)
#else
#define PRCAPIExport __declspec (dllimport)
#endif

enum ACS_PRC_Error
{
	ACS_PRC_ERROR = -1,
	ACS_PRC_ERROR_PRIORITY = -2
};

enum ACS_PRC_PROCTYPE
{
	AP_COMMAND,
	AP_SERVICE
};

void AP_InitProcess (const char procname [], ACS_PRC_PROCTYPE proctype);
// Description:
//		Set the priority of a process.
// Parameters:
//		procname		Process name, 1 - 31 characters
//		proctype		Process type:
//						AP_COMMAND		Command initiated process
//						AP_SERVICE		Service process
// Return value:
//		None.

#endif
