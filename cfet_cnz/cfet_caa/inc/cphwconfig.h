//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cphwconfig.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2015. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class handles the CPHW configuration files.
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
//      B      2011-11-11   UABMAGN     Implemented CXP package handling
//      -      2017-05-17   XMAHIMA     Updated for SW MAU(MAUS) Improvement feature
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef CPHWCONFIG_H
#define CPHWCONFIG_H

#include "common.h"
#include <string>

template<Common::t_cptype>
class Cphwconfig;

//----------------------------------------------------------------------------------------
// Base class for CPHW configuration
//----------------------------------------------------------------------------------------

class CphwconfigBase
{
    friend class Cphwconfig<Common::e_onecp>;

public:
    typedef std::pair<std::string, std::string> PAIR;

    // Constructor
    CphwconfigBase();

    // Destructor
    virtual ~CphwconfigBase();

    // Equality operator
    bool operator==(                                 // Returns true if equal, false otherwise
            const CphwconfigBase& conf               // Object to compare
            ) const;

    // Get the DUMP path and filename
    PAIR getDump() const;                            // Returns the DUMP path and filename

    // Set the DUMP and filename
    void setDump(
            const PAIR& dump                         // DUMP path and filename
            );

    static const char s_HEADING[];
    static const char s_HEADING_LZY[];

protected:
    std::string m_dump;
};

//----------------------------------------------------------------------------------------
// Multi CP system
//----------------------------------------------------------------------------------------

template<>
class Cphwconfig<Common::e_multicp> : public CphwconfigBase
{
public:
    enum sysType_t {e_cp, e_bc};

    // Constructor
    Cphwconfig();

    // Destructor
    ~Cphwconfig();

    // Read the CPHW configuration file
    void read(
            const std::string& path                 // Path to the CPHW configuration file
            );

    // Write the CPHW configuration file
    void write(
            const std::string& path                 // Path to the CPHW configuration file
            );

    // Get the CP type
    unsigned short getCPType() const;               // Returns the CP type

    // Get the system type
    sysType_t getSysType() const;                   // Returns the system type

    // Set the CP type
    void setCpType(
            unsigned short cpType                   // CP type
            );

    // Set the system type
    void setSysType(
            sysType_t sysType                       // System type
            );
    std::string m_cpMauVersionMCP;

private:
    std::pair<bool, unsigned short> m_cpType;
    std::pair<bool, sysType_t> m_sysType;

    static const std::string& s_CPHW_LZY;
    static const std::string& s_CP_TYPE;
    static const std::string& s_SYS_TYPE;
    static const std::string& s_CP_MAU;
    static const unsigned short s_APZMIN;
    static const unsigned short s_APZMAX;
};

//----------------------------------------------------------------------------------------
// One CP system
//----------------------------------------------------------------------------------------

template<>
class Cphwconfig<Common::e_onecp>
{
public:
    typedef std::pair<std::string, std::string> PAIR;

    // Constructor
    Cphwconfig();

    // Destructor
    ~Cphwconfig();

    CphwconfigBase& operator[](                   // Returns base class object
            char side                             // CP side, 'A' or 'B'
            );

    // Read the CPHW configuration file
    void read(
            const std::string& path              // Path to the CPHW configuration file
            );

    // Write the CPHW configuration file
    void write(
            const std::string& path              // Path to the CPHW configuration file
            );
    std::string m_cpMauCurVersion[2];

private:
    int checkSide(                               // Returns 0 for 'A'-sid
                                                 // and 1 for 'B'-side
            char side                            // CP side, 'A' or 'B'
            ) const;

    CphwconfigBase m_configbase[2];
};

#endif
