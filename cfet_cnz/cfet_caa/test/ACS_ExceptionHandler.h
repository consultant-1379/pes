#ifndef ACS_EXCEPTIONHANDLER_H_
#define ACS_EXCEPTIONHANDLER_H_

 // Follow AP design rule for crash dump config and handling.
int AP_SetCleanupAndCrashRoutine(const char command[], void*)
{
   return 0;
}

#endif /* ACS_EXCEPTIONHANDLER_H_ */
