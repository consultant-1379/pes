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

#include "ftpdtp.h"
#include "logger.h"

using namespace std;
using namespace boost;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Set input filename
//----------------------------------------------------------------------------------------
void FtpClientDTP::setInputFilename(
   string const& filename)
{
   m_filename = filename;
}

//----------------------------------------------------------------------------------------
// Set information
//----------------------------------------------------------------------------------------
void FtpClientDTP::setInfo(
   string const& hostname,
   string const& servicename)
{
   m_hostname = hostname;
   m_servicename = servicename;
}

//----------------------------------------------------------------------------------------
// Start transfer
//----------------------------------------------------------------------------------------
void FtpClientDTP::start()
{
   // Start an asynchronous resolve to translate the server and service
   // names into a list of endpoints
   boost::asio::ip::tcp::resolver::query a_query(m_hostname, m_servicename);
   m_resolver.async_resolve(
      a_query,
      boost::bind(
         &self_type::handleResolve,
         shared_from_this(),
         boost::asio::placeholders::error,
         boost::asio::placeholders::iterator));
}

//----------------------------------------------------------------------------------------
// Close transfer
//----------------------------------------------------------------------------------------
void FtpClientDTP::close()
{
   m_asioservice.post(
      boost::bind(&self_type::closeSocket, shared_from_this()));
}

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
FtpClientDTP::FtpClientDTP(
   boost::asio::io_service& ioservice)
   :
m_asioservice(ioservice),
m_resolver(ioservice),
m_socket(ioservice),
m_hostname(""),
m_servicename("")
{
}

//----------------------------------------------------------------------------------------
// Resolve the ip address
//----------------------------------------------------------------------------------------
void FtpClientDTP::handleResolve(
   boost::system::error_code const&         ftperror,
   boost::asio::ip::tcp::resolver::iterator iter)
{
   if (!ftperror)
   {
      // Attempt a connection to the first endpoint in the list.
      // Each endpoint will be tried until we successfully establish
      // a connection.
      boost::asio::ip::tcp::endpoint const a_endpoint(*iter);
      m_socket.async_connect(
         a_endpoint,
         boost::bind(
            &self_type::handleConnect,
            shared_from_this(),
            boost::asio::placeholders::error,
            ++iter));
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClientDTP:" << std::endl;
         s << "handleResolve failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle connection
//----------------------------------------------------------------------------------------
void FtpClientDTP::handleConnect(
   boost::system::error_code const&         ftperror,
   boost::asio::ip::tcp::resolver::iterator iter)
{
   if (!ftperror)
   {
      // Read the response status line.
      handleWriteContent(ftperror);
   }
   else if (iter != boost::asio::ip::tcp::resolver::iterator())
   {
      // The connection failed. Try the next endpoint in the list.
      m_socket.close();
      boost::asio::ip::tcp::endpoint const a_endpoint(*iter);
      m_socket.async_connect(
         a_endpoint,
         boost::bind(
            &self_type::handleConnect,
            shared_from_this(),
            boost::asio::placeholders::error,
            ++iter));
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClientDTP:" << std::endl;
         s << "handleConnect failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Write content to file
//----------------------------------------------------------------------------------------
void FtpClientDTP::handleWriteContent(
   boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      m_stream.open(
         m_filename.c_str(),
         ios::binary | ios_base::in);
      if (!m_stream.is_open())
      {
         closeSocket();
      }
      else
      {
         string dataTransfer((istreambuf_iterator<char>(m_stream)), istreambuf_iterator<char>());
         vector<char> vec_buf(dataTransfer.begin(),dataTransfer.end());
         
         boost::asio::async_write(
            m_socket,
            boost::asio::buffer(vec_buf, vec_buf.size()),
            boost::asio::transfer_at_least(1),
            boost::bind(
               &self_type::closeSocket,
               shared_from_this())
               );
         
         m_stream.close();
      }
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClientDTP:" << std::endl;
         s << "handleWriteContent failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Write data
//----------------------------------------------------------------------------------------
void FtpClientDTP::handleWrite(
   boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      // Read the response status line.
      boost::asio::async_read(
         m_socket,
         m_response,
         boost::bind(
            &self_type::handleWriteContent,
            shared_from_this(),
            boost::asio::placeholders::error));
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClientDTP:" << std::endl;
         s << "handleWrite failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      closeSocket();
   }  
}

//----------------------------------------------------------------------------------------
// Close socket
//----------------------------------------------------------------------------------------
void FtpClientDTP::closeSocket()
{
   m_socket.close();
}

}
