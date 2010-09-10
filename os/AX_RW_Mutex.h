
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_RW_Mutex.h
* 文件标识：
* 摘　　要：AX读写锁 .h文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/

#ifndef AX_RW_MUTEX_INCLUDE
#define AX_RW_MUTEX_INCLUDE

#include "AX_OS.h"

//#define AX_HAS_PROCESS_MUTEX

class AX_RW_Mutex
{
public:
	/// Initialize a readers/writer lock.
	AX_RW_Mutex (int type = AX_USYNC_THREAD,
		const char *name = 0,
		void *arg = 0);

	/// Implicitly destroy a readers/writer lock
	~AX_RW_Mutex (void);

	int remove (void);

	/// Acquire a read lock, but block if a writer hold the lock.
	int acquire_read (void);

	/// Acquire a write lock, but block if any readers or a
	/// writer hold the lock.
	int acquire_write (void);

	/**
	* Conditionally acquire a read lock (i.e., won't block).  Returns
	* -1 on failure.  If we "failed" because someone else already had
	* the lock, <errno> is set to <EBUSY>.
	*/
	int tryacquire_read (void);

	/// Conditionally acquire a write lock (i.e., won't block).
	int tryacquire_write (void);

	/**
	* Conditionally upgrade a read lock to a write lock.  This only
	* works if there are no other readers present, in which case the
	* method returns 0.  Otherwise, the method returns -1 and sets
	* <errno> to <EBUSY>.  Note that the caller of this method *must*
	* already possess this lock as a read lock (but this condition is
	* not checked by the current implementation).
	*/
//	int tryacquire_write_upgrade (void);

	/**
	* Note, for interface uniformity with other synchronization
	* wrappers we include the <acquire> method.  This is implemented as
	* a write-lock to safe...
	*/
	int acquire (void);

	/**
	* Note, for interface uniformity with other synchronization
	* wrappers we include the <tryacquire> method.  This is implemented
	* as a write-lock to be safe...  Returns -1 on failure.  If we
	* "failed" because someone else already had the lock, <errno> is
	* set to <EBUSY>.
	*/
	int tryacquire (void);

	/// Unlock a readers/writer lock.
	int release (void);

protected:
	/// This lock resides in shared memory.
	AX_rwlock_t *process_lock_;

	/// Readers/writer lock.
	AX_rwlock_t lock_;

	/// Keeps track of whether <remove> has been called yet to avoid
	/// multiple <remove> calls, e.g., explicitly and implicitly in the
	/// destructor.  This flag isn't protected by a lock, so make sure
	/// that you don't have multiple threads simultaneously calling
	/// <remove> on the same object, which is a bad idea anyway...
	int removed_;

private:
	// = Prevent assignment and initialization.
	void operator= (const AX_RW_Mutex &);
	AX_RW_Mutex (const AX_RW_Mutex &);
};

#include "AX_RW_Mutex.inl"

#endif

