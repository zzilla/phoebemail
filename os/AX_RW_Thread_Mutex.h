
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_RW_Thread_Mutex.h
* 文件标识：
* 摘　　要：AX读写线程锁 .h文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月20日
* 修订记录：创建
*/

#ifndef AX_RW_THREAD_MUTEX_INCLUDE
#define AX_RW_THREAD_MUTEX_INCLUDE

#include "AX_RW_Mutex.h"

class AX_RW_Thread_Mutex : public AX_RW_Mutex
{
	AX_RW_Thread_Mutex (const char *name = 0,	void *arg = 0);

	/// Default dtor.
	~AX_RW_Thread_Mutex (void);
};
#include "AX_RW_Thread_Mutex.inl"

#endif

