//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cmdparser.h
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
//      Class for parsing console commands.
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

#ifndef CMDPARSER_H_
#define CMDPARSER_H_

#include <string>
#include <deque>

namespace PES_CLH {

class CmdParser
{
   friend std::ostream& operator<<(std::ostream& s, const CmdParser& getcmd);

public:
   // Class for handling option without argument
   class Opt
   {
      friend class CmdParser;

   public:
      // Constructor
      Opt(
            const std::string& opt      // Option
            );

      // Get option string
      std::string getOpt() const;   

      // Was option found in command?
      bool found() const;                  

   private:
      // Check if option is a legal identifier
      void isIdentifier(
            const std::string& opt      // Option
            ) const;

      std::string m_opt;
      bool m_found;
   };

   // Class for handling option with argument
   class Optarg: public Opt
   {
      friend class CmdParser;

   public:
      // Constructor
      Optarg(
            const std::string& opt      // Option
            );

      // Get option argument
      std::string getArg() const;      

   private:
      std::string m_arg;
   };

   // Constructor
   CmdParser(
         int argc,                     // Number of arguments 
         const char* argv[]            // Argument list
         );

   // Destructor
   ~CmdParser();
   
   // Get command name
   std::string getCmdName() const;      // Returns command name

   // Fetch option
   bool fetchOpt(                       // Returns true if option was found
         Opt& opt                       // Option
         );

   // Fetch option with argument
   bool fetchOpt(                      // Returns true if option was found
         Optarg& optarg                // Option and argument
         );

   // Fetch parameter
   bool fetchPar(                      // Returns true if parameter was found
         std::string& par              // Parameter
         );
   
   // Check command
   void check();

   // Check if option was supplied
   bool optFound(                      // Returns true if option was found in the command parsing
         const std::string& opt        // Option
         ) const; 

private:
   typedef std::deque<std::string> LIST;
   typedef LIST::iterator LISTITER;
   typedef LIST::const_iterator LISTCITER;

   std::string m_command;
   LIST m_arglist;
   LISTITER m_iter;
};

std::ostream& operator<<(std::ostream& s, const CmdParser& getcmd);

}

#endif // CMDPARSER_H_
