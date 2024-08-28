#include "acs_aeh_evreport.h"
#include <iostream>
#include <fstream>

using namespace std;

const char acs_aeh_evreport::s_eventfile[] = "eventfile.log";

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
acs_aeh_evreport::acs_aeh_evreport() :
acs_aeh_error()
{
}

acs_aeh_evreport::acs_aeh_evreport(const acs_aeh_evreport& evReport) :
acs_aeh_error(evReport)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
acs_aeh_evreport::~acs_aeh_evreport()
{
}

//----------------------------------------------------------------------------------------
// Send a event/alarm to AEH
//----------------------------------------------------------------------------------------
ACS_AEH_ReturnType acs_aeh_evreport::sendEventMessage(
                        acs_aeh_processName processNamePtr,
                        acs_aeh_specificProblem specificProblem,
                        acs_aeh_percSeverity percSeverityPtr,
                        acs_aeh_probableCause probableCausePtr,
                        acs_aeh_objClassOfReference objClassOfReferencePtr,
                        acs_aeh_objectOfReference objectOfReferencePtr,
                        acs_aeh_problemData problemDataPtr,
                        acs_aeh_problemText problemTextPtr
                        )
{
   ofstream of(s_eventfile, ios::out | ios::app);
   if (of.is_open())
   {
      of << "======================================" << endl;
      of << "Process Name:         " << processNamePtr << endl;
      of << "Specific Problem:     " << specificProblem << endl;
      of << "Severity:             " << percSeverityPtr << endl;
      of << "Probable Cause:       " << probableCausePtr << endl;
      of << "Object class of ref.: " << objClassOfReferencePtr << endl;
      of << "Object of reference:  " << objectOfReferencePtr << endl;
      of << "Problem data:         " << problemDataPtr << endl;
      of << "Problem text:         " << problemTextPtr << endl;
      of << endl;

      m_error = ACS_AEH_noErrorType;
      m_errorText = ACS_AEH_noError;

      return ACS_AEH_ok;
   }
   else
   {
      m_error = ACS_AEH_eventDeliveryFailure;
      m_errorText = "Failed to write to log file.";

      return ACS_AEH_error;
   }
}

