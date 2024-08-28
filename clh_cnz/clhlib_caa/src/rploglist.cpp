//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      rploglist.cpp
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
//      Implementation for class that describing list of RP log file.
//      Most of the codes are based on rploglist.cpp on APG43
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

#include <algorithm>
#include <exception.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "parameters.h"
#include "rploglist.h"

namespace fs = boost::filesystem;

namespace PES_CLH
{


//========================================================================================
// Constructor
//========================================================================================
RPLogList::RPLogList(void)
{
}

//========================================================================================
// Destructor
//========================================================================================
RPLogList::~RPLogList(void)
{
}

void RPLogList::print(bool isOptDir)
{
    if (isOptDir)
    {
         printOptD();
    }
    else
    {
        printOptF();
    }
}

//========================================================================================
// Add RP log that match with the filter to the rplist
//========================================================================================
void RPLogList::listRpLogs(
        const std::string& rpNum,
        const Time& staTime,
        const Time& stoTime,
        bool isOptDir
)
{
    std::vector<std::string> fileList;
    std::vector<std::string>::iterator fileIter;

    listFiles(fileList);
    std::sort(fileList.begin(), fileList.end());

    if (fileList.empty() == false)
    {
        for (fileIter = fileList.begin(); fileIter != fileList.end(); fileIter++)
        {
            const string file = *fileIter;
            int pos = file.find_first_of("_", 3);
            string rpstr = file.substr(3, pos - 3);
            pos++;
            int pos2 = file.find_first_of("_", pos);
            string mag = file.substr(pos, pos2 - pos);
            pos = pos2 + 1;
            pos2 = file.find_first_of("_", pos);
            string slot = file.substr(pos, pos2 - pos);
            const size_t TimeStampLength = 15; // yyyymmdd_hhmmss
            size_t timeStampEnd = file.find_last_of("0123456789");

            if (timeStampEnd != std::string::npos &&
                    (timeStampEnd > TimeStampLength))
            {
               const string timeStamp = file.substr(timeStampEnd -
                       (TimeStampLength - 1), TimeStampLength);
               const string datestr = timeStamp.substr(0,8);// YYYYMMDD any number
               const string timestr = timeStamp.substr(9,6);// HHMMSS any number
               Time time;
               string logname;
               time.set(datestr, timestr, true);
               pos = file.find_last_of("_", timeStampEnd - TimeStampLength - 1);
               pos++;
               logname = file.substr(pos, timeStampEnd - TimeStampLength - pos);

               if (rpNum == "")
               {
                  if (time >= staTime && time < stoTime)
                  {
                     RPLogItem item(logname, mag, slot, rpstr, timeStamp, file);
                     //rplist.add(item, isOptDir);
                     add(item, isOptDir);
                  }
               }
               else
               {
                  if (time >= staTime && time < stoTime && rpNum == rpstr)
                  {
                     RPLogItem item(logname, mag, slot, rpstr, timeStamp, file);
                     add(item, isOptDir);
                  }
               }
            }

        }
    }
}

//========================================================================================
// Get RP log list
//========================================================================================
std::list<RPLogItem>& RPLogList::getRpLogList(void)
{
    return m_rpLogItems;
}

//========================================================================================
// Add one RP log file to the list
//========================================================================================
void RPLogList::add(const RPLogItem& item, bool isOptDir)
{
   list<RPLogItem>::iterator it;
   bool done = false;
   Time timeOfNewItem;
   Time startTime;
   Time stopTime;

   it = m_rpLogItems.begin();
   if (isOptDir)
   {
      while (it != m_rpLogItems.end())
      {
         if (it->getLogName() == item.getLogName() &&
                  it->getMag() == item.getMag() &&
                  it->getSlot() == item.getSlot() &&
                  it->getRPNo() == item.getRPNo())
         {
            timeOfNewItem.set(item.getStartTime().substr(0, 8), item.getStartTime().substr(9, 6), true);
            startTime.set(it->getStartTime().substr(0, 8), it->getStartTime().substr(9, 6), true);
            stopTime.set(it->getStopTime().substr(0, 8), it->getStopTime().substr(9, 6), true);

            if (timeOfNewItem < startTime)
            {
               it->setStartTime(item.getStartTime());
               done = true;
            }
            else
            {
               if (timeOfNewItem > stopTime)
               {
                  it->setStopTime(item.getStopTime());
                  done = true;
               }
               else
               {
                  if (timeOfNewItem > startTime && timeOfNewItem < stopTime)
                  {
                     done = true;
                  }
               }
            }
         }
         it++;
      }
   }

   if (!done)
   {
      m_rpLogItems.push_back(item);
   }
}

//========================================================================================
// Print with -d option
//========================================================================================
void RPLogList::printOptD()
{
   string strLine;
   char timestBuf[200];
   list<RPLogItem>::iterator it;

   sprintf(
            timestBuf,
            "%-15.15s%-9.9s%-6.6s%-6.6s%-17.15s%-15.15s",
            "Log type",
            "Mag",
            "Slot",
            "Rpno",
            "First time",
            "Last time");
   strLine = timestBuf;
   cout << strLine << endl;
   it = m_rpLogItems.begin();

   if (m_rpLogItems.size() == 0)
   {
       sprintf(
               timestBuf,
               "%-15.15s%-9.9s%-6.6s%-6.6s%-17.15s%-15.15s",
               "-",
               "-",
               "-",
               "-",
               "-",
               "-");
       strLine = timestBuf;
       cout << strLine << endl;
       return;
   }
   while (it != m_rpLogItems.end())
   {
       string tmp = it->getMag() + ".0.0.0";

       sprintf(
               timestBuf,
               "%-15.15s%-9.9s%-6.6s%-6.6s%-17.15s%-15.15s",
               it->getLogName().c_str(),
               tmp.c_str(),
               it->getSlot().c_str(),
               it->getRPNo().c_str(),
               it->getStartTime().c_str(),
               it->getStopTime().c_str());

       strLine = timestBuf;
       it++;
       cout << strLine << endl;
   }
}

//========================================================================================
// Print with -f option
//========================================================================================
void RPLogList::printOptF()
{
   string strLine;
   char timestBuf[200];
   list<RPLogItem >::iterator it;

   sprintf(
           timestBuf,
           "%-15.15s%-9.9s%-6.6s%-6.6s%-50s",
           "Log type",
           "Mag",
           "Slot",
           "Rpno",
           "Filename");
   strLine = timestBuf;
   cout << strLine << endl;
   it = m_rpLogItems.begin();

   if (m_rpLogItems.size() == 0)
   {
       sprintf(
               timestBuf,
               "%-15.15s%-9.9s%-6.6s%-6.6s%-50s",
               "-",
               "-",
               "-",
               "-",
               "-");
       strLine = timestBuf;
       cout << strLine << endl;
       return;
   }
   while (it != m_rpLogItems.end())
   {
       string tmp = it->getMag() + ".0.0.0";

       sprintf(
               timestBuf,
               "%-15.15s%-9.9s%-6.6s%-6.6s%-50s",
               it->getLogName().c_str(),
               tmp.c_str(),
               it->getSlot().c_str(),
               it->getRPNo().c_str(),
               it->getFileName().c_str());

       strLine = timestBuf;
       it++;
       cout << strLine << endl;
   }
}

//========================================================================================
// Get all log files in RP directory
//========================================================================================
void RPLogList::listFiles(std::vector<std::string>& database)
{
   fs::path rpFolder("RP/");
   const fs::path& rpDir = BaseParameters::getApzLogsPath() / rpFolder;
   fs::directory_iterator end_iter;

   const boost::regex rpPattern("RP_\\d{1,4}_\\d{1,2}_\\d{1,2}_\\d{8}_\\d{6}_([a-zA-Z0-9]+)._\\d{8}_\\d{6}(.txt|.bin)");
   if (!fs::exists(rpDir))
   {
      Exception ex(Exception::system(), WHERE__);
      //ex << *this << endl;
      ex << "Failed to access to folder " << rpDir<< ".";
      ex.sysError();
      throw ex;
   }

   // Check if it is a folder
   if (fs::is_directory(rpDir))
   {
      // iterate all elements in the folder
      for (fs::directory_iterator dir_iter(rpDir); dir_iter!= end_iter; dir_iter++)
      {
         const fs::path& path = *dir_iter;
         const string& name = path.filename().c_str();
         // Get all files that match fully with the pattern
         if (fs::is_regular_file(dir_iter->status()) && regex_match(name, rpPattern))
         {
             // save the file name into the database
             database.push_back(name);
         }
      }
   }
}
}
