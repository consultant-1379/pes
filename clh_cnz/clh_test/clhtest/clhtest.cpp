#include <ltime.h>
#include <cpinfo.h>
#include <loginfo.h>
#include <logtask.h>
#include <cmdparser.h>
#include <message.h>
#include <exception.h>
#include <ACS_APGCC_Util.H>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <vector>

using namespace std;
using namespace PES_CLH;

namespace fs = boost::filesystem;

//----------------------------------------------------------------------------------------
// Create CLH header
//----------------------------------------------------------------------------------------
void createClhHeader(ostream& s, size_t size)
{
   // Create header
   s.setf(ios::left, ios::adjustfield);
   s << CLHMessage::s_headerTag << endl;

   // Get current time
   const uint64_t time = Time::now();

   // Insert time in header
   {
      ostringstream t;
      t << time << endl;
      s << setw(18) << setfill(' ') << t.str();
   }

   // Insert event size in header
   {
      ostringstream t;
      t << size << endl;
      s << setw(14) << setfill(' ') << t.str();
   }
}

//----------------------------------------------------------------------------------------
// Create TESRV header
//----------------------------------------------------------------------------------------
void createTesrvHeader(ostream& s, size_t size)
{
   // Create header
   s << TESRVMessage::s_headerTag << endl;

   // Get current time
   const uint64_t time = Time::now();

   // Insert time in header
   s.write(reinterpret_cast<const char*>(&time), sizeof(uint64_t));

   // Insert event size in header
   s.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
   s.put(0x0a);
}

//----------------------------------------------------------------------------------------
// Create data
//----------------------------------------------------------------------------------------
void createData(ostream& s, size_t& size)
{
   for (size_t i = 0; i < size - 1; i++)
   {
      s << ((i % 72)? 'X': '\n');
   }
   s << endl;
}

//----------------------------------------------------------------------------------------
// Sleep for <time> microseconds
//----------------------------------------------------------------------------------------
void microSleep(uint32_t time)
{
   timespec req = {0, time * 1000};
   nanosleep(&req, 0);
}

//----------------------------------------------------------------------------------------
// Create append log event
//----------------------------------------------------------------------------------------
void createAppendLog(
            const BaseParameters& parameters,
            const fs::path& path,
            uint16_t repeat
            )
{
   for (int i = 0; repeat? i < repeat: true; ++i)
   {
      fs::fstream fs(path, ios_base::binary | ios_base::out | ios_base::trunc);
      if (fs.is_open() == false)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to open file '" << path << "'.";
         ex.sysError();
         throw ex;
      }

      size_t size = 1 + rand() % 0x3ff;
      const string& logname = parameters.getLogName();
      size_t hsize = size + logname.size() + 2;
      t_headertype headertype = parameters.getHeaderType();

      string xm;
      if (parameters.hasXmNo())
      {
         uint32_t xmno = 1 + rand() % 16;
         if (xmno > 13)
         {
            xm = "KERNEL: ";
         }
         else if (xmno > 10)
         {
            xm = "PARENTX: ";
         }
         else
         {
            xm = "XM " + boost::lexical_cast<string>(xmno) + ": ";
         }
         hsize += xm.size();
      }

      switch (headertype)
      {
         case e_noHeader:
            break;

         case e_clhHeader:
            createClhHeader(fs, hsize);
            break;

         case e_tesrvHeader:
            createTesrvHeader(fs, hsize);
            break;

         default:
            assert(!"Unknown header type");
      }

      fs << xm << logname << ": ";
      createData(fs, size);

      fs.close();

      cout << "Event generated." << endl;
      // Sleep for 100 millisecond
      microSleep(100000);

   }
}

//----------------------------------------------------------------------------------------
// Create file log event
//----------------------------------------------------------------------------------------
void createFileLog(
         const BaseParameters& parameters,
         const fs::path& path
         )
{
   fs::fstream fs(path, ios_base::binary | ios_base::out | ios_base::trunc);
   if (fs.is_open() == false)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to open file '" << path << "'.";
      ex.sysError();
      throw ex;
   }

   size_t size = 1 + rand() % 0x10000;
   const string& logname = parameters.getLogName();
   size_t hsize = parameters.hasXmNo()? 0: size + logname.size() + 2;
   t_headertype headertype = parameters.getHeaderType();
   switch (headertype)
   {
      case e_noHeader:
         break;

      case e_clhHeader:
         createClhHeader(fs, hsize);
         break;

      default:
         assert(!"Unknown header type");
   }

   fs << logname << ": ";
   createData(fs, size);

   // Sleep for 1 second
   microSleep(1000000);
}

