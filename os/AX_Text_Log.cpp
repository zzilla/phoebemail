#include <stdlib.h>
#include <stdio.h>
#include "AX_OS.h"
#include "AX_Text_Log.h"
//#include <string>
//#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#include <time.h>
#include <TlHelp32.h>
#else
#include <unistd.h>
#include <linux/unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#ifndef _WIN32
_syscall0(pid_t,gettid);
#endif


AX_Logger::AX_Logger()
{
	_errorFile=NULL;
	_warningFile=NULL;
	_infoFile=NULL;
}
AX_Logger::~AX_Logger()
{
	if(NULL!=_errorFile)
	{
		fclose(_errorFile);
		_errorFile=NULL;
	}
	if(NULL!=_warningFile)
	{
		fclose(_warningFile);
		_warningFile=NULL;
	}
	if(NULL!=_infoFile)
	{
		fclose(_infoFile);
		_infoFile=NULL;
	}
}
bool AX_Logger::levelEnabled(int level)
{
	if(1<=level && 30>=level)
	{
		return _configSet[level-1]._enabled;
	}
	return false;
}

void AX_Logger::refreshLoggerLevel()
{
}
void AX_Logger::trace(int level, const char* str)
{
	if(NULL==str || 1>level || 30< level)
		return;
	_mutex.acquire();
	char preTrace[256]={0};
#ifndef _WIN32
	time_t nowTime=time(NULL);
	struct tm * localTime=localtime(&nowTime);
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
#endif
#ifdef _WIN32
	_snprintf(preTrace,sizeof(preTrace),"[%lu pid=%u]",AX_OS::get_time(),AX_OS::getpid());
#else
	snprintf(preTrace,sizeof(preTrace),"[%d-%02d-%02d %02d:%02d:%02d:%03lu pid=%u tid=%u]",
		localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,
		localTime->tm_hour,localTime->tm_min,localTime->tm_sec,
		tv.tv_usec/1000,getpid(),gettid());
#endif

	if(1<=level && 10>=level)
	{
		if(_errorFile)
		{
			fprintf(_errorFile,"%s %s",preTrace,str);
			fflush(_errorFile);
		}
		if(_warningFile)
		{
			fprintf(_warningFile,"%s %s",preTrace,str);
			fflush(_warningFile);
		}
		if(_infoFile)
		{
			fprintf(_infoFile,"%s %s",preTrace,str);
			fflush(_infoFile);
		}
	}
	else if(11<=level && 20>= level)
	{
		if(_warningFile)
		{
			fprintf(_warningFile,"%s %s",preTrace,str);
			fflush(_warningFile);
		}
		if(_infoFile)
		{
			fprintf(_infoFile,"%s %s",preTrace,str);
			fflush(_infoFile);
		}
	}
	else if(21<=level && 30>=level)
	{
		if(_infoFile)
		{
			fprintf(_infoFile,"%s %s",preTrace,str);
			fflush(_infoFile);
		}
	}
	_mutex.release();
}
#ifdef _WIN32
bool AX_Logger::getProcessName(char* name, int maxLen)
{
	//AX_Func_Mutex func_lock(_mutex);
	if(NULL==name|| 0==maxLen)
		return false;
	PROCESSENTRY32 pInfo = {0};
	pInfo.dwSize = sizeof(PROCESSENTRY32);
	int id = GetCurrentProcessId();
	//_mutex.acquire();
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	Process32First(hSnapShot, &pInfo) ;
	do
	{
		if(id == pInfo.th32ProcessID)
		{
			strcpy(name,pInfo.szExeFile);
			break ;
		}
	}while(Process32Next(hSnapShot, &pInfo) != FALSE);
	//_mutex.release();
	return true;
}
#else
bool AX_Logger::getProcessName(char* name, int maxLen)
{
	if(NULL==name|| 0==maxLen)
		return false;
	_mutex.acquire();
	*name=0;
	pid_t pid=getpid();
	char cmdFile[256];
	snprintf(cmdFile,sizeof(cmdFile),"/proc/%u/cmdline",pid);
	FILE* fp=fopen(cmdFile,"rb");
	bool ret;
	if(fp)
	{
		char* tmpName=NULL;
		size_t tmpNameLen=0;
		if(getline(&tmpName,&tmpNameLen,fp))
		{
			if(0<snprintf(name,maxLen,"%s",tmpName))
				ret=true;
			if(tmpName)
				free(tmpName);
		}
		fclose(fp);
	}
	_mutex.release();
	return ret;
}
#endif
void AX_Logger::initLogFile()
{
	time_t nowTime=time(NULL);
	struct tm *localTime=localtime(&nowTime);
	char processName[256]={0};
	getProcessName(processName,sizeof(processName));
	if(NULL==_errorFile)
	{
		char errorFileName[256]={0};
		if(0!=*processName)
		{
#ifdef _WIN32
			_snprintf(errorFileName,sizeof(errorFileName),"logs\\%ws_error_%d%02d%02d_%u.log",
				(wchar_t*)processName,localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,AX_OS::getpid());
#else
			snprintf(errorFileName,sizeof(errorFileName),"logs/%s_error_%d%02d%02d_%u.log",
				processName,localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,getpid());
#endif
		}
		else
		{
#ifdef _WIN32
			_snprintf(errorFileName,sizeof(errorFileName),"logs/unknow_error_%lu_%d%02d%02d_%u.log",
				time(NULL),localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,AX_OS::getpid());
#else
			snprintf(errorFileName,sizeof(errorFileName),"logs/unknow_error_%lu_%d%02d%02d_%u.log",
				time(NULL),localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,getpid());
#endif
		}
		if(0!=*errorFileName)
		{
			//printf("%s\n",errorFileName);
			_errorFile=fopen(errorFileName,"w+");
		}
	}
	if(NULL==_warningFile)
	{
		char warningFileName[256]={0};
		if(0!=*processName)
		{
#ifdef _WIN32
			_snprintf(warningFileName,sizeof(warningFileName),"logs\\%ws_warning_%d%02d%02d_%u.log",
				(wchar_t*)processName,localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,AX_OS::getpid());
#else
			snprintf(warningFileName,sizeof(warningFileName),"logs/%s_warning_%d%02d%02d_%u.log",
				processName,localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,getpid());
#endif
		}
		else
		{
#ifdef _WIN32
			_snprintf(warningFileName,sizeof(warningFileName),"logs/unknow_warning_%lu_%d%02d%02d_%u.log",
				time(NULL),localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,AX_OS::getpid());
#else
			snprintf(warningFileName,sizeof(warningFileName),"logs/unknow_warning_%lu_%d%02d%02d_%u.log",
				time(NULL),localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,getpid());
#endif
		}
		if(0!=*warningFileName)
		{
			_warningFile=fopen(warningFileName,"w+");
		}
	}
	if(NULL==_infoFile)
	{
		char infoFileName[256]={0};
		if(0!=*processName)
		{
#ifdef _WIN32
			_snprintf(infoFileName,sizeof(infoFileName),"logs\\%ws_info_%d%02d%02d_%u.log",
				(wchar_t*)processName,localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,AX_OS::getpid());
#else
			snprintf(infoFileName,sizeof(infoFileName),"logs/%s_info_%d%02d%02d_%u.log",
				processName,localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,getpid());
#endif
		}
		else
		{
#ifdef _WIN32
			_snprintf(infoFileName,sizeof(infoFileName),"logs/unknow_info_%lu_%d%02d%02d_%u.log",
				time(NULL),localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,AX_OS::getpid());
#else
			snprintf(infoFileName,sizeof(infoFileName),"logs/unknow_info_%lu_%d%02d%02d_%u.log",
				time(NULL),localTime->tm_year+1900,localTime->tm_mon+1,localTime->tm_mday,getpid());
#endif
		}
		if(0!=*infoFileName)
		{
			_infoFile=fopen(infoFileName,"w+");
		}
	}
}

