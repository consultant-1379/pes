//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      ftptask.h
//
//  COPYRIGHT
//      Copyright Ericsson AB 2013. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for ftp task.
//      
//  ERROR HANDLING
//      C++ exceptions are used for error handling.
//
//  DOCUMENT NO
//      190 89-CAA 109 1424  PA1
//
//  AUTHOR
//      XDT/DEK XLOBUNG (Long Nguyen)
//
//  REVISION HISTORY
//      Rev.   Date         Prepared    Description
//      ----   ----         --------    -----------
//      PA1    2013-08-01   XLOBUNG     CLH for AP2.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#ifndef FTP_CLIENT_
#define FTP_CLIENT_

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <string>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include "ftpdtp.h"

namespace PES_CLH {

//========================================================================================
// Class FtpClient
//========================================================================================

class FtpClient
{
   typedef FtpClient self_type;

public:
   // Constructor
   FtpClient(
         boost::asio::io_service& ioservice,               // Asio Service
         std::string const&       server,                  // Server IP
         std::string const&       userid,                  // User name
         std::string const&       password,                // Password
         std::string const&       url,                     // URL
         std::string const&       filename);               // Filename
      
   // Close
   void close();
   
   // Get error number
   int error_num() const;
   
   // Get error message
   std::string error_what() const;
   
   // Check if the file is stored
   bool isStored() const;
   
private:
   // Connection State
   enum state_t
   {
      STATE_UNCONNECTED,
      STATE_HOST_LOOKUP,
      STATE_CONNECTING,
      STATE_CONNECTED,
      STATE_PASSWORD_REQUIRED,
      STATE_PASSWORD_CERTIFYING,
      STATE_LOGGED_IN,
      STATE_CHANGED_DIRECTORY,
      STATE_SET_TYPE_BINARY,
      STATE_SET_PASSIVE_MODE,
      STATE_STORING,
      STATE_STORED,
      STATE_CLOSING,
   };
   
   // Commands
   enum dtp_command
   {
      COMMAND_NULL,
      COMMAND_LIST,
      COMMAND_NLIST,
      COMMAND_STOR,
      COMMAND_PASV,
   };
   
   // Get endpoints
   void handleResolve(
         boost::system::error_code const&         ftperror,
         boost::asio::ip::tcp::resolver::iterator iter);
      
   // Handle connection
   void handleConnect(
         boost::system::error_code const&         ftperror,
         boost::asio::ip::tcp::resolver::iterator iter);
      
   // Handle write request for logging in
   void handleWriteRequestLogin(
         boost::system::error_code const& ftperror);
      
   // Handle read request for logging in
   void handleReadLogin(
         boost::system::error_code const& ftperror);
      
   // Handle write request commands
   void handleWriteRequestCommand(
         boost::system::error_code const& ftperror);
      
   // Handle reading commands
   void handleReadCommand(
         boost::system::error_code const& ftperror);
      
   // Handle writing content
   void handleWriteRequestContent(
         boost::system::error_code const& ftperror);
      
   // Handle read content
   void handleReadContent(
         boost::system::error_code const& ftperror);
      
   // Close socket
   void closeSocket();
   
   // Read response
   std::string readResponse();
   
   // Execute PASV command
   void commandPASV(
         std::string const& ftpresponse);
      
   // Execute STOR command
   void commandSTOR();
   
   state_t                        m_state;              // Connection state
   dtp_command                    m_dtpcommand;         // Commands
   std::string                    m_url;                // URL
   std::string                    m_userid;             // Username
   std::string                    m_password;           // Password
   std::string                    m_filename;           // Filename
   FtpClientDTP::shared_ptr       m_newconnection;      // New connection
   boost::asio::io_service&       m_ioservice;          // Asio service
   boost::asio::ip::tcp::resolver m_resolver;           // Resolver
   boost::asio::ip::tcp::socket   m_socket;             // Socket
   boost::asio::deadline_timer    m_timer;              // Timer
   boost::asio::streambuf         m_request;            // Stream request
   boost::asio::streambuf         m_response;           // Stream response
   boost::system::error_code      m_error;              // Error code
};

}

#endif
