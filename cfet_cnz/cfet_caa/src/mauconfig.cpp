//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      mauconfig.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2014 - 2021. All rights reserved.
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
//      PA1    2014-02-19   XLOBUNG     XPU Config introduced
//      -      2017-05-17   XMAHIMA     Updated for SW MAU(MAUS) Improvement feature
//      
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <exception.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <CPHW_MAUS_API_Libh.h>
#include <unistd.h>

#include "mauconfig.h"
#include "dialogue.h"
#include "cphwconfig.h"


using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

// Heading  for XPU configuration
//const char Mauitemtable::s_HEADING[] = "NO  FILENAME                      PRODUCT             VERSION";
const char Mauitemtable::s_HEADING[]   = "NO    PRODUCT             VERSION";
const char Mauitemtable::s_HEADINGCP[] = "NO    PRODUCT             VERSION   MAUSCORE_PRODUCT    MAUSCORE_VERSION";
//const string& Mauinfo::s_MAUCONFIG =         "/cluster/storage/system/config/cphw/maus/";
const string& Mauinfo::s_MAUCONFIG =         "/cluster/storage/system/config/cphw-maus/";
const string& Mauinfo::s_MAU1CFGFILENAME =   "maus1.conf";
const string& Mauinfo::s_MAU2CFGFILENAME =   "maus2.conf";
const string& Mauinfo::s_MAULIBPATH =        "/opt/ap/cphw/lib64/";
//const string& Mauinfo::s_MAULIBFNPATTERN =   "libMauCore.so.\\d+.\\d+.\\d+";
const string& Mauinfo::s_MAULIBFNPATTERN =   "libCXC\\d+.so.\\d+.\\d+.\\d+\[a-z]*";
//const string& Mauinfo::s_MAULIBFNPATTERN =   "libCXC1060362.so.\\d+.\\d+.\\d+";
const string& Mauinfo::s_INFONA =            "N/A";

const string& Mauinfo::s_APNODEASTR =         "AP node A:";
const string& Mauinfo::s_APNODEBSTR =         "AP node B:";

const string& Mauinfo::s_NEWSECTION =        "[new]";
const string& Mauinfo::s_CURSECTION =        "[current]";
const string& Mauinfo::s_BAKSECTION =        "[backup]";

const string& Mauinfo::s_NEWOPTAPA =         "new.new-APNodeA";
const string& Mauinfo::s_NEWOPTAPASUF =      "new-APNodeA";
const string& Mauinfo::s_NEWOPTAPB =         "new.new-APNodeB";
const string& Mauinfo::s_NEWOPTAPBSUF =      "new-APNodeB";

const string& Mauinfo::s_CUROPTAPA =         "current.current-APNodeA";
const string& Mauinfo::s_CUROPTAPASUF =      "current-APNodeA";
const string& Mauinfo::s_CUROPTAPB =         "current.current-APNodeB";
const string& Mauinfo::s_CUROPTAPBSUF =      "current-APNodeB";

const string& Mauinfo::s_BAKOPTAPA =         "backup.backup-APNodeA";
const string& Mauinfo::s_BAKOPTAPASUF =      "backup-APNodeA";
const string& Mauinfo::s_BAKOPTAPB =         "backup.backup-APNodeB";
const string& Mauinfo::s_BAKOPTAPBSUF =      "backup-APNodeB";

const int Mauinfo::s_MAUS1IDX =              1;
const int Mauinfo::s_MAUS2IDX =              2;

const string& Mauinfo::s_INFOPRODUCT =       "MAUS_CORE_CXC_ID=([^,]*)";
const string& Mauinfo::s_INFOREV =           "MAUS_CORE_CXC_REV=([^,]*)";
const string& Mauinfo::s_STARTMAUINFO =      "# Start APMAU info";
const string& Mauinfo::s_STOPMAUINFO =       "# End APMAU info";


const string& Mauinfo::s_BOOTIMAGE =          "boot/image";
const string& Mauinfo::s_CPHWCONFIG =         "hw_config";
const string& Mauinfo::s_CPMAUDEFFNPATTERN =  "CX[a-zA-Z0-9_]*$";
const string& Mauinfo::s_INFOCPMAUPROREV =    "CXC_NAME=([^,]*)";
const string& Mauinfo::s_INFOMAUSCOREPROREV = "MAUCORE=([^,]*)";

const char s_BAK[]  = "BAK";    // Backwards
const char s_CUR[]  = "CUR";    // Current

const char s_HEADING1[] = "MAUS                    MAUSCORE";  // Heading for CP-MAU and MAUSCORE configuration

//////////////////////////
// MAU ITEM
//////////////////////////
//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Mauitem::Mauitem():
				m_filename(""),
				m_productnumber(""),
				m_revision(""),
				m_mauscoreproductnumber(""),
				m_mauscorerevision("")
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Mauitem::~Mauitem()
{
}

//----------------------------------------------------------------------------------------
// Get filename
//----------------------------------------------------------------------------------------
const string Mauitem::getFilename() const
{
	return m_filename;
}

//----------------------------------------------------------------------------------------
// Set filename
//----------------------------------------------------------------------------------------
void Mauitem::setFilename(const string& filename)
{
	m_filename = filename;
}

//----------------------------------------------------------------------------------------
// Get MAU product number
//----------------------------------------------------------------------------------------
const string& Mauitem::getProductNumber() const
{
	return m_productnumber;
}

//----------------------------------------------------------------------------------------
// Set MAU product number
//----------------------------------------------------------------------------------------
void Mauitem::setProductNumber(const string& productnumber)
{
	m_productnumber = productnumber;
}

//----------------------------------------------------------------------------------------
// Get MAU revision
//----------------------------------------------------------------------------------------
const string& Mauitem::getRevision() const
{
	return m_revision;
}

//----------------------------------------------------------------------------------------
// Set MAU revision
//----------------------------------------------------------------------------------------
void Mauitem::setRevision(const string& revision)
{
	m_revision = revision;
}

//----------------------------------------------------------------------------------------
// Display AP MAU item
//----------------------------------------------------------------------------------------
void Mauitem::displayItem()
{
        (void)cout.setf(ios::left, ios::adjustfield);
        //cout << setw(30) << m_filename << setw(20) << m_productnumber << setw(10) << m_revision << endl;
        cout << setw(20) << m_productnumber << setw(10) << m_revision << endl;
        (void)cout.unsetf(ios::left);
}

//----------------------------------------------------------------------------------------
// Display CP MAU item
//----------------------------------------------------------------------------------------
void Mauitem::displayItemCp()
{
        (void)cout.setf(ios::left, ios::adjustfield);
        //cout << setw(30) << m_filename << setw(20) << m_productnumber << setw(10) << m_revision << endl;
        cout << setw(20) << m_productnumber << setw(10) << m_revision//
             << setw(20) << m_mauscoreproductnumber << setw(10) << m_mauscorerevision << endl;
        (void)cout.unsetf(ios::left);
}

