//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      pesconfig.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2017. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class handles the XPU configuration files.
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 0xxx  PA1
//
//  AUTHOR
//      XDT/XLOBUNG (Long Nguyen)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2014-02-19   XLOBUNG     XPU Config introduced.
//      PA2    2017-04-26   EGIAAVA     TR HV81603.
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef XPUCONFIG_H
#define XPUCONFIG_H

#include "common.h"
#include <hwcinfo.h>
#include <string>
#include <vector>

class Xpuboard
{
public:
    // Constructor
    Xpuboard(const std::string& board, const std::string& mem);
    // Destructor
    ~Xpuboard();
    // Get board name
    const std::string& getBoardName() const;
    // Get memory size
    const std::string& getMemSize() const;
    // Equality operator
    bool operator==(const Xpuboard& board) const;
    // Display board
    void display();
private:
    std::string     m_boardname;                    // Board name
    std::string     m_memsize;                      // Memory size
};

class Xpuboardset
{
public:
    // Constructor
    Xpuboardset();
    // Destructor
    ~Xpuboardset();
    // Add XPU board
    void addXpuBoard(Xpuboard board);
    // Get XPU board set
    std::vector<Xpuboard> getXpuboardset() const;
    // Set default set
    void setDefaultSet();
    // Check default set
    bool isDefault() const;
    // Set filename
    void setFilename(const std::string& filename);
    // Get filename
    const std::string& getFilename() const;
    // Equality operator
    bool operator==(const Xpuboardset& board) const;
    // Display set
    void display();
    // Check if emptiness
    bool isEmpty() const;
private:
    std::vector<Xpuboard>   m_xpuboardset;          // Board Set
    bool                    m_isdefault;            // Default set
    std::string             m_filename;             // File name
};

class Xpuconfig
{
public:
    // Constructor
    Xpuconfig();
    // Destructor
    ~Xpuconfig();
    // Read default files
    bool read(const HWCInfo& hwcinfo);
    // List XPU configuration
    void listXpuConfiguration();
    // Select XPU configuration
    void selectXpuConfiguration();
    // Display new XPU setting
    void displayNewSetting(const HWCInfo& hwcinfo);
    // Display current setting
    void displayCurSetting(const HWCInfo& hwcinfo);
    // Display backup setting
    void displayBakSetting(const HWCInfo& hwcinfo);
    // Store current setting to backup setting
    void storeCurToBakSetting(const HWCInfo& hwcinfo, const string& cfgfilepath);
    // Restore backup setting to current setting
    string restoreBakToCurSettingStep1(const string& bakfile);
    void restoreBakToCurSettingStep2(const HWCInfo& hwcinfo, const string& filename);
    // Check if no option
    bool isNoOption() const;

private:
    Xpuboardset             m_defaultset;           // Default set
    vector<Xpuboardset>     m_optionsetlist;        // Options
    std::string             m_ipxedir;              // iPXE dir

    // Parse a default file
    Xpuboardset parse(const string& filepath);
    // for Blade memory configuration
    int listFiles(
            const std::string& path,                    // path to default files
            std::vector<std::string>& database          // store all default files
    );

    static const char s_HEADING[];                  // Heading for XPU configuration
    static const char s_HEADING2[];
    static const char s_HEADING3[];
    static const char s_HEADING4[];
    static const char s_HEADING5[];
    static const std::string& s_BOOTIMAGE;
    static const std::string& s_BOOTPREFIX;
    static const std::string& s_CPHWCONFIG;
};

class CompareObjects
{
public:
    bool operator() (const string& file1, const string& file2);
};
#endif
