//==============================================================================
//
// NAME
//      ACS_PRC_API.h
//
//  COPYRIGHT Ericsson AB, Sweden 2003.
//  All rights reserved.
//
//  The Copyright to the computer program(s) herein 
//  is the property of Ericsson AB, Sweden.
//  The program(s) may be used and/or copied only with 
//  the written permission from Ericsson AB or in 
//  accordance with the terms and conditions stipulated in the 
//  agreement/contract under which the program(s) have been 
//  supplied.
//
// .DESCRIPTION 
// 	This program contains methods for
//	reboot node
//	switch node
//  stop groups and resources
//  start groups and resources
//	add/remove resources and groups
//	read node status
//
// DOCUMENT NO
//	190 89-CAA 109 0520 Ux
//
// AUTHOR 
// 	2003-05-14 by EAB/UKY/GB LMGR
//
// SEE ALSO 
// 	-
//
//==============================================================================

#ifndef ACS_PRC_API_H
#define ACS_PRC_API_H

#include <iostream>
#include <string.h>
#include <saImmOm.h>
#include <saClm.h>
#include <vector>
using namespace std;
// Reason for the reboot or stopResource or stopGroup

const int rebootLimit       = 0;	// Only one reboot / 30 min
const int cyclicRebootLimit = 1;	// After second reboot within 30 min=>manual

enum reasonType {causedByFailover,		// Caused by failover
				 causedByError,			// Caused by fault
				 functionChange,		// Initiated by FCH
				 softFunctionChange,	// Initiated by SFC
				 userInitiated,			// Initated by anyone
				 causedByEvent,			// Initated by PRC_Eva
				 manualRebootInitiated,	// Manually initated reboot
				 systemInitiated,		// Internal use only
				 unknownReason,			// Unknown
				 referenceFC,			// Function Change FCR
				 nsfInitiated,			// Initiated by NSF
				 resourceFailed,		// Initiated by a resource failing too often
				 hwmResetInitiated,		// Initiated by an hwmreset command
				 hwmBlkInitiated,		// Initiated by an hwmblk command
				 hwmDblkInitiated,		// Initiated by an hwmdblk command
				 manualShutdown,		// Caused by standard windows shutdown procedure
				 bugcheckInitiated,		// Caused by a Bugcheck (Blue Screen)
				 fccReset,				// Caused by an fcc_reset command
				 amBoardWatchdog,		// Caused by the AM-Board: Watchdog
				 amBoardTemperature,	// Caused by the AM-Board: High Temperature
				 amBoardVoltage,		// Caused by the AM-Board: Bad Voltage
				 amBoardPMC,			// Caused by the AM-Board: PMC Power Signal
				 amBoardCPCI};			// Caused by the AM-Board: CPCI Bus Signal
 
enum
{
	ACS_PRC_IMM_ERROR = -1,
	ACS_PRC_IMM_ERROR_SEARCH = -2,
};

enum
{
	ACS_PRC_LOCAL_NODE = 0,
	ACS_PRC_REMOTE_NODE = 1,
};
 
// reasonType is reported in the ISP-log.
//
// Rules for reasonType in resourceStateEvent:
// resourceStateType		reasonType
//    started				functionChange,
//							referenceFC,
//							softFunctionChange,
//							manualRebootInitiated,	// if manually stopped
//							unknownReason			// all other cases
//    stopped,failed		any reason
//    inserted,removed		functionChange
//    unknownResourceState	-
//
// Rules for reasonType in resourceStateSync:
// The reason reported in previous resourceStateEvent.
//	
// Rules for reasonType in serviceStateEvent:
// resourceStateType		reasonType
//    started				functionChange,
//							referenceFC,
//							softFunctionChange,
//							unknownReason			// all other cases
//    stopped,failed		functionChange,
//							referenceFC,
//							softFunctionChange,
//							manualRebootInitiated,
//							unknownReason
//    inserted,removed		-
//    unknownResourceState	-
//
// Rules for reasonType in serviceStateSync:
// The reason reported in previous serviceStateEvent.
//
// Rules for reasonType in nodeStateEvent:
//							reasonType
//							any reason

// Rules for reasonType in nodeStateSync:
// The reason reported in previous nodeStateEvent.

// Rules for reasonType in runLevelEvent:
// runlevel					reasonType
//     0					-
//     1,2					any reason					
//     3,4					reason for the failing resoure/service
//     5					unknownReason

// Rules for reasonType in runLevelSync:
// The reason reported in previous runLevelEvent.

