#ifndef ACS_APBM_API_H_
#define ACS_APBM_API_H_

#include "acs_apbm_trapmessage.h"
#include <string>
#include <vector>

class acs_apbm_api
{
public:
	// Default constructor
   acs_apbm_api();

	// Destructor
	virtual ~acs_apbm_api();

	// Subscribes the caller application to trap message notification from APBM server
	int subscribe_trap(int bitmap, acs_apbm::trap_handle_t& trap_handle);

   // Unsubscribes the caller application to trap message notification from APBM server
	int unsubscribe_trap(acs_apbm::trap_handle_t trap_handle);

	//	------
	//	Provides user with requested trap information
	//	------
	//	In - trapHandle is the handle for requested trap
	//	In/Out - value is a pointer on a string array to receive
	//	the information. Should be created by the user
	//	Out - returns number of elements in array value or -1 if
	//	there was an error
	//	------
	int get_trap(acs_apbm::trap_handle_t trap_handle, acs_apbm_trapmessage& trap_message);

	//	------
	//	Provides user with information about specified board
	//	------
	//	In - slotNr is the slot number for the board
	//	In/Out - BoardInfo is a pointer on a string array to
	//	receive the information. Should be created by the user
	//	Out - returns number of elements in array BoardInfo or
	//	-1 if there was an error
	//	------

private:
	acs_apbm_api(const acs_apbm_api& rhs);
	acs_apbm_api& operator=(const acs_apbm_api& rhs);
};

#endif // ACS_APBM_API_H_
