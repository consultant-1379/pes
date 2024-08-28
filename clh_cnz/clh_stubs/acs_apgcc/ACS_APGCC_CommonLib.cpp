#include "ACS_APGCC_CommonLib.h"
#include <string.h>

const char s_apzdata[] =      "/data/apz/data";
const char s_apzlogs[] =      "/data/apz/logs";
const char s_cpsdata[] =      "/data/cps/data";
const char s_cpslogs[] =      "/data/cps/logs";
const char s_ftpvol[] =       "/data/ftpvol";
const char s_internalRoot[] = "/data/opt/ap/internal_root";
const char s_supportData[] =  "/data/opt/ap/internal_root/support_data";

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
ACS_APGCC_CommonLib::ACS_APGCC_CommonLib()
{
}

//----------------------------------------------------------------------------------------
// This function returns a physical path string from a logical name value
//----------------------------------------------------------------------------------------
ACS_APGCC_DNFPath_ReturnTypeT ACS_APGCC_CommonLib::GetDataDiskPath(
                                                      const char* logicalName,
                                                      char* path,
                                                      int &len
                                                      )
{
   const char* diskPath = 0;
   if (strcmp(logicalName, "APZ_DATA") == 0)
   {
      diskPath = s_apzdata;
   }
   else if (strcmp(logicalName, "APZ_LOGS") == 0)
   {
      diskPath = s_apzlogs;
   }
   else if (strcmp(logicalName, "CPS_DATA") == 0)
   {
      diskPath = s_cpsdata;
   }
   else if (strcmp(logicalName, "CPS_LOGS") == 0)
   {
      diskPath = s_cpslogs;
   }
   else if (strcmp(logicalName, "FTP_VOL") == 0)
   {
      diskPath = s_ftpvol;
   }
   else
   {
   	path[0] = 0;
      return ACS_APGCC_FAULT_LOGICAL_NAME;
   }

   if (len > (int)strlen(diskPath))
   {
      len = strlen(diskPath);
      strcpy(path, diskPath);
      return ACS_APGCC_DNFPATH_SUCCESS;
   }
   else
   {
    	path[0] = 0;
      return ACS_APGCC_STRING_BUFFER_SMALL;
   }
}

//----------------------------------------------------------------------------------------
// This function returns a physical path string from a logical name value
//----------------------------------------------------------------------------------------
ACS_APGCC_DNFPath_ReturnTypeT ACS_APGCC_CommonLib::GetFileMPath(
                                                      const char* logicalName,
                                                      char* path,
                                                      int &len
                                                      )
{
   const char* diskPath = 0;
   if (strcmp(logicalName, "internalRoot") == 0)
   {
      diskPath = s_internalRoot;
   }
   else if (strcmp(logicalName, "supportData") == 0)
   {
      diskPath = s_supportData;
   }
   else
   {
      path[0] = 0;
      return ACS_APGCC_FAULT_LOGICAL_NAME;
   }

   if (len > (int)strlen(diskPath))
   {
      len = strlen(diskPath);
      strcpy(path, diskPath);
      return ACS_APGCC_DNFPATH_SUCCESS;
   }
   else
   {
      path[0] = 0;
      return ACS_APGCC_STRING_BUFFER_SMALL;
   }
}

//----------------------------------------------------------------------------------------
// This function returns a physical path string from a logical name value
//----------------------------------------------------------------------------------------
ACS_APGCC_DNFPath_ReturnTypeT GetDataDiskPathForCp(
                                             const char* p_LogicalName,
                                             unsigned int cpId,
                                             char* p_Path,
                                             int &p_Len
                                             )
{
   return ACS_APGCC_DNFPATH_SUCCESS;
}

//----------------------------------------------------------------------------------------
// This function checks and returns the hardware version on the AP
//----------------------------------------------------------------------------------------
ACS_APGCC_HwVer_ReturnTypeT ACS_APGCC_CommonLib::GetHWVersion(
                                                      char* hwVersion,
                                                      int &len
                                                      )
{
   if (len > (int)strlen(hwVersion))
   {
      strncpy(hwVersion, ACS_APGCC_HWVER_APG43_GEP2_STRING, len);
      len = strlen(ACS_APGCC_HWVER_APG43_GEP2_STRING);
      return ACS_APGCC_HWVER_APG43;
   }
   else
   {
      return ACS_APGCC_STRING_BUFFER_TOO_SMALL;
   }
}
