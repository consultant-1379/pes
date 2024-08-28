//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dumpinfo.cpp
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
//      R2B    2011-10-20   XBAODOO     HO90552 solved
//      F      2011-11-11   UABMAGN     New file ported to APG43 Linux
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "dumpinfo.h"
#include "parser.h"
#include "exception.h"
#include <fstream>
#include <iomanip>

using namespace std;

//========================================================================================
// Base class for DUMP information
//========================================================================================

const string& DumpinfoBase::s_CPHW_LZY =    "CPHW_LZY";
const string& DumpinfoBase::s_PES_LZY  =    "PES_LZY";
const string& DumpinfoBase::s_APZ_config =  "APZ_config";

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------

DumpinfoBase::DumpinfoBase() :
m_apzconfig(),
m_cphwdump(),
m_pesdump()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------

DumpinfoBase::~DumpinfoBase()
{
}

//----------------------------------------------------------------------------------------
// Set CPHW DUMP
//----------------------------------------------------------------------------------------

void
DumpinfoBase::setCphwDump(const string& cphwdump)
{
    m_cphwdump = cphwdump;
}

//----------------------------------------------------------------------------------------
// Set PES DUMP
//----------------------------------------------------------------------------------------

void
DumpinfoBase::setPesDump(const string& pesdump)
{
    m_pesdump = pesdump;
}

//----------------------------------------------------------------------------------------
// Set APZ configuration
//----------------------------------------------------------------------------------------

void
DumpinfoBase::setApzConfig(const string& apzconfig)
{
    m_apzconfig = apzconfig;
}

//========================================================================================
// Multi CP system
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------

Dumpinfo<Common::e_multicp>::Dumpinfo()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------

Dumpinfo<Common::e_multicp>::~Dumpinfo()
{
}

//----------------------------------------------------------------------------------------
// Read information file
//----------------------------------------------------------------------------------------

void
Dumpinfo<Common::e_multicp>::read(const string& path)
{
    // Open DUMP information file
    ifstream fs(path.c_str(), ios_base::in);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    int lineno;
    string line;

    // Parse file
    lineno = 0;
    while (fs.eof() == false)
    {
        lineno++;
        (void)getline(fs, line);
        Parser parser(line);
        parser.skipSpace();
        if (!parser.eoln() && (parser.peek() != '#'))    // Ignore comment and empty line
        {
            parser.skipSpace();

            // Get tag
            const string& tag = parser.getIdent();
            parser.skipSpace();
            char ch = parser.getChar();
            if (ch != ':')
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Unexpected token '" << ch << "' was found at line "
                   << lineno << " in file '" << path << "'.";
                throw ex;
            }
            parser.skipSpace();

            // Get product path
            const string& item = parser.getString();
            size_t pos = item.find_first_of(Parser::s_ILLFILE);
            if (pos != string::npos)
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Illegal file path was found at line " << lineno << "\nin file '"
                   << path << "'.";
                throw ex;
            }
            parser.skipSpace();

            // Check remaining line
            if ((parser.eoln() == false) && (parser.peek() != '#'))
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Unexpected token '" << parser.getChar()
                   << "' was found at line " << lineno << " in file '" << path << "'.";
                throw ex;
            }

            // Insert item
            if (tag ==s_CPHW_LZY)
            {
                m_cphwdump = item;
            }
            else if (tag == s_PES_LZY)
            {
                m_pesdump = item;
            }
            else if (tag == s_APZ_config)
            {
                m_apzconfig = item;
            }
            else
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Illegal tag '" << tag << "' was found at line "
                   << lineno << " in file '" << path << "'.";
                throw ex;
            }
        }
    }

    // Close file
    (void)fs.close();
}

//----------------------------------------------------------------------------------------
// Write information file
//----------------------------------------------------------------------------------------

