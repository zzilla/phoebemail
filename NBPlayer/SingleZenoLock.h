#ifndef __SINGLEZENOLOCK_INC__
#define __SINGLEZENOLOCK_INC__

#include "AX_Mutex.h"

class CSingleZenoLock
{
public:
	CSingleZenoLock(AX_Mutex& mutex, bool bInitLock = true)
	{
		m_pMutex = &mutex;
		m_bLocked = false;
		if (bInitLock)
		{
			acquire();
		}
	}
	~CSingleZenoLock(void)
	{
		release();
	}

	void acquire (void)
	{
		if (m_bLocked == false)
		{
			m_bLocked = true;
			m_pMutex->acquire();
		}
	}
	void release (void)
	{
		if (m_bLocked)
		{
			m_bLocked = false;
			m_pMutex->release();
		}
	}

protected:
	AX_Mutex* m_pMutex;
	bool m_bLocked;
};



#endif //