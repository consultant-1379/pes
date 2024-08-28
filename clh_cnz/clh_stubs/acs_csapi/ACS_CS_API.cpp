#include "ACS_CS_API.h"
#include "acs_apgcc_paramhandling.h"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <algorithm>
#include <errno.h>
#include <string.h>
#include <assert.h>

using namespace std;

//========================================================================================
// Class ACS_CS_API_IdList
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
ACS_CS_API_IdList_R1::ACS_CS_API_IdList_R1():
m_idp(0),
m_size(0)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
ACS_CS_API_IdList_R1::~ACS_CS_API_IdList_R1()
{
   if (m_idp)
   {
      delete[] m_idp;
   }
}

//----------------------------------------------------------------------------------------
// [] operator
//----------------------------------------------------------------------------------------
uint16_t ACS_CS_API_IdList_R1::operator[](const size_t index) const
{
	assert(index < m_size);
	return m_idp[index];
}

//----------------------------------------------------------------------------------------
// Get size of ID list
//----------------------------------------------------------------------------------------
size_t ACS_CS_API_IdList_R1::size() const
{
   return m_size;
}

//========================================================================================
// Class ACS_CS_API_CP
//========================================================================================

const ACS_CS_API_CP_R1 ACS_CS_API_CP_R1::s_cpList[] =
{
	// The CP:s and blades are hard-coded
	ACS_CS_API_CP_R1(0, 		21403, 21260),		// BC0
	ACS_CS_API_CP_R1(1, 		21403, 21260),		// BC1
	ACS_CS_API_CP_R1(1001,	21255, 21255),		// CP1
	ACS_CS_API_CP_R1(1002,	21260, 21260)		// CP2
};

const uint16_t ACS_CS_API_CP_R1::s_size =
   sizeof(ACS_CS_API_CP_R1::s_cpList)/sizeof(ACS_CS_API_CP_R1);

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
ACS_CS_API_CP_R1::ACS_CS_API_CP_R1():
m_cpid(static_cast<CPID>(-1)),
m_name(),
m_system(0),
m_type(0)
{
}

ACS_CS_API_CP_R1::ACS_CS_API_CP_R1(
			CPID cpid,
			uint16_t system,
			uint16_t type
			):
