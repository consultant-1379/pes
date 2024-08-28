//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cpinfo.h
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

#ifndef CPINFO_H
#define CPINFO_H

#include <ACS_CS_API.h>
#include <string>

class CPInfo
{
public:
    CPInfo();
    virtual ~CPInfo();

    // Set CP identity
    bool setId(                                     // Returns true if CP identity found,
                                                    // false otherwise
            CPID cpId                               // CP identity
            );

    // Set CP name
    bool setName(                                   // Returns true if CP name found,
                                                    // false otherwise
            const std::string& name                 // CP name
            );

    // Get CP name
    std::string getName() const;                    // Returns CP name

    // Get CP identity
    CPID getCpId() const;                           // Returns CP identity

    // Get CP type
    unsigned short getCPType() const;               // Returns CP type

    // Get default CP name
    static std::string getDefaultCPName(            // Returns default CP name
            CPID cpId                               // CP identity
            );

    // Inquire if this is a multiple CP system
    static bool isMultiCPSystem();                  // Returns true if multiple CP system,
                                                    // false otherwise
protected:
    std::pair<bool, CPID> m_cpId;
    std::string m_name;

private:
    CPInfo(const CPInfo&) {}

    CPInfo& operator=(
            const CPInfo&
            ) 
            {return *this;}                        //lint !e1529

    ACS_CS_API_CP* m_cpTable;
};

#endif
