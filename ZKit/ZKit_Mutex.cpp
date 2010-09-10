
#include "AX_OS.h"
#include "ZKit_Mutex.h"

BEGIN_ZKIT

Mutex::Mutex()
{
	//AX_OS::thread_mutex_init(&m_mutex, NULL);
}

Mutex::~Mutex()
{
	//AX_OS::thread_mutex_destroy(&m_mutex);
}

void Mutex::Lock() const
{
	//AX_OS::thread_mutex_lock(&m_mutex);
	m_mutex.acquire();
}

void Mutex::Unlock() const
{
	//AX_OS::thread_mutex_unlock(&m_mutex);
	m_mutex.release();
}

END_ZKIT
