/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Thread_Mutex.h
* 文件标识：
* 摘　　要：AX线程互斥类 .h文件
*

* 当前版本：1.0
* 原作者　：YEYM
* 完成日期：2008年6月23日
* 修订记录：创建
* 完成线程局部存储区域
*/
#ifndef AX_THREAD_TSS
#define AX_THREAD_TSS

#include "AX_OS.h"
#include "AX_Mutex.h"
#include "AX_Thread_Mutex.h"
///////////////////////////（WINDWOS）调试通过
template <class TYPE>
class AX_TSS
{
public:
	// = Initialization and termination methods.

	/**
	* If caller has passed us a non-NULL ts_obj *, then we'll just use
	* this to initialize the thread-specific value (but only for the
	* calling thread).  Thus, subsequent calls to <operator->> in this
	* thread will return this value.  This is useful since it enables
	* us to assign objects to thread-specific data that have
	* arbitrarily complex constructors.
	*
	*/
	AX_TSS (TYPE *ts_obj = 0);

	/// Deregister with thread-key administration.
	virtual ~AX_TSS (void);

	// = Accessors.

	/**
	* Get the thread-specific object for the key associated with this
	* object.  Returns 0 if the data has never been initialized,
	* otherwise returns a pointer to the data.
	*/
	TYPE *ts_object (void) const;

	/// Set the thread-specific object for the key associated with this
	/// object.
	int ts_object (TYPE *);

	/// Use a "smart pointer" to get the thread-specific object
	/// associated with the <key_>.
	TYPE *operator-> () const;

	/// Return or create and return the calling threads TYPE object.
	operator TYPE *(void) const;

	/// Hook for construction parameters.
	virtual TYPE *make_TSS_TYPE (void) const;

	// = Utility methods.

	/// Dump the state of an object.
	void dump (void) const;

	// ACE_ALLOC_HOOK_DECLARE;
	// Declare the dynamic allocation hooks.

protected:
	/// Actually implements the code that retrieves the object from
	/// thread-specific storage.
	TYPE *ts_get (void) const;

	/// Factors out common code for initializing TSS.  This must NOT be
	/// called with the lock held...
	int ts_init (void);

	/// Avoid race conditions during initialization.
	AX_Thread_Mutex keylock_;

	/// "First time in" flag.
	int once_;

	/// Key for the thread-specific error data.
	AX_thread_key_t key_;

	/// "Destructor" that deletes internal TYPE * when thread exits.
	static void cleanup (void *ptr);
private:
	void operator= (const AX_TSS<TYPE> &);
	AX_TSS (const AX_TSS<TYPE> &);
};

template <class TYPE>
class ACE_TSS_Type_Adapter
{
public:
	/// Constructor.  Inlined here so that it should _always_ be inlined.
	ACE_TSS_Type_Adapter (const TYPE value = 0): value_ (value) {}

	/// TYPE conversion.  Inlined here so that it should _always_ be
	/// inlined.
	operator TYPE () const { return value_; };

	/// TYPE & conversion.  Inlined here so that it should _always_ be
	/// inlined.
	operator TYPE &() { return value_; };

private:
	/// The wrapped value.
	TYPE value_;
};

#include "AX_Thread_Tss.inl"

#endif
