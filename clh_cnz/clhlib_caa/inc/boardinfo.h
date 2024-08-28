//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      boardinfo.h
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

#ifndef BOARDINFO_H_
#define BOARDINFO_H_

#include <ACS_CS_API.h>
#include <iostream>
#include <string>
#include <set>

namespace PES_CLH {

//========================================================================================
// Class Magazine
//========================================================================================

class Magazine
{
   friend std::ostream& operator<<(
         std::ostream& s,
         const Magazine& magazine
         );

public:
   // Constructors
   Magazine();

   Magazine(
         uint32_t id
         );

   Magazine(
         const std::string& magazine
         );

   // Destructor
   ~Magazine();

   // Magazine address operators
   operator uint32_t() const;
   operator std::string() const;
   int operator[](uint32_t index) const;

private:
   union
   {
      uint32_t m_id;
      uint8_t m_plug[4];
   };
};

//========================================================================================
// Class Slot
//========================================================================================

class Slot
{
   friend std::ostream& operator<<(
         std::ostream& s,
         const Slot& slot
         );

public:
   // Constructors
   Slot();

   Slot(
         const std::string& slot
         );

   Slot(
         uint16_t id
         );

   // Destructor
   ~Slot();

   // Slot position operator
   operator uint16_t() const;

private:
   uint16_t m_id;
};

//========================================================================================
// Class BoardInfo
//========================================================================================

class BoardTable;

class BoardInfo
{
   friend class BoardTable;

public:
   // Constructor
   BoardInfo();

   // Destructor
   ~BoardInfo();

   // Get magazine address
   uint32_t getMagazine() const;          // Returns magazine address

   // Get slot position
   uint16_t getSlot() const;              // Returns slot number 0 - 25

   // Get HWC system type
   uint16_t getSysType() const;           // Returns HWC system type
                                          // SysType_BC: Blade CP
                                          // SysType_CP: Double sided CP
                                          // SysType_AP: Adjunct Processor

   // Get system number
   uint16_t getSysNo() const;             // Returns system number 0 - 9999

   // Get HWC Functional Board Name (FBN) Identifier
   uint16_t getFBN() const;               // Returns FBN identifier
                                          // FBN_SCBRP: Support and Communication Board - Regional
                                          //            Processor
                                          // FBN_RPBIS: Regional Processor Bus Interface -
                                          //            Serial
                                          // FBN_CPUB:  Central Processor Unit Board
                                          // FBN_MAUB:  MAintenance Unit Board
                                          // FBN_APUB:  Adjunct Processor Unit Board
                                          // FBN_Disk:  Hard disk
                                          // FBN_DVD:   Digital Versatile Disc
                                          // FBN_GEA:   Generic Ericsson Alarm Board
                                          // FBN_SCXB:  Support and Communication Board
                                          // FBN_IPTB:
                                          // FBN_EPB1:
                                          // FBN_CETB:

   // Get HWC side identifier
   uint16_t getSide() const;              // Returns HWC side identifier
                                          // Side_A: Side A
                                          // Side_B: Side B

   // Get sequence number
   uint16_t getSeqNo() const;             // Returns sequence number

   // Get IP address on backplane A
   uint32_t getIPEthA() const;            // Returns IP address

   // Get IP address on backplane B
   uint32_t getIPEthB() const;            // Returns IP address

   // Get alias IP address on backplane A
   uint32_t getAliasEthA() const;         // Returns IP

   // Get alias IP address on backplane B
   uint32_t getAliasEthB() const;         // Returns IP

   // Get netmask for IP alias address on backplane A
   uint32_t getAliasNetmaskEthA() const;
                                          // Returns netmask

   // Get netmask for IP alias address on backplane B
   uint32_t getAliasNetmaskEthB() const;
                                          // Returns netmask

   // Get HWC DHCP Method
   uint16_t getDhcpMethod() const;        // Returns HWC DHCP Method
                                          // DHCP_None:   No IP assignment
                                          // DHCP_Normal: Normal IP assignment
                                          // DHCP_Client: Client IP assignment

   // Get system identity
   uint16_t getSysId() const;             // Returns system identity

   // Get error message
   static std::string getErrorMessage(    // Returns error message
         ACS_CS_API_NS::CS_API_Result result
                                          // Result code
         );

private:
   // Constructor
   BoardInfo(
         BoardID boardId,
         ACS_CS_API_HWC* hwcTable
         );

   BoardID m_boardId;
   ACS_CS_API_HWC* m_hwcInstance;
};

//========================================================================================
// Class BoardTable
//========================================================================================

class BoardTable: public ACS_CS_API_HWCTableObserver
{
   friend class BoardInfo;

   typedef std::set<BoardID> BOARDIDLIST;
   typedef BOARDIDLIST::const_iterator BOARDIDLISTCITER;
   typedef void (*t_callback)(void*);

public:
   // Class const_iterator
   class const_iterator
   {
      friend class BoardTable;

   public:
      // Constructor
      const_iterator();

      // Destructor
      ~const_iterator();

      // Reference operator
      const BoardInfo* operator->();

      // Pointer operator
      const BoardInfo& operator*();

      // Increment operator (prefix)
      const_iterator& operator++();

      // Equality operator
      bool operator==(
            const const_iterator& iter
            ) const;

      // Unequality operator
      bool operator!=(
            const const_iterator& iter
            ) const;

   private:
      // Constructor
      const_iterator(
            BOARDIDLISTCITER iter,
            ACS_CS_API_HWC* m_boardInstance
            );

      BOARDIDLISTCITER m_iter;
      BoardInfo m_boardInfo;
   };

   // Constructors
   BoardTable();

   BoardTable(
         void* instptr,                     // Instance pointer
         t_callback callback               // Callback function
         );

   // Destructor
   virtual ~BoardTable();

   // Find board info from magazine address id and slot position
   const_iterator find(                   // Returns iterator
         const Magazine& magazine,        // Magazine address
         const Slot& slot                 // Slot position
         ) const;

   // Get first element in list
   const_iterator begin() const;

   // Get beyond last element in list
   const_iterator end() const;
   
   // Notification that the HWC table has been updated
   void update(const ACS_CS_API_HWCTableChange&);

   // Reset board list.
   void reset();

private:
   // Initialize board table
   void init();

   ACS_CS_API_HWC* m_hwcInstance;         // Board table instance
   BOARDIDLIST m_boardIdList;             // List of CP ID:s
   void* m_instptr;                       // Instance pointer
   t_callback m_callback;                 // Callback function
};

}

#endif // BOARDINFO_H_
