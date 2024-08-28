#include "acs_apbm_api.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
acs_apbm_api::acs_apbm_api()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
acs_apbm_api::~acs_apbm_api()
{
}

//----------------------------------------------------------------------------------------
// Subscribes the caller application to trap message notification from APBM server
//----------------------------------------------------------------------------------------
int acs_apbm_api::subscribe_trap(int bitmap, acs_apbm::trap_handle_t& trap_handle)
{
   trap_handle = open("/tmp/seltest", O_RDWR | O_NONBLOCK);
   return (trap_handle == -1)? -3: 0;
}

//----------------------------------------------------------------------------------------
// Unsubscribes the caller application to trap message notification from APBM server
//----------------------------------------------------------------------------------------
int acs_apbm_api::unsubscribe_trap(acs_apbm::trap_handle_t trap_handle)
{
   int ret = close(trap_handle);
   return (ret == -1)? -3: 0;
}

//----------------------------------------------------------------------------------------
// Provides user with requested trap information
//----------------------------------------------------------------------------------------
int acs_apbm_api::get_trap(acs_apbm::trap_handle_t trap_handle, acs_apbm_trapmessage& trap_message)
{
   // Write data
   int size = read(trap_handle, trap_message.m_data, 46);
   return (size > 0)? 0: -3;
}

