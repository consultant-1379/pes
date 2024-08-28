#include "acs_aeh_error.h"

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
acs_aeh_error::acs_aeh_error() :
m_error(ACS_AEH_noErrorType),
m_errorText(ACS_AEH_noError)
{
}

acs_aeh_error::acs_aeh_error(const acs_aeh_error& error) :
m_error(error.m_error),
m_errorText(error.m_errorText)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
acs_aeh_error::~acs_aeh_error()
{
}

//----------------------------------------------------------------------------------------
// Assignment operator
//----------------------------------------------------------------------------------------
acs_aeh_error& acs_aeh_error::operator=(const acs_aeh_error& error)
{
   return *this;
}

//----------------------------------------------------------------------------------------
// Get error
//----------------------------------------------------------------------------------------
ACS_AEH_ErrorType acs_aeh_error::getError() const
{
   return m_error;
}

//----------------------------------------------------------------------------------------
// Get error text
//----------------------------------------------------------------------------------------
const char* acs_aeh_error::getErrorText() const
{
   return m_errorText;
}
