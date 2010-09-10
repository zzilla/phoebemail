
#ifndef _AX_THREAD_MANAGER_H
#define _AX_THREAD_MANAGER_H

#include "AX_OS.h"
#include "AX_Contains.h"
//#include "AX_Thread_Adapter.h"
//#include "AX_Thread.h"
//#include "AX_Thread_Exit.h"
#include "AX_Base_Thread.h"

#include <stack>
#include <list>
using namespace std;
class AX_OS_Thread_Descriptor;
class AX_Thread_Descriptor;
class AX_Thread_Exit;


class AX_At_Thread_Exit
{
	friend class AX_Thread_Descriptor;
	friend class AX_Thread_Manager;
public:
	/// Default constructor
	AX_At_Thread_Exit (void);

	/// The destructor
	virtual ~AX_At_Thread_Exit (void);

	/// <At_Thread_Exit> has the ownership?
	int is_owner (void) const;

	/// Set the ownership of the <At_Thread_Exit>.
	int is_owner (int owner);

	/// This <At_Thread_Exit> was applied?
	int was_applied (void) const;

	/// Set applied state of <At_Thread_Exit>.
	int was_applied (int applied);

protected:
	/// The next <At_Thread_Exit> hook in the list.
	AX_At_Thread_Exit *next_;

	/// Do the apply if necessary
	void do_apply (void);

	/// The apply method.
	virtual void apply (void) = 0;

	/// The Thread_Descriptor where this at is registered.
    AX_Thread_Descriptor* td_;

	/// The at was applied?
	int was_applied_;

	/// The at has the ownership of this?
	int is_owner_;
};

class AX_At_Thread_Exit_Func : public AX_At_Thread_Exit
{
public:
	/// Constructor
	AX_At_Thread_Exit_Func (void *object,
		AX_CLEANUP_FUNC func,
		void *param = 0);

	virtual ~AX_At_Thread_Exit_Func (void);

protected:
	/// The object to be cleanup
	void *object_;

	/// The cleanup func
	AX_CLEANUP_FUNC func_;

	/// A param if required
	void *param_;

	/// The apply method
	void apply (void);
};
/**
* @class ACE_Thread_Manager
*
* @brief Manages a pool of threads.
*
* This class allows operations on groups of threads atomically.
* The default behavior of thread manager is to wait on
* all threads under it's management when it gets destructed.
* Therefore, remember to remove a thread from thread manager if
* you don't want it to wait for the thread. There are also
* function to disable this default wait-on-exit behavior.
* However, if your program depends on turning this off to run
* correctly, you are probably doing something wrong.  Rule of
* thumb, use ACE_Thread to manage your daemon threads.
* Notice that if there're threads live beyond the scope of
* <main>, you are sure to have resource leaks in your program.
* Remember to wait on threads before exiting <main> if that
* could happen in your programs.
*/
class  AX_Thread_Manager : public AX_OS_Thread_Descriptor
{
public:
	friend class AX_Thread_Control;

	// Allow ACE_THread_Exit to register the global TSS instance object.
	friend class AX_Thread_Exit;

	friend class AX_Thread_Descriptor;

//#ifdef _WIN32
	typedef int (AX_Thread_Manager::*AX_THR_MEMBER_FUNC)(AX_Thread_Descriptor *, int);
//#endif

	/// These are the various states a thread managed by the
	/// ACE_Thread_Manager can be in.
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

	// = Initialization and termination methods.
	/**
	* @brief Initialization and termination methods.
	*
	* Internally, ACE_Thread_Manager keeps a freelist for caching
	* resources it uses to keep track of managed threads (not the
	* threads themselves.)  @a prealloc, @a lwm, @a inc, @hwm
	* determine the initial size, the low water mark, increment step,
	* and high water mark of the freelist.
	*
	* @sa ACE_Free_List
	*/
	AX_Thread_Manager (size_t preaolloc = 0 ,
		size_t lwm = 0 ,
		size_t inc = 0,
		size_t hwm = 0);
	~AX_Thread_Manager (void);

#if ! defined (ACE_THREAD_MANAGER_LACKS_STATICS)
	/// Get pointer to a process-wide <ACE_Thread_Manager>.
	static AX_Thread_Manager *instance (void);

	/// Set pointer to a process-wide <ACE_Thread_Manager> and return
	/// existing pointer.
	static AX_Thread_Manager *instance (AX_Thread_Manager *);

	/// Delete the dynamically allocated Singleton
	static void close_singleton (void);
#endif /* ! defined (ACE_THREAD_MANAGER_LACKS_STATICS) */

