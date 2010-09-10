#ifndef _AX_TEXT_LOG_H
#define _AX_TEXT_LOG_H

#include "AX_Mutex.h"

class AX_Text_Format
{
public:
	AX_Text_Format(const char *buf, int len)
		:_pos((char*)buf),_buf((char*)buf),_len(len)
	{
	}
	~AX_Text_Format()
	{
	}
public:
	AX_Text_Format& operator<<(char c)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%c",c);
#else
			int ret=snprintf(_pos,_len,"%c",c);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned char uc)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%c",uc);
#else
			int ret=snprintf(_pos,_len,"%c",uc);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(short s)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%hd",s);
#else
			int ret=snprintf(_pos,_len,"%hd",s);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned short us)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%hu",us);
#else
			int ret=snprintf(_pos,_len,"%hu",us);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(int i)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%d",i);
#else
			int ret=snprintf(_pos,_len,"%d",i);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned int ui)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%u",ui);
#else
			int ret=snprintf(_pos,_len,"%u",ui);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(long l)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%ld",l);
#else
			int ret=snprintf(_pos,_len,"%ld",l);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(unsigned long ul)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%lu",ul);
#else
			int ret=snprintf(_pos,_len,"%lu",ul);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(float f)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%f",f);
#else
			int ret=snprintf(_pos,_len,"%f",f);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(double d)
	{
		if(0<_len)
		{
#ifdef _WIN32
			int ret=_snprintf(_pos,_len,"%f",d);
#else
			int ret=snprintf(_pos,_len,"%f",d);
#endif
			if(0<ret)
			{
				_pos+=ret;
				_len-=ret;
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(const char * str)
	{
		if(0<_len)
		{
			if(NULL!=str)
			{
#ifdef _WIN32
				int ret=_snprintf(_pos,_len,"%s",str);
#else
				int ret=snprintf(_pos,_len,"%s",str);
#endif
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
			else
			{
#ifdef _WIN32
				int ret=_snprintf(_pos,_len,"NULL");
#else
				int ret=snprintf(_pos,_len,"NULL");
#endif
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
		}
		return *this;
	}

protected:
	char* _pos;
	char* _buf;
	int _len;
};


struct AX_Logger_Config_Unit
{
	int _level;
	const char* _configNam;
	bool  _enabled;
};

class AX_Logger
{
public:
	~AX_Logger();
public:
	bool levelEnabled(int level);
	void refreshLoggerLevel();
	void trace(int level, const char* str);
	static AX_Logger& instance();
protected:
	void initLogFile();
	bool getProcessName(char* buf,int maxLen);
protected:
	static AX_Logger  _logger;
	static AX_Logger_Config_Unit  _configSet[];
	static bool s_inited;

	FILE* _errorFile;
	FILE* _warningFile;
	FILE* _infoFile;
	AX_Mutex _mutex;
private:
	void operator = (AX_Logger&);
	AX_Logger();
};


#define AX_TRACE(level, tag, str)	\
{	\
	if(AX_Logger::instance().levelEnabled((level)))	\
{	\
	char tmpBuf[2048]={0};	\
	AX_Text_Format txtFormat(tmpBuf,sizeof(tmpBuf));	\
	txtFormat<<tag<<str;	\
	AX_Logger::instance().trace(level,tmpBuf);	\
}	\
}

#define INFOTRACE(str)	\
	AX_TRACE(21,"Info:",str<<"\n")

#define WARNINGTRACE(str)	\
	AX_TRACE(11,"Warning:",str<<"\n")

#define ERRTRACE(str)	\
	AX_TRACE(1,"Error:",str<<"\n")
#endif

