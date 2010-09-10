
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_RW_Thread_Mutex.cpp
* 文件标识：
* 摘　　要：AX读写线程锁 .cpp文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月20日
* 修订记录：创建
*/

#include <AX_RW_Thread_Mutex.h>

//#include "AX_RW_Thread_Mutex.inl"


AX_RW_Thread_Mutex::AX_RW_Thread_Mutex (const char *name,	void *arg)
:
AX_RW_Mutex (AX_USYNC_THREAD, name, arg)
{
	// ACE_TRACE ("ACE_RW_Thread_Mutex::ACE_RW_Thread_Mutex");
}

