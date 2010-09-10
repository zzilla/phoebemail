#include "AX_Logger.h"
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#include <share.h>
#include <direct.h>
#include <assert.h>
#include <tchar.h>
#pragma warning(disable : 4996)
#else
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#endif

#ifndef assert
#define assert
#endif

bool CFileLogHandler::m_forceFlush=false;

int CFileLogHandler::_controlcout = 0;

//AX_Mutex CFileLogHandler::mutex;
CLogRecord::CLogRecord(LEVEL level, const char* loggerName,
					   const char* message, const char* sourceFile, unsigned int sourceLine)
					   : m_loggerName(loggerName), m_message(message), m_sourceFile(sourceFile)
{
	m_level = level;
	ftime(&m_timeStamp);
	m_sourceLine = sourceLine;
}

int CLogRecord::GetFormatedMessage(string& result,bool bformat)
{
	if (bformat)
	{
		char	tempBuffer[256];
		struct tm*	tempTm = localtime(&m_timeStamp.time);
		result += "Logger name: ";
		result += m_loggerName;
		result += "\nLevel: ";
		result += levelNames[m_level];
		strftime(tempBuffer, 255, "%d/%m/%Y %H:%M:%S.", tempTm);
		result += "\nTime: ";
		result += tempBuffer;
		sprintf(tempBuffer, "%u", m_timeStamp.millitm);
		result += tempBuffer;
		result += "\nSequence: ";
		sprintf(tempBuffer, "%u", m_sequenceNumber);
		result += tempBuffer;
		result += "\nSource file: ";
		result += m_sourceFile;
		result += "\nSource line: ";
		sprintf(tempBuffer, "%u", m_sourceLine);
		result += tempBuffer;
		result += "\nMessage: ";
		result += m_message;
		result += "\n\n";
	}
	else
	{
		result += m_message;
		result += "\n";
	}

	return 0;
}

CFileLogHandler::CFileLogHandler(LEVEL level, const char* fileName, 
								 unsigned int maxLength, bool console,bool format)
								 : CLogHandler(level,format), m_fileName(fileName)
{
	m_maxLength = maxLength;
	m_pBuf  = NULL;

	if ( console)
	{
		_controlcout++;
		m_fileHandler = 2;
	}
	else
	{	
		if ( MakeSureDirectoryExist(fileName) )
		{
			OpenLogFile();
		}

		m_pBuf = new char[DEFAULT_LOG_BUF_LEN];
		m_bufLen = DEFAULT_LOG_BUF_LEN;
		m_writePos = 0;
	}
}

CFileLogHandler::~CFileLogHandler()
{
	FlushRecords();
	//关闭文件
#ifdef WIN32
	//mutex.acquire();
	//_controlcout --;
	//mutex.release();
	if (_controlcout <=0)
	{
		_close(m_fileHandler);
	}
#else
	if (m_fileHandler != -1)
	{
		close(m_fileHandler);
		m_fileHandler = -1;
	}
#endif

	if ( NULL != m_pBuf )
	{
		delete []m_pBuf;
		m_pBuf = NULL;
	}
}

void CFileLogHandler::SetBuffer(int bufLen)
{
	if ( m_bufLen != bufLen && bufLen >= MIN_LOG_BUF_LEN )
	{
		if ( NULL != m_pBuf )
		{
			delete []m_pBuf;
			m_pBuf = NULL;
		}

		m_pBuf = new char[bufLen];
		m_bufLen = bufLen;
	}
}

