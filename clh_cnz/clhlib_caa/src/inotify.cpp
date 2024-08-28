//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      inotify.cpp
//
//  COPYRIGHT
//      Copyright Ericsson AB 2012. All rights reserved.
//      
//      The Copyright to the computer program(s) herein is the property of
//      Ericsson AB, Sweden. The program(s) may be used and/or copied only
//      with the written permission from Ericsson AB or in accordance with
//      the terms and conditions stipulated in the agreement/contract under
//      which the program(s) have been supplied.
//
//  DESCRIPTION
//      Class for handling file notifications.
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
//      PA1    2012-07-03   UABTSO      CLH adapted to APG43 on Linux.
//
//  SEE ALSO
//      -
//
//----------------------------------------------------------------------------------------
//#</heading>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#include "inotify.h"
#include "exception.h"
#include "logger.h"

using namespace std;

namespace PES_CLH {

//========================================================================================
// Class Inotify
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
Inotify::Inotify() :
m_ifd(-1),
m_eventbuf(),
m_readptr(0),
m_writeptr(0)
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Inotify::~Inotify()
{
}

//----------------------------------------------------------------------------------------
// Open file notifications
//----------------------------------------------------------------------------------------
int Inotify::open(bool block)
{
   m_ifd = inotify_init();
   if (m_ifd < 0)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to initialize notification.";
      ex.sysError();
      throw ex;
   }
   if (block == false)
   {
      // Non blocking read
      fcntl(m_ifd, F_SETFL, O_NONBLOCK);
   }
   m_readptr = m_eventbuf;
   m_writeptr = m_eventbuf;

   return m_ifd;
}

//----------------------------------------------------------------------------------------
// Close file notifications
//----------------------------------------------------------------------------------------
void Inotify::close()
{
   if (m_ifd)
   {
      int ret = ::close(m_ifd);
      if (ret)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to close notification.";
         Logger::event(ex);
      }
   }

   m_ifd = -1;
   m_readptr = 0;
   m_writeptr = 0;
}

//----------------------------------------------------------------------------------------
// Add a file watch
//----------------------------------------------------------------------------------------
int Inotify::addWatch(const string& path, uint32_t mask)
{
   int wd = inotify_add_watch(m_ifd, path.c_str(), mask);
   if (wd == -1)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to add a notification watch.";
      ex.sysError();
      throw ex;
   }

   return wd;
}

//----------------------------------------------------------------------------------------
// Remove a file watch
//----------------------------------------------------------------------------------------
void Inotify::rmWatch(int wd)
{
   int ret = inotify_rm_watch(m_ifd, wd);
   if (ret == -1)
   {
      Exception ex(Exception::system(), WHERE__);
      ex << "Failed to remove a notification watch.";
      ex.sysError();
      throw ex;
   }
}

//----------------------------------------------------------------------------------------
// Read a file event
//----------------------------------------------------------------------------------------
bool Inotify::getEvent(Inotify::Event& event)
{
   if (m_readptr == m_writeptr)
   {
      // Buffer is empty - read new events and store in buffer
      m_readptr = m_eventbuf;
      m_writeptr = m_eventbuf;

      int buflen = ::read(m_ifd, m_eventbuf, s_bufsize);
      if (buflen == -1)
      {
         if (errno == EAGAIN)
         {
            return false;
         }
         else
         {
            Exception ex(Exception::system(), WHERE__);
            ex << "Failed to read a notification event.";
            ex.sysError();
            throw ex;
         }
      }
      else if (buflen > s_bufsize)
      {
         Exception ex(Exception::internal(), WHERE__);
         ex << "Buffer overflow.";
         throw ex;
      }

      m_writeptr += buflen;
   }

   if (m_readptr < m_writeptr)
   {
      // Get an event from buffer and return
      const inotify_event* ievent = reinterpret_cast<const inotify_event*>(m_readptr);

      event.m_wd = ievent->wd;
      event.m_mask = ievent->mask;
      event.m_name = ievent->len ? ievent->name : "";
      m_readptr += sizeof(inotify_event) + ievent->len;

      if (ievent->mask & IN_UNMOUNT)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to read a notification event." << endl;
         ex << "The backing filesystem was unmounted.";
         throw ex;
      }

      if (ievent->mask & IN_Q_OVERFLOW)
      {
         Exception ex(Exception::system(), WHERE__);
         ex << "Failed to read a notification event." << endl;
         ex << "The inotify queue overflowed.";
         throw ex;
      }
   }
   else
   {
      Exception ex(Exception::internal(), WHERE__);
      ex << "Event buffer inconsistency.";
      throw ex;
   }

   return true;
}

//========================================================================================
// Subclass Event
//========================================================================================

//----------------------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------------------
Inotify::Event::Event() :
m_wd(0),
m_mask(0),
m_name()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
Inotify::Event::~Event()
{
}

//----------------------------------------------------------------------------------------
// Get watch descriptor
//----------------------------------------------------------------------------------------
uint32_t Inotify::Event::getWd() const
{
   return m_wd;
}

//----------------------------------------------------------------------------------------
// Get event mask
//----------------------------------------------------------------------------------------
uint32_t Inotify::Event::getMask() const
{
   return m_mask;
}

//----------------------------------------------------------------------------------------
// Get event name
//----------------------------------------------------------------------------------------
std::string Inotify::Event::getName() const
{
   return m_name;
}

//----------------------------------------------------------------------------------------
// Outstream operator
//----------------------------------------------------------------------------------------
ostream& operator<<(ostream& s, const Inotify::Event& event)
{
   s << "Watch handle " << event.m_wd;
   s << ", file name '" << event.m_name << "'." << endl;
   for (uint32_t tmask = 1; tmask != 0; tmask<<= 1)
   {
      switch (event.m_mask & tmask)
      {
      case 0:
         break;
      case IN_ACCESS:
         s << "File was accessed." << endl;
         break;
      case IN_MODIFY:
         s << "File was modified." << endl;
         break;
      case IN_ATTRIB:
         s << "Metadata was changed." << endl;
         break;
      case IN_CLOSE_WRITE:
         s << "Writable file was closed." << endl;
         break;
      case IN_CLOSE_NOWRITE:
         s << "Unwritable file was closed." << endl;
         break;
      case IN_OPEN:
         s << "File was opened." << endl;
         break;
      case IN_MOVED_FROM:
         s << "File was moved from X." << endl;
         break;
      case IN_MOVED_TO:
         s << "File was moved to Y." << endl;
         break;
      case IN_CREATE:
         s << "Subfile was created." << endl;
         break;
      case IN_DELETE:
         s << "Subfile was deleted." << endl;
         break;
      case IN_DELETE_SELF:
         s << "Self was deleted." << endl;
         break;
      case IN_MOVE_SELF:
         s << "Self was moved." << endl;
         break;
      case IN_UNMOUNT:
         s << "Backing file system was unmounted." << endl;
         break;
      case IN_Q_OVERFLOW:
         s << "Event queued overflowed." << endl;
         break;
      case IN_IGNORED:
         s << "File was ignored." << endl;
         break;
      case IN_ISDIR:
         s << "The event occurred against a directory." << endl;
         break;
      default:
         s << "Unknown event (" << tmask << ")." << endl;
      }
   }
   return s;
}

}
