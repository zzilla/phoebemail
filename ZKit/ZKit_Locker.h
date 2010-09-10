#ifndef _ZKit_Locker_h_
#define _ZKit_Locker_h_

#include "ZKit_IMutex.h"

BEGIN_ZKIT
class Locker
{
public:
	Locker(const IMutex& mutex) : m_mutex(&mutex)
	{
		m_mutex->Lock();
	};

	~Locker()
	{
		m_mutex->Unlock();
	};

private:
	const IMutex* m_mutex;
};

#define LOCK(mutex) Locker locker(mutex);

END_ZKIT
#endif // _ZKit_Lock_h_
