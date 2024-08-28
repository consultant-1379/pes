//#<heading>
//----------------------------------------------------------------------------------------
//PACKAGE INFO
//  FILE
//      dumps.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2020, 2013. All rights reserved.
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
//      R2A03  2013-02-26   XVUNGUY     Add functions for Blade memory configuration
//      C      2013-11-09   XHUNCAI     Implement user-specified path for dumps.
//      -      2015-01-21   EGIAAVA     CFET was adapted for SLES12 and new BOOST LIB 1_54
//      -      2017-05-17   XMAHIMA     Updated for SW MAU(MAUS) Improvement feature
//      -      2017-10-26   XMAHIMA     Updated for HW34608
//      -      2018-01-05   XMAHIMA     Updated for HW55166
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>
#include <exception.h>
#include <common.h>
#include <parser.h>
#include <cphwconfig.h>
#include <pesconfig.h>
#include <dumpinfo.h>
#include <memconfig.h>
#include <dialogue.h>
#include <hwcinfo.h>
#include <dumps.h>
#include <mauconfig.h>
#include <acs_apgcc_paramhandling.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <dirent.h>
#include <errno.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <xpuconfig.h>

using namespace std;
namespace fs = boost::filesystem;

//========================================================================================
// Base class for Dumps
//========================================================================================
const string& DumpsBase::s_APZVMDIR =           "boot/apz_vm";
const string& DumpsBase::s_BOOTIMAGE =          "boot/image";
const string& DumpsBase::s_PESCONFIG =          "configdb";
const string& DumpsBase::s_CPHWCONFIG =         "hw_config";
//const string& DumpsBase::s_LZYINFO =          "lzyinfo.cur";
const string& DumpsBase::s_DUMPINFO =           "lzyinfo.cur";
const string& DumpsBase::s_BOOTPREFIX =         "boot_image_";
const string& DumpsBase::s_RUFPREFIX =          "ruf_image_";
const string& DumpsBase::s_FWUPGRADE =          "fw_upgrade";
const string& DumpsBase::s_URLF =               "urlf";
const string& DumpsBase::s_URCF =               "urcf";
const string& DumpsBase::s_CONFIGFILE =         "CDA_102*";

const string& DumpsBase::s_DEFAULT_1 =		"default_1";
const string& DumpsBase::s_DEFAULT_2 =		"default_2";
const string& DumpsBase::s_DEFAULT =		"default";
const string& DumpsBase::s_PXELINUX =		"pxelinux.cfg";

char DumpsBase::m_cpmwPath[] = "";
char DumpsBase::m_logCpmwPath[] = "";
string DumpsBase::m_dumpName;
//----------------------------------------------------------------------------------------
// Read text from a file
//----------------------------------------------------------------------------------------
string 
DumpsBase::readText(const string& file)
{
    // Open file for reading
    ifstream fs(file.c_str(), ios_base::in);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to open file '" << file << "'.";
        throw ex;
    }
    string text;
    fs >> text;
    fs.close();
    return text;
}
//----------------------------------------------------------------------------------------
// Write text to a file
//----------------------------------------------------------------------------------------
void 
DumpsBase::writeText(const string& file, const string text)
{
    // Create file for writing
    ofstream fs(file.c_str(), ios_base::out | ios_base::trunc);
    if (fs.is_open() == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create file '" << file << "'.";
        throw ex;
    }
    fs << text;
    fs.close();
}

void DumpsBase::modifyPatches(std::string& cpinfo, std::string& patchList)
{

   string fileOpts;
   if(cpinfo == "a" || cpinfo == "b")
      fileOpts = " -s " + cpinfo + " -e ";
   else
      fileOpts = " -c " + cpinfo + " -e ";

   string filePath= Common::bootOptsTool() + fileOpts;
   filePath += "'" + patchList +"'";
   const char *invokeCmd = filePath.c_str();

   int eCode=0;
   try{
     eCode=Common::_execlp(invokeCmd);
   } catch (Exception& ex)
   {
        cerr << "Unable to process the request: " << ex.detailInfo();
   }
   if( eCode !=0 )
   {
      cerr << "Error when executing (general fault)" << endl;
   }
}

void DumpsBase::modifyPatches(std::string& cpname, std::string& cpside,std::string& patchList)
{
   string filePath= Common::bootOptsTool() + " -c " + cpname + " -s " + cpside + " -e ";
   filePath += "'" + patchList + "'";
   const char *invokeCmd = filePath.c_str();

   int eCode=0;
   try{
       eCode=Common::_execlp(invokeCmd);
    } catch (Exception& ex)
    {
           cerr << "Unable to process the request: " << ex.detailInfo();
    }
   if( eCode !=0 )
   {
       cerr << "Error when executing (general fault)" << endl;
   }
}

void DumpsBase::listPatches(std::string& cpinfo)
{

   string fileOpts;
   if(cpinfo == "a" || cpinfo == "b")
      fileOpts = " -s " + cpinfo + " -l " ;
   else
      fileOpts = " -c " + cpinfo + " -l " ;

   string filePath= Common::bootOptsTool() + fileOpts ;
   const char *invokeCmd = filePath.c_str();

   int eCode=0;
   try{
       eCode=Common::_execlp(invokeCmd);
    } catch (Exception& ex)
    {
	cerr << "Unable to process the request: " << ex.detailInfo();
    }
   if( eCode !=0 )
   {
	cerr << "Error when executing (general fault)" << endl;
   }	

}

void DumpsBase::listPatches(std::string& cpinfo, std::string& cpside)
{

   string filePath= Common::bootOptsTool() + " -c " + cpinfo + " -s " + cpside + " -l " ;
   const char *invokeCmd = filePath.c_str();

   int eCode=0;
   try{
       eCode=Common::_execlp(invokeCmd);
    } catch (Exception& ex)
    {
	cerr << "Unable to process the request: " << ex.detailInfo();
    }
   if( eCode !=0 )
   {
       cerr << "Error when executing (general fault)" << endl;
   }

}

//----------------------------------------------------------------------------------------
// validate packagepath with logical nbi and user-specified path
//----------------------------------------------------------------------------------------
void DumpsBase::validatePath(const std::string& packagePath) {
    //Get user-specified path in TS shell
    fs::path packpath(packagePath);
    if (Common::fileExists(packagePath) && Common::isTroubleShootingShell()){
        m_dumpName = packpath.leaf().string();
        if (!packpath.has_root_path()) {
            m_cpmwPath[0] = '.';
            m_cpmwPath[1] = '/';
            strncpy(&m_cpmwPath[2], packpath.branch_path().string().c_str(), sizeof(m_cpmwPath));
        } else {
            strncpy(m_cpmwPath, packpath.branch_path().string().c_str(), sizeof(m_cpmwPath));
        }
        m_cpmwPath[sizeof(m_cpmwPath) - 1] = '\0';
    } else {
        if (packpath.has_branch_path() && !Common::isTroubleShootingShell()) {
            Exception ex(Exception::e_WrongDump_21);
            ex << "Unreasonable value for dump.";
            throw ex;
        }
        ACS_APGCC_DNFPath_ReturnTypeT result;
        char logicName[20] = "swPackageCP";
        int len = 50;
        acs_apgcc_paramhandling apar;
        ACS_CC_ReturnType ret;
        // Get logical nbi path
        //ret = apar.getParameter("northBoundFoldersId=1", logicName, m_logCpmwPath);
        ret = apar.getParameter<50>("AxeNbiFoldersnbiFoldersMId=1", logicName, m_logCpmwPath);
        if (ret != ACS_CC_SUCCESS) {
            Exception ex(Exception::e_system_28);
            ex << "Failed to read cpmw logical path from PHA.";
            throw ex;
        }
        string nbiPath = (string) m_logCpmwPath;
        fs::path packpath(packagePath);
        if (packpath.has_branch_path()) {
            if (nbiPath.compare(packpath.branch_path().string())) {
                Exception ex(Exception::e_WrongDumpPath_22);
                ex << packagePath;
                throw ex;
            }
        } else {
            (const_cast<string &> (packagePath)).assign(nbiPath);
            (const_cast<string &> (packagePath)).append("/");
            (const_cast<string &> (packagePath)).append(packpath.string());
        }
        m_dumpName = packpath.leaf().string();
        ACS_APGCC_CommonLib lib;
        //Get absolute nbi path
        result = lib.GetFileMPath(logicName, m_cpmwPath, len);
        if (result != ACS_APGCC_DNFPATH_SUCCESS) {
            Exception ex(Exception::e_internal_29);
            switch (result) {
            case ACS_APGCC_STRING_BUFFER_SMALL:
                ex << "String buffer too small.";
                break;
            case ACS_APGCC_FAULT_LOGICAL_NAME:
                ex << "Faulty logical name.";
                break;
            default:
                ex << "Unexpected return code.";
            }
            throw ex;
        }
    }
}

void
DumpsBase::unzip(const std::string& source, const std::string& dest, int cpId)
{
	//cout << "DumpsBase::unzip() dest is: <" << dest << ">" << endl;

	string dname = dest;
	dname += '/';
	dname += s_PXELINUX;
	dname += '/';
 	string fname1 = dname;
   	fname1 += s_DEFAULT_1;
    string fname2 = dname;
    fname2 += s_DEFAULT_2;
 	Exception ex(Exception::e_ok_0);
    if ((cpId == 1001 || cpId == 1002))
    {
        Common::deleteFile(fname1, ex);
        Common::deleteFile(fname2, ex);
    }

    Common::unzip(source, dest);

    if ((cpId == 1001 || cpId == 1002))
    {
    	string fname = dname;
    	fname += s_DEFAULT;
    	if (Common::isMAUS(cpId))
    	{
    		if (Common::fileExists(fname2))
    		{
    			Common::copyFile(fname2, fname, ex);
    		}
    	}
    	else
    	{
    		if (Common::fileExists(fname1))
    		{
    			Common::copyFile(fname1, fname, ex);
    		}
    	}
    }
}

void DumpsBase::removeCpMauCxcFiles(const string& dirPath)
{
	const fs::path& cxcDir = dirPath;
	const string& cpMauFilesPattern =  "CXC[a-zA-Z0-9_.]*$";
	const boost::regex defFilePattern(cpMauFilesPattern);
	// Get files from cxc dir
	fs::directory_iterator end;
	for (fs::directory_iterator iter(cxcDir); iter != end; ++iter)
	{
		const fs::path& path = *iter;
		const string& file = path.filename().c_str();
		if (regex_match(file, defFilePattern))
		{
			fs::remove(dirPath+"/"+file);
		}
	}
}

