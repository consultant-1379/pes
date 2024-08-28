//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dialogue.h
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
//      This class is used for handling command dialogues.
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

#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <string>

class Dialogue
{
public:

    // Constructor
    Dialogue();

    // Destructor
    ~Dialogue();

    // Match a keyword towards a string, keyword may be a substring of string
    // starting at first character
    static bool matchKeyWord(                       // Returns true if there is a match,
                                                    // false otherwise
            const std::string& keyWord,             // Keyword 
            const std::string& str                  // String
            );

    // Test for keywords "yes" or "no"
    static bool affirm(                             // Returns true if there is a match,
                                                    // false otherwise
            const std::string& text = ""            // Prompt text
            );

    // Read input from user dialogue
    static std::string userInput(
            const std::string& text = ""            // Prompt text
            );
    
private:
};

#endif