
inline AX_thr_func_return
AX_Thread_Control::status (AX_thr_func_return s)
{
	//printf ("ACE_Thread_Control::status");
	return this->status_ = s;
}

// Get the exit status.

inline AX_thr_func_return
AX_Thread_Control::status (void)
{
	//printf ("ACE_Thread_Control::status");
	return this->status_;
}

// Returns the current <Thread_Manager>.