//----------------------------------------------------------------------------------------
// Get CP-MAUSCORE product number
//----------------------------------------------------------------------------------------
const string& Mauitem::getMausCoreProductNumber() const
{
        return m_mauscoreproductnumber;
}

//----------------------------------------------------------------------------------------
// Set CP-MAUSCORE product number
//----------------------------------------------------------------------------------------
void Mauitem::setMausCoreProductNumber(const string& mauscoreproductnumber)
{
        m_mauscoreproductnumber = mauscoreproductnumber;
}

//----------------------------------------------------------------------------------------
// Get CP-MAUSCORE revision
//----------------------------------------------------------------------------------------
const string& Mauitem::getMausCoreRevision() const
{
        return m_mauscorerevision;
}
//----------------------------------------------------------------------------------------
// Set CP-MAUSCORE revision
//----------------------------------------------------------------------------------------
void Mauitem::setMausCoreRevision(const string& mauscorerevision)
{
        m_mauscorerevision = mauscorerevision;
}

//////////////////////////
// END MAU ITEM
//////////////////////////

//////////////////////////
// MAU ITEM TABLE
//////////////////////////

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Mauitemtable::Mauitemtable()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Mauitemtable::~Mauitemtable()
{
}

//----------------------------------------------------------------------------------------
// Push back an item
//----------------------------------------------------------------------------------------
void Mauitemtable::pushItem(Mauitem item)
{
	m_mautable.push_back(item);
}

//----------------------------------------------------------------------------------------
// Display table
//----------------------------------------------------------------------------------------
void Mauitemtable::displayTable()
{
	if (m_mautable.size() == 0)
	{
		throw Exception(Exception::e_no_maus_conf_32, "There is no available MAU configuration.");
	}

	cout << s_HEADING << endl;
	for (vector<Mauitem>::size_type i = 0; i < m_mautable.size(); i++)
	{
		(void)cout.setf(ios::right, ios::adjustfield);
//		cout << setw(5) << i <<" ";
		cout <<  " " << i << setw(4) <<" ";
		m_mautable[i].displayItem();
		(void)cout.unsetf(ios::left);
	}
}

//----------------------------------------------------------------------------------------
// Display CP-MAU table
//----------------------------------------------------------------------------------------
void Mauitemtable::displayTableCp()
{
        if (m_mautable.size() == 0)
        {
                throw Exception(Exception::e_no_maus_conf_32, "There is no available MAU configuration.");
        }

        cout << s_HEADINGCP << endl;
        for (vector<Mauitem>::size_type i = 0; i < m_mautable.size(); i++)
        {
                (void)cout.setf(ios::right, ios::adjustfield);
//              cout << setw(5) << i <<" ";
                cout <<  " " << i << setw(4) <<" ";
                m_mautable[i].displayItemCp();
                (void)cout.unsetf(ios::left);
        }
}

//----------------------------------------------------------------------------------------
// Get out an item
//----------------------------------------------------------------------------------------
Mauitem Mauitemtable::getItemByIndex(int index) const
{
	return m_mautable[index];
}

//----------------------------------------------------------------------------------------
// Get size of the table
//----------------------------------------------------------------------------------------
int Mauitemtable::getTableSize() const
{
	return m_mautable.size();
}

//----------------------------------------------------------------------------------------
// Sort table
//----------------------------------------------------------------------------------------
void Mauitemtable::sortTable()
{
	// Do the sorting
}

