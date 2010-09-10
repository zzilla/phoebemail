#ifndef _AX_THREAD_HOOK
#define _AX_THREAD_HOOK

#include "AX_OS.h"
/**
* @class ACE_Thread_Hook
*
* @brief This class makes it possible to provide user-defined "start"
* hooks that are called before the thread entry point function
* is invoked.
*/

class AX_Thread_Hook
{

public:
	AX_Thread_Hook();

	/// Destructor.
	virtual ~AX_Thread_Hook (void);

	/**
	* This method can be overridden in a subclass to customize this
	* pre-function call "hook" invocation that can perform
	* initialization processing before the thread entry point <func>
	* method is called back.  The @a func and @a arg passed into the
	* start hook are the same as those passed by the application that
	* spawned the thread.
	*/
	virtual AX_thr_func_return start (AX_THR_FUNC func,
		void *arg);
	int add(AX_Hook_Routine *);
	void remove();
	int size();
	//void run();
private:
	stack<AX_Hook_Routine*> Hook_Stack;
	///// sets the system wide thread hook, returns the previous thread
	///// hook or 0 if none is set.
	static AX_Thread_Hook *thread_hook (AX_Thread_Hook *hook);
	bool hook_;

	///// Returns the current system thread hook.
	//static AX_Thread_Hook *thread_hook (void);
};
#endif
