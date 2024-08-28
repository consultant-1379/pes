//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      mauconfig.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2015 - 2021. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class handles the MAU configuration files.
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
//      PA1    2015-05-06   XLOBUNG     MAU Config introduced.
//      -      2017-05-17   XMAHIMA     Updated for SW MAU(MAUS) Improvement feature
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef MAUCONFIG_H
#define MAUCONFIG_H

#include "common.h"
#include <string>
#include <vector>

// Class of MAU information
class Mauinfo
{
public:
    Mauinfo() {};
    ~Mauinfo() {};

    static const std::string& s_MAUCONFIG;          // Path to MAU config directory
    static const std::string& s_MAU1CFGFILENAME;    // MAU config file for MAUS1 name
    static const std::string& s_MAU2CFGFILENAME;    // MAU config file for MAUS2 name
    static const std::string& s_MAULIBPATH;         // Path to MAU lib directory
    static const std::string& s_MAULIBFNPATTERN;    // Path to MAU lib directory
    static const std::string& s_INFONA;             // Information not available
    static const std::string& s_APNODEASTR;
    static const std::string& s_APNODEBSTR;
    // This part is for MAUS.conf
    static const std::string& s_NEWSECTION;
    static const std::string& s_CURSECTION;
    static const std::string& s_BAKSECTION;
    
    static const std::string& s_NEWOPTAPA;
    static const std::string& s_NEWOPTAPASUF;
    static const std::string& s_NEWOPTAPB;
    static const std::string& s_NEWOPTAPBSUF;
    static const std::string& s_CUROPTAPA;
    static const std::string& s_CUROPTAPASUF;
    static const std::string& s_CUROPTAPB;
    static const std::string& s_CUROPTAPBSUF;
    static const std::string& s_BAKOPTAPA;
    static const std::string& s_BAKOPTAPASUF;
    static const std::string& s_BAKOPTAPB;
    static const std::string& s_BAKOPTAPBSUF;
    // End
    
    static const int s_MAUS1IDX;
    static const int s_MAUS2IDX;

    // This is for lib*.txt
    static const std::string& s_INFOPRODUCT;
    static const std::string& s_INFOREV;
    static const std::string& s_STARTMAUINFO;
    static const std::string& s_STOPMAUINFO;

    static const std::string& s_BOOTIMAGE;
    static const std::string& s_CPHWCONFIG;
    // regular expression for reading CP-MAU and CP-MAUSCORE 
    // product number and revision
    static const std::string& s_CPMAUDEFFNPATTERN;
    static const std::string& s_INFOCPMAUPROREV;
    static const std::string& s_INFOMAUSCOREPROREV;

};

// Class of an available MAU item
class Mauitem
{
public:
    // Constructor
    Mauitem();
    // Destructor
    ~Mauitem();
    // Get filename
    const std::string getFilename() const;
    // Set filename
    void setFilename(const std::string& filename);
    // Get MAU product number
    const std::string& getProductNumber() const;
    // Set MAU product number
    void setProductNumber(const std::string& productnumber);
    // Get MAU revsion
    const std::string& getRevision() const;
    // Set MAU revision
    void setRevision(const std::string& revision);
    // Display AP MAU item
    void displayItem();
    // Get CP-MAUSCORE product number
    const std::string& getMausCoreProductNumber() const;
    // Set CP-MAUSCORE product number
    void setMausCoreProductNumber(const std::string& mauscoreproductnumber);
    // Get CP-MAUSCORE revision
    const std::string& getMausCoreRevision() const;
    // Set CP-MAUSCORE revision
    void setMausCoreRevision(const std::string& mauscorerevision);
    // Display CP-MAU item
    void displayItemCp();


private:
    // File name
    std::string   m_filename;
    // Product number of a MAU 
    std::string   m_productnumber;
    // Revision of a MAU 
    std::string   m_revision;
    // Product number of CP-MAUSCORE lib
    std::string   m_mauscoreproductnumber;
    // Revision of a CP-MAUSCORE lib
    std::string   m_mauscorerevision;

};

