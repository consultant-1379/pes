//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      exception.cpp
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
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <exception.h>
#include <string.h>

// Fault messages
const char Exception::s_OKAY[] =                "OK"; 
const char Exception::s_GENERAL[] =             "Error when executing (general fault): "; 
const char Exception::s_USAGE[] =               ""; //"Usage: "; 

const char Exception::s_PARAMETER[] =           "Parameter error: ";
const char Exception::s_APPLICATION[] =         "Application error: ";
const char Exception::s_SYSTEM[] =              "System error: ";
const char Exception::s_INTERNAL[] =            "Internal error: ";
const char Exception::s_ABORT[] =               "Aborted";
const char Exception::s_NOTIMPL[] =             "Not yet implemented";
const char Exception::s_ILLCMDSYSCONF[] =       "Illegal command in this system configuration:\n";
const char Exception::s_ILLOPTSYSCONF[] =       "Illegal option usage in this system configuration:\n";
const char Exception::s_CPNOTDEFINED[] =        "CP is not defined:\n";
const char Exception::s_NOTONACTIVE[] =         "Command must be executed on the active node.";
const char Exception::s_WrongDump[] =           "Wrong Dump: ";
const char Exception::s_WrongDumpPath[] =       "Incorrect dump path:";
const char Exception::s_MAUS[] =                "MAUS error: ";


using namespace std;

//----------------------------------------------------------------------------------------
// Constructor     
//----------------------------------------------------------------------------------------
Exception::Exception(t_errorcode error) :
m_error(error),
m_lResult(0),
m_s()
{
}

//----------------------------------------------------------------------------------------
// Constructor     
//----------------------------------------------------------------------------------------
Exception::Exception(t_errorcode error, uint16_t lResult) :
m_error(error),
m_lResult(lResult),
m_s()
{
}

//----------------------------------------------------------------------------------------
// Constructor     
//----------------------------------------------------------------------------------------
Exception::Exception(t_errorcode error, const string& detail, uint16_t lResult) :
m_error(error),
m_lResult(lResult),
m_s()
{
    m_s << detail;
}

//----------------------------------------------------------------------------------------
// Copy constructor     
//----------------------------------------------------------------------------------------
Exception::Exception(const Exception& ex) :
m_error(ex.m_error),
m_lResult(ex.m_lResult),
m_s()
{  
       m_s << ex.m_s.str();   
}

//----------------------------------------------------------------------------------------
// Destructor    
//----------------------------------------------------------------------------------------
Exception::~Exception()
{
}

//----------------------------------------------------------------------------------------
// Assignment operator.
//----------------------------------------------------------------------------------------
Exception& 
Exception::operator=(const Exception& ex) 
{
    if (this != &ex)
    {
        m_error = ex.m_error;
        m_lResult = ex.m_lResult;
        m_s << ex.m_s.str();
    }
    return *this;
}

//----------------------------------------------------------------------------------------
// Get error code    
//----------------------------------------------------------------------------------------
Exception::t_errorcode
Exception::errorCode() const
{
    return m_error;
}

//----------------------------------------------------------------------------------------
// Get Windows result code    
//----------------------------------------------------------------------------------------
uint16_t
Exception::resultCode() const
{
    return m_lResult;
}

//----------------------------------------------------------------------------------------
// Get error text    
//----------------------------------------------------------------------------------------
const string 
Exception::errorText() const
{
    string text;
    switch (m_error)
    {
    //case e_ok_0:                  text = s_OKAY;          break;
    case e_general_1:               text = s_GENERAL;       break;
    case e_usage_2:                 text = s_USAGE;         break;
    case e_alreadyexec_3:           text = "";              break;
    case e_cpboardnotfound_4:       text = "";              break;
    case e_extrainputonline_5:      text = "";              break;
    case e_noinputonline_6:         text = "";              break;
    case e_illegalcpside_7:         text = "";              break;
    case e_illegaldumptype_8:       text = "";              break;
    case e_illegaldumpversion_9:    text = "";              break;
    case e_illegalnumber_10:        text = "";              break;
    case e_incorr_argfor_i_int_11:  text = "";              break;
    case e_incorr_argfor_i_ovf_12:  text = "";              break;
    case e_packagefilenotfound_13:  text = "";              break;
    case e_nobackup_cphw_conf_14:   text = "";              break;
    case e_nobackup_pes_conf_15:    text = "";              break;
    case e_noconf_cp_or_side_16:    text = "";              break;
    case e_nonew_cphw_conf_17:      text = "";              break;
    case e_nonew_pes_conf_18:       text = "";              break;
    case e_unexp_input_chk_19:      text = "";              break;
    case e_notonactivenode_20:      text = s_NOTONACTIVE;   break;
    case e_WrongDump_21:            text = s_WrongDump;     break;
    case e_WrongDumpPath_22:        text = s_WrongDumpPath; break;
    case e_parameter_26:            text = s_PARAMETER;     break;
    case e_application_27:          text = s_APPLICATION;   break;
    case e_system_28:               text = s_SYSTEM;        break;
    case e_internal_29:             text = s_INTERNAL;      break;
    case e_abort_30:                text = s_ABORT;         break;
    case e_notimpl_31:              text = s_NOTIMPL;       break;
    case e_no_maus_conf_32:         text = "";              break;
    case e_maus_error_33:           text = s_MAUS;          break;
    case e_illcmd_sysconf_115:      text = s_ILLCMDSYSCONF; break;
    case e_illopt_sysconf_116:      text = s_ILLOPTSYSCONF; break;
    case e_cp_not_defined_118:      text = s_CPNOTDEFINED;  break;
    default:                        text = "***";
    }
    return text;
}

//----------------------------------------------------------------------------------------
// Get detailed info about the error  
//----------------------------------------------------------------------------------------
const string
Exception::detailInfo() const
{
    return m_s.str();
}

//----------------------------------------------------------------------------------------
// Windows error message
//----------------------------------------------------------------------------------------
string
Exception::strError(long lResult) const
{
return strerror(lResult);
} 

//----------------------------------------------------------------------------------------
// Send error message to the output stream    
//----------------------------------------------------------------------------------------
ostream& 
operator<<(ostream& s, const Exception& ex)
{ 
    s.clear();
    s << ex.errorText();   

    string str = ex.detailInfo();
    if (str.empty() == false)
    {
        // s << ": " << str; // xxx uabmha, fix
        s << str;
    }
    if (ex.m_lResult)
    {
        s << endl;
        s << ex.strError(ex.m_lResult);
    } 
    return s;
}

