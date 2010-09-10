
/*
* Copyright (c) 2008, 杭州安谐软件有限公司
* All rights reserved.
*
* 文件名称：AX_OS.h
* 文件标识：
* 摘　　要：操作系统适配层 .h文件　－　win32&linux
*

* 当前版本：1.0
* 原作者　：林坚彦
* 完成日期：2008年5月15日
* 修订记录：创建
*/


#ifndef AX_OS_INCLUDE
#define AX_OS_INCLUDE
#include "platform.h"
#include <stdio.h>
#include <stack>
#include <stdlib.h>
using namespace std;

#if defined(WINCE)
//#include <windows.h>
//#include <winsock2i.h>
#include "winsock2i.h"
#include <windef.h>
#elif defined(WIN32)
#include "winsock2i.h"
#include <windef.h>
#include <errno.h>
#include <process.h>
#pragma warning(disable:4996)
#else
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/times.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <signal.h>
#endif

//global macros
# define AX_BIT_ENABLED(WORD, BIT) (((WORD) & (BIT)) != 0)
# define AX_BIT_DISABLED(WORD, BIT) (((WORD) & (BIT)) == 0)
# define AX_SET_BITS(WORD, BITS) (WORD |= (BITS))
# define AX_CLR_BITS(WORD, BITS) (WORD &= ~(BITS))

# define AX_MIN(a,b)      (((b) > (a)) ? (a) : (b))
# define AX_MAX(a,b)      (((a) > (b)) ? (a) : (b))
# define AX_RANGE(a,b,c)  (AX_MIN(AX_MAX((a), (c)), (b))
//end global macros

#define THR_SUSPEND     0x1
#define THR_STOPPED		0x2
typedef void (*AX_CLEANUP_FUNC)(void *object, void *param);
typedef void (*AX_THR_DEST)(void*);

typedef int ssize_t;

typedef void* (*AX_THR_FUNC )(void* );

typedef void (*AX_HOOK_FUNC)(void*);

typedef struct  
{
	AX_HOOK_FUNC func;
	void *arg;
}AX_Hook_Routine,AX_Exit_Routine;

#ifdef WIN32
typedef CRITICAL_SECTION AX_thread_mutex_t;
//typedef ULONG WINAPI(* a)(LPVOID);
typedef HANDLE AX_event_t;

typedef DWORD AX_thread_t;
typedef HANDLE AX_hthread_t;
#define ENOTSUP	ENOSYS 
//struct AX_hthread_t 
//{
//	HANDLE   _threadId;
//	int _status;
//	int operator!=(const AX_hthread_t temp)
//	{
//		if (temp._threadId == NULL)
//		{
//			return 0;
//		}
//		return 1;
//	}
//	int operator==(const AX_hthread_t& temp)
//	{
//		if (this->_threadId != temp._threadId)
//		{
//			return 0;
//		}
//		return 1;
//	}
//};
typedef DWORD AX_thr_func_return;
typedef HANDLE AX_sema_t;

class AX_cond_t;
//struct  AX_sema_t
//{
//	/// Serializes access to
//	CRITICAL_SECTION lock_;
//
//	/// This event is signaled whenever the count becomes non-zero.
//	HANDLE count_nonzero_;
//
//	/// Current count of the semaphore.
//	u_int count_;
//};
struct  AX_Mutex_t
{
	/// Either USYNC_THREAD or USYNC_PROCESS
	int type_;
	union
	{
		HANDLE proc_mutex_;
		CRITICAL_SECTION thr_mutex_;
	};
	HANDLE event;
};

class AX_cond_t
{
public:
	long waiters (void) const {return this->waiters_;}	/// Returns the number of waiters.

	long waiters_;	/// Number of waiting threads.
	AX_Mutex_t waiters_lock_;	/// Serialize access to the waiters count.
	AX_sema_t sema_;	/// Queue up threads waiting for the condition to become signaled.
	HANDLE waiters_done_;
	size_t was_broadcast_;	/// Keeps track of whether we were broadcasting or just signaling.
};

struct AX_condattr_t
{
	int type;
};

