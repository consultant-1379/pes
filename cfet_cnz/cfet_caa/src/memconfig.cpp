//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      memconfig.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2010, 2014. All rights reserved.
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
//      B      2011-11-11   UABMAGN     Implemented CXP package handling.
//      C      2014-01-14   XLOBUNG     Fix for CLH hang when all lines in configdb.ini 
//                                      are commented out.
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <memconfig.h>
#include <exception.h>
#include <hwcinfo.h> // uabmha: Added for RPB-S column handling
#include <fstream>
#include <iomanip>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

const string& Memconfig::s_CONFFILE = "CDA_102";

// Heading  for machines with no RPB-S :
const char Memconfig::s_HEADING[] = "PS(MW)   RS(MW)   DS(MW)";
// Additional heading part for APZ21250 (has RPB-S) (3 spaces + 5 char's longer):
const char Memconfig::s_HEADING_2[] = "   RPB-S";
//Added RESERVED(GB) to heading for blade spare part (3 spaces + 12  chars long)
const char Memconfig::s_HEADING_3[] = "   RESERVED(MB)";



//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Memconfig::Memconfig() :
m_ds(),
m_ps(),
m_rs(),
m_rsv(),
m_resrv(false),
m_rpbs(false),
m_empty(true)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Memconfig::~Memconfig()
{
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool 
Memconfig::operator==(const Memconfig& conf) const
{
    if (m_empty)
    {
       return false;
    }
   
    return  m_ds == conf.m_ds &&
            m_ps == conf.m_ps &&
            m_rs == conf.m_rs &&
            m_rsv == conf.m_rsv &&
            m_resrv == conf.m_resrv &&
            m_rpbs == conf.m_rpbs;
}

//----------------------------------------------------------------------------------------
// Parse store size
//----------------------------------------------------------------------------------------
bool
Memconfig::getRpbsFlag(void)
{
   return m_rpbs;
}

//----------------------------------------------------------------------------------------
// check if "reserved" flag exists in the CDA file
//----------------------------------------------------------------------------------------
bool
Memconfig::getReservedFlag()
{
   return m_resrv;
}
//----------------------------------------------------------------------------------------
// Get the reserved parameter value in the CDA file
//----------------------------------------------------------------------------------------
int
Memconfig::readReservedValue()
{
   return m_rsv;
}

//----------------------------------------------------------------------------------------
// Parse store size
//----------------------------------------------------------------------------------------
unsigned int
Memconfig::parseSize(const string& keyword, Parser& parser) const
{
    if (m_empty)
    {
       return 0;
    }
    
    parser.skipSpace();
    unsigned int size=0;
    if (parser.findToken(keyword))
    {
        parser.skipSpace();
        size = (unsigned int)parser.getNumeral();
    }
    else
    {
        Exception ex(Exception::e_parameter_26);
        ex << "Keyword '" << keyword << "' was expected";
        throw ex;
    }
    return size;
}

//----------------------------------------------------------------------------------------
// Parse configuration file
//----------------------------------------------------------------------------------------
void 
Memconfig::read(const string& path, bool legacy)
{
    try
    {
        const fs::path fspath(path);
        if (fs::is_regular_file(fspath))
        {
             m_empty = false;
        }
    }
    catch (std::exception&)
    {
        m_empty = true;
    }

    if (m_empty)
    {
        return;
    }
    
    // Open file
    ifstream fs(path.c_str(), ios_base::in);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }

    bool memread(false);
    string line;
    int lineno(0);
    if (legacy == false)
    {
        while (fs.eof() == false)
        {
            lineno++;
            (void)getline(fs, line);
            Parser parser(line);    
            if (!parser.eoln() && (parser.peek() != '#'))    // Ignore comment and empty line
            {
                if (memread == true)
                {
                    Exception ex(Exception::e_parameter_26);
                    ex << "Unexpected token '" << parser.getChar() 
                       << "' was found at line " << lineno << " in file '" 
                       << path << "'.";
                    throw ex;
                }
            
                try
                {
                    m_ds = parseSize("-ds", parser);        // Data store size
                    m_ps = parseSize("-ps", parser);        // Program store size
                    m_rs = parseSize("-rs", parser);        // Reference store size
                }
                catch (Exception& ex)
                {
                    ex << " at line " << lineno << " in file '" << path << "'.";
                    throw ex;
                }

                parser.skipSpace();
                if (parser.eoln() == false)
                {
                	// Is there "RESERVED" keyword ?
                	const string& resrvkeyword = "-reserved";
                	// Is there an RP bus?
                	const string& keyword = "-norpbs";
                	if (parser.findToken(keyword))
                	{
                		m_rpbs = false;
                	}
                	else if (parser.eoln() == false)
                	{
                		if (line.find(resrvkeyword))
                		{
                			m_rsv = parseSize("-reserved", parser);
                			m_resrv = true;
                		}
                		else
                		{
                			cout<<"did not find reserved keyword in file"<<endl;
                			Exception ex(Exception::e_parameter_26);
                			ex << "Keyword '" << resrvkeyword << "' was expected at line "
                					<< lineno << " in file '" << path << "'.";
                			throw ex;
                		}
                	}
                	else
                	{
                		cout<<"did not find keyword in file"<<endl;
                		Exception ex(Exception::e_parameter_26);
                		ex << "Keyword '" << keyword << "' was expected at line "
                				<< lineno << " in file '" << path << "'.";
                		throw ex;
                	}
                	// Check remaining line
                    if ((parser.eoln() == false) && (parser.peek() != '#'))
                    {
                        Exception ex(Exception::e_parameter_26);
                        ex << "Unexpected token '" << parser.getChar() 
                           << "' was found at line " << lineno << " in file '" 
                           << path << "'.";
                        throw ex;
                    }
                }
                else
                {
                	m_rpbs = true;
                	m_resrv = false;
                }
                memread = true;
            }
        }
    }
    else
    {
        // Legacy file format
        while (fs.eof() == false)
        {
            lineno++;
            (void)getline(fs, line);
            Parser parser(line);    
            if (!parser.eoln() && (parser.peek() != '#'))    // Ignore comment and empty line
            {
                parser.skipSpace();
                if (parser.findToken("DS"))
                {
                    parser.skipSpace();
                    if (parser.findToken(":"))
                    {
                        parser.skipSpace();
                        m_ds = (unsigned int)parser.getNumeral();
                    }
                    else
                    {
                        Exception ex(Exception::e_parameter_26);
                        ex << "Unexpected token '" << parser.getChar() 
                           << "' was found at line " << lineno << " in file '" 
                           << path << "'.";
                        throw ex;
                    }
                }
                else if (parser.findToken("PS"))
                {
                    parser.skipSpace();
                    if (parser.findToken(":"))
                    {
                        parser.skipSpace();
                        m_ps = (unsigned int)parser.getNumeral();
                    }
                    else
                    {
                        Exception ex(Exception::e_parameter_26);
                        ex << "Unexpected token '" << parser.getChar() 
                           << "' was found at line " << lineno << " in file '" 
                           << path << "'.";
                        throw ex;
                    }
                }
                else if (parser.findToken("RS"))
                {
                    parser.skipSpace();
                    if (parser.findToken(":"))
                    {
                        parser.skipSpace();
                        m_rs = (unsigned int)parser.getNumeral();
                    }
                    else
                    {
                        Exception ex(Exception::e_parameter_26);
                        ex << "Unexpected token '" << parser.getChar() 
                           << "' was found at line " << lineno << " in file '" 
                           << path << "'.";
                        throw ex;
                    }
                }
                else
                {
                    Exception ex(Exception::e_parameter_26);
                    ex << "Unexpected keyword '" << parser.getString() 
                       << "' found  at line " << lineno << "in file '" << path 
                       << "'.";
                    throw ex;
                }

                parser.skipSpace();
                if (parser.eoln() == false)
                {
                    // Check remaining line
                    if ((parser.eoln() == false) && (parser.peek() != '#'))
                    {
                        Exception ex(Exception::e_parameter_26);
                        ex << "Unexpected token '" << parser.getChar() 
                           << "' was found at line " << lineno << " in file '" 
                           << path << "'.";
                        throw ex;
                    }
                }
            }
        }
    }

    // Close file
    fs.close();
}

//----------------------------------------------------------------------------------------
// Check if the memconfig is empty
//----------------------------------------------------------------------------------------
bool 
Memconfig::isEmpty() const
{
   return m_empty;
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
ostream& 
operator<<(ostream& s, const Memconfig& conf)
{
    if (!conf.isEmpty())
    {
        char buf[10];
        (void)s.setf(ios::right, ios::adjustfield);
        sprintf(buf, "%d", conf.m_ps/2);
        s << setw(6) << (conf.m_ps? buf: "all") << "   ";
        sprintf(buf, "%d", conf.m_rs/2);
        s << setw(6) << (conf.m_rs? buf: "all") << "   ";
        sprintf(buf, "%d", conf.m_ds/2);
        s << setw(6) << (conf.m_ds? buf: "all");
        if(conf.m_resrv)
        {
        	s << "   ";
        	sprintf(buf, "%d", conf.m_rsv);
        	s << setw(12) << (conf.m_rsv? buf: "0") ;
        }
        
        (void)s.unsetf(ios::right); // to restore the default
    }
    
    return s;
}
