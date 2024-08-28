//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rptask.h
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
//      Class for the file type logs.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      XDT/DEK (Long Nguyen)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-02-28   XLOBUNG     New version.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef RPTASK_H_
#define RPTASK_H_

#include "basetask.h"
#include "parameters.h"
#include <boost/filesystem/fstream.hpp>
#include <deque>

namespace fs = boost::filesystem;

namespace PES_CLH {
class RPType
{
    friend std::ostream& operator<<(std::ostream& s, const RPType& rptype);
public:
    // Constructor
    RPType();

    // Destructor
    ~RPType();

    // Close log
    void close();

    // Add file into list
    void addFilename(const std::string& file);

    // Add file size
    void addFileSize(uintmax_t size);

    // Sort file name by date
    void sortList();

    // Insert file
    void insert(
            const fs::path& path
            );

    // Set log directory
    void setLogDir(fs::path path);

    // Set parameters
    void setParameters(
            const std::string name,
            uintmax_t maxsize,
            uint64_t maxtime,
            uint16_t divider
            );

    // Get total file size
    uintmax_t getTotalFileSize();

private:
    typedef std::pair<Time, uint16_t> PAIR;
    typedef std::deque<std::string> FILELIST;
    typedef FILELIST::iterator FILELISTITER;
    typedef FILELIST::const_iterator FILELISTCITER;
    FILELIST m_filelist;
    uintmax_t m_currentsize;
    fs::path m_logdir;
    uintmax_t m_totalquota;
    uint64_t m_maxtime;
    uint16_t m_divider;
    std::string m_logname;

    FILELISTITER getDivIter();

    void maintainLogSize();

    // Parse a log subfile name
    static Time parseFileName(
          const std::string& file
          );
    static bool compareByDate(
          const std::string& file1,
          const std::string& file2
          );

    // Stream textual information about the log entry
    void stream(std::ostream& s) const;
};

class RPTask: public BaseTask
{
public:
   // Constructor
   RPTask();

   // Destructor
   virtual ~RPTask();

   // Act on log event
   void event(
         const fs::path& path
         );

   // Open log
   void open();

   // Close log
   void close();

   // Insert event log
   void insert(
         const fs::path& source            // RP file
         );

   // Get parent log path
   fs::path getParentDir() const {return "";}

    // Get a log directory
    fs::path getLogDir() const;

    // Create a log directory
    void createLogDir() const;

    // Get log parameters
    const BaseParameters& getParameters() const;

    // Read events
    Period readEvents(                    // Returns time for first and last event
            const Period&,                // Time period
            const Filter&,                // Search filter
            t_eventcb eventcb = 0,        // Event callback function
            std::ostream& os = std::cout  // Outstream
            ) const;

    // List time for first and last event
    Period listEvents(                    // Returns time for first and last event
          const Period&                   // Time period
          ) const;

    // Transfer log files for a CP or blade
   void transferLogs(
          const Period&,                  // Time period
          const Filter&                   // Search filter
          ) const;
protected:
   RPType m_rpdump;
   RPType m_rplog;

private:
   // Disable default copy constructor
   RPTask(const RPTask&);

   // Disable default assignment operator
   RPTask& operator=(const RPTask&);

   // Stream textual information about the log entry
   void stream(std::ostream& s) const;

   Parameters<e_rp> m_parameters;
};

}

#endif // RPTASK_H_