bool CFileLogHandler::Publish(CLogRecord& record)
{
	if (GetLevel() == LEVEL_OFF)
	{
		return true;
	}

	if (record.GetLevel() < GetLevel())
	{
		return true;
	}

	record.SetSequenceNumber(m_sequenceNumber++);

	string buffer("");
	record.GetFormatedMessage(buffer, m_format);
	int len = (int)buffer.size();

	if ( 2 == m_fileHandler)
	{
		//控制台，直接写入
#ifdef WIN32
		if (_write(m_fileHandler, buffer.c_str(), len) != len)
#else
		if (write(m_fileHandler, buffer.c_str(), len) != len)
#endif		
		{
			assert(false);
			return false;
		}
	}
	else if ( m_fileHandler != -1 )
	{
		const char* data = buffer.c_str();
		int freeLen = m_bufLen - m_writePos;
		int writelen = (freeLen > len) ? len : freeLen;

		memcpy(m_pBuf + m_writePos, data, writelen);
		m_writePos += writelen;
#ifndef WIN32
		m_forceFlush = true;
#endif
		//如果缓存已满，就写入到磁盘
		if ( m_writePos == m_bufLen || (m_forceFlush && 0<m_writePos))
		{
			m_forceFlush=false;
			
			if(FlushRecords() < 0)
			{
			   return false;
			}
			
			//检查文件大小是否到上限			
#ifdef WIN32
			struct _stat statistics;
			if (m_fileHandler != -1 && _fstat(m_fileHandler, &statistics) == 0)  //如果磁盘满FlushRecords会将句柄给关掉
#else
			struct stat statistics;
			if (m_fileHandler != -1 && fstat(m_fileHandler, &statistics) == 0)
#endif			
			{
				if ((unsigned int)statistics.st_size >= m_maxLength)
				{
#ifdef WIN32
					_close(m_fileHandler);
#else
					close(m_fileHandler);
#endif				
					m_fileHandler = -1;

					//打开新的日志文件
					OpenLogFile();
				}
			}
		} //end of if ( m_writePos + len >= m_bufLen )

		//剩余数据写入到缓存
		int remainLen = len - writelen; 
		
		if ( remainLen > 0 && m_pBuf != NULL )
		{
			memcpy(m_pBuf + m_writePos, data + writelen, remainLen);
			m_writePos += remainLen;
		}
	}
	
    //	FlushRecords();
	return true;
}

void CFileLogHandler::ForceFlush()
{
	m_forceFlush=true;
}

int CFileLogHandler::FlushRecords()
{
	if ( -1 == m_fileHandler || 2 == m_fileHandler )
	{
		return -1;
	}

#ifdef WIN32
	int writeLen = _write(m_fileHandler, m_pBuf, m_writePos);
#else
	int writeLen = write(m_fileHandler, m_pBuf, m_writePos);
#endif

	if (writeLen != m_writePos)
	{
		//assert(false);   

		if ( writeLen != -1 )
		{
			memmove(m_pBuf, m_pBuf + writeLen, m_writePos - writeLen);
			m_writePos -= writeLen;
		}
		else
		{
			//写入失败，就打开新的文件来写入
#ifdef WIN32
			_close(m_fileHandler);
#else
			close(m_fileHandler);
#endif				
			m_fileHandler = -1;
			//打开新的日志文件
			if (errno != ENOSPC)	///等缓冲区的长度大于磁盘空间的时候，会进入到这里
			{
			OpenLogFile();
				return FlushRecords();
			}
			else
				return -2;
		}
	}
	else
	{
		m_writePos = 0;
	}

	return 0;
}

