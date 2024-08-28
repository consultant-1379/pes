//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      ltime.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013, 2021. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for date and time functions.
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
//      PA1    2013-03-01   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-04-10   XLOBUNG     TR HR21336.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "ltime.h"
#include "exception.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <algorithm>
#include <errno.h>

using namespace std;

namespace PES_CLH {

const Time Time::s_mintime = Time::min();       // 1601-01-01 00:00:00.000000
const Time Time::s_maxtime = Time::max();       // 10000-01-01 00:00:00.000000

const time_t Time::s_second = 1;                // Second
const time_t Time::s_minute = 60;               // Minute
const time_t Time::s_hour = 3600;               // Hour
const time_t Time::s_day = 86400;               // Day

const int64_t Time::s_empty = int64_t(1) << 62;
                                                // Empty time

Time::t_zone Time::s_zone(e_local);
Time::t_format Time::s_format(e_plain);

//========================================================================================
// Class Time
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Time::Time() :
m_time(),
m_empty(true)
{
	isDst = -1;
}

Time::Time(const string& date, const string& time, const string& usec) :
m_time(),
m_empty(false)
{
   isDst = -1;
   set(date, time, usec);
}

Time::Time(int64_t time, bool local) :
m_time(),
m_empty(false)
{
   isDst = -1;
   if (time != s_empty)
   {
      lldiv_t div = lldiv(time, 1000000);
      m_time.tv_sec = div.quot;
      if (div.rem >=0)
      {
         m_time.tv_usec = div.rem;
      }
      else
      {
         m_time.tv_usec = -div.rem;
      }
      
      if (local)
      {
         // Convert from local time to UTC time
         int64_t offset = getZoneOffset();
         timeval res = {offset, 0};
         timersub(&m_time, &res, &m_time);
      }
      validate();
   }
   else
   {
      m_empty = true;
   }
}

Time::Time(timeval time) :
m_time(time),
m_empty(false)
{
   isDst = -1;
   validate();
}

Time::Time(time_t tv_sec, suseconds_t tv_usec) :
m_time(),
m_empty(false)
{
   m_time.tv_sec = tv_sec;
   m_time.tv_usec = tv_usec;
   isDst = -1;

   validate();
}

//----------------------------------------------------------------------------------------
//   Destructor
//----------------------------------------------------------------------------------------
Time::~Time()
{
}

//----------------------------------------------------------------------------------------
//   Equality operator
//----------------------------------------------------------------------------------------
bool Time::operator==(const Time& time) const
{
   validate();
   time.validate();
   return timercmp(&m_time, &time.m_time, ==);
}

//----------------------------------------------------------------------------------------
//   Less than operator
//----------------------------------------------------------------------------------------
bool Time::operator<(const Time& time) const
{
   validate();
   time.validate();
   return timercmp(&m_time, &time.m_time, <);
}

//----------------------------------------------------------------------------------------
//   Greater than operator
//----------------------------------------------------------------------------------------
bool Time::operator>(const Time& time) const
{
   validate();
   time.validate();
   return timercmp(&m_time, &time.m_time, >);
}

//----------------------------------------------------------------------------------------
//   Less than or equality operator
//----------------------------------------------------------------------------------------
bool Time::operator<=(const Time& time) const
{
   validate();
   time.validate();
   return !timercmp(&m_time, &time.m_time, >);
}

//----------------------------------------------------------------------------------------
//   Greater than or equality operator
//----------------------------------------------------------------------------------------
bool Time::operator>=(const Time& time) const
{
   validate();
   time.validate();
   return !timercmp(&m_time, &time.m_time, <);
}

//----------------------------------------------------------------------------------------
//   String operator
//----------------------------------------------------------------------------------------
Time::operator string() const
{
   validate();
   return get();
}

//----------------------------------------------------------------------------------------
// int64_t operator
//----------------------------------------------------------------------------------------
Time::operator int64_t() const
{
   if (empty() == false)
   {
      validate();
      
      if (m_time.tv_sec < 0)
      {
         // Handling the time before 19700101 00:00
         return m_time.tv_sec * 1000000 - m_time.tv_usec;
      }
      
      return m_time.tv_sec * 1000000 + m_time.tv_usec;
   }
   else
   {
      return s_empty;
   }
}

//----------------------------------------------------------------------------------------
// timeval operator
//----------------------------------------------------------------------------------------
Time::operator timeval() const
{
   validate();
   return m_time;
}

//----------------------------------------------------------------------------------------
//   Add a time value
//----------------------------------------------------------------------------------------
Time Time::operator+(const time_t& time) const
{
   validate();
   timeval res1 = {time, 0};
   timeval res2;
   timeradd(&m_time, &res1, &res2);

   return Time(res2.tv_sec, res2.tv_usec);
}

//----------------------------------------------------------------------------------------
//   Subtract a time value
//----------------------------------------------------------------------------------------
Time Time::operator-(const time_t& time) const
{
   validate();
   timeval res1 = {time, 0};
   timeval res2;
   timersub(&m_time, &res1, &res2);

   return Time(res2.tv_sec, res2.tv_usec);
}

//----------------------------------------------------------------------------------------
//   Add a time value, modify object
//----------------------------------------------------------------------------------------
Time& Time::operator+=(const time_t& time)
{
   validate();
   timeval res = {time, 0};
   timeradd(&m_time, &res, &m_time);

   return *this;
}

//----------------------------------------------------------------------------------------
//   Subtract a time value, modify object
//----------------------------------------------------------------------------------------
Time& Time::operator-=(const time_t& time)
{
   validate();
   timeval res = {time, 0};
   timersub(&m_time, &res, &m_time);

   return *this;
}

//----------------------------------------------------------------------------------------
// Calculate difference between two time objects
//----------------------------------------------------------------------------------------
time_t Time::operator-(const Time& time) const
{
   validate();
   time.validate();
   timeval res;
   timersub(&m_time, &time.m_time, &res);

   return res.tv_sec;
}

//----------------------------------------------------------------------------------------
//   Set date - time will be set to 00:00:00
//----------------------------------------------------------------------------------------
void Time::setDate(const string& date)
{
   // Expected format: YYYYMMDD
   string tdate = date;
   if (date.size() == 8)
   {
      tdate.insert(6, 1, '-');
      tdate.insert(4, 1, '-');
   }
   else
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }

