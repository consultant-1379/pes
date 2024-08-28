//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      amfcallbackcontrol.cpp
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

#include "amfcallbackcontrol.h"
#include <logger.h>
#include <server.h>
#include <signal.h>
#include <sys/eventfd.h>

using namespace std;

namespace PES_CLH {

const char* AMFcallbackControl::s_state[] =
{
   "",
   "ACTIVE",
   "STANDBY",
   "QUIESCED",
   "QUIESCING",
   "UNDEFINED"
};

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
AMFcallbackControl::AMFcallbackControl(Engine* engine, const string& daemon_name):
ACS_APGCC_ApplicationManager(daemon_name.c_str()),
m_engine(engine),
m_thread(),
m_healthCount(0)
{
}

//----------------------------------------------------------------------------------------
//	Destructor
//----------------------------------------------------------------------------------------
AMFcallbackControl::~AMFcallbackControl()
{
}

//----------------------------------------------------------------------------------------
// The component received the state "ACTIVE"
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performStateTransitionToActiveJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState)
{
   ACS_APGCC_ReturnType retcode;
   switch (previousHAState)
   {
      case ACS_APGCC_AMF_HA_ACTIVE:
         retcode = ACS_APGCC_SUCCESS;
         break;

      default:
      {
         ostringstream s;
         s << "State transition from '" << s_state[previousHAState] << "' occurred.";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());

         retcode = startJob();
      }
   }

   return retcode;
}

//----------------------------------------------------------------------------------------
// The component received the state "STANDBY"
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performStateTransitionToPassiveJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState)
{
   switch (previousHAState)
   {
      case ACS_APGCC_AMF_HA_STANDBY:
         break;

      default:
      {
         ostringstream s;
         s << "State transition from '" << s_state[previousHAState] << "' occurred.";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());

         stopJob();
      }
   }

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// The component received the state "QUIESCING"
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performStateTransitionToQueisingJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState)
{
   switch (previousHAState)
   {
      case ACS_APGCC_AMF_HA_QUIESCING:
         break;

      default:
      {
         ostringstream s;
         s << "State transition from '" << s_state[previousHAState] << "' occurred.";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());

         stopJob();
      }
   }

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// The component received the state "QUIESCED"
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performStateTransitionToQuiescedJobs(
         ACS_APGCC_AMF_HA_StateT previousHAState)
{
   switch (previousHAState)
   {
      case ACS_APGCC_AMF_HA_QUIESCED:
         break;

      default:
      {
         ostringstream s;
         s << "State transition from '" << s_state[previousHAState] << "' occurred.";
         Logger::event(LOG_LEVEL_INFO, WHERE__, s.str());

         stopJob();
      }
   }

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// The component is ready to terminate itself
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performComponentTerminateJobs()
{
   Logger::event(
            LOG_LEVEL_INFO,
            WHERE__,
            "My application component received terminate callback."
            );

   stopJob();

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// Removal jobs are done
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performComponentRemoveJobs()
{
   Logger::event(
            LOG_LEVEL_INFO,
            WHERE__,
            "Application assignment is removed now."
            );

   stopJob();

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// Perform shutdown operations
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performApplicationShutdownJobs()
{
   Logger::event(
            LOG_LEVEL_INFO,
            WHERE__,
            "Shutting down the application."
            );

   stopJob();

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// Perform health check
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::performComponentHealthCheck()
{
   // Application has received health check callback from AMF. Check the
   // sanity of the application and reply to AMF that you are ok.
   ++m_healthCount;
   if (m_healthCount == 100)
   {
      Logger logger(LOG_LEVEL_TRACE);
      if (logger)
      {
         logger.event(
                  WHERE__,
                  "My application component received health check query 100 times."
                  );
      }
      m_healthCount = 0;
   }

   return ACS_APGCC_SUCCESS;
}

//----------------------------------------------------------------------------------------
// Execute server code
//----------------------------------------------------------------------------------------
void AMFcallbackControl::execute()
{
   try
   {
      if (m_engine != 0)
      {
         m_engine->execute();
      }
   }
   catch (Exception& ex)
   {
      Logger::event(ex);

      // Application thread being exited due to an exception
      componentReportError(ACS_APGCC_COMPONENT_RESTART);
   }
   catch (exception& e)
   {
      // Boost exception
      Exception ex(Exception::system(), WHERE__);
      ex << e.what();
      Logger::event(ex);

      // Application thread being exited due to an exception
      componentReportError(ACS_APGCC_COMPONENT_RESTART);
   }
}

//----------------------------------------------------------------------------------------
// Start server thread
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType AMFcallbackControl::startJob()
{
   ACS_APGCC_ReturnType retcode;
   if (m_thread.get_id() == boost::thread::id())
   {
      try
      {
         Engine::setStopPoint(false);
         m_thread = boost::thread(&AMFcallbackControl::execute, this);

         Logger::event(
                  LOG_LEVEL_INFO,
                  WHERE__,
                  "The application thread was created."
                  );

         retcode = ACS_APGCC_SUCCESS;
      }
      catch (exception& e)
      {
         ostringstream s;
         s << "Failed to create application thread." << endl;
         s << e.what();
         Logger::event(LOG_LEVEL_ERROR, WHERE__, s.str());

         retcode = ACS_APGCC_FAILURE;
      }
   }
   else
   {
      retcode = ACS_APGCC_SUCCESS;
   }

   return retcode;
}

//----------------------------------------------------------------------------------------
// Shutdown server thread gracefully
//----------------------------------------------------------------------------------------
void AMFcallbackControl::stopJob()
{
   if (m_thread.get_id() != boost::thread::id())
   {
      if (m_engine != 0)
      {
         Engine::setStopPoint(true);
         m_engine->reset(Engine::e_shutdown);
      }
      m_thread.join();

      Logger::event(
                  LOG_LEVEL_INFO,
                  WHERE__,
                  "Application thread exited successfully."
                  );
   }
}

//----------------------------------------------------------------------------------------
// Init the Engine pointer
//----------------------------------------------------------------------------------------
void AMFcallbackControl::initEngine()
{
   m_engine = new Engine;
}

}
