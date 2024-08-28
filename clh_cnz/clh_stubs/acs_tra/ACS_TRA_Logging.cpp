#include "ACS_TRA_Logging.h"
#include <string>

using namespace std;

const char ACS_TRA_Logging::s_logpath[] = "/var/log/acs/tra/logging/";

ACS_TRA_Logging::ACS_TRA_Logging():
m_fs()
{
}

ACS_TRA_Logging::~ACS_TRA_Logging()
{
}

ACS_TRA_LogResult ACS_TRA_Logging::Open(const char* logfile)
{
	string tlogfile = s_logpath;
	tlogfile += logfile;
	tlogfile += ".log";
   m_fs.open(tlogfile.c_str(), std::ios_base::out | std::ios_base::trunc);
	return m_fs.is_open()? TRA_LOG_OK: TRA_LOG_ERROR;
}

void ACS_TRA_Logging::Close()
{
   if (m_fs.is_open())
	{
	   m_fs.close();
	}
}

ACS_TRA_LogResult ACS_TRA_Logging::Write(const char* message, ACS_TRA_LogLevel Log_Level)
{
	m_fs << message << endl;
	m_fs.flush();
	return TRA_LOG_OK;
}

ACS_TRA_LogResult ACS_TRA_Logging::getLastError(void) const
{
	return TRA_LOG_OK;
}

const char* ACS_TRA_Logging::getLastErrorText(void) const
{
	return "";
}