	/// No-op.  Currently unused.
	int open (size_t size = 0);

	/**
	* Release all resources.
	* By default, this method will wait till all threads
	* exit.  However, when called from <close_singleton>, most global resources
	* are destroyed and thus, we don't try to wait but just clean up the thread
	* descriptor list.
	*/
	int close (void);

	// The <ACE_thread_t> * argument to each of the <spawn> family member
	// functions is interpreted and used as shown in the following
	// table.  NOTE:  the final option, to provide task names, is _only_
	// supported on VxWorks!
	//
	// Value of ACE_thread_t * argument  Use                         Platforms
	// ================================  ==========================  =========
	// 0                                 Not used.                   All
	// non-0 (and points to 0 char *     The task name is passed     All
	//   on VxWorks)                       back in the char *.
	// non-0, points to non-0 char *     The char * is used as       VxWorks only
	//                                     the task name.  The
	//                                     argument is not modified.

	/**
	* Create a new thread, which executes <func> with argument <arg>.
	* Returns: on success a unique group id that can be used to control
	* other threads added to the same group.  On failure, returns -1.
	*/
	int spawn (AX_THR_FUNC func,
		void *arg = 0,
		long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
		AX_thread_t * = 0,
		AX_hthread_t *t_handle = 0,
		long priority = AX_DEFAULT_THREAD_PRIORITY,
		int grp_id = -1,
		void *stack = 0,
		size_t stack_size = 0);

	/**
	* Spawn N new threads, which execute <func> with argument <arg>.
	* If <thread_ids> != 0 the thread_ids of successfully spawned
	* threads will be placed into the <thread_ids> buffer (which must
	* be the same size as <n>).  If <stack> != 0 it is assumed to be an
	* array of <n> pointers to the base of the stacks to use for the
	* threads being spawned.  If <stack_size> != 0 it is assumed to be
	* an array of <n> values indicating how big each of the
	* corresponding <stack>s are.  If <thread_handles> != 0 it is
	* assumed to be an array of <n> thread_handles that will be
	* assigned the values of the thread handles being spawned.
	*
	* Threads in Thread_Manager can be manipulated in groups based on
	* <grp_id> or <task> using functions such as kill_grp() or
	* cancel_task().
	*
	* If <grp_id> is assigned, the newly spawned threads are added into
	* the group.  Otherwise, the Thread_Manager assigns these <n>
	* threads with a grp_id.  You should choose either assigning
	* <grp_id> everytime, or let the Thread_Manager handles it for
	* you consistently.
	*
	* The argument <task> is usually assigned by
	* <ACE_Task_Base::activate>.  It associates the newly spawned
	* threads with an ACE_Task instance, which defaults to <this>.
	*
	* @retval -1 on failure (<errno> will explain...), otherwise returns the
	* group id of the threads.
	*/
	//int spawn_n (size_t n,
	//	AX_THR_FUNC func,
	//	void *arg = 0,
	//	long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
	//	long priority = AX_DEFAULT_THREAD_PRIORITY,
	//	int grp_id = -1,
	//	AX_Task_Base *task = 0,
	//	AX_hthread_t thread_handles[] = 0,
	//	void *stack[] = 0,
	//	size_t stack_size[] = 0);

	/**
	* Spawn N new threads, which execute <func> with argument <arg>.
	* If <thread_ids> != 0 the thread_ids of successfully spawned
	* threads will be placed into the <thread_ids> buffer (which must
	* be the same size as <n>).  If <stack> != 0 it is assumed to be an
	* array of <n> pointers to the base of the stacks to use for the
	* threads being spawned.  If <stack_size> != 0 it is assumed to be
	* an array of <n> values indicating how big each of the
	* corresponding <stack>s are.  If <thread_handles> != 0 it is
	* assumed to be an array of <n> thread_handles that will be
	* assigned the values of the thread handles being spawned.
	*
	* Threads in Thread_Manager can be manipulated in groups based on
	* <grp_id> or <task> using functions such as kill_grp() or
	* cancel_task().
	*
	* If <grp_id> is assigned, the newly spawned threads are added into
	* the group.  Otherwise, the Thread_Manager assigns these <n>
	* threads with a grp_id.  You should choose either assigning
	* <grp_id> everytime, or let the Thread_Manager handles it for
	* you consistently.
	*
	* The argument <task> is usually assigned by
	* <ACE_Task_Base::activate>.  It associates the newly spawned
	* threads with an ACE_Task instance, which defaults to <this>.
	*
	* @retval -1 on failure (<errno> will explain...), otherwise returns the
	* group id of the threads.
	*/
	//int spawn_n (AX_thread_t thread_ids[],
	//	size_t n,
	//	AX_THR_FUNC func,
	//	void *arg,
	//	long flags,
	//	long priority = AX_DEFAULT_THREAD_PRIORITY,
	//	int grp_id = -1,
	//	void *stack[] = 0,
	//	size_t stack_size[] = 0,
	//	AX_hthread_t thread_handles[] = 0,
	//	AX_Task_Base *task = 0);

