/*=================================================================== */
/**
   @file   ACS_AMF_CC_Types.h

   @brief Header file for APG Common Class API to Integrate with AMF.

          This module abstracts all coremw types from the application. 
		  All applications are expected to include this header 
   @version 1.0.0
*/
/*
   HISTORY
   This section contains reference to problem report and related
   software correction performed inside this module


   PR           DATE      INITIALS    DESCRIPTION
   -----------------------------------------------------------
   N/A       DD/MM/YYYY     NS       Initial Release
==================================================================== */

#ifndef ACS_APGCC_AMF_TYPES_H_
#define ACS_APGCC_AMF_TYPES_H_

//#include "saAmf.h"
#include <stdint.h>

#define SA_AMF_HEALTHCHECK_KEY_MAX 512
#define SA_MAX_NAME_LENGTH 512

#define ACS_APGCC_HEALTHCHECK_KEY_MAX SA_AMF_HEALTHCHECK_KEY_MAX
#define ACS_APGCC_MAX_NAME_LENGTH SA_MAX_NAME_LENGTH
#define ACS_APGCC_NCSCC_RC_SUCCESS	1
#define ACS_APGCC_NCSCC_RC_FAILURE	2

#ifndef ACS_APGCC_BOOL
	typedef unsigned int ACS_APGCC_BOOL;
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NODE_ONE
#define NODE_ONE 1
#endif

#ifndef NODE_TWO
#define NODE_TWO 2
#endif

typedef struct {
	unsigned char key [ ACS_APGCC_HEALTHCHECK_KEY_MAX ];
	unsigned short keyLen;
}ACS_APGCC_AMF_HealthCheckKeyT;

typedef struct {
	unsigned short length;
	unsigned char value[ACS_APGCC_MAX_NAME_LENGTH];
}ACS_APGCC_AIS_NameT;

#ifndef ACS_APGCC_SEL_OBJ_DEFINED
#define ACS_APGCC__SEL_OBJ_DEFINED
	typedef struct acs_apgcc_sel_obj{
		int raise_obj;
		int rmv_obj;
	}ACS_APGCC_SEL_OBJ;
#endif

typedef enum {
	ACS_APGCC_SUCCESS = 0,
	ACS_APGCC_FAILURE = 1

}ACS_APGCC_ReturnType;

typedef enum {
	ACS_APGCC_HA_SUCCESS = 0,
	ACS_APGCC_HA_FAILURE = 1,
	ACS_APGCC_HA_FAILURE_CLOSE = 2
}ACS_APGCC_HA_ReturnType;

typedef ACS_APGCC_ReturnType ACS_APGCC_ErrorT ;
typedef uint64_t ACS_APGCC_SelObjT ;

typedef enum {
	ACS_APGCC_AMF_DISPATCH_ONE 		= 1,
	ACS_APGCC_AMF_DISPATCH_ALL 		= 2,
	ACS_APGCC_AMF_DISPATCH_BLOCKING		= 3
}ACS_APGCC_AMF_DispatchFlagsT;

typedef enum {
    	ACS_APGCC_AMF_HA_ACTIVE 	= 1,
    	ACS_APGCC_AMF_HA_STANDBY 	= 2,
    	ACS_APGCC_AMF_HA_QUIESCED 	= 3,
    	ACS_APGCC_AMF_HA_QUIESCING 	= 4,
	ACS_APGCC_AMF_HA_UNDEFINED 	= 5 
} ACS_APGCC_AMF_HA_StateT;

typedef enum {
	ACS_APGCC_NO_RECOMMENDATION	= 1,
    	ACS_APGCC_COMPONENT_RESTART 	= 2,
    	ACS_APGCC_COMPONENT_FAILOVER	= 3,
    	ACS_APGCC_NODE_SWITCHOVER 	= 4,
    	ACS_APGCC_NODE_FAILOVER 	= 5,
    	ACS_APGCC_NODE_FAILFAST 	= 6,
    	ACS_APGCC_CLUSTER_RESET 	= 7,
    	ACS_APGCC_APPLICATION_RESTART 	= 8,
	ACS_APGCC_CONTAINER_RESTART	= 9
} ACS_APGCC_AMF_RecommendedRecoveryT ;

typedef enum {
	ACS_APGCC_RDA_UNDEFINED = 0,
        ACS_APGCC_RDA_ACTIVE	= 1,
        ACS_APGCC_RDA_STANDBY	= 2,	
        ACS_APGCC_RDA_QUIESCED	= 3,
        ACS_APGCC_RDA_ASSERTING	= 4,
        ACS_APGCC_RDA_YIELDING	= 5
}ACS_APGCC_RDARoleT;


#endif /* ACS_APGCC_AMF_TYPES_H_ */

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
