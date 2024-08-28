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

#include "ftptask.h"
#include "logger.h"

using namespace std;
using namespace boost;

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
FtpClient::FtpClient(
         boost::asio::io_service& ioservice,
         string const&       server,
         string const&       userid,
         string const&       password,
         string const&       url,
         string const&       filename):
m_state(STATE_UNCONNECTED),
m_dtpcommand(COMMAND_NULL),
m_url(url),
m_userid(userid),
m_password(password),
m_filename(filename),
m_ioservice(ioservice),
m_resolver(ioservice),
m_socket(ioservice),
m_timer(ioservice)
{
   // Start an asynchronous resolve to translate the server and service names
   // into a list of endpoints.
   boost::asio::ip::tcp::resolver::query a_query(server, "ftp");
   m_resolver.async_resolve(
         a_query,
         boost::bind(
            &self_type::handleResolve,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator
            )
         );
   m_state = STATE_HOST_LOOKUP;
}

//----------------------------------------------------------------------------------------
// Close
//----------------------------------------------------------------------------------------
void FtpClient::close()
{
   m_ioservice.post(boost::bind(&self_type::closeSocket, this));
}

//----------------------------------------------------------------------------------------
// Get error number
//----------------------------------------------------------------------------------------
int FtpClient::error_num() const
{
   return m_error.value();
}

//----------------------------------------------------------------------------------------
// Get error message
//----------------------------------------------------------------------------------------
string FtpClient::error_what() const
{
   return m_error.message();
}

