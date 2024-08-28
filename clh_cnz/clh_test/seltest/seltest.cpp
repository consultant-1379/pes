#include <cmdparser.h>
#include <boardinfo.h>
#include <exception.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

using namespace std;
using namespace PES_CLH;

//----------------------------------------------------------------------------------------
// Usage
//----------------------------------------------------------------------------------------
void usage()
{
   cout << "Usage: seltest -r subrack -s slot" << endl;
}

//----------------------------------------------------------------------------------------
// Main program
//----------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
   try
   {
      char path[] = "/tmp/seltest";
      struct stat st;
      if (stat(path, &st) == -1)
      {
         int ret = mkfifo(path, 0666);
         if (ret != 0)
         {
            Exception ex(Exception::system(), WHERE__);
            ex << "Failed to create FIFO.";
            throw;
         }
      }

      // Declare command options
      CmdParser::Optarg optSubRack("r");
      CmdParser::Optarg optSlot("s");

      // Parse command
      CmdParser cmdparser(argc, argv);

      // Analyze parameters
      cmdparser.fetchOpt(optSubRack);
      cmdparser.fetchOpt(optSlot);

      // End of command check
      cmdparser.check();

      if (!optSubRack.found() || !optSlot.found())
      {
         throw Exception(Exception::usage(), WHERE__);
      }

      Magazine magazine(optSubRack.getArg());
      Slot slot(optSlot.getArg());
      cout << "Magazine: " << magazine << endl;
      cout << "Slot:     " << slot << endl;

      int fd = open(path, O_WRONLY /*| O_NONBLOCK*/);

      // Write data
      char data[64];
      const char event[] = "FC89724C0104C00070258800000400";
      size_t offset = 0;
      reinterpret_cast<int&>(data[offset]) = 2;                   // OID
      offset += sizeof(int);
      reinterpret_cast<int&>(data[offset]) = magazine[0] | magazine[1] << 4 | magazine[3] << 8;
                                                                  // Subrack
      offset += sizeof(int);
      reinterpret_cast<int&>(data[offset]) = slot;                // Slot
      offset += sizeof(int);
      size_t size = strlen(event);
      reinterpret_cast<uint32_t&>(data[offset]) = size;           // Size
      offset += sizeof(uint32_t);
      strncpy(&data[offset], event, size);
      size += offset;

      // Send data
      size = write(fd, data, size);
      if (size == 0)
      {
         Exception ex(Exception::parameter(), WHERE__);
         ex << "Write failed.";
         throw ex;
      }

      close(fd);
   }
   catch (Exception& ex)
   {
      cerr << ex << endl;
      uint16_t retCode = ex.getErrCode();
      if (retCode == Exception::usage().first)
      {
         // If incorrect usage, print command format
         cerr << endl;
         usage();
      }
      cerr << endl;
      return 1;
   }

   return 0;
}
