//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      pesconfig.cpp
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

#include <pesconfig.h>
#include <parser.h>
#include <exception.h>
#include <fstream>
#include <iomanip>

using namespace std;

//========================================================================================
// Base class for CPHW configuration
//========================================================================================

const string& PesconfigBase::s_APZ_VM =         "APZ_VM";
const string& PesconfigBase::s_APZ_config =     "APZ_config";
const string& PesconfigBase::s_PES_LZY =        "PES_LZY";
const string& PesconfigBase::s_PCIHSRV =        "PCIHSRV";

const char PesconfigBase::s_HEADING[] =         "PRODUCT                 PACKAGE INFO";
const char PesconfigBase::s_HEADING_LZY[] =     "PRODUCT                 LZY INFO";

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
PesconfigBase::PesconfigBase() :
m_apzvm(),
m_apzconfig(),
m_dump(),
m_pcihsrv()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
PesconfigBase::~PesconfigBase()
{
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool 
PesconfigBase::operator==(const PesconfigBase& conf) const
{
    return m_apzvm == conf.m_apzvm && 
           m_apzconfig == conf.m_apzconfig &&
           m_dump == conf.m_dump &&
           m_pcihsrv == conf.m_pcihsrv;
}

//----------------------------------------------------------------------------------------
// Get APZ_VM
//----------------------------------------------------------------------------------------
PesconfigBase::PAIR 
PesconfigBase::getApzVm() const 
{
    size_t pos = m_apzvm.find_last_of("/\\");
    return make_pair(m_apzvm.substr(0, pos), m_apzvm.substr(pos + 1));
}

//----------------------------------------------------------------------------------------
// Get PCIHSRV
//----------------------------------------------------------------------------------------

PesconfigBase::PAIR
PesconfigBase::getPcihSrv() const
{
    size_t pos = m_pcihsrv.find_last_of("/\\");
    return make_pair(m_pcihsrv.substr(0, pos), m_pcihsrv.substr(pos + 1));
}

//----------------------------------------------------------------------------------------
// Get APZ configuration
//----------------------------------------------------------------------------------------
PesconfigBase::PAIR  
PesconfigBase::getApzConfig() const 
{
    size_t pos = m_apzconfig.find_last_of("/\\");
    return make_pair(m_apzconfig.substr(0, pos), m_apzconfig.substr(pos + 1));
}

//----------------------------------------------------------------------------------------
// Get PES DUMP
//----------------------------------------------------------------------------------------
string 
PesconfigBase::getDump() const
{
    return m_dump;
}

//----------------------------------------------------------------------------------------
// Set APZ_VM
//----------------------------------------------------------------------------------------
void
PesconfigBase::setApzVm(const PesconfigBase::PAIR& apzvm) 
{
    m_apzvm = apzvm.first + "/" + apzvm.second;    // path to be removed? 
}

//----------------------------------------------------------------------------------------
// Set APZ configuration
//----------------------------------------------------------------------------------------
void
PesconfigBase::setApzConfig(const PesconfigBase::PAIR& apzconfig) 
{
    m_apzconfig = apzconfig.first + "/" + apzconfig.second;    // path to be removed? 
}

//----------------------------------------------------------------------------------------
// Set PES DUMP
//----------------------------------------------------------------------------------------
void 
PesconfigBase::setDump(const string& dump)
{
    m_dump = dump;
}

//----------------------------------------------------------------------------------------
// Set PCIHSRV
//----------------------------------------------------------------------------------------
void 
PesconfigBase::setPcihSrv(const string& pcihsrv) 
{
    m_pcihsrv = pcihsrv;
}

//========================================================================================
// Multi CP system
//========================================================================================

const string& Pesconfig<Common::e_multicp>::s_CP_TYPE =         "CP_TYPE";
const string& Pesconfig<Common::e_multicp>::s_SYS_TYPE =        "SYS_TYPE";
const unsigned short Pesconfig<Common::e_multicp>::s_APZMIN =   21200;
const unsigned short Pesconfig<Common::e_multicp>::s_APZMAX =   21299;

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Pesconfig<Common::e_multicp>::Pesconfig()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Pesconfig<Common::e_multicp>::~Pesconfig()
{
}

//----------------------------------------------------------------------------------------
// Read configuration file
//----------------------------------------------------------------------------------------
void
Pesconfig<Common::e_multicp>::read(const string& path)
{
    // Open PES configuration file
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

            // Get item
            if (tag == s_APZ_VM)
            {
                const string& item = parser.getString();
                size_t pos = item.find_first_of(Parser::s_ILLFILE);
                if (pos != string::npos)
                {
                    Exception ex(Exception::e_parameter_26);
                    ex << "Illegal file path was found at line " << lineno 
                       << "\nin file '" << path << "'.";
                    throw ex;
                }
                m_apzvm = item;
            }
            else if (tag == s_APZ_config)
            {
                const string& item = parser.getString();
                size_t pos = item.find_first_of(Parser::s_ILLFILE);
                if (pos != string::npos)
                {
                    Exception ex(Exception::e_parameter_26);
                    ex << "Illegal file path was found at line " << lineno 
                       << "\nin file '" << path << "'.";
                    throw ex;
                }
                m_apzconfig = item;
            }
            else if (tag == s_PES_LZY)
            {
                m_dump = parser.getString();
            }
            else if (tag == s_CP_TYPE)
            {
                int num;
                try
                {
                    num = (parser.getNumeral());
                }
                catch (Exception& ex)
                {
                    ex << " at line " << lineno << " in file '" << path << "'.";
                    throw ex;
                }    
                if (num >= s_APZMIN && num <= s_APZMAX)
                {
                    m_cpType = make_pair(true, static_cast<unsigned short>(num));
                }
                else
                {
                    Exception ex(Exception::e_parameter_26);
                    ex << "Illegal CP type was found at line " << lineno 
                       << " in file '" << path << "'.";
                    throw ex;
                }    
            }
            else if (tag == s_SYS_TYPE)
            {
                const string& sysType = Common::toUpper(parser.getString());
                if (sysType == "CP")
                {
                    m_sysType = make_pair(true, e_cp);
                }
                else if (sysType == "BC")
                {
                    m_sysType = make_pair(true, e_bc);
                }
                else
                {
                    Exception ex(Exception::e_parameter_26);
                    ex << "Illegal system type was found at line " << lineno 
                       << " in file '" << path << "'.";
                    throw ex;
                }    
            }
            else
            {
                Exception ex(Exception::e_parameter_26);
                ex << "Illegal tag '" << tag << "' was found at line " 
                   << lineno << " in file '" << path << "'.";
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
        }
    }

    // Close file
    (void)fs.close();
}

