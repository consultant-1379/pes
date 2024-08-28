//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cpinfo.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2010. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class handles information about the central processors.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 0xxx  PA1
//
//  AUTHOR
//      EAB/FLE/DF UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      A      2010-03-16   UABTSO      CFET was migrated to APG50.
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <exception.h>
#include <cpinfo.h>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
CPInfo::CPInfo() :
m_cpId(false, 0),
m_name(),
m_cpTable(0)
{
    if (isMultiCPSystem() == false)
    {
        Exception ex(Exception::e_illopt_sysconf_116);
        ex << "CP name must not be specified for a one CP system.";
        throw ex;
    }        

    m_cpTable = ACS_CS_API::createCPInstance();
    if (m_cpTable == 0)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create CP instance.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
CPInfo::~CPInfo()
{
    ACS_CS_API::deleteCPInstance(m_cpTable);
}

//----------------------------------------------------------------------------------------
// Set CP identity
//----------------------------------------------------------------------------------------
bool
CPInfo::setId(CPID cpId)
{
    ACS_CS_API_NS::CS_API_Result result;
    ACS_CS_API_Name cpName;

    // Get CP name
    result =  m_cpTable->getCPName(cpId, cpName);
    if (result != ACS_CS_API_NS::Result_Success)
    {
        // CP identity not found
        return false;
    }

    // Get default CP name
    result = ACS_CS_API_NetworkElement::getDefaultCPName(cpId, cpName);
    if (result != ACS_CS_API_NS::Result_Success)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to find CP identity " << cpId << ".";
        throw ex;
    }

    size_t size(127);
    char buf[128];
    result = cpName.getName(buf, size);

    // Save values
    m_cpId = std::pair<bool, CPID>(true, cpId);
    m_name = buf;

    return true;
}

//----------------------------------------------------------------------------------------
// Set CP name
//----------------------------------------------------------------------------------------
bool 
CPInfo::setName(const string& name)
{
    ACS_CS_API_NS::CS_API_Result result;
    CPID cpId(0);
    ACS_CS_API_Name cpName(name.c_str());

    // Get CP identity
    result = m_cpTable->getCPId(cpName, cpId);
    if (result != ACS_CS_API_NS::Result_Success)
    {
        // CP name not found
        return false;
    }

    // Save values
    m_cpId = std::pair<bool, CPID>(true, cpId);
    m_name = getDefaultCPName(cpId);

    return true;
}

//----------------------------------------------------------------------------------------
// Get CP name 
//----------------------------------------------------------------------------------------
string 
CPInfo::getName() const
{
    if (m_name.empty() == false)
    {
        return m_name;
    }
    else
    {
        Exception ex(Exception::e_internal_29);
        ex << "CP name is undefined.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Get CP identity 
//----------------------------------------------------------------------------------------
CPID 
CPInfo::getCpId() const
{
    if (m_cpId.first == true)
    {
        return m_cpId.second;
    }
    else
    {
        Exception ex(Exception::e_internal_29);
        ex << "CP identity is undefined.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Get CP type
//----------------------------------------------------------------------------------------
unsigned short 
CPInfo::getCPType() const
{
    unsigned short cpType;
    ACS_CS_API_NS::CS_API_Result result = m_cpTable->getCPType(getCpId(), cpType);
    if (result == ACS_CS_API_NS::Result_NoValue)
    {
        Exception ex(Exception::e_system_28);
        ex << "CP type was not found.";
        throw ex;
    }
    else if (result != ACS_CS_API_NS::Result_Success)
    {
        Exception ex(Exception::e_system_28);
        ex << "Could not get CP type - ";
        switch (result)
        {
        case ACS_CS_API_NS::Result_NoEntry:     ex << "no entry.";    break;
        case ACS_CS_API_NS::Result_NoAccess: ex << "no access."; break;
        case ACS_CS_API_NS::Result_Failure:     ex << "failure.";   break;
        default: ex << "Unknown error.";
        }
        throw ex;
    }

    return cpType;    
}

//----------------------------------------------------------------------------------------
// Get default CP name
//----------------------------------------------------------------------------------------
string
CPInfo::getDefaultCPName(CPID cpId)
{
    ACS_CS_API_NS::CS_API_Result result;
    ACS_CS_API_Name cpName;

    result = ACS_CS_API_NetworkElement::getDefaultCPName(cpId, cpName);
    if (result != ACS_CS_API_NS::Result_Success)
    {
        Exception ex(Exception::e_general_1);
        ex << "Failed to find CP identity " << cpId << ".";
        throw ex;
    }

    size_t size(127);
    char buf[128];
    result = cpName.getName(buf, size);
    if (result != ACS_CS_API_NS::Result_Success)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to get CP name " << cpId << ".";
        throw ex;
    }

    return buf;
}

//----------------------------------------------------------------------------------------
// Check if this is a multiple CP system
//----------------------------------------------------------------------------------------
bool
CPInfo::isMultiCPSystem()
{
    bool multiCPSystem(false);
    ACS_CS_API_NS::CS_API_Result result;
    try
    {
        result = ACS_CS_API_NetworkElement::isMultipleCPSystem(multiCPSystem);
    }
    catch (...)
    {
        // Exception thrown probably due to CSAPI is missing.
        // Assume this is not a multi CP system
        return false;
    }
    if (result != ACS_CS_API_NS::Result_Success)
    {    
        Exception ex(Exception::e_system_28);
        ex << "Call to CS API failed.";
        throw ex;
    }
    return multiCPSystem;
}
