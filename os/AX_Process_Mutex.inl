//inline const AX_mutex_t &
//AX_Process_Mutex::lock (void) const
//{
//	// ACE_TRACE ("ACE_Process_Mutex::lock");
//	return this->lock_.lock ();
//}
//#endif /* !_ACE_USE_SV_SEM */

// Explicitly destroy the mutex.
inline int
AX_Process_Mutex::remove (void)
{
	return this->lock_.remove ();
}

// Acquire lock ownership (wait on priority queue if necessary).
inline int
AX_Process_Mutex::acquire (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.acquire (0, SEM_UNDO);
//#else
	return this->lock_.acquire ();
//#endif // _ACE_USE_SV_SEM
}

// Acquire lock ownership (wait on priority queue if necessary).
inline int
AX_Process_Mutex::acquire (timeval &tv)
{
//#if !defined (_ACE_USE_SV_SEM)
	return this->lock_.acquire (tv);
//#else
//	ACE_UNUSED_ARG (tv);
//	ACE_NOTSUP_RETURN (-1);
//#endif  /* !_ACE_USE_SV_SEM */
}

// Conditionally acquire lock (i.e., don't wait on queue).
inline int
AX_Process_Mutex::tryacquire (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.tryacquire (0, SEM_UNDO);
//#else
	return this->lock_.tryacquire ();
//#endif // _ACE_USE_SV_SEM
}

// Release lock and unblock a thread at head of priority queue.
inline int
AX_Process_Mutex::release (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.release (0, SEM_UNDO);
//#else
	return this->lock_.release ();
//#endif // _ACE_USE_SV_SEM
}

// Acquire lock ownership (wait on priority queue if necessary).
inline int
AX_Process_Mutex::acquire_read (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.acquire_read (0, SEM_UNDO);
//#else
	return this->lock_.acquire_read ();
//#endif // _ACE_USE_SV_SEM
}

// Acquire lock ownership (wait on priority queue if necessary).
inline int
AX_Process_Mutex::acquire_write (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.acquire_write (0, SEM_UNDO);
//#else
	return this->lock_.acquire_write ();
//#endif // _ACE_USE_SV_SEM
}

// Conditionally acquire a lock (i.e., won't block).
inline int
AX_Process_Mutex::tryacquire_read (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.tryacquire_read (0, SEM_UNDO);
//#else
	return this->lock_.tryacquire_read ();
//#endif // _ACE_USE_SV_SEM
}

// Conditionally acquire a lock (i.e., won't block).
inline int
AX_Process_Mutex::tryacquire_write (void)
{
//#if defined (_ACE_USE_SV_SEM)
//	return this->lock_.tryacquire_write (0, SEM_UNDO);
//#else
	return this->lock_.tryacquire_write ();
//#endif // _ACE_USE_SV_SEM
}

//inline int
//AX_Process_Mutex::tryacquire_write_upgrade (void)
//{
//	return 0;
//}