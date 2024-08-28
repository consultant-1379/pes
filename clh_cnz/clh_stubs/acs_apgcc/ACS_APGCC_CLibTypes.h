/*=================================================================== */
/**
   @file   ACS_APGCC_CLibTypes.h

   @brief .

          
   @version 1.0.0
*/
/*
   HISTORY
  
   PR           DATE      INITIALS    DESCRIPTION
   -----------------------------------------------------------
   N/A       DD/MM/YYYY     NS       Initial Release
==================================================================== */

#ifndef ACS_APGCC_CLIBTYPES_H_
#define ACS_APGCC_CLIBTYPES_H_

typedef struct {
	bool  readFlag;
	bool  writeFlag;
	bool  executeFlag;
} ACS_APGCC_PermissonT;

typedef struct {
	ACS_APGCC_PermissonT ownerPermission;
	ACS_APGCC_PermissonT groupPermission;
	ACS_APGCC_PermissonT otherPermission;
}ACS_APGCC_basicLinuxACLT;

typedef struct {
	char* groupName;
 	ACS_APGCC_PermissonT groupPermission;
}ACS_APGCC_extendedLinuxACLT;


typedef enum {
	ACS_APGCC_STRING_BUFFER_TOO_SMALL	= 	-2,
	ACS_APGCC_UNDEFINED_HWVER		= 	-1,
	ACS_APGCC_HWVER_APG40C2			=	1,	
	ACS_APGCC_HWVER_APG40C4			=	2,
	ACS_APGCC_HWVER_APG43			=	3
	
}ACS_APGCC_HwVer_ReturnTypeT;   	// to retain design base

typedef enum {
	ACS_APGCC_STRING_BUFFER_SMALL	= 	-2,
	ACS_APGCC_FAULT_LOGICAL_NAME	= 	-1,
	ACS_APGCC_DNFPATH_SUCCESS	=        0,
	ACS_APGCC_DNFPATH_FAILURE	=	 1	
}ACS_APGCC_DNFPath_ReturnTypeT;   	// to retain design base

typedef enum {
	ACS_APGCC_BAD_IV_MAT		 = -8,
	ACS_APGCC_BAD_PARAMS		 = -7,	
	ACS_APGCC_BAD_ROUNDS		 = -6,
	ACS_APGCC_BAD_CIPHER_STATE       = -5,
	ACS_APGCC_BAD_CIPHER_MODE	 = -4,
	ACS_APGCC_BAD_KEY_INSTANCE	 = -3,
	ACS_APGCC_BAD_KEY_MAT		 = -2,
	ACS_APGCC_BAD_KEY_DIR		 = -1,
	ACS_APGCC_ENCRPT_DECRPT_SUCCESS	 =  0,
	ACS_APGCC_ENCRPT_DECRPT_FAILURE	 =  1
}ACS_APGCC_EncrptDecryptData_ReturnTypeT;   	// to retain design base

typedef enum {
	ACS_APGCC_CLIB_SCCCESS	= 	0,
	ACS_APGCC_CLIB_FAILURE	= 	1
}ACS_APGCC_CLib_ReturnTypeT;		

typedef enum {
	ACS_APGCC_DIR_ENCRYPT=0,
	ACS_APGCC_DIR_DECRYPT=1
}ACS_APGCC_DIRFlagT;

#endif /* ACS_APGCC_CLIBTYPES_H_*/

//----------------------------------------------------------------------------
//
//  COPYRIGHT Ericsson AB 2010
//
//  The copyright to the computer program(s) herein is the property of
//  ERICSSON AB, Sweden. The programs may be used and/or copied only
//  with the written permission from ERICSSON AB or in accordance with
//  the terms and conditions stipulated in the agreement/contract under
//  which the program(s) have been supplied.
//
//----------------------------------------------------------------------------
