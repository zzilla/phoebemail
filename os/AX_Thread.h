
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_Thread.h
* 文件标识：
* 摘　　要：AX线程类 .h文件
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月19日
* 修订记录：创建
*/

#ifndef AX_THREAD_INCLUDE
#define AX_THREAD_INCLUDE

#include "AX_OS.h"
//#include "AX_Thread_Adapter.h"
enum
{
	/// Uninitialized.
	AX_THR_IDLE = 0x00000000,

	/// Created but not yet running.
	AX_THR_SPAWNED = 0x00000001,

	/// Thread is active (naturally, we don't know if it's actually
	/// *running* because we aren't the scheduler...).
	AX_THR_RUNNING = 0x00000002,

	/// Thread is suspended.
	AX_THR_SUSPENDED = 0x00000004,

	/// Thread has been cancelled (which is an indiction that it needs to
	/// terminate...).
	AX_THR_CANCELLED = 0x00000008,

	/// Thread has shutdown, but the slot in the thread manager hasn't
	/// been reclaimed yet.
	AX_THR_TERMINATED = 0x00000010,

	/// Join operation has been invoked on the thread by thread manager.
	AX_THR_JOINING = 0x10000000
};
class AX_Thread
{

public:
	/**
	* Creates a new thread having <flags> attributes and running <func> with <args>
	*.<thr_id> and <t_handle> are set to the thread's ID and handle (?),
	* respectively.  The thread runs at <priority> priority (see
	* below).
	*
	* The <flags> are a bitwise-OR of the following:
	* = BEGIN<INDENT>
	* THR_CANCEL_DISABLE, THR_CANCEL_ENABLE, THR_CANCEL_DEFERRED,
	* THR_CANCEL_ASYNCHRONOUS, THR_BOUND, THR_NEW_LWP, THR_DETACHED,
	* THR_SUSPENDED, THR_DAEMON, THR_JOINABLE, THR_SCHED_FIFO,
	* THR_SCHED_RR, THR_SCHED_DEFAULT, THR_EXPLICIT_SCHED,
	* THR_SCOPE_SYSTEM, THR_SCOPE_PROCESS
	* = END<INDENT>
	*
	* By default, or if <priority> is set to
	* ACE_DEFAULT_THREAD_PRIORITY, an "appropriate" priority value for
	* the given scheduling policy (specified in <flags}>, e.g.,
	* <THR_SCHED_DEFAULT>) is used.  This value is calculated
	* dynamically, and is the median value between the minimum and
	* maximum priority values for the given policy.  If an explicit
	* value is given, it is used.  Note that actual priority values are
	* EXTREMEMLY implementation-dependent, and are probably best
	* avoided.
	*/
	static int spawn (AX_THR_FUNC func,
		void *arg = 0,
		long flags = THR_NEW_LWP | THR_JOINABLE,
		AX_thread_t *t_id = 0,
		AX_hthread_t *t_handle = 0,
		long priority = AX_DEFAULT_THREAD_PRIORITY,
		void *stack = 0,
		size_t stack_size = 0);

	/**
	* Spawn N new threads
	* @see spawn()
	*/
	static size_t spawn_n (size_t n,
		AX_THR_FUNC func,
		void *arg = 0,
		long flags = THR_NEW_LWP | THR_JOINABLE,
		long priority = AX_DEFAULT_THREAD_PRIORITY,
		void *stack[] = 0,
		size_t stack_size[] = 0);

	/**
	* Spawn <n> new threads
	* @see spawn()
	*/
	static size_t spawn_n (AX_thread_t thread_ids[],
		size_t n,
		AX_THR_FUNC func,
		void *arg,
		long flags,
		long priority = AX_DEFAULT_THREAD_PRIORITY,
		void *stack[] = 0,
		size_t stack_size[] = 0,
		AX_hthread_t thread_handles[] = 0);

