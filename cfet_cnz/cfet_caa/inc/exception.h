//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      exception.h
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
//      This class is used for error handling.
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
//             2010-11-09   XCSVERE     New Fault code 20 has been added
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>
#include <iostream>
#include <sstream>
#include <stdint.h>

#define IGNORE_EXCEPTION(func) try {func;} catch (Exception&) {}

//----------------------------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------------------------

class Exception
{
    // Stream operator method.
    friend std::ostream& operator<<(                // Returns the output stream
            std::ostream& s,                        // Output stream
            const Exception& ex                     // Exception instance
            );

public:
    // The first three error codes are defined for the purpose of 
    // being compatible with "Design Rule for AP Commands".
    
   enum t_errorcode {
      // Standard return codes for AP commands
      e_ok_0 =                          0, // OK
      e_general_1 =                     1, // Error when executing (general fault)
      e_usage_2 =                       2, // Usage error
      e_alreadyexec_3 =                 3, // Command is already executing.
      e_cpboardnotfound_4 =             4, // CP board not found.
      e_extrainputonline_5 =            5, // Extra input on line.
      e_noinputonline_6 =               6, // No input on line.
      e_illegalcpside_7 =               7, // Illegal CP side.
      e_illegaldumptype_8 =             8, // Illegal dump type.
      e_illegaldumpversion_9 =          9, // Illegal dump version.
      e_illegalnumber_10 =              10, // Illegal number.
      e_incorr_argfor_i_int_11 =        11, // Incorrect argument for option '-i'
                                            // - Integer was expected.
      e_incorr_argfor_i_ovf_12 =        12, // Incorrect argument for option '-i'
                                            // - Overflow occurred.
      e_packagefilenotfound_13 =        13, // Package file <package> was not found.
      e_nobackup_cphw_conf_14 =         14, // There is no backup CPHW configuration.
      e_nobackup_pes_conf_15 =          15, // There is no backup PES configuration.
      e_noconf_cp_or_side_16 =          16, // There is no configuration for this
                                            // CP or CP side.
      e_nonew_cphw_conf_17 =            17, // There is no new CPHW configuration
      e_nonew_pes_conf_18 =             18, // There is no new PES configuration
      e_unexp_input_chk_19 =            19, // Unexpected input
      e_notonactivenode_20 =            20, // Command must be executed on the active node
      e_WrongDump_21 =                  21, // Wrong Dump given
      e_WrongDumpPath_22 =              22, // Dump is not in the correct path
      e_parameter_26 =                  26, // Parameter error
      e_application_27 =                27, // Application error
      e_system_28 =                     28, // System error
      e_internal_29 =                   29, // Internal error
      e_abort_30 =                      30, // Aborted by user
      e_notimpl_31 =                    31, // Not implemented
      e_no_maus_conf_32 =               32, // There is no MAUS configuration file.
      e_maus_error_33 =                 33, // MAUS errors
      e_illcmd_sysconf_115 =            115, // Illegal command in this system conf.
      e_illopt_sysconf_116 =            116, // Illegal option usage in this system conf.
      e_cp_not_defined_118 =            118  // CP is not defined.
   };
    
    // Constructor
    Exception(
            t_errorcode errcode                     // Error code
            );
    
    // Constructor
    Exception(
            t_errorcode errcode,                    // Error code
            uint16_t lResult                        // Windows result code
            );

    // Constructor
    Exception(
            t_errorcode errcode,                    // Error code
            const std::string& detail,              // Optional textual information
            uint16_t lResult = 0                    // Windows result code
            );

    // Copy constructor
    Exception(
            const Exception& ex                     // Exception object
            );
    
    // Destructor
    ~Exception();
    
    // Assignment operator.
    Exception& operator=(                           // Returns reference to exception object
            const Exception& ex                     // Exception object
            );
    
    // Read the error code
    t_errorcode errorCode() const;                  // Returns the error code

    // Read the Windows result code
    uint16_t resultCode() const;                    // Returns the Windows result code

    // Read the error text.
    const std::string errorText() const;            // Returns the error text string

    // Read the detailed information
    const std::string detailInfo() const;           // Returns optional textual information
    
    // Stream detailed information to the Exception object
    template<typename T>
    std::stringstream& operator<<(                  // Returns detailed error information.
            const T& t                              // Info to stream.
            )
    {
        m_s << t;
        return m_s;
    }
    
private:

    // Translate a windows error code to an error text
    std::string strError(                           // Returns error text
                long lResult                        // Windows result code
                ) const;

    static const char s_OKAY[]; 
    static const char s_GENERAL[]; 
    static const char s_USAGE[];
    static const char s_PARAMETER[];
    static const char s_APPLICATION[];
    static const char s_SYSTEM[];
    static const char s_INTERNAL[];
    static const char s_ABORT[];
    static const char s_NOTIMPL[];
    static const char s_ILLCMDSYSCONF[];
    static const char s_ILLOPTSYSCONF[];
    static const char s_CPNOTDEFINED[];
    static const char s_NOTONACTIVE[];
    static const char s_WrongDump[];
    static const char s_WrongDumpPath[];
    static const char s_MAUS[];

    t_errorcode m_error;                            // Error code
    uint16_t m_lResult;                             // Windows result code
    std::stringstream m_s;                          // Error stream
};

#endif
