
#include "AX_Thread_Adapter.h"
#include "AX_Thread_Manager.h"

//////////////////////////////////////////////////////////////////////////

AX_Base_Thread_Adapter::AX_Base_Thread_Adapter (
	AX_THR_FUNC user_func,
	void *arg,
	AX_THR_C_FUNC entry_point,
	AX_OS_Thread_Descriptor *td
#if defined (_WIN32)
	, AX_SEH_EXCEPT_HANDLER selector
	, AX_SEH_EXCEPT_HANDLER handler
#endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
	)
	: arg_ (arg)
	, entry_point_ (entry_point)
	, thr_desc_ (td)
{
	user_func_ = user_func;
	//printf ("ACE_Base_Thread_Adapter::ACE_Base_Thread_Adapter");

#ifdef ACE_USES_GPROF
	getitimer (ITIMER_PROF, &itimer_);
#endif // ACE_USES_GPROF
}

AX_Base_Thread_Adapter::~AX_Base_Thread_Adapter (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

AX_Thread_Adapter::AX_Thread_Adapter (
	AX_THR_FUNC user_func,
	void *arg,
	AX_THR_C_FUNC entry_point,
	//AX_Thread_Manager *thr_mgr,
	AX_Thread_Descriptor *td,
	AX_Thread_Exit *exit,
	AX_Thread_Hook *hook
#ifdef _WIN32
	, AX_SEH_EXCEPT_HANDLER selector
	, AX_SEH_EXCEPT_HANDLER handler
#endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
	)
	:AX_Base_Thread_Adapter(
	user_func
	, arg
	, entry_point
	, td
#if defined (_WIN32)
	, selector
	, handler
#endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
	)
	//,thr_mgr_(thr_mgr)
	,exit_(exit)
	,hook_(hook)
{
	//printf ("AX_Thread_Adapter::AX_Thread_Adapter");
}
AX_Thread_Adapter::AX_Thread_Adapter (
									  AX_THR_FUNC user_func,
									  void *arg,
									  AX_THR_C_FUNC entry_point,
									  //AX_Thread_Manager *thr_mgr,
									  AX_Thread_Descriptor *td
#ifdef _WIN32
									  , AX_SEH_EXCEPT_HANDLER selector
									  , AX_SEH_EXCEPT_HANDLER handler
#endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
									  )
									  :AX_Base_Thread_Adapter(
									  user_func
									  , arg
									  , entry_point
									  , td
#if defined (_WIN32)
									  , selector
									  , handler
#endif /* ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS */
									  )
									  //,thr_mgr_(thr_mgr)
{
	//printf ("AX_Thread_Adapter::AX_Thread_Adapter");
}

AX_Thread_Adapter::~AX_Thread_Adapter (void)
{
}

AX_thr_func_return
AX_Thread_Adapter::invoke (void)
{
	return this->invoke_i ();
}

AX_thr_func_return
AX_Thread_Adapter::invoke_i (void)
{
	// Extract the arguments.
	AX_THR_FUNC func = this->user_func_;
	void *arg = this->arg_;

#if defined (ACE_WIN32) && defined (ACE_HAS_MFC) && (ACE_HAS_MFC != 0)
	AX_OS_Thread_Descriptor *thr_desc = this->thr_desc_;
#endif /* ACE_WIN32 && ACE_HAS_MFC && (ACE_HAS_MFC != 0) */

	delete this;

	AX_thr_func_return status = 0;

			//AX_Thread_Hook *hook =
			//	ACE_OS_Object_Manager::thread_hook ();

	if (hook_)
	{
		// Invoke the start hook to give the user a chance to
		// perform some initialization processing before the
		// <func> is invoked.
		status = hook_->start (func, arg);
	}
	else
	{
		// Call thread entry point.
#ifdef _WIN32
		status = (DWORD)(*func) (arg);
#else
		status = (*func) (arg);
#endif
	}
	if (exit_)
	{
		exit_->run();
	}
	return status;
}