//========================================================================================
// Multi CP system
//========================================================================================
bool Dumps<Common::e_multicp>::reserved_flag = false;
//-------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// Install CPHW package
//----------------------------------------------------------------------------------------
void Dumps<Common::e_multicp>::installCPHW(const string& packagePath)
{
	string absolutePackagePath;
	DumpsBase::validatePath(packagePath); //validates the package path with nbi path.
	absolutePackagePath = (string) m_cpmwPath;
	absolutePackagePath.append("/");
	absolutePackagePath.append(m_dumpName);
	// Check if package file exists
	if (Common::fileExists(absolutePackagePath) == false) {
		Exception ex(Exception::e_packagefilenotfound_13);
		ex << "Package file '" << m_logCpmwPath << "/" << m_dumpName
				<< "' was not found.";
		throw ex;
	}
    // Create temporary directory
    const string& tempdir = Common::createTempDir();
    // 1.) Pre-test the package (at first level here) before unzipping it.
    // Bails out with Application error if something in package is corrupt.
    Common::testunzip(absolutePackagePath, 1);
    // Unzip the package to a temporary directory
    Common::unzip(absolutePackagePath, tempdir);
    // TR HN20934 fix
    string hwconfigfile;
    hwconfigfile = tempdir + "/" + s_CPHWCONFIG + ".new";
    if (Common::fileExists(hwconfigfile) == false) // TR HN20934 fix
    {
        // Delete zip directory
        try {
            Common::deleteDirTree(tempdir);
        } catch (Exception& ex)
        {
            cout << "Warning: " << ex.detailInfo() << endl;
        }
        Exception ex(Exception::e_WrongDump_21);
        ex << "It should be the CPHW dump to be installed";
        throw ex;
    }
    // 2.) Pre-test the package (at second level here)
    //     before unzipping the second level later.
    // Bails out with Application error if something in package
    // at second level is corrupt.
    Common::testunzip(tempdir, 2);
    // Set file attributes to normal
    Common::setAttrsRec(tempdir);
    // Check the package
    string source;
    string dest;
    // Read the configuration file
    Cphwconfig<Common::e_multicp> cphwconf;
    source = tempdir + "/config.ini";
    cphwconf.read(source);
    //const Cphwconfig<Common::e_multicp>::PAIR& lzy = cphwconf.getLzy();
    const Cphwconfig<Common::e_multicp>::PAIR& dump = cphwconf.getDump();
    // Create the boot image directory
    const string& bootimgdir = Common::getApzDataPath() + "/" + s_BOOTIMAGE;
    Common::createDir(bootimgdir);
    // Create the following symbolic links (Until it is decided by IO)
    //for directory structure
    Common::createSymlinks();
    // Assemble configuration path
    ostringstream s;
    s <<  bootimgdir << "/config_";
    switch (cphwconf.getSysType())
    {
    case Cphwconfig<Common::e_multicp>::e_cp: s << "cp"; break;
    case Cphwconfig<Common::e_multicp>::e_bc: s << "bc"; break;
    default: throw Exception(Exception::e_internal_29, "Illegal system type.");
    }
    //s << "_" << cphwconf.getCPType() << ".ini";
    s << ".ini"; // Ignore CP type, it will not be supported anymore
    dest = s.str();
    // Copy the configuration file
    Common::copyFile(source, dest, false);
    source = tempdir + "/" + s_BOOTPREFIX + dump.second + ".zip";
    dest = bootimgdir + "/" + s_BOOTPREFIX + dump.second + ".zip";
    Common::copyFile(source, dest, false);
    // Unzip the ruf image file
    source = tempdir + "/" + s_RUFPREFIX + dump.second + ".zip";
    dest = Common::getApzDataPath() + "/boot";
    Common::unzip(source, dest);

    // Delete zip directory
    try
    {
        Common::deleteDirTree(tempdir);
    }
    catch (Exception& ex)
    {
        cout << "Warning: " << ex.detailInfo() << endl;
    }
}

//----------------------------------------------------------------------------------------
// Install PES package
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::installPES(const string& packagePath)
{
    string absolutePackagePath;
    DumpsBase::validatePath(packagePath); //validates the package path with nbi path.
    absolutePackagePath = (string) m_cpmwPath;
    absolutePackagePath.append("/");
    absolutePackagePath.append(m_dumpName);
    // Check if package file exists
    if (Common::fileExists(absolutePackagePath) == false)
    {
        Exception ex(Exception::e_packagefilenotfound_13);
        ex << "Package file '" << m_logCpmwPath<<"/"<<m_dumpName<<  "' was not found.";
        throw ex;
    }
    // Create temporary directory
    const string& tempdir = Common::createTempDir();
    // 1.) Pre-test the package (at first level here) before unzipping it.
    // Bails out with Application error if something in package is corrupt.
    Common::testunzip(absolutePackagePath, 1);
    // Unzip the package to a temporary directory
    Common::unzip(absolutePackagePath, tempdir);
    // TR HN20934 fix
    string pesconfigfile;
    pesconfigfile = tempdir + "/" + s_CPHWCONFIG + ".new";
    if (Common::fileExists(pesconfigfile) == true) // TR HN20934 fix
    {
        // Delete zip directory
        try
        {
            Common::deleteDirTree(tempdir);
        }
        catch (Exception& ex)
        {
            cout << "Warning: " << ex.detailInfo() << endl;
        }
        Exception ex(Exception::e_WrongDump_21);
        ex << "It should be the PES dump to be installed";
        throw ex;
    }
    // Set file attributes to normal
    Common::setAttrsRec(tempdir);
    // Check the package
    string source;
    string dest;
    // Read the configuration file
    Pesconfig<Common::e_multicp> pesconf;
    source = tempdir + "/config.ini";
    pesconf.read(source);
    const Pesconfig<Common::e_multicp>::PAIR& apzvm = pesconf.getApzVm();
    // Create the apz_vm directory
    const string& apzvmdir = Common::getApzDataPath() + "/" + s_APZVMDIR;
    Common::createDir(apzvmdir);
    // Create symbolic link for apz_vm directory (Until it is decided by IO)
    const string vmsymlink = "/data/apz/data/boot/APZ_VM";
    const string vmdir = "apz_vm";
    if (Common::fileExists(vmsymlink) == false)
    {
        // Create symbolic link
        Common::createSymbolicLink(vmsymlink, vmdir);
    }
    // Create the following symbolic links (Until it is decided by IO)
    //for directory structure (This is for, in case hw dump is not installed yet)
    Common::createSymlinks();
    // Assemble configuration path
    ostringstream s;
    s << apzvmdir << "/config_";
    switch (pesconf.getSysType())
    {
    case Pesconfig<Common::e_multicp>::e_cp: s << "cp"; break;
    case Pesconfig<Common::e_multicp>::e_bc: s << "bc"; break;
    default: throw Exception(Exception::e_internal_29, "Illegal system type.");
    }
    //s << "_" << pesconf.getCPType() << ".ini";
    s << ".ini"; // Ignore CP type, it will not be supported anymore
    dest = s.str();
    // Copy the configuration file
    Common::copyFile(source, dest, false);
    // Copy boot image file
    source = tempdir + "/" + apzvm.second;
    dest = apzvmdir + "/" + apzvm.second;
    Common::copyFile(source, dest, false);
    // Copy memory configuration file(s)
    source = tempdir + "/" + s_CONFIGFILE;
    dest = Common::getApzDataPath() + "/" + s_APZVMDIR;
    Common::copyFiles(source, dest, false);
    // Delete zip directory
    try
    {
        Common::deleteDirTree(tempdir);
    }
    catch (Exception& ex)
    {
        cout << "Warning: " << ex.detailInfo() << endl;
    }
}


