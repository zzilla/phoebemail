
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_OS.inl
* 文件标识：
* 摘　　要：操作系统适配层 .inl文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/


//目前Windows Mobile SDK 5  不提供 InitializeSecurityDescriptor
//同时在CreateSemaphore和CreateEvent中，所以在编译WINCE版本时候，屏蔽掉这2个函数
#ifndef WINCE 


#ifdef WIN32

inline LPSECURITY_ATTRIBUTES
AX_OS::default_win32_security_attributes (LPSECURITY_ATTRIBUTES sa)
{
	if (sa == 0)
	{
		// @@ This is a good place to use pthread_once.
		static SECURITY_ATTRIBUTES default_sa;
		static SECURITY_DESCRIPTOR sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);
		default_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		default_sa.lpSecurityDescriptor = &sd;
		default_sa.bInheritHandle       = TRUE;
		sa = &default_sa;
	}
	return sa;
}

inline LPSECURITY_ATTRIBUTES
AX_OS::default_win32_security_attributes_r (LPSECURITY_ATTRIBUTES sa,
											LPSECURITY_ATTRIBUTES sa_buffer,
											SECURITY_DESCRIPTOR* sd_buffer)
{
	if (sa == 0)
	{
		if (sa_buffer != 0 && sd_buffer != 0)
		{
			InitializeSecurityDescriptor
				(sd_buffer, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl (sd_buffer, TRUE, NULL, FALSE);
			sa_buffer->nLength = sizeof(SECURITY_ATTRIBUTES);
			sa_buffer->lpSecurityDescriptor = sd_buffer;
			sa_buffer->bInheritHandle       = TRUE;
			sa = sa_buffer;
		}
	}
	return sa;
}

#else
#include <sys/types.h>
#include <sys/wait.h>

#endif	//#ifdef WIN32

#endif//WINCE 

inline int 
AX_OS::sleep(int msec)
{
#ifdef WIN32
	::Sleep(msec);
	return 0;
#else
	::usleep(msec*1000);
	return 0;
#endif
}

inline int 
AX_OS::thread_mutex_init (AX_Mutex_t *m, int lock_type/* = 0*/)
{
#ifdef WIN32
	::InitializeCriticalSection (&m->thr_mutex_);	//默认递归
//	m->event = CreateEvent(NULL,FALSE,FALSE,NULL);
	return 0;
#else
	pthread_mutexattr_t attr;
	int ret=pthread_mutexattr_init(&attr);
	if(0==ret)
	{
		ret=pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		if(0==ret)
		{
			ret=pthread_mutex_init(m,&attr);
		}
		pthread_mutexattr_destroy(&attr);
	}
	return ret;
	//attr.__mutexkind = PTHREAD_MUTEX_RECURSIVE_NP;	//默认递归
	//return pthread_mutex_init (m, &attr);
#endif
}

inline int 
AX_OS::thread_mutex_lock (AX_Mutex_t *m)
{
#ifdef WIN32
	::EnterCriticalSection (&m->thr_mutex_); 
	//int threadId = GetCurrentThreadId();
	//printf("the current thread :%d is entry\n",threadId);
	return 0;
#else
	return pthread_mutex_lock (m);
#endif
}

inline int 
AX_OS::thread_mutex_lock (AX_Mutex_t *m, const timeval &timeout)
{
#ifdef WIN32
	/*
	int index = WaitForSingleObject(m->event,timeout.tv_sec*1000+timeout.tv_usec);
	switch (index)
	{
	case WAIT_OBJECT_0:
		::EnterCriticalSection (&m->thr_mutex_); 
		break;
	default:
		return -1;
	}
	*/
	//unsupported
	return 0;
#else
	return pthread_mutex_timedlock(m, (timespec*)&timeout);
#endif
}

inline int 
AX_OS::thread_mutex_lock (AX_Mutex_t *m, const timeval *timeout)
{
	return timeout == 0
		? AX_OS::thread_mutex_lock (m)
		: AX_OS::thread_mutex_lock (m, *timeout);
}

inline int 
AX_OS::thread_mutex_trylock (AX_Mutex_t *m)
{
#ifdef WIN32
	BOOL result = ::TryEnterCriticalSection (&m->thr_mutex_);
	if (result == TRUE)
	{
		result = 0;
	}
	else
	{
		result = -1;
		//errno = EBUSY;
	}
	return result;
#else
	int result = pthread_mutex_trylock (m);
	if (result == 1)
	{
		result = 0;
	}
	else if (result == 0) 
	{
		result = -1;
		//errno = EBUSY;
	}
	return result;
#endif
}

inline int 
AX_OS::thread_mutex_unlock (AX_Mutex_t *m)
{
#ifdef WIN32
	//if (m->event!=0)
	{
	//	::SetEvent(m->event);
	}
	//int threadId = GetCurrentThreadId();
	//printf("the current thread :%d is leaving\n",threadId);
	::LeaveCriticalSection (&m->thr_mutex_); 
	return 0;
#else
	return pthread_mutex_unlock (m);
#endif
}

inline int 
AX_OS::thread_mutex_destroy (AX_Mutex_t *m)
{
#ifdef WIN32
	//if (m->event!=0)
	{
	//	::CloseHandle(m->event);
	}
	::DeleteCriticalSection (&m->thr_mutex_);
	return 0;
#else
	return pthread_mutex_destroy (m);
#endif
}

inline int 
AX_OS::sema_init (AX_sema_t *s,
				  unsigned int count,
				  int type,
				  const char *name,
				  void *arg,
				  int max
#ifdef _WIN32
				  ,LPSECURITY_ATTRIBUTES sa
#endif				
				  )
{
#if defined(WINCE)
	*s = ::CreateSemaphoreA(
		NULL,
		count,
		max,
		name);

	if (*s == 0)
		return -1;
	else
		return 0;
	return 0;
#elif defined(WIN32)
	SECURITY_ATTRIBUTES sa_buffer;
	SECURITY_DESCRIPTOR sd_buffer;
	*s = ::CreateSemaphoreA
		(AX_OS::default_win32_security_attributes_r (sa, &sa_buffer, &sd_buffer),
		count,
		max,
		name);

	if (*s == 0)
		return -1;
	else
		return 0;
	return 0;

#else	//linux
	int res=sem_init(s,0,count);
	if (res!=0)
	{
		perror("semaphore initializeating failed\n");
		return -1;
	}
	return 0;
#endif
}

inline int 
AX_OS::sema_post (AX_sema_t *s)
{
#ifdef WIN32
	int result = ::ReleaseSemaphore (*s, 1, 0);
	return result != 0 ? 0 : -1;
#else	//linux
	//printf("%x \n",s);
	int result=sem_post(s);
	return result;
#endif
}

inline int 
AX_OS::sema_post (AX_sema_t *s, unsigned int release_count)
{
#ifdef WIN32
	int result = ::ReleaseSemaphore (*s, release_count, 0);
	return result != 0 ? 0 : -1;
#else	//linux
	for (unsigned int i = 0; i < release_count; i++)
	{
		AX_OS::sema_post(s);
	}
	return 0;
#endif
}

inline int 
AX_OS::sema_wait (AX_sema_t *s)
{
#ifdef WIN32
	switch (::WaitForSingleObject (*s, INFINITE))
	{
	case WAIT_OBJECT_0:
		return 0;
	default:
		return -1;
	}

#else	//linux
	int result=sem_wait(s);
	return result;
#endif
}

inline int 
AX_OS::sema_wait (AX_sema_t *s, timeval &tv)
{
#ifdef WIN32
	int msec_timeout;
	msec_timeout = tv.tv_sec*1000 + tv.tv_usec/1000;
	switch (::WaitForSingleObject (*s, msec_timeout))
	{
	case WAIT_OBJECT_0:
		// Update time to when acquired
		//later...
		return 0;
	case WAIT_TIMEOUT:
		//errno = ETIME;
		return -1;
	default:
		return -1;
	}

#else	//linux
	AX_OS::sema_wait(s);
	return 0;
#endif
}
inline int AX_OS::sema_trywait(AX_sema_t *s)
{
#ifdef _WIN32
	int index = ::WaitForSingleObject(*s,0);
	switch (index)
	{
	case WAIT_OBJECT_0:
		return 0;
	case WAIT_TIMEOUT:
		return -1;
	}
#else
	return sem_trywait(s);
#endif
}
inline int 
AX_OS::sema_wait (AX_sema_t *s, timeval *tv)
{
	  return tv == 0 ? AX_OS::sema_wait (s) : AX_OS::sema_wait (s, *tv);
}

inline int
AX_OS::sema_destroy (AX_sema_t *s)
{

#ifdef WIN32
	BOOL result = ::CloseHandle (*s);
	return result!=FALSE ? 0 : -1;
#else	//linux
	return sem_destroy(s);
#endif
}

inline int 
AX_OS::event_init (AX_event_t *event,
					   int manual_reset,
					   int initial_state,
					   int type,
					   const char *name,
					   void *arg,
					   LPSECURITY_ATTRIBUTES sa)
{
#if defined(WINCE)
	*event = ::CreateEventA (NULL,
		manual_reset,
		initial_state,
		name);
	if (*event == 0)
		return -1;
	else
		return 0;
#elif defined(WIN32)
	SECURITY_ATTRIBUTES sa_buffer;
	SECURITY_DESCRIPTOR sd_buffer;
	*event = ::CreateEventA (AX_OS::default_win32_security_attributes_r
		(sa, &sa_buffer, &sd_buffer),
		manual_reset,
		initial_state,
		name);
	if (*event == 0)
		return -1;
	else
		return 0;
#else //linux
	pthread_mutex_init(event,NULL);
	return 0;
#endif
}

inline int 
AX_OS::event_destroy (AX_event_t *event)
{
#ifdef WIN32
	BOOL result = ::CloseHandle (*event);
	return result!=FALSE ? 0 : -1;
#else //linux
	pthread_mutex_destroy(event);
	return 0;
#endif
}


inline int 
AX_OS::event_reset (AX_event_t *event)
{
#ifdef WIN32
	BOOL result = ::ResetEvent (*event);
	return result!=FALSE ? 0 : -1;
#else //linux
	pthread_mutex_lock(event);
	return 0;
#endif
}

inline int 
AX_OS::event_signal (AX_event_t *event)
{
#ifdef WIN32
	BOOL result = ::SetEvent (*event);
	return result!=FALSE ? 0 : -1;
#else //linux
	if (pthread_mutex_unlock(event)!=0)
	{
		return -1;
	}
	return 0;
#endif
}
inline int AX_OS::event_pulse(AX_event_t *event)
{
	AX_OS::event_signal(event);
	AX_OS::event_reset(event);
	return 1;
}
inline int 
AX_OS::event_timedwait (AX_event_t *event,	timeval *timeout)
{
#ifdef WIN32
	DWORD result;
	if (timeout == 0)
		// Wait forever
		result = ::WaitForSingleObject (*event, INFINITE);
	else if (timeout->tv_sec == 0 && timeout->tv_usec == 0)
		// Do a "poll".
		result = ::WaitForSingleObject (*event, 0);
	else
	{
		int msec_timeout;
		msec_timeout = timeout->tv_sec*1000 + timeout->tv_usec/1000;
		result = ::WaitForSingleObject (*event, msec_timeout);
	}

	switch (result)
	{
	case WAIT_OBJECT_0:
		return 0;
	case WAIT_TIMEOUT:
		//errno = ETIME;
		return -1;
	default:
		return -1;
	}

#else //linux
	pthread_mutex_timedlock(event,(const timespec*)timeout);
	return -1;
#endif
}

inline int 
AX_OS::event_wait (AX_event_t *event)
{
#ifdef WIN32
	switch (::WaitForSingleObject (*event, INFINITE))
	{
	case WAIT_OBJECT_0:
		return 0;
	default:
		return -1;
	}
#else //linux

	return pthread_mutex_lock(event);
#endif
}

inline int
AX_OS::cond_init (AX_cond_t *cv, short type, const char *name, void *arg)
{
#ifdef WIN32
	cv->waiters_ = 0;
	cv->was_broadcast_ = 0;

	int result = 0;
	if (AX_OS::sema_init (&cv->sema_, 0, type, name, arg) == -1)
		result = -1;
	else if (AX_OS::thread_mutex_init (&cv->waiters_lock_) == -1)
		result = -1;
	else if (AX_OS::event_init (&cv->waiters_done_) == -1)
		result = -1;

	return result;
#else //linux
	int result = pthread_cond_init(cv,NULL);
	return result;
#endif
}

inline int 
AX_OS::cond_init (AX_cond_t *cv, AX_condattr_t &attributes, const char *name,	void *arg)
{
#ifdef WIN32  
	return AX_OS::cond_init (cv, static_cast<short> (attributes.type), name, arg);
#else //linux
	pthread_cond_init(cv,&attributes);
	return 0;
#endif
}

inline int 
AX_OS::cond_wait (AX_cond_t *cv, AX_Mutex_t *external_mutex)
{
#ifdef WIN32
	AX_OS::thread_mutex_lock (&cv->waiters_lock_);
	cv->waiters_++;
	AX_OS::thread_mutex_unlock (&cv->waiters_lock_);

	int result = 0;

	if (AX_OS::thread_mutex_unlock (external_mutex) != 0)
		return -1;

	// Wait to be awakened by a AX_OS::cond_signal() or
	// AX_OS::cond_broadcast().
	result = AX_OS::sema_wait (&cv->sema_);

	// Reacquire lock to avoid race conditions on the <waiters_> count.
	AX_OS::thread_mutex_lock (&cv->waiters_lock_);
	// We're ready to return, so there's one less waiter.
	cv->waiters_--;
	int last_waiter = cv->was_broadcast_ && cv->waiters_ == 0;
	AX_OS::thread_mutex_unlock (&cv->waiters_lock_);

	if (result == -1)
		// Bad things happened, so let's just return below.
		/* NOOP */;
	else if (last_waiter)
		AX_OS::event_signal (&cv->waiters_done_);

	AX_OS::thread_mutex_lock (external_mutex);

	return result;
#else //linux
	int result = pthread_cond_wait(cv,external_mutex);
	//printf("cond_wait :%d",result);
	return result;
#endif
}

inline int AX_OS::cond_timedwait(AX_cond_t *cv,AX_Mutex_t *external_mutex,timeval *timeout)
{
#ifdef _WIN32
	AX_OS::thread_mutex_lock (&cv->waiters_lock_);
	cv->waiters_++;
	AX_OS::thread_mutex_unlock (&cv->waiters_lock_);

	int result = 0;

	if (AX_OS::thread_mutex_unlock (external_mutex) != 0)
		return -1;
	result = AX_OS::sema_wait (&cv->sema_,timeout);

	// Reacquire lock to avoid race conditions on the <waiters_> count.
	AX_OS::thread_mutex_lock (&cv->waiters_lock_);
	// We're ready to return, so there's one less waiter.
	cv->waiters_--;
	int last_waiter = cv->was_broadcast_ && cv->waiters_ == 0;
	AX_OS::thread_mutex_unlock (&cv->waiters_lock_);

	if (result == -1)
		// Bad things happened, so let's just return below.
		/* NOOP */;
	else if (last_waiter)
		AX_OS::event_signal (&cv->waiters_done_);

	AX_OS::thread_mutex_lock (external_mutex);
	return result;
#else
	return  pthread_cond_timedwait(cv,external_mutex,(const timespec*)timeout);
#endif
}
inline int 
AX_OS::cond_signal (AX_cond_t *cv)
{
#ifdef WIN32
	AX_OS::thread_mutex_lock (&cv->waiters_lock_);
	int have_waiters = cv->waiters_ > 0;
	AX_OS::thread_mutex_unlock (&cv->waiters_lock_);

	if (have_waiters != 0)
		return AX_OS::sema_post (&cv->sema_);
	else
		return 0; // No-op
#else //linux
	return pthread_cond_signal(cv);
#endif
}

inline int 
AX_OS::cond_broadcast (AX_cond_t *cv)
{
#ifdef WIN32
	// This is needed to ensure that <waiters_> and <was_broadcast_> are
	// consistent relative to each other.
	AX_OS::thread_mutex_lock (&cv->waiters_lock_);
	int have_waiters = 0;

	if (cv->waiters_ > 0)
	{
		// We are broadcasting, even if there is just one waiter...
		// Record the fact that we are broadcasting.  This helps the
		// cond_wait() method know how to optimize itself.  Be sure to
		// set this with the <waiters_lock_> held.
		cv->was_broadcast_ = 1;
		have_waiters = 1;
	}

	AX_OS::thread_mutex_unlock (&cv->waiters_lock_);
	int result = 0;
	if (have_waiters)
	{
		// Wake up all the waiters.
		if (AX_OS::sema_post (&cv->sema_, cv->waiters_) == -1)
			result = -1;
		// the counting semaphore.
		else if (AX_OS::event_wait (&cv->waiters_done_) == -1)
			result = -1;
		// This is okay, even without the <waiters_lock_> held because
		// no other waiter threads can wake up to access it.
		cv->was_broadcast_ = 0;
	}
	return result;
#else //linux
	pthread_cond_broadcast(cv);
	return 0;
#endif
}

inline int
AX_OS::cond_destroy (AX_cond_t *cv)
{
#ifdef WIN32
	AX_OS::event_destroy (&cv->waiters_done_);
	AX_OS::thread_mutex_destroy (&cv->waiters_lock_);
	return AX_OS::sema_destroy (&cv->sema_);
#else //linux
	pthread_cond_destroy(cv);
	return 0;
#endif
}

inline int 
AX_OS::condattr_init (AX_condattr_t &attributes, int type)
{
#ifdef WIN32
	attributes.type = type;
	return 0;
#else //linux
	return pthread_condattr_init(&attributes);
#endif
}

inline int 
AX_OS::condattr_destroy (AX_condattr_t &attributes)
{
#ifdef WIN32  
	return 0;
#else //linux
	pthread_condattr_destroy(&attributes);
	return 0;
#endif
}

inline int
AX_OS::rw_rdlock (AX_rwlock_t *rw)
{
#ifdef WIN32
	int result = 0;
	if (AX_OS::thread_mutex_lock (&rw->lock_) == -1)
		result = -1; // -1 means didn't get the mutex.
	else
	{
		// Give preference to writers who are waiting.
		while (rw->ref_count_ < 0 || rw->num_waiting_writers_ > 0)
		{
			rw->num_waiting_readers_++;
			if (AX_OS::cond_wait (&rw->waiting_readers_, &rw->lock_) == -1)
			{
				result = -2; // -2 means that we need to release the mutex.
				break;
			}
			rw->num_waiting_readers_--;
		}
	}
	if (result == 0)
		rw->ref_count_++;
	if (result != -1)
		AX_OS::thread_mutex_unlock (&rw->lock_);

	return 0;
#else	//linux
	return pthread_rwlock_rdlock (rw);
#endif
}

inline int 
AX_OS::rw_tryrdlock (AX_rwlock_t *rw)
{
#ifdef WIN32
	int result = -1;
	if (AX_OS::thread_mutex_lock (&rw->lock_) != -1)
	{
		//AX_Errno_Guard error (errno);
		if (rw->ref_count_ == -1 || rw->num_waiting_writers_ > 0)
		{
			//error = EBUSY;
			result = -1;
		}
		else
		{
			rw->ref_count_++;
			result = 0;
		}

		AX_OS::thread_mutex_unlock (&rw->lock_);
	}
	return result;
#else	//linux
	return pthread_rwlock_tryrdlock (rw);
#endif
}

inline int 
AX_OS::rw_trywrlock (AX_rwlock_t *rw)
{
#ifdef WIN32
	int result = -1;
	if (AX_OS::thread_mutex_lock (&rw->lock_) != -1)
	{
		//AX_Errno_Guard error (errno);
		if (rw->ref_count_ != 0)
		{
			//error = EBUSY;
			result = -1;
		}
		else
		{
			rw->ref_count_ = -1;
			result = 0;
		}

		AX_OS::thread_mutex_unlock (&rw->lock_);
	}
	return result;
#else	//linux
	return pthread_rwlock_trywrlock (rw);
#endif
}

inline int 
AX_OS::rw_unlock (AX_rwlock_t *rw)
{
#ifdef WIN32
	if (AX_OS::thread_mutex_lock (&rw->lock_) == -1)
		return -1;

	if (rw->ref_count_ > 0) // Releasing a reader.
		rw->ref_count_--;
	else if (rw->ref_count_ == -1) // Releasing a writer.
		rw->ref_count_ = 0;
	else
		return -1; // @@ AX_ASSERT (!"count should not be 0!\n");

	int result = 0;
	//AX_Errno_Guard error (errno);

	if (rw->important_writer_ && rw->ref_count_ == 1)
		// only the reader requesting to upgrade its lock is left over.
	{
		result = AX_OS::cond_signal (&rw->waiting_important_writer_);
		//error = errno;
	}
	else if (rw->num_waiting_writers_ > 0 && rw->ref_count_ == 0)
		// give preference to writers over readers...
	{
		result = AX_OS::cond_signal (&rw->waiting_writers_);
		//error =  errno;
	}
	else if (rw->num_waiting_readers_ > 0 && rw->num_waiting_writers_ == 0)
	{
		result = AX_OS::cond_broadcast (&rw->waiting_readers_);
		//error = errno;
	}

	AX_OS::thread_mutex_unlock (&rw->lock_);
	return result;
#else	//linux
	return pthread_rwlock_unlock (rw);
#endif
}

inline int 
AX_OS::rw_wrlock (AX_rwlock_t *rw)
{
#ifdef WIN32
	int result = 0;

	if (AX_OS::thread_mutex_lock (&rw->lock_) == -1)
		result = -1; // -1 means didn't get the mutex.
	else
	{
		while (rw->ref_count_ != 0)
		{
			rw->num_waiting_writers_++;

			if (AX_OS::cond_wait (&rw->waiting_writers_, &rw->lock_) == -1)
			{
				result = -2; // -2 means we need to release the mutex.
				break;
			}

			rw->num_waiting_writers_--;
		}
	}
	if (result == 0)
		rw->ref_count_ = -1;
	if (result != -1)
		AX_OS::thread_mutex_unlock (&rw->lock_);
	return 0;
#else	//linux
	return pthread_rwlock_wrlock (rw);
#endif
}

inline int 
AX_OS::rwlock_destroy (AX_rwlock_t *rw)
{
#ifdef WIN32
	AX_OS::thread_mutex_destroy (&rw->lock_);
	AX_OS::cond_destroy (&rw->waiting_readers_);
	AX_OS::cond_destroy (&rw->waiting_important_writer_);
	return AX_OS::cond_destroy (&rw->waiting_writers_);
#else	//linux
	return pthread_rwlock_destroy (rw);
#endif
}

inline int 
AX_OS::rwlock_init (AX_rwlock_t *rw,
						int type,
						const char *name,
						void *arg)
{
#ifdef WIN32

	int result = -1;

	AX_condattr_t attributes;
	if (AX_OS::condattr_init (attributes, type) == 0)
	{
		if (AX_OS::thread_mutex_init (&rw->lock_, type) == 0
			&& AX_OS::cond_init (&rw->waiting_readers_, attributes, 0, arg) == 0
			&& AX_OS::cond_init (&rw->waiting_writers_, attributes, 0, arg) == 0
			&& AX_OS::cond_init (&rw->waiting_important_writer_, attributes, 0, arg) == 0)
		{
			// Success!
			rw->ref_count_ = 0;
			rw->num_waiting_writers_ = 0;
			rw->num_waiting_readers_ = 0;
			rw->important_writer_ = 0;
			result = 0;
		}
		AX_OS::condattr_destroy (attributes);
	}

	if (result == -1)
	{
		// Save/restore errno.
		//AX_Errno_Guard error (errno);
		AX_OS::thread_mutex_destroy (&rw->lock_);
		AX_OS::cond_destroy (&rw->waiting_readers_);
		AX_OS::cond_destroy (&rw->waiting_writers_);
		AX_OS::cond_destroy (&rw->waiting_important_writer_);
	}
	return result;

#else	//linux

	int status;
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init (&attr);
	//下面这行有时不需要
	pthread_rwlockattr_setpshared(&attr, (type == AX_USYNC_THREAD ? PTHREAD_PROCESS_PRIVATE :	PTHREAD_PROCESS_SHARED));
	status = pthread_rwlock_init (rw, &attr);
	pthread_rwlockattr_destroy (&attr);

	return status;
#endif
}

inline int
AX_OS::thr_join (AX_hthread_t thr_handle,
					 AX_thr_func_return *status)
{
#ifdef WIN32
	AX_thr_func_return local_status = 0;

	// Make sure that status is non-NULL.
	if (status == 0)
		status = &local_status;

	if (::WaitForSingleObject (thr_handle, INFINITE) == WAIT_OBJECT_0
		&& ::GetExitCodeThread (thr_handle, status) != FALSE)
	{
		::CloseHandle (thr_handle);
		return 0;
	}
	return -1;

#else	//linux
	int result = 0;

	AX_thr_func_return temp;
	if (pthread_join (thr_handle, status==0 ? &temp : status) != 0)
		return -1;

	// Joinable threads need to be detached after joining on Pthreads
	pthread_detach (thr_handle);

	return result;

#endif
}

#if !defined(WINCE) //Windows CE下面不支持 OpenThread,所以不能从ThreadId转换到ThreadHandle

inline int
AX_OS::thr_join (AX_thread_t waiter_id,
					 AX_thread_t *thr_id,
					 AX_thr_func_return *status)
{
#ifndef _WIN32
	return AX_OS::thr_join(waiter_id, status);
#else
	HANDLE result=OpenThread(THREAD_ALL_ACCESS,FALSE,waiter_id) ;
	if (result==INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	return AX_OS::thr_join(result,status);
#endif
}

inline int
AX_OS::thr_kill (AX_thread_t thr_id, int signum)
{
#ifdef WIN32
	HANDLE thr_handle=OpenThread(THREAD_ALL_ACCESS,FALSE,thr_id);
	int result=TerminateThread(thr_handle,-1);
	//unsupported
	//why not use TerminateThread() ?
	return 0;

#else	//linux

	//UNUSED_ARG (thr_id);
	int result = 0;

	if (pthread_kill (thr_id, signum) != 0)
		return -1;

	return result;

#endif
}

#endif//WINCE

#ifdef WIN32
inline int
AX_OS::thr_kill (AX_hthread_t id, int signum)
{
   int result = TerminateThread(id, -1);
   return result;
}

#endif

inline AX_thread_t
AX_OS::thr_self (void)
{
#ifdef WIN32
	return ::GetCurrentThreadId ();
#else	//linux
	return pthread_self ();
#endif
}

inline void
AX_OS::thr_self (AX_hthread_t &self)
{
#ifdef WIN32
	//HANDLE hThread=::GetCurrentThread();
	self = ::GetCurrentThread ();
	//self._threadId=hThread;
#else	//linux
	self = pthread_self ();
#endif
}
#if !defined(WINCE)//Windows CE下面不支持 GetThreadPriority,GetPriorityClass，所以禁用这3个函数

inline int 
AX_OS::thr_getprio (AX_hthread_t ht_id, int &priority, int &policy)
{
#ifdef WIN32
	priority = ::GetThreadPriority (ht_id);

	DWORD priority_class = ::GetPriorityClass (::GetCurrentProcess ());
	if (priority_class == 0 && ::GetLastError () != NO_ERROR)
		return -1;

	policy =
		(priority_class ==
		REALTIME_PRIORITY_CLASS) ? THR_SCHED_FIFO : THR_SCHED_DEFAULT;
#else	//linux

	struct sched_param param;
	int result;

	result = pthread_getschedparam (ht_id, &policy, &param)==0 ? 0 : -1;
	if (result==-1)
		return result;

	priority = param.sched_priority;
	return result;

#endif
	return 1;
}

inline int 
AX_OS::thr_getprio (AX_hthread_t ht_id, int &priority)
{
	int policy = 0;
	return AX_OS::thr_getprio (ht_id, priority, policy);
}

#endif//WINCE
inline int 
AX_OS::thr_setprio (AX_hthread_t ht_id,
						int priority,
						int policy)
{
#ifdef WIN32
	int result = ::SetThreadPriority (ht_id, priority);
	return result!=FALSE ? 0 : -1;
#else	//linux
	int result;
	struct sched_param param;
	memset ((void *) &param, 0, sizeof param);

	// If <policy> is -1, we don't want to use it for
	// pthread_setschedparam().  Instead, obtain policy from
	// pthread_getschedparam().
	if (policy == -1)
	{
		result = pthread_getschedparam (ht_id, &policy, &param)==0 ? 0 : -1;
		//printf("policy == -1 result :%d\n",result);
		if (result == -1)
			return result;
	}

	param.sched_priority = priority;

	result = pthread_setschedparam (ht_id, policy, &param)==0 ? 0 : -1;
	//printf("thr_setprio return  result :%d\n",result);

	return result;
#endif
}


inline int
AX_OS::thr_suspend (AX_hthread_t target_thread)
{
#ifdef WIN32
	if (::SuspendThread (target_thread) == (DWORD) -1 )
		return -1;
	return 0;
	
#else	//linux
	//AX_SET_BITS(target_thread._status,THR_SUSPEND);
	//do 
	//{
	//	AX_OS::sleep(1000);
	//} while (AX_BIT_ENABLED(target_thread._status,THR_SUSPEND));
	return -1;
#endif
}

inline int
AX_OS::thr_continue (AX_hthread_t target_thread,int status)
{
#ifdef WIN32
	//if (AX_BIT_ENABLED(target_thread._status,THR_STOPPED))
	//{
	//	::ExitThread(0);
	//}
	if (::ResumeThread (target_thread) == (DWORD) -1)
		return -1;
	return 0;

#else	//linux
	//if (AX_BIT_ENABLED(target_thread._status,THR_SUSPEND))
	//{
		AX_CLR_BITS(status,THR_SUSPEND);
	//}
	return -1;
#endif
}

inline int
AX_OS::thr_cancel (AX_hthread_t thr_id)
{
#ifdef WIN32 
	int status = 0;
	//unsupported
	//why not use TerminateThread() ?
	AX_SET_BITS(status,THR_STOPPED);
	return status;

#else	//linux
	int result = pthread_cancel (thr_id);
	return result==0 ? 0 : -1;
#endif
}
inline int AX_OS::rw_trywrlock_upgrade(AX_rwlock_t *rw)
{
#ifdef _WIN32
	int result = 0;
	if (AX_OS::thread_mutex_lock(&rw->lock_) == -1)
		return -1;
	// -1 means didn't get the mutex, error
	else if (rw->important_writer_)
		// an other reader upgrades already
	{
		result = -1;
#if !defined(WINCE)
		errno = EBUSY;
#endif//WINCE
	}
	else
	{
		while (rw->ref_count_ > 1) // wait until only I am left
		{
			rw->num_waiting_writers_++; // prohibit any more readers
			rw->important_writer_ = 1;

			if (AX_OS::cond_wait (&rw->waiting_important_writer_, &rw->lock_) == -1)
			{
				result = -1;
				// we know that we have the lock again, we have this guarantee,
				// but something went wrong
			}
			rw->important_writer_ = 0;
			rw->num_waiting_writers_--;
		}
		if (result == 0)
		{
			// nothing bad happend
			rw->ref_count_ = -1;
			// now I am a writer
			// everything is O.K.
		}
	}

	return AX_OS::thread_mutex_unlock(&rw->lock_);
#else
	return -1;
#endif
}
inline void AX_OS::thr_yield()
{
#ifdef _WIN32
	::Sleep (0);
#else
	pthread_yield ();
#endif
	return;
}
inline uint64 AX_OS::get_time()
{
#ifdef WIN32
	LARGE_INTEGER litmp; 
	LONGLONG QPart1;
	double dfFreq; 
	QueryPerformanceFrequency(&litmp);  // 获得计数器的时钟频率 
	dfFreq = (double)litmp.QuadPart; 
	QueryPerformanceCounter(&litmp);  
	QPart1 = litmp.QuadPart; 
	return (uint64)((QPart1 / dfFreq) * 1000);
#else  
	struct timeval tv = {0};
	gettimeofday(&tv, 0);
	uint64 retTime=tv.tv_sec*1000;
	retTime+=tv.tv_usec/1000;
	return retTime;
	//return times(&tm) * 10;
	//return (clock() / CLOCKS_PER_SEC * 100);
#endif
}
inline char*
AX_OS::itoa(int value,char *string,int radix)
{
#if defined(WINCE)
	int rt=0;
	if(string==NULL)
		return NULL;
	if(radix<=0||radix>30)
		return NULL;
	rt=sprintf(string,"%d",value);
	if(rt>radix)
		return NULL;
	string[rt]='\0';
	return string;
#elif defined(WIN32)
	return ::itoa(value, string, radix);
#else
	int rt=0;
	if(string==NULL)
		return NULL;
	if(radix<=0||radix>30)
		return NULL;
	rt=snprintf(string,radix,"%d",value);
	if(rt>radix)
		return NULL;
	string[rt]='\0';
	return string;
#endif
}
inline int AX_OS::thr_equal(AX_thread_t t1, AX_thread_t t2)
{
#ifdef _WIN32
	if (t2 == t1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
#else
	return pthread_equal(t1,t2);
#endif
}
inline int AX_OS::thr_cmp(AX_hthread_t t1, AX_hthread_t t2)
{
#ifdef _WIN32
	return t1 == t2 ?1:0;
#else
	return pthread_equal(t1,t2);
#endif
}
inline
int AX_OS::exit(int status /* = 0  */)
{
#ifdef _WIN32
	::ExitProcess(status);
#else
	exit(status);
	return 0;
#endif
}
inline int AX_OS::getpid()
{
#ifdef _WIN32
	return ::GetCurrentProcessId();
#else
	return getpid();
#endif
}
inline
int AX_OS::waitpid(AX_hthread_t process,int *exitCode,int options)
{
#ifdef _WIN32
	::GetExitCodeProcess(process,(LPDWORD)exitCode);
#else
	waitpid(process,exitCode,options);
#endif
	return 1;
}
#if !defined(WINCE)//WINCE下不支持 getenv,setenv 
inline
char* AX_OS::getenv(char* environment)
{
#ifdef _WIN32
	return ::getenv(environment);
#else
	return getenv(environment);
#endif
}

inline
int AX_OS::putenv(char *string)
{
#ifdef _WIN32
	return::putenv(string);
#else
	return putenv(string);
#endif
}
#endif//WINCE

inline
int AX_OS::kill(AX_hthread_t pid, int signum)
{
#ifdef _WIN32
	return ::TerminateProcess(pid,signum);
#else
	return kill(pid,0);
#endif

}

inline
int AX_OS::alarm(int seconds)
{
#ifndef _WIN32
	return alarm(0);
#endif
	return 1;
}

#if !defined(WINCE)
inline
int AX_OS::system(char* string)
{
#ifdef _WIN32
	return ::system(string);
#else
	return system(string);
#endif
}
#endif

inline unsigned long AX_OS::atoul(const char* string)
{
	return strtoul(string,NULL,10);
}

inline uint64 AX_OS::atoul64(const char* string)
{
#ifdef _WIN32
	return _strtoui64(string, NULL, 10);
#else
	return strtoull(string, NULL, 10);
#endif
}
