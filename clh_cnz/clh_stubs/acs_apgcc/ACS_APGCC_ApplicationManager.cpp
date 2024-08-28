#include "ACS_APGCC_ApplicationManager.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
ACS_APGCC_ApplicationManager::ACS_APGCC_ApplicationManager(const char* name, const char*):
HAState(ACS_APGCC_AMF_HA_UNDEFINED),
daemonName(name)
{
}

//----------------------------------------------------------------------------------------
// This routine performs the steps required for the applications to register with coremw
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::coreMWInitialize()
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine returns the current HA State of the calling application
//----------------------------------------------------------------------------------------
ACS_APGCC_AMF_HA_StateT ACS_APGCC_ApplicationManager::getHAState() const
{
   return ACS_APGCC_AMF_HA_UNDEFINED;
}

//----------------------------------------------------------------------------------------
// This routine returns the SelectionObject Handle which application can use to receive
// the callbacks from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_SelObjT ACS_APGCC_ApplicationManager::getSelObj() const
{
   return ACS_APGCC_SelObjT();
}

//----------------------------------------------------------------------------------------
// This routine is used by the application when it needs to clear error reported
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::componentReportError(
                                       ACS_APGCC_AMF_RecommendedRecoveryT)
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is used by the application when it needs to dispatch the callbacks
// received from the CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::dispatch(
                                       ACS_APGCC_AMF_DispatchFlagsT)
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// HAState as ACTIVE from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performStateTransitionToActiveJobs(
                                       ACS_APGCC_AMF_HA_StateT)
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// HAState as STANDBY from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performStateTransitionToPassiveJobs(
                                       ACS_APGCC_AMF_HA_StateT)
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// HAState as QUEISING from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performStateTransitionToQueisingJobs(
                                       ACS_APGCC_AMF_HA_StateT)
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// Health Check callback from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performStateTransitionToQuiescedJobs(
                                       ACS_APGCC_AMF_HA_StateT
                                       )
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// Health Check callback from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performComponentHealthCheck()
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// Terminate callback from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performComponentTerminateJobs()
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// Component Remove callback from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performComponentRemoveJobs()
{
   return ACS_APGCC_FAILURE;
}

//----------------------------------------------------------------------------------------
// Set state
//----------------------------------------------------------------------------------------
void ACS_APGCC_ApplicationManager::setState()
{
   bool cont = true;
   do
   {
      cout << endl;
      cout << "Specify state:" << endl;
      cout << "0  Keep state" << endl;
      cout << "1  ACS_APGCC_AMF_HA_ACTIVE" << endl;
      cout << "2  ACS_APGCC_AMF_HA_STANDBY" << endl;
      cout << "3  ACS_APGCC_AMF_HA_QUIESCED" << endl;
      cout << "4  ACS_APGCC_AMF_HA_QUIESCING" << endl;
      cout << "5  ACS_APGCC_AMF_HA_UNDEFINED" << endl;
      cout << "> ";
      string cmd;
      getline(cin, cmd);
      if (cmd.empty()) continue;
      try
      {
         int num = boost::lexical_cast<int>(cmd);
         switch (num)
         {
            case 0: break;
            case 1: HAState = ACS_APGCC_AMF_HA_ACTIVE;    break;
            case 2: HAState = ACS_APGCC_AMF_HA_STANDBY;   break;
            case 3: HAState = ACS_APGCC_AMF_HA_QUIESCED;  break;
            case 4: HAState = ACS_APGCC_AMF_HA_QUIESCING; break;
            case 5: HAState = ACS_APGCC_AMF_HA_UNDEFINED; break;
            default: throw exception();
         }
         cont = false;
      }
      catch (exception&)
      {
         cont = true;
         cerr << cmd << " is not a valid state" << endl;
      }
   }
   while (cont);
}

//----------------------------------------------------------------------------------------
// This routine is a virtual method and application shall use to override this method.
// This route is invoked by the by the common class ( this class ) when it receives
// ComponentActivate callback from CoreMW
//----------------------------------------------------------------------------------------
ACS_APGCC_HA_ReturnType ACS_APGCC_ApplicationManager::activate()
{
   HAState = ACS_APGCC_AMF_HA_UNDEFINED;
   cout << endl;
   cout << "Activating AMF test program:" << endl;
   do
   {
      cout << endl;
      cout << "Specify callback function:" << endl;
      cout << "0  performComponentHealthCheck" << endl;
      cout << "1  performStateTransitionToActiveJobs" << endl;
      cout << "2  performStateTransitionToPassiveJobs" << endl;
      cout << "3  performStateTransitionToQueisingJobs" << endl;
      cout << "4  performStateTransitionToQuiescedJobs" << endl;
      cout << "5  performComponentTerminateJobs" << endl;
      cout << "6  performComponentRemoveJobs" << endl;
      cout << "7  performApplicationShutdownJobs" << endl;
      cout << "> ";
      string cmd;
      getline(cin, cmd);
      if (cmd.empty()) continue;
      try
      {
         int num = boost::lexical_cast<int>(cmd);
         cout << "num: " << num << endl;
         switch (num)
         {
            case 0:
               performComponentHealthCheck();
               break;

            case 1:
               setState();
               performStateTransitionToActiveJobs(HAState);
               HAState = ACS_APGCC_AMF_HA_ACTIVE;
               break;

            case 2:
               setState();
               performStateTransitionToPassiveJobs(HAState);
               HAState = ACS_APGCC_AMF_HA_STANDBY;
               break;

            case 3:
               setState();
               performStateTransitionToQueisingJobs(HAState);
               HAState = ACS_APGCC_AMF_HA_QUIESCING;
               break;

            case 4:
               setState();
               performStateTransitionToQuiescedJobs(HAState);
               HAState = ACS_APGCC_AMF_HA_QUIESCED;
               break;

            case 5:
               performComponentTerminateJobs();
               break;

            case 6:
               performComponentRemoveJobs();
               break;

            case 7:
               performApplicationShutdownJobs();
               HAState = ACS_APGCC_AMF_HA_UNDEFINED;
               break;

            default:
               throw exception();
         }
         sleep(1);
      }
      catch (exception&)
      {
         cerr << cmd << " is not a valid callback" << endl;
      }
   }
   while (true);

   return ACS_APGCC_HA_SUCCESS;
}

//----------------------------------------------------------------------------------------
// This routine is virtual method, shall be used by applications to perform the shutdown
// operations gracefully.
//----------------------------------------------------------------------------------------
ACS_APGCC_ReturnType ACS_APGCC_ApplicationManager::performApplicationShutdownJobs()
{
   return ACS_APGCC_FAILURE;
}
