//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      boardinfo.cpp
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
//      Class for handling board information.
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

#include "boardinfo.h"
#include "exception.h"
#include "logger.h"
#include <boost/lexical_cast.hpp>

using namespace std;

namespace PES_CLH
{

//=======================================================================================-
// Class Magazine
//=======================================================================================-

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Magazine::Magazine() :
m_id(~uint32_t())
{
}

Magazine::Magazine(uint32_t id) :
m_id(id)
{
}

Magazine::Magazine(const std::string& magazine) :
m_id(~uint32_t())
{
   try
   {
      size_t pos(0);
      size_t pos1(0);
      for (int i = 0; i < 4; i++)
      {
         if (pos1 == string::npos)
         {
            throw Exception(Exception::illSubrack(magazine), WHERE__);
         }

         pos1 = magazine.find('.', pos);
         string subaddr = magazine.substr(pos, pos1 - pos);
         uint16_t plug = boost::lexical_cast<uint16_t>(subaddr);
         if (plug > 15)
         {
            throw Exception(Exception::illSubrack(magazine), WHERE__);
         }
         m_plug[i] = plug;
         pos = pos1 + 1;
      }
      if (pos1 != string::npos)
      {
         throw Exception(Exception::illSubrack(magazine), WHERE__);
      }
   }
   catch (exception& e)
   {
      throw Exception(Exception::illSubrack(magazine), WHERE__);
   }
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Magazine::~Magazine()
{
}

//----------------------------------------------------------------------------------------
// Magazine address operators
//----------------------------------------------------------------------------------------
Magazine::operator uint32_t() const
{
   return m_id;
}

Magazine::operator string() const
{
   ostringstream s;
   s << uint16_t(m_plug[0]) << "." << uint16_t(m_plug[1]) << "."
     << uint16_t(m_plug[2]) << "." << uint16_t(m_plug[3]);
   return s.str();
}

//----------------------------------------------------------------------------------------
// Address plug operator
//----------------------------------------------------------------------------------------
int Magazine::operator[](uint32_t index) const
{
   if (index < 4)
   {
      return m_plug[index];
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Illegal address plug index.";
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Magazine& magazine)
{
   s << magazine.operator string();
   return s;
}

//=======================================================================================-
// Class Slot
//=======================================================================================-

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Slot::Slot() :
m_id(~uint16_t())
{
}

Slot::Slot(uint16_t id) :
m_id(id)
{
}

Slot::Slot(const std::string& slot) :
   m_id(~uint16_t())
{
   try
   {
      m_id = boost::lexical_cast<uint16_t>(slot);
      if ((m_id > 28) || (m_id == 27))
      {
         throw Exception(Exception::illSlot(slot), WHERE__);
      }
   }
   catch (exception& e)
   {
      throw Exception(Exception::illSlot(slot), WHERE__);
   }
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Slot::~Slot()
{
}

//----------------------------------------------------------------------------------------
// Slot position operator
//----------------------------------------------------------------------------------------
Slot::operator uint16_t() const
{
   return m_id;
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Slot& slot)
{
   s << slot.m_id;
   return s;
}

//=======================================================================================-
// Class BoardInfo
//=======================================================================================-

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
BoardInfo::BoardInfo() :
m_boardId(~BoardID()),
m_hwcInstance(0)
{
}

BoardInfo::BoardInfo(BoardID boardId, ACS_CS_API_HWC* hwcTable) :
m_boardId(boardId),
m_hwcInstance(hwcTable)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
BoardInfo::~BoardInfo()
{
}

//----------------------------------------------------------------------------------------
// Get magazine address
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getMagazine() const
{
   uint32_t magazine;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getMagazine(magazine, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get magazine address." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return magazine;
}

//----------------------------------------------------------------------------------------
// Get slot position
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getSlot() const
{
   uint16_t slot;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getSlot(slot, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get slot position." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return slot;
}

//----------------------------------------------------------------------------------------
// Get HWC system type
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getSysType() const
{
   uint16_t sysType;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getSysType(sysType, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get HWC system type." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return sysType;
}

//----------------------------------------------------------------------------------------
// Get system number
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getSysNo() const
{
   uint16_t sysNo;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getSysNo(sysNo, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get system number." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return sysNo;
}

//----------------------------------------------------------------------------------------
// Get HWC Functional Board Name (FBN) Identifier
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getFBN() const
{
   uint16_t fbn;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getFBN(fbn, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get HWC Functional Board Name (FBN) Identifier." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return fbn;
}

//----------------------------------------------------------------------------------------
// Get CP side
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getSide() const
{
   uint16_t side;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getSide(side, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get CP side." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return side;
}

//----------------------------------------------------------------------------------------
// Get sequence number
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getSeqNo() const
{
   uint16_t seqNo;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getSeqNo(seqNo, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get sequence number." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return seqNo;
}

//----------------------------------------------------------------------------------------
// Get IP address on backplane A
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getIPEthA() const
{
   uint32_t ip;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getIPEthA(ip, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get IP address on backplane A." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return ip;
}

//----------------------------------------------------------------------------------------
// Get IP address on backplane B
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getIPEthB() const
{
   uint32_t ip;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getIPEthB(ip, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get IP address on backplane B." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return ip;
}

//----------------------------------------------------------------------------------------
// Get alias IP address on backplane A
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getAliasEthA() const
{
   uint32_t ip;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getAliasEthA(ip, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get alias IP address on backplane A." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return ip;
}

//----------------------------------------------------------------------------------------
// Get alias IP address on backplane B
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getAliasEthB() const
{
   uint32_t ip;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getAliasEthB(ip, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get alias IP address on backplane B." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return ip;
}

//----------------------------------------------------------------------------------------
// Get netmask for IP alias address on backplane A
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getAliasNetmaskEthA() const
{
   uint32_t mask;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getAliasNetmaskEthA(mask, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get netmask for IP alias address on backplane A." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return mask;
}

//----------------------------------------------------------------------------------------
// Get netmask for IP alias address on backplane B
//----------------------------------------------------------------------------------------
uint32_t BoardInfo::getAliasNetmaskEthB() const
{
   uint32_t mask;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getAliasNetmaskEthB(mask, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get netmask for IP alias address on backplane B." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return mask;
}

//----------------------------------------------------------------------------------------
// Get HWC DHCP Method
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getDhcpMethod() const
{
   uint16_t dhcp;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getDhcpMethod(dhcp, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get HWC DHCP Method." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return dhcp;
}

//----------------------------------------------------------------------------------------
// Get system identity
//----------------------------------------------------------------------------------------
uint16_t BoardInfo::getSysId() const
{
   uint16_t sysId;
   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getSysId(sysId, m_boardId);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to get system identity." << endl;
      ex << getErrorMessage(result);
      throw ex;
   }

   return sysId;
}

//----------------------------------------------------------------------------------------
// Get error message
//----------------------------------------------------------------------------------------
string BoardInfo::getErrorMessage(ACS_CS_API_NS::CS_API_Result result)
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

//========================================================================================
// Class BoardTable::const_iterator
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
BoardTable::const_iterator::const_iterator():
m_iter(),
m_boardInfo()
{
}

BoardTable::const_iterator::const_iterator(BOARDIDLISTCITER iter, ACS_CS_API_HWC* hwcInstance):
m_iter(iter),
m_boardInfo()
{
   m_boardInfo.m_hwcInstance = hwcInstance;
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
BoardTable::const_iterator::~const_iterator()
{
}

//----------------------------------------------------------------------------------------
// Reference operator
//----------------------------------------------------------------------------------------
const BoardInfo* BoardTable::const_iterator::operator->()
{
   m_boardInfo.m_boardId = *m_iter;
   return &m_boardInfo;
}

//----------------------------------------------------------------------------------------
// Pointer operator
//----------------------------------------------------------------------------------------
const BoardInfo& BoardTable::const_iterator::operator*()
{
   m_boardInfo.m_boardId = *m_iter;
   return m_boardInfo;
}

//----------------------------------------------------------------------------------------
// Increment operator (prefix)
//----------------------------------------------------------------------------------------
BoardTable::const_iterator& BoardTable::const_iterator::operator++()
{
   ++m_iter;
   return *this;
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool BoardTable::const_iterator::operator==(const const_iterator& iter) const
{
   return m_iter == iter.m_iter;
}

//----------------------------------------------------------------------------------------
// Unequality operator
//----------------------------------------------------------------------------------------
bool BoardTable::const_iterator::operator!=(const const_iterator& iter) const
{
   return m_iter != iter.m_iter;
}


//=======================================================================================-
// Class BoardTable
//=======================================================================================-

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
BoardTable::BoardTable():
m_hwcInstance(0),
m_boardIdList(),
m_instptr(0),
m_callback(0)
{
   // Initialize board table
   init();
}

BoardTable::BoardTable(void* instptr, t_callback callback):
m_hwcInstance(0),
m_boardIdList(),
m_instptr(instptr),
m_callback(callback)
{
   // Initialize board table
   init();
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
BoardTable::~BoardTable()
{
   ACS_CS_API::deleteHWCInstance(m_hwcInstance);
   m_hwcInstance = NULL;
}

//----------------------------------------------------------------------------------------
// Initialize board table
//----------------------------------------------------------------------------------------
void BoardTable::init()
{
   ACS_CS_API_NS::CS_API_Result result;

   // Get HWC table information
   m_hwcInstance = ACS_CS_API::createHWCInstance();
   if (m_hwcInstance == 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to create HWC instance.";
      throw ex;
   }

   // Create search instance
   ACS_CS_API_BoardSearch* const boardSearch =
         ACS_CS_API_HWC::createBoardSearchInstance();
   if (boardSearch == 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to create board search instance.";
      throw ex;
   }

   // Get list of board id:s
   ACS_CS_API_IdList boardList;
   result = m_hwcInstance->getBoardIds(boardList, boardSearch);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);

      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to search for boards." << endl;
      ex << BoardInfo::getErrorMessage(result);
      throw ex;
   }

   for (size_t index = 0; index < boardList.size(); ++index)
   {
      m_boardIdList.insert(boardList[index]);
   }

   // Delete search instance
   ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);
}

//----------------------------------------------------------------------------------------
// Notification that the HWC table has been updated
//----------------------------------------------------------------------------------------
void BoardTable::update(const ACS_CS_API_HWCTableChange&)
{
   // Log event
   Logger logger(LOG_LEVEL_INFO);
   if (logger)
   {
      ostringstream s;
      s << "The hardware table was updated.";
      logger.event(WHERE__, s.str());
   }
   
   // Execute callback function
   if (m_callback != 0)
   {
      m_callback(m_instptr);
   }
}

//----------------------------------------------------------------------------------------
// Find board info for magazine address id and slot position
//----------------------------------------------------------------------------------------
BoardTable::const_iterator BoardTable::find(
                              const Magazine& magazine,
                              const Slot& slot
                              ) const
{
   // Create search instance
   ACS_CS_API_BoardSearch* const boardSearch =
            ACS_CS_API_HWC::createBoardSearchInstance();
   if (boardSearch == 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to create board search instance.";
      throw ex;
   }

   // Define the search criteria
   ACS_CS_API_IdList boardList;
   boardSearch->reset();
   boardSearch->setMagazine(magazine);    // Set magazine address
   boardSearch->setSlot(slot);            // Set slot position

   ACS_CS_API_NS::CS_API_Result result;
   result = m_hwcInstance->getBoardIds(boardList, boardSearch);
   if (result != ACS_CS_API_NS::Result_Success)
   {
      ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);

      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to search for boards." << endl;
      ex << BoardInfo::getErrorMessage(result);
      throw ex;
   }

   const_iterator iter;
   switch (boardList.size())
   {
      case 0:
         iter = end();
         break;

      case 1:
         iter = const_iterator(m_boardIdList.find(boardList[0]), m_hwcInstance);
         break;

      default:
      {
         ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);

         Exception ex(Exception::system(), WHERE__);
         ex << "Table of boards is inconsistent.";
         throw ex;
      }
   }

   // Delete search instance
   ACS_CS_API_HWC::deleteBoardSearchInstance(boardSearch);

   return iter;
}

//----------------------------------------------------------------------------------------
// Get first element in list
//----------------------------------------------------------------------------------------
BoardTable::const_iterator BoardTable::begin() const
{
   return const_iterator(m_boardIdList.begin(), m_hwcInstance);
}

//----------------------------------------------------------------------------------------
// Get beyond last element in list
//----------------------------------------------------------------------------------------
BoardTable::const_iterator BoardTable::end() const
{
   return const_iterator(m_boardIdList.end(), m_hwcInstance);
}

//----------------------------------------------------------------------------------------
// Reset the board list
//----------------------------------------------------------------------------------------
void BoardTable::reset()
{
   // Delete HW instance
   ACS_CS_API::deleteHWCInstance(m_hwcInstance);
   m_hwcInstance = NULL;

   // Clear board list
   m_boardIdList.clear();

   // Re-initiate board list
   init();

   // Log event
   Logger logger(LOG_LEVEL_INFO);
   if (logger)
   {
      ostringstream s;
      s << "Reset HWC Table successfully.";
      logger.event(WHERE__, s.str());
   }
}

}
