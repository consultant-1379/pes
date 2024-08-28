//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      parser.h
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

#ifndef PARSER_H
#define PARSER_H

#include <string>

class Parser
{
public:

    // Constructor
    Parser(const std::string& line);

    // Destructor
    ~Parser();
    
    // Get next character from buffer
    char getChar();                                 // Returns a character from the buffer

    // Get next character from buffer, do not advance the pointer
    char peek() const {return *m_iter;}             // Returns a character from the buffer

    // Get an identity from buffer
    std::string getIdent();                         // Returns identity

    // Get a string from the buffer
    std::string getString();                        // Returns string

    // Find a token in the buffer                
    bool findToken(
            const std::string& token                // Token to search for
            );

    // Get a numeral from the buffer
    int getNumeral();

    // Skip white space in the buffer
    void skipSpace();

    // Have we reached end of the buffer
    bool eoln() const;

    static const char s_ILLFILE[];

private:

    // Copy constructor
    Parser(const Parser&) {};

    std::string m_line;
    std::string::const_iterator m_iter;
};

#endif