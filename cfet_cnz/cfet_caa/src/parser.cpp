//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      parser.cpp
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
//      This class contain methods for parsing configuration files.
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
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <parser.h>
#include <exception.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;

const char Parser::s_ILLFILE[] = ":\"*?<>|";

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Parser::Parser(const string& line) :
m_line(line),
m_iter(m_line.begin())
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Parser::~Parser()
{
}

//----------------------------------------------------------------------------------------
// Get next character
//----------------------------------------------------------------------------------------
char 
Parser::getChar()
{
    char ch = *m_iter;
    m_iter++;
    return ch;
}

//----------------------------------------------------------------------------------------
// Get identifier
//----------------------------------------------------------------------------------------
std::string 
Parser::getIdent()
{
    string ident;
    if (isalpha(*m_iter))
    {
        ident = *m_iter;
        m_iter++;
        while (m_iter != m_line.end() && (isalnum(*m_iter) || *m_iter == '_'))
        {
            ident += *m_iter;
            m_iter++;
        }
    }
    return ident; 
}

//----------------------------------------------------------------------------------------
// Get string
//----------------------------------------------------------------------------------------
string 
Parser::getString()
{
    string path;
    while (m_iter != m_line.end() && !isspace(*m_iter))
    {
        path += *m_iter;
        m_iter++;
    }
    return path; 
}

//----------------------------------------------------------------------------------------
// Find token
//----------------------------------------------------------------------------------------
bool 
Parser::findToken(const std::string& token)
{
    size_t size = token.size();
    if (token == m_line.substr(m_iter - m_line.begin(), size))
    {
        m_iter += size;
        return true;
    }
    else
    {
        return false;
    }
}

//----------------------------------------------------------------------------------------
// Get numeral
//----------------------------------------------------------------------------------------
int
Parser::getNumeral()
{
    const char* buf = &*m_iter;
    char* p;
    errno = 0;
    int var = strtoul(buf, &p, 0);
    if (*p != ' ' && *p != '\t' && *p != 0)
    {
        Exception ex(Exception::e_parameter_26);
        ex << "Integer was expected";
        throw ex;
    }
    if (errno == ERANGE)
    {
        Exception ex(Exception::e_parameter_26);
        ex << "Overflow occurred";
        throw ex;
    }
    m_iter += p - buf;     // uabtso

    return var;
}

//----------------------------------------------------------------------------------------
// Skip space
//----------------------------------------------------------------------------------------
void Parser::skipSpace()
{
    while (m_iter != m_line.end() && isspace(*m_iter))
    {
        m_iter++;
    }
}

//----------------------------------------------------------------------------------------
// Check if end of line
//----------------------------------------------------------------------------------------
bool 
Parser::eoln() const 
{
    return m_iter == m_line.end();
}