// Rules for reasonType in otherNodeShutDown:
//							reasonType
//							functionChange,
//							referenceFC,
//							softFunctionChange,
//							manualRebootInitiated,
//							unknownReason 

// NOTE when using this API from .exe files compiled in VC7.0:
// The ACS_PRC_API is compiled using VC6. 
// There are some incompatibilities between these compilers
// when using WCHAR type. That type is used in many methods in
// the ACS_PRC_API. When using such methods from VC7 exe-files you have
// to surround the #include:
// #define WCHAR unsigned short
// #include "acs_prc_api.h"
// #undef WCHAR 
// #define WCHAR __wchar_t
// And then in each call to ACS_PRC_API, do a type casting to unsigned short.

//class ACS_PRC_API_internal; // Forward

class ACS_PRC_API
{
public:

	  ACS_PRC_API();
	// Description:
	//		Constructor.

	  ~ACS_PRC_API();
	// Description:
	//		Desctructor.

	  bool isRebootPossible (reasonType reason);
	// Description:
	//		The method is used to check if it is possible to 
	//		initiate a reboot. Reboot is in some cases prohibited
	//		to avoid the risk of cyclic reboot.
	//		Following rules apply for reasonType:
	//		manualRebootInitiated and systemInitiated
	//			may always initiate a reboot
	//		causedByFailover, functionChange, referenceFC and softFunctionChange
	//			may initiate 2 reboot within 30 minutes.
	//		causedByError, userInitiated and causedByEvent
	//			may initiate 1 reboot within 30 minutes.
	// Parameters:
	//		reason			Reason for the reboot
	// Return value:
	//		true			It is possible to reboot
	//		false			Not possible to reboot

	  bool rebootNode (reasonType reason = userInitiated,const char* userInfo = "");
	// Description:
	//		Reboot a node
	// Parameters:
	//		reason			The reason for the reboot.
	//						The reason is stored in the ISP-log.
	//						The reason is also used to decide if
	//						reboot is possible. See method
	//						isRebootPossible for further info.
	//		userInfo		Optional user info to be stored in the ISP log
	// Return value:
	//		true			reboot ordered
	//		false			reboot failed. GetLastError may give more details.

	//  bool stopResource(const char* name,reasonType reason = userInitiated,const char* userInfo = "");
	//	Description:
	//		The method stops a specified resource.
	//	Parameters:
	//		name			Resource name
	//		reason			The reason for the stopResource.
	//						The reason is stored in the ISP-log.
	//		userInfo		Optional user info to be stored in the ISP log
	//	Return value:
	//		true			The resource is stopping but it may still be offline pending
	//						Use the method isResourceOffline to verify the resource
	//						is offline.
	//		false			Resource not stopped. GetLastError may give more details.

	//  bool startResource(const char* name,reasonType reason = userInitiated,const char* userInfo = "");
	//	Description:
	//		The method starts a specified resource
	//	Parameters:
	//		name			Resource name
	//		reason			The reason for the stopResource.
	//						The reason is stored in the ISP-log.
	//		userInfo		Optional user info to be stored in the ISP log
	//	Return value:
	//		true			The resource is starting but is may still be online pending
	//						Use the method isResourceOnline to verify the resource
	//						is online.
	//		false			Resource not started. GetLastError may give more details.

	//  bool stopGroup(const char* name,reasonType reason = userInitiated,const char* userInfo = "");
	//	Description:
	//		The method stops a specified resource group.
	//	Parameters:
	//		name			Resource Group name
	//		reason			The reason for the stopGroup.
	//						The reason is stored in the ISP-log.
	//		userInfo		Optional user info to be stored in the ISP log
	//	Return value:
	//		true			The group is stopping
	//						Use the method isGroupOffline to verify that all resources
	//						are offline.
	//		false			Resource Group not stopped. GetLastError may give more details.

	//  bool startGroup(const char* name,reasonType reason = userInitiated,const char* userInfo = "");
	//	Description:
	//		The method starts a specified resource group
	//	Parameters:
	//		name			Resource group name
	//		reason			The reason for the stopResource.
	//						The reason is stored in the ISP-log.
	//		userInfo		Optional user info to be stored in the ISP log
	//	Return value:
	//		true			The group is starting
	//						Use the method isGroupOnline to verify that all resources
	//						are online.
	//		false			Resource Group not started. GetLastError may give more details.

	//  bool isResourceOnline(const char* name);
	//  bool isResourceOffline(const char* name);
	//	Description:
	//		These methods are used to verify if a resource is online
	//		or offline.
	//	Parameters:
	//		name			The Resource name
	//	Return value:
	//		true			Resource is online/offline
	//		false			Resource is not online/offline or error
	//						detected. GetLastError() returns an error
	//						code if error is detected.

