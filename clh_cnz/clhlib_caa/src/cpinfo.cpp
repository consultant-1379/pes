//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cpinfo.cpp
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
//      Class for handling CP information.
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
//      PA1    2013-04-02   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include "common.h"
#include "cpinfo.h"
#include "exception.h"
#include "logger.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

using namespace std;

namespace PES_CLH {

//========================================================================================
//	Class CPInfo
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
CPInfo::CPInfo() :
m_cpId(~CPID()),
m_cpInstance(0)
{
   if (CPTable::isMultiCPSystem() == false)
   {
      m_cpId = 1001;
   }
}

CPInfo::CPInfo(CPID cpId, ACS_CS_API_CP* m_cpInstance) :
m_cpId(cpId),
m_cpInstance(m_cpInstance)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
CPInfo::~CPInfo()
{
}

//----------------------------------------------------------------------------------------
// Get CPID
//----------------------------------------------------------------------------------------
CPID CPInfo::getCPID() const
{
   return m_cpId;
}

//----------------------------------------------------------------------------------------
// Get CP name
//----------------------------------------------------------------------------------------
std::string CPInfo::getName() const
{
   if (CPTable::isMultiCPSystem())
   {
      ACS_CS_API_NS::CS_API_Result result;
      ACS_CS_API_Name cpname;
      char name[32];

      // Get CP name
      result = m_cpInstance->getCPName(m_cpId, cpname);
      if (result == ACS_CS_API_NS::Result_Success)
      {
         size_t size = 31;
         cpname.getName(name, size);
      }
      else
      {
         // Failed to obtain CP Info
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to obtain CP Info." << endl;
         ex << getErrorMessage(result);
         throw ex;
      }

      return boost::to_lower_copy(string(name));
   }
   else
   {
      return "";
   }
}

//----------------------------------------------------------------------------------------
// Get APZ system
//----------------------------------------------------------------------------------------
t_apzSystem CPInfo::getAPZSystem() const
{
   t_apzSystem apzsystem;
   if (CPTable::isMultiCPSystem())
   {
      // Multi CP system
      ACS_CS_API_NS::CS_API_Result result;
      uint16_t tapzsystem;
      result = m_cpInstance->getAPZSystem(m_cpId, tapzsystem);
      if (result != ACS_CS_API_NS::Result_Success)
      {
        if (result != ACS_CS_API_NS::Result_NoValue)
        {
            Exception ex(Exception::system(), WHERE__);
            ex << "Could not get APZ system." << endl;
            ex << getErrorMessage(result);
            throw ex;
        }
      }

      switch (tapzsystem)
      {
         case 21255: apzsystem = e_apz21255; break;
         case 21260: apzsystem = e_apz21260; break;
         case 21270: apzsystem = e_apz21270; break;
         case 21401: apzsystem = e_apz21401; break;
         case 21403: apzsystem = e_apz21403; break;
         case 21410: apzsystem = e_apz21410; break;
         case 21420: apzsystem = e_apz21420; break;
         default:
         {
            apzsystem = e_undefined;
            Logger logger(LOG_LEVEL_WARN);
            if (logger)
            {
               ostringstream s;
               s << "Unknown APZ system for CP '" << boost::to_upper_copy(getName());
               logger.event(WHERE__, s.str());
            }
            break;
         }
      }
   }
   else
   {
      // One CP system
      switch (Common::getCPType())
      {
         case 0:
         case 1: apzsystem = e_classic;  break;
         case 2: apzsystem = e_apz21240; break;
         case 3: apzsystem = e_apz21250; break;
         case 4: apzsystem = e_apz21255; break;
         default:
         {
            apzsystem = e_undefined;
            Logger logger(LOG_LEVEL_WARN);
            if (logger)
            {
               ostringstream s;
               s << "Undefined CP type.";
               logger.event(WHERE__, s.str());
            }
            break;
         }
      }

      // Check if it is running on vAPZ
      if (Common::getNodeArchitecture() == Common::VIRTUALIZED)
      {
         apzsystem = e_apz21270;
      }
   }

   return apzsystem;
}

//----------------------------------------------------------------------------------------
// Get error message
//----------------------------------------------------------------------------------------
string CPInfo::getErrorMessage(ACS_CS_API_NS::CS_API_Result result)
{
   switch (result)
   {
   case ACS_CS_API_NS::Result_Success:
      return "The request has succeeded.";
      break;

   case ACS_CS_API_NS::Result_NoEntry:
      return "There is no matching entry in the table.";
      break;

   case ACS_CS_API_NS::Result_NoValue:
      return "The entry exists in the table but doesn't have "
             "a value stored for the requested information.";
      break;

   case ACS_CS_API_NS::Result_NoAccess:
      return "Unable to access CS.";
      break;

   case ACS_CS_API_NS::Result_Failure:
      return "CS cannot fulfill the request because of an error.";
      break;

   default:
      return "Unknown error.";
   }
}

//----------------------------------------------------------------------------------------
// Get MAU Type
//----------------------------------------------------------------------------------------
t_mauType CPInfo::getMAUType() const
{
   ACS_CS_API_NS::CS_API_Result result;
   ACS_CS_API_NS::MauType mauType;
   ACS_CS_API_CommonBasedArchitecture::ArchitectureValue arcValue;

   // Check if the is VIRTUALIZED ENV
   result = ACS_CS_API_NetworkElement::getNodeArchitecture(arcValue);
   if (result == ACS_CS_API_NS::Result_Success)
   {
      if (arcValue == ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED)
      {
         // In Virtual APZ only MAUS type is applicable.
         return e_maus;
      }
   }

   if (CPTable::isMultiCPSystem())
   {
      result = m_cpInstance->getMauType(m_cpId, mauType);
   }
   else
   {
      ACS_CS_API_CP* cpInstance = ACS_CS_API::createCPInstance();
      if (cpInstance != NULL)
      {
         result = cpInstance->getMauType(m_cpId, mauType);
         ACS_CS_API::deleteCPInstance(cpInstance);
         cpInstance = NULL;
      }
   }
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         ostringstream s;
         s << "Unknown MAU Type for CP '" << boost::to_upper_copy(getName());
         s << "Set default to no MAU";
         logger.event(WHERE__, s.str());
      }
      return e_mauundefined;
   }
   else
   {
      switch (mauType)
      {
         case ACS_CS_API_NS::MAUB:
            return e_maub;
         case ACS_CS_API_NS::MAUS:
            return e_maus;
         default:
            return e_mauundefined;
      }
   }
}