//----------------------------------------------------------------------------------------
// Create directory log event
//----------------------------------------------------------------------------------------
void createDirLog(
         const BaseParameters& parameters,
         const fs::path& path
         )
{
   fs::fstream fs(path, ios_base::binary | ios_base::out | ios_base::trunc);
   if (fs.is_open() == false)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to open file '" << path << "'.";
      ex.sysError();
      throw ex;
   }

   size_t size = 1 + rand() % 0x10000;
   const string& logname = parameters.getLogName();
   fs << logname << ": ";
   createData(fs, size);
   fs.close();
}

//----------------------------------------------------------------------------------------
// Create log event
//----------------------------------------------------------------------------------------
void createEvent(const BaseParameters& parameters, const fs::path& logpath, uint16_t repeat)
{
   srand(time(0));

   switch (parameters.getLogType())
   {
      case e_error:
      case e_event:
      case e_syslog:
      case e_binlog:
      case e_evlogcpsb:
      case e_evlogpcih:
      case e_consolsrm:
      case e_consolbmc:
      case e_consolmp:
      case e_consolpcih:
      case e_consolsyscon:
      case e_xpulog:
      case e_trace:
      {
         fs::path path = logpath / parameters.getPath() / parameters.getFilePrefix();
         path.replace_extension(".tmp");
         createAppendLog(parameters, path, repeat);
      }
      break;

      case e_corecpbb:
      case e_corecpsb:
      case e_corecpub:
      {
         fs::path path = logpath / parameters.getPath() / parameters.getFilePrefix();
         path.replace_extension(".tmp");
         createFileLog(parameters, path);
      }
      break;

      case e_corepcih:
      {
         fs::path path = logpath / parameters.getPath();
         createFileLog(parameters, path / "Apz_vmCore.tmp");
         microSleep(1000);          // Sleep for 1000 microsecond
         createFileLog(parameters, path / "pcihsrv_tick.tmp");
         microSleep(1000);          // Sleep for 1000 microsecond
         createFileLog(parameters, path / "pcihsrv_stat.tmp");
         microSleep(1000);          // Sleep for 1000 microsecond
      }
      break;

      case e_crashcpbb:
      {
         fs::path path = logpath / parameters.getPath();
         createDirLog(parameters, path / "bounds");
         microSleep(10000);          // Sleep for 10 milliseconds
         createDirLog(parameters, path / "msgbuf.savecore");
         microSleep(10000);          // Sleep for 10 milliseconds
         createDirLog(parameters, path /  "evm.buf");
         microSleep(10000);          // Sleep for 10 milliseconds
         createDirLog(parameters, path / "vmzcore");
         microSleep(10000);          // Sleep for 10 milliseconds
      }
      break;

      case e_crashcpsb:
      {
         // Create directory
         const Time& time = Time::now();
         string date;
         string ttime;
         time.get(date, ttime);
         const string& timestamp = date + ttime.substr(0, 4);
         const fs::path& path = logpath / parameters.getPath() / ("cpsba_" + timestamp);
         fs::create_directory(path);

         createDirLog(parameters, path / ("dump." + timestamp));
         microSleep(1000);
         createDirLog(parameters, path / ("map." + timestamp));
         microSleep(1000);
      }
      break;

      case e_crashpcih:
      {
         // Create directory
         const Time& time = Time::now();
         string date;
         string ttime;
         time.get(date, ttime);
         const string& timestamp = date + ttime.substr(0, 4);
         const fs::path& path = logpath / parameters.getPath() / ("pciha_" + timestamp);
         fs::create_directory(path);

         createDirLog(parameters, path / ("dump." + timestamp));
         microSleep(1000);
         createDirLog(parameters, path / ("map." + timestamp));
         microSleep(1000);
         createDirLog(parameters, path / ("reg." + timestamp));
         microSleep(1000);
      }
      break;

      case e_crashcpub:
      {
         const fs::path& path = logpath / parameters.getPath();
         const Time& time = Time::now();
         ostringstream s;
         s << "dump_file_" << time << ".gz";
         createFileLog(parameters, path / s.str());
         microSleep(1000);          // Sleep for 1000 microsecond
      }
      break;

      case e_salinfocpsb:
      {
         // Create directory
         const Time& time = Time::now();
         string date;
         string ttime;
         time.get(date, ttime);
         string timestamp = date + ttime.substr(0, 4);
         const fs::path& path = logpath / parameters.getPath() / ("cpsba_" + timestamp);
         fs::create_directory(path);

         microSleep(4000);
         time.get(date, ttime);
         timestamp = date.substr(0, 4) + "-" + date.substr(4, 2) + "-" + date.substr(6, 2) +
               "-" + ttime.substr(0, 2) + "_" + ttime.substr(2, 2) + "_" + ttime.substr(4, 2);
         createDirLog(parameters, path / (timestamp + "-cpu0-init.0"));
         microSleep(1000);
         createDirLog(parameters, path / (timestamp + "-cpu0-init.0.raw"));
         microSleep(1000);
         createDirLog(parameters, path / (timestamp + "-cpu1-init.0"));
         microSleep(1000);
         createDirLog(parameters, path / (timestamp + "-cpu1-init.0.raw"));
         microSleep(1000);
      }
      break;

      case e_sel:
      case e_ruf:
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << "Handling for this log not yet implemented";
         throw ex;
      }
      break;

      case e_xpucore:
      {
         fs::path path = logpath / parameters.getPath();
         uint32_t xmno = 1 + rand() % 16;
         ostringstream s;
         s << parameters.getFilePrefix() << "_" << setw(4) << setfill('0') << xmno;
         path /= s.str();
         path.replace_extension(".tmp");
         createFileLog(parameters, path);
      }
      break;

      default: assert(!"Illegal log type");
   }
}