	//  bool isGroupOnline(const char* name,char* resourceList,int listSize,int& noResources);
	//  bool isGroupOffline(const char* name,char* resourceList,int listSize,int& noResources);
	// Description:
	//		These methods are used to verify if all resources in a group is online/offline.
	//		The methods returns a list of pending resources.
	// Parameters:
	//		name			The group to verify
	//		resourceList	A area in which the method may store
	//						a list of resources not in the expected state. 
	//						The resources in the list are separated
	//						by the zero-character. The list may be
	//						truncated if the area is not large enough.
	//		listSize		The size of the resourceList area
	//		noResources		OUT: Number of pending resources.
	// Return value:
	//		true			All resources online/offline
	//		false			Not all resources are online/offline or error
	//						detected. GetLastError() returns an error
	//						code if error is detected.
	//						noResources is set to 0 if an error is detected.
	
	bool getSUList(const char* Node, vector<string>& SuList);
	int getSuState(const char* SuName);
	time_t lastRebootTime();
	int askForNodeState( int Node = ACS_PRC_LOCAL_NODE );

	//  bool getGroupList(char* groupList,int listSize,int& noGroups);
	// Description:
	//		The method returns a list of groups.
	// Parameters:
	//		groupList		A area in which the method may store
	//						a list of groups. The 
	//						groups in the list are separated
	//						by the zero-character. The list may be
	//						truncated if the area is not large enough.
	//		listSize		The size of the groupList area
	//		noGroups		OUT: Number of groups.
	// Return value:
	//		true			A list is delivered
	//		false			Error detected. GetLastError may give more info.

	//  bool getResourceList(const char* name,char* resourceList,int listSize,int& noResources);
	// Description:
	//		The method returns a list of resources in the specified group.
	// Parameters:
	//		name			The group to list
	//		resourceList	A area in which the method may store
	//						a list of resources. The 
	//						resources in the list are separated
	//						by the zero-character. The list may be
	//						truncated if the area is not large enough.
	//		listSize		The size of the resourceList area
	//		noResources		OUT: Number of resources.
	// Return value:
	//		true			A list is delivered
	//		false			Error detected. GetLastError may give more info.

	//  bool changeToActiveNode(reasonType reason = userInitiated,const char* userInfo = "");
	//	Description:
	//		A failover to this node is started if this node is the
	//		passive node.
	//		Nothing is done if this node already is active.
	//		The method returns as soon as the failover is started.
	//	Parameters:
	//		reason			The reason for the failover.
	//						The reason is stored in the ISP-log.
	//		userInfo		Optional user info to be stored in the ISP log
	//	Return value:
	//		true			Failover started (or already active)
	//		false			Failover not started. GetLastError may give more details.

	//  bool failover(const char* group,bool toThisNode = true,reasonType reason = userInitiated,const char* userInfo = "");
	//	Description:
	//		A failover to this node or to the other nodeis started.
	//		The method returns as soon as the failover is started.
	//	Parameters:
	//		group			The group to failover
	//		toThisNode		If true, failover to this node
	//						otherwise failover to the other node
	//		reason			The reason for the failover.
	//						The reason is stored in the ISP-log.
	//		userInfo		Optional user info to be stored in the ISP log
	//	Return value:
	//		true			Failover started (or already active)
	//		false			Failover not started. GetLastError may give more details.

	//  bool isOwnerNode(const char* group);
	//  bool isOwnerNode(const char* group,const char* node);
	// Description:
	//		The method verifies if the group is owned by the
	//		specified node.
	// Parameters:
	//		group			The group name
	//		node			The node name. Current node is assumed if omitted.
	// Return value:
	//		true			The group is owned by the specified node
	//		false			The group is not owned by the specified
	//						group. Or error is detected.

	//  int askForNodeState();
	//	Description:
	//		Returns the node-state
	//	Return value:
	//		-1			Undefined
	//		0			Passive node
	//		1			Active node

	//  bool lastRebootTime(time_t& ft,bool startTime = false);
	//	Description:
	//		The method returns time for the last reboot.
	//	Parameters:
	//		ft(OUT)			boot time
	//		startTime		if true, time when the reboot started is returned
	//						else time when the reboot is finished.
	//	Return value:
	//		true			boot time returned
	//		false			error detected and time not returned

