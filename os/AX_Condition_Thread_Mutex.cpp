#include "AX_Condition_Thread_Mutex.h"

void AX_Condition_Thread_Mutex::dump (void) const
{
//#ifdef _WIN32
//	this->cond_.waiters ();
//#endif
}

AX_Condition_Thread_Mutex::AX_Condition_Thread_Mutex (const AX_Thread_Mutex &m,
														const char *name,
														void *arg)
														: mutex_ ((AX_Thread_Mutex &) m),
														removed_ (0)
{
	//printf("before AX_Condition_Thread_Mutex::AX_Condition_Thread_Mutex \n");
#ifdef _WIN32
	//AX_OS::thread_mutex_init(&m);
	int result = AX_OS::cond_init (&this->cond_,
		(short) AX_USYNC_THREAD,
		name,
		arg) ;
#else

	int result = AX_OS::cond_init (&this->cond_,
		0,
		name,
		arg) ;
#endif
	//printf("after AX_Condition_Thread_Mutex::AX_Condition_Thread_Mutex result :%d\n",result);

}
int AX_Condition_Thread_Mutex::remove (void)
{
	int result = 0;

	if (this->removed_ == 0)
	{
		this->removed_ = 1;

		while ((result = AX_OS::cond_destroy (&this->cond_)) == -1
			&& errno == EBUSY)
		{
			AX_OS::cond_broadcast (&this->cond_);
			AX_OS::thr_yield ();
		}
	}
	//AX_OS::thread_mutex_destroy(&m);
	return result;
}
AX_Condition_Thread_Mutex::AX_Condition_Thread_Mutex (AX_Thread_Mutex &m,
							AX_Condition_Attributes &attributes,
							const char *name,
							void *arg)
							: mutex_ (m),
							removed_ (0)
{
	//AX_OS::thread_mutex_init(&m);
	AX_OS::cond_init(&this->cond_, attributes.attributes_,
		name, arg);
}

AX_Condition_Thread_Mutex::~AX_Condition_Thread_Mutex (void)
{
	this->remove();
}

int AX_Condition_Thread_Mutex::wait (void)
{
	// ACE_TRACE ("ACE_Condition_Thread_Mutex::wait");
	return AX_OS::cond_wait (&this->cond_, &this->mutex_.lock_);
}

int AX_Condition_Thread_Mutex::wait (AX_Thread_Mutex &mutex,
								  const timeval *abstime)
{
	// ACE_TRACE ("ACE_Condition_Thread_Mutex::wait");
	if (abstime != NULL)
	{
		return AX_OS::cond_timedwait (&this->cond_,
			&mutex.lock_,
			const_cast <timeval *> (abstime));
	}
	else
	{
		return AX_OS::cond_wait (&this->cond_,
			&mutex.lock_);
	}
}

int AX_Condition_Thread_Mutex::signal (void)
{
	// ACE_TRACE ("ACE_Condition_Thread_Mutex::signal");
	return AX_OS::cond_signal (&this->cond_);
}

int AX_Condition_Thread_Mutex::broadcast (void)
{
	// ACE_TRACE ("ACE_Condition_Thread_Mutex::broadcast");
	return AX_OS::cond_broadcast (&this->cond_);
}
