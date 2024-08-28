//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      ltime.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2011, 2021. All rights reserved.
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
//      PA1    2012-02-23   UABTSO      CLH adapted to APG43 on Linux.
//      PA2    2013-04-10   XLOBUNG     TR HR21336.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef LTIME_H_
#define LTIME_H_

#include <string>
#include <iostream>
#include <sys/time.h>
#include <stdint.h>

namespace PES_CLH {

//========================================================================================
// Class Time
//========================================================================================

class Time
{
   friend class Period;

public:

   // Time zone
   enum t_zone
   {
      e_local,          // Local time zone
      e_utc             // UTC
   };

   // Date format
   enum t_format {
      e_plain,          // "YYYYMMDD_HHMMSS"
      e_long,           // "YYYYMMDD_HHMMSS_uuuuuu"
      e_pretty,         // "YYYY-MM-DD HH:MM:SS.mmm  Zone"
      e_tvsec           // "dddddddddd": Unix time
   };

   Time();

   Time(
         const std::string& date,
         const std::string& time,
         const std::string& usec = std::string()
         );

   Time(
         int64_t time,
         bool local = false
         );

   ~Time();

   bool operator==(
         const Time& time
         ) const;

   bool operator<(
         const Time& time
         ) const;

   bool operator>(
         const Time& time
         ) const;

   bool operator<=(
         const Time& time
         ) const;

   bool operator>=(
         const Time& time
         ) const;

   Time operator+(
         const time_t& time
         ) const;

   Time operator-(
         const time_t& time
         ) const;

   Time& operator+=(
         const time_t& time
         );

   Time& operator-=(
         const time_t& time
         );

   time_t operator-(const Time& time) const;

   operator std::string() const;
   operator int64_t() const;
   operator timeval() const;
   bool empty() const;

   void clear();

   void setDate(
         const std::string& date
         );

   void setTime(
         const std::string& time,
         bool seconds = true
         );

   void set(
         const std::string& date,
         const std::string& time,
         bool seconds = true
         );

   void set(
         const std::string& date,
         const std::string& time,
         const std::string& usec
         );

   void get(
         std::string& date,
         std::string& time
         ) const;

   std::string get(
         t_format format = e_plain,
         t_zone zone = e_local
         ) const;

   bool isDstTime() const; //TR_HY85159

   static Time now();

   static const Time s_mintime;
   static const Time s_maxtime;

   static const time_t s_second;
   static const time_t s_minute;
   static const time_t s_hour;
   static const time_t s_day;

private:

   Time(
         timeval time
         );

   Time(
         time_t tv_sec,
         suseconds_t tv_usec
         );

   int64_t getZoneOffset() const;

   void validate() const;

   bool isLeapYear(int year);

   bool checkInvalidDate(tm tmdate);

   friend std::ostream& operator<<(
         std::ostream& s, const Time& time
         );

   friend std::ostream& operator<<(
         std::ostream& s, t_zone zone
         );

   friend std::ostream& operator<<(
         std::ostream& s,
         t_format format
         );

   static Time min();
   static Time max();

   timeval m_time;
   bool m_empty;

   static t_zone s_zone;
   static t_format s_format;
   static const int64_t s_empty;

   int isDst; //TR_HY85159
};

//========================================================================================
// Class Period
//========================================================================================

class Period
{
   friend std::ostream& operator<<(
         std::ostream& s,
         const Period& period
         );

public:
   Period();

   Period(
         const Time& first,
         const Time& last
         );

   Period(
         const std::string& startdate,
         const std::string& starttime,
         const std::string& stopdate,
         const std::string& stoptime
         );

   ~Period();

   Time first() const;
   Time last() const;
   bool empty() const;

private:
   Time m_first;
   Time m_last;
};

}

#endif // LTIME_H_
