#ifndef _AX_TRACE_H
#define _AX_TRACE_H


#ifdef _WIN32
	#define snprintf _snprintf
#endif

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
			int ret=snprintf(_pos,_len,"%c",c);
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
			int ret=snprintf(_pos,_len,"%c",uc);
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
			int ret=snprintf(_pos,_len,"%hd",s);
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
			int ret=snprintf(_pos,_len,"%hu",us);
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
			int ret=snprintf(_pos,_len,"%d",i);
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
			int ret=snprintf(_pos,_len,"%u",ui);
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
			int ret=snprintf(_pos,_len,"%ld",l);
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
			int ret=snprintf(_pos,_len,"%lu",ul);
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
			int ret=snprintf(_pos,_len,"%f",f);
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
			int ret=snprintf(_pos,_len,"%f",d);
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
				int ret=snprintf(_pos,_len,"%s",str);
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
			else
			{
				int ret=snprintf(_pos,_len,"NULL");
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
		}
		return *this;
	}
	AX_Text_Format& operator<<(void* p)
	{
		if(0<_len)
		{
			if(NULL!=p)
			{
				int ret=snprintf(_pos,_len,"%p",p);
				if(0<ret)
				{
					_pos+=ret;
					_len-=ret;
				}
			}
			else
			{
				int ret=snprintf(_pos,_len,"NULL");
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

class AX_ObjKey
{
protected:
	AX_ObjKey()
	{
	}
	~AX_ObjKey()
	{
	}
protected:
	char __objKey[64];
};

#define AX_SET_OBJKEY(str)	\
	{	\
		AX_Text_Format txtFormat(__objKey,sizeof(__objKey));	\
		txtFormat<<str;	\
	}

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

#define INFOTRACE(str)	 		AX_TRACE(21,"Info:",str<<"\n")
#define WARNINGTRACE(str)		AX_TRACE(11,"Warning:",str<<"\n")
#define ERRTRACE(str)			AX_TRACE(1,"Error:",str<<"\n")
#define DETAILTRACE(str)		AX_TRACE(23,"Detail:",str<<"\n")


#define INFOXTRACE(str)			AX_TRACE(21,"Info:[",__objKey<<"]"<<str<<"\n")
#define WARNINGXTRACE(str)		AX_TRACE(11,"Warning:[",__objKey<<"]"<<str<<"\n")
#define ERRXTRACE(str)			AX_TRACE(1,"Error:[",__objKey<<"]"<<str<<"\n")
#define DETAILXTRACE(str)		AX_TRACE(23,"Detail:[",__objKey<<"]"<<str<<"\n")




class AX_Logger_Func
{
public:
	AX_Logger_Func()
	{
		*_buf=0;
		_enable=false;
	}
	~AX_Logger_Func()
	{
		leave();
	}
public:
	char* getBuf()
	{
		return _buf;
	}
	int getBufSize()
	{
		return sizeof(_buf);
	}
	void enter()
	{
		_enable=true;
		char tmpBuf[2048]={0};
		AX_Text_Format txtFormat(tmpBuf,sizeof(tmpBuf));
		txtFormat<<_buf<<" Enter\n";
		AX_Logger::instance().trace(22,tmpBuf);	\
	}
	void leave()
	{
		if(_enable)
		{
			char tmpBuf[2048]={0};
			AX_Text_Format txtFormat(tmpBuf,sizeof(tmpBuf));
			txtFormat<<_buf<<" Leave\n";
			AX_Logger::instance().trace(22,tmpBuf);	\
		}
	}

protected:
	char  _buf[2048];
	bool  _enable;
};

#define FUNCTRACE(str)	\
	AX_Logger_Func   _$FUNCTRACE$;	\
	if(AX_Logger::instance().levelEnabled(22))	\
	{	\
		AX_Text_Format fm(_$FUNCTRACE$.getBuf(),_$FUNCTRACE$.getBufSize());	\
		fm<<"FUNC:"<<str;	\
		_$FUNCTRACE$.enter();	\
	}

	
#define FUNCXTRACE(str)	\
	AX_Logger_Func   _$FUNCTRACE$;	\
	if(AX_Logger::instance().levelEnabled(22))	\
	{	\
		AX_Text_Format fm(_$FUNCTRACE$.getBuf(),_$FUNCTRACE$.getBufSize());	\
		fm<<"FUNC:["<< __objKey<<"]"<<str;	\
		_$FUNCTRACE$.enter();	\
	}


#endif//_AX_TRACE_H

