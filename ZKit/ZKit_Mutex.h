#ifndef _ZKit_Mutex_h_
#define _ZKit_Mutex_h_

#include "ZKit_IMutex.h"
#include "AX_OS.h"
#include "AX_Mutex.h"

BEGIN_ZKIT

//在已经有AX_Mutex的情况下, 仍然编写这个类, 是为了弥补AX_Mutex的一点缺点:
//AX_Mutex的接口都是非const函数, 因此如果在一个类的const成员函数中使用AX_Mutex将无法通过编译
//而这种情况很常见, 例如Getter函数通常是const的
//另外这个类也提供简化的接口, 满足大部分需求
//by qguo. 2010.04.29
class Mutex : public IMutex
{
public:
	Mutex();
	~Mutex();

	virtual void Lock() const;
	virtual void Unlock() const;
private:
	mutable AX_Mutex m_mutex;
};

END_ZKIT

#endif // _ZKit_Mutex_h_
