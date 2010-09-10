#ifndef _ZKit_IMutex_h_
#define _ZKit_IMutex_h_
#include "ZKit_Config.h"

BEGIN_ZKIT
//interface for mutex.
class IMutex
{
public:
	virtual ~IMutex() {}
	virtual void Lock() const = 0;
	virtual void Unlock() const = 0;
};

END_ZKIT

#endif // _ZKit_IMutex_h_