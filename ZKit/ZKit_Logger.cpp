#include "ZKit_Logger.h"
#include "ZKit_Locker.h"

BEGIN_ZKIT
char* strLogLevel[] = 
{
	"All",
	"Debug",
	"Info",
	"Warn",
	"Error",
	"Fatal",
	"Off"
};

void Logger::WriteLog( const char* format, ... )
{
	if (m_level >= m_minLevel)
	{
		fprintf(stderr, "\n==================Log Begin====================\n");

		char str[30];
		time_t now = time(NULL);
		struct tm* pTm = localtime(&now);   
		_snprintf_s(str, sizeof(str), sizeof(str) - 1, "%d-%02d-%02d %d:%d:%d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);

		fprintf(stderr, "%s %d \n%s [%s]\n", m_file, m_line, str, strLogLevel[m_level]);

		va_list argp;
		va_start(argp, format);
		vfprintf (stderr, format, argp);
		va_end(argp);
		fprintf(stderr, "\n------------------Log End----------------------\n");
	}
	m_mutex.Unlock();
}

END_ZKIT