	/**
	* Called to clean up when a thread exits.
	*
	* @param do_thread_exit If non-0 then ACE_Thread::exit is called to
	*                       exit the thread
	* @param status         If ACE_Thread_Exit is called, this is passed as
	*                       the exit value of the thread.
	* Should _not_ be called by main thread.
	*/
	AX_thr_func_return exit (AX_thr_func_return status = 0,
		int do_thread_exit = 1);

	/**
	* Block until there are no more threads running in this thread
	* manager or @c timeout expires.
	*
	* @param timeout is treated as "absolute" time by default, but this
	*                can be changed to "relative" time by setting the @c
	*                use_absolute_time to 0.
	* @param abandon_detached_threads If non-0, @c wait() will first
	*                                 check thru its thread list for
	*                                 threads with THR_DETACHED or
	*                                 THR_DAEMON flags set and remove
	*                                 these threads.  Notice that
	*                                 unlike other @c wait_*() methods,
	*                                 by default, @c wait() does wait on 
	*                                 all thread spawned by this
	*                                 thread manager no matter the detached 
	*                                 flags are set or not unless it is
	*                                 called with @c
	*                                 abandon_detached_threads flag set. 
	* @param use_absolute_time If non-0 then treat @c timeout as
	*                          absolute time, else relative time.
	* @return 0 on success * and -1 on failure.  
	*
	* NOTE that if this function is called while the @c
	* ACE_Object_Manager is shutting down (as a result of program
	* rundown via @c ACE::fini()), it will not wait for any threads to
	* complete. If you must wait for threads spawned by this thread
	* manager to complete and you are in a ACE rundown situation (such
	* as your object is being destroyed by the @c ACE_Object_Manager)
	* you can use @c wait_grp() instead.
	*/
	int wait (const timeval *timeout = 0,
		bool abandon_detached_threads = false,
		bool use_absolute_time = true);

	/// Join a thread specified by <tid>.  Do not wait on a detached thread.
	int join (AX_thread_t tid, AX_thr_func_return *status = 0);

	/**
	* Block until there are no more threads running in a group.
	* Returns 0 on success and -1 on failure.  Notice that wait_grp
	* will not wait on detached threads.
	*/
	int wait_grp (int grp_id);

	/**
	* Return the "real" handle to the calling thread, caching it if
	* necessary in TSS to speed up subsequent lookups. This is
	* necessary since on some platforms (e.g., Win32) we can't get this
	* handle via direct method calls.  Notice that you should *not*
	* close the handle passed back from this method.  It is used
	* internally by Thread Manager.  On the other hand, you *have to*
	* use this internal thread handle when working on Thread_Manager.
	* Return -1 if fail.
	*/
	int thr_self (AX_hthread_t &);

	/**
	* Return the unique ID of the thread.  This is not strictly
	* necessary (because a thread can always just call
	* <ACE_Thread::self>).  However, we put it here to be complete.
	*/
	AX_thread_t thr_self (void);

	/**
	* Returns a pointer to the current <ACE_Task_Base> we're executing
	* in if this thread is indeed running in an <ACE_Task_Base>, else
	* return 0.
	*/
	//AX_Task_Base *task (void);

	// = Suspend methods, which isn't supported on POSIX pthreads (will not block).
	/// Suspend all threads
	int suspend_all (void);

	/// Suspend a single thread.
	int suspend (AX_thread_t);

	/// Suspend a group of threads.
	int suspend_grp (int grp_id);

	/**
	* True if <t_id> is inactive (i.e., suspended), else false.  Always
	* return false if <t_id> is not managed by the Thread_Manager.
	*/
	int testsuspend (AX_thread_t t_id);

	// = Resume methods, which isn't supported on POSIX pthreads (will not block).
	/// Resume all stopped threads
	int resume_all (void);

	/// Resume a single thread.
	int resume (AX_thread_t);

