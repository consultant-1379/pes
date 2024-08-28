//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cphwconfig.cpp
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

#include <cphwconfig.h>
#include <parser.h>
#include <exception.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace fs = boost::filesystem;
using namespace std;

const char CphwconfigBase::s_HEADING[] = "PACKAGE INFO";
const char CphwconfigBase::s_HEADING_LZY[] = "LZY INFO";

//========================================================================================
// Base class for CPHW configuration
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
CphwconfigBase::CphwconfigBase() :
m_dump()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
CphwconfigBase::~CphwconfigBase()
{
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool 
CphwconfigBase::operator==(const CphwconfigBase& conf) const
{
    return m_dump == conf.m_dump;
}

//----------------------------------------------------------------------------------------
// Get CPHW_DUMP
//----------------------------------------------------------------------------------------
CphwconfigBase::PAIR 
CphwconfigBase::getDump() const
{
    size_t pos = m_dump.find_last_of("/\\");
    if (pos != string::npos)
    {
        return make_pair(m_dump.substr(0, pos), m_dump.substr(pos + 1));
    }
    else
    {
        return make_pair("", m_dump);
    }
}

//----------------------------------------------------------------------------------------
// Set CPHW_DUMP path
//----------------------------------------------------------------------------------------
void
CphwconfigBase::setDump(const CphwconfigBase::PAIR& dump)
{
    m_dump = dump.first + "/" + dump.second;    // path to be removed?
}

//========================================================================================
// Multi CP system
//========================================================================================

const string& Cphwconfig<Common::e_multicp>::s_CPHW_LZY =        "CPHW_LZY";
const string& Cphwconfig<Common::e_multicp>::s_CP_TYPE =         "CP_TYPE";
const string& Cphwconfig<Common::e_multicp>::s_SYS_TYPE =        "SYS_TYPE";
const string& Cphwconfig<Common::e_multicp>::s_CP_MAU =        "CP_MAU";
const unsigned short Cphwconfig<Common::e_multicp>::s_APZMIN =    21200;
const unsigned short Cphwconfig<Common::e_multicp>::s_APZMAX =    21299;

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Cphwconfig<Common::e_multicp>::Cphwconfig() :
CphwconfigBase(),
m_cpType(),
m_sysType(false, sysType_t())
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Cphwconfig<Common::e_multicp>::~Cphwconfig()
{
}

//----------------------------------------------------------------------------------------
// Read configuration file
//----------------------------------------------------------------------------------------
void
Cphwconfig<Common::e_multicp>::read(const string& path)
{
    // Open CPHW configuration file
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
            if (tag == s_CPHW_LZY)
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
                m_dump = item;
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
                m_cpType = make_pair(true,  static_cast<unsigned short>(num));
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
            else if (tag == s_CP_MAU)
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
			    m_cpMauVersionMCP = item.substr(pos + 1);
			}
            else
            {
                Exception ex(Exception::e_parameter_26);
                cerr << endl;
                ex  << "Illegal tag '" << tag << "' was found at line " 
                    << lineno << " in file '" << path << "'.";
                throw ex;
            }
            parser.skipSpace();
    
            // Check remaining line
            if ((parser.eoln() == false) && (parser.peek() != '#'))
            {
                Exception ex(Exception::e_parameter_26);
                cerr << endl;
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
Cphwconfig<Common::e_multicp>::write(const string& path)
{
    // Open CPHW configuration file
    ofstream fs(path.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }
    
    if (m_dump.empty() == false)
    {
        fs << s_CPHW_LZY << ":" << m_dump << endl;
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
        case e_cp:    fs << "CP" << endl; break;
        case e_bc:    fs << "BC" << endl; break;
        default:    throw Exception(Exception::e_internal_29, "Illegal system type.");
        }
    }
    if (!m_cpMauVersionMCP.empty())
    {
        fs << s_CP_MAU << ":" << m_cpMauVersionMCP << endl;
    }

    
    // Close file
    (void)fs.close();
}

//----------------------------------------------------------------------------------------
// Get the CP type
//----------------------------------------------------------------------------------------
unsigned short 
Cphwconfig<Common::e_multicp>::getCPType() const
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
Cphwconfig<Common::e_multicp>::sysType_t 
Cphwconfig<Common::e_multicp>::getSysType() const
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
Cphwconfig<Common::e_multicp>::setCpType(unsigned short cpType)
{
    m_cpType = make_pair(true, cpType);
}

//----------------------------------------------------------------------------------------
// Set the system type
//----------------------------------------------------------------------------------------
void 
Cphwconfig<Common::e_multicp>::setSysType(sysType_t sysType)
{
    m_cpType = make_pair(true, sysType);
}

//========================================================================================
// One CP system
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Cphwconfig<Common::e_onecp>::Cphwconfig() :
m_configbase()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Cphwconfig<Common::e_onecp>::~Cphwconfig()
{
}

//----------------------------------------------------------------------------------------
// Check CP side
//----------------------------------------------------------------------------------------
int 
Cphwconfig<Common::e_onecp>::checkSide(char side) const
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
CphwconfigBase& 
Cphwconfig<Common::e_onecp>::operator[](char side)
{
    return m_configbase[checkSide(side)];    //lint !e1536    
}

//----------------------------------------------------------------------------------------
// Read configuration file
//----------------------------------------------------------------------------------------
void
Cphwconfig<Common::e_onecp>::read(const string& path)
{
    // Open CPHW configuration file
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

            if (ch == ':')
            {
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
              m_configbase[index].m_dump = item;
           }
           else if(ch=='_')
           {
           	// Get product path
               const string& item = parser.getString();
               size_t pos = item.find_first_of(Parser::s_ILLFILE);
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
               m_cpMauCurVersion[index] = item.substr(pos + 1);
           }
           else
           {
               Exception ex(Exception::e_parameter_26);
               ex << "Unexpected token '" << ch << "' was found at line "
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
Cphwconfig<Common::e_onecp>::write(const string& path)
{
    // Open CPHW configuration file
    ofstream fs(path.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }
    if (m_configbase[0].m_dump.empty() == false)
    {
        fs << "A:" << m_configbase[0].m_dump << endl;
    }
    if (m_configbase[1].m_dump.empty() == false)
    {
        fs << "B:" << m_configbase[1].m_dump << endl;
    }
    if (!m_cpMauCurVersion[0].empty())
    {
        fs << "A_CPMAU:" << m_cpMauCurVersion[0] << endl;
    }
    if (!m_cpMauCurVersion[1].empty())
    {
        fs << "B_CPMAU:" << m_cpMauCurVersion[1] << endl;
    }

    
    // Close file
    (void)fs.close();
}
