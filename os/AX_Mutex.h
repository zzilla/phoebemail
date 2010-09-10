
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Mutex.h
* 文件标识：
* 摘　　要：AX互斥类 .h文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/

#ifndef AX_MUTEX_INCLUDE
#define AX_MUTEX_INCLUDE

#include "AX_OS.h"

//#define AX_HAS_PROCESS_MUTEX

class AX_Mutex
{
public:
	/// Initialize the mutex.
	AX_Mutex (int type = AX_USYNC_THREAD);

	/// Implicitly destroy the mutex.
	~AX_Mutex (void);

	/// Explicitly destroy the mutex.
	/**
	* @note Only one thread should call this method since it doesn't
	*        protect against race conditions.
	*/
	int remove (void);

	/// Acquire lock ownership (wait on queue if necessary).
	int acquire (void);

	/// Block the thread until the mutex is acquired or @a tv times out,
	/// in which case -1 is returned and @c errno == @c ETIME.
	/**
	* @note @a tv is assumed  to be in "absolute" rather than
	* "     relative" time.  The value of @a tv is updated upon return
	*       to show the actual(absolute) acquisition time.
	*/
	int acquire (timeval &tv);

	/// Block the thread until the mutex is acquired or @a *tv times
	/// out, in which case -1 is returned and @c errno == @c ETIME.
	/**
	* If @a tv == 0 then call @c acquire() directly.  Otherwise, block
	* the thread until the mutex is acquired or @a tv times out, in
	* which case -1 is returned and @c errno == @c ETIME.
	*
	* @note @a *tv is assumed to be in "absolute" rather than
	*       "relative" time.  The value of @a *tv is updated upon
	*       return to show the actual (absolute) acquisition time.
	*/
	int acquire (timeval *tv);

	/// Conditionally acquire lock (i.e., don't wait on queue).
	/**
	* @return -1 on failure.  If we "failed" because someone
	*         else already had the lock, @c errno is set to @c EBUSY.
	*/
	int tryacquire (void);

	/// Release lock and unblock a thread at head of queue.
	int release (void);


	/// 以下接口仅为兼容性考虑
	int acquire_read (void);
	int acquire_write (void);
	int tryacquire_read (void);
	int tryacquire_write (void);
//	int tryacquire_write_upgrade (void);

protected:
	/// This lock resides in shared memory.
	AX_Mutex_t *process_lock_;

	/// Mutex type supported by the OS.
	AX_Mutex_t lock_;

	/// Keeps track of whether @c remove has been called yet to avoid
	/// multiple @c remove calls, e.g., explicitly and implicitly in the
	/// destructor.  This flag isn't protected by a lock, so make sure
	/// that you don't have multiple threads simultaneously calling
	/// @c remove on the same object, which is a bad idea anyway.
	int removed_;

private:
	// Prevent assignment and initialization.
	void operator= (const AX_Mutex &);
	AX_Mutex (const AX_Mutex &);

};

#include "AX_Mutex.inl"

class AX_Lock
{
public:
	AX_Lock(AX_Mutex& mutex) : m_mutex(mutex){m_mutex.acquire();};
	~AX_Lock(){m_mutex.release();};
private:
	AX_Mutex& m_mutex;
};

#define AX_LOCK(mutex) AX_Lock lock_(mutex);

#endif