//----------------------------------------------------------------------------------------
// Get endpoints
//----------------------------------------------------------------------------------------
void FtpClient::handleResolve(
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
            this,
            boost::asio::placeholders::error,
            ++iter)
         );

      // time out setting
      m_timer.expires_from_now(boost::posix_time::seconds(5));
      m_timer.async_wait(boost::bind(&self_type::closeSocket, this));

      m_state = STATE_CONNECTING;
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleResolve failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle connection
//----------------------------------------------------------------------------------------
void FtpClient::handleConnect(
         boost::system::error_code const&         ftperror,
         boost::asio::ip::tcp::resolver::iterator iter)
{
   if (!ftperror)
   {
      m_state = STATE_CONNECTED;

      // Read the response status line.
      boost::asio::async_read(
         m_socket,
         m_response,
         boost::asio::transfer_at_least(1),
         boost::bind(
            &self_type::handleReadLogin,
            this,
            boost::asio::placeholders::error)
         );
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
            this,
            boost::asio::placeholders::error,
            ++iter)
         );
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleConnect failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle write request for logging in
//----------------------------------------------------------------------------------------
void FtpClient::handleWriteRequestLogin(
   boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      // Read the response status line.
      boost::asio::async_read(
         m_socket,
         m_response,
         boost::asio::transfer_at_least(1),
         boost::bind(
            &self_type::handleReadLogin,
            this,
            boost::asio::placeholders::error)
         );
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleWriteRequestLogin failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle read request for logging in
//----------------------------------------------------------------------------------------
void FtpClient::handleReadLogin(boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      // Check that response is OK.
      istream a_response_stream(&m_response);
        cout << &m_response;

      ostream a_requset_stream(&m_request);
      switch(m_state)
      {
         case STATE_CONNECTED:
         a_requset_stream << "USER " << m_userid << "\r\n";
         // The connection was successful. Send the request.
         boost::asio::async_write(
            m_socket,
            m_request,
            boost::bind(
               &self_type::handleWriteRequestLogin,
               this,
               boost::asio::placeholders::error));
         m_state = STATE_PASSWORD_REQUIRED;
         return;

         case STATE_PASSWORD_REQUIRED:
         a_requset_stream << "PASS " << m_password << "\r\n";
         // The connection was successful. Send the request.
         boost::asio::async_write(
            m_socket,
            m_request,
            boost::bind(
               &self_type::handleWriteRequestCommand,
               this,
               boost::asio::placeholders::error));
         m_state = STATE_PASSWORD_CERTIFYING;
         return;

         default:
         break;
      }
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleReadLogin failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle write request commands
//----------------------------------------------------------------------------------------
void FtpClient::handleWriteRequestCommand(boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      // Read the response status line.
      boost::asio::async_read(
         m_socket,
         m_response,
         boost::asio::transfer_at_least(1),
         boost::bind(
            &self_type::handleReadCommand,
            this,
            boost::asio::placeholders::error));

   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleWriteRequestCommand failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle reading commands
//----------------------------------------------------------------------------------------
void FtpClient::handleReadCommand(boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      // Check that response is OK
      string respstr = readResponse();
      string tmp = respstr.substr(0, 1);
      
      if ("5" == tmp)
      {
         m_state = STATE_UNCONNECTED;
         closeSocket();
         return;
      }
      
      ostream a_requset_stream(&m_request);
      switch(m_state)
      {
         case STATE_PASSWORD_CERTIFYING:
            m_state = STATE_LOGGED_IN;
         case STATE_LOGGED_IN:
            a_requset_stream << "CWD " << m_url << "\r\n";
            // The connection was successful. Send the request.
            boost::asio::async_write(
               m_socket,
               m_request,
               boost::bind(
                  &self_type::handleWriteRequestCommand,
                  this,
                  boost::asio::placeholders::error));
            m_state = STATE_CHANGED_DIRECTORY;
            return;
         case STATE_CHANGED_DIRECTORY:
            a_requset_stream << "TYPE I\r\n";
            // The connection was successful. Send the request.
            boost::asio::async_write(
               m_socket,
               m_request,
               boost::bind(
                  &self_type::handleWriteRequestCommand,
                  this,
                  boost::asio::placeholders::error));
            m_state = STATE_SET_TYPE_BINARY;
            return;
         case STATE_SET_TYPE_BINARY:
            a_requset_stream << "PASV\r\n";
            m_dtpcommand = COMMAND_PASV;
            // The connection was successful. Send the request.
            boost::asio::async_write(
               m_socket,
               m_request,
               boost::bind(
                  &self_type::handleWriteRequestContent,
                  this,
                  boost::asio::placeholders::error));
            m_state = STATE_SET_PASSIVE_MODE;
            return;
         default:
            break;
      }
   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleReadCommand failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle writing content
//----------------------------------------------------------------------------------------
void FtpClient::handleWriteRequestContent(
   boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      // Read the response status line.
      boost::asio::async_read(
         m_socket,
         m_response,
         boost::asio::transfer_at_least(1),
         boost::bind(
            &self_type::handleReadContent,
            this,
            boost::asio::placeholders::error));

   }
   else
   {
      // Log event
      Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleWriteRequestContent failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Handle read content
//----------------------------------------------------------------------------------------
void FtpClient::handleReadContent(
   boost::system::error_code const& ftperror)
{
   if (!ftperror)
   {
      string respstr = "";
      switch (m_dtpcommand)
      {
         case COMMAND_PASV:
            commandPASV(readResponse());
            return;

         case COMMAND_STOR:
            respstr = readResponse().substr(0, 3);
            if ("226" == respstr)
            {
               commandSTOR();
               return;
            }
            break;
         default:
            break;
      }

      // Continue reading remaining data until EOF.
      boost::asio::async_read(
         m_socket,
         m_response,
         boost::asio::transfer_at_least(1),
         boost::bind(
            &self_type::handleReadContent,
            this,
            boost::asio::placeholders::error));
   }
   else
   {
      // Log event
      /*Logger logger(LOG_LEVEL_WARN);
      if (logger)
      {
         std::ostringstream s;
         s << "FtpClient:" << std::endl;
         s << "handleReadContent failed (" << ftperror << ")";
         logger.event(WHERE__, s.str());
      }*/
      m_error = ftperror;
      closeSocket();
   }
}

//----------------------------------------------------------------------------------------
// Close socket
//----------------------------------------------------------------------------------------
void FtpClient::closeSocket()
{
   m_timer.cancel();
   m_socket.close();
}

//----------------------------------------------------------------------------------------
// Read response
//----------------------------------------------------------------------------------------
string FtpClient::readResponse()
{
   istream a_response_stream(&m_response);
   string a_response;
   while (getline(a_response_stream, a_response) && a_response != "\r")
   {
      a_response_stream >> a_response;
   }
   return a_response;
}

//----------------------------------------------------------------------------------------
// Execute PASV command
//----------------------------------------------------------------------------------------
void FtpClient::commandPASV(
   string const& ftpresponse)
{
   // IP
   boost::regex const a_regex_ip(
      ".+\\(([0-9]{1,}),([0-9]{1,}),([0-9]{1,}),([0-9]{1,}),([0-9]{1,}),([0-9]{1,})\\).*");
   string const a_ip(
      boost::regex_replace(
         ftpresponse, a_regex_ip, "$1.$2.$3.$4", boost::format_all));
   unsigned int const a_port_hi(
      boost::lexical_cast< unsigned int >(
         boost::regex_replace(
            ftpresponse, a_regex_ip, "$5", boost::format_all)));
   unsigned int const a_port_low(
      boost::lexical_cast< unsigned int >(
         boost::regex_replace(
            ftpresponse, a_regex_ip, "$6", boost::format_all)));
   string const a_port(
      boost::lexical_cast< string >(a_port_hi * 256 + a_port_low));

   ostream a_request_stream(&m_request);
   // Append to existing file. Otherwise, create new one.
   a_request_stream << "APPE sel.tmp\r\n";
   
   m_newconnection = FtpClientDTP::create(m_ioservice);
   m_newconnection->setInputFilename(m_filename);
   m_newconnection->setInfo(a_ip, a_port);
   m_newconnection->start();
   
   m_dtpcommand = COMMAND_STOR;

   // The connection was successful. Send the request.
   boost::asio::async_write(
      m_socket,
      m_request,
      boost::bind(
         &self_type::handleWriteRequestContent,
         this,
         boost::asio::placeholders::error));
   m_state = STATE_STORING;
}

//----------------------------------------------------------------------------------------
// Execute STOR command
//----------------------------------------------------------------------------------------
void FtpClient::commandSTOR()
{
   // end of writing file
   ostream a_request_stream(&m_request);
   a_request_stream << "QUIT\r\n";
   m_dtpcommand = COMMAND_NULL;

   // The connection was successful. Send the request.
   boost::asio::async_write(
      m_socket,
      m_request,
      boost::bind(
         &self_type::handleWriteRequestContent,
         this,
         boost::asio::placeholders::error));
   m_state = STATE_STORED;
}

//----------------------------------------------------------------------------------------
// Check if the file is stored
//----------------------------------------------------------------------------------------
bool FtpClient::isStored() const
{
   return (m_state == STATE_STORED);
}

}
