#ifndef _AX_THREAD_GUARD
#define _AX_THREAD_GUARD

#include "AX_OS.h"

template <class AX_LOCK>
class AX_Guard
{
public:

	// = Initialization and termination methods.
	AX_Guard (AX_LOCK &l);

	/// Implicitly and automatically acquire (or try to acquire) the
	/// lock.  If @a block is non-0 then <acquire> the <ACE_LOCK>, else
	/// <tryacquire> it.
	AX_Guard (AX_LOCK &l, int block);

	/// Initialise the guard without implicitly acquiring the lock. The
	/// <become_owner> parameter indicates whether the guard should release
	/// the lock implicitly on destruction. The <block> parameter is
	/// ignored and is used here to disambiguate with the preceding
	/// constructor.
	AX_Guard (AX_LOCK &l, int block, int become_owner);

	/// Implicitly release the lock.
	~AX_Guard (void);

	// = Lock accessors.

	/// Explicitly acquire the lock.
	int acquire (void);

	/// Conditionally acquire the lock (i.e., won't block).
	int tryacquire (void);

	/// Explicitly release the lock, but only if it is held!
	int release (void);

	/// Relinquish ownership of the lock so that it is not released
	/// implicitly in the destructor.
	void disown (void);

	// = Utility methods.
	/// 1 if locked, 0 if couldn't acquire the lock
	/// (errno will contain the reason for this).
	int locked (void) const;

	/// Explicitly remove the lock.
	int remove (void);

	/// Dump the state of an object.
	void dump (void) const;

	// ACE_ALLOC_HOOK_DECLARE;
	// Declare the dynamic allocation hooks.

protected:

	/// Helper, meant for subclass only.
	AX_Guard (AX_LOCK *lock): lock_ (lock) {}

	/// Pointer to the ACE_LOCK we're guarding.
	AX_LOCK *lock_;

	/// Keeps track of whether we acquired the lock or failed.
	int owner_;

private:
	// = Prevent assignment and initialization.
	void operator= (const AX_Guard<AX_LOCK> &);
	 AX_Guard (const AX_Guard<AX_LOCK> &);
};

/**
* @class ACE_Write_Guard
*
* @brief This class is similar to class ACE_Guard, though it
* acquires/releases a write lock automatically (naturally, the
* <ACE_LOCK> it is instantiated with must support the appropriate
* API).
*/
template <class AX_LOCK>
class AX_Write_Guard : public AX_Guard<AX_LOCK>
{
public:
	// = Initialization method.

	/// Implicitly and automatically acquire a write lock.
	AX_Write_Guard (AX_LOCK &m);

	/// Implicitly and automatically acquire (or try to acquire) a write
	/// lock.
	AX_Write_Guard (AX_LOCK &m, int block);

	// = Lock accessors.

	/// Explicitly acquire the write lock.
	int acquire_write (void);

	/// Explicitly acquire the write lock.
	int acquire (void);

	/// Conditionally acquire the write lock (i.e., won't block).
	int tryacquire_write (void);

	/// Conditionally acquire the write lock (i.e., won't block).
	int tryacquire (void);

	// = Utility methods.

	/// Dump the state of an object.
	void dump (void) const;

	// ACE_ALLOC_HOOK_DECLARE;
	// Declare the dynamic allocation hooks.
};
template <class AX_LOCK>
class AX_Read_Guard : public AX_Guard<AX_LOCK>
{
public:
	// = Initialization methods.

	/// Implicitly and automatically acquire a read lock.
	AX_Read_Guard (AX_LOCK& m);

	/// Implicitly and automatically acquire (or try to acquire) a read
	/// lock.
	AX_Read_Guard (AX_LOCK &m, int block);

	// = Lock accessors.

	/// Explicitly acquire the read lock.
	int acquire_read (void);

	/// Explicitly acquire the read lock.
	int acquire (void);

	/// Conditionally acquire the read lock (i.e., won't block).
	int tryacquire_read (void);

	/// Conditionally acquire the read lock (i.e., won't block).
	int tryacquire (void);

	// = Utility methods.

	/// Dump the state of an object.
	void dump (void) const;

	// ACE_ALLOC_HOOK_DECLARE;
	// Declare the dynamic allocation hooks.
};
#include "AX_Thread_Guard.inl"
#endif
