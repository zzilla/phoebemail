#ifndef OS_INDEPENDENT_H
#define OS_INDEPENDENT_H

#ifdef WIN32

#include <windows.h>

#else

#include <semaphore.h>
#include <pthread.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "atomic.h"

//	WINDOWS中的宏定义
#define DWORD			unsigned long
#define BOOL			int
#define TRUE			1
#define FALSE			0
#define LONG			long
#define WORD			unsigned short
#define BYTE			unsigned char
#define UINT			unsigned int
#define HANDLE			int
#define HWND			void*
#define INFINITE		0xFFFFFFFF
#define HDC				void*
#define LPVOID			void*
#define LPDWORD			DWORD*
#define LPBYTE			BYTE*
#define STATUS_WAIT_0   ((DWORD)0x00000000L)
#define WAIT_OBJECT_0   ((STATUS_WAIT_0) + 0)
#define SOCKET			unsigned int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR    (-1)
#define SD_RECEIVE      SHUT_RD
#define SD_SEND         SHUT_WR
#define SD_BOTH         SHUT_RDWR
#ifndef NULL
#define NULL			0
#endif
#define closesocket(S)	close(S)
#define __stdcall
#define WINAPI
#define Sleep(x)		usleep(x*1000)


typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);


#ifndef RECT
typedef struct  tagRECT
{
	LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;
#endif


#endif

//////////////////////////////////////////////////////////////////////////

class COSEvent
{
public:
	COSEvent();
    virtual ~COSEvent();

#ifdef WIN32
	HANDLE		m_hEvent;
#else
	sem_t		m_hEvent;
#endif
	BOOL		m_bCreate;
	BOOL		m_bManualReset;
};

class COSThread
{
public:
	COSThread();
	virtual ~COSThread();

#ifdef WIN32
	HANDLE		m_hThread;
#else
	pthread_t	m_hThread;
#endif
	BOOL		m_bCreate;
};


//////////////////////////////////////////////////////////////////////////
//	定义WINDOWS与LINUX通用的库函数，避免代码太多的#ifdef...#else...#endif，影响可读性。

#define OS_EVENT		COSEvent
#define OS_THREAD		COSThread


#ifdef WIN32

#define	LN_LIFECOUNT	LONG

#else

#define LN_LIFECOUNT	atomic_t

#endif


//	库函数，返回值：>=0：成功；<0：失败

int			CreateEventEx(OS_EVENT &event, BOOL bManualReset, BOOL bInitialState);

void		CloseEventEx(OS_EVENT &event);

BOOL		SetEventEx(OS_EVENT &event);

BOOL		ResetEventEx(OS_EVENT &event);

DWORD		WaitForSingleObjectEx(OS_EVENT &event, DWORD dwMilliseconds);

DWORD		WaitForSingleObjectEx(OS_THREAD &thread, DWORD dwMilliseconds);


int			CreateThreadEx(OS_THREAD &thread, DWORD dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);

void		CloseThreadEx(OS_THREAD &thread);

BOOL		TerminateThreadEx(OS_THREAD &thread, DWORD dwExitCode);


DWORD		GetTickCountEx();


LONG		InterlockedIncrementEx(LN_LIFECOUNT *lpAddend);

LONG		InterlockedDecrementEx(LN_LIFECOUNT *lpAddend);

void		InterlockedSetEx(LN_LIFECOUNT *lpAddend, int value);


//	WINDOWS中有，LINUX中没有的库函数
#ifndef WIN32

char*		_strupr(char *string);
char*		_strlwr(char *string);
int			_stricmp(const char *string1, const char *string2);
char*		_strrev(char *string);
char*		itoa(int value, char *string, int radix);

#endif

//////////////////////////////////////////////////////////////////////////

#define GLOBAL_SINGLETON_DECLARE(classname)		\
static classname * instance();

#define GLOBAL_SINGLETON_IMPLEMENT(classname)	\
classname * classname::instance()				\
{												\
	static classname * _instance = 0;			\
	if(0 == _instance)							\
	{											\
		_instance = new classname;				\
	}											\
	return _instance;							\
}

#define GLOBAL_SINGLETON_DELETE

//////////////////////////////////////////////////////////////////////////
/*
//	大小端转换
short		__ConvertBigLittle(short sNum);
int			__ConvertBigLittle(int nNum);
long		__ConvertBigLittle(long lNum);
void		__ConvertBigLittle(char *pSource, char *pDestination, int size);
*/
//////////////////////////////////////////////////////////////////////////


#endif	//	OS_INDEPENDENT_H



