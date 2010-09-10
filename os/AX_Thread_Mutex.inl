
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Thread_Mutex.inl
* 文件标识：
* 摘　　要：AX线程互斥类 .inl文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月20日
* 修订记录：创建
*/

inline int 
AX_Thread_Mutex::remove (void)
{
	int result = 0;

	if (this->removed_ == 0)
	{
		this->removed_ = 1;
		result = AX_OS::thread_mutex_destroy (&this->lock_);
	}

	return result;
}

inline int 
AX_Thread_Mutex::acquire (void)
{
	return AX_OS::thread_mutex_lock (&this->lock_);
}

inline int 
AX_Thread_Mutex::acquire (timeval &tv)
{
	return AX_OS::thread_mutex_lock (&this->lock_, tv);
}

inline int 
AX_Thread_Mutex::acquire (timeval *tv)
{
	return AX_OS::thread_mutex_lock (&this->lock_, tv);
}

inline int 
AX_Thread_Mutex::tryacquire (void)
{
	return AX_OS::thread_mutex_trylock (&this->lock_);
}

inline int 
AX_Thread_Mutex::release (void)
{
	return AX_OS::thread_mutex_unlock (&this->lock_);
}

inline int 
AX_Thread_Mutex::acquire_read (void)
{
	return AX_OS::thread_mutex_lock (&this->lock_);
}

inline int 
AX_Thread_Mutex::acquire_write (void)
{
	return AX_OS::thread_mutex_lock (&this->lock_);
}

inline int 
AX_Thread_Mutex::tryacquire_read (void)
{
	return AX_OS::thread_mutex_trylock (&this->lock_);
}

inline int 
AX_Thread_Mutex::tryacquire_write (void)
{
	return AX_OS::thread_mutex_trylock (&this->lock_);
}


