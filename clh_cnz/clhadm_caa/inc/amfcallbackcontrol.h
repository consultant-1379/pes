//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      amfcallbackcontrol.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013, 2014. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for the AMF callback control routines.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1416  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-01-08   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef AMFCALLBACKCONTROL_H_
#define AMFCALLBACKCONTROL_H_

#include "engine.h"
#include "ACS_APGCC_ApplicationManager.h"
#include <boost/thread.hpp>
#include <stdint.h>

namespace PES_CLH {

class AMFcallbackControl: public ACS_APGCC_ApplicationManager
{

public:
   AMFcallbackControl(
         Engine* engine,
         const std::string& daemon_name
         );

   virtual ~AMFcallbackControl();


   ACS_APGCC_ReturnType performStateTransitionToActiveJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState
         );

   ACS_APGCC_ReturnType performStateTransitionToPassiveJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState
         );

   ACS_APGCC_ReturnType performStateTransitionToQueisingJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState
         );

   ACS_APGCC_ReturnType performStateTransitionToQuiescedJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState
         );

   ACS_APGCC_ReturnType performComponentTerminateJobs();

   ACS_APGCC_ReturnType performComponentRemoveJobs();

   ACS_APGCC_ReturnType performApplicationShutdownJobs();

   ACS_APGCC_ReturnType performComponentHealthCheck();
   
   void initEngine();

private:
   // Disable default copy constructor
   AMFcallbackControl();

   // Execute server code
   void execute();

   // Start server thread
   ACS_APGCC_ReturnType startJob();

   // Shutdown server thread gracefully
   void stopJob();

   Engine* m_engine;
   boost::thread m_thread;
   uint16_t m_healthCount;

   static const char* s_state[];
};

}

#endif // AMFCALLBACKCONTROL_H_
