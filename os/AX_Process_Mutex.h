#ifndef _AX_PROCESS_MUTEX
#define _AX_PROCESS_MUTEX
#include "AX_OS.h"
#include "AX_Mutex.h"

class  AX_Process_Mutex
{
public:
	/**
	* Create a Process_Mutex, passing in the optional @c name.
	*
	* @param name optional, null-terminated string containing the name of
	* the object. Multiple users of the same @c ACE_Process_Mutex must use
	* the same name to access the same object.  If not specified, a name
	* is generated.
	* @param arg optional, attributes to be used to initialize the mutex.
	* If using @c ACE_SV_Semaphore_Complex as the underlying mechanism,
	* this argument is ignored.
	* @param mode optional, the protection mode for either the backing store
	* file (for ACE_Mutex use) or the ACE_SV_Semaphore_Complex that's created.
	*/
	AX_Process_Mutex (char *name = NULL,
		void *arg = 0,
		mode_t mode = 0 );



	~AX_Process_Mutex (void);

	/**
	* Explicitly destroy the mutex.  Note that only one thread should
	* call this method since it doesn't protect against race
	* conditions.
	*
	* @return 0 on success; -1 on failure.
	*/
	int remove (void);

	/**
	* Acquire lock ownership (wait on queue if necessary).
	*
	* @return 0 on success; -1 on failure.
	*/
	int acquire (void);

	/**
	* Acquire lock ownership, but timeout if lock if hasn't been
	* acquired by given time.
	*
	* @param tv the absolute time until which the caller is willing to
	* wait to acquire the lock.
	*
	* @return 0 on success; -1 on failure.
	*/
	int acquire (timeval &tv);

	/**
	* Conditionally acquire lock (i.e., don't wait on queue).
	*
	* @return 0 on success; -1 on failure.  If the lock could not be acquired
	* because someone else already had the lock, @c errno is set to @c EBUSY.
	*/
	int tryacquire (void);

	/// Release lock and unblock a thread at head of queue.
	int release (void);

	/// Acquire lock ownership (wait on queue if necessary).
	int acquire_read (void);

	/// Acquire lock ownership (wait on queue if necessary).
	int acquire_write (void);

	/**
	* Conditionally acquire a lock (i.e., won't block).  Returns -1 on
	* failure.  If we "failed" because someone else already had the
	* lock, <errno> is set to <EBUSY>.
	*/
	int tryacquire_read (void);

	/**
	* Conditionally acquire a lock (i.e., won't block).  Returns -1 on
	* failure.  If we "failed" because someone else already had the
	* lock, <errno> is set to <EBUSY>.
	*/
	int tryacquire_write (void);


	const AX_Mutex &lock (void) const;

	/// Dump the state of an object.
	void dump (void) const;


private:
	/// If the user does not provide a name we generate a unique name in
	/// this buffer.
	char name_[256];

	/// Create and return the unique name.
	const char *unique_name (void);

	AX_Mutex lock_;
};

#include "AX_Process_Mutex.inl"
#endif
