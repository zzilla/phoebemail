
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Thread_Mutex.cpp
* 文件标识：
* 摘　　要：AX线程互斥类 .cpp文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月20日
* 修订记录：创建
*/

#include <AX_Thread_Mutex.h>

//#include "AX_Thread_Mutex.inl"


AX_Thread_Mutex::AX_Thread_Mutex()
 :
removed_ (0)
{
	int result = AX_OS::thread_mutex_init(&this->lock_, 0);
	//printf("after AX_Thread_Mutex::AX_Thread_Mutex result:%d\n",result);
}

AX_Thread_Mutex::~AX_Thread_Mutex(void)
{
	this->remove();
}

