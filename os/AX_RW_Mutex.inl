
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_RW_Mutex.inl
* 文件标识：
* 摘　　要：AX读写锁 .inl文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/

inline int 
AX_RW_Mutex::remove (void)
{
	// AX_TRACE ("AX_RW_Mutex::remove");
	int result = 0;
	if (this->removed_ == 0)
	{
		this->removed_ = 1;
		result = AX_OS::rwlock_destroy (&this->lock_);
	}
	return result;
}

inline int 
AX_RW_Mutex::acquire_read (void)
{
	// AX_TRACE ("AX_RW_Mutex::acquire_read");
	return AX_OS::rw_rdlock (&this->lock_);
}


inline int 
AX_RW_Mutex::acquire_write (void)
{
	// AX_TRACE ("AX_RW_Mutex::acquire_write");
	return AX_OS::rw_wrlock (&this->lock_);
}


inline int 
AX_RW_Mutex::tryacquire_read (void)
{
	// AX_TRACE ("AX_RW_Mutex::tryacquire_read");
	return AX_OS::rw_tryrdlock (&this->lock_);
}


inline int 
AX_RW_Mutex::tryacquire_write (void)
{
	// AX_TRACE ("AX_RW_Mutex::tryacquire_write");
	return AX_OS::rw_trywrlock (&this->lock_);
}


inline int 
AX_RW_Mutex::acquire (void)
{
	// AX_TRACE ("AX_RW_Mutex::acquire");
	return AX_OS::rw_wrlock (&this->lock_);
}


inline int 
AX_RW_Mutex::tryacquire (void)
{
	// AX_TRACE ("AX_RW_Mutex::tryacquire");
	return this->tryacquire_write ();
}


inline int 
AX_RW_Mutex::release (void)
{
	// AX_TRACE ("AX_RW_Mutex::release");
	return AX_OS::rw_unlock (&this->lock_);
}