void
Dumpinfo<Common::e_multicp>::write(const string& path)
{
    // Open DUMP information file
    ofstream fs(path.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    if (m_cphwdump.empty() == false)
    {
        fs << s_CPHW_LZY << ":" << m_cphwdump << endl;
    }
    if (m_pesdump.empty() == false)
    {
        fs << s_PES_LZY << ":" << m_pesdump << endl;
    }
    if (m_apzconfig.empty() == false)
    {
        fs << s_APZ_config << ":" << m_apzconfig << endl;
    }

    // Close file
    (void)fs.close();
}


//========================================================================================
// One CP system
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------

Dumpinfo<Common::e_onecp>::Dumpinfo() :
m_configbase()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------

Dumpinfo<Common::e_onecp>::~Dumpinfo()
{
}

//----------------------------------------------------------------------------------------
// Check CP side
//----------------------------------------------------------------------------------------

int
Dumpinfo<Common::e_onecp>::checkSide(char side) const
{
    switch (toupper(side))
    {
    case 'A': return 0;
    case 'B': return 1;
    default:  throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
}

//----------------------------------------------------------------------------------------
// Subscript operator
//----------------------------------------------------------------------------------------

DumpinfoBase&
Dumpinfo<Common::e_onecp>::operator[](char side)
{
    return m_configbase[checkSide(side)];    //lint !e1536
}

//----------------------------------------------------------------------------------------
// Read information file
//----------------------------------------------------------------------------------------

void
Dumpinfo<Common::e_onecp>::read(const string& path)
{
    // Open DUMP information file
    ifstream fs(path.c_str(), ios_base::in);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    int lineno;
    string line;

    // Parse file
    lineno = 0;
    while (fs.eof() == false)
    {
        lineno++;
        (void)getline(fs, line);
        Parser parser(line);
        parser.skipSpace();
        if (!parser.eoln() && (parser.peek() != '#'))    // Ignore comment and empty line
        {
            char ch;
            parser.skipSpace();

            // CP side
            char side = parser.getChar();
            int index = checkSide(side);

            parser.skipSpace();
            ch = parser.getChar();
            if (ch != ':')
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Unexpected token '" << ch << "' was found at line "
                << lineno << " in file '" << path << "'.";
                throw ex;
            }

            // Get tag
            const string& tag = parser.getIdent();
            parser.skipSpace();
            ch = parser.getChar();
            if (ch != ':')
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Unexpected token '" << ch << "' was found at line "
                   << lineno << " in file '" << path << "'.";
                throw ex;
            }
            parser.skipSpace();

            // Get product path
            const string& item = parser.getString();
            size_t pos = item.find_first_of(Parser::s_ILLFILE);
            if (pos != string::npos)
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Illegal file path was found at line " << lineno << "\nin file '"
                   << path << "'.";
                throw ex;
            }
            parser.skipSpace();

            // Check remaining line
            if ((parser.eoln() == false) && (parser.peek() != '#'))
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Unexpected token '" << parser.getChar()
                   << "' was found at line " << lineno << " in file '" << path << "'.";
                throw ex;
            }

            // Insert item
            if (tag == DumpinfoBase::s_CPHW_LZY)
            {
                m_configbase[index].m_cphwdump = item;
            }
            else if (tag == DumpinfoBase::s_PES_LZY)
            {
                m_configbase[index].m_pesdump = item;
            }
            else if (tag == DumpinfoBase::s_APZ_config)
            {
                m_configbase[index].m_apzconfig = item;
            }
            else
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Illegal tag '" << tag << "' was found at line "
                   << lineno << " in file '" << path << "'.";
                throw ex;
            }
        }
    }

    // Close file
    (void)fs.close();
}

//----------------------------------------------------------------------------------------
// Write information file
//----------------------------------------------------------------------------------------

void
Dumpinfo<Common::e_onecp>::write(const string& path)
{
    // Open DUMP information file
    ofstream fs(path.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    if (m_configbase[0].m_cphwdump.empty() == false)
    {
        fs << "A:" << DumpinfoBase::s_CPHW_LZY << ":" << m_configbase[0].m_cphwdump << endl;
    }
    if (m_configbase[1].m_cphwdump.empty() == false)
    {
        fs << "B:" << DumpinfoBase::s_CPHW_LZY << ":" << m_configbase[1].m_cphwdump << endl;
    }
    if (m_configbase[0].m_pesdump.empty() == false)
    {
        fs << "A:" << DumpinfoBase::s_PES_LZY << ":" << m_configbase[0].m_pesdump << endl;
    }
    if (m_configbase[1].m_pesdump.empty() == false)
    {
        fs << "B:" << DumpinfoBase::s_PES_LZY << ":" << m_configbase[1].m_pesdump << endl;
    }
    if (m_configbase[0].m_apzconfig.empty() == false)
    {
        fs << "A:" << DumpinfoBase::s_APZ_config << ":" << m_configbase[0].m_apzconfig
           << endl;
    }
    if (m_configbase[1].m_apzconfig.empty() == false)
    {
        fs << "B:" << DumpinfoBase::s_APZ_config << ":" << m_configbase[1].m_apzconfig
           << endl;
    }

    // Close file
    (void)fs.close();
}
