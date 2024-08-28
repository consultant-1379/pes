#include "acs_apgcc_omhandler.h"

using namespace std;

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
OmHandler::OmHandler():
m_initialized(false)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
OmHandler::~OmHandler()
{
}

//----------------------------------------------------------------------------------------
// Initialize Interaction between the calling process and IMM.
//----------------------------------------------------------------------------------------
ACS_CC_ReturnType OmHandler::Init()
{
   if (m_initialized == false)
   {
      m_initialized = true;
      return ACS_CC_SUCCESS;
   }
   else
   {
      return ACS_CC_FAILURE;
   }
}

//----------------------------------------------------------------------------------------
// Finalize interaction with IMM and release all handler to it.
//----------------------------------------------------------------------------------------
ACS_CC_ReturnType OmHandler::Finalize()
{
   if (m_initialized == true)
   {
      m_initialized = false;
      return ACS_CC_SUCCESS;
   }
   else
   {
      return ACS_CC_FAILURE;
   }
}

//----------------------------------------------------------------------------------------
// Retrieve the dn of all object that are instances of the class having the name
//----------------------------------------------------------------------------------------
ACS_CC_ReturnType OmHandler::getClassInstances(const char* p_className, vector<string>& p_dnList)
{
   p_dnList.clear();
   if (m_initialized == true)
   {
      p_dnList.push_back("apzFunctionsId=1");
      return ACS_CC_SUCCESS;
   }
   else
   {
      return ACS_CC_FAILURE;
   }
}

