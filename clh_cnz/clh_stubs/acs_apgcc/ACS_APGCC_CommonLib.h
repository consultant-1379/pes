/*=================================================================== 

   @file   ACS_APGCC_CommonLib.h

   @brief .

          
   @version 1.0.0


   HISTORY
   This section contains reference to problem report and related
   software correction performed inside this module


   PR           DATE      INITIALS    DESCRIPTION
   -----------------------------------------------------------
   N/A       DD/MM/YYYY     NS       Initial Release
==================================================================== */

/*=====================================================================
                          DIRECTIVE DECLARATION SECTION
==================================================================== */

#ifndef ACS_APGCC_COMMONLIB_H_
#define ACS_APGCC_COMMONLIB_H_

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <string.h>

#include "acs_apgcc_omhandler.h"
#include "ACS_APGCC_CLibTypes.h"
#include "ACS_CC_Types.h"
//#include "ace/OS.h"

#define ACS_APGCC_HWVER_APG40C2_STRING "APG40C/2"
#define ACS_APGCC_HWVER_APG40C4_STRING "APG40C/4"
#define ACS_APGCC_HWVER_APG43_GEP1_STRING "APG43"
#define ACS_APGCC_HWVER_APG43_GEP2_STRING "APG43/2"
#define PARENT_OBJECT_DN "safApp=safImmService"
#define INSTANCE_DN_DATADISK_STRING "APG43L_DDISKPATH="
#define INSTANCE_DN_FTPVIRTUAL_STRING "APG43L_FTPVIRTUALPATH="

using namespace std;

class ACS_APGCC_CommonLib {

private:

	// Declare class level private data here


protected:

	// Declare class level protected data here


public:

   // Declare the interface services you provide here
   ACS_APGCC_CommonLib();
   ~ACS_APGCC_CommonLib()
   {
   }
   ;

   ACS_APGCC_CLib_ReturnTypeT SetFilePerm(
            const char* FilePath,
            ACS_APGCC_basicLinuxACLT aBasicLinuxACLdata,
            int p_special_perm, char* MessageBuffer);

   ACS_APGCC_HwVer_ReturnTypeT GetHWVersion(
            char* hwVersion,
            int &p_Len
            );

   ACS_APGCC_DNFPath_ReturnTypeT GetDataDiskPath(
            const char* p_LogicalName,
            char* p_Path,
            int &p_Len
            );

   ACS_APGCC_DNFPath_ReturnTypeT GetFileMPath(
            const char* p_LogicalName,
            char* p_Path,
            int &p_Len
            );

   ACS_APGCC_DNFPath_ReturnTypeT GetDataDiskPathForCp(
            const char* p_LogicalName,
            unsigned int cpId,
            char* p_Path,
            int &p_Len
            );

   ACS_APGCC_DNFPath_ReturnTypeT GetFTPVirtualPath(
            const char* p_LogicalName,
            char* p_VirtualDir,
            int &p_Len
            );

   ACS_APGCC_EncrptDecryptData_ReturnTypeT EncryptDecryptData(
            const char* p_Input,
            const char* CommandLine,
            char* p_Output,
            const char* p_Key,
            ACS_APGCC_DIRFlagT p_dFlag,
            int p_dataLen,
            int p_kLen
            );

};


#endif /* end ACS_APGCC_COMMONLIB_H_ */

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
