//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      ftpdtp.h
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
//      Class for data transfer.
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

#ifndef FTP_DTP_
#define FTP_DTP_

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

namespace PES_CLH {

//========================================================================================
// Class FtpClientDTP
//========================================================================================

class FtpClientDTP:
   public boost::enable_shared_from_this< FtpClientDTP >
{
   typedef FtpClientDTP self_type;
   
public:
   // Shared pointer
   typedef boost::shared_ptr< self_type > shared_ptr;
   
   // Get shared pointer
   static shared_ptr create(boost::asio::io_service& ioservice)
   {
      return shared_ptr(new self_type(ioservice));
   }
   
   // Set input filename
   void setInputFilename(std::string const& filename);
   
   // Set informatiion
   void setInfo(std::string const& hostname, std::string const& servicename);
   
   // Start transfer
   void start();
   
   // Close transfer
   void close();
   
private:
   // Constructor
   FtpClientDTP(boost::asio::io_service& ioservice);
   
   // Resolve the ip address
   void handleResolve(
         boost::system::error_code const& ftperror,
         boost::asio::ip::tcp::resolver::iterator iter);
      
   // Handle connection
   void handleConnect(
         boost::system::error_code const& ftperror,
         boost::asio::ip::tcp::resolver::iterator iter);
      
   // Write content to file
   void handleWriteContent(boost::system::error_code const& ftperror);
   
   // Write data
   void handleWrite(boost::system::error_code const& ftperror);
   
   // Close socket
   void closeSocket();
   
   boost::asio::io_service&         m_asioservice;      // Asio Service
   boost::asio::ip::tcp::resolver   m_resolver;         // Resolver
   boost::asio::ip::tcp::socket     m_socket;           // Socket
   boost::asio::streambuf           m_response;         // Response
   std::ifstream                    m_stream;           // File Stream
   std::string                      m_filename;         // Filename
   std::string                      m_hostname;         // Host name
   std::string                      m_servicename;      // Service name
};

}   
#endif
