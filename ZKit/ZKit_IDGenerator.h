#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

#include "winsock2i.h"

#include "platform.h"

#ifndef WIN32
#include "AX_Atomic.h"
#endif

class IDGenerator
{
public:
	IDGenerator(void)
	{
#ifdef WIN32
		id = 0;
#else
		id.counter = 0;
#endif
	}

	~IDGenerator(void)
	{

	}

public:
	uint32 getId(void)
	{
#ifdef WIN32
		return InterlockedIncrement((long*)&id);
#else
		return atomic_inc_return(&id);
#endif
	}

private:

#ifdef WIN32
	uint32	 id;
#else
	atomic_t id;
#endif

};

#endif
