
#ifndef NVCM_REFERABLE_H
#define NVCM_REFERABLE_H

#include "Global.h"
#include "atomiccount.h"

NAMESPACE_BEGIN(SPTools)

class CBaseReferablePtr;

class IReferable
{
protected:
	//ensure not be alloced from stack, or refer crashed
	IReferable(){}
	//it's neccessory when "delete this" in Release()
	virtual ~IReferable(){}
	//not support copy & copy constuctor
	IReferable& operator =( const IReferable& );
	IReferable( const IReferable& );
	
	//should not be called by anyone but CReferablePtr
	friend class CBaseReferablePtr;
protected:
	void AddRef(){mRefCount.ref();}
	//decrease the refer count, delete self automatically when at last one.
	void Release()
	{
		if (mRefCount.deref()) { 
			delete this;
		}
	}
	
	//can only be used by myself
private:
	AtomicCount mRefCount;
};

/*/////////////////////////////////////////////////////////////////////
CReferablePtr
use this class to use class IReferable, simlar as std::auto_ptr
  you can use the class as a pointer of class IReferable usually.
  CBaseReferablePtr is used to be a friend class of IReferable

though there are class derieved from CReferablePtr, the methods
  needn't be virtual. because the derieved class is a simple wrapper
/////////////////////////////////////////////////////////////////////*/
class CBaseReferablePtr
{
protected:
	IReferable* mRefPtr;

public:
	//be care that here do AddRef() once
	CBaseReferablePtr( IReferable* RefPtr = NULL )
		: mRefPtr( RefPtr )
	{
		if( mRefPtr ) mRefPtr->AddRef();
	};
	//auto matically release the pointer
	//care that it's not virtual, so the derived class shouldn't have destructor
	~CBaseReferablePtr()
	{
		if( mRefPtr ) mRefPtr->Release();
	}

	CBaseReferablePtr( const CBaseReferablePtr& RefPtr )
		: mRefPtr( RefPtr.mRefPtr )
	{
		if( mRefPtr ) mRefPtr->AddRef();
	}

	CBaseReferablePtr& operator =( IReferable* RefPtr )
	{
		if(mRefPtr!=RefPtr){
			if(mRefPtr) mRefPtr->Release();
			if((mRefPtr=RefPtr)!=0) mRefPtr->AddRef();
		}
		return *this;
	}

	CBaseReferablePtr& operator =( const CBaseReferablePtr& RefPtr )
	{
		return operator=(RefPtr.mRefPtr);
	}
	
	bool IsEmpty() const
	{
		return mRefPtr == NULL;
	}

	IReferable* operator ->()
	{
		//mustn't be null here
		//assert( mRefPtr );
		return mRefPtr;
	}

	const IReferable* operator ->() const
	{
		//mustn't be null here
		//assert( mRefPtr );
		return mRefPtr;
	}
};

//warpper of CBaseReferablePtr, add template function
//refer to CBaseReferablePtr
//as you see, in fact, CReferablePtr is very similar as CBaseReferablePtr,
//   just some like void *
template<typename X> class CReferable;

template <typename X>
class CReferablePtr : public CBaseReferablePtr
{
public:
	//(IReferable*) can't remove, it's some strange
	CReferablePtr<X>( CReferable<X>* pRef = NULL ) 
		: CBaseReferablePtr( pRef ) {}

	CReferablePtr<X>( const CReferablePtr<X>& RefPtr ) 
		: CBaseReferablePtr( RefPtr ) {}
	
	CReferablePtr<X>& operator =( CReferable<X>* pRef )
	{ 
		return (CReferablePtr<X>&)(CBaseReferablePtr::operator =(pRef)); 
	}

	CReferablePtr<X>& operator =( const CReferablePtr<X>& RefPtr )
	{ 
		return (CReferablePtr<X>&)(CBaseReferablePtr::operator=(RefPtr)); 
	}

	X* operator ->()
	{
		//assert( mRefPtr ); 
		return (X*)( *(CReferable<X>*)mRefPtr ); 
	}

	const X* operator ->() const
	{ 
		//assert( mRefPtr ); 
		return (X*)( *(CReferable<X>*)mRefPtr );
	}
};

//if your class (X) have implement (derived from) IReferable,
//you can use this wrapper to do
template<typename X>
class CReferableObj : public CBaseReferablePtr
{
public:
	CReferableObj<X>( X* pRef = NULL ) : CBaseReferablePtr( pRef ) {}

	CReferableObj<X>( const CReferableObj<X>& RefPtr ) 
		: CBaseReferablePtr( RefPtr ) {}

	CReferableObj<X>& operator =( X* pRef )
	{ 
		return ( CReferableObj<X>& ) ( CBaseReferablePtr::operator =( pRef ) ); 
	}

	CReferableObj<X>& operator =( const CReferablePtr<X>& RefPtr )
	{ 
		return ( CReferableObj<X>& ) ( CBaseReferablePtr::operator =( RefPtr ) ); 
	}

	X* operator ->()
	{ 
		//assert( mRefPtr ); 
		return (X*)mRefPtr; 
	}
	const X* operator ->() const
	{ 
		//assert( mRefPtr ); 
		return (X*)mRefPtr; 
	}
};

/*/////////////////////////////////////////////////////////////////////
CReferable: template
this is a simple class to wrapper your class to realize referable
if your class can derived from IReferable byself, this class is useless
/////////////////////////////////////////////////////////////////////*/
template<typename X>
class CReferable : public IReferable
{
protected:
	//ensure not be alloced from stack, or refer crashed
	//you should construct this object after you have create an X object.
	CReferable<X>( X* DataPtr )	: m_DataPtr( DataPtr )
	{
		//Refer to NULL pointer is useless, but harmless, just let it be.
		//ASSERT( DataPtr );
	}

	~CReferable<X>()
	{
		//delete NULL pointer is harmless
		delete m_DataPtr;
	}
public:
	//instead of the constructor
	static CReferable<X>* CreateObject(  X* DataPtr )
	{
		return new CReferable<X>( DataPtr );
	}

	X* operator ->()
	{
		return m_DataPtr;
	}
	
	const X* operator ->() const
	{
		return m_DataPtr;
	}
	
protected:
	X* m_DataPtr;

};

NAMESPACE_END // End of namespace SPTools
	
#endif // NVCM_REFERABLE_H

