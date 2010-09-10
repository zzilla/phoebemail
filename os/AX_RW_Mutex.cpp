
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_RW_Mutex.cpp
* 文件标识：
* 摘　　要：AX读写锁 .cpp文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/

#include <AX_RW_Mutex.h>

//#include "AX_RW_Mutex.inl"


AX_RW_Mutex::AX_RW_Mutex(int type, const char *name, void *arg)
 :
process_lock_ (0),
removed_ (0)
{
	if (AX_OS::rwlock_init (&this->lock_, type, name, arg) != 0)
		//error
		//noop
		return ;
}

AX_RW_Mutex::~AX_RW_Mutex(void)
{
	this->remove();
}

