//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      server.cpp
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
//      This is the CLH server.
//      
//
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1416  PA1
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

#include "server.h"
#include "amfcallbackcontrol.h"
#include "engine.h"
#include <common.h>
#include <logger.h>
#include <eventhandler.h>
#include <cmdparser.h>
#include <ACS_APGCC_Util.H>
#include <iostream>
#include <signal.h>

using namespace std;
using namespace PES_CLH;

static Engine* s_enginep = 0;

//----------------------------------------------------------------------------------------
// Signal handler
//----------------------------------------------------------------------------------------
void sighandler(int signo)
{
   Logger logger(LOG_LEVEL_INFO);
   Engine::t_runstate runstate;
   string info = strsignal(signo);
   info += " received.";
   switch (signo)
   {
   case SIGINT:   runstate = Engine::e_shutdown;   break;
   case SIGTERM:  runstate = Engine::e_shutdown;   break;
   case SIGUSR1:  runstate = Engine::e_restart;    break;
   case SIGUSR2:  runstate = Engine::e_fault;      break;
   default:       assert(!"Illegal signal");
   }
   logger.event(WHERE__, info);
   if (s_enginep != 0)
   {
      s_enginep->reset(runstate);
   }
}

// --------------------------------------------------------------------------
// Signal handler for AMF service
// -------------------------------------------------------------------------
void sighandlerAmf(int signo)
{
   Logger logger(LOG_LEVEL_INFO);
   string info = strsignal(signo);
   switch (signo)
   {
   // Ignore SIGPIPE which causes process stop immediately is raised during cmw-node-lock
   case SIGPIPE:
      info += " signal received and ignored.";
      EventHandler::send(Exception::parameter().first, info);
      logger.event(WHERE__, info);
      break;
 
   // Exit for unknown signal 
   default:
      info += " signal received and exits.";
      EventHandler::send(Exception::parameter().first, info);
      logger.event(WHERE__, info);
      exit(0);
   }
}

//----------------------------------------------------------------------------------------
// AMF service registration
//----------------------------------------------------------------------------------------
void regAMFService(Engine* engine, 
                   const string& service, 
                   ACS_TRA_LogLevel minlevel, 
                   bool isconsole)
{
   AMFcallbackControl haObj(engine, service);
   
   signal(SIGPIPE, sighandlerAmf);

   // Init the Engine
   haObj.initEngine();
   
   // Initiate internal logging
   Logger::open(service, minlevel, isconsole);
   Logger start(LOG_LEVEL_INFO);
   {
      // Start event is always logged
      ostringstream s;
      s << "Internal log for " << Common::getVersion("server") << endl;
      if (minlevel != LOG_LEVEL_OFF)
      {
         s << "Log level is set to '" << minlevel << "'.";
      }
      else
      {
         s << "Internal logging is disabled.";
      }
      start.event(WHERE__, s.str());
   }
   
   Logger integrity(LOG_LEVEL_DEBUG);
   if (integrity)
   {
      integrity.event(WHERE__, "Integrity check enabled.");
   }

   Logger processid(LOG_LEVEL_INFO);
   if (processid)
   {
      ostringstream s;
      s << "Starting service '" << service << "' with process identity "
        << getpid() << ".";
      processid.event(WHERE__, s.str());
   }

   ACS_APGCC_HA_ReturnType errorCode = haObj.activate();

   Logger stop(LOG_LEVEL_INFO);
   if (stop)
   {
      ostringstream s;
      s << "Service '" << service;
      switch (errorCode)
      {
      case ACS_APGCC_HA_SUCCESS:
         s << "': HA application gracefully closed.";
         break;

      case ACS_APGCC_HA_FAILURE_CLOSE:
         s << "': HA application failed to gracefully close.";
         break;

      case ACS_APGCC_HA_FAILURE:
         s << "': HA activation failed.";
         break;

      default:
         s << "': Unknown return code.";
      }

      stop.event(WHERE__, s.str());
   }

   Logger::event(LOG_LEVEL_INFO, WHERE__, "Stopped CLH server.");
   Logger::close();
}

//----------------------------------------------------------------------------------------
// Command usage
//----------------------------------------------------------------------------------------
void usage(const string& cmdname, bool verbose)
{
   cout << endl;
   cout << "Usage: " << cmdname << " [-f |-b][-l level][-c]" << endl;
   if (verbose == false)
   {
      cout << "Type '" << cmdname << " -h' for command help" << endl;
   }
   else
   {
      cout << "       " << cmdname << " -h" << endl;
      cout << endl;
      cout << "       -f         Execute in foreground" << endl;
      cout << "       -b         Execute in background" << endl;
      cout << "       -l level   Minimum log level ('warn' is the default level):" << endl;
      cout << "                     nolog (don't log anything)" << endl;
      cout << "                     fatal" << endl;
      cout << "                     error" << endl;
      cout << "                     warn" << endl;
      cout << "                     info" << endl;
      cout << "                     debug" << endl;
      cout << "                     trace" << endl;
      cout << "                     all (log everything, same as trace)" << endl;
      cout << "       -c         Print logs to console" << endl;
      cout << "       -h         Command help" << endl;
      cout << "       -v         Software version" << endl;
      cout << endl;
      cout << "       The program is by default registered as an AMF service if" << endl
           << "       none of the -f or -b options are supplied." << endl;
   }
   cout << endl;
}

