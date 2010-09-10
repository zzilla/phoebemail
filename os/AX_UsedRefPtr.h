#ifndef AX_SS_USED_REF_PTR_H_INCLUDED
#define AX_SS_USED_REF_PTR_H_INCLUDED

#include "platform.h"

template<class T>
class AX_UsedRefPtr
{
	typedef AX_UsedRefPtr<T> TYPE;
public:
	AX_UsedRefPtr(T* t=NULL)
	{
		_p=t;
		if(NULL!=_p)
		{
			_p->addUsedRef();
			_p->addRef();
		}
	}
	~AX_UsedRefPtr()
	{
		if(NULL!=_p)
		{
			_p->releaseUsedRef();
			_p->release();
			_p=NULL;
		}
	}
	AX_UsedRefPtr(const TYPE& ptr)
	{
		_p=ptr._p;
		if(NULL!=_p)
		{
			_p->addUsedRef();
			_p->addRef();
		}
	}
public:
	AX_UsedRefPtr& operator=(const TYPE& ptr)
	{
		if(NULL!=_p)
		{
			_p->releaseUsedRef();
			_p->release();
		}
		_p=ptr._p;
		if(NULL!=_p)
		{
			_p->addUsedRef();
			_p->addRef();
		}
		return *this;
	}
	AX_UsedRefPtr& operator=(T* t)
	{
		if(NULL!=_p)
		{
			_p->releaseUsedRef();
			_p->release();
		}
		_p=t;
		if(NULL!=_p)
		{
			_p->addUsedRef();
			_p->addRef();
		}
		return *this;
	}
public:
	T* operator->()
	{
		return _p;
	}
	const T* operator->()const
	{
		return _p;
	}
	bool isNull()
	{
		return NULL==_p;
	}
protected:
	T*	_p;
	template<class C> friend bool operator ==( const C* t,const AX_UsedRefPtr<C>& ptr);
	template<class C> friend bool operator ==( const AX_UsedRefPtr<C>& ptr, const C* t);
	template<class C> friend bool operator ==( const AX_UsedRefPtr<C>& ptr1, const AX_UsedRefPtr<C>& ptr2);
	template<class C> friend bool operator !=( const C* t,const AX_UsedRefPtr<C>& ptr);
	template<class C> friend bool operator !=( const AX_UsedRefPtr<C>& ptr, const C* t);
	template<class C> friend bool operator !=( const AX_UsedRefPtr<C>& ptr1, const AX_UsedRefPtr<C>& ptr2);
};

template<class T>
bool operator ==(  const T* t,const AX_UsedRefPtr<T>& ptr)
{
	return ptr._p==t;
}
template<class T>
bool operator ==( const AX_UsedRefPtr<T>& ptr, const T* t)
{
	return ptr._p==t;
}
template<class T>
bool operator ==( const AX_UsedRefPtr<T>& ptr1, const AX_UsedRefPtr<T>& ptr2)
{
	return ptr1._p==ptr2._p;
}

template<class T>
bool operator !=(  const T* t,const AX_UsedRefPtr<T>& ptr)
{
	return ptr._p!=t;
}

template<class T>
bool operator !=( const AX_UsedRefPtr<T>& ptr, const T* t)
{
	return ptr._p!=t;
}

template<class T>
bool operator !=( const AX_UsedRefPtr<T>& ptr1, const AX_UsedRefPtr<T>& ptr2)
{
	return ptr1._p!=ptr2._p;
}

#endif//AX_SS_USED_REF_PTR_H_INCLUDED