struct AX_rwlock_t
{
public:
	AX_Mutex_t lock_;		/// Serialize access to internal state.
	AX_cond_t waiting_readers_;	/// Reader threads waiting to acquire the lock.
	int num_waiting_readers_;	/// Number of waiting readers.
	AX_cond_t waiting_writers_;	/// Writer threads waiting to acquire the lock.
	int num_waiting_writers_;	/// Number of waiting writers.
	int ref_count_;	/// Value is -1 if writer has the lock, else this keeps track of the
					/// number of readers holding the lock.
	int important_writer_;	/// Indicate that a reader is trying to upgrade
	AX_cond_t waiting_important_writer_;	/// Condition for the upgrading reader
};


struct AX_mutexattr_t
{
	int type;
};
 
typedef DWORD AX_thread_key_t;
typedef int (*AX_SEH_EXCEPT_HANDLER)(void *);
typedef HANDLE AX_HANDLE;
#define AX_INVALID_HANDLE	INVALID_HANDLE_VALUE
#  define PROT_READ PAGE_READONLY
#  define PROT_WRITE PAGE_READWRITE
#  define PROT_RDWR PAGE_READWRITE

#define AX_MT(t)
typedef long pid_t;

#else	//linux
#define THR_CANCEL_ENABLE       0
#define THR_CANCEL_DEFERRED     0
#define THR_CANCEL_ASYNCHRONOUS 0
#define THR_JOINABLE            0
#define THR_SCHED_FIFO          0
#define THR_SCHED_RR            0
#define THR_SCHED_DEFAULT       0
typedef pthread_mutex_t AX_Mutex_t;
typedef pthread_rwlock_t AX_rwlock_t;

#  define PROT_RDWR (PROT_READ|PROT_WRITE)

typedef pthread_cond_t AX_cond_t;
typedef pthread_condattr_t AX_condattr_t;

//typedef struct  
//{
//	pthread_t   _threadId;
//	int _status;
//}AX_hthread_t;
typedef pthread_t AX_hthread_t;
typedef pthread_key_t AX_thread_key_t;

typedef pthread_t AX_thread_t;

typedef void* AX_thr_func_return;

//暂时处理
//typedef int AX_sema_t;
typedef sem_t AX_sema_t;
typedef pthread_mutex_t AX_event_t;
typedef int SECURITY_ATTRIBUTES;
typedef struct _SECURITY_ATTRIBUTES *PSECURITY_ATTRIBUTES;
typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef int AX_HANDLE;
#define AX_INVALID_HANDLE	-1
#endif


//mutex usync type
#define AX_USYNC_THREAD		1
#define AX_USYNC_PROCESS	2

#define MAXLONG     0x7fffffff 
//thread priority
#define AX_DEFAULT_THREAD_PRIORITY 127

#if !defined(WINCE)
#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)

#define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#define THREAD_BASE_PRIORITY_MIN    (-2)  // minimum thread base priority boost
#define THREAD_BASE_PRIORITY_IDLE   (-15) // value that gets a thread to idle

#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE

#else//For WINCE
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)

#define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#endif
// thread dwCreationFlag values

#define DEBUG_PROCESS                     0x00000001
#define DEBUG_ONLY_THIS_PROCESS           0x00000002

#define CREATE_SUSPENDED                  0x00000004

#define DETACHED_PROCESS                  0x00000008

#define CREATE_NEW_CONSOLE                0x00000010

#define NORMAL_PRIORITY_CLASS             0x00000020
#define IDLE_PRIORITY_CLASS               0x00000040
#define HIGH_PRIORITY_CLASS               0x00000080
#define REALTIME_PRIORITY_CLASS           0x00000100

#define CREATE_NEW_PROCESS_GROUP          0x00000200
#define CREATE_UNICODE_ENVIRONMENT        0x00000400

#define CREATE_SEPARATE_WOW_VDM           0x00000800
#define CREATE_SHARED_WOW_VDM             0x00001000
#define CREATE_FORCEDOS                   0x00002000

#define BELOW_NORMAL_PRIORITY_CLASS       0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS       0x00008000
#define STACK_SIZE_PARAM_IS_A_RESERVATION 0x00010000

#define CREATE_BREAKAWAY_FROM_JOB         0x01000000
#define CREATE_PRESERVE_CODE_AUTHZ_LEVEL  0x02000000

#define CREATE_DEFAULT_ERROR_MODE         0x04000000
#define CREATE_NO_WINDOW                  0x08000000

#define PROFILE_USER                      0x10000000
#define PROFILE_KERNEL                    0x20000000
#define PROFILE_SERVER                    0x40000000

