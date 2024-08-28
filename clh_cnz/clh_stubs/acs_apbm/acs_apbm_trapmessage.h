#ifndef ACS_APBM_TRAPMESSAGE_H_
#define ACS_APBM_TRAPMESSAGE_H_

#include <vector>

namespace acs_apbm
{
   typedef int trap_handle_t;
}

class acs_apbm_trapmessage
{
   friend class acs_apbm_api;

	// Constants
public:
	enum TrapTypeConstants {
		SENSOR_STATE_CHANGE	=	1,
		SEL_ENTRY				=	2,
		BOARD_PRESENCE			=	3,
		NIC						=	4,
		RAID						=	5,
		APBM_READY				=	6,
		DISKCONN					=	7
	};

	// Default constructor
	acs_apbm_trapmessage();

private:

public:
	// Destructor
	virtual ~acs_apbm_trapmessage();

	// Fields Accessor
	int OID() const;
	const std::vector<int>& values() const;
	const char* message() const;
	unsigned message_length() const;

private:
   acs_apbm_trapmessage(const acs_apbm_trapmessage& rhs);
	acs_apbm_trapmessage& operator=(const acs_apbm_trapmessage& rhs);

	// Fields
	mutable char m_data[64];
   mutable std::vector<int> m_vec;
};

#endif // ACS_APBM_TRAPMESSAGE_H_
