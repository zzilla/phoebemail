

/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_OS.cpp
* 文件标识：
* 摘　　要：操作系统适配层 .cpp文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/



#include <AX_OS.h>

//#include "AX_OS.inl"

int 
AX_OS::thr_create (AX_THR_FUNC func,
				void *args,
				long flags,
				AX_thread_t *thr_id,
				AX_hthread_t *thr_handle,
				long priority,
				void *stack,
				size_t stacksize)
{
	if (AX_BIT_DISABLED (flags, THR_DETACHED) &&
		AX_BIT_DISABLED (flags, THR_JOINABLE))
		AX_SET_BITS (flags, THR_JOINABLE);

#ifndef WIN32	//linux
	AX_thread_t tmp_thr;
	if (thr_id == 0)
		thr_id = &tmp_thr;
#endif

	AX_hthread_t tmp_handle;
	if (thr_handle == 0)
		thr_handle = &tmp_handle;

#ifdef WIN32
	int start_suspended = AX_BIT_ENABLED (flags, THR_SUSPENDED);
		if (priority != AX_DEFAULT_THREAD_PRIORITY)
			// If we need to set the priority, then we need to start the
			// thread in a suspended mode.
			AX_SET_BITS (flags, THR_SUSPENDED);

	*thr_handle= ::CreateThread
		(0,
		stacksize,
		LPTHREAD_START_ROUTINE (func),
		args,
		flags,
		thr_id);

	if (priority != AX_DEFAULT_THREAD_PRIORITY && *thr_handle != 0)
	{
		// Set the priority of the new thread and then let it
		// continue, but only if the user didn't start it suspended
		// in the first place!
		AX_OS::thr_setprio (*thr_handle, priority);

		if (start_suspended == 0)
			AX_OS::thr_continue (*thr_handle);
	}

	// Close down the handle if no one wants to use it.
	if (thr_handle == &tmp_handle &&(0 != tmp_handle))
		::CloseHandle (tmp_handle);

	if (thr_handle!= 0)
		return 0;
	else
		return -1;

#else	//linux
	int result = 0;

	pthread_attr_t attr = {0};
	if (::pthread_attr_init(&attr) != 0)
		return -1;

  // *** Set Stack Size
	if (stacksize != 0)
	{
		size_t size = stacksize;

		if (::pthread_attr_setstacksize (&attr, size) != 0)
		{
			::pthread_attr_destroy (&attr);
			return -1;
		}
	}

  // *** Set Stack Address
	if (stack != 0)
	{
		if (::pthread_attr_setstackaddr (&attr, stack) != 0)
		{
			::pthread_attr_destroy (&attr);
			return -1;
		}
	}

  // *** Deal with various attributes
	if (flags != 0)
	{
		// *** Set Detach state

		if (AX_BIT_ENABLED (flags, THR_DETACHED)
			|| AX_BIT_ENABLED (flags, THR_JOINABLE))
		{
			int dstate = PTHREAD_CREATE_JOINABLE;

			if (AX_BIT_ENABLED (flags, THR_DETACHED))
				dstate = PTHREAD_CREATE_DETACHED;

			if (::pthread_attr_setdetachstate (&attr, dstate) != 0)
			{
				::pthread_attr_destroy (&attr);
				return -1;
			}
		}

		// Note: if ACE_LACKS_SETDETACH and THR_DETACHED is enabled, we
		// call ::pthread_detach () below.  If THR_DETACHED is not
		// enabled, we call ::pthread_detach () in the Thread_Manager,
		// after joining with the thread.

		// *** Set Policy

		// If we wish to set the priority explicitly, we have to enable
		// explicit scheduling, and a policy, too.
		if (priority != AX_DEFAULT_THREAD_PRIORITY)
		{
			AX_SET_BITS (flags, THR_EXPLICIT_SCHED);
			if (AX_BIT_DISABLED (flags, THR_SCHED_FIFO)
				&& AX_BIT_DISABLED (flags, THR_SCHED_RR)
				&& AX_BIT_DISABLED (flags, THR_SCHED_DEFAULT))
				AX_SET_BITS (flags, THR_SCHED_DEFAULT);
		}

		if (AX_BIT_ENABLED (flags, THR_SCHED_FIFO)
			|| AX_BIT_ENABLED (flags, THR_SCHED_RR)
			|| AX_BIT_ENABLED (flags, THR_SCHED_DEFAULT))
		{
			int spolicy;

			AX_SET_BITS (flags, THR_EXPLICIT_SCHED);

			if (AX_BIT_ENABLED (flags, THR_SCHED_DEFAULT))
				spolicy = SCHED_OTHER;
			else if (AX_BIT_ENABLED (flags, THR_SCHED_FIFO))
				spolicy = SCHED_FIFO;
			else
				spolicy = SCHED_RR;

			result = ::pthread_attr_setschedpolicy (&attr, spolicy);

			if (result != 0)
			{
				::pthread_attr_destroy (&attr);
				return -1;
			}
		}

		// *** Set Priority (use reasonable default priorities)

		// If we wish to explicitly set a scheduling policy, we also
		// have to specify a priority.  We choose a "middle" priority as
		// default.  Maybe this is also necessary on other POSIX'ish
		// implementations?
		/*
		if ((AX_BIT_ENABLED (flags, THR_SCHED_FIFO)
			|| AX_BIT_ENABLED (flags, THR_SCHED_RR)
			|| AX_BIT_ENABLED (flags, THR_SCHED_DEFAULT))
			&& priority == AX_DEFAULT_THREAD_PRIORITY)
		{
			if (AX_BIT_ENABLED (flags, THR_SCHED_FIFO))
				priority = ACE_THR_PRI_FIFO_DEF;
			else if (AX_BIT_ENABLED (flags, THR_SCHED_RR))
				priority = ACE_THR_PRI_RR_DEF;
			else // THR_SCHED_DEFAULT
				priority = ACE_THR_PRI_OTHER_DEF;
		}
		*/
		/*
		if (priority != AX_DEFAULT_THREAD_PRIORITY)
		{
			struct sched_param sparam;
			memset ((void *) &sparam, 0, sizeof(sched_param));

			// The following code forces priority into range.
			if (AX_BIT_ENABLED (flags, THR_SCHED_FIFO))
				sparam.sched_priority =
				AX_MIN (ACE_THR_PRI_FIFO_MAX,
				AX_MAX (ACE_THR_PRI_FIFO_MIN, priority));
			else if (AX_BIT_ENABLED(flags, THR_SCHED_RR))
				sparam.sched_priority =
				AX_MIN (ACE_THR_PRI_RR_MAX,
				AX_MAX (ACE_THR_PRI_RR_MIN, priority));
			else // Default policy, whether set or not
				sparam.sched_priority =
				AX_MIN (ACE_THR_PRI_OTHER_MAX,
				AX_MAX (ACE_THR_PRI_OTHER_MIN, priority));

			result = ::pthread_attr_setschedparam (&attr, &sparam)
			if (result != 0)
			{
				::pthread_attr_destroy (&attr);
				return -1;
			}
		}
		*/

		// *** Set scheduling explicit or inherited
		/*
		if (AX_BIT_ENABLED (flags, THR_INHERIT_SCHED)
			|| AX_BIT_ENABLED (flags, THR_EXPLICIT_SCHED))
		{
			int sched = PTHREAD_DEFAULT_SCHED;

			if (AX_BIT_ENABLED (flags, THR_INHERIT_SCHED))
				sched = PTHREAD_INHERIT_SCHED;
			if (::pthread_attr_setinheritsched (&attr, sched) != 0)
			{
				::pthread_attr_destroy (&attr);
				return -1;
			}
		}
		*/

		// *** Set Scope
		/*
		if (AX_BIT_ENABLED (flags, THR_SCOPE_SYSTEM)
			|| AX_BIT_ENABLED (flags, THR_SCOPE_PROCESS))
		{
			// LinuxThreads do not have support for PTHREAD_SCOPE_PROCESS.
			// Neither does HPUX (up to HP-UX 11.00, as far as I know).
			int scope = PTHREAD_SCOPE_SYSTEM;

			if (AX_BIT_ENABLED (flags, THR_SCOPE_SYSTEM))
				scope = PTHREAD_SCOPE_SYSTEM;

			if (::pthread_attr_setscope (&attr, scope) != 0)
			{
				::pthread_attr_destroy (&attr);
				return -1;
			}
		}
		*/

		// *** start suspended?
		/*
		if (AX_BIT_ENABLED (flags, THR_SUSPENDED))
		{
			if (::pthread_attr_setcreatesuspend_np(&attr) != 0)
			{
				::pthread_attr_destroy (&attr);
				return -1;
			}
		}
		*/

		if (AX_BIT_ENABLED (flags, THR_NEW_LWP))
		{
			//noop
		}
	}

	if (::pthread_create (thr_id, &attr, func, args) == -1 )
		return -1;

	::pthread_attr_destroy (&attr);

	if (result != -1)
	{
		*thr_handle = *thr_id;
	}
	return result;
#endif
}

