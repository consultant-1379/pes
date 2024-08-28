//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      hwcinfo.cpp
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
//      This class handles information about CP and blade cluster boards.
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
#include <hwcinfo.h>
#include <map>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
HWCInfo::HWCInfo() :
CPInfo(),
m_hwcTable(0),
m_side(false, 0),
m_boardId(false, 0)
{
    if (isMultiCPSystem() == false)
    {
        Exception ex(Exception::e_illopt_sysconf_116);
        ex << "CP name must not be specified for a one CP system.";
        throw ex;
    }        

    // Get HWC table information
    m_hwcTable = ACS_CS_API::createHWCInstance();
    if (m_hwcTable == 0)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create HWC instance.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
HWCInfo::~HWCInfo()
{
    ACS_CS_API::deleteHWCInstance(m_hwcTable);
}

//----------------------------------------------------------------------------------------
// Set CP information
//----------------------------------------------------------------------------------------
bool
HWCInfo::setSide(char side)
{
    // Get board identity
    BoardID boardId;

    bool found = getBoardId(getCpId(), side, boardId);
    if (found)
    {
        // Save values
        m_side = std::pair<bool, char>(true, side);
        m_boardId = std::pair<bool, BoardID>(true, boardId);

        return true;
    }
    else
    {
        return false;
    }
}

//----------------------------------------------------------------------------------------
// Get board identity for a CP (and side) 
//----------------------------------------------------------------------------------------
bool
HWCInfo::getBoardId(CPID cpId, char side, BoardID& boardId) const
{
    ACS_CS_API_BoardSearch* boardSearch = ACS_CS_API_HWC::createBoardSearchInstance();
    if (boardSearch == 0)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create board search instance.";
        throw ex;
    }

    ACS_CS_API_IdList boardIds;
    boardSearch->reset();               
    boardSearch->setFBN(ACS_CS_API_HWC_NS::FBN_CPUB);
                                       // Get CPU board
    boardSearch->setSysId(cpId);            // Get boards with current sysId

    ACS_CS_API_NS::CS_API_Result result;
    result = m_hwcTable->getBoardIds(boardIds, boardSearch);
    if (result != ACS_CS_API_NS::Result_Success)
    {
        ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);

        Exception ex(Exception::e_system_28);
        ex << "Failed to search for boards.";
        throw ex;
    }
    ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);

    unsigned short cpside;
    char tside(0);
    bool found(false);

    map<char, bool> cpconf;
    cpconf[0] = false;
    cpconf['A'] = false;
    cpconf['B'] = false;

    // Get CP side for each board found
    for (unsigned int j = 0; j < boardIds.size(); j++)  
    {
        result = m_hwcTable->getSide(cpside, boardIds[j]);
        if (result == ACS_CS_API_NS::Result_Success)
        {
            switch (cpside)
            {
                case ACS_CS_API_HWC_NS::Side_A: tside = 'A'; break;
                case ACS_CS_API_HWC_NS::Side_B: tside = 'B'; break;
                default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
            }
        }
        else if (result == ACS_CS_API_NS::Result_NoValue)
        {
            tside = 0;
        }
        else
        {
            Exception ex(Exception::e_system_28);
            ex << "Failed to get CP side.";
            throw ex;
        }
        
        if (cpconf[(int)tside] == false)
        {
            cpconf[(int)tside] = true;
        }
        else
        {
            Exception ex(Exception::e_system_28);
            ex << "Error in HWC table on AP - Ambiguity found.";
            throw ex;
        }

        switch (tside)
        {
        case 0: 
        case 'A':
            if (side == 0 || side == 'A')
            {
                found = true;
                boardId = boardIds[j];
            }
            break;

        default: // tside == 'B'
            if (side == 'B')
            {
                found = true;
                boardId = boardIds[j];
            }
        }
    }

    // Analyze the CP configuration
    unsigned short sysType = 1000 * (cpId/1000);        // System type
    switch (sysType)
    {
    case ACS_CS_API_HWC_NS::SysType_BC:        // Blade cluster
        if ((cpconf[0] && !cpconf['A'] && !cpconf['B']) ||
           (!cpconf[0] && cpconf['A'] && !cpconf['B']))
        {
            // No side or side 'A'
            if (side != 0)
            {
                Exception ex(Exception::e_illopt_sysconf_116);
                ex << "CP side is not allowed for a single sided CP.";
                throw ex;
            }
        }
        else if (!cpconf[0] && cpconf['A'] && cpconf['B'])
        {
            // Side 'A' and 'B'
            if (side == 0)
            {
                Exception ex(Exception::e_illopt_sysconf_116);
                ex << "CP side is required for a dual CP.";
                throw ex;
            }
        }
        else
        {
            Exception ex(Exception::e_system_28);
            ex << "Error in HWC table on AP - configuration does not match the system type.";
            throw ex;
        }
        break;
    
    case ACS_CS_API_HWC_NS::SysType_CP:            // CP
        if (!cpconf[0] && cpconf['A'] && cpconf['B'])
        {
            // Side 'A' and 'B'
            if (side == 0)
            {
                Exception ex(Exception::e_illopt_sysconf_116);
                ex << "CP side is required for a dual CP.";
                throw ex;
            }
        }
        else
        {
            Exception ex(Exception::e_system_28);
            ex << "Error in HWC table on AP - configuration does not match the system type.";
            throw ex;
        }
        break;

    case ACS_CS_API_HWC_NS::SysType_AP:
    default:
        throw Exception(Exception::e_system_28, "Unexpected system type.");
    }

    return found;
}

//----------------------------------------------------------------------------------------
// Get CP side
//----------------------------------------------------------------------------------------
char 
HWCInfo::getSide() const
{
    if (m_side.first == true)
    {    
        return m_side.second;
    }
    else
    {
        Exception ex(Exception::e_internal_29);
        ex << "CP side is undefined.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Get magazine value
//----------------------------------------------------------------------------------------
uint32_t
HWCInfo::getMagazine() const
{
    if (m_boardId.first == false)
    {
        Exception ex(Exception::e_internal_29);
        ex << "Board identity is undefined.";
        throw ex;
    }

    ACS_CS_API_NS::CS_API_Result result;
    //unsigned long mag;
    uint32_t mag;

    result = m_hwcTable->getMagazine(mag, m_boardId.second); 
    if (result != ACS_CS_API_NS::Result_Success)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to get magazine value.";
        throw ex;
    }
    return mag;
}

//----------------------------------------------------------------------------------------
// Get slot value
//----------------------------------------------------------------------------------------
unsigned short
HWCInfo::getSlot() const
{
    if (m_boardId.first == false)
    {
        Exception ex(Exception::e_internal_29);
        ex << "Board identity is undefined.";
        throw ex;
    }

    ACS_CS_API_NS::CS_API_Result result;
    unsigned short slot;

    result = m_hwcTable->getSlot(slot, m_boardId.second);   
    if (result != ACS_CS_API_NS::Result_Success)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to get slot value.";
        throw ex;
    }
    return slot;
}
