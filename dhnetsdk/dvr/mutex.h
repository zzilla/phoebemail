#ifndef MUTEX_H
#define MUTEX_H

#ifdef WIN32
#include <windows.h>

/*
class CCriticalSection  
{
public:
    CCriticalSection() 
	{ 
		InitializeCriticalSection(&m_sec);
	} 
    ~CCriticalSection() { DeleteCriticalSection(&m_sec); }
public:
    void Lock() { EnterCriticalSection(&m_sec); }
    void Unlock() { LeaveCriticalSection(&m_sec); }
protected:
    CRITICAL_SECTION m_sec;
};
*/

class CCSLock
{
public:
	CCSLock(CRITICAL_SECTION& cs):m_cs(cs)
	{
		EnterCriticalSection(&m_cs);
	}

	~CCSLock()
	{
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION& m_cs;
};

#else

#include <pthread.h>

#endif

class DEVMutex
{
public:
	DEVMutex();
	~DEVMutex();

	int	Lock();
	int	UnLock();

private:

#ifdef WIN32
	CRITICAL_SECTION m_critclSection;
#else
	pthread_mutex_t m_mutex;
#endif
};

#endif

