//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      common.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2020. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of 
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only 
//      with the written permission from Ericsson AB or in accordance with 
//      the terms and conditions stipulated in the agreement/contract under 
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      This class contains common methods used by cfet.
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
//             2010-11-04   XSCVERE     CFET was updated with Boost File system Functions
//      B      2015-11-20   EGIAAVA     Update to introduce -o option for vAPZ
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <common.h>
#include <exception.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <boost/filesystem.hpp>
#include <acs_prc_api.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std;
namespace fs = boost::filesystem;

std::string Common::s_apzpath;

//----------------------------------------------------------------------------------------
// Convert characters in string to upper case 
//----------------------------------------------------------------------------------------
string
Common::toUpper(const string& str)
{
   string tstr(str);
   (void)transform(tstr.begin(), tstr.end(), tstr.begin(), ::toupper);
   return tstr;
}

//----------------------------------------------------------------------------------------
// Convert characters in string to lower case 
//----------------------------------------------------------------------------------------
string
Common::toLower(const string& str)
{
   string tstr(str);
   (void)transform(tstr.begin(), tstr.end(), tstr.begin(), ::tolower);
   return tstr;
}

//----------------------------------------------------------------------------------------
//  Translate string to integer value
//----------------------------------------------------------------------------------------
int64_t
Common::strToInt(const std::string& val)
{
    string::const_iterator iter = val.begin();
    int64_t var;

    char* p;
    errno = 0;
    if (*iter == '-')
    {
        var = strtol(val.c_str(), &p, 0);
    }
    else
    {
        var = strtoul(val.c_str(), &p, 0);
    }
    if (*p != ' ' && *p != '\t' && *p != 0)
    {
        //Exception ex(Exception::e_general_1);
        //ex << "Integer was expected.";
        //throw ex;
        // Throw int indicating "integer was expected" to next level
        throw 1;
    }
    if (errno == ERANGE)
    {
        //Exception ex(Exception::e_general_1);
        //ex << "Overflow occurred.";
        //throw ex;
        // Throw int indicating "overflow occurred" to next level
        throw 2;
    }
    return var;
}

//----------------------------------------------------------------------------------------
// Check if a file exists
//----------------------------------------------------------------------------------------
bool
Common::fileExists(const string& fileName)
{ 
    string tName(fileName);
    bool result;

    // Remove trailing backslash
    string::iterator it = tName.end();
    it--;
    if (*it == '/')
    {
        tName.erase(it);
    }

    try
        {
            fs::path p(tName.c_str());
            result = fs::exists(p);
        }
        catch (fs::filesystem_error e)
        {
            cerr << "Failed to get information for file '" << tName << "'.";
            cerr << e.what();
            return false;
        }
        if (result == true)
            return true;
        else
            return false;
}

//----------------------------------------------------------------------------------------
// Create a hard link
//----------------------------------------------------------------------------------------
void
Common::createHardLink(const string& linkName, const string& fileName)
{
    try
    {
        fs::path p1(fileName.c_str());
        fs::path p2(linkName.c_str());
        fs::create_hard_link(p1,p2);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create hard link for file '" << fileName << "'.";
        ex << e.what();
        throw ex;
    }

}

//----------------------------------------------------------------------------------------
// Delete a file
//----------------------------------------------------------------------------------------
void
Common::deleteFile(const string& fileName)
{
    bool result;
    try
    {
        fs::path p(fileName.c_str());
        result = fs::remove(p);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        // ex << "Failed to delete file '" << fileName << "'.";
        ex << "No write permission to file default." << endl;
        // ex << fileName << ".";
        // ex << e.what();
        throw ex;
    }
    if (result == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to delete file '" << fileName << "'.";
        throw ex;
    }
}


//----------------------------------------------------------------------------------------
// Delete a file
//----------------------------------------------------------------------------------------
void
Common::deleteFile(const string& fileName, Exception& exVal)
{
    bool result;
    try
    {
        fs::path p(fileName.c_str());
        result = fs::remove(p);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        // ex << "Failed to delete file '" << fileName << "'.";
        ex << "No write permission to file default." << endl;
        // ex << fileName << ".";
        // ex << e.what();
        exVal = ex;
    }
    if (result == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to delete file '" << fileName << "'.";
        exVal = ex;
    }
}


