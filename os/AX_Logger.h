#ifndef _AX_LOGGER_H
#define _AX_LOGGER_H

#include <sys/timeb.h>

#include "AX_Mutex.h"
#include <string>
using namespace std;

#define MAX_LOG_HANDLERS_NUMBER 128
#define DEFAULT_LOG_BUF_LEN (100*1024)
#define MIN_LOG_BUF_LEN		(32*1024)

typedef enum _level_
{
	LEVEL_ALL,
	LEVEL_FINEST,
	LEVEL_FINER,
	LEVEL_FINE,
	LEVEL_CONFIG,
	LEVEL_INFO,
	LEVEL_WARNING,
	LEVEL_SEVERE,
	LEVEL_OFF
}LEVEL;

static char	levelNames[][8] =
{
	"ALL", 
	"FINEST", 
	"FINER", 
	"FINE",
	"CONFIG", 
	"INFO",
	"WARNING", 
	"SEVERE", 
	"OFF"
};

class CLogRecord
{
public:
	CLogRecord(LEVEL level, const char* loggerName, const char* message,
		const char* sourceFile, unsigned int sourceLine);

	int GetFormatedMessage(string& result,bool bformat);

	LEVEL GetLevel()
	{
		return m_level;
	}

	string& GetLoggerName()
	{
		return m_loggerName;
	}

	string& GetMessage()
	{
		return m_message;
	}

	timeb& GetTimeStamp()
	{
		return m_timeStamp;
	}

	unsigned int GetSequenceNumber()
	{
		return m_sequenceNumber;
	}

	void SetSequenceNumber(unsigned int sequenceNumber)
	{
		m_sequenceNumber = sequenceNumber;
	}

	string& GetSourceFile()
	{
		return m_sourceFile;
	}

	unsigned int GetSourceLine()
	{
		return m_sourceLine;
	}

private:
	LEVEL	m_level;
	string	m_loggerName;
	string	m_message;
	string	m_sourceFile;
	timeb	m_timeStamp;	
	unsigned int m_sequenceNumber;	
	unsigned int m_sourceLine;
};

class CLogHandler
{
public:
	CLogHandler(LEVEL level,bool format)
	{
		m_sequenceNumber = 0;
		m_currentLevel = level;
		m_format = format;
	}

	void SetLevel(LEVEL level)
	{
		m_currentLevel = level;
	}

	LEVEL GetLevel(void)
	{
		return m_currentLevel;
	}

	virtual bool Publish(CLogRecord& record) = 0;

protected:
	bool m_format;
	unsigned int m_sequenceNumber;
private:
	LEVEL	m_currentLevel;
};

class CFileLogHandler : public CLogHandler
{
public:
	CFileLogHandler(LEVEL level, const char* fileName,
		unsigned int maxLength = 1048576, bool console = false,bool format = true);
	virtual ~CFileLogHandler(void);

	void SetBuffer(int bufLen);

	bool Publish(CLogRecord& record);
	
	int FlushRecords();
	static void ForceFlush();
protected:
	bool MakeSureDirectoryExist(const char* dir);
	bool OpenLogFile();

private:
	char*	m_pBuf;
	int		m_bufLen;
	int		m_writePos;
	string	m_fileName;	
	int		m_fileHandler;
	unsigned int	m_maxLength;
	static int _controlcout;
	static bool m_forceFlush;
	//AX_Mutex mutex;
};

class CLogger
{
public:
	CLogger(const char* loggerName);

	bool log(LEVEL level, const char* sourceFile, unsigned int sourceLine,
		const char* message);

	bool logf(LEVEL level,const char* sourceFile, unsigned int sourceLine,
		const char* message,...);

	int AddHandler(CLogHandler* logHandler);
	CLogHandler* GetHandlerAt(int index);
	CLogHandler* RemoveHandlerAt(int index);

private:
	string		 m_loggerName;
	CLogHandler* m_logHandlers[MAX_LOG_HANDLERS_NUMBER];
	AX_Mutex	m_mutex;
};


#define FINEST_LOG(logger,message) \
{\
	(logger).log(LEVEL_FINEST,__FILE__,__LINE__,message);\
};

#define FINER_LOG(logger,message) \
{\
	(logger).log(LEVEL_FINER,__FILE__,__LINE__,message);\														\
};

#define FINE_LOG(logger,message) \
{\
	(logger).log(LEVEL_FINE,__FILE__,__LINE__,message);\													\
};

#define CONFIG_LOG(logger,message) \
{\
	(logger).log(LEVEL_CONFIG,__FILE__,__LINE__,message);\												\
};

#define INFO_LOG(logger,message) \
{\
	(logger).log(LEVEL_INFO,__FILE__,__LINE__,message);\
};

#define WARNING_LOG(logger,message) \
{\
	(logger).log(LEVEL_WARNING,__FILE__,__LINE__,message);\
};

#define SEVERE_LOG(logger,message) \
{\
	(logger).log(LEVEL_SEVERE,__FILE__,__LINE__,message);\
};
#endif