	/// Resume a group of threads.
	int resume_grp (int grp_id);

	/**
	* True if <t_id> is active (i.e., resumed), else false.  Always
	* return false if <t_id> is not managed by the Thread_Manager.
	*/
	int testresume (AX_thread_t t_id);

	// = Send signals to one or more threads without blocking.
	/**
	* Send <signum> to all stopped threads.  Not supported on platforms
	* that do not have advanced signal support, such as Win32.
	* Send the <signum> to a single thread.  Not supported on platforms
	* that do not have advanced signal support, such as Win32.
	* Send <signum> to a group of threads, not supported on platforms
	* that do not have advanced signal support, such as Win32.
	*/
	int kill_all (int signum);
	int kill (AX_thread_t,
		int signum);
	int kill_grp (int grp_id,
		int signum);

	// = Cancel methods, which provides a cooperative thread-termination mechanism (will not block).
	/**
	* Cancel's all the threads.
	*/
	int cancel_all (int async_cancel = 0);

	/**
	* Cancel a single thread.
	*/
	int cancel (AX_thread_t, int async_cancel = 0);

	/**
	* Cancel a group of threads.
	*/
	int cancel_grp (int grp_id, int async_cancel = 0);

	/**
	* True if <t_id> is cancelled, else false.  Always return false if
	* <t_id> is not managed by the Thread_Manager.
	*/
	int testcancel (AX_thread_t t_id);

	/**
	* True if <t_id> has terminated (i.e., is no longer running),
	* but the slot in the thread manager hasn't been reclaimed yet,
	* else false.  Always return false if <t_id> is not managed by the
	* Thread_Manager.
	*/
	int testterminate (AX_thread_t t_id);

	/// Set group ids for a particular thread id.
	int set_grp (AX_thread_t,
		int grp_id);

	/// Get group ids for a particular thread id.
	int get_grp (AX_thread_t,
		int &grp_id);

	// = The following methods are new methods which resemble current
	// methods in <ACE_Thread Manager>. For example, the <apply_task>
	// method resembles the <apply_thr> method, and <suspend_task>
	// resembles <suspend_thr>.

	// = Operations on ACE_Tasks.

	/**
	* Block until there are no more threads running in a specified task.
	* This method will not wait for either detached or daemon threads;
	* the threads must have been spawned with the @c THR_JOINABLE flag.
	* Upon successful completion, the threads have been joined, so further
	* attempts to join with any of the waited-for threads will fail.
	*
	* @param task  The ACE_Task_Base object whose threads are to waited for.
	*
	* @retval 0  Success.
	* @retval -1 Failure (consult errno for further information).
	*/
	//int wait_task (AX_Task_Base *task);

	///**
	//* Suspend all threads in an ACE_Task.
	//*/
	//int suspend_task (AX_Task_Base *task);

	/**
	* Resume all threads in an ACE_Task.
	*/
	//int resume_task (AX_Task_Base *task);

	///**
	//* Send a signal <signum> to all threads in an <ACE_Task>.
	//*/
	//int kill_task (AX_Task_Base *task,
	//	int signum);

	/**
	* Cancel all threads in an <ACE_Task>.  If <async_cancel> is non-0,
	* then asynchronously cancel these threads if the OS platform
	* supports cancellation.  Otherwise, perform a "cooperative"
	* cancellation.
	*/
	//int cancel_task (AX_Task_Base *task, int async_cancel = 0);

	// = Collect thread handles in the thread manager.  Notice that
	//   the collected information is just a snapshot.
	/// Check if the thread is managed by the thread manager.  Return true if
	/// the thread is found, false otherwise.
	int hthread_within (AX_hthread_t handle);
	int thread_within (AX_thread_t tid);

	/// Returns the number of <ACE_Task_Base> in a group.
	int num_tasks_in_group (int grp_id);

	/// Returns the number of threads in an 
	//int num_threads_in_task (AX_Task_Base *task);

	/**
	* Returns a list of ACE_Task_Base pointers corresponding to the tasks
	* that have active threads in a specified thread group.
	*
	* @param grp_id    The thread group ID to obtain task pointers for.
	*
	* @param task_list is a pointer to an array to receive the list of pointers.
	*                  The caller is responsible for supplying an array with at
	*                  least @arg n entries.
	*
	* @param n         The maximum number of ACE_Task_Base pointers to write
	*                  in @arg task_list.
	*
	* @retval  If successful, the number of pointers returned, which will be
	*          no greater than @arg n. Returns -1 on error.
	*
	* @note    This method has no way to indicate if there are more than
	*          @arg n ACE_Task_Base pointers available. Therefore, it may be
	*          wise to guess a larger value of @arg n than one thinks in cases
	*          where the exact number of tasks is not known.
	*
	* @sa      num_tasks_in_group(), task_all_list()
	*/
	//ssize_t task_list (int grp_id,
	//	AX_Task_Base *task_list[],
	//	size_t n);