//========================================================================================
// Class CPTable::const_iterator
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
CPTable::const_iterator::const_iterator():
m_iter(),
m_cpInfo()
{
}

CPTable::const_iterator::const_iterator(CPIDLISTCITER iter, ACS_CS_API_CP* m_cpInstance):
m_iter(iter),
m_cpInfo()
{
   m_cpInfo.m_cpInstance = m_cpInstance;
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
CPTable::const_iterator::~const_iterator()
{
}

//----------------------------------------------------------------------------------------
// Reference operator
//----------------------------------------------------------------------------------------
const CPInfo* CPTable::const_iterator::operator->()
{
   m_cpInfo.m_cpId = *m_iter;
   return &m_cpInfo;
}

//----------------------------------------------------------------------------------------
// Pointer operator
//----------------------------------------------------------------------------------------
const CPInfo& CPTable::const_iterator::operator*()
{
   m_cpInfo.m_cpId = *m_iter;
   return m_cpInfo;
}

//----------------------------------------------------------------------------------------
// Increment operator (prefix)
//----------------------------------------------------------------------------------------
CPTable::const_iterator& CPTable::const_iterator::operator++()
{
   ++m_iter;
   return *this;
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool CPTable::const_iterator::operator==(const const_iterator& iter) const
{
   return m_iter == iter.m_iter;
}

//----------------------------------------------------------------------------------------
// Unequality operator
//----------------------------------------------------------------------------------------
bool CPTable::const_iterator::operator!=(const const_iterator& iter) const
{
   return m_iter != iter.m_iter;
}


//========================================================================================
// Class CPTable
//========================================================================================

boost::tribool CPTable::s_multiCPSystem(boost::indeterminate);

//----------------------------------------------------------------------------------------
//   Constructors
//----------------------------------------------------------------------------------------
CPTable::CPTable():
m_cpInstance(),
m_cpIdList(),
m_instptr(0),
m_callback(0)
{
   if (isMultiCPSystem() == true)
   {
      // Initialize CP table
      init();
   }
}

CPTable::CPTable(void* instptr, t_callback callback):
m_cpInstance(),
m_cpIdList(),
m_instptr(instptr),
m_callback(callback)
{
   if (isMultiCPSystem() == true)
   {
      // Initialize CP table
      init();
   }
}

//----------------------------------------------------------------------------------------
//   Destructor
//----------------------------------------------------------------------------------------
CPTable::~CPTable()
{
   if (s_multiCPSystem == true)
   {
      ACS_CS_API::deleteCPInstance(m_cpInstance);
      m_cpInstance = NULL;
   }
}

//----------------------------------------------------------------------------------------
// Initialize CP table
//----------------------------------------------------------------------------------------
void CPTable::init()
{
   // Create CP instance
   m_cpInstance = ACS_CS_API::createCPInstance();
   if (m_cpInstance == 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to create CP instance.";
      throw ex;
   }

   // Get CP ID list
   ACS_CS_API_IdList cpIdList;
   ACS_CS_API_NS::CS_API_Result result = m_cpInstance->getCPList(cpIdList);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get CP list." << endl;
      ex << CPInfo::getErrorMessage(result);
      throw ex;
   }

   for (size_t index = 0; index < cpIdList.size(); ++index)
   {
      m_cpIdList.insert(cpIdList[index]);
   }
}

//----------------------------------------------------------------------------------------
// Notification that the HWC table has been updated
//----------------------------------------------------------------------------------------
void CPTable::update(const ACS_CS_API_CpTableChange& observer)
{
   ACS_CS_API_CpTableData *data = observer.cpData;
   bool needReset = false;
   CPID cpid_obs = data->cpId;
   uint16_t apzsys_obs = data->apzSystem;
   const_iterator citer = find(cpid_obs);
   ACS_CS_API_NS::MauType mauType_obs = data->mauType;
   ACS_CS_API_CommonBasedArchitecture::ArchitectureValue arcValue;

   if (citer != end())
   {
      CPInfo cpinfo = *citer;
      CPID cpid = cpinfo.getCPID();
      ACS_CS_API_NS::CS_API_Result result;
      uint16_t tapzsystem;
      result = m_cpInstance->getAPZSystem(cpid, tapzsystem);
      if (result != ACS_CS_API_NS::Result_Success)
      {
         if (result != ACS_CS_API_NS::Result_NoValue)
         {
            tapzsystem = 0;
         }
      }
      if (cpid == cpid_obs && tapzsystem != apzsys_obs)
      {
         needReset = true;
         char *buff = new char[1024];
         size_t size = sizeof(buff);

         data->cpName.getName(buff, size);

         Logger logger(LOG_LEVEL_INFO);
         if (logger)
         {
            ostringstream s;
            s << "APZ system for " << buff << " changed from " << tapzsystem << " to "<< apzsys_obs << endl;
            logger.event(WHERE__, s.str());
         }
      }
      // Check for MAU type change
      // Check if the is VIRTUALIZED ENV
      if (cpid >= 1000) // Dual-sided CP
      {
         result = ACS_CS_API_NetworkElement::getNodeArchitecture(arcValue);
         if (result == ACS_CS_API_NS::Result_Success)
         {
            // In Virtual APZ only MAUS type is applicable.
            if (arcValue != ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED)
            {
               ACS_CS_API_NS::MauType mauType;
               result = m_cpInstance->getMauType(cpid, mauType);
               if (mauType_obs != mauType)
               {
                  needReset = true;
                  char *buff = new char[1024];
                  size_t size = sizeof(buff);
         
                  data->cpName.getName(buff, size);
         
                  Logger logger(LOG_LEVEL_INFO);
                  if (logger)
                  {
                     ostringstream s;
                     s << "MAU type for " << buff << " changed from " << mauType << " to "<< mauType_obs << endl;
                     logger.event(WHERE__, s.str());
                  }
               }
            }
         }
      }
   }

   if (needReset)
   {
      // Execute callback function
      if (m_callback != 0)
      {
         m_callback(m_instptr);
      }
   }
}

//----------------------------------------------------------------------------------------
// Get CPInfo in a one CP system
//----------------------------------------------------------------------------------------
CPInfo CPTable::get() const
{
   if (s_multiCPSystem == true)
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Method not applicable for a multi CP system.";
      throw ex;
   }
   else
   {
      return CPInfo(1001, 0);
   }
}

