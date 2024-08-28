#ifndef ACS_AEH_SIGNALHANDLER_H_
#define ACS_AEH_SIGNALHANDLER_H_

enum t_sigtype
{
   PRC
};

// Setting default signal handler
int ACS_AEH_setSignalExceptionHandler(const char command[], t_sigtype sigtype)
{
   return 0;
}

#endif /* ACS_AEH_SIGNALHANDLER_H_ */
