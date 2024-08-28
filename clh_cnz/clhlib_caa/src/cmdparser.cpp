//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cmdparser.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for parsing console command
//      
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-03-13   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "cmdparser.h"
#include "exception.h"
#include <iostream>

using namespace std;

namespace PES_CLH {

//=======================================================================================
// Class CmdParser::Opt
//=======================================================================================

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
CmdParser::Opt::Opt(const string& opt):
m_opt(opt),
m_found(false)
{
    isIdentifier(opt);
}

//----------------------------------------------------------------------------------------
// Get option
//----------------------------------------------------------------------------------------
string CmdParser::Opt::getOpt() const 
{
   return m_opt;
}

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
bool CmdParser::Opt::found() const 
{
   return m_found;
}

//----------------------------------------------------------------------------------------
// Check if option is a legal identifier
//----------------------------------------------------------------------------------------
void CmdParser::Opt::isIdentifier(const string& opt) const
{
   string::const_iterator iter = opt.begin();
   if (!isalpha(*iter))
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Illegal character '" << *iter << "' in option.";
      throw ex;
   }

   for (; iter != opt.end(); ++iter)
   {
      if (!isalnum(*iter))
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << "Illegal character '" << *iter << "' in option.";
         throw ex;
      }
   }
}

//=======================================================================================
// Class CmdParser::Optarg
//=======================================================================================

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
CmdParser::Optarg::Optarg(const string& opt):
Opt(opt),
m_arg()
{
}

//----------------------------------------------------------------------------------------
// Get option argument
//----------------------------------------------------------------------------------------
string 
CmdParser::Optarg::getArg() const 
{
   return m_arg;
}

//=======================================================================================
// Class CmdParser
//=======================================================================================

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
CmdParser::CmdParser(int argc, const char* argv[]):
m_command(argv[0]),
m_arglist(argv + 1, argv + argc),
m_iter(m_arglist.begin())
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
CmdParser::~CmdParser()
{
}

//----------------------------------------------------------------------------------------
// Get command name
//----------------------------------------------------------------------------------------

string CmdParser::getCmdName() const 
{
   return m_command;
}

//----------------------------------------------------------------------------------------
// Fetch option from command line
//----------------------------------------------------------------------------------------
bool CmdParser::fetchOpt(Opt& opt)
{
   for (LISTITER iter = m_arglist.begin(); iter != m_arglist.end(); ++iter)
   {
      if ("-" + opt.m_opt == *iter)
      {
         iter->erase();
         if (iter >= m_iter)
         {
             m_iter = ++iter;
         }
         opt.m_found = true;
         return true;
      }
   }
   return false;
}

//----------------------------------------------------------------------------------------
// Fetch option and argument from command line
//----------------------------------------------------------------------------------------
bool CmdParser::fetchOpt(Optarg& optarg)
{
    bool found(false);
    LISTITER iter = m_arglist.begin();
    while (iter != m_arglist.end())
    {
        if ("-" + optarg.m_opt == *iter)
        {
            found = true;
            iter->erase();
            break;
        }
        iter++;
    }

    optarg.m_arg.erase();
    if (found)
    {
        ++iter;
        if (iter == m_arglist.end())
        {
            throw Exception(Exception::argExpected("-" + optarg.m_opt), WHERE__);
        }

        if (((*iter)[0] == '-') || iter->empty())
        {
            throw Exception(Exception::argExpected("-" + optarg.m_opt), WHERE__);
        }
        else
        {
            optarg.m_arg = *iter;
            optarg.m_found = true;
            iter->erase();
            if (iter >= m_iter)
            {
                m_iter = ++iter;
            }
        }
    }
    return found;
}

//----------------------------------------------------------------------------------------
// Fetch parameter from command line
//----------------------------------------------------------------------------------------
bool CmdParser::fetchPar(string& par)
{
   par.erase();
   if (m_iter == m_arglist.end())
   {
       return false;
   }

   par = *m_iter;
   if (par[0] == '-')
   {
      throw Exception(Exception::unexpOption(par), WHERE__);
   }
   else
   {
      m_iter->erase();
      ++m_iter;
      return true;
   }
}

//----------------------------------------------------------------------------------------
// Check command 
//----------------------------------------------------------------------------------------
void CmdParser::check()
{
   for (LISTCITER iter = m_arglist.begin(); iter != m_arglist.end(); ++iter)
   {
      const string& par = *iter;
      if (par.empty() == false)
      {
         if (par[0] == '-')
         {
            throw Exception(Exception::unexpOption(par), WHERE__);
         }
         else
         {
            throw Exception(Exception::unexpToken(par), WHERE__);
         }
      }
   }
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const CmdParser& cmdparser)
{
   for (CmdParser::LISTCITER iter = cmdparser.m_arglist.begin();
         iter != cmdparser.m_arglist.end();
         iter++)
   {
      s << (iter->empty()? "(null)": *iter) << " ";
   }
   return s;
}

}