m_cpid(cpid),
m_name(),
m_system(system),
m_type(type)
{
	ACS_CS_API_Name_R1 name;
	ACS_CS_API_NS::CS_API_Result result;
	result = ACS_CS_API_NetworkElement_R1::getDefaultCPName(cpid, name);
	assert(result == ACS_CS_API_NS::Result_Success);
	char buf[16];
	size_t size = 15;
	name.getName(buf, size); 
	m_name.assign(buf);
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
ACS_CS_API_CP_R1::~ACS_CS_API_CP_R1()
{
}

//----------------------------------------------------------------------------------------
// Get CP identity
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_CP_R1::getCPId(
			const ACS_CS_API_Name& name,
			CPID& cpid
			)
{
	size_t size = name.length();
	char* buf = new char[size + 1];
	name.getName(buf, size);
	string tname(buf, size);
	boost::to_upper(tname);
	delete[] buf;

   for (int i = 0; i < s_size; i++)
	{
		if (tname == s_cpList[i].m_name)
		{
			cpid = s_cpList[i].m_cpid;
			return ACS_CS_API_NS::Result_Success;
		}
	}
  
   return ACS_CS_API_NS::Result_NoEntry;
}

//----------------------------------------------------------------------------------------
// Get CP name
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_CP_R1::getCPName(
			CPID cpid, 
			ACS_CS_API_Name& name
			)
{
	const ACS_CS_API_CP_R1* object(0);
   ACS_CS_API_NS::CS_API_Result result = getCPObject(cpid, object);
	if (result == ACS_CS_API_NS::Result_Success)
	{
		name.setName(object->m_name.c_str());
	}
  
   return result;
}

//----------------------------------------------------------------------------------------
// Get list of CP identities
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_CP_R1::getCPList(
				ACS_CS_API_IdList_R1& cpIdList
				)
{
   cpIdList.m_idp = new CPID[s_size];
   for (int i = 0; i < s_size; i++)
   {
      cpIdList.m_idp[i] = s_cpList[i].m_cpid;
   }
   cpIdList.m_size = s_size;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get APZ system
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_CP_R1::getAPZSystem(
			CPID cpid,
			uint16_t& system
			)
{
   const ACS_CS_API_CP_R1* object(0);
   ACS_CS_API_NS::CS_API_Result result = getCPObject(cpid, object);
	if (result == ACS_CS_API_NS::Result_Success)
	{
		system = object->m_system;
	}

	return result;
}

//----------------------------------------------------------------------------------------
// Get CP type
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_CP_R1::getCPType(
			CPID cpid,
			uint16_t& type
			)
{
   const ACS_CS_API_CP_R1* object(0);
   ACS_CS_API_NS::CS_API_Result result = getCPObject(cpid, object);
	if (result == ACS_CS_API_NS::Result_Success)
	{
		type = object->m_type;
	}

	return result;
}

//----------------------------------------------------------------------------------------
// Get CP object
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_CP_R1::getCPObject(
			CPID cpid,
			const ACS_CS_API_CP_R1* &object
			) const
{
	for (int i = 0; i < s_size; i++)
	{
		if (cpid == s_cpList[i].m_cpid)
		{
			object = &s_cpList[i];
			return ACS_CS_API_NS::Result_Success;
		}
	}
  
   return ACS_CS_API_NS::Result_NoEntry;
}

//========================================================================================
// Class ACS_CS_API_HWC
//========================================================================================
const ACS_CS_API_HWC_R1 ACS_CS_API_HWC_R1::s_boardIdList[] =
{
	ACS_CS_API_HWC_R1(
		0x05000402,
		5, 
		ACS_CS_API_HWC_NS::SysType_BC,
		1,
		ACS_CS_API_HWC_NS::FBN_CPUB,
		ACS_CS_API_HWC_NS::Side_A,
		0
		),

	ACS_CS_API_HWC_R1(
		0x05000402,
		6, 
		ACS_CS_API_HWC_NS::SysType_BC,
		1,
		ACS_CS_API_HWC_NS::FBN_CPUB,
		ACS_CS_API_HWC_NS::Side_A,
		1
		),

	ACS_CS_API_HWC_R1(
		0x05000402,
		8, 
		ACS_CS_API_HWC_NS::SysType_CP,
		1,
		ACS_CS_API_HWC_NS::FBN_CPUB,
		ACS_CS_API_HWC_NS::Side_A,
		1001
		),

	ACS_CS_API_HWC_R1(
		0x05000402,
		9,
		ACS_CS_API_HWC_NS::SysType_CP,
		1,
		ACS_CS_API_HWC_NS::FBN_CPUB,
		ACS_CS_API_HWC_NS::Side_B,
		1001
		),

	ACS_CS_API_HWC_R1(
		0x05000402,
		10,
		ACS_CS_API_HWC_NS::SysType_CP,
		1,
		ACS_CS_API_HWC_NS::FBN_CPUB,
		ACS_CS_API_HWC_NS::Side_A,
		1002
		),

	ACS_CS_API_HWC_R1(
		0x05000402,
		11,
		ACS_CS_API_HWC_NS::SysType_CP,
		1,
		ACS_CS_API_HWC_NS::FBN_CPUB,
		ACS_CS_API_HWC_NS::Side_B,
		1002
		)
};

const uint16_t ACS_CS_API_HWC_R1::s_size =
	sizeof(ACS_CS_API_HWC_R1::s_boardIdList)/sizeof(ACS_CS_API_HWC);

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
ACS_CS_API_HWC_R1::ACS_CS_API_HWC_R1():
m_magazine(static_cast<uint32_t>(-1)),
m_slot(static_cast<uint16_t>(-1)),
m_systype(static_cast<ACS_CS_API_HWC_NS::HWC_SystemType_Identifier>(-1)),
m_sysno(static_cast<uint16_t>(-1)),
m_fbn(static_cast<ACS_CS_API_HWC_NS::HWC_FBN_Identifier>(-1)),
m_side(static_cast<ACS_CS_API_HWC_NS::HWC_Side_Identifier>(-1)),
m_sysid(static_cast<uint16_t>(-1))
{
}

ACS_CS_API_HWC_R1::ACS_CS_API_HWC_R1(
			uint32_t magazine,
			uint16_t slot,
			uint16_t systype,
			uint16_t sysno,
			uint16_t fbn,
			uint16_t side,
			uint16_t sysid
			):
m_magazine(magazine),
m_slot(slot),
m_systype(static_cast<ACS_CS_API_HWC_NS::HWC_SystemType_Identifier>(systype)),
m_sysno(sysno),
m_fbn(static_cast<ACS_CS_API_HWC_NS::HWC_FBN_Identifier>(fbn)),
m_side(static_cast<ACS_CS_API_HWC_NS::HWC_Side_Identifier>(side)),
m_sysid(static_cast<uint16_t>(sysid))
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
ACS_CS_API_HWC_R1::~ACS_CS_API_HWC_R1()
{
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool ACS_CS_API_HWC_R1::operator==(const ACS_CS_API_HWC_R1& hwcTable) const
{
	bool equal(true);
	equal &= (m_magazine == hwcTable.m_magazine) || 
			   (hwcTable.m_magazine == static_cast<uint32_t>(-1));
	equal &= (m_slot == hwcTable.m_slot) || 
			   (hwcTable.m_slot == static_cast<uint16_t>(-1));
	equal &= (m_systype == hwcTable.m_systype) || 
			   (hwcTable.m_systype == static_cast<ACS_CS_API_HWC_NS::HWC_SystemType_Identifier>(-1));
	equal &= (m_sysno == hwcTable.m_sysno) || 
			   (hwcTable.m_sysno == static_cast<uint16_t>(-1));
	equal &= (m_fbn == hwcTable.m_fbn) || 
			   (hwcTable.m_fbn == static_cast<ACS_CS_API_HWC_NS::HWC_FBN_Identifier>(-1));
	equal &= (m_side == hwcTable.m_side) || 
			   (hwcTable.m_side == static_cast<ACS_CS_API_HWC_NS::HWC_Side_Identifier>(-1));
	equal &= (m_sysid == hwcTable.m_sysid) || 
			   (hwcTable.m_sysid == static_cast<uint16_t>(-1));

	return equal;
}

//----------------------------------------------------------------------------------------
// Create a board search instance
//----------------------------------------------------------------------------------------
ACS_CS_API_BoardSearch_R1* ACS_CS_API_HWC_R1::createBoardSearchInstance()
{
   return new ACS_CS_API_BoardSearch;
}

//----------------------------------------------------------------------------------------
// Delete a board search instance
//----------------------------------------------------------------------------------------
void ACS_CS_API_HWC_R1::deleteBoardSearchInstance(
			ACS_CS_API_BoardSearch_R1* instance
			)
{
   delete instance;
}

//----------------------------------------------------------------------------------------
// Get a list of board Id:s
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getBoardIds(
			ACS_CS_API_IdList_R1& listObj,
			ACS_CS_API_BoardSearch_R1* searchObj
			)
{
	listObj.m_idp = new BoardID[s_size];
	unsigned int size = 0;
	for (BoardID i = 0; i < s_size; i++)
	{
		if (s_boardIdList[i] == searchObj->m_hwcTable)
		{
			listObj.m_idp[size] = i;
			size++;
		}
	}
	listObj.m_size = size;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get magazine
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getMagazine(
			uint32_t& magazine, 
			BoardID boardId
			)
{
   magazine = s_boardIdList[boardId].m_magazine;
   if (magazine != static_cast<uint32_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//----------------------------------------------------------------------------------------
// Get slot
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getSlot(
			uint16_t& slot,
			BoardID boardId
			)
{
   slot = s_boardIdList[boardId].m_slot;
   if (slot != static_cast<uint16_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//----------------------------------------------------------------------------------------
// Get system type
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getSysType(
			uint16_t& systype, 
			BoardID boardId
			)
{
   systype = static_cast<uint16_t>(s_boardIdList[boardId].m_systype);
   if (systype != static_cast<uint16_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//----------------------------------------------------------------------------------------
// Get system number
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getSysNo(
			uint16_t& sysno,
			BoardID boardId
			)
{
   sysno = s_boardIdList[boardId].m_sysno;
   if (sysno != static_cast<uint16_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//----------------------------------------------------------------------------------------
// Get Functional Board 
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getFBN(
			uint16_t& fbn,
			BoardID boardId
			)
{
   fbn = s_boardIdList[boardId].m_fbn;
   if (fbn != static_cast<uint16_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//----------------------------------------------------------------------------------------
// Get CP side 
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getSide(
			uint16_t& side,
			BoardID boardId
			)
{
   side = s_boardIdList[boardId].m_side;
   if (side != static_cast<uint16_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//----------------------------------------------------------------------------------------
// Get sequence number
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getSeqNo(
         uint16_t& seqNo,
         BoardID boardId
         )
{
   seqNo = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get IP address on backplane A
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getIPEthA(
         uint32_t& ip,
         BoardID boardId
         )
{
   ip = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get IP address on backplane B
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getIPEthB(
         uint32_t& ip,
         BoardID boardId
         )
{
   ip = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get alias IP address on backplane A
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getAliasEthA(
         uint32_t& ip,
         BoardID boardId
         )
{
   ip = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get alias IP address on backplane B
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getAliasEthB(
         uint32_t& ip,
         BoardID boardId
         )
{
   ip = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get netmask for IP alias address on backplane A
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getAliasNetmaskEthA(
         uint32_t& mask,
         BoardID boardId
         )
{
   mask = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get netmask for IP alias address on backplane B
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getAliasNetmaskEthB(
         uint32_t& mask,
         BoardID boardId
         )
{
   mask = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get HWC DHCP Method
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getDhcpMethod(
         uint16_t& dhcp,
         BoardID boardId
         )
{
   dhcp = 0;
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Get system identity
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_HWC_R1::getSysId(
			uint16_t& sysid,
			BoardID boardId
			)
{
   sysid = s_boardIdList[boardId].m_sysid;
   if (sysid != static_cast<uint16_t>(-1))
   {
      return ACS_CS_API_NS::Result_Success;
   }
   else
   {
      return ACS_CS_API_NS::Result_NoValue;
   }
}

//========================================================================================
// Class ACS_CS_API_BoardSearch
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
ACS_CS_API_BoardSearch_R1::ACS_CS_API_BoardSearch_R1():
m_hwcTable()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
ACS_CS_API_BoardSearch_R1::~ACS_CS_API_BoardSearch_R1()
{
}


//----------------------------------------------------------------------------------------
// Set magazine
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setMagazine(
			uint32_t magazine
			)
{
	m_hwcTable.m_magazine = magazine;
}

//----------------------------------------------------------------------------------------
// Set slot
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setSlot(
         uint16_t slot
         )
{
	m_hwcTable.m_slot = slot;
}

//----------------------------------------------------------------------------------------
// Set system type
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setSysType(
			uint16_t systype
			)
{
	m_hwcTable.m_systype = static_cast<ACS_CS_API_HWC_NS::HWC_SystemType_Identifier>(systype);
}

//----------------------------------------------------------------------------------------
// Set system number
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setSysNo(
			uint16_t sysno
			)
{
	m_hwcTable.m_sysno = sysno;
}

//----------------------------------------------------------------------------------------
// Set Functional Board 
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setFBN(
			uint16_t fbn
			)
{
	m_hwcTable.m_fbn = static_cast<ACS_CS_API_HWC_NS::HWC_FBN_Identifier>(fbn);
}

//----------------------------------------------------------------------------------------
// Set CP side 
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setSide(
			uint16_t side
         )
{
	m_hwcTable.m_side = static_cast<ACS_CS_API_HWC_NS::HWC_Side_Identifier>(side);
}

//----------------------------------------------------------------------------------------
// Set system identity
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::setSysId(
			uint16_t sysid
			)
{
	m_hwcTable.m_sysid = sysid;
}

//----------------------------------------------------------------------------------------
// Reset search criterias
//----------------------------------------------------------------------------------------
void ACS_CS_API_BoardSearch_R1::reset()
{
	m_hwcTable = ACS_CS_API_HWC();
}

//========================================================================================
// Class ACS_CS_API_Name
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
ACS_CS_API_Name_R1::ACS_CS_API_Name_R1() :
m_name()
{
}

ACS_CS_API_Name_R1::ACS_CS_API_Name_R1(const char* name) :
m_name(name)
{
}

//----------------------------------------------------------------------------------------
// Destructors
//----------------------------------------------------------------------------------------
ACS_CS_API_Name_R1::~ACS_CS_API_Name_R1()
{
}

//----------------------------------------------------------------------------------------
// Get name
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_Name_R1::getName(
			char* name,
			size_t &nameLength
			) const
{
	nameLength = m_name.size();
   memcpy(name, m_name.c_str(), nameLength);
   name[nameLength] = 0;

   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Set name
//----------------------------------------------------------------------------------------
void ACS_CS_API_Name_R1::setName(
			const char* name
			)
{
	m_name = name;
}

//----------------------------------------------------------------------------------------
// Get name length
//----------------------------------------------------------------------------------------
size_t ACS_CS_API_Name_R1::length() const
{
   return m_name.size();
}


//========================================================================================
// Class ACS_CS_API
//========================================================================================

//----------------------------------------------------------------------------------------
// Create CP instance
//----------------------------------------------------------------------------------------
ACS_CS_API_CP_R1* ACS_CS_API_R1::createCPInstance()
{
   return new ACS_CS_API_CP_R1;
}

//----------------------------------------------------------------------------------------
// Create HWC instance
//----------------------------------------------------------------------------------------
ACS_CS_API_HWC_R1* ACS_CS_API_R1::createHWCInstance()
{
   return new ACS_CS_API_HWC_R1;
}

//----------------------------------------------------------------------------------------
// Delete CP instance
//----------------------------------------------------------------------------------------
void ACS_CS_API_R1::deleteCPInstance(ACS_CS_API_CP_R1* cpInstance)
{
   delete cpInstance;
}

//----------------------------------------------------------------------------------------
// Delete HWC instance
//----------------------------------------------------------------------------------------
void ACS_CS_API_R1::deleteHWCInstance(ACS_CS_API_HWC_R1* hwcInstance)
{
   delete hwcInstance;
}

//========================================================================================
// Class ACS_CS_API_SubscriptionMgr
//========================================================================================

ACS_CS_API_SubscriptionMgr* ACS_CS_API_SubscriptionMgr::s_instance = 0;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
ACS_CS_API_SubscriptionMgr::ACS_CS_API_SubscriptionMgr()
{
}

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
ACS_CS_API_SubscriptionMgr::~ACS_CS_API_SubscriptionMgr()
{
}

//----------------------------------------------------------------------------------------
// Get subscription manager instance
//----------------------------------------------------------------------------------------
ACS_CS_API_SubscriptionMgr* ACS_CS_API_SubscriptionMgr::getInstance()
{
   if (s_instance == 0)
   {
      s_instance = new ACS_CS_API_SubscriptionMgr;
   }
   return s_instance;
}

//----------------------------------------------------------------------------------------
// Subscribe for HWC table changes
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_SubscriptionMgr::subscribeHWCTableChanges(
      ACS_CS_API_HWCTableObserver& observer
      )
{
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Unsubscribe for HWC table changes
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_SubscriptionMgr::unsubscribeHWCTableChanges(
      ACS_CS_API_HWCTableObserver& observer
      )
{
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Subscribe for CP table changes
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_SubscriptionMgr::subscribeCpTableChanges(
      ACS_CS_API_CpTableObserver& observer
      )
{
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Unsubscribe for CP table changes
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_SubscriptionMgr::unsubscribeCpTableChanges(
      ACS_CS_API_CpTableObserver& observer
      )
{
   return ACS_CS_API_NS::Result_Success;
}

//========================================================================================
// Class ACS_CS_API_NetworkElement
//========================================================================================

//----------------------------------------------------------------------------------------
// Get default CP name
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_NetworkElement_R1::getDefaultCPName(
			CPID cpid,
			ACS_CS_API_Name& name
			)
{
   ostringstream s;
   if (cpid < 1000)
   {
      s << "BC" << cpid;
   }
   else if (cpid < 2000)
   {
      s << "CP" << cpid - 1000;
   }
	else if (cpid < 3000)
   {
      s << "AP" << cpid - 2000;
   }
	else
	{
		return ACS_CS_API_NS::Result_Failure;
	}

   const string& tname = s.str();
   name.setName(tname.c_str());
   return ACS_CS_API_NS::Result_Success;
}

//----------------------------------------------------------------------------------------
// Is it a multi CP system?
//----------------------------------------------------------------------------------------
ACS_CS_API_NS::CS_API_Result ACS_CS_API_NetworkElement_R1::isMultipleCPSystem(
			bool& multiCPSystem
			)
{
   multiCPSystem = false;

   acs_apgcc_paramhandling par;
   ACS_CC_ReturnType result;
   result = par.getParameter(
                     "cpConfiguration",
                     "multiCPSystem",
                     &multiCPSystem
                     );

   return (result == ACS_CC_SUCCESS)?
         ACS_CS_API_NS::Result_Success:
         ACS_CS_API_NS::Result_Failure;
}