#define CREATE_IGNORE_SYSTEM_DEFAULT      0x80000000

//thread flags - 未经确认，勿用
#     define THR_CANCEL_DISABLE      0
#     define THR_CANCEL_ENABLE       0
#     define THR_CANCEL_DEFERRED     0
#     define THR_CANCEL_ASYNCHRONOUS 0
#     define THR_DETACHED            0x02000000 /* ignore in most places */
#     define THR_BOUND               0          /* ignore in most places */
#     define THR_NEW_LWP             0          /* ignore in most places */
#     define THR_DAEMON              0          /* ignore in most places */
#     define THR_JOINABLE            0          /* ignore in most places */
#     define THR_SUSPENDED   CREATE_SUSPENDED
#     define THR_USE_AFX             0x01000000
#     define THR_SCHED_FIFO          0
#     define THR_SCHED_RR            0
#     define THR_SCHED_DEFAULT       0
#ifdef WIN32
#     define THR_INHERIT_SCHED       0
#     define THR_SCOPE_PROCESS       0
#     define THR_SCOPE_SYSTEM        0
#else	//linux
#     define THR_SCOPE_SYSTEM        0x00100000
#     define THR_SCOPE_PROCESS       0x00200000
#     define THR_INHERIT_SCHED       0x00400000
#endif

#     define THR_EXPLICIT_SCHED      0x00800000
#     define THR_SCHED_IO            0x01000000

//thread function definition
typedef void* (*AX_THR_C_FUNC)(void *);


#define AX_NEW_RETURN(POINTER,CONSTRUCTOR,RET_VAL) \
	do { POINTER = new CONSTRUCTOR; \
	if (POINTER == 0) { /*errno = ENOMEM;*/ return RET_VAL; } \
	} while (0)

namespace AX_OS
{
//assistant funs
#ifdef WIN32

	/// Default Win32 Security Attributes definition.
	inline LPSECURITY_ATTRIBUTES default_win32_security_attributes (LPSECURITY_ATTRIBUTES);
	inline LPSECURITY_ATTRIBUTES default_win32_security_attributes_r (LPSECURITY_ATTRIBUTES,
		LPSECURITY_ATTRIBUTES,
		SECURITY_DESCRIPTOR*);

#endif	//#ifdef WIN32

//usual apis
	inline int sleep(int msec);	//sleep milliseconds

//mutex 
	inline int thread_mutex_init (AX_Mutex_t *m, int lock_type = 0);

	inline int thread_mutex_lock (AX_Mutex_t *m);
	inline int thread_mutex_lock (AX_Mutex_t *m, const timeval &timeout);
	inline int thread_mutex_lock (AX_Mutex_t *m, const timeval *timeout);

	inline int thread_mutex_trylock (AX_Mutex_t *m);

	inline int thread_mutex_unlock (AX_Mutex_t *m);

	inline int thread_mutex_destroy (AX_Mutex_t *m);

//readers/writer lock
	inline int rw_rdlock (AX_rwlock_t *rw);

	inline int rw_tryrdlock (AX_rwlock_t *rw);

	inline int rw_trywrlock (AX_rwlock_t *rw);

//	inline int rw_trywrlock_upgrade (AX_rwlock_t *rw);

	inline int rw_unlock (AX_rwlock_t *rw);

	inline int rw_wrlock (AX_rwlock_t *rw);

	inline int rwlock_destroy (AX_rwlock_t *rw);

	inline int rwlock_init (AX_rwlock_t *rw,
							int type = AX_USYNC_THREAD,
							const char *name = 0,
							void *arg = 0);

//semaphore
	inline int sema_init (AX_sema_t *s,
							unsigned int count,
							int type = AX_USYNC_THREAD,
							const char *name = 0,
							void *arg = 0,
							int max = 0x7fffffff
#ifdef _WIN32
							,LPSECURITY_ATTRIBUTES sa = 0
#endif						
							);

	inline int sema_post (AX_sema_t *s);
	inline int sema_post (AX_sema_t *s, unsigned int release_count);
	
	inline int sema_wait (AX_sema_t *s);
	inline int sema_wait (AX_sema_t *s, timeval &tv);
	inline int sema_wait (AX_sema_t *s, timeval *tv);