   tm tmtime;
   tmtime.tm_isdst = -1;
   tmtime.tm_hour = 0;
   tmtime.tm_min = 0;
   tmtime.tm_sec = 0;

   const char* ptr = strptime(tdate.c_str(), "%F", &tmtime);
   if (!ptr)
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }
   if (*ptr != 0)
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }
   if (checkInvalidDate(tmtime))
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }

   m_time.tv_sec = mktime(&tmtime);
   m_time.tv_usec = 0;

   // Validate date
   if (timercmp(&m_time, &s_mintime.m_time, <) ||
       !timercmp(&m_time, &s_maxtime.m_time, <))
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }
   m_empty = false;
}

//----------------------------------------------------------------------------------------
//   Set time - date will be set to today
//----------------------------------------------------------------------------------------
void Time::setTime(const string& time, bool seconds)
{
   tm tmtime;

   // Expected format: HHMM[SS]
   string ttime = time;
   if (time.size() == 4)
   {
      ttime.insert(2, 1, ':');
      ttime.append(":00");
   }
   else if (time.size() == 6 && seconds)
   {
      ttime.insert(4, 1, ':');
      ttime.insert(2, 1, ':');
   }
   else
   {
      throw Exception(Exception::illTime(time), WHERE__);
   }

   timeval tval;

   // Get current date and time
   int res = gettimeofday(&tval, NULL);
   if (res != 0)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to get time of day.";
      ex.sysError();
      throw ex;
   }

   tm* tmtimep = localtime_r(&tval.tv_sec, &tmtime);
   if (tmtimep == 0)
   {
      throw Exception(Exception::illTime(time), WHERE__);
   }

   const char* ptr = strptime(ttime.c_str(), "%T", &tmtime);
   if (!ptr)
   {
      throw Exception(Exception::illTime(time), WHERE__);
   }

   m_time.tv_sec = mktime(&tmtime);
   m_time.tv_usec = 0;

   m_empty = false;
}