//----------------------------------------------------------------------------------------
// Get CP side
//----------------------------------------------------------------------------------------
t_cpSide getCpSide(const string& str)
{
   const string& side = boost::to_upper_copy(str);
   if      (side == "A") return e_cpa;
   else if (side == "B") return e_cpb;
   else throw Exception(Exception::illCpSide(side), WHERE__);
}

//----------------------------------------------------------------------------------------
// Usage
//----------------------------------------------------------------------------------------
void usage(const string& cmdname, bool multicp)
{
   cout << "Usage: ";
   if (multicp)
   {
      cout << "-cp cp_name [-s cp_side]";
   }
   else
   {
      cout << "-s cp_side";
   }
   cout << " -l logtype [-r repeat]" << endl;
   cout << "       " << cmdname << "-h" << endl;
}

//----------------------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   const string& path = argv[0];
   size_t pos = path.find_last_of('/') + 1;
   const string& cmdname = path.substr(pos);

   bool multicp(false);
   try
   {
      // Check that we are running on the active node
      bool ok = ACS_APGCC::is_active_node();
      if (ok == false)           // Stand-by
      {
         throw Exception(Exception::illNodeState(), WHERE__);
      }

      // Check CP system, one-CP or multiple-CP
      multicp = CPTable::isMultiCPSystem();

      // Declare comman options
      CmdParser::Optarg optCpName("cp");
      CmdParser::Optarg optCpSide("s");
      CmdParser::Optarg optLogName("l");
      CmdParser::Optarg optRepeat("r");
      CmdParser::Opt optHelp("h");

      // Parse command
      CmdParser cmdparser(argc, argv);

      cmdparser.fetchOpt(optCpName);
      cmdparser.fetchOpt(optCpSide);
      cmdparser.fetchOpt(optLogName);
      cmdparser.fetchOpt(optRepeat);
      cmdparser.fetchOpt(optHelp);

      // End of command check
      cmdparser.check();

      // Analyze parameters
      if (optHelp.found() && !optCpName.found() && !optCpSide.found() &&
          !optLogName.found() && !optRepeat.found())
      {
         // Help
         usage(cmdname, multicp);
         cout << endl;
         if (multicp)
         {
            cout << setw(20) << "" << "-cp cp_name     CP name" << endl;
            cout << setw(20) << "" << "-s cp_side      CP side" << endl;
         }
         cout << setw(12) << "" << "-l logtype      Log type" << endl;
         cout << setw(12) << "" << "-r repeat       Repeat event, 1 - 4294967295 times," << endl;
         cout << setw(12) << "" << "                0: repeat infinitely." << endl;
      }
      else if (!optHelp.found())
      {
         // Generate event
         string cp;
         CPTable cptable;
         CPInfo cpinfo;
         t_cpSide cpside = e_noside;
         uint32_t repeat(1);

         if (multicp)
         {
            // Multi CP system
            if (optCpName.found() == false)
            {
               throw Exception(Exception::usage(), WHERE__);
            }

            // CP information
            cp = boost::to_upper_copy(optCpName.getArg());
            CPTable::const_iterator iter = cptable.find(cp);
            if (iter == cptable.end())
            {
               throw Exception(Exception::cpNotDefined(cp), WHERE__);
            }

            cpinfo = *iter;
            CPID cpid = cpinfo.getCPID();

            // CP or BC?
            if (cpid < ACS_CS_API_HWC_NS::SysType_CP)
            {
               // BC has no CP sides
               if (optCpSide.found())
               {
                  throw Exception(Exception::cpSideNotAllowed(), WHERE__);
               }
               cpside = e_cpa;
            }
            else
            {
               // Dual CP system - check CP side
               if (optCpSide.found())
               {
                  cpside = getCpSide(optCpSide.getArg());
               }
               else
               {
                  throw Exception(Exception::usage(), WHERE__);
               }
            }
         }
         else
         {
            if (optCpName.found())
            {
               throw Exception(Exception::illOption("-" + optCpName.getOpt()), WHERE__);
            }

            cpinfo = cptable.get();

            // Check CP side
            if (optCpSide.found())
            {
               cpside = getCpSide(optCpSide.getArg());
            }
            else
            {
               throw Exception(Exception::usage(), WHERE__);
            }
         }

         if (optRepeat.found())
         {
            try
            {
               repeat = boost::lexical_cast<uint32_t>(optRepeat.getArg());
            }
            catch (exception&)
            {
               Exception ex(Exception::parameter(), WHERE__);
               ex << "Integer value expected for option '-" << optRepeat.getOpt() << "'.";
               throw ex;
            }
         }

         t_apzSystem apzsys = cpinfo.getAPZSystem();

         // Analyze logname
         const string& logname = boost::to_upper_copy(optLogName.getArg());
         LogTable logtable;
         logtable.initialize();
         LogTable::LIST loglist = logtable.getList();
         LogTable::LISTCITER iter = std::find(
                                       loglist.begin(),
                                       loglist.end(),
                                       logname
                                       );
         if (iter == loglist.end())
         {
            throw Exception(Exception::illLogType(logname), WHERE__);
         }

         const LogInfo& loginfo = *iter;
         const BaseParameters& parameters = loginfo.getParameters();
         if (parameters.isValidApz(apzsys) == false)
         {
            throw Exception(Exception::illLogTypeForApz(logname), WHERE__);
         }

         fs::path cpdir = cpinfo.getName();
         switch (cpside)
         {
            case e_cpa: cpdir /= "cpa"; break;
            case e_cpb: cpdir /= "cpb"; break;
            default:    assert(!"Illegal CP side");
         }
         const fs::path& logpath = BaseParameters::getApzLogsPath() / cpdir;
         createEvent(parameters, logpath, repeat);
      }
      else
      {
         Exception ex(Exception::usage(), WHERE__);
         throw ex;
      }
   }
   catch (Exception& ex)
   {
      cerr << ex << endl;
      uint16_t retCode = ex.getErrCode();
      if (retCode == Exception::usage().first)
      {
         // If incorrect usage, print command format
         cerr << endl;
         usage(cmdname, multicp);
      }
      cerr << endl;
      return retCode;
   }

   return 0;
}



