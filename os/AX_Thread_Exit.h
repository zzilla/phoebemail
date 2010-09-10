#ifndef _AX_THREAD_EXIT_H
#define _AX_THREAD_EXIT_H

#include "AX_OS.h"
//#include "AX_Thread_Manager.h"
//#include "AX_Thread_Control.h"

class  AX_Thread_Exit
{
public:
	/// Capture the Thread that will be cleaned up automatically.
	AX_Thread_Exit (void);

	/// Set the <ACE_Thread_Manager>.
	//void thr_mgr (AX_Thread_Manager *tm);

	/// Destructor calls the thread-specific exit hooks when a thread
	/// exits.
	~AX_Thread_Exit (void);

	/// Singleton access point.
	//static AX_Thread_Exit *instance (void);

	/// Cleanup method, used by the <ACE_Object_Manager> to destroy the
	/// singleton.
	static void cleanup (void *instance);

	int register_to_exit(AX_Exit_Routine );
	void remove(void);
	int size(void);
	int run(void);

private:
	/// Automatically add/remove the thread from the
	//AX_Thread_Control thread_control_;

	/**
	* Used to detect whether we should create a new instance (or not)
	* within the instance method -- we don't trust the instance_ ptr
	* because the destructor may have run (if ACE::fini() was called).
	* See bug #526.
	* We don't follow the singleton pattern due to dependency issues.
	*/
	static unsigned int is_constructed_;
//#ifdef _WIN32
	stack<AX_Exit_Routine> Exit_Stack;
//#else
	int size_;
//#endif
};

#endif
