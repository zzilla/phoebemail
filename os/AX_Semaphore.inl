

inline const AX_sema_t &
AX_Semaphore::lock (void) const
{
	// ACE_TRACE ("ACE_Semaphore::lock");
	return this->semaphore_;
}

inline int
AX_Semaphore::remove (void)
{
	// ACE_TRACE ("ACE_Semaphore::remove");
	//printf("the semaphore is remove \n");
	int result = 0;
	if (this->removed_ == 0)
	{
		this->removed_ = 1;
		result = AX_OS::sema_destroy (&this->semaphore_);
	}
	return result;
}

inline int
AX_Semaphore::acquire (void)
{
	// ACE_TRACE ("ACE_Semaphore::acquire");
	return AX_OS::sema_wait (&this->semaphore_);
}

inline int
AX_Semaphore::acquire (timeval &tv)
{
	// ACE_TRACE ("ACE_Semaphore::acquire");
	return AX_OS::sema_wait (&this->semaphore_, tv);
}

inline int
AX_Semaphore::acquire (timeval *tv)
{
	// ACE_TRACE ("ACE_Semaphore::acquire");
	return AX_OS::sema_wait (&this->semaphore_, tv);
}

inline int
AX_Semaphore::tryacquire (void)
{
	// ACE_TRACE ("ACE_Semaphore::tryacquire");
	return AX_OS::sema_trywait (&this->semaphore_);
}

inline int
AX_Semaphore::release (void)
{
	// ACE_TRACE ("ACE_Semaphore::release");
	return AX_OS::sema_post (&this->semaphore_);
}

inline int
AX_Semaphore::release (unsigned int release_count)
{
	// ACE_TRACE ("ACE_Semaphore::release");
	return AX_OS::sema_post (&this->semaphore_, release_count);
}

// Acquire semaphore ownership.  This calls <acquire> and is only
// here to make the <ACE_Semaphore> interface consistent with the
// other synchronization APIs.

inline int
AX_Semaphore::acquire_read (void)
{
	return this->acquire ();
}

// Acquire semaphore ownership.  This calls <acquire> and is only
// here to make the <ACE_Semaphore> interface consistent with the
// other synchronization APIs.

inline int
AX_Semaphore::acquire_write (void)
{
	return this->acquire ();
}

// Conditionally acquire semaphore (i.e., won't block).  This calls
// <tryacquire> and is only here to make the <ACE_Semaphore>
// interface consistent with the other synchronization APIs.

inline int
AX_Semaphore::tryacquire_read (void)
{
	return this->tryacquire ();
}

// Conditionally acquire semaphore (i.e., won't block).  This calls
// <tryacquire> and is only here to make the <ACE_Semaphore>
// interface consistent with the other synchronization APIs.

inline int
AX_Semaphore::tryacquire_write (void)
{
	return this->tryacquire ();
}

// This is only here to make the <ACE_Semaphore> interface consistent
// with the other synchronization APIs.  Assumes the caller has
// already acquired the semaphore using one of the above calls, and
// returns 0 (success) always.
inline int
AX_Semaphore::tryacquire_write_upgrade (void)
{
	return 0;
}