	/**
	* Wait for one or more threads to exit and reap their exit status.
	* thr_join() returns successfully when the target thread terminates.
	*
	* @param thread_id is the ACE_thread_t ID of the thread to wait for.
	*                  If @a thread_id is 0, join() waits for any
	*                  undetached thread in the process to terminate
	*                  on platforms that support this capability
	*                  (for example, Solaris).
	* @param departed  points to a location that is set to the ID of the
	*                  terminated thread if join() returns successfully.
	*                  If @a departed is 0, it is ignored.
	* @param status    Points to the location that receives the joined
	*                  thread's exit value. If @a status is 0, it is ignored.
	*
	* @retval  0 for success; -1 (with errno set) for failure.
	*/
	//win32 doesn't support this

#if 0//不使用这个函数，兼容性不好
	static int join (AX_thread_t thread_id,
		AX_thread_t *departed,
		AX_thr_func_return *status);
#endif

	/// Wait for one thread to exit and reap its exit status.
	static int join (AX_hthread_t,
		AX_thr_func_return * = 0);

	/// Continue the execution of a previously suspended thread.
	static int resume (AX_hthread_t);

	/// Suspend the execution of a particular thread.
	static int suspend (AX_hthread_t);

#if !defined(WINCE)//Windows CE下面不支持 GetThreadPriority,GetPriorityClass，所以禁用这3个函数
	/// Get the priority of a particular thread.
	static int getprio (AX_hthread_t ht_id, int &priority);

	/// Get the priority and policy of a particular thread.
	static int getprio (AX_hthread_t ht_id, int &priority, int &policy);

	/// Set the priority of a particular thread.
	static int setprio (AX_hthread_t ht_id, int priority, int policy = -1);

	static int kill (AX_thread_t, int signum);
#endif
#ifdef WIN32
	/// Send a signal to the thread.
	static int kill (AX_hthread_t, int signum);
#endif


//	/// Yield the thread to another.
	static void yield (void);

	/**
	* Return the unique kernel handle of the thread.  Note that on
	* Win32 this is actually a pseudohandle, which cannot be shared
	* with other processes or waited on by threads.  To locate the real
	* handle, please use the ACE_Thread_Manager::thr_self() method.
	*/
	static void self (AX_hthread_t &t_handle);

	/// Return the unique ID of the thread.
	static AX_thread_t self (void);

	/// Exit the current thread and return "status".
	/// Should _not_ be called by main thread.
//	static void exit (ACE_THR_FUNC_RETURN status = 0);

	/// Get the LWP concurrency level of the process.
//	static int getconcurrency (void);

	/// Set the LWP concurrency level of the process.
//	static int setconcurrency (int new_level);

	/// Change and/or examine calling thread's signal mask.
//	static int sigsetmask (int how,
//		const sigset_t *sigset,
//		sigset_t *osigset = 0);

	/**
	* Allocates a <keyp> that is used to identify data that is specific
	* to each thread in the process.  The key is global to all threads
	* in the process.
	*/ 
	////////////////////////////////////6月20日 YEYM添加
	static int keycreate (AX_thread_key_t *keyp,
		AX_THR_DEST destructor,

		void * = 0);

	/// Free up the key so that other threads can reuse it.
	static int keyfree (AX_thread_key_t key);

	 //Bind value to the thread-specific data key, <key>, for the calling
	 //thread.
	static int setspecific (AX_thread_key_t key,
		void *value);

	 //Stores the current value bound to <key> for the calling thread
	 //into the location pointed to by <valuep>.
	static int getspecific (AX_thread_key_t key,
		void **valuep);

	/// Disable thread cancellation.
//	static int disablecancel (struct cancel_state *old_state);

	/// Enable thread cancellation.
//	static int enablecancel (struct cancel_state *old_state,
//		int flag);

	/// Set the cancellation state.
//	static int setcancelstate (struct cancel_state &new_state,
//	struct cancel_state *old_state);

	/**
	* Cancel a thread.  Note that this method is only portable on
	* platforms, such as POSIX pthreads, that support thread
	* cancellation.
	*/
//	static int cancel (ACE_thread_t t_id);

	/// Test the cancel.
//	static void testcancel (void);
//#ifndef _WIN32
//	AX_Thread_Mutex mutex;
//#endif
private:
	/// Ensure that we don't get instantiated.
	AX_Thread (void);
};

#include "AX_Thread.inl"

#endif

