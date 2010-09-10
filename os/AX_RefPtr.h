#ifndef AX_SPREF_H_INCLUDED
#define AX_SPREF_H_INCLUDED

#include "platform.h"

template<class T>
class AX_RefPtr
{
	typedef AX_RefPtr<T> TYPE;
public:
	AX_RefPtr(T* t=NULL)
	{
		_p=t;
		if(NULL!=_p)
			_p->addRef();
	}
	~AX_RefPtr()
	{
		if(NULL!=_p)
		{
			_p->release();
			_p=NULL;
		}
	}
	AX_RefPtr(const TYPE& ptr)
	{
		_p=ptr._p;
		if(NULL!=_p)
			_p->addRef();
	}
public:
	AX_RefPtr& operator=(const TYPE& ptr)
	{
		if(NULL!=_p)
			_p->release();
		_p=ptr._p;
		if(NULL!=_p)
			_p->addRef();
		return *this;
	}
	AX_RefPtr& operator=(T* t)
	{
		if(NULL!=_p)
			_p->release();
		_p=t;
		if(NULL!=_p)
			_p->addRef();
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
	const bool isNull() const
	{
		return NULL==_p;
	}
protected:
	T*	_p;
	template<class C> friend bool operator ==( const C* t,const AX_RefPtr<C>& ptr);
	template<class C> friend bool operator ==( const AX_RefPtr<C>& ptr, const C* t);
	template<class C> friend bool operator ==( const AX_RefPtr<C>& ptr1, const AX_RefPtr<C>& ptr2);
	template<class C> friend bool operator !=( const C* t,const AX_RefPtr<C>& ptr);
	template<class C> friend bool operator !=( const AX_RefPtr<C>& ptr, const C* t);
	template<class C> friend bool operator !=( const AX_RefPtr<C>& ptr1, const AX_RefPtr<C>& ptr2);
};

template<class T>
bool operator ==(  const T* t,const AX_RefPtr<T>& ptr)
{
	return ptr._p==t;
}
template<class T>
bool operator ==( const AX_RefPtr<T>& ptr, const T* t)
{
	return ptr._p==t;
}
template<class T>
bool operator ==( const AX_RefPtr<T>& ptr1, const AX_RefPtr<T>& ptr2)
{
	return ptr1._p==ptr2._p;
}

template<class T>
bool operator !=(  const T* t,const AX_RefPtr<T>& ptr)
{
	return ptr._p!=t;
}

template<class T>
bool operator !=( const AX_RefPtr<T>& ptr, const T* t)
{
	return ptr._p!=t;
}

template<class T>
bool operator !=( const AX_RefPtr<T>& ptr1, const AX_RefPtr<T>& ptr2)
{
	return ptr1._p!=ptr2._p;
}

#endif//AX_SPREF_H_INCLUDED
