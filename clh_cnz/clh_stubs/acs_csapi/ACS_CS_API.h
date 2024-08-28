#ifndef ACS_CS_API_H
#define ACS_CS_API_H

#include <string>

typedef uint16_t CPID;
typedef uint16_t BoardID; 

//----------------------------------------------------------------------------------------
// Namespace ACS_CS_API_NS
//----------------------------------------------------------------------------------------
namespace ACS_CS_API_NS
{
   // CS API Result Codes
   enum CS_API_Result
   {
      Result_Success =        0,
      Result_NoEntry =        3,
      Result_NoValue =        4,
      Result_NoAccess =       12,
      Result_Failure =        15
   };

   // APZ type identifiers
   enum CS_API_APZ_Type
   {
      APZ21255 =     1,
      APZ21401 =     2
   };
}

//----------------------------------------------------------------------------------------
// Namespace ACS_CS_API_HWC_NS
//----------------------------------------------------------------------------------------
namespace ACS_CS_API_HWC_NS
{
   using namespace ACS_CS_API_NS;   // Import global namespace

   // HWC Functional Board Name Identifiers
   enum HWC_FBN_Identifier
   {
      FBN_SCBRP =       100,
      FBN_RPBIS =       110,
      FBN_CPUB =        200,
      FBN_MAUB =        210,
      FBN_APUB =        300,
      FBN_Disk =        310,
      FBN_DVD  =        320,
      FBN_GEA  =        330,
      FBN_SCXB =        340,
      FBN_IPTB =        350,
      FBN_EPB1 =        360,
      FBN_CETB =        361,
      FBN_EvoET =       370,
      FBN_CMXB =        380
   };

   // HWC System types
   enum HWC_SystemType_Identifier
   {
      SysType_BC =      0,
      SysType_CP =      1000,
      SysType_AP =      2000
   };

   // HWC Side Identifiers
   enum HWC_Side_Identifier
   {
      Side_A =       0,
      Side_B =       1
   };

   // HWC DHCP Methods
   enum HWC_DHCP_Method_Identifier
   {
      DHCP_None =       0,
      DHCP_Normal =     1,
      DHCP_Client =     2
   };
}

