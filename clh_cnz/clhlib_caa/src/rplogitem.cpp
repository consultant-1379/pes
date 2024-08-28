//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rplogitem.cpp
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
//      Implementation for class that describing each RP log file.
//      Most of the codes are based on rplogitem.cpp on APG43.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424
//
//  AUTHOR
//      XDT/DEK/XVUNGUY
//
//  REVISION HISTORY
//      Rev.      Date         Prepared    Description
//      ----      ----         --------    -----------
//      R2A04     2013-03-01   XVUNGUY     CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "rplogitem.h"

namespace PES_CLH
{
//-----------------------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------------------
RPLogItem::RPLogItem(
string logname,
string mag,
string slot,
string rpno,
string start_time,
string filename):
m_logname(logname),
m_mag(mag),
m_slot(slot),
m_rpno(rpno),
m_startTime(start_time),
m_stopTime(start_time),
m_fileName(filename)
{
}

//-----------------------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------------------
RPLogItem::~RPLogItem(void)
{
}

//-----------------------------------------------------------------------------------------
// Get log name
//-----------------------------------------------------------------------------------------
const string RPLogItem::getLogName() const
{
   return m_logname;
}

//-----------------------------------------------------------------------------------------
// Get Magazine
//-----------------------------------------------------------------------------------------
const string RPLogItem::getMag() const
{
   return m_mag;
}

//-----------------------------------------------------------------------------------------
// Get Slot
//-----------------------------------------------------------------------------------------
const string RPLogItem::getSlot() const
{
   return m_slot;
}

//-----------------------------------------------------------------------------------------
// Get RP number
//-----------------------------------------------------------------------------------------
const string RPLogItem::getRPNo() const
{
   return m_rpno;
}

//-----------------------------------------------------------------------------------------
// Set start time
//-----------------------------------------------------------------------------------------
void RPLogItem::setStartTime(const string& time)
{
   m_startTime = time;
}

//-----------------------------------------------------------------------------------------
// Get Start time
//-----------------------------------------------------------------------------------------
const string RPLogItem::getStartTime() const
{
   return m_startTime;
}

//-----------------------------------------------------------------------------------------
// Set Stop time
//-----------------------------------------------------------------------------------------
void RPLogItem::setStopTime(const string& time)
{
   m_stopTime = time;
}

//-----------------------------------------------------------------------------------------
// Get Stop time
//-----------------------------------------------------------------------------------------
const string RPLogItem::getStopTime() const
{
   return m_stopTime;
}

//-----------------------------------------------------------------------------------------
// Get filename
//-----------------------------------------------------------------------------------------
const string RPLogItem::getFileName() const
{
   return m_fileName;
}

}