// Mau item table
class Mauitemtable
{
public:
    // Constructor
    Mauitemtable();
    // Destructor
    ~Mauitemtable();
    // Push back an item
    void pushItem(Mauitem item);
    // Display AP MAU table
    void displayTable();
    // Get out an item
    Mauitem getItemByIndex(int index) const;
    // Get size of the table
    int getTableSize() const;
    // Read AP Mau item table
    bool readMauItemTable();
    // Read CP Mau item table
    bool readCpMauItemTable(char cpside,bool ismcp,int cpnameid);
    // Parse the CXC def file
    bool parseCXCdefInfoFile(const std::string& path,
    		std::string& product,
    		std::string& revision,
                std::string& mauscoreproduct,
                std::string& mauscorerevision);
    // Parse the library information file
    static bool parseLibInfoFile(const std::string& path,
                          std::string& product,
                          std::string& revision);
    // Display CP MAU table
    void displayTableCp();

private:
    // List of available MAU item
    std::vector<Mauitem> m_mautable;
    // Sort table
    void sortTable();

    static const char s_HEADING[];                  // Heading for AP-MAU configuration
    static const char s_HEADINGCP[];                // Heading for CP-MAU configuration
};

//// Class of MAU setting
//class Mausetting
//{
//   public:
//    // Constructor
//    Mausetting();
//    // Destructor
//    ~Mausetting();
//
//private:
//    // Mau item for AP-A
//    Mauitem   m_mauapa;
//    // Mau item for AP-B
//    Mauitem   m_mauapb;
//};

// Class for MAU configuration
class Mauconfig
{
public:
    // Constructor
    Mauconfig(int index);
    // Destructor
    ~Mauconfig();
    // Read default files
    bool read();
    // List MAU configuration
    void listMAUConfiguration();
    // Select MAU configuration
    void selectMAUConfiguration(int cfgnum);
    //List CP MAU configurations
    void listCPMAUConfiguration(char cpside, bool ismcp);
    // Select CP MAU configuration
    void selectCPMAUConfiguration(int cfgnum,char cpside, bool ismcp);
    // print CP MAU version
    void printCPMAUVersion(const std::string& version,char cpside, bool ismcp);
    // Reads the link and returns the filename to which the link is pointing
    static std::string safeReadlink(const std::string& filename);
    //returns 0 for side A,1 for side B
    static int checkSide(char side);
    // Display new MAU setting
    void displayNewSetting();
    // Display current MAU setting
    void displayCurSetting();
    // Display backup MAU setting
    void displayBakSetting();
    // Display all MAU setting
    void displayAllSetting();
    // Install the new configuration
    void installNewConfiguration(int side);
    // Install the bak configuration
    void installBakConfiguration(int side);
    // Reload MAUS lib
    bool reloadMAUS(int side);

    // Calculate checksum
    bool checksum(int side, int config);

    // Install Mibs
    bool installMibs(int side, int config);

private:
    enum t_mauversion {e_baka = 0, e_bakb = 1, e_cura = 2, e_curb = 3, e_newa = 4, e_newb = 5, e_all = 6};
    //Mausetting             m_mausettings[3];           // Default set
    int		m_mauindex;                 // MAUS1 or MAUS2
    string	m_mauconfig[e_all];
    string	m_tmpconfig[e_all];

    // Parse a config file
    bool parse();
    // Write down config file
    bool writeCfgFile(const std::string& path);
    // RC to string
    const std::string convertFCtoString(int fc) const;
    // Print file info
    void printFileInfo(const std::string& fn) const;
    // Parse the CXC def file for CP-MAUSCORE details
    bool parseCXCdefForMausCoreInfo(const std::string& path,
                std::string& mauscore_prodrev);
};

#endif