//----------------------------------------------------------------------------------------
// Set date and time
//----------------------------------------------------------------------------------------
void Time::set(const string& date, const string& time, bool seconds)
{
    // Expected format: YYYYMMDD
    string tdate = date;
    if (date.size() == 8)
    {
       tdate.insert(6, 1, '-');
       tdate.insert(4, 1, '-');
    }
    else
    {
       throw Exception(Exception::illDate(date), WHERE__);
    }

    const char* ptr;
    tm tmtime;
    tmtime.tm_isdst = -1;
    ptr = strptime(tdate.c_str(), "%F", &tmtime);
    if (!ptr)
    {
       throw Exception(Exception::illDate(date), WHERE__);
    }
    if (*ptr != 0)
    {
       throw Exception(Exception::illDate(date), WHERE__);
    }
    if (checkInvalidDate(tmtime))
    {
   throw Exception(Exception::illDate(date), WHERE__);
    }

    // Expected format: HHMM[SS]
    string ttime = time;
    if (time.size() == 4)
    {
       ttime.insert(2, 1, ':');
       ttime.append(":00");
    }
    else if (time.size() == 6 && seconds)
    {
       ttime.insert(4, 1, ':');
       ttime.insert(2, 1, ':');
    }
    else
    {
       throw Exception(Exception::illTime(time), WHERE__);
    }

    ptr = strptime(ttime.c_str(), "%T", &tmtime);
    if (!ptr)
    {
       throw Exception(Exception::illTime(time), WHERE__);
    }

    m_time.tv_sec = mktime(&tmtime);

    // Validate date
    if (timercmp(&m_time, &s_mintime.m_time, <) ||
        !timercmp(&m_time, &s_maxtime.m_time, <))
    {
       throw Exception(Exception::illDate(date), WHERE__);
    }
    m_empty = false;
}

//----------------------------------------------------------------------------------------
// Set date and time (with microseconds)
//----------------------------------------------------------------------------------------
void Time::set(const string& date, const string& time, const string& usec)
{
   // Expected format: YYYYMMDD
   string tdate = date;
   if (date.size() == 8)
   {
      tdate.insert(6, 1, '-');
      tdate.insert(4, 1, '-');
   }
   else
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }

   const char* ptr;
   tm tmtime;
   tmtime.tm_isdst = -1;
   ptr = strptime(tdate.c_str(), "%F", &tmtime);
   if (!ptr)
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }
   if (*ptr != 0)
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }
   if (checkInvalidDate(tmtime))
   {
   throw Exception(Exception::illDate(date), WHERE__);
   }

   // Expected format: HHMMSS
   string ttime = time;
   if (time.size() == 6)
   {
      ttime.insert(4, 1, ':');
      ttime.insert(2, 1, ':');
   }
   else
   {
      throw Exception(Exception::illTime(time), WHERE__);
   }

   ptr = strptime(ttime.c_str(), "%T", &tmtime);
   if (!ptr)
   {
      throw Exception(Exception::illTime(time), WHERE__);
   }

   m_time.tv_sec = mktime(&tmtime);
   isDst = tmtime.tm_isdst; //TR_HY85159: saving the isdst value from local variable tmtime into class variable.

   // Expected format: UUUUUU
   if (usec.empty())
   {
      m_time.tv_usec = 0;
   }
   else if (usec.size() == 6)
   {
      try
      {
         m_time.tv_usec = boost::lexical_cast<int64_t>(usec);
      }
      catch (exception& e)
      {
         throw Exception(Exception::illTime(time), WHERE__);
      }
   }
   else
   {
       throw Exception(Exception::illTime(time), WHERE__);
   }

   // Validate date
   if (timercmp(&m_time, &s_mintime.m_time, <) ||
       !timercmp(&m_time, &s_maxtime.m_time, <))
   {
      throw Exception(Exception::illDate(date), WHERE__);
   }
   m_empty = false;
}

//----------------------------------------------------------------------------------------
//   Get date and time as two strings: "YYYYMMDD", "HHMMSS"
//----------------------------------------------------------------------------------------
void Time::get(std::string& date, std::string& time) const
{
   validate();

   tm* tmtime = localtime(&m_time.tv_sec);
   if (tmtime == 0)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to convert file time to system time.";
      ex.sysError();
      throw ex;
   }

   char buf[32];
   strftime(buf, 31, "%Y%m%d", tmtime);
   date = buf;
   strftime(buf, 31, "%H%M%S", tmtime);
   time = buf;
}

//----------------------------------------------------------------------------------------
//   Get date and time as one string
// Plain format:  "YYYYMMDD_HHMMSS"
// Long format:   "YYYYMMDD_HHMMSS_uuuuuu"
// Pretty format: "YYYY-MM-DD HH:MM:SS.mmm  Zone"
//----------------------------------------------------------------------------------------
string Time::get(t_format format, t_zone zone) const
{
   validate();

   tm tmtime;
   tmtime.tm_isdst = -1;
   tm* ptr;
   if (zone == e_local)
   {
      ptr = localtime_r(&m_time.tv_sec, &tmtime);

      //HY85159: It was found that sometimes wrong localtime are generated during DST change.
      //Hence making use of mktime() by setting the tm_idst flag to -1 so that it will correct
      //the tmtime if any wrong local time generated by localtime_r.
      tmtime.tm_isdst = -1;
      time_t mktime_tv_sec = mktime(&tmtime);
   }
   else
   {
      ptr = gmtime_r(&m_time.tv_sec, &tmtime);
   }
   if (!ptr)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to get time.";
      ex.sysError();
      throw ex;
   }

   char buf[32];
   ostringstream s;
   switch (format)
   {
   case e_plain:
      s << "%Y%m%d_%H%M%S";
      break;

   case e_long:
      s << "%Y%m%d_%H%M%S_" << setw(6) << setfill('0') << m_time.tv_usec;
      break;

   case e_pretty:
      s << "%F  %T." << setw(3) << setfill('0') << m_time.tv_usec/1000 << "  %Z";
      break;

   case e_tvsec:
      s << m_time.tv_sec;
      break;

   default:
      assert(!"Illegal time format");
   }

   strftime(buf, 31, s.str().c_str(), &tmtime);
   return buf;
}

