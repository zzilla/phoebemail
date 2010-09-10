
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Thread.inl
* 文件标识：
* 摘　　要：AX线程类 .inl文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月19日
* 修订记录：创建
*/
inline AX_Thread::AX_Thread()
{
//#ifndef _WIN32
//	AX_OS::thread_mutex_init(&mutex);
//#endif
}
inline int 
AX_Thread::spawn (AX_THR_FUNC func,
				  void *arg,
				  long flags,
				  AX_thread_t *t_id,
				  AX_hthread_t *t_handle,
				  long priority,
				  void *stack,
				  size_t stack_size)
{
	return AX_OS::thr_create (func,
		arg,
		flags,
		t_id,
		t_handle,
		priority,
		stack,
		stack_size);
}
#if 0//不使用这个函数，兼容性不好
inline int 
AX_Thread::join (AX_thread_t wait_for,
				 AX_thread_t *departed,
				 AX_thr_func_return *status)
{
	return AX_OS::thr_join (wait_for, departed, status);
}
#endif//0

inline int 
AX_Thread::join (AX_hthread_t wait_for, AX_thr_func_return * status)
{
	return AX_OS::thr_join (wait_for, status);
}


inline int 
AX_Thread::resume (AX_hthread_t t_id)
{
#ifndef _WIN32
	//AX_OS::thread_mutex_unlock(&mutex);
#endif
	return AX_OS::thr_continue(t_id);
}

inline int 
AX_Thread::suspend (AX_hthread_t t_id)
{
#ifndef _WIN32
	//AX_OS::thread_mutex_lock(&mutex,NULL);
#endif
	return AX_OS::thr_suspend(t_id);
}

#if !defined(WINCE)//Windows CE下面不支持 GetThreadPriority,GetPriorityClass，所以禁用这3个函数
inline int 
AX_Thread::getprio (AX_hthread_t ht_id, int &priority)
{
	return AX_OS::thr_getprio (ht_id, priority);
}

inline int 
AX_Thread::getprio (AX_hthread_t ht_id, int &priority, int &policy)
{
	return AX_OS::thr_getprio(ht_id, priority, policy);
}

inline int 
AX_Thread::setprio (AX_hthread_t ht_id, int priority, int policy)
{
	return AX_OS::thr_setprio (ht_id, priority, policy);
}
#endif//WINCE

inline int 
AX_Thread::kill (AX_hthread_t t_id, int signum)
{
	return AX_OS::thr_kill (t_id, signum);
}

inline void 
AX_Thread::self (AX_hthread_t &t_handle)
{
	AX_OS::thr_self (t_handle);
}

inline AX_thread_t 
AX_Thread::self (void)
{
	return AX_OS::thr_self ();
}

inline int AX_Thread::keycreate(AX_thread_key_t *keyp, AX_THR_DEST destructor, void *arg)
{
	return AX_OS::thr_key_create(keyp,destructor);
}
inline int AX_Thread::keyfree(AX_thread_key_t key)
{
	return AX_OS::thr_keyfree(key);
}
inline int AX_Thread::setspecific(AX_thread_key_t key,void *value)
{
	return AX_OS::thr_key_set(key,value);
}
inline int
AX_Thread::getspecific (AX_thread_key_t key, void **valuep)
{
	// ACE_TRACE ("ACE_Thread::getspecific");
	*valuep  = AX_OS::thr_key_get(key);
	return 1;
}
inline void AX_Thread::yield()
{
	AX_Thread::yield();
}