	/**
	* Returns in <thread_list> a list of up to <n> thread ids in an
	* <ACE_Task_Base>.  The caller must allocate the memory for
	* <thread_list>.  In case of an error, -1 is returned. If no
	* requested values are found, 0 is returned, otherwise correct
	* number of retrieved values are returned.
	*/
	//ssize_t thread_list (AX_Task_Base *task,
	//	AX_thread_t thread_list[],
	//	size_t n);

	/**
	* Returns in <hthread_list> a list of up to <n> thread handles in
	* an <ACE_Task_Base>.  The caller must allocate memory for
	* <hthread_list>.  In case of an error, -1 is returned. If no
	* requested values are found, 0 is returned, otherwise correct
	* number of retrieved values are returned.
	*/
	//ssize_t hthread_list (AX_Task_Base *task,
	//	AX_hthread_t hthread_list[],
	//	size_t n);

	/**
	* Returns in <thread_list> a list of up to <n> thread ids in a
	* group <grp_id>.  The caller must allocate the memory for
	* <thread_list>.  In case of an error, -1 is returned. If no
	* requested values are found, 0 is returned, otherwise correct
	* number of retrieved values are returned.
	*/
	int thread_grp_list (int grp_id,
		AX_thread_t thread_list[],
		size_t n);

	/**
	* Returns in <hthread_list> a list of up to <n> thread handles in
	* a group <grp_id>.  The caller must allocate memory for
	* <hthread_list>.
	*/
	int hthread_grp_list (int grp_id,
		AX_hthread_t hthread_list[],
		size_t n);

	/**
	* Returns a list of ACE_Task_Base pointers corresponding to the tasks
	* that have active threads managed by this instance.
	*
	* @param task_list is a pointer to an array to receive the list of pointers.
	*                  The caller is responsible for supplying an array with at
	*                  least @arg n entries.
	*
	* @param n         The maximum number of ACE_Task_Base pointers to write
	*                  in @arg task_list.
	*
	* @retval  If successful, the number of pointers returned, which will be
	*          no greater than @arg n. Returns -1 on error.
	*
	* @note    This method has no way to indicate if there are more than
	*          @arg n ACE_Task_Base pointers available. Therefore, it may be
	*          wise to guess a larger value of @arg n than one thinks in cases
	*          where the exact number of tasks is not known.
	*
	* @sa      count_threads()
	*/
	//ssize_t task_all_list (AX_Task_Base *task_list[],
	//	size_t n);

	/**
	* Returns in <thread_list> a list of up to <n> thread ids.  The
	* caller must allocate the memory for <thread_list>.  In case of an
	* error, -1 is returned. If no requested values are found, 0 is
	* returned, otherwise correct number of retrieved values are
	* returned.
	*/
	int thread_all_list (AX_thread_t thread_list[],
		size_t n);

	/// Set group ids for a particular task.
	//int set_grp (AX_Task_Base *task, int grp_id);

	///// Get group ids for a particular task.
	//int get_grp (AX_Task_Base *task, int &grp_id);

	/// Return a count of the current number of threads active in the
	/// <Thread_Manager>.
	size_t count_threads (void) const;

	/// Get the state of the thread. Returns false if the thread is not
	/// managed by this thread manager.
	int thr_state (AX_thread_t id, int & state);

#if !defined(ACE_USE_ONE_SHOT_AT_THREAD_EXIT)
	/**
	* Register an At_Thread_Exit hook and the ownership is acquire by
	* Thread_Descriptor, this is the usual case when the AT is dynamically
	* allocated.
	*/
	int at_exit (AX_At_Thread_Exit* cleanup);

	/// Register an At_Thread_Exit hook and the ownership is retained for the
	/// caller. Normally used when the at_exit hook is created in stack.
	int at_exit (AX_At_Thread_Exit& cleanup);
#endif /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */

	/**
	*
	*****
	* @deprecated This function is deprecated.  Please use the previous two
	*    at_exit method.  Notice that you should avoid mixing this method
	*    with the previous two at_exit methods.
	*****
	*
	* Register an object (or array) for cleanup at
	* thread termination.  "cleanup_hook" points to a (global, or
	* static member) function that is called for the object or array
	* when it to be destroyed.  It may perform any necessary cleanup
	* specific for that object or its class.  "param" is passed as the
	* second parameter to the "cleanup_hook" function; the first
	* parameter is the object (or array) to be destroyed.
	* "cleanup_hook", for example, may delete the object (or array).
	* If <cleanup_hook> == 0, the <object> will _NOT_ get cleanup at
	* thread exit.  You can use this to cancel the previously added
	* at_exit.
	*/
	int at_exit (void *object,
		AX_CLEANUP_FUNC cleanup_hook,
		void *param);

	/// Access function to determine whether the Thread_Manager will
	/// wait for its thread to exit or not when being closing down.
	void wait_on_exit (int dowait);
	int wait_on_exit (void);

	/// Dump the state of an object.
	void dump (void);


protected:
	// = Accessors for ACE_Thread_Descriptors.
	/**
	* Get a pointer to the calling thread's own thread_descriptor.
	* This must be called from a spawn thread.  This function will
	* fetch the info from TSS.
	*/
	AX_Thread_Descriptor *thread_desc_self (void);

	/// Return a pointer to the thread's Thread_Descriptor,
	/// 0 if fail.
	AX_Thread_Descriptor *thread_descriptor (AX_thread_t);

	/// Return a pointer to the thread's Thread_Descriptor,
	/// 0 if fail.
	AX_Thread_Descriptor *hthread_descriptor (AX_hthread_t);

	/// Create a new thread (must be called with locks held).
	int spawn_i (AX_THR_FUNC func,
		void *arg,
		long flags,
		AX_thread_t * = 0,
		AX_hthread_t *t_handle = 0,
		long priority = 0,
		int grp_id = -1,
		void *stack = 0,
		size_t stack_size = 0);

	/// Run the registered hooks when the thread exits.
	void run_thread_exit_hooks (int i);

	/// Locate the index of the table slot occupied by <t_id>.  Returns
	/// -1 if <t_id> is not in the table doesn't contain <t_id>.
	AX_Thread_Descriptor *find_thread (AX_thread_t t_id);

	/// Locate the index of the table slot occupied by <h_id>.  Returns
	/// -1 if <h_id> is not in the table doesn't contain <h_id>.
	AX_Thread_Descriptor *find_hthread (AX_hthread_t h_id);

	/**
	* Locate the thread descriptor address of the list occupied by
	* <task>.  Returns 0 if <task> is not in the table doesn't contain
	* <task>.
	*/
	//AX_Thread_Descriptor *find_task (ACE_Task_Base *task,
	//	size_t slot = 0);

	/// Insert a thread in the table (checks for duplicates).
	int insert_thr (AX_thread_t t_id,
		AX_hthread_t,
		int grp_id = -1,
		long flags = 0);

	/// Append a thread in the table (adds at the end, growing the table
	/// if necessary).
	int append_thr (AX_thread_t t_id, AX_hthread_t,
		int,
		int grp_id,
		//ACE_Task_Base *task = 0,
		long flags = 0,
		AX_Thread_Descriptor *td = 0);

	/// Remove thread from the table.
	void remove_thr (AX_Thread_Descriptor *td,
		int close_handler);

	/// Remove all threads from the table.
	void remove_thr_all (void);

	// = The following four methods implement a simple scheme for
	// operating on a collection of threads atomically.

	/**
	* Efficiently check whether <thread> is in a particular <state>.
	* This call updates the TSS cache if possible to speed up
	* subsequent searches.
	*/
	int check_state (int state,
		AX_thread_t thread,
		int enable = 1);

	/// Apply <func> to all members of the table that match the <task>
	//int apply_task (ACE_Task_Base *task,
	//	ACE_THR_MEMBER_FUNC,
	//	int = 0);

	/// Apply <func> to all members of the table that match the <grp_id>.
	int apply_grp (int grp_id,
		AX_THR_MEMBER_FUNC func,
		int arg = 0);

	/// Apply <func> to all members of the table.
	int apply_all (AX_THR_MEMBER_FUNC,
		int  = 0);
	int cancel_thr (AX_Thread_Descriptor *td, int async_cancel);

	/// Join the thread described in <tda>.
	int join_thr (AX_Thread_Descriptor *td,
		int = 0);

	/// Resume the thread described in <tda>.
	int resume_thr (AX_Thread_Descriptor *td,
		int = 0);