	inline int sema_destroy (AX_sema_t *s);

//event ...
	inline int event_init (AX_event_t *event,
							int manual_reset = 0,
							int initial_state = 0,
							int type = AX_USYNC_THREAD,
							const char *name = 0,
							void *arg = 0,
							LPSECURITY_ATTRIBUTES sa = 0);

	inline int event_destroy (AX_event_t *event);

	inline int event_reset (AX_event_t *event);

	inline int event_signal (AX_event_t *event);

	inline int event_timedwait (AX_event_t *event,	timeval *timeout);

	inline int event_wait (AX_event_t *event);

//condition
	inline int cond_init (AX_cond_t *cv, short type, const char *name = 0,	void *arg = 0);
	inline int cond_init (AX_cond_t *cv, AX_condattr_t &attributes, const char *name = 0,	void *arg = 0);

	inline int cond_wait (AX_cond_t *cv, AX_Mutex_t *external_mutex);

	inline int cond_signal (AX_cond_t *cv);

	inline int cond_broadcast (AX_cond_t *cv);

	inline int cond_destroy (AX_cond_t *cv);

	inline int condattr_init (AX_condattr_t &attributes, int type = AX_USYNC_THREAD);

	inline int condattr_destroy (AX_condattr_t &attributes);

//thread
	int thr_create (AX_THR_FUNC func,
		void *args,
		long flags,
		AX_thread_t *thr_id,
		AX_hthread_t *thr_handle = 0,
		long priority = AX_DEFAULT_THREAD_PRIORITY,
		void *stack = 0,
		size_t stacksize = 0);

	inline int thr_join (AX_hthread_t waiter_id, AX_thr_func_return *status);
#if !defined(WINCE) //Windows CE下面不支持 OpenThread,所以不能从ThreadId转换到ThreadHandle
	inline int thr_join (AX_thread_t waiter_id,	AX_thread_t *thr_id, AX_thr_func_return *status);

	inline int thr_kill (AX_thread_t thr_id,int signum);
#endif
#ifdef WIN32
	inline int thr_kill (AX_hthread_t id,int signum);
#endif

	inline AX_thread_t thr_self (void);
	inline void thr_self (AX_hthread_t &);

#if !defined(WINCE)//Windows CE下面不支持 GetThreadPriority,GetPriorityClass，所以禁用这3个函数
	inline int thr_getprio (AX_hthread_t id, int &priority);
	inline int thr_getprio (AX_hthread_t id, int &priority, int &policy);
#endif//WINCE
	inline int thr_setprio (AX_hthread_t ht_id,	int priority, int policy = -1);

	inline int thr_suspend (AX_hthread_t target_thread);

	inline int thr_continue (AX_hthread_t target_thread,int status = 0);

	inline int thr_cancel (AX_hthread_t t_id);

	void thr_exit (AX_thr_func_return status = 0);


	// 2008-5-30 yeym 添加
	inline char* itoa(int value,char *string,int radix);

	uint64 get_time();
	inline void thr_yield (void);
	/// 2008-6-16
	 int thr_key_create(AX_thread_key_t *key,AX_THR_DEST func = NULL);
	 int thr_key_set(AX_thread_key_t,const void *point = NULL);
	 void* thr_key_get(AX_thread_key_t);
	 int thr_keyfree(AX_thread_key_t);

	 inline int cond_timedwait(AX_cond_t *cv,AX_Mutex_t *external_mutex,timeval *timeout);
	 inline int event_pulse (AX_event_t *event);
	 inline int sema_trywait(AX_sema_t *s);

	 inline int rw_trywrlock_upgrade(AX_rwlock_t *rw);

	 inline int thr_equal(AX_thread_t t1, AX_thread_t t2);
	 inline int thr_cmp(AX_hthread_t t1, AX_hthread_t t2);


	 inline int fork();
	 inline int kill(AX_hthread_t pid, int signum);
	 inline int exit(int status = 0 ); 
	 inline int getpid(void);
	 inline int waitpid(AX_hthread_t,int *exitCode,int options);
#if !defined(WINCE)//WINCE下不支持 getenv,setenv 
	 inline char* getenv(char*);
	 inline int putenv(char *string);
#endif
	 inline int alarm(int);
#if !defined(WINCE)
	 inline int system(char* string);
#endif
	 inline unsigned long atoul(const char* string);
	 inline uint64 atoul64(const char* string);

}

#include "AX_OS.inl"

#endif