bool AX_Logger::s_inited = false;
AX_Logger& AX_Logger::instance()
{
	//static bool s_inited=false;
	_logger._mutex.acquire();
	if(!s_inited)
	{
		_logger.initLogFile();
		s_inited=true;
	}
	_logger._mutex.release();
	return _logger;
}

AX_Logger  AX_Logger::_logger;
AX_Logger_Config_Unit AX_Logger::_configSet[]=
{
	{1, "Error", true},
	{2, "N/A", false},
	{3, "N/A", false},
	{4, "N/A", false},
	{5, "N/A", false},
	{6, "N/A", false},
	{7, "N/A", false},
	{8, "N/A", false},
	{9, "N/A", false},
	{10, "N/A", false},
	{11, "Warning", true},
	{12, "N/A", false},
	{13, "N/A", false},
	{14, "N/A", false},
	{15, "N/A", false},
	{16, "N/A", false},
	{17, "N/A", false},
	{18, "N/A", false},
	{19, "N/A", false},
	{20, "N/A", false},
	{21, "INFO", true},
	{22, "Detail", false},
	{23, "N/A", false},
	{24, "N/A", false},
	{25, "N/A", false},
	{26, "N/A", false},
	{27, "N/A", false},
	{28, "N/A", false},
	{29, "N/A", false},
	{30, "N/A", false}
};