	//  bool addResource(const char* group,const char* resource,int restart,bool local);
	//  bool deleteResource(const char* resource,bool local);
	//  bool addGroup(const char* group,const char* node,bool local);
	//  bool deleteGroup(const char* group,bool local);
	//  bool setIspInfo(const char* group,int misc);
	//	Description:
	//		These methods are used by FCH (FCR) only. They are used
	//		to report addition/deletion of resources and groups.
	//		The cluster database is NOT updated by these methods.
	//		Only PRC internal view of the cluster database and the
	//		ISP information is updated.
	//	Parameters:
	//		group			Group to be added or removed
	//		resource		Resource to be added or removed
	//		node			The node where the group is located
	//		restart			Restart type
	//						ClusterResourceDontRestart
	//						ClusterResourceRestartNoNotify
	//						ClusterResourceRestartNotify
	//		local			If true, update PRC on this node only
	//						Else update both nodes.
	//		misc			0: Set ISP info 'stopped' by Function Change for all
	//						   resources in the specified group.
	//						1: Set ISP info rebooted by Function Change.
	//						   group is not used but visible in prc-log.
	//						2: Set ISP info 'stopped' by FCR for all
	//						   resources in the specified group.
	//						3: Set ISP info rebooted by FCR.
	//						   group is not used but visible in prc-log.
	//	Return value:
	//		true			The resource or group is added or removed
	//		false			Error detected. GetLastError may give more details.

	//  int isActiveBlock(const char* blockName);
	// Description:
	//		The method searches for blockName in the 'passive DataBase'.
	//		If found, the method returns 0 otherwise 1.
	//		That is, any blockName not found in the 'passive DataBase'
	//		is assumed to be active.
	// Parameters:
	//		blockName				CXC_NUMBER, For example CXC137416 or
	//								MODULE_NAME, For example DSDBIN or
	//								ServiceName, For example ACS_DSD_Daemon or
	//								ProcessName, For example ACS_DSDaemon.
	// Return value:
	//		0						Passive block
	//		1						Active block
	//		-1						Error. More info using GetLastError()

	//  bool getPackageInfo(const char* package,
	//								 const char* infoKey,
	//								 char* buffer,
	//								 int &bufferSize);
	// Description:
	//		The method is used to retrieve information about a
	//		package. Returned values are char*.
	//		The method can be used i tree ways:
	//		1.	Get a specified value from the registry:
	//			Enter package, infoKey and bufferSize.
	//			Requested value is received in the buffer and bufferSize.
	//		2.	Request all values for a package:
	//			Enter package. infoKey set to "". bufferSize large enough
	//			to contain all values.
	//			All values are received in the buffer in the form
	//			keyName=value\0
	//		3.	Decode values retrieved in 2 above.
	//			package is ignored. Enter infoKey and buffer.
	//			bufferSize set to 0.
	//			At return, bufferSize is the buffer-index to the first char
	//			in the value.
	// Parameters:
	//		package					Package name, 
	//								that is CXC-number (for example CXC1371060)
	//								or module-name (for example (USABIN).
	//		infoKey					Tells what information to retrieve.
	//								Possible values are:
	//								CNI 
	//								CXC_NAME
	//								CXC_NUMBER
	//								CXC_VERSION
	//								MODULE_NAME
	//								PHAPARAM
	//								PLATFORM_TYPE
	//								PRODUCT_NAME
	//								SUBSYSTEM
	//								or NULL or empty "". In that case all the above
	//								mentioned values are retrieved and stored in the
	//								buffer in the form:
	//								CNI=xxx\0CXC_NAME=xxx\0...\0\0
	//		buffer					Area used to store the retrieved value
	//		bufferSize				IN: Size of buffer
	//								OUT: Number of char stored in buffer
	// Return value:
	//		true					value returned
	//		false					value not returned. More info may be found
	//								using GetLastError()

	int getLastError(void) const;
	const char* getLastErrorText(void) const;

private:

	int getHaState ( SaNameT objectName );
	//bool getCounter(const char* entryName, int &value);
	SaImmHandleT immHandle;
	std::string errorText;
	int error;
	void setError(int err, const char* errText);
	int generalReboot (const char* initiator,  reasonType reason,  string message, bool reboot, bool thisNode,bool ignoreFCH, bool spawnProcess,bool log);
	string addStrings(const char* s1,const char* s2,const char* s3, string s4,const char* s5);
	int load_process_information (pid_t & process_id, char * process_name, size_t size);
	bool executeReboot (string message,bool reboot);
	bool rebootPRCMAN( );

protected :

	ACS_PRC_API( const ACS_PRC_API &){};

	inline ACS_PRC_API & operator=(const ACS_PRC_API &){ return *this; };

}; // End of class ACS_PRC_API

#endif