//----------------------------------------------------------------------------------------
// Read Mau item table
//----------------------------------------------------------------------------------------
bool Mauitemtable::readMauItemTable()
{
	const boost::regex libfilepattern(Mauinfo::s_MAULIBFNPATTERN);
	const fs::path& libdir = Mauinfo::s_MAULIBPATH;

	// Get files from lib64 dir
	fs::directory_iterator end;
	for (fs::directory_iterator iter(libdir); iter != end; ++iter)
	{
		const fs::path& path = *iter;
		const string& file = path.filename().c_str();
		if (regex_match(file, libfilepattern))
		{
			// Check of *.txt file
			const string& txtfile = file + ".txt";
			const fs::path& pathtxtfile = libdir / txtfile;

			if (fs::exists(pathtxtfile) == true)
			{
				string pn = "";
				string rev = "";
				if (parseLibInfoFile(pathtxtfile.string(), pn, rev))
				{
					Mauitem maui;
					maui.setFilename(file);
					maui.setProductNumber(pn);
					maui.setRevision(rev);
					m_mautable.push_back(maui);
				}
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------------------------
// Read Mau item table
//----------------------------------------------------------------------------------------
bool Mauitemtable::parseLibInfoFile(const std::string& path,
		std::string& product,
		std::string& revision)
{
	ifstream conffile;
	//boost::regex proexpr("MAUS_CORE_CXC_ID=\"([^,]*)\"");
	//boost::regex revexpr("MAUS_CORE_CXC_REV=\"([^,]*)\"");
	 boost::regex proexpr(Mauinfo::s_INFOPRODUCT);
	 boost::regex revexpr(Mauinfo::s_INFOREV);
     boost::smatch matches;

	conffile.open(path.c_str(), ios_base::in);
	if (!conffile.is_open())
	{
		return false;
	}

	// Initial assignment
	product = "";
	revision = "";
	// Read lines
	try
	{
		while (!conffile.eof())
		{
			string line;
			conffile >> line;

			if (boost::regex_match(line, matches, proexpr))
			{
				string pro(matches[0]);
				size_t pos = pro.find_first_of("=");
				product = pro.substr(pos + 1, pro.length() - pos - 1);
			}
			if (boost::regex_match(line, matches, revexpr))
			{
				string rev(matches[0]);
				size_t pos = rev.find_first_of("=");
				revision = rev.substr(pos + 1, rev.length() - pos - 1);
			}
		}
	}
	catch (...)
	{
		conffile.close();
		return false;
	}

	if (product == "" || revision == "")
	{
		conffile.close();
		return false;
	}
	conffile.close();
	return true;
}

//----------------------------------------------------------------------------------------
// Read cp Mau item table
//----------------------------------------------------------------------------------------
bool Mauitemtable::readCpMauItemTable(char cpside,bool ismcp,int cpnameId)
{
	string sidedir="";
        const string& datadir = Common::getApzDataPath();
	if(ismcp)
	{
		string cpname= boost::lexical_cast<string>(cpnameId);
		string cpdir = datadir + "/cp" + cpname;
		switch (cpside)
		{
		  case 0:
		  case 'A': sidedir = cpdir + "/cpa"; break;
		  case 'B': sidedir = cpdir + "/cpb"; break;
		  default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
		}
	}
	else
	{
	    const string& bootimgdir = datadir + "/" + Mauinfo::s_BOOTIMAGE;
		switch (cpside)
		{
		  case 0:
		  case 'A': sidedir = bootimgdir + "/a"; break;
		  case 'B': sidedir = bootimgdir + "/b"; break;
		  default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
		}
	}

	const boost::regex deffilepattern(Mauinfo::s_CPMAUDEFFNPATTERN);
	const fs::path& cxcdir = sidedir+"/";

	// Get files from cxc dir
	fs::directory_iterator end;
	for (fs::directory_iterator iter(cxcdir); iter != end; ++iter)
	{
		const fs::path& path = *iter;
		const string& file = path.filename().c_str();
		if (regex_match(file, deffilepattern))
		{
			// Check of *.txt file
			const string& deffile = file + ".def";
			const fs::path& pathdeffile = cxcdir / deffile;

			if (fs::exists(pathdeffile) == true)
			{
				string pn = "";
				string rev = "";
                                string mauscorepn = "";
                                string mauscorerev = "";
				if (parseCXCdefInfoFile(pathdeffile.string(), pn, rev, mauscorepn, mauscorerev))
				{
					Mauitem maui;
					maui.setFilename(file);
					maui.setProductNumber(pn);
					maui.setRevision(rev);
                                        maui.setMausCoreProductNumber(mauscorepn);
                                        maui.setMausCoreRevision(mauscorerev);
					m_mautable.push_back(maui);
				}
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------------------------
// Parses the file and read product number and revision
//----------------------------------------------------------------------------------------
bool Mauitemtable::parseCXCdefInfoFile(const std::string& path,
		std::string& product,
		std::string& revision,
                std::string& mauscoreproduct,
                std::string& mauscorerevision)
{
  	ifstream conffile;
	boost::regex prorevexpr(Mauinfo::s_INFOCPMAUPROREV);
	boost::regex mauscoreprorevexpr(Mauinfo::s_INFOMAUSCOREPROREV);
        boost::smatch matches;

	conffile.open(path.c_str(), ios_base::in);
	if (!conffile.is_open())
	{
		return false;
	}

	// Initial assignment
	product = "";
	revision = "";
        mauscoreproduct = "";
        mauscorerevision = "";

	// Read lines
	try
	{
		while (!conffile.eof())
		{
			string line;
			conffile >> line;

			if (boost::regex_match(line, matches, prorevexpr))
			{
				string pro(matches[0]);
                                
				size_t pos = pro.find_last_of("_");
				size_t pos1 = pro.find_first_of("=");
				product = pro.substr(pos1 + 1, pos - pos1 -1);
				revision = pro.substr(pos + 1, pro.length() - 1);
			}
                        else if (boost::regex_match(line, matches, mauscoreprorevexpr))
                        {

                                string pro(matches[0]);

                                size_t pos = pro.find_last_of("_");
                                size_t pos1 = pro.find_first_of("=");
                                mauscoreproduct = pro.substr(pos1 + 1, pos - pos1 -1);
                                mauscorerevision = pro.substr(pos + 1, pro.length() - 1);
                        }
		}
	}
	catch (...)
	{
		conffile.close();
		return false;
	}

	if ((product == "" || revision == "") || (mauscoreproduct == "" || mauscorerevision == ""))
	{
		conffile.close();
		return false;
	}
	conffile.close();
	return true;
}


//////////////////////////
// END MAU ITEM TABLE
//////////////////////////


//////////////////////////
// MAU CONFIG
//////////////////////////

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Mauconfig::Mauconfig(int index):
				m_mauindex(index)
{
	for (int i = 0; i < e_all; ++i)
	{
		m_tmpconfig [i] = m_mauconfig[i] = "";
	}
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Mauconfig::~Mauconfig()
{
}

//----------------------------------------------------------------------------------------
// List MAU configurations on cp
//
void Mauconfig::listCPMAUConfiguration(char cpside, bool ismcp)
{
	Mauitemtable mautable;
	mautable.readCpMauItemTable(cpside,ismcp,m_mauindex);
	mautable.displayTableCp();
}

void Mauconfig::printCPMAUVersion(const string& version,
                                  char cpside,
                                  bool ismcp)
{
	const string& datadir = Common::getApzDataPath();
	const string& bootimgdir = datadir + "/" + Mauinfo::s_BOOTIMAGE;

	if(ismcp)
	{
		string cpmaupath = "";
		string cpname= boost::lexical_cast<string>(m_mauindex);
		string cpdir = datadir + "/cp" + cpname;
		Cphwconfig<Common::e_multicp> cphwconf;
		switch (cpside)
		{
	         	case 0:
		        case 'A': cpmaupath = cpdir + "/cpa"; break;
		        case 'B': cpmaupath = cpdir + "/cpb"; break;
		        default: throw Exception(Exception::e_internal_29,
                                                 "Illegal CP side.");
	        }
		if(version==s_CUR)
		{
		   if (Common::fileExists(cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".cur"))
		   {
		   	cphwconf.read(cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".cur");
                        const fs::path& cxcdir = cpmaupath+"/";
                        const string& curcpmaufile = cphwconf.m_cpMauVersionMCP + ".def";
                        const fs::path& curcpmaudeffile = cxcdir / curcpmaufile;

                        if (fs::exists(curcpmaudeffile))
                        {
                           string mauscore_prodrev = "";
                           if(parseCXCdefForMausCoreInfo(curcpmaudeffile.string(),
                                                         mauscore_prodrev))
                           {
                              cout << "Current settings" << endl << endl;
                              cout << "CP node " << cpside <<":" << endl << endl;
                              cout << s_HEADING1 << endl;
                              (void)cout.setf(ios::left, ios::adjustfield);
                              cout << setw(24) << cphwconf.m_cpMauVersionMCP//
                                      << setw(24) << mauscore_prodrev << endl << endl;
                              (void)cout.setf(ios::left, ios::adjustfield);
                           }else{
                                   throw Exception(Exception::e_maus_error_33,
                                                   "Failed to parse CP-MAU def file.");
                           }
                        }else{
                                throw Exception(Exception::e_maus_error_33,
                                                "CP-MAU def file does not exist.");
                        }
		   }else{
				cout<< "CPHW dump does not exist."<< endl;
		   }
		}
		else if(version==s_BAK)
		{
                   const fs::path& bak_file = cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".bak";
		   if ((Common::fileExists(cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".bak")) &&
                         !(fs::is_empty(bak_file)))
		   {
		      cphwconf.read(cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".bak");
                      const fs::path& cxcdir = cpmaupath+"/";
                      const string& curcpmaufile = cphwconf.m_cpMauVersionMCP + ".def";
                      const fs::path& curcpmaudeffile = cxcdir / curcpmaufile;
                      if (fs::exists(curcpmaudeffile))
                      {
                         string mauscore_prodrev = "";
                         if(parseCXCdefForMausCoreInfo(curcpmaudeffile.string(),
                                                       mauscore_prodrev))
                         {
                            cout << "Backup settings" << endl << endl;
                            cout << "CP node " << cpside <<":" << endl << endl;
                            cout << s_HEADING1 << endl;
                            (void)cout.setf(ios::left, ios::adjustfield);
                            cout << setw(24) << cphwconf.m_cpMauVersionMCP//
                                    << setw(24) << mauscore_prodrev << endl << endl;
                            (void)cout.setf(ios::left, ios::adjustfield);
                          }else{
                                  throw Exception(Exception::e_maus_error_33,
                                                  "Failed to parse CP-MAU def file.");
                          }
                      }else{
                               throw Exception(Exception::e_maus_error_33,
                                               "CP-MAU def file does not exist.");
                       }
		   }else{
				cout<< "Backup CPHW dump does not exist."<< endl;
		   }
                }
		else
		{
			Exception ex(Exception::e_illegaldumpversion_9);
			ex << "Illegal dump version '" << version << "'.";
			throw ex;
		}

	}
	else
	{
           string cpmaupath = "";
           const string& datadir = Common::getApzDataPath();
           const string& bootimgdir = datadir + "/" + Mauinfo::s_BOOTIMAGE;
           Cphwconfig<Common::e_onecp> cphwconf;
           int index=checkSide(cpside);
           switch (cpside)
           {
              case 0:
              case 'A': cpmaupath = bootimgdir + "/a"; break;
              case 'B': cpmaupath = bootimgdir + "/b"; break;
              default: throw Exception(Exception::e_internal_29,
                                       "Illegal CP side.");
           }
           if(version==s_CUR)
	   {
	      if (Common::fileExists(bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".cur"))
	      {
	         cphwconf.read(bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".cur");
                 const fs::path& cxcdir = cpmaupath+"/";
                 const string& curcpmaufile = cphwconf.m_cpMauCurVersion[index] + ".def";
                 const fs::path& curcpmaudeffile = cxcdir / curcpmaufile;
                 if (fs::exists(curcpmaudeffile))
                 {
                    string mauscore_prodrev = "";
                    if(parseCXCdefForMausCoreInfo(curcpmaudeffile.string(),
                                                  mauscore_prodrev))
                    {
                       cout << "Current settings" << endl << endl;
                       cout << "CP node " << cpside <<":" << endl << endl;
                       cout << s_HEADING1 << endl;
                       (void)cout.setf(ios::left, ios::adjustfield);
                       cout << setw(24) << cphwconf.m_cpMauCurVersion[index]//
                               << setw(24) << mauscore_prodrev << endl << endl;
                       (void)cout.setf(ios::left, ios::adjustfield);
                    }else{
                            throw Exception(Exception::e_maus_error_33,
                                            "Failed to parse CP-MAU def file.");
                    }
                 }else{
                         throw Exception(Exception::e_maus_error_33,
                                         "CP-MAU def file does not exist.");
                 }
	      }else{
				cout<< "CPHW dump does not exist."<< endl;
              }
           }
	   else if(version==s_BAK)
	   {
              const fs::path& bak_file = bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".bak";
	      if ((Common::fileExists(bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".bak")) && 
                    !(fs::is_empty(bak_file)))
	      {
	         cphwconf.read(bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".bak");
                 const fs::path& cxcdir = cpmaupath+"/";
                 const string& curcpmaufile = cphwconf.m_cpMauCurVersion[index] + ".def";
                 const fs::path& curcpmaudeffile = cxcdir / curcpmaufile;
                 if (fs::exists(curcpmaudeffile))
                 {
                    string mauscore_prodrev = "";
                    if(parseCXCdefForMausCoreInfo(curcpmaudeffile.string(),
                                                  mauscore_prodrev))
                    {
                       cout << "Backup settings" << endl << endl;
                       cout << "CP node " << cpside <<":" << endl << endl;
                       cout << s_HEADING1 << endl;
                       (void)cout.setf(ios::left, ios::adjustfield);
                       cout << setw(24) << cphwconf.m_cpMauCurVersion[index]//
                               << setw(24) << mauscore_prodrev << endl << endl;
                       (void)cout.setf(ios::left, ios::adjustfield);
                    }else{
                            throw Exception(Exception::e_maus_error_33,
                                            "Failed to parse CP-MAU def file.");
                    }
                 }else{
                         throw Exception(Exception::e_maus_error_33,
                                         "CP-MAU def file does not exist.");
                 }
	       }else{
				cout<< "Backup CPHW dump does not exist."<< endl;
	       }
	    }
	    else
	    {
	       Exception ex(Exception::e_illegaldumpversion_9);
	       ex << "Illegal dump version '" << version << "'.";
	       throw ex;
	    }
	}

}

//----------------------------------------------------------------------------------------
// Select CP MAU configuration
//----------------------------------------------------------------------------------------
void Mauconfig::selectCPMAUConfiguration(int cfgnum,char cpside, bool ismcp)
{
	Mauitemtable mautable;
	// Get table with CP MAU configurations
		mautable.readCpMauItemTable(cpside,ismcp,m_mauindex);

		// Check the valid range
		if (cfgnum < 0 || cfgnum > mautable.getTableSize() - 1)
		{
			// throw exception here
			throw Exception(Exception::e_unexp_input_chk_19, "Illegal input.");
		}
		string cpmaupath = "";
			const string& datadir = Common::getApzDataPath();
			const string& bootimgdir = datadir + "/" + Mauinfo::s_BOOTIMAGE;
			if(ismcp){
				 string cpname= boost::lexical_cast<string>(m_mauindex);
				 string cpdir = datadir + "/cp" + cpname;
				    switch (cpside)
				    {
				    case 0:
				    case 'A': cpmaupath = cpdir + "/cpa"; break;
				    case 'B': cpmaupath = cpdir + "/cpb"; break;
				    default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
				    }

			}
			else
			{
			    switch (cpside)
			    {
			    case 0:
			    case 'A': cpmaupath = bootimgdir + "/a"; break;
			    case 'B': cpmaupath = bootimgdir + "/b"; break;
			    default: throw Exception(Exception::e_internal_29, "Illegal CP side.");
			    }

			}
		Mauitem maui = mautable.getItemByIndex(cfgnum);
		string selectedcpmaufilename = maui.getFilename();

		//checking soft link
		string filenamelinked = safeReadlink(cpmaupath+"/CP-MAU");
		if(filenamelinked==selectedcpmaufilename)
		{
			cout<<"CP-MAU is already configured to the selected version"<<endl;
			return;
		}

		//modifying hw_config.* file
		int cur=0,bak=1;
		if(ismcp){
			Cphwconfig<Common::e_multicp> conf[2];
			string file[2];
			file[cur] = cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".cur";
			file[bak] = cpmaupath+ "/" + Mauinfo::s_CPHWCONFIG + ".bak";
			conf[cur].read(file[cur]);
			if(Common::fileExists(file[bak]))
			{
			   conf[bak].read(file[bak]);
			   conf[bak].m_cpMauVersionMCP=conf[cur].m_cpMauVersionMCP;
			   conf[bak].write(file[bak]);
			}
			conf[cur].m_cpMauVersionMCP=selectedcpmaufilename;
			conf[cur].write(file[cur]);
		}
		else{
			int index=checkSide(cpside);
			Cphwconfig<Common::e_onecp> conf[2];
			string file[2];
			file[cur] = bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".cur";
			file[bak] = bootimgdir+ "/" + Mauinfo::s_CPHWCONFIG + ".bak";
			conf[cur].read(file[cur]);
			if(Common::fileExists(file[bak]))
			{
			   conf[bak].read(file[bak]);
			   conf[bak].m_cpMauCurVersion[index]=conf[cur].m_cpMauCurVersion[index];
			   conf[bak].write(file[bak]);
			}
			conf[cur].m_cpMauCurVersion[index]=selectedcpmaufilename;
			conf[cur].write(file[cur]);
		}

		//modifying soft link if not pointed to the selected version
		if(!filenamelinked.empty() &&  filenamelinked!=cpmaupath)
		{
			try
			{
			Common::deleteSymbolicLink(cpmaupath+"/CP-MAU");
			Common::createSymbolicLink(cpmaupath+"/CP-MAU", selectedcpmaufilename);
			}
			catch(Exception& ex)
			{
				cerr << ex << endl;
			}
		}


}

int Mauconfig::checkSide(char side)
{
    switch (toupper(side))
    {
    case 'A': return 0;
    case 'B': return 1;
    default:  throw Exception(Exception::e_internal_29, "Illegal CP side.");
    }
}



string Mauconfig::safeReadlink(const std::string& filename)
{
    size_t bufferSize = 255;
    //Increase buffer size until the buffer is large enough
    while (1) {
        char* buffer = new char[bufferSize];
        size_t rc = readlink (filename.c_str(), buffer, bufferSize);
        if (rc == -1) {
            delete[] buffer;
            if(errno == EINVAL) {
                //We know that bufsize is positive, so
                // the file is not a symlink.
                errno = 0;
                return filename;
            } else if(errno == ENAMETOOLONG) {
                bufferSize += 255;
            } else {
                //errno still contains the error code
                return "";
            }
        } else {
            //Success! rc == number of valid chars in buffer
            errno = 0;
            return string(buffer, rc);
        }
    }
}



//----------------------------------------------------------------------------------------
// List MAU configuration
//----------------------------------------------------------------------------------------
void Mauconfig::listMAUConfiguration()
{
	Mauitemtable mautable;
	mautable.readMauItemTable();
	mautable.displayTable();
}

//----------------------------------------------------------------------------------------
// Select MAU configuration
//----------------------------------------------------------------------------------------
void Mauconfig::selectMAUConfiguration(int cfgnum)
{
	Mauitemtable mautable;
	fs::path pathtocfgfile;
	fstream conffile;

	// Get list of MAU.so files
	mautable.readMauItemTable();
	// Check the valid range
	if (cfgnum < 0 || cfgnum > mautable.getTableSize() - 1)
	{
		// throw exception here
		throw Exception(Exception::e_unexp_input_chk_19, "Illegal input.");
	}

	// Check if the config file exist
	if (m_mauindex == Mauinfo::s_MAUS1IDX)
	{
		pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU1CFGFILENAME;
	}
	else
	{
		pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU2CFGFILENAME;
	}

	if (fs::exists(pathtocfgfile) == false)
	{
		// Not exist, create new
		// Create the path
		const fs::path& dir = Mauinfo::s_MAUCONFIG;

		try
		{
			fs::create_directories(dir);
		}
		catch (std::exception& e)
		{
			// throw exception
			//cout << "Failed in creating dir..." << endl;
			throw Exception(Exception::e_maus_error_33, "Failed to create dir: " + dir.string() + ".");
			//return;
		}

		conffile.open(pathtocfgfile.string().c_str(), ios_base::out);

		if (conffile.is_open() == false)
		{
			// throw exception
			//cout << "Failed in creating file..." << endl;
			throw Exception(Exception::e_maus_error_33, "Failed to open file: " + pathtocfgfile.string() + ".");
			//return;
		}
		conffile.close();
	}
	// Parse the config file
	if (parse())
	{
		Mauitem maui = mautable.getItemByIndex(cfgnum);
		const string& newfilename = maui.getFilename();
		m_mauconfig[e_newa] = newfilename;
		m_mauconfig[e_newb] = newfilename;
		// Write config file
		if (!writeCfgFile(pathtocfgfile.string()))
		{
			// Exception
			throw Exception(Exception::e_maus_error_33, "Failed to write: " + pathtocfgfile.string() + ".");
		}
	}
	else
	{
		throw Exception(Exception::e_maus_error_33, "Failed to parse: " + pathtocfgfile.string()  + ".");
	}
}

//----------------------------------------------------------------------------------------
// Parse a config file
//----------------------------------------------------------------------------------------
bool Mauconfig::parse()
{
	po::options_description desc("MAUS");
	po::variables_map vm;
	fs::path pathtocfgfile;
	ifstream settingsfile;
	if (m_mauindex == Mauinfo::s_MAUS1IDX)
	{
		pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU1CFGFILENAME;
	}
	else
	{
		pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU2CFGFILENAME;
	}
	// Checking the existence of MAUS.conf
	if (fs::exists(pathtocfgfile) == false)
	{
		throw Exception(Exception::e_maus_error_33, "There is no MAUS configuration file.");
	}
	// Open the MAUS.conf
	try
	{
		settingsfile.open(pathtocfgfile.string().c_str(), ios_base::in);
		if (settingsfile.is_open() == false)
		{
			// throw exception
			cout << "Failed to open file..." << endl;
		}
		desc.add_options()
            				(Mauinfo::s_NEWOPTAPA.c_str(),po::value<string>(&m_mauconfig[e_newa]), "")
            				(Mauinfo::s_NEWOPTAPB.c_str(),po::value<string>(&m_mauconfig[e_newb]), "")
            				(Mauinfo::s_CUROPTAPA.c_str(),po::value<string>(&m_mauconfig[e_cura]), "")
            				(Mauinfo::s_CUROPTAPB.c_str(),po::value<string>(&m_mauconfig[e_curb]), "")
            				(Mauinfo::s_BAKOPTAPA.c_str(),po::value<string>(&m_mauconfig[e_baka]), "")
            				(Mauinfo::s_BAKOPTAPB.c_str(),po::value<string>(&m_mauconfig[e_bakb]), "")
            				;
		po::store(po::parse_config_file(settingsfile, desc), vm);
		settingsfile.close();
		po::notify(vm);
	}
	catch (std::exception& e)
	{
		throw Exception(Exception::e_maus_error_33, "Failed to parse: " + pathtocfgfile.string()  + ".");
		//return false;
	}
	return true;
}

//----------------------------------------------------------------------------------------
// Write down config file
//----------------------------------------------------------------------------------------
bool Mauconfig::writeCfgFile(const string& path)
{
	ofstream conffile;

	conffile.open(path.c_str(), ios_base::out | ios_base::trunc);

	if (conffile.is_open() == false)
	{
		return false;
	}

	// Write new conf
	if (m_mauconfig[e_newa] != "" || m_mauconfig[e_newb] != "")
	{
		conffile << Mauinfo::s_NEWSECTION << endl;
	}
	if (m_mauconfig[e_newa] != "")
	{
		conffile << Mauinfo::s_NEWOPTAPASUF << "=" << m_mauconfig[e_newa] << endl;
	}
	if (m_mauconfig[e_newb] != "")
	{
		conffile << Mauinfo::s_NEWOPTAPBSUF << "=" << m_mauconfig[e_newb] << endl;
	}
	conffile << endl;
	// Write cur conf
	if (m_mauconfig[e_cura] != "" || m_mauconfig[e_curb] != "")
	{
		conffile << Mauinfo::s_CURSECTION << endl;
	}
	if (m_mauconfig[e_cura] != "")
	{
		conffile << Mauinfo::s_CUROPTAPASUF << "=" << m_mauconfig[e_cura] << endl;
	}
	if (m_mauconfig[e_curb] != "")
	{
		conffile << Mauinfo::s_CUROPTAPBSUF << "=" << m_mauconfig[e_curb] << endl;
	}
	conffile << endl;
	// Write bak conf
	if (m_mauconfig[e_baka] != "" || m_mauconfig[e_bakb] != "")
	{
		conffile << Mauinfo::s_BAKSECTION << endl;
	}
	if (m_mauconfig[e_baka] != "")
	{
		conffile << Mauinfo::s_BAKOPTAPASUF << "=" << m_mauconfig[e_baka] << endl;
	}
	if (m_mauconfig[e_bakb] != "")
	{
		conffile << Mauinfo::s_BAKOPTAPBSUF << "=" << m_mauconfig[e_bakb] << endl;
	}
	conffile << endl;
	conffile.close();
	return true;
}

//----------------------------------------------------------------------------------------
// Display new MAU setting
//----------------------------------------------------------------------------------------
void Mauconfig::displayNewSetting()
{
	// Parse the config file
	if (parse())
	{
		cout << "New settings" << endl << endl;
		cout << Mauinfo::s_APNODEASTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_newa] << endl << endl;
		printFileInfo(m_mauconfig[e_newa]);
		cout << Mauinfo::s_APNODEBSTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_newb] << endl << endl;
		printFileInfo(m_mauconfig[e_newb]);
	}
	else
	{
		throw Exception(Exception::e_maus_error_33, "Failed to parse config file.");
	}
}

//----------------------------------------------------------------------------------------
// Display current MAU setting
//----------------------------------------------------------------------------------------
void Mauconfig::displayCurSetting()
{
	// Parse the config file
	if (parse())
	{
		cout << "Current settings" << endl << endl;
		cout << Mauinfo::s_APNODEASTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_cura] << endl << endl;
		printFileInfo(m_mauconfig[e_cura]);
		cout << Mauinfo::s_APNODEBSTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_curb] << endl << endl;
		printFileInfo(m_mauconfig[e_curb]);
	}
	else
	{
		throw Exception(Exception::e_maus_error_33, "Failed to parse config file.");
	}
}

//----------------------------------------------------------------------------------------
// Display backup MAU setting
//----------------------------------------------------------------------------------------
void Mauconfig::displayBakSetting()
{
	// Parse the config file
	if (parse())
	{
		cout << "Backup settings" << endl << endl;
		cout << Mauinfo::s_APNODEASTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_baka] << endl << endl;
		printFileInfo(m_mauconfig[e_baka]);
		cout << Mauinfo::s_APNODEBSTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_bakb] << endl << endl;
		printFileInfo(m_mauconfig[e_baka]);
	}
	else
	{
		throw Exception(Exception::e_maus_error_33, "Failed to parse config file.");
	}
}

//----------------------------------------------------------------------------------------
// Display all MAU setting
//----------------------------------------------------------------------------------------
void Mauconfig::displayAllSetting()
{
	// Parse the config file
	if (parse())
	{
		// New
		cout << "New settings" << endl << endl;
		cout << Mauinfo::s_APNODEASTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_newa] << endl << endl;
		printFileInfo(m_mauconfig[e_newa]);
		cout << Mauinfo::s_APNODEBSTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_newb] << endl << endl;
		printFileInfo(m_mauconfig[e_newb]);
		// Current
		cout << "Current settings" << endl << endl;
		cout << Mauinfo::s_APNODEASTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_cura] << endl << endl;
		printFileInfo(m_mauconfig[e_cura]);
		cout << Mauinfo::s_APNODEBSTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_curb] << endl << endl;
		printFileInfo(m_mauconfig[e_curb]);
		// Backup
		cout << "Backup settings" << endl << endl;
		cout << Mauinfo::s_APNODEASTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_baka] << endl << endl;
		printFileInfo(m_mauconfig[e_baka]);
		cout << Mauinfo::s_APNODEBSTR << endl << endl;
		cout << "MAUS" << endl;
		//cout << m_mauconfig[e_bakb] << endl << endl;
		printFileInfo(m_mauconfig[e_bakb]);
	}
	else
	{
		throw Exception(Exception::e_maus_error_33, "Failed to parse config file.");
	}
}

