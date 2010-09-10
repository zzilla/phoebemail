#ifndef _AX_EVENT_H
#define _AX_EVENT_H
#include "AX_OS.h"

class AX_Event    // ÔÚWINDOWS²âÊÔÍ¨¹ý
{
public:
	/// Constructor that creates event.
	AX_Event (int manual_reset = 0,
		int initial_state = 0,
		int type = 0,
		const char *name = 0,
		void *arg = 0);

	/// Implicitly destroy the event variable.
	~AX_Event (void);

	/**
	* Explicitly destroy the event variable.  Note that only one thread
	* should call this method since it doesn't protect against race
	* conditions.
	*/
	int remove (void);

	/// Underlying handle to event.
	AX_event_t handle (void) const;

	/**
	* Set the underlying handle to event. Note that this method assumes
	* ownership of the <handle> and will close it down in <remove>.  If
	* you want the <handle> to stay open when <remove> is called make
	* sure to call <dup> on the <handle> before closing it.  You are
	* responsible for the closing the existing <handle> before
	* overwriting it.
	*/
	void handle (AX_event_t new_handle);

	/**
	* if MANUAL reset
	*    sleep till the event becomes signaled
	*    event remains signaled after wait() completes.
	* else AUTO reset
	*    sleep till the event becomes signaled
	*    event resets wait() completes.
	*/
	int wait (void);

	/// Same as wait() above, but this one can be timed
	/// @a abstime is absolute time-of-day if if @a use_absolute_time
	/// is non-0, else it is relative time.
	int wait (const timeval *abstime,
		int use_absolute_time = 1);

	/**
	* if MANUAL reset
	*    wake up all waiting threads
	*    set to signaled state
	* else AUTO reset
	*    if no thread is waiting, set to signaled state
	*    if thread(s) are waiting, wake up one waiting thread and
	*    reset event
	*/
	int signal (void);

	/**
	* if MANUAL reset
	*    wakeup all waiting threads and
	*    reset event
	* else AUTO reset
	*    wakeup one waiting thread (if present) and
	*    reset event
	*/
	int pulse (void);

	/// Set to nonsignaled state.
	int reset (void);

	/// Dump the state of an object.
	void dump (void) const;

protected:
	/// The underlying handle.
	AX_event_t handle_;

	/// Keeps track of whether <remove> has been called yet to avoid
	/// multiple <remove> calls, e.g., explicitly and implicitly in the
	/// destructor.  This flag isn't protected by a lock, so make sure
	/// that you don't have multiple threads simultaneously calling
	/// <remove> on the same object, which is a bad idea anyway...
	int removed_;

private:
	// = Prevent copying.
	AX_Event (const AX_Event& event);
	const AX_Event &operator= (const AX_Event &rhs);
};
#endif
