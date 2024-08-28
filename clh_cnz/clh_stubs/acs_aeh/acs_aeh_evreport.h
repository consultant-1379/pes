#ifndef ACS_AEH_EVREPORT_H_
#define ACS_AEH_EVREPORT_H_

#include <stdint.h>

enum ACS_AEH_ErrorReturnType
{
   ACS_AEH_syntaxError = 0,
   ACS_AEH_eventDeliveryFailure
};

enum ACS_AEH_ReturnType
{
   ACS_AEH_Not_ok = 0,
   ACS_AEH_ok = 1
};

class ACS_AEH_EvReport
{
public:
   ACS_AEH_EvReport();

   ACS_AEH_ReturnType sendEventMessage(
            const char* processName,
            uint32_t eventNumber,
            const char* severity,
            const char* cause,
            const char* eventHeader,
            const char* objectOfReference,
            const char* data,
            const char* text
            );

   ACS_AEH_ErrorReturnType getError();

private:
   static const char s_eventfile[];
};

#endif