//----------------------------------------------------------------------------------------
// Install the new configuration
// Input: 0: APA
//        1: APB
//        2: BOTH
//----------------------------------------------------------------------------------------
void Mauconfig::installNewConfiguration(int side)
{
	fs::path pathtocfgfile;
	bool ok = false;

	// Parse the config file
	if (parse())
	{
#if 0
		// Copy cur to bak
		switch (side)
		{
		case 0:
			m_mauconfig[e_baka] = m_mauconfig[e_cura];
			break;
		case 1:
			m_mauconfig[e_bakb] = m_mauconfig[e_curb];
			break;
		default:
			m_mauconfig[e_baka] = m_mauconfig[e_cura];
			m_mauconfig[e_bakb] = m_mauconfig[e_curb];
			break;
		}
#endif

		// Copy cur to tmp
		for (int i = 0; i < e_all; ++i)
		{
			m_tmpconfig [i] = m_mauconfig[i];
		}

		// Copy new to cur
		switch (side)
		{
		case 0:
			if (m_mauconfig[e_newa] == "")
			{
				throw Exception(Exception::e_maus_error_33, "New setting for APA is empty.");
			}
			else
			{
				if (m_mauconfig[e_cura] == m_mauconfig[e_newa])
				{
					ok = Dialogue::affirm(
							"Warning: Current and New configurations are equal,\n"
							"do you want to continue anyway? "
					);
					cout << endl;
					if (!ok) return;
				}

				m_mauconfig[e_cura] = m_mauconfig[e_newa];	// Copy new to cur
				m_mauconfig[e_baka] = m_tmpconfig[e_cura];	// Copy cur to bak
			}
			break;
		case 1:
			if (m_mauconfig[e_newb] == "")
			{
				throw Exception(Exception::e_maus_error_33, "New setting for APB is empty.");
			}
			else
			{
				if (m_mauconfig[e_curb] == m_mauconfig[e_newb])
				{
					bool ok = Dialogue::affirm(
							"Warning: Current and New configurations are equal,\n"
							"do you want to continue anyway? "
					);
					cout << endl;
					if (!ok) return;
				}
				m_mauconfig[e_curb] = m_mauconfig[e_newb];
				m_mauconfig[e_bakb] = m_tmpconfig[e_curb];
			}
			break;
		default:
			if (m_mauconfig[e_newa] == "" || m_mauconfig[e_newb] == "")
			{
				throw Exception(Exception::e_maus_error_33, "New setting for APA or APB is empty.");
			}
			else
			{
				m_mauconfig[e_cura] = m_mauconfig[e_newa];
				m_mauconfig[e_curb] = m_mauconfig[e_newb];

				m_mauconfig[e_baka] = m_tmpconfig[e_cura];
				m_mauconfig[e_bakb] = m_tmpconfig[e_curb];

			}
			break;
		}

		if (!checksum(side, 0))
		{
			string msg = "Failed to calculate checksum on new config, ";
			msg += side == 0 ? "Node A." : "Node B.";
			throw Exception(Exception::e_maus_error_33, msg);
		}

		// Perform unzip when new is different to cur config
		//if (!ok)
		//{
			if (!installMibs(side, 0))
			{
				string msg = "Failed to install mibs on new config, side ";
				msg += side == 0 ? "apa" : "apb";
				throw Exception(Exception::e_maus_error_33, msg);
			}
		//}

		// Write to config file
		if (m_mauindex == Mauinfo::s_MAUS1IDX)
		{
			pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU1CFGFILENAME;
		}
		else
		{
			pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU2CFGFILENAME;
		}
		if (writeCfgFile(pathtocfgfile.string()))
		{
			if (reloadMAUS(side) == false)
			{
				//cout << " Failed to reload AP MAUS..." << endl;
			}
		}
		else
		{
			throw Exception(Exception::e_maus_error_33, "Failed to write config file.");
		}
	}
}

