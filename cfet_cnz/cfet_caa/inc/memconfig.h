//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      memconfig.h
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
//      This class handles the memory configuration files.
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

#ifndef MEMCONFIG_H
#define MEMCONFIG_H

#include "parser.h"
#include <string>

class Memconfig
{
    // Outstream operator
    friend std::ostream& operator<<(                // Returns the output stream instance
            std::ostream& s,                        // Output stream instance
            const Memconfig& conf                   // Memconfig instance
            );

public:

    // Constructor
    Memconfig();

    // Destructor
    ~Memconfig();

    // Equality operator
    bool operator==(                                // Returns true if equal, false otherwise
            const Memconfig& conf                   // Memconfig instance
            ) const;

    // Get the value of the serial RP bus flag (m_rpbs)
    bool getRpbsFlag(void);

    // Read the memory configuration file
    void read(
            const std::string& path,                // Path to memory configuration file
            bool legacy = false                     // True if legacy file format (APZ21240)
            );

    // Check if the memconfig is empty
    bool isEmpty() const;
 
    //Get the value of reserved flag (m_resrv) in the CDA file
    bool getReservedFlag();
    // Get the reserved value from CDA file
    int readReservedValue();

    static const std::string& s_CONFFILE;
    static const char s_HEADING[];                  // Heading for APZ21250 (has RPB-S)
    static const char s_HEADING_2[];                 // Heading for other machines
    static const char s_HEADING_3[];			// Heading for handling spare part (has RESERVED)

private:

    // Parse the memory size
    unsigned int parseSize(                         // Memory size (in MW)
            const std::string& keyword,             // Memory type, "-ds", "-ps" or "-rs"
            Parser& parser                          // Parser instance
            ) const;

    unsigned int m_ds;
    unsigned int m_ps;
    unsigned int m_rs;
    unsigned int m_rsv;			// memory size (in MW), for spare part handling of GEP5-48 with GEp7
    bool m_rpbs;
    bool m_empty;
    bool m_resrv;
};

#endif