//----------------------------------------------------------------------------------------
// Write configuration file
//----------------------------------------------------------------------------------------
void
Pesconfig<Common::e_multicp>::write(const string& path)
{
    // Open PES configuration file
    ofstream fs(path.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    if (m_apzvm.empty() == false)
    {
        fs << s_APZ_VM << ":" << m_apzvm << endl;
    }
    if (m_apzconfig.empty() == false)
    {
        fs << s_APZ_config << ":" << m_apzconfig << endl;
    }
    if (m_dump.empty() == false)
    {
        fs << s_PES_LZY << ":" << m_dump << endl;
    }
    if (m_cpType.first == true)
    {
        fs << s_CP_TYPE << ":" << m_cpType.second << endl;
    }
    if (m_sysType.first == true)
    {
        fs << s_SYS_TYPE << ":";
        switch (m_sysType.second)
        {
        case e_cp:    fs << "CP"; break;
        case e_bc:    fs << "BC"; break;
        default:    throw Exception(Exception::e_internal_29, "Illegal system type.");
        }
    }

    // Close file
    (void)fs.close();
}

//----------------------------------------------------------------------------------------
// Get the CP type
//----------------------------------------------------------------------------------------
unsigned short 
Pesconfig<Common::e_multicp>::getCPType() const
{
    if (m_cpType.first == true)
    {
        return m_cpType.second;
    }
    else
    {
        Exception ex(Exception::e_internal_29);
        ex << "CP type is undefined.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Get the system type
//----------------------------------------------------------------------------------------
Pesconfig<Common::e_multicp>::sysType_t 
Pesconfig<Common::e_multicp>::getSysType() const
{
    if (m_sysType.first == true)
    {
        return m_sysType.second;
    }
    else
    {
        Exception ex(Exception::e_internal_29);
        ex << "System type is undefined.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Set the CP type
//----------------------------------------------------------------------------------------
void 
Pesconfig<Common::e_multicp>::setCpType(unsigned short cpType)
{
    m_cpType = make_pair(true, cpType);
}

//----------------------------------------------------------------------------------------
// Set the system type
//----------------------------------------------------------------------------------------
void 
Pesconfig<Common::e_multicp>::setSysType(sysType_t sysType)
{
    m_cpType = make_pair(true, sysType);
}


//========================================================================================
// One CP system
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Pesconfig<Common::e_onecp>::Pesconfig() :
m_configbase()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Pesconfig<Common::e_onecp>::~Pesconfig()
{
}

//----------------------------------------------------------------------------------------
// Check CP side
//----------------------------------------------------------------------------------------
int 
Pesconfig<Common::e_onecp>::checkSide(char side) const
{
    switch (toupper(side))
    {
    case 'A': return Exception::e_ok_0;
    case 'B': return 1;
    default:  throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
}

//----------------------------------------------------------------------------------------
// Subscript operator
//----------------------------------------------------------------------------------------
PesconfigBase&
Pesconfig<Common::e_onecp>::operator[](char side) 
{
    return m_configbase[checkSide(side)];    //lint !e1536    
}

//----------------------------------------------------------------------------------------
// Read configuration file
//----------------------------------------------------------------------------------------
void
Pesconfig<Common::e_onecp>::read(const string& path)
{
    // Open PES configuration file
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
                   << "' was found at line " << lineno << "\nin file '" << path 
                   << "'.";
                throw ex;
            }

            // Insert item
            if (tag == PesconfigBase::s_APZ_VM)
            {
                m_configbase[index].m_apzvm = item;
            }
            else if (tag == PesconfigBase::s_APZ_config)
            {
                m_configbase[index].m_apzconfig = item;
            }
            else if (tag == PesconfigBase::s_PES_LZY)
            {
                m_configbase[index].m_dump = item;
            }
            else if (tag == PesconfigBase::s_PCIHSRV)
            {
                m_configbase[index].m_pcihsrv = item;
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
// Write configuration file
//----------------------------------------------------------------------------------------
void
Pesconfig<Common::e_onecp>::write(const string& path)
{
    // Open PES configuration file
    ofstream fs(path.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    if (m_configbase[0].m_apzvm.empty() == false)
    {
        fs << "A:" << PesconfigBase::s_APZ_VM << ":" << m_configbase[0].m_apzvm << endl;
    }
    if (m_configbase[1].m_apzvm.empty() == false)
    {
        fs << "B:" << PesconfigBase::s_APZ_VM << ":" << m_configbase[1].m_apzvm << endl;
    }
    if (m_configbase[0].m_apzconfig.empty() == false)
    {
        fs << "A:" << PesconfigBase::s_APZ_config << ":" << m_configbase[0].m_apzconfig 
           << endl;
    }
    if (m_configbase[1].m_apzconfig.empty() == false)
    {
        fs << "B:" << PesconfigBase::s_APZ_config << ":" << m_configbase[1].m_apzconfig 
           << endl;
    }
    if (m_configbase[0].m_pcihsrv.empty() == false)
    {
        fs << "A:" << PesconfigBase::s_PCIHSRV << ":" << m_configbase[0].m_pcihsrv << endl;
    }
    if (m_configbase[1].m_pcihsrv.empty() == false)
    {
        fs << "B:" << PesconfigBase::s_PCIHSRV << ":" << m_configbase[1].m_pcihsrv << endl;
    }
    if (m_configbase[0].m_dump.empty() == false)
    {
        fs << "A:" << PesconfigBase::s_PES_LZY << ":" << m_configbase[0].m_dump << endl;
    }
    if (m_configbase[1].m_dump.empty() == false)
    {
        fs << "B:" << PesconfigBase::s_PES_LZY << ":" << m_configbase[1].m_dump << endl;
    }

    // Close file
    (void)fs.close();
}
