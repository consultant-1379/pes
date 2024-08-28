//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      dumps.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2010. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class handles installation of dump packages, configuration dumps,
//      memory settings and listing of dump versions.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 0xxx  PA1
//
//  AUTHOR
//      EAB/FLE/DF UABTSO (Thomas Olsson)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      A      2010-03-16   UABTSO      CFET was migrated to APG50.
//      B      2011-11-11   UABMAGN     Implemented CXP package handling
//      C      2013-02-26   XVUNGUY     Add functions for Blade memory configuration
//      -      2017-05-17   XMAHIMA     Updated for SW MAU(MAUS) Improvement feature
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>
#ifndef DUMPS_H
#define DUMPS_H
#include "pesconfig.h"
#include "hwcinfo.h"
#include <string>
template<Common::t_cptype>
class Dumps;
//----------------------------------------------------------------------------------------
// Base class for PES configuration
//----------------------------------------------------------------------------------------
class DumpsBase
{
public:
    enum t_conftype {e_cphw, e_pes, e_mem};
    enum t_dumpversion {e_bak, e_cur, e_new, e_all};

    static void modifyPatches(
	    std::string& cpinfo,
            std::string& patchList
    );

    static void modifyPatches(
            std::string& cpname,
            std::string& cpside,
            std::string& patchList
    );

    static void listPatches(
	    std::string& cpinfo
    );

    static void listPatches(
	    std::string& cpinfo,
            std::string& cpside
    );

protected:
    DumpsBase() {}
    virtual ~DumpsBase() {}
    // Read text from a file
    static std::string readText(                        // Returns text
            const std::string& file                     // File name name
    );
    // Write text to a file
    static void writeText(
            const std::string& file,                    // File name name
            const std::string text                      // Text
    );
    static void validatePath(
            const std::string& packagePath              // Installation package file
    );

    static void unzip(
            const std::string& source,
            const std::string& dest,
            int CpId
    );

    //To remove all previous existing cxc files in directory before unzipping the boot image
    static void removeCpMauCxcFiles(const std::string& dirPath);


    static char m_logCpmwPath[50];                      // logical nbi cpmw path
    static char m_cpmwPath[250];                        // absolute nbi cpmw path
    static std::string m_dumpName;
    static const std::string& s_APZVMDIR;
    static const std::string& s_BOOTIMAGE;
    static const std::string& s_PESCONFIG;
    static const std::string& s_CPHWCONFIG;
    //static const std::string& s_LZYINFO;
    static const std::string& s_DUMPINFO;
    static const std::string& s_BOOTPREFIX;
    static const std::string& s_RUFPREFIX;
    static const std::string& s_FWUPGRADE;
    static const std::string& s_URLF;
    static const std::string& s_URCF;
    static const std::string& s_CONFIGFILE;

    static const std::string&	s_DEFAULT_1;
    static const std::string&	s_DEFAULT_2;
    static const std::string&	s_DEFAULT;
    static const std::string&	s_PXELINUX;
};
//----------------------------------------------------------------------------------------
// Multi CP system
//----------------------------------------------------------------------------------------
template<>
class Dumps<Common::e_multicp> : public DumpsBase
{
public:
    // Install CPHW package
    static void installCPHW(
            const std::string& packagePath              // Installation package file
    );
    // Install PES package
    static void installPES(
            const std::string& packagePath              // Installation package file
    );
    // Configure the CPHW dump
    static void configCPHW(
            const HWCInfo& hwcinfo,                     // HWC information
            t_dumpversion version                       // Dump version
    );
    // Configure the PES dump
    static void configPES(
            const HWCInfo& hwcinfo,                     // HWC information
            t_dumpversion version                       // Dump version
    );
    // Print configurations
    static void printConfigurations(
            const HWCInfo& hwcinfo,                     // HWC information
            t_dumpversion version                       // Dump version
    );
    // Print double sided configurations (in multi-CP system)
    static void printDoubleSidedConfigurations(
            const HWCInfo& hwcinfo,                     // HWC information
            t_dumpversion version                       // Dump version
    );
    // Memory configuration
    static void memoryConfig(
            const HWCInfo& hwcinfo                      // HWC information
    );
    // Blade Memory configuration (in multi-CP system)
    static void bladeMemoryConfig(
            const HWCInfo& hwcinfo                      // HWC information
    );
    // Remove configuration
    static void removeConfig(
            const std::string& cpname,                  // CP name
            char cpside                                 // CP side
    );

    // check for the presence of Reserved flag in a CDA file
    static void readReservedFlag();
    static bool reserved_flag;


private:
    // Print CPHW configuration
    static void printCPHWconfig(
            const std::string& conffile                 // Configuration file
    );
    // Print PES configuration
    static void printPESconfig(
            const std::string& conffile                 // Configuration file
    );
    // List memory configurations
    static Common::VECTOR listMemoryConfigs(            // Returns list of memory configurations
            const std::string& apzconf                  // APZ configuration
    );
    // Select memory configuration
    static bool selectMemoryConfig(                     // Returns true if configuration
            // was found, false otherwise
            Pesconfig<Common::e_multicp>& conf          // PlexEngine configuraion object
    );
    static bool isNumber(const std::string& s);         // Check input string is number or not

    static bool isNewDefaultFile(const std::string& path);    //Check if the default file is new format

    static bool displayNewSm(const std::string& pxedir,
            std::vector<std::string>& database,
            std::vector <std::string>& invalidDb);

    static bool displayOldSm(const std::string& path);

};
//----------------------------------------------------------------------------------------
// One CP system
//----------------------------------------------------------------------------------------
template<>
class Dumps<Common::e_onecp> : public DumpsBase
{
public:
    // Install CPHW package
    static void installCPHW(
            const std::string& packagePath              // Installation package file
    );
    // Install PES package
    static void installPES(
            const std::string& packagePath              // Installation package file
    );
    // Configure the CPHW dump
    static void configCPHW(
            char side,                                  // CP side
            t_dumpversion version                       // Dump version
    );
    // Configure the PES dump
    static void configPES(
            char side,                                  // CP side
            t_dumpversion version                       // Dump version
    );
    // Print configurations
    static void printConfigurations(
            t_dumpversion version, char side            // Dump version
    );
    // Memory configuration
    static void memoryConfig(
            char side                                   // CP side
    );
    // check for the presence of Reserved flag in a CDA file
    static void readReservedFlag();
    static bool reserved_flag;

private:
    enum t_apzType {e_apz21240, e_apz21250, e_apz21255, e_apz21401};
    // Get APZ type
    static t_apzType getApzType();                      // Returns APZ type
    static const std::string& s_BOOTPREFIXDOT;
    static const std::string& s_UPBBUPGRADE;
    static const std::string& s_RPHMIUPGRADE;
    static const std::string& s_CPBBUPGRADE;
    static const char s_APCONFBIN[];
    // Print CPHW configuration
    static void printCPHWconfig(
            const std::string& conffile,                // Configuration file
            char side                                   // CP side
    );
    // Print PES configuration
    static void printPESconfig(
            const std::string& conffile,                // Configuration file
            char side,                                  // CP side
            t_apzType apzType                           // "CPandProtocolType" (fetched from PHA parameter)
    );
    // List memory configurations
    static Common::VECTOR listMemoryConfigs(            // Returns list of memory configurations
            const std::string& apzconf                  // APZ configuration
    );
    // Select memory configuration
    static bool selectMemoryConfig(                     // Returns true if configuration
            // was found, false otherwise
            Pesconfig<Common::e_onecp>& conf,           // PlexEngine configuration object
            char side                                   // CP side
    );
};
#endif
