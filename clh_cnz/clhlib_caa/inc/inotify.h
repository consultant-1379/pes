//#<heading>
//----------------------------------------------------------------------------------------
//
//  FILE
//      inotify.h
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

#ifndef INOTIFY_H_
#define INOTIFY_H_

#include <string>

namespace PES_CLH {

//----------------------------------------------------------------------------------------
// Valid events:
//
// The following are legal, implemented events that user-space can watch for
// IN_ACCESS              File was accessed
// IN_MODIFY              File was modified
// IN_ATTRIB              Meta data changed
// IN_CLOSE_WRITE         Writable file was closed
// IN_CLOSE_NOWRITE       Unwritable file closed
// IN_OPEN                File was opened
// IN_MOVED_FROM          File was moved from X
// IN_MOVED_TO            File was moved to Y
// IN_CREATE              Subfile was created
// IN_DELETE              Subfile was deleted
// IN_DELETE_SELF         Self was deleted
// IN_MOVE_SELF           Self was moved
//
// The following are legal events. They are sent as needed to any watch
// IN_UNMOUNT             Backing file system was unmounted
// IN_Q_OVERFLOW          Event queued overflowed
// IN_IGNORED             File was ignored
//
// Helper events
// IN_CLOSE               File was closed (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)
// IN_MOVE                File was moved (IN_MOVED_FROM | IN_MOVED_TO)
// IN_ALL_EVENTS          All of the events
//
// Special flags
// IN_ONLYDIR             Only watch the path if it is a directory
// IN_DONT_FOLLOW         Don't follow a symbolic link
// IN_EXCL_UNLINK         Exclude events on unlinked objects
// IN_MASK_ADD            Add to the mask of an already existing watch
// IN_ISDIR               Event occurred against directory
// IN_ONESHOT             Only send event once
//
//----------------------------------------------------------------------------------------

#include <sys/inotify.h>

class Inotify
{
public:
   class Event
   {
      friend class Inotify;

      friend std::ostream& operator<<(
            std::ostream& s,
            const Event& event
            );

   public:
      Event();
      ~Event();

      uint32_t getWd() const;
      uint32_t getMask() const;
      std::string getName() const;

   private:

      int m_wd;
      uint32_t m_mask;
      std::string m_name;
   };

   // Constructor
   Inotify();

   // Destructor
   ~Inotify();

   // Open file notifications
   int open(
         bool block = true
         );

   // Close file notifications
   void close();

   // Add a file watch
   int addWatch(
         const std::string& path,
         uint32_t mask
         );

   // Remove a file watch
   void rmWatch(
         int wd
         );

   // Get a file event
   bool getEvent(Event& event);

private:
   int m_ifd;
   static const int s_bufsize = 4096;
   char m_eventbuf[s_bufsize];
   char* m_readptr;
   char* m_writeptr;
};

}

#endif // INOTIFY_H_
