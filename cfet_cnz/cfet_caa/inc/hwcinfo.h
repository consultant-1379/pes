//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      hwcinfo.h
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

#ifndef HWCINFO_H
#define HWCINFO_H

#include "cpinfo.h"
#include <ACS_CS_API.h>
#include <string>

class HWCInfo : public CPInfo
{
public:
    HWCInfo();
    ~HWCInfo();

    // Set CP side
    bool setSide(                                   // Returns true if board exists,
                                                    // false otherwise
            char side                               // CP side, 'A', 'B' or 0
            );

    // Get CP side
    char getSide() const;                           // Returns CP side, 'A', 'B' or 0

    // Get magazine value
    uint32_t getMagazine() const;                   // Returns magazine value

    // Get slot value
    unsigned short getSlot() const;                 // Returns slot value        

private:
    HWCInfo(
            const HWCInfo&
            ) : CPInfo() {}

    HWCInfo& operator=(
            const HWCInfo&
            ) 
            {return *this;}                         //lint !e1529
    
    //    Get board identity
    bool getBoardId(                                // Returns true if board identity found,
                                                    // false otherwise
            CPID cpId,                              // CP identity
            char side,                              // CP side, 'A', 'B' or 0
            BoardID& boardId                        // Board identity
            ) const;

    ACS_CS_API_HWC* m_hwcTable;
    std::pair<bool, char> m_side;
    std::pair<bool, BoardID> m_boardId;
};

#endif
