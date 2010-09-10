#ifndef _AX_SEMAPHORE_H
#define _AX_SEMAPHORE_H
#include "AX_OS.h"

class AX_Semaphore  //在WINDOWS平台下面测试通过
{
public:
	// = Initialization and termination.
	/// Initialize the semaphore, with initial value of "count".
	AX_Semaphore (unsigned int count = 1, // By default make this unlocked.
		int type = 0,
		const char *name = 0,
		void * = 0,
		int max = 0x7fffffff);

	/// Implicitly destroy the semaphore.
	~AX_Semaphore (void);

	/**
	* Explicitly destroy the semaphore.  Note that only one thread
	* should call this method since it doesn't protect against race
	* conditions.
	*/
	int remove (void);

	/// Block the thread until the semaphore count becomes
	/// greater than 0, then decrement it.
	int acquire (void);

	/**
	* Block the thread until the semaphore count becomes greater than 0
	* (at which point it is decremented) or until <tv> times out (in
	* which case -1 is returned and <errno> == <ETIME>).  Note that <tv>
	* is assumed to be in "absolute" rather than "relative" time.  The
	* value of <tv> is updated upon return to show the actual
	* (absolute) acquisition time.
	*
	* @note Solaris threads do not support timed semaphores.
	* Therefore, if you're running on Solaris you might want to
	* consider using the ACE POSIX pthreads implementation instead,
	* which can be enabled by compiling ACE with
	* -DACE_HAS_PTHREADS, rather than -DACE_HAS_STHREADS or
	* -DACE_HAS_POSIX_SEM. */
	int acquire (timeval &tv);

	/**
	* If <tv> == 0 then call <acquire()> directly.  Otherwise, Block
	* the thread until the semaphore count becomes greater than 0
	* (at which point it is decremented) or until <tv> times out (in
	* which case -1 is returned and <errno> == <ETIME>).  Note that
	* <*tv> is assumed to be in "absolute" rather than "relative" time.
	* The value of <*tv> is updated upon return to show the actual
	* (absolute) acquisition time.
	*
	* @note Solaris threads do not support timed semaphores.
	* Therefore, if you're running on Solaris you might want to
	* consider using the ACE POSIX pthreads implementation instead,
	* which can be enabled by compiling ACE with
	* -DACE_HAS_PTHREADS, rather than -DACE_HAS_STHREADS or
	* -DACE_HAS_POSIX_SEM.  */
	int acquire (timeval *tv);

	/**
	* Conditionally decrement the semaphore if count is greater than 0
	* (i.e., won't block).  Returns -1 on failure.  If we "failed"
	* because someone else already had the lock, <errno> is set to
	* <EBUSY>.
	*/
	int tryacquire (void);

	/// Increment the semaphore by 1, potentially unblocking a waiting
	/// thread.
	int release (void);

	/// Increment the semaphore by <release_count>, potentially
	/// unblocking waiting threads.
	int release (unsigned int release_count);

	/**
	* Acquire semaphore ownership.  This calls <acquire> and is only
	* here to make the <ACE_Semaphore> interface consistent with the
	* other synchronization APIs.
	*/
	int acquire_read (void);

	/**
	* Acquire semaphore ownership.  This calls <acquire> and is only
	* here to make the <ACE_Semaphore> interface consistent with the
	* other synchronization APIs.
	*/
	int acquire_write (void);

	/**
	* Conditionally acquire semaphore (i.e., won't block).  This calls
	* <tryacquire> and is only here to make the <ACE_Semaphore>
	* interface consistent with the other synchronization APIs.
	* Returns -1 on failure.  If we "failed" because someone else
	* already had the lock, <errno> is set to <EBUSY>.
	*/
	int tryacquire_read (void);

	/**
	* Conditionally acquire semaphore (i.e., won't block).  This calls
	* <tryacquire> and is only here to make the <ACE_Semaphore>
	* interface consistent with the other synchronization APIs.
	* Returns -1 on failure.  If we "failed" because someone else
	* already had the lock, <errno> is set to <EBUSY>.
	*/
	int tryacquire_write (void);

	/**
	* This is only here to make the <ACE_Semaphore>
	* interface consistent with the other synchronization APIs.
	* Assumes the caller has already acquired the semaphore using one of
	* the above calls, and returns 0 (success) always.
	*/
	int tryacquire_write_upgrade (void);

	/// Dump the state of an object.
	void dump (void) const;

	/// Return the underlying lock.
	const AX_sema_t &lock (void) const;

protected:
	AX_sema_t semaphore_;

	/// Keeps track of whether <remove> has been called yet to avoid
	/// multiple <remove> calls, e.g., explicitly and implicitly in the
	/// destructor.  This flag isn't protected by a lock, so make sure
	/// that you don't have multiple threads simultaneously calling
	/// <remove> on the same object, which is a bad idea anyway...
	int removed_;

private:
	// = Prevent assignment and initialization.
	void operator= (const AX_Semaphore &);
	AX_Semaphore (const AX_Semaphore &);
};
#include "AX_Semaphore.inl"
#endif

