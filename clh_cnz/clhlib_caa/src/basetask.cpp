//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      basetask.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2011. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Base class for the task classes.
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
//      PA1    2012-02-14   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "basetask.h"

using namespace std;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
BaseTask::BaseTask():
m_logsize(0),
m_isopen(false),
m_isnoncpub(false),
m_issetap2(false)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
BaseTask::~BaseTask()
{
}

//----------------------------------------------------------------------------------------
// Calculate log size - used for integrity check during testing
//----------------------------------------------------------------------------------------
size_t BaseTask::calculateLogSize() const
{
   size_t size(0);
   const fs::path& logdir = getLogDir();
   fs::directory_iterator end;
   for (fs::directory_iterator iter(logdir); iter != end; ++iter)
   {
      const fs::path& path = *iter;
      if (boost::regex_match(path.filename().c_str(), getParameters().getLogFile()))
      {
         fs::file_status stat(fs::symlink_status(path));
         if (fs::is_directory(stat))
         {
            // Directory
            for (fs::directory_iterator siter(path); siter != end; ++siter)
            {
               const fs::path& subfile = *siter;
               fs::file_status stat(fs::symlink_status(subfile));
               if (fs::is_regular_file(stat))
               {
                  size += fs::file_size(subfile);
               }
            }
         }
         else
         {
            // File
            size += fs::file_size(path);
         }
      }
   }
   return size;
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& s, const BaseTask& logtask)
{
   logtask.stream(s);
   return s;
}

//----------------------------------------------------------------------------------------
// Set value if this is the hanlder for Non-CPUB
//----------------------------------------------------------------------------------------
void BaseTask::setNonCPUB(bool noncpub)
{
   m_isnoncpub = noncpub;
}

//----------------------------------------------------------------------------------------
// Check if this is the handler for Non-CPUB
//----------------------------------------------------------------------------------------
bool BaseTask::isNonCPUB() const
{
   return m_isnoncpub;
}

//----------------------------------------------------------------------------------------
// Set value if this is the hanlder for Non-CPUB
//----------------------------------------------------------------------------------------
void BaseTask::setAP2()
{
   m_issetap2 = true;
}

//----------------------------------------------------------------------------------------
// Open for SEL AP2
//----------------------------------------------------------------------------------------
void BaseTask::openSELAP2()
{
   m_logsize = 0;
   m_isopen = true;
}

}