	/// Suspend the thread described in <tda>.
	int suspend_thr (AX_Thread_Descriptor *td,
		int = 0);

	/// Send signal <signum> to the thread described in <tda>.
	int kill_thr (AX_Thread_Descriptor *td,
		int signum);

	///// Set the cancellation flag for the thread described in <tda>.
	//int cancel_thr (AX_Thread_Descriptor *td,
	//	int async_cancel = 0);

	/// Register a thread as terminated and put it into the <terminated_thr_list_>.
	int register_as_terminated (AX_Thread_Descriptor *td);

	/// Setting the static ACE_TSS_TYPE (ACE_Thread_Exit) *thr_exit_ pointer.
	static int set_thr_exit (AX_Thread_Exit *ptr);

	/**
	* Keeping a list of thread descriptors within the thread manager.
	* Double-linked list enables us to cache the entries in TSS
	* and adding/removing thread descriptor entries without
	* affecting other thread's descriptor entries.
	*/
//	ACE_Double_Linked_List<AX_Thread_Descriptor> thr_list_;
//
//#if !defined (ACE_VXWORKS)
//	/// Collect terminated but not yet joined thread entries.
//	ACE_Double_Linked_List<AX_Thread_Descriptor_Base> terminated_thr_list_;
//#endif /* ACE_VXWORKS */

	/// Collect pointers to thread descriptors of threads to be removed later.
	//ACE_Unbounded_Queue<ACE_Thread_Descriptor*> thr_to_be_removed_;

	/// Keeps track of the next group id to assign.
	int grp_id_;

	/// Set if we want the Thread_Manager to wait on all threads before
	/// being closed, reset otherwise.
	int automatic_wait_;

	// = ACE_Thread_Mutex and condition variable for synchronizing termination.
#if defined (ACE_HAS_THREADS)
	/// Serialize access to the <zero_cond_>.
	AX_Thread_Mutex lock_;

	/// Keep track of when there are no more threads.
	AX_Condition_Thread_Mutex zero_cond_;
#endif /* ACE_HAS_THREADS */

	//ACE_Locked_Free_List<ACE_Thread_Descriptor, ACE_SYNCH_MUTEX> thread_desc_freelist_;

private:
	AX_Double_Linked_List<AX_Thread_Descriptor> AX_Thread_Descriptor_list;
	AX_Double_Linked_List<AX_Thread_Descriptor> thread_desc_freelist_;
	AX_Double_Linked_List<AX_Thread_Descriptor> terminated_thr_list_;
	AX_Double_Linked_List<AX_Thread_Descriptor> thr_list_;
	AX_Double_Linked_List<AX_Thread_Descriptor> thr_to_be_removed_;
#if ! defined (ACE_THREAD_MANAGER_LACKS_STATICS)
	/// Pointer to a process-wide <ACE_Thread_Manager>.
	static AX_Thread_Manager *thr_mgr_;

	/// Must delete the <thr_mgr_> if non-0.
	static int delete_thr_mgr_;

	/// Global ACE_TSS (ACE_Thread_Exit) object ptr.
	static AX_Thread_Exit *thr_exit_;
#endif /* ! defined (ACE_THREAD_MANAGER_LACKS_STATICS) */
};

class AX_Thread_Descriptor_Base : public AX_OS_Thread_Descriptor
{

	friend class AX_Thread_Manager;
	friend class AX_Double_Linked_List<AX_Thread_Descriptor_Base>;
	friend class AX_Double_Linked_List_Iterator_Base<AX_Thread_Descriptor_Base>;
	friend class AX_Double_Linked_List_Iterator<AX_Thread_Descriptor_Base>;
	friend class AX_Double_Linked_List<AX_Thread_Descriptor>;
	friend class AX_Double_Linked_List_Iterator_Base<AX_Thread_Descriptor>;
	friend class AX_Double_Linked_List_Iterator<AX_Thread_Descriptor>;
public:
	AX_Thread_Descriptor_Base (void);
	~AX_Thread_Descriptor_Base (void);

	// = We need the following operators to make Borland happy.

	/// Equality operator.
	bool operator== (const AX_Thread_Descriptor_Base &rhs) const;

	/// Inequality operator.
	bool operator!= (const AX_Thread_Descriptor_Base &rhs) const;

	/// Group ID.
	int grp_id (void) const;

	/// Current state of the thread.
	int state (void) const;

	/// Return the pointer to an <ACE_Task_Base> or NULL if there's no
	/// <ACE_Task_Base> associated with this thread.;
	//ACE_Task_Base *task (void) const;

protected:
	/// Reset this base thread descriptor.
	void reset (void);

