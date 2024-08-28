//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      memconfig.cpp
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
//      XDT/XLOBUNG (XLOBUNG)
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

#include <xpuconfig.h>
#include <exception.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <dialogue.h>
#include <dumps.h>
#include <cphwconfig.h>

using namespace std;
namespace fs = boost::filesystem;

// Heading  for XPU configuration
const char Xpuconfig::s_HEADING[] = "NO  ROJ TYPE                 XPU(MB)    CURRENT";
const char Xpuconfig::s_HEADING2[] = "XPU MEMORY INFO";
const char Xpuconfig::s_HEADING3[] = "    ROJ TYPE                 XPU(MB)";
const char Xpuconfig::s_HEADING4[] = "    TYPE                     XPU(MB)";
const char Xpuconfig::s_HEADING5[] = "NO  TYPE                     XPU(MB)    CURRENT";
const string& Xpuconfig::s_BOOTIMAGE =          "boot/image";
const string& Xpuconfig::s_BOOTPREFIX =         "boot_image_";
const string& Xpuconfig::s_CPHWCONFIG =         "hw_config";

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Xpuboard::Xpuboard(const string& board, const string& mem)
{
    m_boardname = board;
    m_memsize = mem;
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Xpuboard::~Xpuboard()
{
}

//----------------------------------------------------------------------------------------
// Get board name
//----------------------------------------------------------------------------------------
const string& Xpuboard::getBoardName() const
{
    return m_boardname;
}

//----------------------------------------------------------------------------------------
// Get memory size
//----------------------------------------------------------------------------------------
const string& Xpuboard::getMemSize() const
{
    return m_memsize;
}

//----------------------------------------------------------------------------------------
// Get memory size
//----------------------------------------------------------------------------------------
bool Xpuboard::operator ==(const Xpuboard& board) const
{
    return ((m_boardname == board.m_boardname) && (m_memsize == board.m_memsize));
}

//----------------------------------------------------------------------------------------
// Display board
//----------------------------------------------------------------------------------------
void Xpuboard::display()
{
    (void)cout.setf(ios::left, ios::adjustfield);
    cout << setw(4) << "    "<< setw(25) << m_boardname << setw(11) << m_memsize << endl;
    (void)cout.unsetf(ios::left);
}


//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Xpuboardset::Xpuboardset():
m_isdefault(false)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Xpuboardset::~Xpuboardset()
{
}

//----------------------------------------------------------------------------------------
// Add XPU board
//----------------------------------------------------------------------------------------
void Xpuboardset::addXpuBoard(Xpuboard board)
{
    m_xpuboardset.push_back(board);
}

//----------------------------------------------------------------------------------------
// Get XPU board set
//----------------------------------------------------------------------------------------
vector<Xpuboard> Xpuboardset::getXpuboardset() const
{
    return m_xpuboardset;
}

//----------------------------------------------------------------------------------------
// Set default set
//----------------------------------------------------------------------------------------
void Xpuboardset::setDefaultSet()
{
    m_isdefault = true;
}

//----------------------------------------------------------------------------------------
// Check default set
//----------------------------------------------------------------------------------------
bool Xpuboardset::isDefault() const
{
    return m_isdefault;
}

//----------------------------------------------------------------------------------------
// Set filename
//----------------------------------------------------------------------------------------
void Xpuboardset::setFilename(const string& filename)
{
    m_filename = filename;
}

//----------------------------------------------------------------------------------------
// Get filename
//----------------------------------------------------------------------------------------
const string& Xpuboardset::getFilename() const
{
    return m_filename;
}

//----------------------------------------------------------------------------------------
// Equality operator
//----------------------------------------------------------------------------------------
bool Xpuboardset::operator==(const Xpuboardset& board) const
{
    if (m_xpuboardset.size() == board.getXpuboardset().size())
    {
        if (equal(m_xpuboardset.begin(),
                  m_xpuboardset.end(),
                  board.getXpuboardset().begin()))
        {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------------------
// Display set
//----------------------------------------------------------------------------------------
void Xpuboardset::display()
{
    for (vector<Xpuboard>::iterator iter = m_xpuboardset.begin();
            iter != m_xpuboardset.end(); iter++)
    {
        iter->display();
    }
}

//----------------------------------------------------------------------------------------
// Check if emptiness
//----------------------------------------------------------------------------------------
bool Xpuboardset::isEmpty() const
{
    return (m_xpuboardset.size() == 0);
}


//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Xpuconfig::Xpuconfig():
m_ipxedir("")
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Xpuconfig::~Xpuconfig()
{
}

//----------------------------------------------------------------------------------------
// Read default files
//----------------------------------------------------------------------------------------
bool Xpuconfig::read(const HWCInfo& hwcinfo)
{
    const string& datadir = Common::getApzDataPath();
    // Path to pxelinux.cfg folder
    const string& pxedir = datadir + "/" + Common::toLower(hwcinfo.getName())+ "/cpa/pxelinux.cfg/";
    Dialogue dialog;
    vector<string> defaultfiles;
    //bool valid = false;
    // Get all default files
    int fileCount = listFiles(pxedir, defaultfiles);

    if (fileCount == 0)
    {
        Exception ex(Exception::e_system_28);
        ex << "Missing default file." << endl;
        throw ex;
    }

    vector<string>::iterator iter = defaultfiles.begin();
    const string& defaultpath = pxedir + (*iter);
    // Store the path
    m_ipxedir = pxedir;
    // Read the default file
    m_defaultset = parse(defaultpath);
    m_defaultset.setFilename(*iter);
    // Read the option files
    iter++;

    while (iter != defaultfiles.end())
    {
        Xpuboardset xpuboard;
        const string& optpath = pxedir + (*iter);
        xpuboard = parse(optpath);
        xpuboard.setFilename(*iter);

        // If not empty
        if (!xpuboard.isEmpty())
        {
            if (xpuboard == m_defaultset)
            {
                xpuboard.setDefaultSet();
            }
            m_optionsetlist.push_back(xpuboard);
        }
        iter++;
    }
    return true;
}

//----------------------------------------------------------------------------------------
// Parse a default file
//----------------------------------------------------------------------------------------
Xpuboardset Xpuconfig::parse(const string& filepath)
{
	Xpuboardset xpuboardset;
    ifstream ifs(filepath.c_str());

    if (!ifs.is_open())
    {
        // Handle error
        return xpuboardset;
    }

    //Get file content
    string content((istreambuf_iterator<char>(ifs)),
                    (istreambuf_iterator<char>()));
    ifs.close();

    string regExp;
    if (Common::isVAPZ())
    {
    	regExp = "<vxpumem> ([^<]+)";
    }
    else
    {
    	regExp = "<xpumem> ([^<]+)";
    }
    // Pattern for searching gepX
    boost::regex gepEx(regExp);
    // Save match strings
    string::const_iterator start = content.begin();
    string::const_iterator end = content.end();
    boost::match_results<string::const_iterator> gepMatch;
    boost::match_flag_type flags = boost::match_default;
    vector<string> lines;

    while (boost::regex_search(start, end, gepMatch, gepEx, flags))
    {
        lines.push_back(gepMatch.str());
        start = gepMatch[0].second;
    }

    if (lines.size() > 0)
    {
        for (vector<string>::const_iterator iter = lines.begin(); iter != lines.end(); iter++)
        {
            // Hold the string "<xpumem> ROJ 208 8xx/2*, 3740 "
            const string& tmp1 = *iter;
            // Take the string "ROJ 208 8xx/2*, 3740"
            const string& tmp2 = tmp1.substr(9, tmp1.length() - 9);
            vector<string> tmp3;
            boost::split(tmp3, tmp2, boost::is_any_of(","));
            if (tmp3.size() != 2)
            {
                return xpuboardset;
            }
            string board = tmp3[0];
            boost::trim(board);

            string mem = tmp3[1];
            boost::trim(mem);

            Xpuboard xpuboard(board, mem);
            xpuboardset.addXpuBoard(xpuboard);
        }
    }

    return xpuboardset;
}

//----------------------------------------------------------------------------------------
// List XPU configuration
//----------------------------------------------------------------------------------------
void Xpuconfig::listXpuConfiguration()
{
    // Check if no option
    if (isNoOption())
    {
        // Display message
        cout << "XPU configuration does not exist." << endl;
        // Exit
        return;
    }

    unsigned int count = 1;

    cout << "XPU memory configurations:" << endl << endl;
    if (Common::isVAPZ())
    {
    	cout << Xpuconfig::s_HEADING5 << endl;
    }
    else
    {
    	cout << Xpuconfig::s_HEADING << endl;
    }
    // List options
    for (vector<Xpuboardset>::iterator iter = m_optionsetlist.begin();
            iter != m_optionsetlist.end(); iter++)
    {
        stringstream sCount;
        sCount << "[" << count << "]";
        (void)cout.setf(ios::left, ios::adjustfield);
        cout << setw(40) << sCount.str() << setw(7) << (iter->isDefault()?"yes":"")<< endl;
        (void)cout.unsetf(ios::left);
        iter->display();
        count++;
    }
}

//----------------------------------------------------------------------------------------
// Select XPU configuration
//----------------------------------------------------------------------------------------
void Xpuconfig::selectXpuConfiguration()
{
    // Check if no option
    if (isNoOption())
    {
        // Exit
        return;
    }
    // Change memory configuration
    cout << endl;
    bool loop(true);

    do
    {
        stringstream s;
        s << m_optionsetlist.size();
        const string& txt = "Select a number [1 - " + s.str() + "] or type 'q' to quit";
        Dialogue dialogue;
        const string& input = Common::toUpper(dialogue.userInput(txt));

        if (input == "Q")
        {
            cout << "No changes were made." << endl;
            return;
        }
        else
        {
            try
            {
                unsigned int num = (unsigned int)Common::strToInt(input);
                if (num > 0 && num <= m_optionsetlist.size())
                {
                    // To do
                    num--;
                    const string& deffile = m_ipxedir + m_defaultset.getFilename();
                    const string& overfile = m_ipxedir + m_optionsetlist[num].getFilename();
                    // Overwrite default file
                    Common::copyFile(overfile, deffile, false);
                    loop = false;
                    cout << "Configuration was saved." << endl;
                }
                else
                {
                    throw Exception(Exception::e_unexp_input_chk_19, "Illegal input.");
                }
            }
            catch (...)
            {
                throw Exception(Exception::e_unexp_input_chk_19, "Illegal input.");
                loop = false;
            }
        }
        cout << endl;
    } while (loop);
}

//----------------------------------------------------------------------------------------
// Display new XPU setting
//----------------------------------------------------------------------------------------
void Xpuconfig::displayNewSetting(const HWCInfo& hwcinfo)
{
    const string& datadir = Common::getApzDataPath();
    string dest;
    Cphwconfig<Common::e_multicp> conf;
    string file;
    const string& bootimgdir = datadir + "/" + s_BOOTIMAGE;
    string cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    Cphwconfig<Common::e_multicp>::PAIR cphwdump;
    ostringstream s;
    s << "config_bc.ini";
    file = bootimgdir + "/" + s.str();
    if (Common::fileExists(file) == false)
    {
        // No new setting here
        return;
    }
    // Read the configuration file
    conf.read(file);
    string source;
    cphwdump = conf.getDump();
    source = bootimgdir + "/" + s_BOOTPREFIX + cphwdump.second + ".zip";
    string tempdir;
    try{
        tempdir = Common::createTempDir();
        Common::unzip(source, tempdir);
        const string& ipxefile = tempdir + "/pxelinux.cfg/default";
        Xpuboardset xpuboardset = parse(ipxefile);
        if (xpuboardset.isEmpty())
        {
            // Exit
            return;
        }
        // Display
        cout << Xpuconfig::s_HEADING2 << endl;
        if (Common::isVAPZ())
        {
        	cout << Xpuconfig::s_HEADING4 << endl;
        }
        else
        {
        	cout << Xpuconfig::s_HEADING3 << endl;
        }
        xpuboardset.display();
        cout << endl;
    } catch (...)
    {
        // Error here
    }

    try
    {
        // Check if exist

        Common::deleteDirTree(tempdir);
    }
    catch (Exception& ex)
    {
        cout << "Warning: " << ex.detailInfo() << endl;
    }
}

//----------------------------------------------------------------------------------------
// Display current setting
//----------------------------------------------------------------------------------------
void Xpuconfig::displayCurSetting(const HWCInfo& hwcinfo)
{
    const string& datadir = Common::getApzDataPath();
    const string& pxepath = datadir + "/" +
            Common::toLower(hwcinfo.getName())+ "/cpa/pxelinux.cfg/default";
    Xpuboardset xpuboardset = parse(pxepath);

    if (xpuboardset.isEmpty())
    {
        // Exit
        return;
    }
    // Display
    cout << Xpuconfig::s_HEADING2 << endl;
    if (Common::isVAPZ())
    {
    	cout << Xpuconfig::s_HEADING4 << endl;
    }
    else
    {
    	cout << Xpuconfig::s_HEADING3 << endl;
    }
    xpuboardset.display();
    cout << endl;
}

//----------------------------------------------------------------------------------------
// Display backup setting
//----------------------------------------------------------------------------------------
void Xpuconfig::displayBakSetting(const HWCInfo& hwcinfo)
{
    const string& datadir = Common::getApzDataPath();
    string cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    string sidedir = cpdir + "/cpa";
    const string& pxepath = sidedir + "/pxelinux.cfg/";
    const string& file = sidedir + "/" + s_CPHWCONFIG + ".bak";
    if (Common::fileExists(file) == false)
    {
        // No backup here
        return;
    }

    ifstream ifs(file.c_str());

    if (!ifs.is_open())
    {
        // Handle error
        return;
    }

    //Get file content
    string content((istreambuf_iterator<char>(ifs)),
                    (istreambuf_iterator<char>()));
    ifs.close();

    // Pattern for searching gepX
    boost::regex gepEx("#XPU:(.*)");
    // Save match strings
    string::const_iterator start = content.begin();
    string::const_iterator end = content.end();
    boost::match_results<string::const_iterator> gepMatch;
    boost::match_flag_type flags = boost::match_default;
    vector<string> lines;

    while (boost::regex_search(start, end, gepMatch, gepEx, flags))
    {
        lines.push_back(gepMatch.str());
        start = gepMatch[0].second;
    }

    if (lines.size() > 0)
    {
        vector<string>::iterator iter = lines.begin();
        const string& line = *iter;
        const string& filename = line.substr(5, line.length() - 5);
        const string& fullpath = pxepath + filename;
        Xpuboardset xpuboardset = parse(fullpath);

        if (xpuboardset.isEmpty())
        {
            // Exit
            return;
        }

        // Display
        cout << Xpuconfig::s_HEADING2 << endl;
        if (Common::isVAPZ())
        {
        	cout << Xpuconfig::s_HEADING4 << endl;
        }
        else
        {
        	cout << Xpuconfig::s_HEADING3 << endl;
        }
        xpuboardset.display();
        cout << endl;
    }
}

//----------------------------------------------------------------------------------------
// Store current setting to backup setting
//----------------------------------------------------------------------------------------
void Xpuconfig::storeCurToBakSetting(const HWCInfo& hwcinfo, const string& cfgfilepath)
{
    read(hwcinfo);

    if (isNoOption())
    {
        // Exit
        return;
    }
    // List options
    for (vector<Xpuboardset>::iterator iter = m_optionsetlist.begin();
            iter != m_optionsetlist.end(); iter++)
    {
        if (iter->isDefault())
        {
            stringstream sCount;
            sCount << "#XPU:" << iter->getFilename();
            // Write to file
            ofstream fs(cfgfilepath.c_str(), ios::out | ios::app );
            if (fs.is_open() == false)
            {
                Exception ex(Exception::e_system_28);
                ex << "Failed to open file '" << cfgfilepath.c_str() << "'.";
                throw ex;
            }
            fs << endl << sCount.str();
            // Close file
            (void)fs.close();
            break;
        }
    }
}

// for Blade memory configuration
// List all default files
//
int Xpuconfig::listFiles(const string& path, vector<string>& database)
{
    const fs::path pxeDir(path);
    fs::directory_iterator end_iter;
    // Pattern for searching default file
    boost::regex defFile("(^default[_0-9]*)");

    int ret = 0;

    // Check if pxelinux.cfg folder exist
    if (!fs::exists(pxeDir))
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open directory '" << path << "'.";
        throw ex;
    }

    // Check if pxelinux.cfg is a directory
    if (fs::is_directory(pxeDir))
    {
        for (fs::directory_iterator dir_iter(pxeDir); dir_iter!= end_iter; dir_iter++)
        {
            const fs::path& path = *dir_iter;
            const string& name = path.filename().c_str();
            // Get all files that match fully with the pattern
            if ((fs::is_regular_file(dir_iter->status()))&&(boost::regex_match(name, defFile)))
            {
                ret++; // count number of default file
                database.push_back(name);
            }
        }
    }
    CompareObjects co;
    sort(database.begin(), database.end(), co);
    return ret;
}

//----------------------------------------------------------------------------------------
// Restore backup setting to current setting
//----------------------------------------------------------------------------------------
string Xpuconfig::restoreBakToCurSettingStep1(const string& bakfile)
{
    string line = "";
    // Step1: Get the default filename
    ifstream ifs(bakfile.c_str());

    if (!ifs.is_open())
    {
        // Handle error
        return line;
    }

    //Get file content
    string content((istreambuf_iterator<char>(ifs)),
                    (istreambuf_iterator<char>()));
    ifs.close();
    // Pattern for searching gepX
    boost::regex gepEx("#XPU:(.*)");
    // Save match strings
    string::const_iterator start = content.begin();
    string::const_iterator end = content.end();
    boost::match_results<string::const_iterator> gepMatch;
    boost::match_flag_type flags = boost::match_default;
    vector<string> lines;

    while (boost::regex_search(start, end, gepMatch, gepEx, flags))
    {
        lines.push_back(gepMatch.str());
        start = gepMatch[0].second;
    }

    if (lines.size() > 0)
    {
        vector<string>::iterator iter = lines.begin();
        string deffile = *iter;
        string filename = deffile.substr(5, deffile.length() - 5);
        return filename;
    }

    return line;
}

//----------------------------------------------------------------------------------------
// Restore backup setting to current setting
//----------------------------------------------------------------------------------------
void Xpuconfig::restoreBakToCurSettingStep2(const HWCInfo& hwcinfo, const string& filename)
{
    // Step2
    const string& datadir = Common::getApzDataPath();
    const string& pxedir = datadir + "/" +
            Common::toLower(hwcinfo.getName())+ "/cpa/pxelinux.cfg/";
    const string& pxedef = pxedir + "default";
    const string& pxedef_src = pxedir + filename;
    Common::copyFile(pxedef_src, pxedef, false);
}

//----------------------------------------------------------------------------------------
// Check if no option
//----------------------------------------------------------------------------------------
bool Xpuconfig::isNoOption() const
{
    return (m_optionsetlist.size() == 0);
}

//----------------------------------------------------------------------------------------
// Compare default filename
//----------------------------------------------------------------------------------------
bool CompareObjects::operator () (const string& file1, const string& file2)
{
    // Return true if file1 < file2
    // default (7 char) => smallest => shortest
    const string& deffile = "default_";

    if (file1.length() < deffile.length())
    {
        return true;
    }

    if (file2.length() < deffile.length())
    {
        return false;
    }

    // Parse the number
    const string& sNum1 = file1.substr(deffile.length(), file1.length() - deffile.length());
    const string& sNum2 = file2.substr(deffile.length(), file2.length() - deffile.length());

    try
    {
        unsigned int num1 = (unsigned int)Common::strToInt(sNum1);
        unsigned int num2 = (unsigned int)Common::strToInt(sNum2);

        return (num1 < num2);
    } catch (...)
    {
        return false;
    }

    return false;
}