//----------------------------------------------------------------------------------------
// Check leap year
//----------------------------------------------------------------------------------------
bool Time::isLeapYear(int year)
{
   if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
      return true; /* leap */
    else
       return false; /* no leap */
}

//----------------------------------------------------------------------------------------
// Check if the date is invalid
//----------------------------------------------------------------------------------------
bool Time::checkInvalidDate(tm tmdate)
{
   int year = tmdate.tm_year + 1900;
   int month = tmdate.tm_mon + 1;
   int day = tmdate.tm_mday;

   switch (month)
   {
   case 2:
      if (isLeapYear(year))
      {
         if (day > 29)
         {
            return true;
         }
      }
      else
      {
         if (day > 28)
         {
            return true;
         }
      }
      break;
   case 4:
   case 6:
   case 9:
   case 11:
      if (day > 30)
      {
         return true;
      }
      break;
   }

   return false;
}

//----------------------------------------------------------------------------------------
// Clear the time
//----------------------------------------------------------------------------------------
void Time::clear()
{
   m_empty = true;
}

//----------------------------------------------------------------------------------------
// Get current date and time
//----------------------------------------------------------------------------------------
Time Time::now()
{
   timeval time;
   int res = gettimeofday(&time, 0);
   if (res != 0)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to get time of day.";
      ex.sysError();
      throw ex;
   }
   return Time(time.tv_sec, time.tv_usec);
}

//----------------------------------------------------------------------------------------
// Get min time (1601-01-01 00:00:00.000000)
//----------------------------------------------------------------------------------------
Time Time::min()
{
   tm tmtime;
   tmtime.tm_isdst = -1;
   tmtime.tm_year = 1601 - 1900;
   tmtime.tm_mon = 0;
   tmtime.tm_mday = 1;
   tmtime.tm_hour = 0;
   tmtime.tm_min = 0;
   tmtime.tm_sec = 0;
   suseconds_t tv_usec = 0;

   time_t tv_sec = mktime(&tmtime);
   if (tv_sec == -1)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to convert time.";
      ex.sysError();
      throw ex;
   }

   return Time(tv_sec, tv_usec);
}

//----------------------------------------------------------------------------------------
// Get max time (10000-01-01 00:00:00.000000)
//----------------------------------------------------------------------------------------
Time Time::max()
{
   tm tmtime;
   tmtime.tm_isdst = -1;
   tmtime.tm_year = 10000 - 1900;
   tmtime.tm_mon = 0;
   tmtime.tm_mday = 1;
   tmtime.tm_hour = 0;
   tmtime.tm_min = 0;
   tmtime.tm_sec = 0;
   suseconds_t tv_usec = 0;

   time_t tv_sec = mktime(&tmtime);
   if (tv_sec == -1)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to convert time.";
      ex.sysError();
      throw ex;
   }

   return Time(tv_sec, tv_usec);
}

//----------------------------------------------------------------------------------------
// Calculate the offset in seconds between local time and UTC time
//----------------------------------------------------------------------------------------
int64_t Time::getZoneOffset() const
{
   timeval time;
   /*int res = gettimeofday(&time, 0);
   if (res != 0)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to get time of day.";
      ex.sysError();
      throw ex;
   }*/
   time.tv_usec = 0;
   time.tv_sec = m_time.tv_sec;

   tm tmtime;
   tmtime.tm_isdst = -1;
   tm* ptr = localtime_r(&time.tv_sec, &tmtime);
   if (!ptr)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Failed to get local time.";
      ex.sysError();
      throw ex;
   }

   return tmtime.tm_gmtoff;
}

//----------------------------------------------------------------------------------------
// Check if time is empty
//----------------------------------------------------------------------------------------
bool Time::empty() const
{
   return m_empty;
}