//----------------------------------------------------------------------------------------
// Configure the CPHW dump
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::configCPHW(const HWCInfo& hwcinfo, t_dumpversion version)
{
    const string& datadir = Common::getApzDataPath();
    string dest;
    Cphwconfig<Common::e_multicp> conf[3];
    string file[3];
    const string& bootimgdir = datadir + "/" + s_BOOTIMAGE;
    string cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    string sidedir;
    bool cpmauversionsequal=true;
    switch (hwcinfo.getSide())
    {
    case 0:
    case 'A': sidedir = cpdir + "/cpa"; break;
    case 'B': sidedir = cpdir + "/cpb"; break;
    default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }

    Cphwconfig<Common::e_multicp>::PAIR cphwdump;
    if (version == e_new)
    {
        // Assemble configuration file name
        ostringstream s;
        s << "config_";
        CPID cpId = hwcinfo.getCpId();
        unsigned short sysType = 1000 * (cpId/1000);
        switch (sysType)
        {
        case ACS_CS_API_HWC_NS::SysType_BC: s << "bc"; break;
        case ACS_CS_API_HWC_NS::SysType_CP: s << "cp"; break;
        default:
            Exception ex(Exception::e_internal_29);
            ex << "Illegal CP identity " << cpId << ".";
            throw ex;
        }
        //s << "_" << hwcinfo.getCPType() << ".ini";
        s << ".ini"; // Ignore CP type, it will not be supported anymore
        // Check if new configuration file exists
        file[e_new] = bootimgdir + "/" + s.str();
        if (Common::fileExists(file[e_new]) == false)
        {
            Exception ex(Exception::e_nonew_cphw_conf_17);
            ex << "There is no new CPHW configuration.";
            throw ex;
        }
        // Read the configuration file
        conf[e_new].read(file[e_new]);
        // Create CP directory
        Common::createDir(sidedir);
        // Create soft links for cpdir and sidedir (Until it is decided by IO)
        Common::createLink(cpdir);
        Common::createLink(sidedir);
        // Create CP info file
        const string& text = hwcinfo.getSide() == 0? "single": "double";
        const string& cpinfo = cpdir + "/cpinfo";
        writeText(cpinfo, text);
        // Backup previous configuration
        file[e_cur] = sidedir + "/" + s_CPHWCONFIG + ".cur";
        if (Common::fileExists(file[e_cur]))
        {
            try
            {
                // Check if new and current configurations are equal
                conf[e_cur].read(file[e_cur]);
                if(Common::isMAUS(1001) || Common::isMAUS(1002))
                {
                	cpmauversionsequal=false;
                	if(conf[e_new].m_cpMauVersionMCP == conf[e_cur].m_cpMauVersionMCP)
                	{
                		cpmauversionsequal=true;
                	}
                }
                if (conf[e_new] == conf[e_cur] && cpmauversionsequal)
                {
                    bool ok = Dialogue::affirm(
                            "Warning: New and current configurations are equal,\n"
                            "do you want to continue anyway? "
                    );
                    cout << endl;
                    if (ok == false) return;
                }
                file[e_bak] = sidedir + "/" + s_CPHWCONFIG + ".bak";
                // Backup configuration
                conf[e_bak] = conf[e_cur];
                conf[e_bak].write(file[e_bak]);
                // Write XPU infor
                Xpuconfig xpuconfig;
                xpuconfig.storeCurToBakSetting(hwcinfo, file[e_bak]);
            }
            catch (Exception& ex)
            {
                if (ex.errorCode() == Exception::e_parameter_26)
                {
                    cerr << ex << endl;
                    cerr << "Warning: Error(s) were found in current configuration file, "
                            << "no backup was created." << endl << endl;
                }
                else
                {
                	cerr << "a throw at checking detaul file: " << ex << endl;
                    throw ex;
                }
            }
        }
        // Update current configuration
        conf[e_cur] = conf[e_new];
        conf[e_cur].write(file[e_cur]);
        // Unzip the boot image file
        string source;
        cphwdump = conf[e_new].getDump();
        source = bootimgdir + "/" + s_BOOTPREFIX + cphwdump.second + ".zip";
        removeCpMauCxcFiles(sidedir);
        //Common::unzip(source, sidedir);
        unzip(source, sidedir, hwcinfo.getCpId());

        // Create FW directory
        const string& fwdir = sidedir + "/" + s_FWUPGRADE;
        Common::createDir(fwdir);
        // Copy FW configuration
        source = datadir + "/boot/fw/" + s_URLF + "." + cphwdump.second;
        dest = fwdir + "/" + s_URLF;
        Common::copyFile(source, dest, false);
        // Create directory for symbolic links

        const string& board = datadir + "/board";
        Common::createDir(board);
        // Get magazine and slot values
        unsigned long mag = hwcinfo.getMagazine();
        unsigned short slot = hwcinfo.getSlot();
        // Assemble magazine and slot values to a symbolic link name
        s.str("");
        s << "m" << (mag & 0xFF) << "_" << ((mag >> 8) & 0xFF) << "_"
                 << ((mag >> 16) & 0xFF) << "_" << (mag >> 24 & 0xFF) << "_s" << (slot & 0xFF);
        // Create link info file
        const string& linkinfo = sidedir + "/board";
        writeText(linkinfo, s.str());
        const string& symlink = board + "/" + s.str();
        if (Common::fileExists(symlink) == false)
        {
            // Create symbolic link
            Common::createSymbolicLink(symlink, sidedir);
        }
        cout << "CPHW dump has been upgraded to the latest version." << endl;
    }
    else if (version == e_bak)
    {
        // XPU config
        Xpuconfig xpuconfig;
        // Check if backup configuration file exists
        file[e_bak] = sidedir + "/" + s_CPHWCONFIG + ".bak";
        if (Common::fileExists(file[e_bak]) == false)
        {
            Exception ex(Exception::e_nobackup_cphw_conf_14);
            ex << "There is no backup CPHW configuration.";
            throw ex;
        }
        const string& xpufilename = xpuconfig.restoreBakToCurSettingStep1(file[e_bak]);
        // Read the configuration file
        conf[e_bak].read(file[e_bak]);
        file[e_cur] = sidedir + "/" + s_CPHWCONFIG + ".cur";
        if (Common::fileExists(file[e_cur]))
        {
            // Check if backup and current configurations are equal
            conf[e_cur].read(file[e_cur]);
            //check for MAUS - introduced for SW-MAU improvement
            if(Common::isMAUS(1001) || Common::isMAUS(1002))
            {
            	cpmauversionsequal=false;
            	if(conf[e_bak].m_cpMauVersionMCP == conf[e_cur].m_cpMauVersionMCP)
            	{
            		cpmauversionsequal=true;
            	}
            }
            if (conf[e_bak] == conf[e_cur] && cpmauversionsequal)
            {
                bool ok = Dialogue::affirm(
                        "Warning: Backup and current configurations are equal,\n"
                        "do you want to continue anyway? "
                );
                cout << endl;
                if (ok == false) return;
            }
        }

        // Unzip the backup boot image file
        string source;
        cphwdump = conf[e_bak].getDump();
        source = bootimgdir + "/" + s_BOOTPREFIX + cphwdump.second + ".zip";
        removeCpMauCxcFiles(sidedir); //removes CXC files present in the previous dump - introduced for sw-mau improvement

        //Common::unzip(source, sidedir);
        unzip(source, sidedir, hwcinfo.getCpId());

        string cpMauFileToSoftLink=Mauconfig::safeReadlink(sidedir + "/CP-MAU");

        // Restore configuration
        conf[e_cur] = conf[e_bak];
        //changes in the config file for CP-MAU - introduced for sw-mau improvement
        if(cpMauFileToSoftLink!=sidedir+"/CP-MAU" && Common::fileExists(sidedir+"/"+conf[e_bak].m_cpMauVersionMCP)){
            conf[e_cur].m_cpMauVersionMCP = conf[e_bak].m_cpMauVersionMCP;
        }
        else if(cpMauFileToSoftLink!=sidedir+"/CP-MAU" && !Common::fileExists(sidedir+"/"+conf[e_bak].m_cpMauVersionMCP)){
          conf[e_cur].m_cpMauVersionMCP = cpMauFileToSoftLink;
        }
        else{
       	 conf[e_cur].m_cpMauVersionMCP = "";
        }
        conf[e_cur].write(file[e_cur]);

        //creates a symbolic link - introduced for sw-mau improvement
        if(!cpMauFileToSoftLink.empty() && cpMauFileToSoftLink!=sidedir+"/CP-MAU" && !conf[e_bak].m_cpMauVersionMCP.empty()
                    && Common::fileExists(sidedir+"/"+conf[e_bak].m_cpMauVersionMCP) && cpMauFileToSoftLink!=conf[e_bak].m_cpMauVersionMCP)
        {
        	Common::deleteSymbolicLink(sidedir+"/CP-MAU");
        	Common::createSymbolicLink(sidedir+"/CP-MAU", conf[e_bak].m_cpMauVersionMCP);
        }

        // XPU config
        if (xpufilename != "")
        {
            xpuconfig.restoreBakToCurSettingStep2(hwcinfo, xpufilename);
        }
        // Copy FW configuration
        source = datadir + "/boot/fw/" + s_URLF + "." + cphwdump.second;
        dest = sidedir + "/" + s_FWUPGRADE + "/" + s_URLF;
        Common::copyFile(source, dest, false);
        cout << "CPHW dump has been restored to the previous version." << endl;
    }
    else
    {
        throw Exception(Exception::e_internal_29, "Illegal dump version.");
    }

    /*
       Handling of Security mitigations during startup and Upgrade scenarios
       Installation phase: 
		It checks for existance of bootopts_bc/cp file in kernel_opts path and 
		file not present. Now it applies the default patches to default file.
       Upgrade+Restore Phase: 
		It checks for existance of bootopts_bc/cp file in kernel_opts path and
		file present and reapplies the pacthes to deafult file.
    */
    char bladeside = tolower(hwcinfo.getSide());
    const string& cpname = Common::toLower(hwcinfo.getName());
    const string& bladename = cpname + bladeside;
    const string& data_bootopts_file = datadir + "/" + "kernel_opts" + "/" + "bootopts_" + bladename;
    string option = "";
    if (Common::fileExists(data_bootopts_file))
    {
      option = " -u";
    } else{
      option = " -i";
    }

    string filePath;
    if( bladeside == NULL)
    {
      filePath = Common::bootOptsTool() + " -c " + cpname + option;
    } else {
      filePath = Common::bootOptsTool() + " -c " + cpname + " -s " + bladeside  + option;
    }
    
    const char *invokeCmd = filePath.c_str();
    int eCode=0;
    try{
      eCode=Common::_execlp(invokeCmd);
    } catch (Exception& ex)
    {
      cerr << "Unable to process the request: " << ex.detailInfo();
    }
    if( eCode !=0 ){
      cerr << "WARNING: Security mitigations are NOT applied." << endl;
    }

    // Update the DUMP information file
    Dumpinfo<Common::e_multicp> dumpinfo;
    const string& dumpfile = sidedir + "/" + s_DUMPINFO;
    if (Common::fileExists(dumpfile))
    {
        dumpinfo.read(dumpfile);
    }
    dumpinfo.setCphwDump(cphwdump.second);
    dumpinfo.write(dumpfile);
}
//----------------------------------------------------------------------------------------
// Configure the PES dump
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::configPES(const HWCInfo& hwcinfo, t_dumpversion version)
{
    const string& datadir = Common::getApzDataPath();
    string dest;
    Pesconfig<Common::e_multicp> conf[3];
    string file[3];
    string cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    string sidedir;
    switch (hwcinfo.getSide())
    {
    case 0:
    case 'A': sidedir = cpdir + "/cpa"; break;
    case 'B': sidedir = cpdir + "/cpb"; break;
    default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
    if (version == e_new)
    {
        // Assemble configuration file name
        ostringstream s;
        s << "config_";
        CPID cpId = hwcinfo.getCpId();
        unsigned short sysType = 1000 * (cpId/1000);
        switch (sysType)
        {
        case ACS_CS_API_HWC_NS::SysType_BC: s << "bc"; break;
        case ACS_CS_API_HWC_NS::SysType_CP: s << "cp"; break;
        default:
            Exception ex(Exception::e_internal_29);
            ex << "Illegal CP identity " << cpId << ".";
            throw ex;
        }
        //s << "_" << hwcinfo.getCPType() << ".ini";
        s << ".ini"; // Ignore CP type, it will not be supported anymore
        // Check if new configuration file exists
        const string& apzvmdir = datadir + "/" + s_APZVMDIR;
        file[e_new] = apzvmdir + "/" + s.str();
        if (Common::fileExists(file[e_new]) == false)
        {
            Exception ex(Exception::e_nonew_pes_conf_18);
            ex << "There is no new Plex Engine configuration.";
            throw ex;
        }
        // Read the configuration file
        conf[e_new].read(file[e_new]);
        // Create CP directory
        Common::createDir(sidedir);
        // Create soft links for cpdir and sidedir (Until it is decided by IO)
        Common::createLink(cpdir);
        Common::createLink(sidedir);
        // Create CP info file
        const string& text = hwcinfo.getSide() == 0? "single": "double";
        const string& cpinfo = cpdir + "/cpinfo";
        writeText(cpinfo, text);
        // Backup previous configuration
        file[e_cur] = sidedir + "/" + s_PESCONFIG + ".ini";
        if (Common::fileExists(file[e_cur]))
        {
            try
            {
                // Check if new and current configurations are equal
                conf[e_cur].read(file[e_cur]);
                if (conf[e_new] == conf[e_cur])
                {
                    bool ok = Dialogue::affirm(
                            "Warning: New and current configurations are equal,\n"
                            "do you want to continue anyway? "
                    );
                    cout << endl;
                    if (ok == false) return;
                }
                file[e_bak] = sidedir + "/" + s_PESCONFIG + ".bak";
                // Backup configuration
                conf[e_bak] = conf[e_cur];
                conf[e_bak].write(file[e_bak]);
            }
            catch (Exception& ex)
            {
                if (ex.errorCode() == Exception::e_parameter_26)
                {
                    cerr << ex << endl;
                    cerr << "Warning: Error(s) were found in current configuration file, "
                            << "no backup was created." << endl << endl;
                }
                else
                {
                    throw ex;
                }
            }
        }
        // Update current configuration
        conf[e_cur] = conf[e_new];
        conf[e_cur].write(file[e_cur]);
        cout << "PES dump has been upgraded to the latest version." << endl;
    }
    else if (version == e_bak)
    {
        // Check if backup configuration file exists
        file[e_bak] = sidedir + "/" + s_PESCONFIG + ".bak";
        if (Common::fileExists(file[e_bak]) == false)
        {
            Exception ex(Exception::e_nobackup_pes_conf_15);
            ex << "There is no backup Plex Engine configuration.";
            throw ex;
        }
        // Read the configuration file
        conf[e_bak].read(file[e_bak]);
        file[e_cur] = sidedir + "/" + s_PESCONFIG + ".ini";
        if (Common::fileExists(file[e_cur]))
        {
            // Check if backup and current configurations are equal
            conf[e_cur].read(file[e_cur]);
            if (conf[e_bak] == conf[e_cur])
            {
                bool ok = Dialogue::affirm(
                        "Warning: Backup and current configurations are equal,\n"
                        "do you want to continue anyway? "
                );
                cout << endl;
                if (ok == false) return;
            }
        }
        // Restore configuration
        conf[e_cur] = conf[e_bak];
        conf[e_cur].write(file[e_cur]);
        cout << "PES dump has been restored to the previous version." << endl;
    }
    else
    {
        throw Exception(Exception::e_internal_29, "Illegal dump version.");
    }
    // Update the DUMP information file
    Dumpinfo<Common::e_multicp> dumpinfo;
    const string& dumpfile = sidedir + "/" + s_DUMPINFO;
    if (Common::fileExists(dumpfile))
    {
        dumpinfo.read(dumpfile);
    }
    dumpinfo.setPesDump(conf[e_cur].getDump());
    dumpinfo.setApzConfig(conf[e_cur].getApzConfig().second);
    dumpinfo.write(dumpfile);
}
//----------------------------------------------------------------------------------------
// Print CPHW configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::printCPHWconfig(const string& conffile)
{
    cout << "CPHW dump";
    if (Common::fileExists(conffile))
    {
        Cphwconfig<Common::e_multicp> cphwconf;
        cphwconf.read(conffile);
        cout << ":" << endl << endl;
        if (cphwconf.getDump().second.substr(0,3) == "LZY")
        {
            cout << Cphwconfig<Common::e_multicp>::s_HEADING_LZY << endl;
        }
        else
        {
            cout << Cphwconfig<Common::e_multicp>::s_HEADING << endl;
        }
        //cphwconf.read(conffile);
        (void)cout.setf(ios::left, ios::adjustfield);
        cout << cphwconf.getDump().second << endl;
        (void)cout.unsetf(ios::left); // to restore the default
    }
    else
    {
        cout << " does not exist." << endl;
    }
    cout << endl;
}
//----------------------------------------------------------------------------------------
// Print PES configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::printPESconfig(const string& conffile)
{
    cout << "PES dump";
    if (Common::fileExists(conffile))
    {
        Pesconfig<Common::e_multicp> pesconf;
        pesconf.read(conffile);
        cout << ":" << endl << endl;
        if (pesconf.getDump().substr(0,3) == "LZY")
        {
            cout << Pesconfig<Common::e_multicp>::s_HEADING_LZY << endl;
        }
	else
        {
            cout << Pesconfig<Common::e_multicp>::s_HEADING << endl;
        }
        //pesconf.read(conffile);
        (void)cout.setf(ios::left, ios::adjustfield);
        cout << setw(24) << pesconf.getApzVm().second << setw(24) << pesconf.getDump()
                 << endl << endl;
        const Pesconfig<Common::e_multicp>::PAIR& apzconf = pesconf.getApzConfig();
        const string& memfile =
                Common::getApzDataPath() + "/" + s_APZVMDIR + "/" + apzconf.second;
        Memconfig memconf;
        memconf.read(memfile);
        (void)cout.setf(ios::left, ios::adjustfield);
	readReservedFlag();
        if(reserved_flag)
        {
           cout << setw(24) << "PRODUCT" << Memconfig::s_HEADING << setw(11) << Memconfig::s_HEADING_3 << endl;
	   if(!memconf.readReservedValue())
	   {
	     cout << setw(24) << apzconf.second << memconf;
             cout << setw(15) << "0" ;
	   }
	   else
	     cout << setw(24) << apzconf.second << memconf;
        }
        else
        {
        	// uabmha: Use the heading without "RPB-S"
        	cout << setw(24) << "PRODUCT" << Memconfig::s_HEADING << endl;
		cout << setw(24) << apzconf.second << memconf << endl;

        }
        // Mod. needed also in operator<<(ostream &s, const MemConfig &conf)
        // used below, to not print out anything in the RPB-S column...
        (void)cout.unsetf(ios::left); // to restore the default
    }
    else
    {
        cout << " does not exist." << endl;
    }
    cout << endl << endl;
}
//----------------------------------------------------------------------------------------
// Print configurations
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::printConfigurations(const HWCInfo& hwcinfo, t_dumpversion version)
{
    const string& datadir = Common::getApzDataPath();
    string cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    string sidedir;
    bool singleSide = false;
    char cpSide;
    switch (hwcinfo.getSide())
    {
    case 0:
        singleSide = true;
    case 'A':
        sidedir = cpdir + "/cpa";
        cpSide = 'A';
        break;
    case 'B':
        sidedir = cpdir + "/cpb";
        cpSide = 'B';
        break;
    default:
        throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
    string conffile;
    switch (version)
    {
    case e_all:
    case e_new:
    {
        // Assemble configuration file name
        ostringstream s;
        s << "config_";
        CPID cpId = hwcinfo.getCpId();
        unsigned short sysType = 1000 * (cpId/1000);
        switch (sysType)
        {
        case ACS_CS_API_HWC_NS::SysType_BC: s << "bc"; break;
        case ACS_CS_API_HWC_NS::SysType_CP: s << "cp"; break;
        default:
            Exception ex(Exception::e_internal_29);
            ex << "Illegal CP identity " << cpId << ".";
            throw ex;
        }
        //s << "_" << hwcinfo.getCPType() << ".ini";
        s << ".ini"; // Ignore CP type, it will not be supported anymore
        cout << "New settings" << endl << endl;
        conffile = datadir + "/" + s_BOOTIMAGE + "/" + s.str();
        //if (!singleSide) // SPX HS46538
        //    cout << "CP side " << cpSide << " ";
        printCPHWconfig(conffile);
        // Print XPU configuration
        if (singleSide)
        {
            Xpuconfig xpucfg;
            xpucfg.displayNewSetting(hwcinfo);
        }
        // Moved from printCPHWconfig
        cout << endl;
        conffile = datadir + "/" + s_APZVMDIR + "/" + s.str();
        //if (!singleSide) //SPX HS46538
        //    cout << "CP side " << cpSide << " ";
        printPESconfig(conffile);
    }
    if (version != e_all) break;
    case e_cur:     //lint !e616
    {
        cout << "Current settings" << endl << endl;
        conffile = sidedir + "/" + s_CPHWCONFIG + ".cur";
        //if (!singleSide) HS46538
        //    cout << "CP side " << cpSide << " ";
        printCPHWconfig(conffile);
        // Print XPU configuration
        if (singleSide)
        {
            Xpuconfig xpucfg;
            xpucfg.displayCurSetting(hwcinfo);
        }
        // Moved from printCPHWconfig
        cout << endl;
        conffile = sidedir + "/" + s_PESCONFIG + ".ini";
        //if (!singleSide) HS46538
        //    cout << "CP side " << cpSide << " ";
        printPESconfig(conffile);
    }
    if (version != e_all) break;
    case e_bak:        //lint !e616
    {
        cout << "Backup settings" << endl << endl;
        conffile = sidedir + "/" + s_CPHWCONFIG + ".bak";
        //if (!singleSide) HS46538
        //    cout << "CP side " << cpSide << " ";
        printCPHWconfig(conffile);
        // Print XPU configuration
        if (singleSide)
        {
            Xpuconfig xpucfg;
            xpucfg.displayBakSetting(hwcinfo);
        }
        // Moved from printCPHWconfig
        cout << endl;
        conffile = sidedir + "/" + s_PESCONFIG + ".bak";
        //if (!singleSide) HS46538
        //    cout << "CP side " << cpSide << " ";
        printPESconfig(conffile);
    }
    break;
    default:
        throw Exception(Exception::e_internal_29, "Illegal version.");
    }
}
//----------------------------------------------------------------------------------------
// Print configurations for double sided CP's in a multi CP system
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::printDoubleSidedConfigurations(const HWCInfo& hwcinfo, t_dumpversion version)
{
    const string& datadir = Common::getApzDataPath();
    string cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    string sidedir1, sidedir2;
    sidedir1 = cpdir + "/cpa";
    sidedir2 = cpdir + "/cpb";
    string conffile;
    switch (version)
    {
    case e_all:
    case e_new:
    {
        // Assemble configuration file name
        ostringstream s;
        s << "config_";
        CPID cpId = hwcinfo.getCpId();
        unsigned short sysType = 1000 * (cpId/1000);
        switch (sysType)
        {
        case ACS_CS_API_HWC_NS::SysType_BC: s << "bc"; break;
        case ACS_CS_API_HWC_NS::SysType_CP: s << "cp"; break;
        default:
            Exception ex(Exception::e_internal_29);
            ex << "Illegal CP identity " << cpId << ".";
            throw ex;
        }
        //s << "_" << hwcinfo.getCPType() << ".ini";
        s << ".ini"; // Ignore CP type, it will not be supported anymore
        cout << "New settings" << endl << endl;
        conffile = datadir + "/" + s_BOOTIMAGE + "/" + s.str();
        cout << "CP side A ";
        printCPHWconfig(conffile);
        conffile = datadir + "/" + s_APZVMDIR + "/" + s.str();
        cout << "CP side A ";
        printPESconfig(conffile);
        conffile = datadir + "/" + s_BOOTIMAGE + "/" + s.str();
        cout << "CP side B ";
        printCPHWconfig(conffile);
        conffile = datadir + "/" + s_APZVMDIR + "/" + s.str();
        cout << "CP side B ";
        printPESconfig(conffile);
    }
    if (version != e_all) break;
    case e_cur:     //lint !e616
    {
        cout << "Current settings" << endl << endl;
        conffile = sidedir1 + "/" + s_CPHWCONFIG + ".cur";
        cout << "CP side A ";
        printCPHWconfig(conffile);
        conffile = sidedir1 + "/" + s_PESCONFIG + ".ini";
        cout << "CP side A ";
        printPESconfig(conffile);
        conffile = sidedir2 + "/" + s_CPHWCONFIG + ".cur";
        cout << "CP side B ";
        printCPHWconfig(conffile);
        conffile = sidedir2 + "/" + s_PESCONFIG + ".ini";
        cout << "CP side B ";
        printPESconfig(conffile);
    }
    if (version != e_all) break;
    case e_bak:        //lint !e616
    {
        cout << "Backup settings" << endl << endl;
        conffile = sidedir1 + "/" + s_CPHWCONFIG + ".bak";
        cout << "CP side A ";
        printCPHWconfig(conffile);
        conffile = sidedir1 + "/" + s_PESCONFIG + ".bak";
        cout << "CP side A ";
        printPESconfig(conffile);
        conffile = sidedir2 + "/" + s_CPHWCONFIG + ".bak";
        cout << "CP side B ";
        printCPHWconfig(conffile);
        conffile = sidedir2 + "/" + s_PESCONFIG + ".bak";
        cout << "CP side B ";
        printPESconfig(conffile);
    }
    break;
    default:
        throw Exception(Exception::e_internal_29, "Illegal version.");
    }
}

//----------------------------------------------------------------------------------------
// Check for the presence of RESERVED flag in CDA file
//----------------------------------------------------------------------------------------
void Dumps<Common::e_multicp>::readReservedFlag()
{
	Common::VECTOR vec;
	const string& path = Common::getApzDataPath() + "/" + s_APZVMDIR;
	DIR* dp = opendir(path.c_str());
	if (dp == NULL)
	{
		(void)cout.unsetf(ios::left); // to restore the default
		Exception ex(Exception::e_system_28, errno);
		ex << "Failed to open directory '" << path << "'.";
		throw ex;
	}
	int index(0);
	dirent* dirp;
	while ((dirp = readdir(dp)) != NULL)
	{
		const string& name = dirp->d_name;
		if (name.find(Memconfig::s_CONFFILE) == 0)
		{
			Common::VECTORCITER iter = find(vec.begin(), vec.end(), name);
			if (iter == vec.end())
			{
				index++;
				vec.push_back(name);
				Memconfig memconf;
				memconf.read(path + "/" + name);
				if(memconf.getReservedFlag())
				{
					reserved_flag = true;
					break;
				}
				else
				{
					reserved_flag = false;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------
// List memory configurations
//----------------------------------------------------------------------------------------
Common::VECTOR
Dumps<Common::e_multicp>::listMemoryConfigs(const string& apzconf)
{
    (void)cout.setf(ios::left, ios::adjustfield);
    cout << "Memory configurations:" << endl << endl;
    Memconfig memconf;
    readReservedFlag();
    if(reserved_flag)
    {
    	//Heading should print RESERVED flag
    	cout << setw(5) << "NO" << setw(22) << "PRODUCT" << setw(24)
    			<< Memconfig::s_HEADING <<  setw(11) << Memconfig::s_HEADING_3 << "   CURRENT" << endl;
    }
    else
    {
    	cout << setw(5) << "NO" << setw(22) << "PRODUCT" << setw(27)
    			<< Memconfig::s_HEADING << "CURRENT" << endl;
    }
    Common::VECTOR vec;
    // List available memory configurations
    const string& path = Common::getApzDataPath() + "/" + s_APZVMDIR;
    DIR* dp = opendir(path.c_str());
    if (dp == NULL)
    {
        (void)cout.unsetf(ios::left); // to restore the default
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open directory '" << path << "'.";
        throw ex;
    }
    int index(0);
    dirent* dirp;
    while ((dirp = readdir(dp)) != NULL)
    {
        const string& name = dirp->d_name;
        if (name.find(Memconfig::s_CONFFILE) == 0)
        {
            Common::VECTORCITER iter = find(vec.begin(), vec.end(), name);
            if (iter == vec.end())
            {
                index++;
                vec.push_back(name);
                Memconfig memconf;
                memconf.read(path + "/" + name);
                (void) cout.setf(ios::right, ios::adjustfield);
                cout << setw(2) << index << "   ";
                (void) cout.setf(ios::left, ios::adjustfield);
		cout << setw(22) << name << memconf;
		if(!memconf.getReservedFlag())
		{
		   cout << setw(15) << "0";
		}
                // uabmha: No RPB-S column printed now, so
                if (name == apzconf)
                {
                    cout << "   yes";
                }
                cout << endl;
            }
        }
    }
    if(reserved_flag)
    {
    	cout << endl;
    	cout << "CDAs with RESERVED values are provided for compatibility among boards with different memory sizes."<<endl;
    }

    (void)cout.unsetf(ios::left); // to restore the default
    closedir(dp);
    return vec;
}
//----------------------------------------------------------------------------------------
// Select memory configuration
//----------------------------------------------------------------------------------------
bool
Dumps<Common::e_multicp>::selectMemoryConfig(Pesconfig<Common::e_multicp>& conf)
{
    const Pesconfig<Common::e_multicp>::PAIR& apzconf = conf.getApzConfig();
    const vector<string>& vec = listMemoryConfigs(conf.getApzConfig().second);
    // Change memory configuration
    cout << endl;
    bool loop(true);
    do
    {
        Dialogue dialogue;
        const string& input = Common::toUpper(dialogue.userInput(
                "Select a number or type 'q' to quit"
        ));
        if (input == "Q")
        {
            return false;
        }
        else
        {
            try
            {
                unsigned int num = (unsigned int)Common::strToInt(input);
                if (num > 0 && num <= vec.size())
                {

                   conf.setApzConfig(make_pair(apzconf.first, vec[num - 1]));    // path to be removed?
                    loop = false;
                }
                else
                {
                    throw Exception(Exception::e_illegalnumber_10, "Illegal number.");
                }
            }
            catch (Exception& ex)
            {
                cerr << ex << endl;
            }
        }
        cout << endl;
    }
    while (loop);
    return true;
}
//----------------------------------------------------------------------------------------
// Memory configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::memoryConfig(const HWCInfo& hwcinfo)
{
    const string& datadir = Common::getApzDataPath();
    const string& cpdir = datadir + "/" + Common::toLower(hwcinfo.getName());
    string sidedir;
    switch (hwcinfo.getSide())
    {
    case 0:
    case 'A': sidedir = cpdir + "/cpa"; break;
    case 'B': sidedir = cpdir + "/cpb"; break;
    default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
    const string& pesfile = sidedir + "/" + s_PESCONFIG + ".ini";
    if (Common::fileExists(pesfile) == false)
    {
        Exception ex(Exception::e_noconf_cp_or_side_16);
        ex << "There is no configuration for this CP or CP side." << endl;
        throw ex;
    }
    Pesconfig<Common::e_multicp> pesconf;
    pesconf.read(pesfile);
    // Select memory configuration
    if (selectMemoryConfig(pesconf))
    {
        // Update the DUMP information file
        Dumpinfo<Common::e_multicp> dumpinfo;
        const string& dumpfile = sidedir + "/" + s_DUMPINFO;
        dumpinfo.read(dumpfile);
        dumpinfo.setApzConfig(pesconf.getApzConfig().second);
        dumpinfo.write(dumpfile);
        cout << "Configuration was saved." << endl;
    }
    else
    {
        cout << "No changes were made." << endl;
    }
    pesconf.write(pesfile);
}
//----------------------------------------------------------------------------------------
// Blade memory configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_multicp>::bladeMemoryConfig(const HWCInfo& hwcinfo)
{
    Xpuconfig xpuconfig;
    xpuconfig.read(hwcinfo);
    xpuconfig.listXpuConfiguration();
    xpuconfig.selectXpuConfiguration();
}
//----------------------------------------------------------------------------------------
// Remove configuration
//----------------------------------------------------------------------------------------
void 
Dumps<Common::e_multicp>::removeConfig(const string& cpname, char cpside)         
{
    enum {e_current, e_other};
    const string& datadir = Common::getApzDataPath();
    const string& cpdir = datadir + "/" + cpname;
    if (Common::fileExists(cpdir) == false)
    {
        Exception ex(Exception::e_noconf_cp_or_side_16);
        ex << "There is no configuration for this CP or CP side." << endl;
        throw ex;
    }
    // Read CP info
    const string& cpinfo = cpdir + "/cpinfo";
    string text = readText(cpinfo);
    string sidedir[2];
    switch (cpside)
    {
    case 0:
        if (text == "single")
        {
            sidedir[e_current] = cpdir + "/CPA";
            sidedir[e_other] = cpdir + "/CPB";
        }
        else
        {
            Exception ex(Exception::e_illopt_sysconf_116);
            ex << "CP side is not allowed for a single sided CP.";
            throw ex;
        }
        break;
    case 'A':
        if (text == "double")
        {
            sidedir[e_current] = cpdir + "/CPA";
            sidedir[e_other] = cpdir + "/CPB";
        }
        else
        {
            Exception ex(Exception::e_illopt_sysconf_116);
            ex << "CP side is required for a dual CP.";
            throw ex;
        }
        break;
    case 'B':
        if (text == "double")
        {
            sidedir[e_current] = cpdir + "/CPB";
            sidedir[e_other] = cpdir + "/CPA";
        }
        else
        {
            Exception ex(Exception::e_illopt_sysconf_116);
            ex << "CP side is required for a dual CP.";
            throw ex;
        }
        break;
    default:
        throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
    if (Common::fileExists(sidedir[e_current]) == true)
    {
        const string& linkinfo = sidedir[e_current] + "/board";
        if (Common::fileExists(linkinfo) == true)
        {
            // Find symbolic link
            const string& symlink = datadir + "/board/" + readText(linkinfo);
            if (Common::fileExists(symlink) == true)
            {
                // Delete symbolic link
                try
                {
                    Common::deleteSymbolicLink(symlink);
                }
                catch (Exception& ex)
                {
                    cerr << ex << endl;
                }
            }
        }
        // Delete CP configuration
        Common::deleteDirTree(sidedir[e_current]);
    }
    else
    {
        Exception ex(Exception::e_noconf_cp_or_side_16);
        ex << "There is no configuration for this CP or CP side." << endl;
        throw ex;
    }
    if (Common::fileExists(sidedir[e_other]) == false)
    {
        // CP directory empty - remove it
        Common::deleteDirTree(cpdir);
    }
    cout << "Configuration has been removed." << endl;
}

//=================
// Check if input string is under number format or not
//=================
bool Dumps<Common::e_multicp>::isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

//=================
// Check if format of default file is new one or old
//=================
bool Dumps<Common::e_multicp>::isNewDefaultFile(const std::string& path)
{
    string firstLine;

    // Parse the file content to file stream
    ifstream file(path.c_str());
    if (!file.is_open())
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open file '" << path << "'.";
        throw ex;
    }
    //get first line of file
    getline(file,firstLine);

    // return true if fisrt line is "#!ipxe"
    return (firstLine == "#!ipxe");
}
bool Dumps<Common::e_multicp>::displayNewSm(const std::string& pxedir,
        std::vector <std::string>& database,
        std::vector <std::string>& invalidDb)
{
    vector<string>::iterator it;
    ostringstream s;
    string filename;
    int count = 0;
    bool valid = false;
    // Display all default file name
    for (it=database.begin(); it!=database.end(); ++it)
    {
        const string& path = pxedir + *it;
        ifstream file(path.c_str());
        valid = false;

        filename = *it;
        count++;
        if (database.size() > 1)
        {
            s << "["<< count << "] "<< *it << endl;
        }
        else
        {
            s << *it << endl;
        }
        // Check if can open file
        if (!file.is_open())
        {
            Exception ex(Exception::e_system_28, errno);
            ex << "Failed to open '" << filename << "' file.";
            throw ex;
        }

        //Get file content
        string content( (std::istreambuf_iterator<char>(file) ),
                (std::istreambuf_iterator<char>()    ) );

        // Pattern for searching gepX
        boost::regex gepId_smId("(:cpub_[^\n]+)|(sm=[^\\s]+.)");
        // Pattern for getting sm content
       // boost::regex smId("(sm=[^\\s]+.)");
        // Save match strings
        boost::smatch smMatch;
        std::string::const_iterator it=content.begin();
        std::string::const_iterator end=content.end();
        //Search whole file

        while (boost::regex_search(it, end, smMatch,gepId_smId))
        {
            // Found gepID
            if (smMatch[1].matched)
            {
                // Remove ":cpub_" string, then extract pattern gepId
                string gepTxt(smMatch[1].first + 6, smMatch[1].second);
                it = smMatch[0].second;

                if (boost::regex_search(it, end, smMatch, gepId_smId))
                {

                    if (smMatch[2].matched)
                    {
                    // Found smID, then extract pattern sm=Id without spacing
                    string smTxt(smMatch[2].first, smMatch[2].second-1);
                    s << "    " << gepTxt << ": [" << smTxt << "]"<< endl;
                    valid = true;
                    }
                    else if (smMatch[1].matched)
                    {
                        break;
                    }
                    else
                    {
                        // Do nothing
                    }
                }

            }
            it =  smMatch[0].second;
        }// end while

        if (valid == false)
        {
            s << "    [Invalid file]" << endl;
            invalidDb.push_back(filename);
            //cout << "Error: " << filename << " file is invalid" << endl;
            //return valid;
        }

    } // end for

    // Display all default information if they are invalid
    cout << s.str() << endl;
    return true;
}

bool Dumps<Common::e_multicp>::displayOldSm(const std::string& path)
{
    ostringstream s;
    ifstream file(path.c_str());
    if (!file.is_open())
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open 'default' file.";
        throw ex;
    }

    //Get file content
    string content( (std::istreambuf_iterator<char>(file) ),
            (std::istreambuf_iterator<char>()    ) );

    // Pattern "APPEND "
    boost::regex appendTxt("(APPEND\\s)");

    // Pattern smID
    boost::regex smId("(sm=[^\\s]+.)");
    boost::smatch smMatch;
    std::string::const_iterator it=content.begin();
    std::string::const_iterator end=content.end();

    // Just display one default file
    s << "default: ";
    bool valid = false;
    //get first line of file
    while (boost::regex_search(it, end, smMatch,appendTxt))
    {
        if (smMatch[1].matched)
        {
            // Extract pattern :gepId
            //string gepTxt(smMatch[1].first + 1, smMatch[1].second);
            it = smMatch[0].second;
            if (boost::regex_search(it, end, smMatch, smId))
            {
                // Extract pattern sm=Id
                string smTxt(smMatch[1].first, smMatch[1].second-1);
                s << "[" << smTxt << "]"<< endl;

                valid = true;
            }
        }
        it =  smMatch[0].second;
    }// end while

    if (valid)
    {
        cout << s.str();
    }
    else
    {
        cout << "Error: default file is invalid" << endl;
    }

    return true;
} // end if
//========================================================================================
// One CP system
//========================================================================================
const string& Dumps<Common::e_onecp>::s_BOOTPREFIXDOT =     "boot_image.";
const string& Dumps<Common::e_onecp>::s_UPBBUPGRADE =       "fw/upbb_ubgrade";
const string& Dumps<Common::e_onecp>::s_RPHMIUPGRADE =      "fw/rphmi_ubgrade";
const string& Dumps<Common::e_onecp>::s_CPBBUPGRADE =       "fw/cpbb_ubgrade";
const char Dumps<Common::e_onecp>::s_APCONFBIN[] =          "ACS/CXC1371147";
bool Dumps<Common::e_onecp>::reserved_flag = false;
//----------------------------------------------------------------------------------------
// Get APZ type
//----------------------------------------------------------------------------------------
Dumps<Common::e_onecp>::t_apzType
Dumps<Common::e_onecp>::getApzType()
{
    acs_apgcc_paramhandling apar;
    int cpType;
    ACS_CC_ReturnType result;
    OmHandler immHandle;
    std::vector<std::string> dnList;
    result = immHandle.Init();
    if (result != ACS_CC_SUCCESS)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to read CP type from PHA.";
        throw ex;
    }
    result = immHandle.getClassInstances("AxeFunctions", dnList);
    if (result != ACS_CC_SUCCESS)
    {
        immHandle.Finalize();
        Exception ex(Exception::e_system_28);
        ex << "Failed to read CP type from PHA.";
        throw ex;
    }
    immHandle.Finalize();
    const char *pAttrName = "apzProtocolType";
    result = apar.getParameter(dnList[0], pAttrName, &cpType);
    if (result != ACS_CC_SUCCESS)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to read CP type from PHA.";
        throw ex;
    } 
    switch (cpType)
    {
    case 0:
    case 1:
    {
        Exception ex(Exception::e_system_28);
        ex << "Illegal CP type.";
        throw ex;
    }
    case 2: return e_apz21240;
    case 3: return e_apz21250;
    case 4: return e_apz21255;
    default:
    {
        Exception ex(Exception::e_system_28);
        ex << "Undefined CP type.";
        throw ex;
    }
    }
}
//----------------------------------------------------------------------------------------
// Install CPHW package
//----------------------------------------------------------------------------------------
void Dumps<Common::e_onecp>::installCPHW(const string& packagePath) {
    // Get APZ type
    //t_apzType apzType = getApzType();
    string absolutePackagePath;
    DumpsBase::validatePath(packagePath); //validates the package path with nbi path.
    absolutePackagePath = (string) m_cpmwPath;
    absolutePackagePath.append("/");
    absolutePackagePath.append(m_dumpName);
    // Check if package file exists
    if (Common::fileExists(absolutePackagePath) == false)
    {
        Exception ex(Exception::e_packagefilenotfound_13);
        ex << "Package file '" << m_logCpmwPath << "/" << m_dumpName
                << "' was not found.";
        throw ex;
    }
    // Create temporary directory
    const string& tempdir = Common::createTempDir();
    // 1.) Pre-test the package (at first level here) before unzipping it.
    // Bails out with Application error if something in package is corrupt.
    Common::testunzip(absolutePackagePath, 1);
    // Unzip the package to a temporary directory
    Common::unzip(absolutePackagePath, tempdir);
    // TR HN20934 fix
    string hwconfigfile;
    hwconfigfile = tempdir + "/" + s_CPHWCONFIG + ".new";
    if (Common::fileExists(hwconfigfile) == false) // TR HN20934 fix
    {
        // Delete zip directory
        try {
            Common::deleteDirTree(tempdir);
        } catch (Exception& ex) {
            cout << "Warning: " << ex.detailInfo() << endl;
        }
        Exception ex(Exception::e_WrongDump_21);
        ex << "It should be the CPHW dump to be installed";
        throw ex;
    }
    // 2.) Pre-test the package (at second level here)
    //     before unzipping the second level later.
    // Bails out with Application error if something in package
    // at second level is corrupt.
    Common::testunzip(tempdir, 2);
    // Set file attributes to normal
    Common::setAttrsRec(tempdir);
    //cout << "APZ type = " << apzType << endl;
    string source;
    string dest;
    source = tempdir + "/" + s_CPHWCONFIG + ".new";
    // Read the configuration file
    Cphwconfig<Common::e_onecp> cphwconf;
    cphwconf.read(source);
    string dump[2];
    dump[0] = cphwconf['A'].getDump().second;
    dump[1] = cphwconf['B'].getDump().second;
    // Create the boot image directory
    const string& datadir = Common::getApzDataPath();
    const string& bootimgdir = datadir + "/" + s_BOOTIMAGE;
    Common::createDir(bootimgdir);
    // Create the following symbolic links (Until it is decided by IO)
    //for directory structure
    Common::createSymlinks();
    // Copy the configuration file
    dest = bootimgdir + "/" + s_CPHWCONFIG + ".new";
    Common::copyFile(source, dest, false);
    // Copy boot image file
    source = tempdir + "/" + s_BOOTPREFIX + dump[0] + ".zip";
    dest = bootimgdir + "/" + s_BOOTPREFIX + dump[0] + ".zip";
    Common::copyFile(source, dest, false);
    if (dump[1] != dump[0]) {
        source = tempdir + "/" + s_BOOTPREFIX + dump[1] + ".zip";
        dest = bootimgdir + "/" + s_BOOTPREFIX + dump[1] + ".zip";
        Common::copyFile(source, dest, false);
    }
    // Unzip the ruf image file
    source = tempdir + "/" + s_RUFPREFIX + dump[0] + ".zip";
    dest = datadir + "/boot";
    Common::unzip(source, dest);
    if (dump[1] != dump[0]) {
        source = tempdir + "" + s_RUFPREFIX + dump[1] + ".zip";
        Common::unzip(source, dest);
    }
    // Delete zip directory
    try {
        Common::deleteDirTree(tempdir);
    } catch (Exception& ex) {
        cout << "Warning: " << ex.detailInfo() << endl;
    }
}
//----------------------------------------------------------------------------------------
// Install PES package
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::installPES(const string& packagePath)
{
    string absolutePackagePath;
    DumpsBase::validatePath(packagePath); //validates the package path with nbi path.
    // Get APZ type
    t_apzType apzType = getApzType();
    absolutePackagePath = (string) m_cpmwPath;
    absolutePackagePath.append("/");
    absolutePackagePath.append(m_dumpName);
    // Check if package file exists
    if (Common::fileExists(absolutePackagePath) == false)
    {
        Exception ex(Exception::e_packagefilenotfound_13);
        ex << "Package file '" <<m_logCpmwPath<<"/"<<m_dumpName<< "' was not found.";
        throw ex;
    }
    // Create temporary directory
    const string& tempdir = Common::createTempDir();
    // 1.) Pre-test the package (at first level here) before unzipping it.
    // Bails out with Application error if something in package is corrupt.
    Common::testunzip(absolutePackagePath, 1);
    // Unzip the package to a temporary directory
    Common::unzip(absolutePackagePath, tempdir);
    // TR HN20934 fix
    string pesconfigfile;
    pesconfigfile = tempdir + "/" + s_CPHWCONFIG + ".new";
    if (Common::fileExists(pesconfigfile) == true) // TR HN20934 fix
    {
        // Delete zip directory
        try
        {
            Common::deleteDirTree(tempdir);
        }
        catch (Exception& ex)
        {
            cout << "Warning: " << ex.detailInfo() << endl;
        }
        Exception ex(Exception::e_WrongDump_21);
        ex << "It should be the PES dump to be installed";
        throw ex;
    }
    // Set file attributes to normal
    Common::setAttrsRec(tempdir);
    // Check the package
    string source;
    string dest;
    source = tempdir + "/" + s_PESCONFIG + ".ini";
    // Read the configuration file
    Pesconfig<Common::e_onecp> pesconf;
    pesconf.read(source);
    PesconfigBase::PAIR apzvm[2];
    apzvm[0] = pesconf['A'].getApzVm();
    apzvm[1] = pesconf['B'].getApzVm();
    // Create the apz_vm directory
    const string& apzvmdir = Common::getApzDataPath() + "/" + s_APZVMDIR;
    Common::createDir(apzvmdir);
    // Create symbolic link for apz_vm directory (Until it is decided by IO)
    const string vmsymlink = "/data/apz/data/boot/APZ_VM";
    const string vmdir = "apz_vm";
    if (Common::fileExists(vmsymlink) == false)
    {
        // Create symbolic link
        Common::createSymbolicLink(vmsymlink, vmdir);
    }
    // Create the following symbolic links (Until it is decided by IO)
    //for directory structure (This is for, in case hw dump is not installed yet)
    Common::createSymlinks();
    // Create the image directory
    const string& imagedir = Common::getApzDataPath() + "/" + s_BOOTIMAGE;
    Common::createDir(imagedir);
    // Copy the configuration file
    dest = imagedir + "/" + s_PESCONFIG + ".new";
    Common::copyFile(source, dest, false);
    // Copy boot image file
    source = tempdir + "/" + apzvm[0].second;
    dest = apzvmdir + "/" + apzvm[0].second;
    Common::copyFile(source, dest, false);
    if (apzvm[1].second != apzvm[0].second)
    {
        source = tempdir + "/" + apzvm[1].second;
        dest = apzvmdir + "/" + apzvm[1].second;
        Common::copyFile(source, dest, false);
    }
    // Copy PCIH image file
    if (apzType == e_apz21250)        // APZ21250
    {
        PesconfigBase::PAIR pcihsrv[2];
        pcihsrv[0] = pesconf['A'].getPcihSrv();
        pcihsrv[1] = pesconf['B'].getPcihSrv();
        source = tempdir + "/" + pcihsrv[0].second;
        dest = apzvmdir + "/" + pcihsrv[0].second;
        Common::copyFile(source, dest, false);
        if (pcihsrv[1].second != pcihsrv[0].second)
        {
            source = tempdir + "/" + pcihsrv[1].second;
            dest = apzvmdir + "/" + pcihsrv[1].second;
            Common::copyFile(source, dest, false);
        }
    }
    // Copy memory configuration file(s)
    source = tempdir + "/" + s_CONFIGFILE;
    dest = Common::getApzDataPath() + "/" + s_APZVMDIR;
    Common::copyFiles(source, dest, false);
    // Delete zip directory
    try
    {
        Common::deleteDirTree(tempdir);
    }
    catch (Exception& ex)
    {
        cout << "Warning: " << ex.detailInfo() << endl;
    }
}
//----------------------------------------------------------------------------------------
// Configure the CPHW dump
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::configCPHW(char side, t_dumpversion version) {
    // Get APZ type
    //t_apzType apzType = getApzType();
    //cout << "configCPHW apzType =" << apzType << endl;
    const string& datadir = Common::getApzDataPath();
    string dest;
    Cphwconfig<Common::e_onecp> conf[3];
    string file[3];
    int index=Mauconfig::checkSide(side);
    bool cpmauversionsequal=true;
    const string& imagedir = datadir + "/" + s_BOOTIMAGE;
    string cphwdump;
    if (version == e_new) {
        // Check if new configuration file exists
        file[e_new] = imagedir + "/" + s_CPHWCONFIG + ".new";
        if (Common::fileExists(file[e_new]) == false) {
            Exception ex(Exception::e_nonew_cphw_conf_17);
            ex << "There is no new CPHW configuration.";
            throw ex;
        }
        // Read the new configuration file
        conf[e_new].read(file[e_new]);
        // Backup current configuration
        file[e_cur] = imagedir + "/" + s_CPHWCONFIG + ".cur";
        if (Common::fileExists(file[e_cur])) {
            // Check if new and current configurations are equal
            conf[e_cur].read(file[e_cur]);
            if(Common::isMAUS(1001))
            {
            	cpmauversionsequal=false;
            	if(conf[e_new].m_cpMauCurVersion[index] == conf[e_cur].m_cpMauCurVersion[index])
            	{
            		cpmauversionsequal=true;
            	}
            }
            if (conf[e_new][side] == conf[e_cur][side] && cpmauversionsequal) {
                bool ok = Dialogue::affirm(
                        "Warning: New and current configurations are equal,\n"
                        "do you want to continue anyway? ");
                cout << endl;
                if (ok == false)
                    return;
            }
            // Check if backup configuration file exists
            file[e_bak] = imagedir + "/" + s_CPHWCONFIG + ".bak";
            if (Common::fileExists(file[e_bak]) == true) {
                // Read the backup configuration file
                conf[e_bak].read(file[e_bak]);
            }
            // Backup configuration
            conf[e_bak][side] = conf[e_cur][side];
            conf[e_bak].m_cpMauCurVersion[index] = conf[e_cur].m_cpMauCurVersion[index];
            conf[e_bak].write(file[e_bak]);
        }
        // Update current configuration
        conf[e_cur][side] = conf[e_new][side];
        conf[e_cur].m_cpMauCurVersion[index] = conf[e_new].m_cpMauCurVersion[index];
        conf[e_cur].write(file[e_cur]);
        string source;
        string temp;
        cphwdump = conf[e_new][side].getDump().second;
        //cout << "Configure APZ type = " << apzType << endl;
        // Create CP directory
        temp = "/ ";
        temp[1] = (char) tolower(side);
        const string& cpdir = imagedir + temp;
        Common::createDir(cpdir);
        // Unzip the boot image file
        source = imagedir + "/" + s_BOOTPREFIX + cphwdump + ".zip";
        removeCpMauCxcFiles(cpdir);
        //Common::unzip(source, cpdir);
        unzip(source, cpdir, 1001);

        // Copy FW configuration
        temp = "/cp ";
        temp[3] = (char) tolower(side);
        dest = datadir + "/boot" + temp + "/" + s_FWUPGRADE + "/" + s_URLF;
        source = dest + "." + cphwdump;
        Common::copyFile(source, dest, false);

	cout << "CPHW dump has been upgraded to the latest version." << endl; 

    } else if (version == e_bak) {
        // Check if backup configuration file exists
        file[e_bak] = imagedir + "/" + s_CPHWCONFIG + ".bak";
        if (Common::fileExists(file[e_bak]) == false) {
            Exception ex(Exception::e_nobackup_cphw_conf_14);
            ex << "There is no backup CPHW configuration.";
            throw ex;
        }
        // Read the configuration file
        conf[e_bak].read(file[e_bak]);
        if (conf[e_bak][side].getDump().second.empty()) {
            Exception ex(Exception::e_nobackup_cphw_conf_14);
            ex << "There is no backup CPHW configuration.";
            throw ex;
        }
        file[e_cur] = imagedir + "/" + s_CPHWCONFIG + ".cur";
        if (Common::fileExists(file[e_cur])) {
            // Check if backup and current configurations are equal
            conf[e_cur].read(file[e_cur]);
            //check for MAUS - introduced for sw-mau improvement
            if(Common::isMAUS(1001))
            {
            	cpmauversionsequal=false;
            	if(conf[e_bak].m_cpMauCurVersion[index] == conf[e_cur].m_cpMauCurVersion[index])
            	{
            		cpmauversionsequal=true;
            	}
            }

            if (conf[e_bak][side] == conf[e_cur][side] && cpmauversionsequal) {
                bool ok = Dialogue::affirm(
                        "Warning: Backup and current configurations are equal,\n"
                        "do you want to continue anyway? ");
                cout << endl;
                if (ok == false)
                    return;
            }
        }
        // Unzip the backup boot image file
        string source;
        cphwdump = conf[e_bak][side].getDump().second;
        source = imagedir + "/" + s_BOOTPREFIX + cphwdump + ".zip";
        string temp("/ ");
        temp[1] = (char) tolower(side);
        const string& cpdir = imagedir + temp;
        removeCpMauCxcFiles(cpdir); //removes CXC files present in the previous dump - introduced for sw-mau improvement

        //Common::unzip(source, cpdir);
        unzip(source, cpdir, 1001);


        string cpMauFileToSoftLink=Mauconfig::safeReadlink(cpdir+"/CP-MAU");
	
        // Restore configuration
        conf[e_cur][side] = conf[e_bak][side];
        //changes in the config file for CP-MAU - introduced for sw-mau improvement
        if(cpMauFileToSoftLink!=cpdir+"/CP-MAU" && Common::fileExists(cpdir+"/"+conf[e_bak].m_cpMauCurVersion[index])){
            conf[e_cur].m_cpMauCurVersion[index] = conf[e_bak].m_cpMauCurVersion[index];
        }
        else if(cpMauFileToSoftLink!=cpdir+"/CP-MAU" && !Common::fileExists(cpdir+"/"+conf[e_bak].m_cpMauCurVersion[index])){
          conf[e_cur].m_cpMauCurVersion[index] = cpMauFileToSoftLink;
        }
        else{
       	 conf[e_cur].m_cpMauCurVersion[index] = "";
        }
        conf[e_cur].write(file[e_cur]);


        //creates a symbolic link - introduced for sw-mau improvement
        if(!cpMauFileToSoftLink.empty() && cpMauFileToSoftLink!=cpdir+"/CP-MAU" && !conf[e_bak].m_cpMauCurVersion[index].empty()
                && Common::fileExists(cpdir+"/"+conf[e_bak].m_cpMauCurVersion[index]) && cpMauFileToSoftLink!=conf[e_bak].m_cpMauCurVersion[index])
        {
        	Common::deleteSymbolicLink(cpdir+"/CP-MAU");
        	Common::createSymbolicLink(cpdir+"/CP-MAU", conf[e_bak].m_cpMauCurVersion[index]);
        }

        // Copy FW configuration
        temp = "/cp ";
        temp[3] = (char) tolower(side);
        dest = datadir + "/boot" + temp + "/" + s_FWUPGRADE + "/" + s_URLF;
        source = dest + "." + cphwdump;
        Common::copyFile(source, dest, false);
        cout << "CPHW dump has been restored to the previous version." << endl;
    } else {
        throw Exception(Exception::e_internal_29, "Illegal dump version.");
    }

    /*
       Handling of Security mitigations during startup and Upgrade scenarios
       Installation phase:
                It checks for existance of bootopts_cp file in kernel_opts path and
                if file not present, it applies the default patches to default file.
       Upgrade+Restore Phase:
                It checks for existance of bootopts_cp file in kernel_opts path and
                if file is present, it reapplies the pacthes to deafult file.
    */
    char bladeside = tolower(side);
    const string& data_bootopts_file = datadir + "/" + "kernel_opts" + "/" + "bootopts_cp" + bladeside;
    string filePath;
    int eCode=0;

    if (Common::fileExists(data_bootopts_file))
    {
        filePath = Common::bootOptsTool() + " -s " + bladeside + " -u ";
    }
    else
    {
        filePath = Common::bootOptsTool() + " -s " + bladeside + " -i ";
    }

    const char *invokeCmd = filePath.c_str();

    try{
        eCode=Common::_execlp(invokeCmd);
    } catch (Exception& ex)
    {
        cerr << "Unable to process the request: " << ex.detailInfo();
    }
    if( eCode !=0 )
    {
        cerr << "WARNING: Security mitigations are NOT applied." << endl;
    }

    // Update the DUMP information file
    Dumpinfo<Common::e_onecp> dumpinfo;
    const string& dumpfile = imagedir + "/" + s_DUMPINFO;
    if (Common::fileExists(dumpfile)) {
        dumpinfo.read(dumpfile);
    }
    dumpinfo[(int) side].setCphwDump(cphwdump);
    dumpinfo.write(dumpfile);
}
//----------------------------------------------------------------------------------------
// Configure the PES dump
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::configPES(char side, t_dumpversion version)
{
    const string& datadir = Common::getApzDataPath();
    string dest;
    Pesconfig<Common::e_onecp> conf[3];
    string file[3];
    const string& imagedir = datadir + "/" + s_BOOTIMAGE;
    if (version == e_new)
    {
        // Check if new configuration file exists
        file[e_new] = imagedir + "/" + s_PESCONFIG + ".new";
        if (Common::fileExists(file[e_new]) == false)
        {
            Exception ex(Exception::e_nonew_pes_conf_18);
            ex << "There is no new Plex Engine configuration.";
            throw ex;
        }
        // Read the configuration file
        conf[e_new].read(file[e_new]);
        // Backup previous configuration
        file[e_cur] = imagedir + "/" + s_PESCONFIG + ".ini";
        if (Common::fileExists(file[e_cur]))
        {
            // Check if new and current configurations are equal
            conf[e_cur].read(file[e_cur]);
            if (conf[e_new][side] == conf[e_cur][side])
            {
                bool ok = Dialogue::affirm(
                        "Warning: New and current configurations are equal,\n"
                        "do you want to continue anyway? "
                );
                cout << endl;
                if (ok == false) return;
            }
            // Check if backup configuration file exists
            file[e_bak] = imagedir + "/" + s_PESCONFIG + ".bak";
            if (Common::fileExists(file[e_bak]) == true)
            {
                // Read the backup configuration file
                conf[e_bak].read(file[e_bak]);
            }
            // Backup configuration
            conf[e_bak][side] = conf[e_cur][side];
            conf[e_bak].write(file[e_bak]);
        }
        // Update current configuration
        conf[e_cur][side] = conf[e_new][side];
        conf[e_cur].write(file[e_cur]);
        cout << "PES dump has been upgraded to the latest version." << endl;
    }
    else if (version == e_bak)
    {
        // Check if backup configuration file exists
        file[e_bak] = imagedir + "/" + s_PESCONFIG + ".bak";
        if (Common::fileExists(file[e_bak]) == false)
        {
            Exception ex(Exception::e_nobackup_pes_conf_15);
            ex << "There is no backup Plex Engine configuration.";
            throw ex;
        }
        // Read the configuration file
        conf[e_bak].read(file[e_bak]);
        if (conf[e_bak][side].getApzVm().second.empty())
        {
            Exception ex(Exception::e_nobackup_pes_conf_15);
            ex << "There is no backup Plex Engine configuration.";
            throw ex;
        }
        file[e_cur] = imagedir + "/" + s_PESCONFIG + ".ini";
        if (Common::fileExists(file[e_cur]))
        {
            // Check if backup and current configurations are equal
            conf[e_cur].read(file[e_cur]);
            if (conf[e_bak][side] == conf[e_cur][side])
            {
                bool ok = Dialogue::affirm(
                        "Warning: Backup and current configurations are equal,\n"
                        "do you want to continue anyway? "
                );
                cout << endl;
                if (ok == false) return;
            }
        }
        // Restore configuration
        conf[e_cur][side] = conf[e_bak][side];
        conf[e_cur].write(file[e_cur]);
        cout << "PES dump has been restored to the previous version." << endl;
    }
    else
    {
        throw Exception(Exception::e_internal_29, "Illegal dump version.");
    }
    // Update the DUMP information file
    Dumpinfo<Common::e_onecp> dumpinfo;
    const string& dumpfile = imagedir + "/" + s_DUMPINFO;
    if (Common::fileExists(dumpfile))
    {
        dumpinfo.read(dumpfile);
    }
    dumpinfo[(int)side].setPesDump(conf[e_cur][side].getDump());
    dumpinfo[(int)side].setApzConfig(conf[e_cur][side].getApzConfig().second);
    dumpinfo.write(dumpfile);
}

//----------------------------------------------------------------------------------------
// Print CPHW configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::printCPHWconfig(const string& conffile, char side)
{
    cout << "CPHW dump";
    if (Common::fileExists(conffile))
    {
        Cphwconfig<Common::e_onecp> cphwconf;
        cphwconf.read(conffile);
        const string& dump = cphwconf[(int)side].getDump().second;
        if (dump.empty() == false)
        {
            cout << ":" << endl << endl;
            if(dump.substr(0,3)=="LZY")
            {
                cout << CphwconfigBase::s_HEADING_LZY << endl;
            }
            else
            {
                cout << CphwconfigBase::s_HEADING << endl;
            }
            //cphwconf.read(conffile);
            (void)cout.setf(ios::left, ios::adjustfield);
            cout << dump << endl;
            (void)cout.unsetf(ios::left); // to restore the default
        }
        else
        {
            cout << " does not exist." << endl;
        }
    }
    else
    {
        cout << " does not exist." << endl;
    }
    cout << endl << endl;
}
//----------------------------------------------------------------------------------------
// Print PES configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::printPESconfig(const string& conffile, char side, t_apzType apzType)
{
    // Check for 21240 and set legacy flag for config file format
    bool legacy = (apzType == e_apz21240);
    cout << "PES dump";
    if (Common::fileExists(conffile))
    {
        Pesconfig<Common::e_onecp> pesconf;
        pesconf.read(conffile);
        const string& apzvm = pesconf[(int)side].getApzVm().second;
        if (apzvm.empty() == false)
        {
            cout << ":" << endl << endl;
            if(pesconf[side].getDump().substr(0,3)=="LZY")
            {
                cout << PesconfigBase::s_HEADING_LZY << endl;
            }
            else
            {
                cout << PesconfigBase::s_HEADING << endl;
            }
            //pesconf.read(conffile);
            (void)cout.setf(ios::left, ios::adjustfield);
            cout << setw(24) << apzvm << setw(24) << pesconf[(int)side].getDump() << endl << endl;
            const PesconfigBase::PAIR& apzconf = pesconf[(int)side].getApzConfig();
            const string& memfile =
                    Common::getApzDataPath() + "/" + s_APZVMDIR + "/" + apzconf.second;
            Memconfig memconf;
            memconf.read(memfile, legacy);
            (void)cout.setf(ios::left, ios::adjustfield);
            cout << setw(24) << "PRODUCT";
            if (apzType == e_apz21250)
            {
                // Printout heading with RPB-S + column content :
                cout << Memconfig::s_HEADING << Memconfig::s_HEADING_2 << endl;
                cout << setw(24) << apzconf.second << memconf;
                cout << "   " << setw(5) << (memconf.getRpbsFlag()? "yes": "no ") << endl;
            }
            else
            {
		readReservedFlag();
            	if(reserved_flag)
            	{
                   cout << Memconfig::s_HEADING << setw(11) << Memconfig::s_HEADING_3 << endl;
                   if(!memconf.readReservedValue())
                   {
                      cout << setw(24) << apzconf.second << memconf;
                      cout << setw(15) << "0" ;
                   }
                   else
                      cout << setw(24) << apzconf.second << memconf;
                }
            	else
            	{
            		// Printout heading without RPB-S and no column content :
            		cout << Memconfig::s_HEADING << endl;
			cout << setw(24) << apzconf.second << memconf;	         
            	}
            }
            (void)cout.unsetf(ios::left); // to restore the default
        }
        else
        {
            // PES dump not found
            cout << " does not exist." << endl;
        }
    }
    else
    {
        cout << " does not exist." << endl;
    }
    cout << endl << endl;
}
//----------------------------------------------------------------------------------------
// Print configurations
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::printConfigurations(t_dumpversion version, char side)
{
    // Get APZ type
    t_apzType apzType = getApzType();
    // bool legacy = (apzType == e_apz21240);
    const string& datadir = Common::getApzDataPath();
    const string& imagedir = datadir + "/" + s_BOOTIMAGE;
    string conffile;
    switch (version)
    {
    case e_all:
    case e_new:
    {
        cout << "New settings" << endl << endl;
        if (side == 'A')
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".new";
            cout << "CP side A ";
            printCPHWconfig(conffile, 'A');
            conffile = imagedir + "/" + s_PESCONFIG + ".new";
            cout << "CP side A ";
            printPESconfig(conffile, 'A', apzType);
        }
        else if(side == 'B')
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".new";
            cout << "CP side B ";
            printCPHWconfig(conffile, 'B');
            conffile = imagedir + "/" + s_PESCONFIG + ".new";
            cout << "CP side B ";
            printPESconfig(conffile, 'B', apzType);
        }
        else
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".new";
            cout << "CP side A ";
            printCPHWconfig(conffile, 'A');
            conffile = imagedir + "/" + s_PESCONFIG + ".new";
            cout << "CP side A ";
            printPESconfig(conffile, 'A', apzType);
            conffile = imagedir + "/" + s_CPHWCONFIG + ".new";
            cout << "CP side B ";
            printCPHWconfig(conffile, 'B');
            conffile = imagedir + "/" + s_PESCONFIG + ".new";
            cout << "CP side B ";
            printPESconfig(conffile, 'B', apzType);
        }
    }
    if (version != e_all) break;
    case e_cur:     //lint !e616
    {
        cout << "Current settings" << endl << endl;
        if (side == 'A')
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".cur";
            cout << "CP side A ";
            printCPHWconfig(conffile, 'A');
            conffile = imagedir + "/" + s_PESCONFIG + ".ini";
            cout << "CP side A ";
            printPESconfig(conffile, 'A', apzType);
        }
        else if (side == 'B')
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".cur";
            cout << "CP side B ";
            printCPHWconfig(conffile, 'B');
            conffile = imagedir + "/" + s_PESCONFIG + ".ini";
            cout << "CP side B ";
            printPESconfig(conffile, 'B', apzType);
        }
        else
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".cur";
            cout << "CP side A ";
            printCPHWconfig(conffile, 'A');
            conffile = imagedir + "/" + s_PESCONFIG + ".ini";
            cout << "CP side A ";
            printPESconfig(conffile, 'A', apzType);
            conffile = imagedir + "/" + s_CPHWCONFIG + ".cur";
            cout << "CP side B ";
            printCPHWconfig(conffile, 'B');
            conffile = imagedir + "/" + s_PESCONFIG + ".ini";
            cout << "CP side B ";
            printPESconfig(conffile, 'B', apzType);
        }
    }
    if (version != e_all) break;
    case e_bak:        //lint !e616
    {
        cout << "Backup settings" << endl << endl;
        if (side == 'A')
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".bak";
            cout << "CP side A ";
            printCPHWconfig(conffile, 'A');
            conffile = imagedir + "/" + s_PESCONFIG + ".bak";
            cout << "CP side A ";
            printPESconfig(conffile, 'A', apzType);
        }
        else if (side == 'B')
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".bak";
            cout << "CP side B ";
            printCPHWconfig(conffile, 'B');
            conffile = imagedir + "/" + s_PESCONFIG + ".bak";
            cout << "CP side B ";
            printPESconfig(conffile, 'B', apzType);
        }
        else
        {
            conffile = imagedir + "/" + s_CPHWCONFIG + ".bak";
            cout << "CP side A ";
            printCPHWconfig(conffile, 'A');
            conffile = imagedir + "/" + s_PESCONFIG + ".bak";
            cout << "CP side A ";
            printPESconfig(conffile, 'A', apzType);
            conffile = imagedir + "/" + s_CPHWCONFIG + ".bak";
            cout << "CP side B ";
            printCPHWconfig(conffile, 'B');
            conffile = imagedir + "/" + s_PESCONFIG + ".bak";
            cout << "CP side B ";
            printPESconfig(conffile, 'B', apzType);
        }
    }
    break;
    default:
        throw Exception(Exception::e_internal_29, "Illegal version.");
    }
}