class ACS_CS_API_BoardSearch_R1;
class ACS_CS_API_HWC_R1;
class ACS_CS_API_CP_R1;
class ACS_CS_API_R1;

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_Name_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_Name_R1
{
public:
	 ACS_CS_API_Name_R1();
    ACS_CS_API_Name_R1(const ACS_CS_API_Name_R1& right);
    ACS_CS_API_Name_R1(const char* name);
    virtual ~ACS_CS_API_Name_R1();

    ACS_CS_API_Name_R1& operator=(const ACS_CS_API_Name_R1& right);

    ACS_CS_API_NS::CS_API_Result getName(char* name, size_t& nameLength) const;
    void setName(const char* newName);
    size_t length () const;

private:
	std::string m_name;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_IdList_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_IdList_R1
{
	friend class ACS_CS_API_HWC_R1;
	friend class ACS_CS_API_CP_R1;

public:
   ACS_CS_API_IdList_R1();
   ~ACS_CS_API_IdList_R1();

   uint16_t operator[](const size_t index) const;

   size_t size() const;

private:
   BoardID* m_idp;
   unsigned int m_size;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_HWC_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_HWC_R1
{
	friend class ACS_CS_API_BoardSearch_R1;
	friend class ACS_CS_API_R1;

public:
   virtual ~ACS_CS_API_HWC_R1();

	bool operator==(const ACS_CS_API_HWC_R1& hwcTable) const;

   static ACS_CS_API_BoardSearch_R1* createBoardSearchInstance();

   static void deleteBoardSearchInstance(
            ACS_CS_API_BoardSearch_R1* instance
            );

   ACS_CS_API_NS::CS_API_Result getMagazine(
				uint32_t& magazine, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getSlot(
				uint16_t& slot, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getSysType(
				uint16_t& sysType, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getSysNo(
				uint16_t& sysNo, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getFBN(
				uint16_t& fbn, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getSide(
				uint16_t& side, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getSeqNo(
				uint16_t& seqNo, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getIPEthA(
				uint32_t& ip, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getIPEthB(
				uint32_t& ip, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getAliasEthA(
				uint32_t& ip, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getAliasEthB(
				uint32_t& ip, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getAliasNetmaskEthA(
				uint32_t& mask, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getAliasNetmaskEthB(
				uint32_t& mask, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getDhcpMethod(
				uint16_t& dhcp, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getSysId(
				uint16_t& sysId, 
				BoardID boardId
				);

   ACS_CS_API_NS::CS_API_Result getBoardIds(
				ACS_CS_API_IdList_R1& listObj,
				ACS_CS_API_BoardSearch_R1* searchObj
				);

private:
   ACS_CS_API_HWC_R1();

   ACS_CS_API_HWC_R1(
         uint32_t m_magazine,
			uint16_t m_slot,
			uint16_t m_systype,
			uint16_t m_sysno,
			uint16_t m_fbn,
			uint16_t m_side,
			uint16_t m_sysid
         );

   static const ACS_CS_API_HWC_R1 s_boardIdList[];
	static const uint16_t s_size;

   uint32_t m_magazine;
   uint16_t m_slot;
	ACS_CS_API_HWC_NS::HWC_SystemType_Identifier m_systype;
	uint16_t m_sysno;
	ACS_CS_API_HWC_NS::HWC_FBN_Identifier m_fbn;
   ACS_CS_API_HWC_NS::HWC_Side_Identifier m_side;
	uint16_t m_sysid;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_BoardSearch_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_BoardSearch_R1
{
	friend class ACS_CS_API_HWC_R1;

public:
	virtual ~ACS_CS_API_BoardSearch_R1();

   void setMagazine(uint32_t magazine);
   void setSlot(uint16_t slot);
   void setSysType(uint16_t sysType);
   void setSysNo(uint16_t sysNo);
   void setFBN(uint16_t fbn);
   void setSide(uint16_t side);
   void setSeqNo(uint16_t seqNo);
   void setIPEthA(uint32_t address);
   void setIPEthB(uint32_t address);
   void setAliasEthA(uint32_t address);
   void setAliasEthB(uint32_t address);
   void setAliasNetmaskEthA(uint32_t mask);
   void setAliasNetmaskEthB(uint32_t mask);
   void setDhcpMethod(uint16_t method);
   void setSysId(uint16_t sysId);
   void reset();

private:
	ACS_CS_API_BoardSearch_R1();

	ACS_CS_API_HWC_R1 m_hwcTable;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_CP_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_CP_R1
{
	friend class ACS_CS_API_R1;

public:

  	virtual ~ACS_CS_API_CP_R1();

	ACS_CS_API_NS::CS_API_Result getCPId(
				const ACS_CS_API_Name_R1& name,
				CPID& cpid
				);

	ACS_CS_API_NS::CS_API_Result getCPName(
				CPID cpid, 
				ACS_CS_API_Name_R1& name
				);

	ACS_CS_API_NS::CS_API_Result getAPZType(
				CPID cpid, 
				ACS_CS_API_NS::CS_API_APZ_Type& type
				);

	ACS_CS_API_NS::CS_API_Result getCPList(
				ACS_CS_API_IdList_R1& cpList
				);

	ACS_CS_API_NS::CS_API_Result getAPZSystem(
				CPID cpid, 
				uint16_t& system
				);

	ACS_CS_API_NS::CS_API_Result getCPType(
				CPID cpid, 
				uint16_t& type
				);

	ACS_CS_API_NS::CS_API_Result getCPAliasName(
				CPID cpid, 
				ACS_CS_API_Name_R1& alias,
				bool& isAlias
				);

private:
	ACS_CS_API_CP_R1();

	ACS_CS_API_CP_R1(
				CPID cpid,
				uint16_t system,
				uint16_t type
				);

	ACS_CS_API_NS::CS_API_Result getCPObject(
				CPID cpid,
				const ACS_CS_API_CP_R1* &object
				) const;

   static const ACS_CS_API_CP_R1 s_cpList[];
	static const uint16_t s_size;

	CPID m_cpid;
	std::string m_name;
	uint16_t m_system;
	uint16_t m_type;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_R1
{
public:
   static ACS_CS_API_CP_R1* createCPInstance();

   static ACS_CS_API_HWC_R1* createHWCInstance();

   static void deleteCPInstance(
            ACS_CS_API_CP_R1* instance
            );

   static void deleteHWCInstance(
            ACS_CS_API_HWC_R1* instance
            );

private:

};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_HWCTableChange
//----------------------------------------------------------------------------------------
class ACS_CS_API_HWCTableChange
{
public:
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_HWCTableObserver
//----------------------------------------------------------------------------------------
class ACS_CS_API_HWCTableObserver
{
public:
   ACS_CS_API_HWCTableObserver() {}
   virtual ~ACS_CS_API_HWCTableObserver() {}
   virtual void update(const ACS_CS_API_HWCTableChange&) = 0;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_CpTableChange
//----------------------------------------------------------------------------------------
class ACS_CS_API_CpTableChange
{
public:
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_CpTableObserver
//----------------------------------------------------------------------------------------
class ACS_CS_API_CpTableObserver
{
public:
   ACS_CS_API_CpTableObserver() {}
   virtual ~ACS_CS_API_CpTableObserver() {}
   virtual void update(const ACS_CS_API_CpTableChange&) = 0;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_SubscriptionMgr
//----------------------------------------------------------------------------------------
class ACS_CS_API_SubscriptionMgr
{
public:
   static ACS_CS_API_SubscriptionMgr* getInstance();

   ACS_CS_API_NS::CS_API_Result subscribeHWCTableChanges(
            ACS_CS_API_HWCTableObserver& observer
            );

   ACS_CS_API_NS::CS_API_Result unsubscribeHWCTableChanges(
            ACS_CS_API_HWCTableObserver& observer
            );

   ACS_CS_API_NS::CS_API_Result subscribeCpTableChanges(
            ACS_CS_API_CpTableObserver& observer
            );

   ACS_CS_API_NS::CS_API_Result unsubscribeCpTableChanges(
            ACS_CS_API_CpTableObserver& observer
            );

private:
   ACS_CS_API_SubscriptionMgr();
   ~ACS_CS_API_SubscriptionMgr();

   static ACS_CS_API_SubscriptionMgr* s_instance;
};

//----------------------------------------------------------------------------------------
// Class ACS_CS_API_NetworkElement_R1
//----------------------------------------------------------------------------------------
class ACS_CS_API_NetworkElement_R1
{
public:
	static ACS_CS_API_NS::CS_API_Result getDefaultCPName(
            CPID cpid,
            ACS_CS_API_Name_R1& name
            );

	static ACS_CS_API_NS::CS_API_Result isMultipleCPSystem(
            bool& multiCPSystem
            );

};

typedef ACS_CS_API_R1 ACS_CS_API;
typedef ACS_CS_API_Name_R1 ACS_CS_API_Name;
typedef ACS_CS_API_IdList_R1 ACS_CS_API_IdList;
typedef ACS_CS_API_CP_R1 ACS_CS_API_CP;
typedef ACS_CS_API_HWC_R1 ACS_CS_API_HWC;
typedef ACS_CS_API_NetworkElement_R1 ACS_CS_API_NetworkElement;
typedef ACS_CS_API_BoardSearch_R1 ACS_CS_API_BoardSearch;

#endif /* ACS_CS_API_H_ */