//----------------------------------------------------------------------------------------^M
// Remove the debug file (auto_exec)
//----------------------------------------------------------------------------------------^M
void
Common::removeDebugFile()
{
    const string auto_exec = getApzDataPath() + "/boot/image/auto_exec" ; 
    if(Common::fileExists(auto_exec))
    {
        Common::deleteFile(auto_exec);
    }

}
//----------------------------------------------------------------------------------------
// Move a file
//----------------------------------------------------------------------------------------
void
Common::moveFile(const string& source, const string& dest, bool)
{
    try
    {
        fs::path psource(source.c_str());
        fs::path pdest(dest.c_str());
        fs::rename(psource,pdest);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to move file '" << source << "' to '" << dest << "'.";
        ex << e.what();
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Copy a file
//----------------------------------------------------------------------------------------
void
Common::copyFile(const string& source, const string& dest, bool failIfExists)
{
    if (failIfExists)
    {
        if (fileExists(dest) == true)
        {
            Exception ex(Exception::e_system_28);
            ex << "Target file '" << dest << "' already exists.";
            throw ex;
        }
    }

    if (fileExists(dest) == true)
    {
        deleteFile(dest);
    }

    try
    {
        fs::path pfrom(source.c_str());
        fs::path psource(dest.c_str());
        fs::copy_file(pfrom,psource);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to copy file '" << source << "' to '" << dest << "'.";
        ex << e.what();
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Copy a file
//----------------------------------------------------------------------------------------
void
Common::copyFile(const string& source, const string& dest, Exception& exVal)
{

    if (fileExists(dest) == true)
    {
        deleteFile(dest, exVal);		// Nothrow deleteFile
    }

    try
    {
        fs::path pfrom(source.c_str());
        fs::path psource(dest.c_str());
        fs::copy_file(pfrom,psource);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to copy file '" << source << "' to '" << dest << "'.";
        ex << e.what();
        exVal = ex;
    }
}


//----------------------------------------------------------------------------------------
// Copy files
//----------------------------------------------------------------------------------------
void
Common::copyFiles(const string& source, const string& dest, bool failIfExists)
{
    size_t pos = source.find_last_of('/');
    const string& path = source.substr(0, pos);
    
    DIR* dp = opendir(path.c_str());
    if (dp == NULL)
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open directory '" << path << "'.";
        throw ex;
    }

    string pattern = source.substr(pos + 1);
    pos = pattern.find_first_of('*');
    pattern.erase(pos);
    
    dirent* dirp;
    while ((dirp = readdir(dp)) != NULL)
    {
        const string& name = dirp->d_name;
        if (name != "." && name != "..")
        {
            const string& tsource = path + "/" + name;
            if (fileExists(tsource) == false)
                {
    
                    Exception ex(Exception::e_system_28);
                    ex << "Target file '" << dest << "' does not exist.";
                    closedir(dp);
                    throw ex;
                }
            fs::path p(tsource.c_str());
            if(fs::is_directory(p)== false)
                {
                // File
                if (name.find(pattern) == 0)
                {
                    const string& tdest = dest + "/" + name;
                    copyFile(tsource, tdest, failIfExists);
                }
                }
        }
    }
    closedir(dp);
}

//----------------------------------------------------------------------------------------
// Create a directory
//----------------------------------------------------------------------------------------
void
Common::createDir(const string& path)
{
    string::size_type pos(0);
    while (pos != string::npos)
    {
        pos++;
        pos = path.find_first_of('/', pos);
        const string& dir = path.substr(0, pos);
        try
        {
            fs::path p(dir.c_str());
            fs::create_directory(p);
        }
        catch (fs::filesystem_error e)
        {
            Exception ex(Exception::e_system_28);
            ex << "Failed to create directory '" << dir << "'.";
            ex << e.what();
            throw ex;
        }
    }
}

//----------------------------------------------------------------------------------------
// Remove a directory
//----------------------------------------------------------------------------------------
void
Common::removeDirectory(const string& path)
{
    bool result;
    try
    {
        fs::path p(path.c_str());
        result = fs::remove(p);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to remove directory '" << path << "'.";
        ex << e.what();
        throw ex;
    }
    if (result == false)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to remove directory '" << path << "'.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Delete a directory recursively
//----------------------------------------------------------------------------------------
void
Common::deleteDirTree(const string& path)
{

    unsigned long nofiledelete; // Number of files removed
    try
    {
        fs::path p(path.c_str());
        nofiledelete = fs::remove_all(p);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to remove directory '" << path << "'.";
        ex << e.what();
        throw ex;
    }
    if (nofiledelete == 0)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to remove directory '" << path << "'.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Set attributes to normal for files recursively
//----------------------------------------------------------------------------------------
void
Common::setAttrsRec(const string& path)
{
    DIR* dp = opendir(path.c_str());
    if (dp == NULL)
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to open directory '" << path << "'.";
        throw ex;
    }
    
    dirent* dirp;
    while ((dirp = readdir(dp)) != NULL)
    {
        const string& name = dirp->d_name;
        if (name != "." && name != "..")
        {
            int res;
            const string& tpath = path + "/" + name;
            if (fileExists(tpath) == false)
            {
                Exception ex(Exception::e_system_28);
                ex << "Failed to get information for file '" << tpath << "'.";
                closedir(dp);
                throw ex;
            }
    
            mode_t mode;
            fs::path p(tpath.c_str());
            if(fs::is_directory(p)== true)
            {
                // Directory
                setAttrsRec(tpath.c_str());
                mode = 0755;
            }
            else
            {
                // File
                mode = 0644;
            }
    
            res = chmod(tpath.c_str(), mode);
            if (res)
            {
                Exception ex(Exception::e_system_28, errno);
                ex << "Failed to change permissions for file '" << tpath << "'.";
                closedir(dp);
                throw ex;
            }
        }
    }
    closedir(dp);
}

//----------------------------------------------------------------------------------------
// Create symbolic link
//----------------------------------------------------------------------------------------
void
Common::createSymbolicLink(const string& link, const string& dir)
{
    try
    {
        fs::path p1(dir.c_str());
        fs::path p2(link.c_str());
        fs::create_symlink(p1,p2);
    }
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create symbolic link for file '" << dir << "'.";
        ex << e.what();
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Delete symbolic link
//----------------------------------------------------------------------------------------
void
Common::deleteSymbolicLink(const string& link)
{
    int res = unlink(link.c_str());
    if (res)
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to delete symbolic link '" << link << "'.";
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Execute a console command
//----------------------------------------------------------------------------------------
int Common::executeCommand(const string& command, ostream& os)
{
    char cbuf[512];
    
    FILE* fp = popen((command + " 2>&1").c_str(), "r");
    if (fp == 0)
    {
    
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to launch command." << command << "'.";
        throw ex;
    }
    while (fgets(cbuf, 512, fp) != NULL)
    {
        os << cbuf;
    }
    return pclose(fp);
}


//----------------------------------------------------------------------------------------
// Create temporary directory
//----------------------------------------------------------------------------------------
string
Common::createTempDir()
{
    // Create temporary directory
    char tempdir[] = "/tmp/cfet-XXXXXX";
    const char* ptr = mkdtemp(tempdir);
    if (ptr == 0)
    {
        Exception ex(Exception::e_system_28, errno);
        ex << "Failed to create temporary directory.";
        throw ex;
    }
    return tempdir;
}

//----------------------------------------------------------------------------------------
// Pre-test the zip file.
// This function is called before unzip to pre-test the source
// before unzipping it, so that a corrupt file is caught before some part is extracted.
// Like Common::unzip(), it uses a struct of zip command forms to pick from.
//----------------------------------------------------------------------------------------
void
Common::testunzip(const string& source, const int level)
{
    ostringstream os; 
    stringstream command;
    command << "unzip -t " << "\"";
    
    if (level == 1)
    {
        // We are testing a zipfile name, source is just a filename
        // so command string is now already prepared, except for the final "
        // E.g. : zipc -t "source"     (Note: "source" is here "packagePath"
        command << source << "\"";
    }
    else if (level == 2)
    {
        // We now want to test what is unpacked at the second level
        // of the zipfile into a source, which here is a tempdir
        // that should be followed by /*, so add that to command string.
        // E.g. : zipc -t "source/*"   (Note: "source" is here "tempdir"
        // NOTE: Had to be changed into boot_image* and ruf_image* below
        // since zipc -t could not handle the other plain files at this level...
        command << source << "/*.zip\"";            //lint !e661 !e662
    }
    
    //cout << "testunzip command is: " << command.str() << endl;
    
    // Execute the zip command with test option
    int error = Common::executeCommand(command.str(), os);
    //cout << "Testunzip debug: " << "outs: " << os.str() << " errs: " << os.str() << endl;
    //cout << "Testunzip debug: " << "error code from zip command was: " << error << endl;
    
    if ((error != 0) ||
        (os.str().find("Error") != string::npos) ||
        (os.str().find("NOT OK") != string::npos))
    {
        // The final return code was non-zero or an "Error" or "NOT OK" string was found
        // from a failed test "in the middle", in the zip command
        // output returned in the errs string...
        Exception ex(Exception::e_application_27);
        ex << "Command execution failed for command '" << command.str() << "'." << endl;
        ex << os.str();        
        ex << "Unzip rejected. Package failed zip file integrity test "
              "(possibly corrupt package)." << endl;
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Unzip file
//----------------------------------------------------------------------------------------
void
Common::unzip(const string& source, const string& dest)
{
    // Find zip packager program
    ostringstream os; 
    stringstream command;
    command << "unzip -o " << "\"" << source << "\"" << " -d " << "\"" << dest << "\"";
    // Execute unzip command
    int error = Common::executeCommand(command.str(), os);
    if (error != 0)
    {
        Exception ex(Exception::e_application_27);
        ex << "Command execution failed for command '" << command.str() << "'." << endl;
        ex << os.str();
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Get APG version
//----------------------------------------------------------------------------------------
#if 0
ACS_APGCC_HwVer_ReturnTypeT
Common::getApgVersion() 
{
    // uint16_t len = 0;
    // int ver = GetHWVersion(0, &len);
    //
    // switch (ver)
    // {
    // case -2:
    //     throw Exception(Exception::e_internal_29, "String buffer too small.");
    //     break;
    //
    // case -1:
    //     throw Exception(Exception::e_internal_29, "Undefined hardware version.");
    //     break;
    //
    // case 4:     break;    // APG50
    //
    // default:
    //     throw Exception(Exception::e_internal_29, "Unexpected hardware version.");
    // }
    //
    // return ver;

    ACS_APGCC_HwVer_ReturnTypeT result;
    char hwVersion[64];
    int p_Len=64;
    ACS_APGCC_CommonLib lib;
    result = lib.GetHWVersion(hwVersion, p_Len);
    return result;
}
#endif

//----------------------------------------------------------------------------------------
// Set APZ data path
//----------------------------------------------------------------------------------------
void 
Common::setApzDataPath()
{
	char logicName[16] = "APZ_DATA";
    char path[64];
    int len = 63;

    ACS_APGCC_DNFPath_ReturnTypeT result;
    ACS_APGCC_CommonLib lib;
    result = lib.GetDataDiskPath(logicName, path, len);

    if ( result == ACS_APGCC_DNFPATH_SUCCESS )
    {
        // Are we in active node?
        ACS_PRC_API prc;
        int nodestate = prc.askForNodeState();

        if (nodestate == 1)
        {
            // Yes
            s_apzpath = path;
        }
        else if (nodestate == 2)
        {
            // No, should not run on PASSIVE
            Exception ex(Exception::e_notonactivenode_20);
            // ex << "Command must be executed on the active node ";
            throw ex;
        }
        else
        {
            Exception ex(Exception::e_system_28);
            ex << "Node state is undefined.";
            throw ex;
        }
    }
    else
    {
        Exception ex(Exception::e_internal_29);
        switch (result)
        {
        case ACS_APGCC_STRING_BUFFER_SMALL :  ex << "String buffer too small.";        break;
        case ACS_APGCC_FAULT_LOGICAL_NAME  :  ex << "Faulty logical name.";            break;
        default                            :  ex << "Unexpected return code.";
        }
        throw ex;
    }
}

//----------------------------------------------------------------------------------------
// Get APZ data path
//----------------------------------------------------------------------------------------
const string
Common::getApzDataPath()
{
    return s_apzpath;
}

//------------------------------------------------------------------------------------------
// Get current port number of login shell
//------------------------------------------------------------------------------------------
bool
Common::getCurrentPortNo(int& port){
    char* portNo = getenv("PORT");
    if ( portNo == 0 ){
        return false;
    }
        port = atoi(portNo); // Will return zero if conversion fails
    
    return (port > 0);
}
//------------------------------------------------------------------------------------------
// Check if a TS shell
//------------------------------------------------------------------------------------------
bool
Common::isTroubleShootingShell(){
    int portNo = 0;
    if (getCurrentPortNo(portNo)){
        if ((portNo == 4422) || (portNo == 4423)) {
        return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------------------
// CReate symbolic links for directory strcuture after HW dump installation
//------------------------------------------------------------------------------------------
void
Common::createSymlinks()
{
    Common::createLink("/data/apz");
    Common::createLink("/data/apz/data");
    Common::createLink("/data/apz/data/boot");
    
    // Create Symbolic link for "Boot" too..
    const string Bootsymlink = "/data/apz/data/Boot";
    const string Bootdir = "boot";
    if (Common::fileExists(Bootsymlink) == false)
    {
        // Create symbolic link
        Common::createSymbolicLink(Bootsymlink, Bootdir);
    }
}

void 
Common::createLink(const string& dir)
{
    try
    {       fs::path path = dir;
            const fs::path& file = path.filename();
            const fs::path& symlink = path.parent_path() / boost::to_upper_copy(file.string());
            if (Common::fileExists(symlink.string()) == false)
            {
                // Create symbolic link
                fs::create_symlink(file, symlink);
            }
    }
   
    catch (fs::filesystem_error e)
    {
        Exception ex(Exception::e_system_28);
        ex << "Failed to create symbolic link for file '" << dir << "'.";
        ex << e.what();
        throw ex;
    }
}

//------------------------------------------------------------------------------------------
// Check if MAUS
//------------------------------------------------------------------------------------------
bool
Common::isMAUS(CPID cpId)
{
   ACS_CS_API_NS::CS_API_Result result;
   ACS_CS_API_NS::MauType mauType;
   
   // Check if the is VIRTUALIZED ENV
   if (isVAPZ())
   {
	   // In Virtual APZ only MAUS type is applicable.
	   return true;
   }
   
   ACS_CS_API_CP* cpInstance = ACS_CS_API::createCPInstance();
   if (cpInstance != NULL)
   {
      result = cpInstance->getMauType(cpId, mauType);
      ACS_CS_API::deleteCPInstance(cpInstance);
      cpInstance = NULL;
   }
   
   if (mauType == ACS_CS_API_NS::MAUS)
   {
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------------------
// Check if vAPZ
//------------------------------------------------------------------------------------------
bool Common::isVAPZ()
{
	ACS_CS_API_NS::CS_API_Result result;
	ACS_CS_API_CommonBasedArchitecture::ArchitectureValue arcValue;

	// Check if the is VIRTUALIZED ENV
	result = ACS_CS_API_NetworkElement::getNodeArchitecture(arcValue);
	return (result == ACS_CS_API_NS::Result_Success) && (arcValue == ACS_CS_API_CommonBasedArchitecture::VIRTUALIZED);
}

//----------------------------------------------------------------------------------------
// Check CP name
//----------------------------------------------------------------------------------------
bool Common::isCpName(const string& name)
{
    boost::regex e("cp(1|2)|bc[0-9]|bc[1-5][0-9]|bc6[0-3]");
    boost::smatch what;

    return boost::regex_match(name, what, e, boost::match_extra);
}

//----------------------------------------------------------------------------------------
// Return bootOpts script path
//----------------------------------------------------------------------------------------
string Common::bootOptsTool()
{
   string path="/opt/ap/pes/bin/bootOpts";
   return path;
}

//----------------------------------------------------------------------------------------
// Execute bootOpts script
//----------------------------------------------------------------------------------------
int Common::_execlp(const char *cmdStr)
{

   int status, rCode=0;
   pid_t pid = fork();
   if (pid == 0)
   {
       try{
           if(execlp("sh","sh", "-c", cmdStr, (char *) NULL) == -1)
           {
               rCode = -1;
           }

       }catch(Exception& ex)
       {
           rCode = -1;
       }
       exit(0);
       cout << "exited from fork" <<endl;
   }
   else
   {
       if (pid < 0)
       {
           rCode = -1;
       }
   }
   if (rCode != -1)
   {
       waitpid(pid, &status, 0);
       // Child terminated normally 
       if (WIFEXITED(status)) 
       {
           rCode = 0;
       } 
       else 
       {
           rCode = -1;
       }

   }
   return rCode;
}


