//----------------------------------------------------------------------------------------
//
//  FILE
//      pes_clhapi.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012. All rights reserved.
//
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for for listing and transferring CPS trace logs.
//
//  ERROR HANDLING
//      -
//
//  DOCUMENT NO
//      190 89-CAA 109 1426  PA1
//
//  AUTHOR
//      EAB/FLE/EM UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2012-10-05   UABTSO      New API created for listing and transferring
//                                      CPS trace logs.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------

#ifndef PES_CLHAPI_H_
#define PES_CLHAPI_H_

#include <string>
#include <string.h>
#include <stdlib.h>

enum desttype_t
{
   e_file,                      // Log transferred to file
   e_media                      // Log transferred to media
};

namespace PES_CLH {class Api_impl;}

class Pes_clhapi
{
public:

   // Constructor
   Pes_clhapi();

   // Destructor
   ~Pes_clhapi();

   // Callback for log messages
   typedef void (*t_eventcb)(
         const char* cptime,               // CP time, format is "YYYYMMDD_HHmmss_uuuuuu"
         const char* aptime,               // AP time, format is "YYYYMMDD_HHmmss"
         size_t size,                      // Message size
         const char* evmsg                 // Log message
         );

   // Read trace log, receive events in the callback, the call is blocked until all data
   // has been received 
   int readLog(                           // Returns 0 for successful operation. See description for
                                          // error codes for possible faults.
         const std::string& cpname,       // CP name, for a one CP system an empty string is provided
         const std::string& cpside,       // CP side, "A" or "B" for a one CP system or SPX and an
                                          // empty string for a single sided CP (blade). The deprecated
                                          // "CPA" and "CPB" are kept for backward compatibility.
         std::string& startdate,          // Start date, returns date for first log greater than start date.
         std::string& starttime,          // Start time, returns time for first log greater than start time.
         std::string& stopdate,           // Stop date, returns date for first log less than stop date.
         std::string& stoptime,           // Stop time, returns time for first log less than stop time.
         t_eventcb eventcb   = 0          // Call back method. If eventcb value is 0, no callback will be
                                          // performed.
         ) const
   {
      char* tstartdate = strdup(startdate.c_str());
      char* tstarttime = strdup(starttime.c_str());
      char* tstopdate = strdup(stopdate.c_str());
      char* tstoptime = strdup(stoptime.c_str());

      int res = readLog_p(
                  cpname.c_str(),
                  cpside.c_str(),
                  tstartdate,
                  tstarttime,
                  tstopdate,
                  tstoptime,
                  eventcb
                  );

      startdate = tstartdate;
      starttime = tstarttime;
      stopdate = tstopdate;
      stoptime = tstoptime;

      free(tstartdate);
      free(tstarttime);
      free(tstopdate);
      free(tstoptime);

      return res;
   }

   // Transfer logs to a destination
   int transferLogs(                      // Returns 0 for successful operation. See error handling
                                          // for possible error codes.
         const std::string& cpname,       // CP name, in a one CP system an empty string is provided.
                                          // If CP name is empty in a multi CP system, logs for all
                                          // CP:s and blades will be transferred.
         const std::string& startdate,    // Start date
         const std::string& starttime,    // Start time
         const std::string& stopdate,     // Stop date
         const std::string& stoptime,     // Stop time
          desttype_t desttype             // Destination type.
         ) const
   {
      return transferLogs_p(
                           cpname.c_str(),
                           startdate.c_str(),
                           starttime.c_str(),
                           stopdate.c_str(),
                           stoptime.c_str(),
                           desttype
                           );
   }

   // Following table shows legal string values for the start and stop dates/times
   // and the resulting start and stop times

   // +============+============+============================+
   // | Start date | Start time | Resulting start date/time  |
   // +------------+------------+----------------------------+
   // | "YYYYMMDD" | "HHmm"     | YYYY-MM-DD HH:mm:00.000000 |
   // +------------+------------+----------------------------+
   // | ""         | "HHmm"     |      Today HH:mm:00.000000 |
   // +------------+------------+----------------------------+
   // | "YYYYMMDD" | ""         | YYYY-MM-DD 00:00:00.000000 |
   // +------------+------------+----------------------------+
   // | ""         | ""         | 1601-01-01 00:00:00.000000 |
   // +============+============+============================+
   // | Stop date  | Stop time  | Resulting stop date/time   |
   // +------------+------------+----------------------------+
   // | "YYYYMMDD" | "HHmm"     | YYYY-MM-DD HH:mm:59.999999 |
   // +------------+------------+----------------------------+
   // | ""         | "HHmm"     |      Today HH:mm:59.999999 |
   // +------------+------------+----------------------------+
   // | "YYYYMMDD" | ""         | YYYY-MM-DD 23:59:59.999999 |
   // +------------+------------+----------------------------+
   // | ""         | ""         | 9999-12-31 23:59:59.999999 |
   // +============+============+============================+

   // Get error text for previous call
   std::string getErrorText() const
   {
      return getErrorText_p();
   }

   // Error handling:
   // Error code   Description
   //   0          Successful execution.
   //  19          Start time greater than stop time.
   //  20          Illegal date value
   //  21          Illegal time value
   //  22          Illegal CP side.
   //  23          CP side not allowed.
   //  24          Illegal transfer type.
   //  33          Not able to transfer to DVD.
   //  35          No file to compress.
   // 100          Parameter error
   // 101          System error
   // 102          Internal error
   // 118          CP is not defined.

private:
   Pes_clhapi(const Pes_clhapi&);
   Pes_clhapi& operator=(const Pes_clhapi&);

   int readLog_p(
         const char* cpname,
         const char* cpside,
         char* &startdate,
         char* &starttime,
         char* &stopdate,
         char* &stoptime,
         Pes_clhapi::t_eventcb eventcb
         ) const;

   int transferLogs_p(
         const char* cpname,
         const char* startdate,
         const char* starttime,
         const char* stopdate,
         const char* stoptime,
         desttype_t desttype
         ) const;

   const char* getErrorText_p() const;

   PES_CLH::Api_impl* m_apiptr;           // Pointer to implementation class
};

#endif // PES_CLHAPI_H_
