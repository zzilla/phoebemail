
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Mutex.inl
* 文件标识：
* 摘　　要：AX互斥类 .inl文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/

inline int 
AX_Mutex::remove (void)
{
	int result = 0;

#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
	{
		//process mutex, to be implemented
	}
	else
	{
#endif
		if (this->removed_ == 0)
		{
			this->removed_ = 1;
			result = AX_OS::thread_mutex_destroy (&this->lock_);
		}
#ifdef AX_HAS_PROCESS_MUTEX
	}
#endif

	return result;
}

inline int 
AX_Mutex::acquire (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_lock (&this->lock_);
}

inline int 
AX_Mutex::acquire (timeval &tv)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_lock (&this->lock_, tv);
}

inline int 
AX_Mutex::acquire (timeval *tv)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_lock (&this->lock_, tv);
}

inline int 
AX_Mutex::tryacquire (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_trylock (&this->lock_);
}

inline int 
AX_Mutex::release (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_unlock (&this->lock_);
}

inline int 
AX_Mutex::acquire_read (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_lock (&this->lock_);
}

inline int 
AX_Mutex::acquire_write (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_lock (&this->lock_);
}

inline int 
AX_Mutex::tryacquire_read (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_trylock (&this->lock_);
}

inline int 
AX_Mutex::tryacquire_write (void)
{
#ifdef AX_HAS_PROCESS_MUTEX
	if (this->process_lock_)
		return -1;
#endif
	return AX_OS::thread_mutex_trylock (&this->lock_);
}


