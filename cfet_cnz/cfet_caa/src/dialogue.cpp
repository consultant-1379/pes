//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dialogue.cpp
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

#include <common.h>
#include <dialogue.h>
#include <exception.h>
#include <parser.h>
#include <iostream>
#include <sstream>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Dialogue::Dialogue()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Dialogue::~Dialogue()
{
}

//----------------------------------------------------------------------------------------
// Match a string against a keyword
//----------------------------------------------------------------------------------------
bool
Dialogue::matchKeyWord(const string& keyWord, const string& str)
{
    string tstr(str);
    tstr = Common::toUpper(tstr);
    return !tstr.empty() && !keyWord.find(tstr);
}

//----------------------------------------------------------------------------------------
// Expect "YES" or "NO" answer
//----------------------------------------------------------------------------------------
bool 
Dialogue::affirm(const string& text)
{
    char ibuf[128];
    bool loop(true);

    while (loop)
    {
        try
        {
            cout << text << "y[es]/n[o]: ";
            cin.clear();
            (void)cin.getline(ibuf, 128);

            Parser parser(ibuf);
            parser.skipSpace();
            if (parser.eoln() == true)
            {
                Exception ex(Exception::e_noinputonline_6);
                ex << "No input on line.";
                throw ex;
            }
            const string& answer = parser.getString();
            parser.skipSpace();
            if (parser.eoln() == false)
            {
                Exception ex(Exception::e_extrainputonline_5);
                ex << "Extra input on line.";
                throw ex;
            }
            (void)Common::toUpper(answer);
            if (matchKeyWord("YES", answer))
            {
                return true;
            }
            else if (matchKeyWord("NO", answer))
            {
                return false;
            }
            else
            {
                Exception ex(Exception::e_unexp_input_chk_19);
                ex << "Unexpected input '" << answer << "'.";
                throw ex;
            }
        }
        catch (Exception& ex)
        {
            cerr << ex << endl;
        }
        cout << endl;
    }
    return false;
}

//----------------------------------------------------------------------------------------
// Expect input from user
//----------------------------------------------------------------------------------------
string 
Dialogue::userInput(const string& text)
{
    char ibuf[128];
    bool loop(true);

    while (loop)
    {
        try
        {
            cout << text << ": ";
            cin.clear();
            (void)cin.getline(ibuf, 128);

            Parser parser(ibuf);
            parser.skipSpace();
            if (parser.eoln() == true)
            {
                Exception ex(Exception::e_noinputonline_6);
                ex << "No input on line.";
                throw ex;
            }
            const string& answer = parser.getString();
            parser.skipSpace();
            if (parser.eoln() == false)
            {
                Exception ex(Exception::e_extrainputonline_5);
                ex << "Extra input on line.";
                throw ex;
            }
            return answer;
        }
        catch (Exception& ex)
        {
            cerr << ex << endl;
        }
        cout << endl;
    }
    return "";
}

