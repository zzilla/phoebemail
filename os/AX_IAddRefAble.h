/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_IAddRefAble.h
* 文件标识：
* 摘要：计数指针，适用于多线程指针安全传递。
*
* 历史：创建，2006年7月26日，李明江(li_mingjiang@dahuatech.com)
*		整理，2008年5月21日，林坚彦(linjy@anxiesoft.cn)
*		重写，2008年6月11日，林坚彦(linjy@anxiesoft.cn)
*
*/
//////////////////////////////////////////////////////////////////////

#if !defined AX_IADDREFABLD_INCLUDE
#define AX_IADDREFABLD_INCLUDE

#include "AX_Mutex.h"

class AX_IAddRefAble
{
public:
	AX_IAddRefAble();
	virtual ~AX_IAddRefAble();

	virtual int addRef(void);
	virtual int release(void);

protected:
	int	_ref;
	AX_Mutex _mutex;
};

/* linux下的atomic_t系列函数似乎不能保证线程同步，重新实现
#if defined(WIN32)
	#include "winsock2i.h"
#else
	#include <pthread.h> 

	#ifdef LINUX
		#include "AX_Atomic.h"
	#else
		#include <asm/asm-armnommu/atomic.h>
	#endif
#endif

class AX_IAddRefAble
{
public:
	AX_IAddRefAble();
	virtual ~AX_IAddRefAble();

	virtual int addRef(void);
	virtual int release(void);

protected:

#ifdef WIN32
	long _ref;
#else
	atomic_t _ref;
#endif
};
*/

#include "AX_IAddRefAble.inl"

#endif // !defined AX_IADDREFABLD_INCLUDE