//----------------------------------------------------------------------------------------
// Find CPInfo for CPID in a multi CP system
//----------------------------------------------------------------------------------------
CPTable::const_iterator CPTable::find(CPID cpId) const
{
   if (s_multiCPSystem == true)
   {
      return const_iterator(m_cpIdList.find(cpId), m_cpInstance);
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Method not applicable for a one CP system.";
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Find CPInfo for CP name in a multi CP system
//----------------------------------------------------------------------------------------
CPTable::const_iterator CPTable::find(const string& name) const
{
   if (s_multiCPSystem == true)
   {
      ACS_CS_API_NS::CS_API_Result result;
      CPID cpId;

      // Get CP name
      result = m_cpInstance->getCPId(boost::to_upper_copy(name).c_str(), cpId);
      if (result == ACS_CS_API_NS::Result_Success)
      {
         return const_iterator(m_cpIdList.find(cpId), m_cpInstance);
      }
      else if (result == ACS_CS_API_NS::Result_NoEntry)
      {
         return end();
      }
      else
      {
         // Failed to obtain CP Info
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to obtain CP Info." << endl;
         ex << CPInfo::getErrorMessage(result);
         throw ex;
      }
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Method not applicable for a one CP system.";
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Get first element in list
//----------------------------------------------------------------------------------------
CPTable::const_iterator CPTable::begin() const
{
   if (s_multiCPSystem == true)
   {
      return const_iterator(m_cpIdList.begin(), m_cpInstance);
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Method not applicable for a one CP system.";
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Get beyond last element in list
//----------------------------------------------------------------------------------------
CPTable::const_iterator CPTable::end() const
{
   if (s_multiCPSystem == true)
   {
      return const_iterator(m_cpIdList.end(), m_cpInstance);
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Method not applicable for a one CP system.";
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
//   Check if this is a multiple CP system
//----------------------------------------------------------------------------------------
bool CPTable::isMultiCPSystem()
{
   if (boost::indeterminate(s_multiCPSystem))
   {
      bool multiCPSystem;
      ACS_CS_API_NS::CS_API_Result result;
      result = ACS_CS_API_NetworkElement::isMultipleCPSystem(multiCPSystem);
      if (result != ACS_CS_API_NS::Result_Success)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Call to CS API failed." << endl;
         ex << CPInfo::getErrorMessage(result);
         throw ex;
      }
      s_multiCPSystem = multiCPSystem;
   }
   return s_multiCPSystem;
}

//----------------------------------------------------------------------------------------
// APZ system outstream operator 
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, t_apzSystem apzsystem)
{
   switch (apzsystem)
   {
   case e_classic:  s << "APZ Classic"; break;
   case e_apz21240: s << "APZ 212 40";  break;
   case e_apz21250: s << "APZ 212 50";  break;
   case e_apz21255: s << "APZ 212 55";  break;
   case e_apz21260: s << "APZ 212 60";  break;
   case e_apz21270: s << "APZ 212 70";  break;
   case e_apz21401: s << "APZ 214 01";  break;
   case e_apz21403: s << "APZ 214 03";  break;
   case e_apz21410: s << "APZ 214 10";  break;
   case e_apz21420: s << "APZ 214 20";  break;
   default:
      Exception ex(Exception::internal(), WHERE__);
      ex << "Unknown APZ system.";
      throw ex;
   };

   return s;
}

//----------------------------------------------------------------------------------------
// Reset the board cp
//----------------------------------------------------------------------------------------
void CPTable::reset()
{
   if (s_multiCPSystem == true)
   {
      // Delete CP instance
      ACS_CS_API::deleteCPInstance(m_cpInstance);
      m_cpInstance = NULL;

      // Clear board list
      m_cpIdList.clear();

      // Re-initiate CP list
      init();

      // Log event
      Logger logger(LOG_LEVEL_INFO);
      if (logger)
      {
         ostringstream s;
         s << "Reset CP Table successfully.";
         logger.event(WHERE__, s.str());
      }
   }
}

}