//----------------------------------------------------------------------------------------
// Install the bak configuration
//----------------------------------------------------------------------------------------
void Mauconfig::installBakConfiguration(int side)
{
	fs::path pathtocfgfile;
	bool ok = false;

	// Parse the config file
	if (parse())
	{
#if 0
		// Copy bak to cur
		switch (side)
		{
		case 0:
			if (m_mauconfig[e_baka] == "")
			{
				throw Exception(Exception::e_maus_error_33, "Backup setting for APA is empty.");
			}
			else
			{
				tmp1 = m_mauconfig[e_cura];
				m_mauconfig[e_cura] = m_mauconfig[e_baka];
			}
			break;
		case 1:
			if (m_mauconfig[e_bakb] == "")
			{
				throw Exception(Exception::e_maus_error_33, "Backup setting for APB is empty.");
			}
			else
			{
				tmp2 = m_mauconfig[e_curb];
				m_mauconfig[e_curb] = m_mauconfig[e_bakb];
			}
			break;
		default:
			if (m_mauconfig[e_baka] == "" || m_mauconfig[e_bakb] == "")
			{
				throw Exception(Exception::e_maus_error_33, "Backup setting for APA or APB is empty.");
			}
			else
			{
				tmp1 = m_mauconfig[e_cura];
				m_mauconfig[e_cura] = m_mauconfig[e_baka];
				tmp2 = m_mauconfig[e_curb];
				m_mauconfig[e_curb] = m_mauconfig[e_bakb];
			}
			break;
		}
#endif

		// Copy cur to tmp
		for (int i = 0; i < e_all; ++i)
		{
			m_tmpconfig [i] = m_mauconfig[i];
		}

		// Copy cur to bak
		switch (side)
		{
		case 0:

			if (m_mauconfig[e_baka] == m_mauconfig[e_cura])
			{
				bool ok = Dialogue::affirm(
						"Warning: Backup and Current configurations are equal,\n"
						"do you want to continue anyway? "
				);
				cout << endl;
				if (!ok) return;

			}

			m_mauconfig[e_baka] = m_mauconfig[e_cura];
			m_mauconfig[e_cura] = m_tmpconfig[e_baka];
			break;
		case 1:
			// tmp2 = m_mauconfig[e_curb];
			if (m_mauconfig[e_bakb] == m_mauconfig[e_curb])
			{
				ok = Dialogue::affirm(
						"Warning: Backup and Current configurations are equal,\n"
						"do you want to continue anyway? "
				);
				cout << endl;
				if (!ok) return;

			}

			m_mauconfig[e_bakb] = m_mauconfig[e_curb];
			m_mauconfig[e_curb] = m_tmpconfig[e_bakb];
			break;
		default:
			m_mauconfig[e_baka] = m_mauconfig[e_cura];
			m_mauconfig[e_bakb] = m_mauconfig[e_curb];

			m_mauconfig[e_cura] = m_tmpconfig[e_baka];
			m_mauconfig[e_curb] = m_tmpconfig[e_bakb];

			break;
		}
		// Write to config file
		if (m_mauindex == Mauinfo::s_MAUS1IDX)
		{
			pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU1CFGFILENAME;
		}
		else
		{
			pathtocfgfile = Mauinfo::s_MAUCONFIG + Mauinfo::s_MAU2CFGFILENAME;
		}

		if (!checksum(side, 2))
		{
			string msg = "Failed to calculate checksum on backup config, side ";
			msg += side == 0 ? "apa" : "apb";
			throw Exception(Exception::e_maus_error_33, msg);
		}

		// Perform unzip when backup is different to cur config
		//if (!ok)
		//{
			if (!installMibs(side, 2))
			{
				string msg = "Failed to install mibs on backup config, side ";
				msg += side == 0 ? "apa" : "apb";
				throw Exception(Exception::e_maus_error_33, msg);
			}
		//}
		if (writeCfgFile(pathtocfgfile.string()))
		{
			if (reloadMAUS(side) == false)
			{
				//cout << " Failed to reload AP MAUS..." << endl;
			}
		}
		else
		{
			throw Exception(Exception::e_maus_error_33, "Failed to write config file.");
		}
	}
}