//----------------------------------------------------------------------------------------
// Assert that time is valid
//----------------------------------------------------------------------------------------
void Time::validate() const
{
   if (empty())
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Empty time.";
      throw ex;
   }
   if (timercmp(&m_time, &s_mintime.m_time, <) ||
       timercmp(&m_time, &s_maxtime.m_time, >))
   {
      Exception ex(Exception::parameter(), WHERE__);
      ex << "Invalid time.";
      throw ex;
   }
}


//----------------------------------------------------------------------------------------
//   Check if it is DST time
//----------------------------------------------------------------------------------------
bool Time::isDstTime() const
{
	return (isDst == 1);
}


//----------------------------------------------------------------------------------------
//   Outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Time& time)
{
   if (time.empty() == false)
   {
      s << time.get(Time::s_format, Time::s_zone);
   }
   else
   {
      s << "(empty)";
   }
   Time::s_zone = Time::e_local;
   Time::s_format = Time::e_plain;
   return s;
}

//----------------------------------------------------------------------------------------
// Time zone operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, Time::t_zone zone)
{
   Time::s_zone = zone;
   return s;
}

//----------------------------------------------------------------------------------------
// Format operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, Time::t_format format)
{
   Time::s_format = format;
   return s;
}

//========================================================================================
// Class Period
//========================================================================================

//----------------------------------------------------------------------------------------
//   Constructors
//----------------------------------------------------------------------------------------
Period::Period():
m_first(Time::s_mintime),
m_last(Time::s_maxtime)
{
   m_last -= Time::s_second;
   m_last.m_time.tv_usec = 999999;
}

Period::Period(const Time& first, const Time& last) :
m_first(first),
m_last(last)
{
   if (m_first.empty() && m_last.empty())
   {
      return;
   }

   m_first.validate();
   m_last.validate();

   if (m_first > m_last)
   {
      throw Exception(Exception::illTimeInter(), WHERE__);
   }
}

Period::Period(
         const string& startdate,
         const string& starttime,
         const string& stopdate,
         const string& stoptime
         ) :
m_first(),
m_last()
{
   // Start time
   if (!starttime.empty() && !startdate.empty())
   {
      // Start time and date provided
      m_first.set(startdate, starttime, false);
   }
   else if (starttime.empty() && !startdate.empty())
   {
      // Start date provided
      m_first.setDate(startdate);
   }
   else if (!starttime.empty() && startdate.empty())
   {
      // Start time provided
      m_first.setTime(starttime, false);
   }
   else
   {
      // No start time or date provided
      m_first = Time::s_mintime;
   }

   // Stop time
   if (!stoptime.empty() && !stopdate.empty())
   {
      // Stop time and date provided
      m_last.set(stopdate, stoptime, false);
      m_last += Time::s_minute - Time::s_second;
      m_last.m_time.tv_usec = 999999;
   }
   else if (stoptime.empty() && !stopdate.empty())
   {
      // Stop date provided
      m_last.setDate(stopdate);
      m_last += Time::s_day - Time::s_second;
      m_last.m_time.tv_usec = 999999;
   }
   else if (!stoptime.empty() && stopdate.empty())
   {
      // Stop time provided
      m_last.setTime(stoptime, false);
      m_last += Time::s_minute - Time::s_second;
      m_last.m_time.tv_usec = 999999;
   }
   else
   {
      // No stop time or date provided
      m_last = Time::s_maxtime;
      m_last -= Time::s_second;
      m_last.m_time.tv_usec = 999999;
   }

   // Stop time must not be smaller than start time
   if (m_first > m_last)
   {
      throw Exception(Exception::illTimeInter(), WHERE__);
   }
}

//----------------------------------------------------------------------------------------
//   Destructor
//----------------------------------------------------------------------------------------
Period::~Period()
{
}

//----------------------------------------------------------------------------------------
//   Get first time in period
//----------------------------------------------------------------------------------------
Time Period::first() const
{
   if (empty())
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Time period is empty.";
      throw ex;
   }
   return m_first;
}

//----------------------------------------------------------------------------------------
//   Get first time in period
//----------------------------------------------------------------------------------------
Time Period::last() const
{
   if (empty())
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Time period is empty.";
      throw ex;
   }
   return m_last;
}

//----------------------------------------------------------------------------------------
//   Check if period is empty
//----------------------------------------------------------------------------------------
bool Period::empty() const
{
   return m_first.empty();
}

//----------------------------------------------------------------------------------------
//   Outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Period& period)
{
   if (period.empty() == false)
   {
      s << period.m_first << "   " << period.m_last;
   }
   return s;
}

}

