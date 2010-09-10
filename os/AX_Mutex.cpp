
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Mutex.cpp
* 文件标识：
* 摘　　要：AX互斥类 .cpp文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/

//#include "../inlcude/AX_Mutex.h"
#include <AX_Mutex.h>

//#include "AX_Mutex.inl"


AX_Mutex::AX_Mutex(int type)
 :
process_lock_ (0),
removed_ (0)
{
	// ACE_TRACE ("ACE_Mutex::ACE_Mutex");
#ifdef AX_HAS_PROCESS_MUTEX
	if (type == USYNC_PROCESS)
	{
		//process mutex, to be implemented
	}
	else
	{
#endif
		if (AX_OS::thread_mutex_init(&this->lock_, type) != 0)
		{
			//error
			//TRACE ..
		}
#ifdef AX_HAS_PROCESS_MUTEX
	}
#endif
}

AX_Mutex::~AX_Mutex(void)
{
	this->remove();
}