bool CFileLogHandler::MakeSureDirectoryExist(const char* dir)
{
	char temp[256];
	int len = (int)strlen(dir);

	strcpy(temp, dir);

	for ( int i = 0; i < len; ++i )
	{
		//windows下的分隔符可能是'\\'
		if ( dir[i] == '\\' || dir[i] == '/' )
		{
			//将该字符设成结束符，temp可以截取前面一段作为字符串访问
			temp[i] = '\0';

#ifdef WIN32
			if ( _access(temp, 0) == -1 )
			{
				if ( _mkdir(temp) == -1 )
				{
#else
			if ( access(temp, 0) == -1 )
			{
				if ( mkdir(temp, 0777) == -1 )
				{
#endif	
					return false;
				}
			}

			//再将该字符还原，以便访问下个目录
			temp[i] = dir[i];
		}
	}

	return true;
}

inline bool CFileLogHandler::OpenLogFile()
{
	time_t curtime = time(NULL);
	struct tm* temp = localtime(&curtime);
	char filename[256];

	sprintf(filename, "%s_%02d_%02d_%02d_%02d_%02d_%02d.log", 
		m_fileName.c_str(), 
		temp->tm_year + 1900, temp->tm_mon + 1, temp->tm_mday, 
		temp->tm_hour, temp->tm_min, temp->tm_sec);

#ifdef WIN32
	m_fileHandler = _sopen(filename,
		_O_CREAT | _O_APPEND | _O_BINARY | _O_WRONLY,
		_SH_DENYNO, 
		_S_IREAD | _S_IWRITE);
#else
	m_fileHandler = open(filename,
		O_CREAT | O_APPEND | O_WRONLY,
		S_IREAD | S_IWRITE);
#endif


	if ( -1 == m_fileHandler )
	{
		if (errno != ENOSPC)
		{
			assert(false);
		}
		
		//printf("-1 == m_fileHandler errno:%d\n",errno);
		//DWORD dwError = GetLastError();
		
		return false;
	}

	return true;
}

CLogger::CLogger(const char* loggerName)
: m_loggerName(loggerName)
{
	memset(m_logHandlers, 0, sizeof(m_logHandlers));
}

bool CLogger::log(LEVEL level, const char* sourceFile, unsigned int sourceLine, const char* message)
{
	CLogRecord	record(level, m_loggerName.c_str(), message,
		sourceFile, sourceLine);
	m_mutex.acquire();

	for (int i = 0; i < MAX_LOG_HANDLERS_NUMBER; i++)
	{
		if (m_logHandlers[i] != NULL)
		{
			if ( !m_logHandlers[i]->Publish(record) )
			{
				assert(false);
			}
		}
	}
	m_mutex.release();
	return true;
}

int CLogger::AddHandler(CLogHandler* logHandler)
{
	int	i = 0;
	int freeIndex = -1;
	m_mutex.acquire();

	for (i = 0; i < MAX_LOG_HANDLERS_NUMBER; i++)
	{
		if (m_logHandlers[i] == logHandler)
		{
			m_mutex.release();
			return true;
		}
		else if ( NULL == m_logHandlers[i] && -1 == freeIndex )
		{
			freeIndex = i;
		}
	}

	if ( -1 != freeIndex )
	{
		m_logHandlers[freeIndex] = logHandler;
		m_mutex.release();
		return true;
	}
	m_mutex.release();
	return freeIndex;
}

CLogHandler* CLogger::GetHandlerAt(int index)
{
	m_mutex.acquire();
	CLogHandler *temp = NULL;
	if (index < MAX_LOG_HANDLERS_NUMBER)
	{
		temp = m_logHandlers[index];
	}
	m_mutex.release();
	return temp;
}

CLogHandler* CLogger::RemoveHandlerAt(int index)
{
	m_mutex.acquire();
	
	if (index >= MAX_LOG_HANDLERS_NUMBER)
	{
		m_mutex.release();
		return NULL;
	}
	
	CLogHandler* result = m_logHandlers[index];
	m_logHandlers[index] = NULL;
	m_mutex.release();
	
	return result;
}

static char buffer[1024] = {0};

bool CLogger::logf(LEVEL level, const char* sourceFile, unsigned int sourceLine, const char* message,...)
{
	va_list argp;
	m_mutex.acquire();
	va_start(argp, message);
#ifdef _WIN32
	_vsnprintf ((char*) buffer, sizeof (buffer) / sizeof (char),  
		(const char*)message, argp) ;
#else
	vsnprintf ((char*) buffer, sizeof (buffer) / sizeof (char),  
		(const char*)message, argp) ;
#endif

	va_end(argp);
	CLogRecord	record(level, m_loggerName.c_str(), buffer,
		sourceFile, sourceLine);

	for (int i = 0; i < MAX_LOG_HANDLERS_NUMBER; i++)
	{
		if (m_logHandlers[i] != NULL)
		{
			if ( !m_logHandlers[i]->Publish(record) )
			{
				//assert(false);
			}
		}
	}
	
	m_mutex.release();
	
	return true;
}