//----------------------------------------------------------------------------------------
// Check for the presence of RESERVED flag in CDA file
//----------------------------------------------------------------------------------------
void Dumps<Common::e_onecp>::readReservedFlag()
{
	Common::VECTOR vec;
	const string& path = Common::getApzDataPath() + "/" + s_APZVMDIR;
	DIR* dp = opendir(path.c_str());
	if (dp == NULL)
	{
		(void)cout.unsetf(ios::left); // to restore the default
		Exception ex(Exception::e_system_28, errno);
		ex << "Failed to open directory '" << path << "'.";
		throw ex;
	}
	int index(0);
	dirent* dirp;
	while ((dirp = readdir(dp)) != NULL)
	{
		const string& name = dirp->d_name;
		if (name.find(Memconfig::s_CONFFILE) == 0)
		{
			Common::VECTORCITER iter = find(vec.begin(), vec.end(), name);
			if (iter == vec.end())
			{
				index++;
				vec.push_back(name);
				Memconfig memconf;
				memconf.read(path + "/" + name);
				if(memconf.getReservedFlag())
				{
					reserved_flag = true;
					break;
				}
				else
				{
					reserved_flag = false;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------
// List memory configurations
//----------------------------------------------------------------------------------------
Common::VECTOR
Dumps<Common::e_onecp>::listMemoryConfigs(const string& apzconf)
{
    // Get APZ type
    t_apzType apzType = getApzType();
    bool legacy = (apzType == e_apz21240);
    (void)cout.setf(ios::left, ios::adjustfield);
    cout << "Memory configurations:" << endl << endl;
    if (apzType == e_apz21250)
    {
        // Print an RPB-S heading too...
        cout << setw(5) << "NO" << setw(22) << "PRODUCT" << setw(35)
                   << Memconfig::s_HEADING << Memconfig::s_HEADING_2 << "CURRENT" << endl;
    }
	else
	{
		 readReservedFlag();
		 if(reserved_flag)
		 {
			 //Heading should print RESERVED flag
			 cout << setw(5) << "NO" << setw(22) << "PRODUCT" << setw(24)
					 << Memconfig::s_HEADING <<  setw(11) << Memconfig::s_HEADING_3 << "   CURRENT" << endl;
		 }
		 else
		 {
			 // No heading for RPB-S will be printed...
			 cout << setw(5) << "NO" << setw(22) << "PRODUCT" << setw(27) << Memconfig::s_HEADING << "CURRENT" << endl;
		 }
	}
    Common::VECTOR vec;
    // List available memory configurations
    const string& path = Common::getApzDataPath() + "/" + s_APZVMDIR;
    DIR* dp = opendir(path.c_str());
    if (dp == NULL)
    {
        (void)cout.unsetf(ios::left); // to restore the default
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open directory '" << path << "'.";
        throw ex;
    }
    int index(0);
    dirent* dirp;
    while ((dirp = readdir(dp)) != NULL)
    {
        const string& name = dirp->d_name;
        if (name.find(Memconfig::s_CONFFILE) == 0)
        {
            Common::VECTORCITER iter = find(vec.begin(), vec.end(), name);
            if (iter == vec.end())
            {
                index++;
                vec.push_back(name);
                Memconfig memconf;
                memconf.read(path + "/" + name, legacy);
                (void) cout.setf(ios::right, ios::adjustfield);
                cout << setw(2) << index << "   ";
                (void) cout.setf(ios::left, ios::adjustfield);
                cout << setw(22) << name << memconf;
                // uabmha: Show RPB-S column for 21250 only... :
                // (Note: the operator<< for memconf above is adapted accordingly.)
                if (apzType == e_apz21250)
                {
                    cout << "   " << setw(5) << (memconf.getRpbsFlag() ? "yes" : "no ")
                            << "   ";
                }
		if(!memconf.getReservedFlag())
                {
                    cout << setw(15) << "0";
                }
                if (name == apzconf)
                {
                    cout << "   yes";
                }
                cout << endl;
            }
        }
    }
    if(reserved_flag)
    {
    	cout << endl;
    	cout << "CDAs with RESERVED values are provided for compatibility among boards with different memory sizes."<<endl;
    }
    (void)cout.unsetf(ios::left); // to restore the default
    closedir(dp);
    return vec;
}
//----------------------------------------------------------------------------------------
// Select memory configuration
//----------------------------------------------------------------------------------------
bool
Dumps<Common::e_onecp>::selectMemoryConfig(Pesconfig<Common::e_onecp>& conf, char side)
{
    const PesconfigBase::PAIR& apzconf = conf[(int)side].getApzConfig();
    const vector<string>& vec = listMemoryConfigs(conf[(int)side].getApzConfig().second);
    // Change memory configuration
    cout << endl;
    bool loop(true);
    do
    {
        Dialogue dialogue;
        const string& input = Common::toUpper(dialogue.userInput(
                "Select a number or type 'q' to quit"
        ));
        if (input == "Q")
        {
            return false;
        }
        else
        {
            try
            {
                unsigned int num = (unsigned int)Common::strToInt(input);
                if (num > 0 && num <= vec.size())
                {
                    conf[(int)side].setApzConfig(
                            make_pair(apzconf.first, vec[num - 1])
                    );    // path to be removed?
                    loop = false;
                }
                else
                {
                    throw Exception(Exception::e_illegalnumber_10, "Illegal number.");
                }
            }
            catch (Exception& ex)
            {
                cerr << ex << endl;
            }
        }
        cout << endl;
    }
    while (loop);
    return true;
}
//----------------------------------------------------------------------------------------
// Memory configuration
//----------------------------------------------------------------------------------------
void
Dumps<Common::e_onecp>::memoryConfig(char side)
{
    const string& datadir = Common::getApzDataPath();
    const string& imagedir = datadir + "/" + s_BOOTIMAGE;
    const string& pesfile = imagedir + "/" + s_PESCONFIG + ".ini";
    if (Common::fileExists(pesfile) == false)
    {
        Exception ex(Exception::e_noconf_cp_or_side_16);
        ex << "There is no configuration for this CP or CP side." << endl;
        throw ex;
    }
    Pesconfig<Common::e_onecp> pesconf;
    pesconf.read(pesfile);
    // Select memory configuration
    if (selectMemoryConfig(pesconf, side))
    {
        // Update the DUMP information file
        Dumpinfo<Common::e_onecp> dumpinfo;
        const string& dumpfile = imagedir + "/" + s_DUMPINFO;
        dumpinfo.read(dumpfile);
        dumpinfo[(int)side].setApzConfig(pesconf[(int)side].getApzConfig().second);
        dumpinfo.write(dumpfile);
        cout << "Configuration was saved." << endl;
    }
    else
    {
        cout << "No changes were made." << endl;
    }
    pesconf.write(pesfile);
}
