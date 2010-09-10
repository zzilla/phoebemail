/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_IAddRefAble.inl
* 文件标识：
* 摘要：计数指针，适用于多线程指针安全传递。
*
* 历史：创建，2006年7月26日，李明江(li_mingjiang@dahuatech.com)
*		整理，2008年5月21日，林坚彦(linjy@anxiesoft.cn)
*		重写，2008年6月11日，林坚彦(linjy@anxiesoft.cn)
*
*/

inline AX_IAddRefAble::AX_IAddRefAble()
{
	_ref = 0;
}

inline AX_IAddRefAble::~AX_IAddRefAble()
{
}

inline int
AX_IAddRefAble::addRef(void)
{
	_mutex.acquire();
	int ret = (++_ref);
	_mutex.release();

	return ret;
}

inline int
AX_IAddRefAble::release(void)
{
	_mutex.acquire();
	int ret = (--_ref);
	_mutex.release();

	//if ret < 0, the user program must check why. and prevent it.
	if (ret <= 0)
	{
		delete this;
		return 0;
	}

	return ret;
}

/* linux下的atomic_t系列函数似乎不能保证线程同步，重新实现
inline AX_IAddRefAble::AX_IAddRefAble()
{
#ifdef WIN32
	_ref = 0;
#else
	_ref.counter = 0;
#endif
}

inline AX_IAddRefAble::~AX_IAddRefAble()
{

}

inline int
AX_IAddRefAble::addRef(void)
{
#ifdef WIN32
	return InterlockedIncrement(&_ref);
#else
	return atomic_inc_return(&_ref);
#endif
}

inline int
AX_IAddRefAble::release(void)
{
#ifdef WIN32
	if (InterlockedDecrement(&_ref) <= 0)
	{
		delete this;
		return 0;
	}

	return _ref;
#else
	if(atomic_dec_return(&_ref) <= 0)
	{
		delete this;
		return 0;
	}

	return _ref.counter;
#endif
}
*/