void 
AX_OS::thr_exit (AX_thr_func_return status)
{
#ifdef WIN32
	// no op?
	::ExitThread(status);
	return ;
#else	//linux
	::pthread_exit (status);
#endif
}

int AX_atoi(const char *num)
{
	int sum=0;
#ifdef _WIN32
	sum=atoi(num);
#else
	int temp;
	int i=0;
	while(num[i]!='\0')
	{
		if (num[i]>'9'||num[i]<'0')
		{
			break;
		}
		temp=num[i]-'0';
		sum=sum*10+temp;
		i++;
	}

#endif	
	return sum;
}

char* AX_itoa(int a,char* temp,int length)
{
#if defined(WINCE)
	int i=0;
	int b=a;
	while(b>0)
	{
		i++;
		b=b/10;
	}
	temp[i]='\0';
	while(a>0&&i<length)
	{
		temp[i-1]=a%10+'0';
		i--;
		a=a/10;
	}
	return temp;
#elif defined(WIN32)
	return itoa(a,temp,length);
#else
	int i=0;
	int b=a;
	while(b>0)
	{
		i++;
		b=b/10;
	}
	temp[i]='\0';
	while(a>0&&i<length)
	{
		temp[i-1]=a%10+'0';
		i--;
		a=a/10;
	}
	return temp;
#endif
}

int AX_OS::thr_key_create(AX_thread_key_t *key,AX_THR_DEST func /* = NULL */)
{
	int index = -1;
#ifdef _WIN32
	index = TlsAlloc();
	*key = index;
#else
	if (key != NULL)
	{
		index = pthread_key_create(key,func);
		*key = index;
	}
#endif
	return index;
}

void* AX_OS::thr_key_get(AX_thread_key_t key)
{
#ifdef _WIN32
	return TlsGetValue(key);
#else
	return pthread_getspecific(key);
#endif
}
int AX_OS::thr_key_set(AX_thread_key_t key,const void *point /* = NULL */)
{
	int result;
#ifdef _WIN32
	result = TlsSetValue(key,(LPVOID)point);
#else
	result =  pthread_setspecific(key,point);
#endif
	return result;
}
int AX_OS::thr_keyfree(AX_thread_key_t key)
{
#ifdef _WIN32
	return TlsFree(key);
#else
	return pthread_key_delete(key);
#endif
}
