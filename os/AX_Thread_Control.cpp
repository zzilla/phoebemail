#include "AX_Thread_Control.h"

void
AX_Thread_Control::dump (void) const
{
	//printf ("ACE_Thread_Control::dump");
}

int
AX_Thread_Control::insert (AX_Thread_Manager *tm, int insert)
{
	//printf ("ACE_Thread_Control::insert");

	AX_hthread_t t_id;
	AX_OS::thr_self (t_id);
	this->tm_ = tm;

	if (insert)
		return this->tm_->insert_thr (AX_OS::thr_self (), t_id);
	else
		return 0;
}

// Initialize the thread controller.

AX_Thread_Control::AX_Thread_Control (AX_Thread_Manager *t,
										int insert)
										: tm_ (t),
										status_ (0)
{
	//printf ("ACE_Thread_Control::ACE_Thread_Control");

	if (this->tm_ != 0 && insert)
	{
		AX_hthread_t t_id;
		AX_OS::thr_self (t_id);
		this->tm_->insert_thr (AX_OS::thr_self (), t_id);
	}
}

// Automatically kill thread on exit.

AX_Thread_Control::~AX_Thread_Control (void)
{
	//printf ("ACE_Thread_Control::~ACE_Thread_Control");

#ifdef _WIN32
	this->exit (this->status_, 0);
#else
	this->exit (this->status_, 1);
#endif /* ACE_HAS_RECURSIVE_THR_EXIT_SEMANTICS */
}

// Exit from thread (but clean up first).

AX_thr_func_return
AX_Thread_Control::exit (AX_thr_func_return exit_status, int do_thr_exit)
{
	//printf("ACE_Thread_Control::exit");

	if (this->tm_ != 0)
		return this->tm_->exit (exit_status, do_thr_exit);
	else
	{
		AX_OS::thr_exit (exit_status);
		return 0;
	}
}
// Atomically set a new <Thread_Manager> and return the old
// <Thread_Manager>.

AX_Thread_Manager *
AX_Thread_Control::thr_mgr (AX_Thread_Manager *tm)
{
	//printf("ACE_Thread_Control::thr_mgr");
	AX_Thread_Manager *o_tm = this->tm_;
	this->tm_ = tm;
	return o_tm;
}
AX_Thread_Manager *
AX_Thread_Control::thr_mgr (void)
{
	//printf ("ACE_Thread_Control::thr_mgr");
	return this->tm_;
}

