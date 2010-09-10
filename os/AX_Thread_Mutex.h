
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Thread_Mutex.h
* 文件标识：
* 摘　　要：AX线程互斥类 .h文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月20日
* 修订记录：创建
*/

#ifndef AX_THREAD_MUTEX_INCLUDE
#define AX_THREAD_MUTEX_INCLUDE

#include "AX_OS.h"

class AX_Thread_Mutex
{
public:
	/// Initialize the mutex.
	AX_Thread_Mutex ();

	/// Implicitly destroy the mutex.
	~AX_Thread_Mutex (void);

	/// Explicitly destroy the mutex.
	int remove (void);

	int acquire (void);

	int acquire (timeval &tv);

	int acquire (timeval *tv);

	int tryacquire (void);

	int release (void);

	int acquire_read (void);
	int acquire_write (void);
	int tryacquire_read (void);
	int tryacquire_write (void);

//protected:
	AX_Mutex_t lock_;
	int removed_;

private:
	// Prevent assignment and initialization.
	void operator= (const AX_Thread_Mutex &);
	AX_Thread_Mutex (const AX_Thread_Mutex &);

};

#include "AX_Thread_Mutex.inl"

#endif