//----------------------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   const string& path = argv[0];
   size_t pos = path.find_last_of('/') + 1;
   const string& cmdname = path.substr(pos);

   // Declare command options
   CmdParser::Opt foreground("f");
   CmdParser::Opt background("b");
   CmdParser::Optarg loglevel("l");
   CmdParser::Opt console("c");
   CmdParser::Opt help("h");
   CmdParser::Opt version("v");

   try
   {
      // Parse command
      CmdParser cmdparser(argc, argv);

      cmdparser.fetchOpt(foreground);
      cmdparser.fetchOpt(background);
      cmdparser.fetchOpt(loglevel);
      cmdparser.fetchOpt(console);
      cmdparser.fetchOpt(help);
      cmdparser.fetchOpt(version);

      // End of command check
      cmdparser.check();

      if (help.found())
      {
         if (foreground.found() || background.found() ||
             loglevel.found() || console.found() || version.found())
         {
             Exception ex(Exception::usage(), WHERE__);
             throw ex;
         }

         usage(cmdname, true);
         return 0;
      }

      if (version.found())
      {
         if (foreground.found() || background.found() ||
             loglevel.found() || console.found() || help.found())
         {
             Exception ex(Exception::usage(), WHERE__);
             throw ex;
         }

         cout << endl << Common::getVersion("server") << endl << endl;
         return 0;
      }

      if (foreground.found() && background.found())
        {
         throw Exception(Exception::usage(), WHERE__);
      }
   }
   catch (Exception& ex)
   {
      // Incorrect usage, print command format
      cerr << endl << ex;
      usage(cmdname);
      return ex.getErrCode();
   }

   try
   {
      // Log level
      ACS_TRA_LogLevel minlevel = LOG_LEVEL_WARN;
      if (loglevel.found())
      {
         string lstr = loglevel.getArg();
         if      (lstr == "nolog") minlevel = LOG_LEVEL_OFF;
         else if (lstr == "fatal") minlevel = LOG_LEVEL_FATAL;
         else if (lstr == "error") minlevel = LOG_LEVEL_ERROR;
         else if (lstr == "warn")  minlevel = LOG_LEVEL_WARN;
         else if (lstr == "info")  minlevel = LOG_LEVEL_INFO;
         else if (lstr == "debug") minlevel = LOG_LEVEL_DEBUG;
         else if (lstr == "trace") minlevel = LOG_LEVEL_TRACE;
         else if (lstr == "all")   minlevel = LOG_LEVEL_TRACE;
         else
         {
            Exception ex(Exception::parameter(), WHERE__);
            ex << "Log level '" << lstr << "' is unknown.";
            throw ex;
         }
      }

      if (foreground.found() || background.found())
      {
         // Check that we are running on the active node
         bool ok = ACS_APGCC::is_active_node();
         if (ok == false)             // Stand-by
         {
            throw Exception(Exception::illNodeState(), WHERE__);
         }

         // Setup signal handling
         signal(SIGINT, sighandler);
         signal(SIGTERM, sighandler);
         signal(SIGUSR1, sighandler);
         signal(SIGUSR2, sighandler);
      }

      // Initiate ACS event handling
      EventHandler::init(cmdname);

      // Execution context
      if (background.found())
      {
         // Execute in background
         int ret = daemon(1, console.found());
         if (ret != 0)
         {
            Exception ex(Exception::system(), WHERE__);
            ex << "Failed to daemonize process.";
            ex.sysError();
            throw ex;
         }
      }
      else if (foreground.found())
      {
      }

      // Check that no other instance of this executable is running
      bool ok = Common::createLock(cmdname, false, "");
      if (ok == false)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Process '" << cmdname << "' is already executing.";
         throw ex;
      }

      if (foreground.found() || background.found())
      {
         // Execute
         s_enginep = new Engine;
         if (s_enginep != 0)
         {
            s_enginep->execute();
         }
      }
      else
      {
         // Register as an AMF service
         regAMFService(s_enginep, cmdname, minlevel, console.found());
      }

      delete s_enginep;
      s_enginep = 0;
   }
   catch (Exception& ex)
   {
      delete s_enginep;
      s_enginep = 0;

      // Log the error
      Logger::event(LOG_LEVEL_FATAL, ex);

      Logger::event(LOG_LEVEL_INFO, WHERE__, "Stopped CLH server.");
      Logger::close();

      cerr << ex << endl << endl;

      return ex.getErrCode();
   }
   return 0;
}