//----------------------------------------------------------------------------------------
// Reload MAUS lib
//----------------------------------------------------------------------------------------
bool Mauconfig::reloadMAUS(int side)
{
	CPHW_MAUS_API_Libh::Result result = CPHW_MAUS_API_Libh::RC_SUCCESS;
	CPHW_MAUS_API_Libh mauslib(m_mauindex);

	// Unload the current running lib
	result = mauslib.unloadLib(side);
	if (result == CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED ||
			result == CPHW_MAUS_API_Libh::RC_SUCCESS)
	{
		int32_t msec = 400;
		//cout << "sleep for " << msec << " ms before loading" << endl;
		struct timespec req;
		req.tv_sec = 0;
		req.tv_nsec = msec * 1000L * 1000L;    // Example 100 ms = 100 000 000 nano seconds
		nanosleep(&req, NULL);
		//cout << "wake up and start loading" << endl;
		result = mauslib.loadLib(side);
		//cout << "loadLib: " << result << endl;
		if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
		{
			throw Exception(Exception::e_maus_error_33, convertFCtoString(result));
		}
	}
	else
	{
		throw Exception(Exception::e_maus_error_33, convertFCtoString(result));
	}

	return true;
}

bool Mauconfig::checksum(int side, int config)
{
	CPHW_MAUS_API_Libh mauslib(m_mauindex);

	CPHW_MAUS_API_Libh_R1::Result result = mauslib.performChecksum(side, config);
	if (result != CPHW_MAUS_API_Libh::RC_SUCCESS)
	{
		throw Exception(Exception::e_maus_error_33, convertFCtoString(result));
	}

	return true;
}

