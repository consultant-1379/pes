//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dumpinfo.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2011. All rights reserved.
//
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class handles the dump information.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 0696  D
//
//  AUTHOR
//      XDT/DEK XBAODOO (Bao Do)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      A      2008-01-15   UABTSO      CFET adapted for a multiple CP environment.
//      B      2008-05-12   UABMHA      TR HI81299 solved (return codes, printouts etc.)
//      C      2008-06-10   UABMHA      PCN 28266 solved (changes for -s option with cfeted -v)
//      D      2008-11-20   UABMHA      TR HJ56210 (HJ81258) solved (syntax printout of cfeted...)
//                                      TR HJ77678 solved (cfeted -d does not report corruption in zipfile)
//                                      Changed behaviour: cfeted -c and -v: RPB-S column only printed for APZ21250
//      E      2009-01-12   UABMHA      TR HJ77678 (cont...) removed faulty line that gave install PES problem
//      R2A    2011-09-21   XBAODOO     AXE12 CR3: Adding the handling of CXP package
//      F      2011-11-11   UABMAGN     New file ported to APG43 Linux
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef DUMPINFO_H
#define DUMPINFO_H

#include "common.h"
#include <string>

template<Common::t_cptype>
class Dumpinfo;

//----------------------------------------------------------------------------------------
// Base class for DUMP information
//----------------------------------------------------------------------------------------

class DumpinfoBase
{
    friend class Dumpinfo<Common::e_multicp>;
    friend class Dumpinfo<Common::e_onecp>;

public:

    // Constructor
    DumpinfoBase();

    // Destructor
    virtual ~DumpinfoBase();

    // Set the CPHW DUMP name
    void setCphwDump(
            const std::string& cphwdump              // CPHW DUMP name
            );

    // Set the PES DUMP name
    void setPesDump(
            const std::string& pesdump               // PES DUMP name
            );

    // Set the APZ configuration name
    void setApzConfig(
            const std::string& apzconfig            // APZ configuration name
            );

private:
    std::string m_apzconfig;
    std::string m_cphwdump;
    std::string m_pesdump;

    static const std::string& s_CPHW_LZY;
    static const std::string& s_PES_LZY;
    static const std::string& s_APZ_config;
};

//----------------------------------------------------------------------------------------
// Multi CP system
//----------------------------------------------------------------------------------------

template<>
class Dumpinfo<Common::e_multicp> : public DumpinfoBase
{
public:

    // Constructor
    Dumpinfo();

    // Destructor
    virtual ~Dumpinfo();

    // Read the DUMP information file
    void read(
            const std::string& path                 // Path to DUMP information file
            );

    // Write the DUMP information file
    void write(
            const std::string& path                 // Path to DUMP information file
            );

private:
};

//----------------------------------------------------------------------------------------
// One CP system
//----------------------------------------------------------------------------------------

template<>
class Dumpinfo<Common::e_onecp>
{
public:
    // Constructor
    Dumpinfo();

    // Destructor
    ~Dumpinfo();

    DumpinfoBase& operator[](                       // Returns base class instance
            char side                               // CP side, 'A' or 'B'
            );

    // Read the DUMP information file
    void read(
            const std::string& path                 // Path to DUMP information file
            );

    // Write the DUMP information file
    void write(
            const std::string& path                 // Path to DUMP information file
            );

private:
    int checkSide(                                  // Returns 0 for 'A'-side
                                                    // and 1 for 'B'-side
            char side                               // CP side, 'A' or 'B'
            ) const;

    DumpinfoBase m_configbase[2];
};

#endif
