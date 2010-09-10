#ifndef CONDITION_THREAD_MUTEX
#define CONDITION_THREAD_MUTEX

#include "AX_OS.h"
#include "AX_Mutex.h"
#include "AX_Thread_Mutex.h"
#ifndef _WIN32
#include <errno.h>
#endif
class AX_Condition_Thread_Mutex;
class AX_Condition_Attributes
{
public:
	/// Constructor
	AX_Condition_Attributes (int type);

	/// Destructor
	~AX_Condition_Attributes (void);

private:
	friend class AX_Condition_Thread_Mutex;

	/// The attributes
	AX_condattr_t attributes_;

private:
	// = Prevent assignment and initialization.
	void operator= (const AX_Condition_Attributes &);
	AX_Condition_Attributes (const AX_Condition_Attributes &);
};

/**
* @class ACE_Condition_Thread_Mutex
*
* @brief ACE_Condition variable wrapper written using ACE_Mutexes This
* allows threads to block until shared data changes state.
* A condition variable enables threads to atomically block and
* test the condition under the protection of a mutual exclu-
* sion lock (mutex) until the condition is satisfied.  That is,
* the mutex must have been held by the thread before calling
* wait or signal on the condition.  If the condition is false,
* a thread blocks on a condition variable and atomically
* releases the mutex that is waiting for the condition to
* change.  If another thread changes the condition, it may wake
* up waiting threads by signaling the associated condition
* variable.  The waiting threads, upon awakening, reacquire the
* mutex and re-evaluate the condition.
*
* This should be an instantiation of ACE_Condition but problems
* with compilers precludes this...
*/
class AX_Condition_Thread_Mutex   
{
public:
	/// Initialize the condition variable.
	AX_Condition_Thread_Mutex (const AX_Thread_Mutex &m,const char *name = 0,void *arg = 0);

	/// Initialize the condition variable.
	AX_Condition_Thread_Mutex (AX_Thread_Mutex &m,
		AX_Condition_Attributes &attributes,
		const char *name = 0,
		void *arg = 0);

	/// Implicitly destroy the condition variable.
	~AX_Condition_Thread_Mutex (void);

	/**
	* Explicitly destroy the condition variable.  Note that only one
	* thread should call this method since it doesn't protect against
	* race conditions.
	*/
	int remove (void);

	/**
	* Block on condition, or until absolute time-of-day has passed.  If
	* abstime == 0 use "blocking" <wait> semantics.  Else, if <abstime>
	* != 0 and the call times out before the condition is signaled
	* <wait> returns -1 and sets errno to ETIME.
	*/
	int wait (const timeval *abstime);

	/// Block on condition.
	int wait (void);

	/**
	* Block on condition or until absolute time-of-day has passed.  If
	* abstime == 0 use "blocking" wait() semantics on the <mutex>
	* passed as a parameter (this is useful if you need to store the
	* <Condition> in shared memory).  Else, if <abstime> != 0 and the
	* call times out before the condition is signaled <wait> returns -1
	* and sets errno to ETIME.
	*/
	int wait (AX_Thread_Mutex &mutex, const timeval *abstime = 0);

	/// Signal one waiting thread.
	int signal (void);

	/// Signal *all* waiting threads.
	int broadcast (void);

	/// Returns a reference to the underlying mutex;
	AX_Thread_Mutex& mutex (void);

	/// Dump the state of an object.
	void dump (void) const;


protected:
	/// Condition variable.
	AX_cond_t cond_;

	/// Reference to mutex lock.
	AX_Thread_Mutex &mutex_;

	/// Keeps track of whether <remove> has been called yet to avoid
	/// multiple <remove> calls, e.g., explicitly and implicitly in the
	/// destructor.  This flag isn't protected by a lock, so make sure
	/// that you don't have multiple threads simultaneously calling
	/// <remove> on the same object, which is a bad idea anyway...
	int removed_;

private:
	// = Prevent assignment and initialization.
	void operator= (const AX_Condition_Thread_Mutex &);
	AX_Condition_Thread_Mutex (const AX_Condition_Thread_Mutex &);
};

#include "AX_Condition_Thread_Mutex.inl"

#endif
