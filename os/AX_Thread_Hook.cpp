#include "AX_Thread_Hook.h"

AX_Thread_Hook::AX_Thread_Hook()
{
}
AX_Thread_Hook::~AX_Thread_Hook ()
{
}

AX_thr_func_return
AX_Thread_Hook::start (AX_THR_FUNC func,
						void *arg)
{
	while(!Hook_Stack.empty())
	{
		AX_Hook_Routine* routine = Hook_Stack.top();
		(routine->func)(routine->arg);
		Hook_Stack.pop();
	}
#ifdef _WIN32
	return (DWORD)(func) (arg);
#else
	return (func) (arg);
#endif
}

int AX_Thread_Hook::add(AX_Hook_Routine *routine)
{
	Hook_Stack.push(routine);
	return 1;
}

void AX_Thread_Hook::remove()
{
	if (!Hook_Stack.empty())
	{
		Hook_Stack.pop();
	}
}
int AX_Thread_Hook::size()
{
	return (int)Hook_Stack.size();
}

//AX_Thread_Hook *
//AX_Thread_Hook::thread_hook (AX_Thread_Hook *hook)
//{
//	return ACE_OS_Object_Manager::thread_hook (hook);
//}
//
//AX_Thread_Hook *
//AX_Thread_Hook::thread_hook (void)
//{
//	return ACE_OS_Object_Manager::thread_hook ();
//}