bool Mauconfig::installMibs(int side, int config)
{
	CPHW_MAUS_API_Libh mauslib(m_mauindex);
	return mauslib.installMibs(side, config) == CPHW_MAUS_API_Libh::RC_SUCCESS;;
}

//----------------------------------------------------------------------------------------
// RC to string
//----------------------------------------------------------------------------------------
const std::string Mauconfig::convertFCtoString(int fc) const
{
	switch (fc)
	{
	case CPHW_MAUS_API_Libh::RC_SUCCESS:
		return "Success.";
	case CPHW_MAUS_API_Libh::RC_NOCONTACT:
		return "No contact.";
	case CPHW_MAUS_API_Libh::RC_FILE_NOT_FOUND:
		return "File not found.";
	case CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB:
		return "Failed to load lib.";
	case CPHW_MAUS_API_Libh::RC_MASK_VALUE_OUT_OF_RANGE:
		return "Mask value out of range.";
	case CPHW_MAUS_API_Libh::RC_OPERATION_NOT_IMPLEMENTED:
		return "Operation not implemented.";
	case CPHW_MAUS_API_Libh::RC_FAILED_TO_GET_OPER_STATE:
		return "Failed to get operation state.";
	case CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB:
		return "Failed to unload lib.";
	case CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED:
		return "Lib already unloaded.";
	case CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED:
		return "Lib already loaded.";
	case CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED_NODEA:
		return "Lib already loaded node A.";
	case CPHW_MAUS_API_Libh::RC_LIB_ALREADY_LOADED_NODEB:
		return "Lib already loaded node B.";
	case CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED_NODEA:
		return "Lib already unloaded node A.";
	case CPHW_MAUS_API_Libh::RC_LIB_ALREADY_UNLOADED_NODEB:
		return "Lib already unloaded node B.";
	case CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB_NODEA:
		return "Failed to unload lib node A.";
		//case CPHW_MAUS_API_Libh::RC_FAILED_TO_UNLOAD_LIB_NODEB:
		//   return "Failed to unload lib node B.";
	case CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB_NODEA:
		return "Failed to load lib node A.";
	case CPHW_MAUS_API_Libh::RC_FAILED_TO_LOAD_LIB_NODEB:
		return "Failed to load lib node B.";

	case CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEA:
		return "Checksum calculation failed on node A.";
	case CPHW_MAUS_API_Libh::RC_CHECK_SUM_FAILED_NODEB:
		return "Checksum calculation failed on node B.";

	case CPHW_MAUS_API_Libh::RC_INSTALL_MIBS_FAILED_NODEA:
		return "Mibs file installation failed on node A.";
	case CPHW_MAUS_API_Libh::RC_INSTALL_MIBS_FAILED_NODEB:
		return "Mibs file installation failed on node B.";

	case CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEA:
		return "Server not contactable for node A.";
	case CPHW_MAUS_API_Libh::RC_NOCONTACT_NODEB:
		return "Server not contactable for node B.";

	default:
		return "Unknown issue.";
	}
}

