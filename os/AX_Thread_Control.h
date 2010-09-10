#ifndef _AX_THREAD_CONTROL_H
#define _AX_THREAD_CONTROL_H

#include "AX_OS.h"
#include "AX_Thread_Manager.h"
class AX_Thread_Manager;

class AX_Thread_Control
{
public:
	/// Initialize the thread control object.  If <insert> != 0, then
	/// register the thread with the Thread_Manager.
	AX_Thread_Control (AX_Thread_Manager *tm = 0,
		int insert = 0);

	/// Remove the thread from its associated <Thread_Manager> and exit
	/// the thread if <do_thr_exit> is enabled.
	~AX_Thread_Control (void);

	/// Remove this thread from its associated ACE_Thread_Manager and exit
	/// the thread if @a do_thr_exit is enabled.
	AX_thr_func_return exit (AX_thr_func_return status,
		int do_thr_exit);

	/// Store the <Thread_Manager> and use it to register ourselves for
	/// correct shutdown.
	int insert (AX_Thread_Manager *tm, int insert = 0);

	/// Returns the current <Thread_Manager>.
	AX_Thread_Manager *thr_mgr (void);

	/// Atomically set a new <Thread_Manager> and return the old
	/// <Thread_Manager>.
	AX_Thread_Manager *thr_mgr (AX_Thread_Manager *);

	/// Set the exit status (and return existing status).
	AX_thr_func_return status (AX_thr_func_return status);

	/// Get the current exit status.
	AX_thr_func_return status (void);

	/// Dump the state of an object.
	void dump (void) const;

private:
	/// Pointer to the thread manager for this block of code.
	AX_Thread_Manager *tm_;

	/// Keeps track of the exit status for the thread.
	AX_thr_func_return status_;

};

#include "AX_Thread_Control.inl"

#endif
