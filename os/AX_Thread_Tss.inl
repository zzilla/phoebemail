#include "AX_Thread_Tss.h"

template <class TYPE> inline
AX_TSS<TYPE>::~AX_TSS (void)
{
#ifdef _WIN32
	cleanup(0);
#endif
	if (this->once_ != 0)
	{
		//AX_OS::thr_key_detach (this->key_, this);
		AX_OS::thr_keyfree (this->key_);
	}
}

template <class TYPE>inline
AX_TSS<TYPE>::AX_TSS (TYPE *ts_obj)
: once_ (0)
{
	ts_init();

	AX_OS::thr_key_set(this->key_,(void *) ts_obj);
		//printf("AX_Thread::setspecific ");

}

template <class TYPE> inline int
AX_TSS<TYPE>::ts_init (void) 
{
	int result = AX_OS::thr_key_create(&key_,&cleanup);
	return result;
}

template <class TYPE> inline TYPE *
AX_TSS<TYPE>::ts_object (void) const
{
	return (TYPE*)AX_OS::thr_key_get(key_);
}

template <class TYPE> inline int 
AX_TSS<TYPE>::ts_object (TYPE *type)
{
	return AX_OS::thr_key_set(key_,(void *)type);
}

template <class TYPE> inline TYPE *
AX_TSS<TYPE>::ts_get (void) const
{
	return AX_OS::thr_key_get(key_);
}

template <class TYPE> inline TYPE *
AX_TSS<TYPE>::make_TSS_TYPE (void) const
{
	TYPE *temp = 0;

	temp = new TYPE;
	return temp;
}

template <class TYPE> inline void
AX_TSS<TYPE>::dump (void) const
{
	this->keylock_.dump ();
}

template <class TYPE> inline void
AX_TSS<TYPE>::cleanup (void *ptr)
{
	//printf("cleanup ");
	// Cast this to the concrete TYPE * so the destructor gets called.
	//delete (TYPE *) ptr;
}
