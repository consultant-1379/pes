//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      pesconfig.h
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
//      This class handles the PES configuration files.
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
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef PESCONFIG_H
#define PESCONFIG_H

#include "common.h"
#include <string>

template<Common::t_cptype>
class Pesconfig;

//----------------------------------------------------------------------------------------
// Base class for PES configuration
//----------------------------------------------------------------------------------------

class PesconfigBase
{
    friend class Pesconfig<Common::e_multicp>;
    friend class Pesconfig<Common::e_onecp>;

public:
    typedef std::pair<std::string, std::string> PAIR;

    // Constructor
    PesconfigBase();

    // Destructor
    virtual ~PesconfigBase();

    // Equality operator
    bool operator==(                                // Returns true if equal, false otherwise
            const PesconfigBase& conf               // Object to compare
            ) const;

    // Get the APZ VM path and file name
    PAIR getApzVm() const;                          // Returns the APZ VM path and file name

    // Get the PCIH Server path and file name
    PAIR getPcihSrv() const;                        // Returns the PCIH Server path and file name

    // Get the APZ configuration path and file name
    PAIR getApzConfig() const;                      // Returns the APZ configuration path and 
                                                    // file name

    // Get the PES DUMP file name
    std::string getDump() const;                    // Returns the PES DUMP file name

    // Set the APZ VM path and file name
    void setApzVm(
            const PAIR& apzvm                       // APZ VM path and file name
            );

    // Set the APZ configuration path and file name
    void setApzConfig(
            const PAIR& apzconfig                   // APZ configuration path and file name
            );

    // Set the PES DUMP file name
    void setDump(
            const std::string& dump                 // PES DUMP file name
            );

    // Set the PCIHSRV file name
    void setPcihSrv(
            const std::string& pcihsrv              // PCIHSRV file name
            );

    static const char s_HEADING[];
    static const char s_HEADING_LZY[];

private:
    std::string m_apzvm;
    std::string m_apzconfig;
    std::string m_dump;
    std::string m_pcihsrv;

    static const std::string& s_APZ_VM; 
    static const std::string& s_APZ_config; 
    static const std::string& s_PES_LZY; 
    static const std::string& s_PCIHSRV;
};

//----------------------------------------------------------------------------------------
// Multi CP system
//----------------------------------------------------------------------------------------

template<>
class Pesconfig<Common::e_multicp> : public PesconfigBase
{
public:
    enum sysType_t {e_cp, e_bc};

    // Constructor
    Pesconfig();

    // Destructor
    ~Pesconfig();

    // Read the PES configuration file
    void read(
            const std::string& path                 // Path to the PES configuration file
            );

    // Write the PES configuration file
    void write(
            const std::string& path                 // Path to the PES configuration file
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

private:
    std::pair<bool, unsigned short> m_cpType;
    std::pair<bool, sysType_t> m_sysType;

    static const std::string& s_CP_TYPE;
    static const std::string& s_SYS_TYPE;
    static const unsigned short s_APZMIN;
    static const unsigned short s_APZMAX;
};

//----------------------------------------------------------------------------------------
// One CP system
//----------------------------------------------------------------------------------------

template<>
class Pesconfig<Common::e_onecp>
{
public:

    // Constructor
    Pesconfig();

    // Destructor
    ~Pesconfig();
    
    PesconfigBase& operator[](                      // Returns base class instance
            char side                               // CP side, 'A' or 'B'
            );

    // Read the PES configuration file
    void read(
            const std::string& path                 // Path to the PES configuration file
            );

    // Write the PES configuration file
    void write(
            const std::string& path                 // Path to the PES configuration file
            );

private:

    int checkSide(                                  // Returns 0 for 'A'-side 
                                                    // and 1 for 'B'-side
            char side                               // CP side, 'A' or 'B'
            ) const;

    PesconfigBase m_configbase[2];
};

#endif
