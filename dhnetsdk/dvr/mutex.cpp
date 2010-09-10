#include "mutex.h"

DEVMutex::DEVMutex()
{
#ifndef WIN32
	pthread_mutexattr_t attr;
//	attr.__mutexkind = PTHREAD_MUTEX_RECURSIVE_NP;
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex, &attr);
#else
	InitializeCriticalSection(&m_critclSection);
#endif
}

DEVMutex::~DEVMutex()
{
#ifndef WIN32
	pthread_mutex_destroy(&m_mutex);
#else
	DeleteCriticalSection(&m_critclSection);
#endif
}

int DEVMutex::Lock()
{
#ifdef WIN32
	EnterCriticalSection(&m_critclSection);
	return 0;
#else
	return pthread_mutex_lock(&m_mutex);
#endif
}

int DEVMutex::UnLock()
{
#ifdef WIN32
	LeaveCriticalSection(&m_critclSection);
	return 0;
#else
	return pthread_mutex_unlock(&m_mutex);
#endif
}

