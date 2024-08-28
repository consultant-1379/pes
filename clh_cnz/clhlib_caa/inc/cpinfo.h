//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      cpinfo.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013, 2014. All rights reserved.
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
//      PA2    2014-06-18   XLOBUNG     Add MAUS logs
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef CPINFO_H_
#define CPINFO_H_

#include <ACS_CS_API.h>
#include <boost/logic/tribool.hpp>
#include <string>
#include <set>

namespace PES_CLH {

enum t_apzSystem
{
   e_undefined = -1,
   e_classic,
   e_apz21240,
   e_apz21250,
   e_apz21255,
   e_apz21260,
   e_apz21270,
   e_apz21401,
   e_apz21403,
   e_apz21410,
   e_apz21420
};

enum t_cpSide
{
   e_noside =       0,
   e_cpa =          1,
   e_cpb =          2,
   e_bothsides =    3
};

enum t_mauType
{
   e_mauundefined =  0,
   e_maub =          1,
   e_maus =          2
};

class CPTable;

//========================================================================================
// Class CPInfo
//========================================================================================

class CPInfo
{
   friend class CPTable;

public:
   // Constructor
   CPInfo();

   // Destructor
   virtual ~CPInfo();

   // Get CP identity
   CPID getCPID() const;                   // Returns CP identity

   // Get CP name
   std::string getName() const;            // Returns CP name

   // Get APZ system   
   t_apzSystem getAPZSystem() const;       // Return the APZ system

   // Get error message
   static std::string getErrorMessage(     // Returns error message
         ACS_CS_API_NS::CS_API_Result result // Result code
         );
         
   // Get MAU Type
   t_mauType getMAUType() const;           // Return the MAU Type

private:
   // Constructor
   CPInfo(
         CPID cpId,                       // CP identity
         ACS_CS_API_CP* m_cpInstance      // CP table instance
         );

   CPID m_cpId;                           // CP identity
   ACS_CS_API_CP* m_cpInstance;           // CP table instance
};

//========================================================================================
// Class CPTable
//========================================================================================

class CPTable: public ACS_CS_API_CpTableObserver
{
   friend class CPInfo;

   typedef std::set<CPID> CPIDLIST;
   typedef CPIDLIST::const_iterator CPIDLISTCITER;
   typedef void (*t_callback)(void*);

public:
   // Class const_iterator
   class const_iterator
   {
      friend class CPTable;

   public:
      // Constructor
      const_iterator();

      // Destructor
      ~const_iterator();

      // Reference operator
      const CPInfo* operator->();

      // Pointer operator
      const CPInfo& operator*();

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
            CPIDLISTCITER iter,
            ACS_CS_API_CP* m_cpInstance
            );

      CPIDLISTCITER m_iter;
      CPInfo m_cpInfo;
   };

   // Constructors
   CPTable();

   CPTable(
         void* instptr,                    // Instance pointer
         t_callback callback               // Callback function
         );

   // Destructor
   virtual ~CPTable();

   // Get CP info in a one CP system
   CPInfo get() const;                    // Returns CP info

   // Find CP info for CPID in a multi CP system
   const_iterator find(                   // Returns iterator
         CPID cpId                        // CP id
         ) const;

   // Find CP info for CP name in a multi CP system
   const_iterator find(                   // Returns iterator
         const std::string& name          // CP name
         ) const;

   // Get first element in list
   const_iterator begin() const;

   // Get beyond last element in list
   const_iterator end() const;

   // Inquire if this is a multiple CP system
   static bool isMultiCPSystem();          // Returns true if multiple CP system,
                                          // false otherwise
                                          
   // Notification that the CP table has been updated
   void update(const ACS_CS_API_CpTableChange& observer);

   // Reset cp list.
   void reset();

private:
   // Initialize CP table
   void init();
   
   ACS_CS_API_CP* m_cpInstance;           // CP table instance
   CPIDLIST m_cpIdList;                   // List of CP ID:s
   void* m_instptr;                       // Instance pointer
   t_callback m_callback;                 // Callback function

   static boost::tribool s_multiCPSystem; // True if multi CP system
};

std::ostream& operator<<(std::ostream& s, t_apzSystem apzsystem);

}

#endif // CPINFO_H_