//----------------------------------------------------------------------------------------
// Print file info
//----------------------------------------------------------------------------------------
void Mauconfig::printFileInfo(const std::string& fn) const
{
	const string& path = Mauinfo::s_MAULIBPATH + fn + ".txt";
	string pro = "";
	string rev = "";

	if (Mauitemtable::parseLibInfoFile(path, pro, rev))
	{
		cout << pro << "_" << rev << endl << endl;
	}
	else
	{
		cout << fn << endl << endl;
	}
}

//----------------------------------------------------------------------------------------
// Parse the CXC def file for CP-MAUSCORE details
//----------------------------------------------------------------------------------------
bool Mauconfig::parseCXCdefForMausCoreInfo(const std::string& path,
                                           std::string& mauscore_prodrev)
{
        ifstream conffile;
        boost::regex mauscoreprorevexpr(Mauinfo::s_INFOMAUSCOREPROREV);
        boost::smatch matches;

        conffile.open(path.c_str(), ios_base::in);
        if (!conffile.is_open())
        {
                return false;
        }

        // Initial assignment
        mauscore_prodrev = "";
        // Read lines
        try
        {
                while (!conffile.eof())
                {
                        string line;
                        conffile >> line;

                        if (boost::regex_match(line, matches, mauscoreprorevexpr))
                        {
                                string pro(matches[0]);
                                size_t pos1 = pro.find_first_of("=");
                                mauscore_prodrev = pro.substr(pos1 + 1);
                        }
                }
        }
        catch (...)
        {
                conffile.close();
                return false;
        }

        if (mauscore_prodrev == "")
        {
                conffile.close();
                return false;
        }
        conffile.close();
        return true;

}