	/// Unique thread ID.
	AX_thread_t thr_id_;

	/// Unique handle to thread (used by Win32 and AIX).
	AX_hthread_t thr_handle_;

	/// Group ID.
	int grp_id_;

	/// Current state of the thread.
	int thr_state_;

	/// Pointer to an <ACE_Task_Base> or NULL if there's no
	/// <ACE_Task_Base>.
	//ACE_Task_Base *task_;

	/// We need these pointers to maintain the double-linked list in a
	/// thread managers.
	AX_Thread_Descriptor_Base *next_;
	AX_Thread_Descriptor_Base *prev_;
};
class AX_Thread_Descriptor :public AX_Thread_Descriptor_Base
{
	friend class AX_At_Thread_Exit;
	friend class AX_Thread_Manager;
public:
	// = Initialization method.
	AX_Thread_Descriptor (void);

	// = Accessor methods.
	/// Unique thread id.
	AX_thread_t self (void) const;

	/// Unique handle to thread (used by Win32 and AIX).
	void self (AX_hthread_t &);

	/// Dump the state of an object.
	void dump (void) const;

#if !defined(ACE_USE_ONE_SHOT_AT_THREAD_EXIT)

	/**
	* Register an At_Thread_Exit hook and the ownership is acquire by
	* Thread_Descriptor, this is the usual case when the AT is dynamically
	* allocated.
	*/
	int at_exit (AX_At_Thread_Exit* cleanup);

	/// Register an At_Thread_Exit hook and the ownership is retained for the
	/// caller. Normally used when the at_exit hook is created in stack.
	int at_exit (AX_At_Thread_Exit& cleanup);
#endif /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */

	/**
	* Register an object (or array) for cleanup at thread termination.
	* "cleanup_hook" points to a (global, or static member) function
	* that is called for the object or array when it to be destroyed.
	* It may perform any necessary cleanup specific for that object or
	* its class.  "param" is passed as the second parameter to the
	* "cleanup_hook" function; the first parameter is the object (or
	* array) to be destroyed.  Returns 0 on success, non-zero on
	* failure: -1 if virtual memory is exhausted or 1 if the object (or
	* arrayt) had already been registered.
	*/
	int at_exit (void *object,
		AX_CLEANUP_FUNC cleanup_hook,
		void *param);

	/// Do nothing destructor to keep some compilers happy
	~AX_Thread_Descriptor (void);

	/**
	* Do nothing but to acquire the thread descriptor's lock and
	* release.  This will first check if the thread is registered or
	* not.  If it is already registered, there's no need to reacquire
	* the lock again.  This is used mainly to get newly spawned thread
	* in synch with thread manager and prevent it from accessing its
	* thread descriptor before it gets fully built.  This function is
	* only called from ACE_Log_Msg::thr_desc.
	*/
	//void acquire_release (void);
	//void acquire (void);
	//void release (void);

	/**
	* Set/get the @c next_ pointer.  These are required by the
	* ACE_Free_List.
	*/
	void set_next (AX_Thread_Descriptor *td);
	AX_Thread_Descriptor *get_next (void) const;

private:
	/// Reset this thread descriptor.
	void reset (AX_Thread_Manager *tm);

#if !defined (ACE_USE_ONE_SHOT_AT_THREAD_EXIT)
	/// Pop an At_Thread_Exit from at thread termination list, apply the at
	/// if apply is true.
	void at_pop (int apply = 1);

	/// Push an At_Thread_Exit to at thread termination list and set the
	/// ownership of at.
	void at_push (AX_At_Thread_Exit* cleanup,
		int is_owner = 0);

	/// Run the AT_Thread_Exit hooks.
	void do_at_exit (void);

	/// Terminate realize the cleanup process to thread termination
	void terminate (void);


	/// The AT_Thread_Exit list
	AX_At_Thread_Exit *at_exit_list_;
#endif  /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */

	/**
	* Stores the cleanup info for a thread.
	* @note This should be generalized to be a stack of ACE_Cleanup_Info's.
	*/
	//AX_Cleanup_Info cleanup_info_;


	AX_Thread_Manager* tm_;

	///// Registration lock to prevent premature removal of thread descriptor.
	//ACE_DEFAULT_THREAD_MANAGER_LOCK *sync_;

	///// Keep track of termination status.
	bool terminated_;
};

#include "AX_Thread_Manager.inl"
#endif
