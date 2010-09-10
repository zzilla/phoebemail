#include "AX_Thread_Manager.h"
#include "AX_Thread_Mutex.h"
#include "AX_Thread_Exit.h"
#include "AX_Thread.h"
#include "AX_Thread_Adapter.h"

#define AX_EXECUTE_OP(OP, ARG) \
	AX_MT (ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1)); \
	if (this->thr_to_be_removed_.is_empty ())								\
	{																\
		return -1;													\
	}																\
	AX_FIND (this->find_thread (t_id), ptr); \
	if (ptr == 0) \
	{ \
	errno = ENOENT; \
	return -1; \
	} \
	int result = OP (ptr, ARG); \
	while (! this->thr_to_be_removed_.is_empty ()) { \
	AX_Thread_Descriptor * td = 0; \
	this->thr_to_be_removed_.dequeue_head (td); \
	this->remove_thr (td, 1); \
	} \
	return result

AX_At_Thread_Exit::AX_At_Thread_Exit ()
: next_ (0),
td_ (0),
was_applied_ (0),
is_owner_ (1)
{
}

AX_At_Thread_Exit::~AX_At_Thread_Exit (void)
{
	this->do_apply ();
}

AX_At_Thread_Exit_Func::~AX_At_Thread_Exit_Func (void)
{
	this->do_apply ();
}
// Return the unique ID of the thread.

AX_thread_t
AX_Thread_Manager::thr_self (void)
{
	printf ("ACE_Thread_Manager::thr_self");
	return AX_Thread::self ();
}
void
AX_At_Thread_Exit_Func::apply (void)
{
	this->func_ (this->object_, this->param_);
}
AX_Thread_Descriptor::~AX_Thread_Descriptor()
{

}
void AX_Thread_Descriptor::at_push(AX_At_Thread_Exit* cleanup, int is_owner /* = 0 */)
{
	printf ("ACE_Thread_Descriptor::at_push");
	cleanup->is_owner (is_owner);
	cleanup->td_ = this;
	cleanup->next_ = at_exit_list_;
	at_exit_list_ = cleanup;
}
void AX_Thread_Descriptor::at_pop (int apply)

{
	printf ("ACE_Thread_Descriptor::at_pop");
	// Get first at from at_exit_list
	AX_At_Thread_Exit* at = this->at_exit_list_;
	// Remove at from at_exit list
	this->at_exit_list_ = at->next_;
	// Apply if required
	if (apply)
	{
		at->apply ();
		// Do the apply method
		at->was_applied (1);
		// Mark at has been applied to avoid double apply from
		// at destructor
	}
	// If at is not owner delete at.
	if (!at->is_owner ())
		delete at;
}
int
AX_Thread_Descriptor::at_exit (AX_At_Thread_Exit& cleanup)
{
	printf ("ACE_Thread_Descriptor::at_exit");
	at_push (&cleanup, 1);
	return 0;
}

int
AX_Thread_Descriptor::at_exit (AX_At_Thread_Exit* cleanup)
{
	printf ("ACE_Thread_Descriptor::at_exit");
	if (cleanup==0)
		return -1;
	else
	{
		this->at_push (cleanup);
		return 0;
	}
}
void AX_Thread_Descriptor::do_at_exit()
{
	if (tm_ != NULL)
	{
		tm_->run_thread_exit_hooks(0);
	}
	return ;
}
void
AX_Thread_Descriptor::terminate ()

{
	printf ("ACE_Thread_Descriptor::terminate");

	if (!terminated_)
	{
		//ACE_Log_Msg* log_msg = this->log_msg_;
		terminated_ = true;
		// Run at_exit hooks
		this->do_at_exit ();
		// We must remove Thread_Descriptor from Thread_Manager list
		if (this->tm_ != 0)
		{
			int close_handle = 0;

#if !defined (ACE_VXWORKS)
			// Threads created with THR_DAEMON shouldn't exist here, but
			// just to be safe, let's put it here.

			if (AX_BIT_DISABLED (this->thr_state_, AX_Thread_Manager::AX_THR_JOINING))
			{
				if (AX_BIT_DISABLED (this->flags_, THR_DETACHED | THR_DAEMON)
					|| AX_BIT_ENABLED (this->flags_, THR_JOINABLE))
				{
					// Mark thread as terminated.
					AX_SET_BITS (this->thr_state_, AX_Thread_Manager::AX_THR_TERMINATED);
					tm_->register_as_terminated (this);
					// Must copy the information here because td will be
					// "freed" below.
				}
#if defined (_WIN32)
				else
				{
					close_handle = 1;
				}
#endif /* ACE_WIN32 */
			}
#endif /* ! ACE_VXWORKS */

			// Remove thread descriptor from the table.
			if (this->tm_ != 0)
				tm_->remove_thr (this, close_handle);
		}

		// Check if we need delete ACE_Log_Msg instance
		// If ACE_TSS_cleanup was not executed first log_msg == 0
		//if (log_msg == 0)
		//{
		//	// Only inform to ACE_TSS_cleanup that it must delete the log instance
		//	// setting ACE_LOG_MSG thr_desc to 0.
		//	ACE_LOG_MSG->thr_desc (0);
		//}
		//else
		//{
		//	// Thread_Descriptor is the owner of the Log_Msg instance!!
		//	// deleted.
		//	this->log_msg_ = 0;
		//	delete log_msg;
		//}
	}
}
AX_Thread_Manager *AX_Thread_Manager::thr_mgr_ =NULL;

// Controls whether the Thread_Manager is deleted when we shut down
// (we can only delete it safely if we created it!)
int AX_Thread_Manager::delete_thr_mgr_ = NULL;

AX_Thread_Exit *AX_Thread_Manager::thr_exit_ = 0;

int
AX_Thread_Manager::set_thr_exit (AX_Thread_Exit *ptr)
{
	if (AX_Thread_Manager::thr_exit_ == 0)
		AX_Thread_Manager::thr_exit_ = ptr;
	else
		return -1;
	return 0;
}
#if ! defined (ACE_THREAD_MANAGER_LACKS_STATICS)
AX_Thread_Manager *
AX_Thread_Manager::instance (void)
{
	printf("ACE_Thread_Manager::instance");

	if (AX_Thread_Manager::thr_mgr_ == 0)
	{
		// Perform Double-Checked Locking Optimization.
		AX_MT(ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon,
			*ACE_Static_Object_Lock::instance (), 0));

		if (AX_Thread_Manager::thr_mgr_ == 0)
		{
			AX_NEW_RETURN (AX_Thread_Manager::thr_mgr_,
				AX_Thread_Manager,
				0);
			AX_Thread_Manager::delete_thr_mgr_ = 1;
		}
	}

	return AX_Thread_Manager::thr_mgr_;
}

AX_Thread_Manager *
AX_Thread_Manager::instance (AX_Thread_Manager *tm)
{
	printf ("ACE_Thread_Manager::instance");
	//ACE_GUARD_RETURN (AX_Recursive_Thread_Mutex, ace_mon,
	//	*ACE_Static_Object_Lock::instance (), 0);

	AX_Thread_Manager *t = AX_Thread_Manager::thr_mgr_;
	// We can't safely delete it since we don't know who created it!
	AX_Thread_Manager::delete_thr_mgr_ = 0;

	AX_Thread_Manager::thr_mgr_ = tm;
	return t;
}

void
AX_Thread_Manager::close_singleton (void)
{
	printf ("ACE_Thread_Manager::close_singleton");

	//ACE_GUARD (ACE_Recursive_Thread_Mutex, ace_mon,
	//	*ACE_Static_Object_Lock::instance ());

	if (AX_Thread_Manager::delete_thr_mgr_)
	{
		// First, we clean up the thread descriptor list.
		AX_Thread_Manager::thr_mgr_->close ();
		delete AX_Thread_Manager::thr_mgr_;
		AX_Thread_Manager::thr_mgr_ = 0;
		AX_Thread_Manager::delete_thr_mgr_ = 0;
	}

	AX_Thread_Exit::cleanup (AX_Thread_Manager::thr_exit_);
}
int
AX_Thread_Manager::spawn_i (AX_THR_FUNC func,
							 void *args,
							 long flags,
							 AX_thread_t *t_id,
							 AX_hthread_t *t_handle,
							 long priority,
							 int grp_id,
							 void *stack,
							 size_t stack_size)
{
	// First, threads created by Thread Manager should not be daemon threads.
	// Using assertion is probably a bit too strong.  However, it helps
	// finding this kind of error as early as possible.  Perhaps we can replace
	// assertion by returning error.
	AX_BIT_DISABLED (flags, THR_DAEMON);

	// Create a new thread running <func>.  *Must* be called with the
	// <lock_> held...
	// Get a "new" Thread Descriptor from the freelist.
	auto_ptr<AX_Thread_Descriptor> new_thr_desc (this->thread_desc_freelist_.remove ());

	// Reset thread descriptor status
	new_thr_desc->reset (this);

	AX_Thread_Adapter *thread_args = 0;
//# if defined (ACE_HAS_WIN32_STRUCTURAL_EXCEPTIONS)
//	AX_NEW_RETURN (thread_args,
//		ACE_Thread_Adapter (func,
//		args,
//		(ACE_THR_C_FUNC) ACE_THREAD_ADAPTER_NAME,
//		this,
//		new_thr_desc.get (),
//		AX_OS_Object_Manager::seh_except_selector(),
//		AX_OS_Object_Manager::seh_except_handler()),
//		-1);
//# else
//	AX_NEW_RETURN (thread_args,
//		AX_Thread_Adapter (func,
//		args,
//		//(AX_THR_C_FUNC) ACE_THREAD_ADAPTER_NAME,
//		(AX_THR_C_FUNC) NULL,
//		this,
//		new_thr_desc.get ()),
//		-1);
//# endif

	printf ("ACE_Thread_Manager::spawn_i");
	AX_hthread_t thr_handle;

	AX_thread_t thr_id;
	if (t_id == 0)
		t_id = &thr_id;

	//new_thr_desc->sync_->acquire ();
	// Acquire the <sync_> lock to block the spawned thread from
	// removing this Thread Descriptor before it gets put into our
	// thread table.

	int const result = AX_Thread::spawn (func,
		args,
		flags,
		t_id,
		&thr_handle,
		priority,
		stack,
		stack_size);
		//thread_args);

	if (result != 0)
	{
		//// _Don't_ clobber errno here!  result is either 0 or -1, and
		//// ACE_OS::thr_create () already set errno!  D. Levine 28 Mar 1997
		//// errno = result;
		//ACE_Errno_Guard guard (errno);     // Lock release may smash errno
		//new_thr_desc->sync_->release ();
		return -1;
	}

#if defined (_WIN32)
	// Have to duplicate handle if client asks for it.
	// @@ How are thread handles implemented on AIX?  Do they
	// also need to be duplicated?
	if (t_handle != 0)
		(void) ::DuplicateHandle (::GetCurrentProcess (),
		thr_handle,
		::GetCurrentProcess (),
		t_handle,
		0,
		TRUE,
		DUPLICATE_SAME_ACCESS);
#else  /* ! ACE_HAS_WTHREADS */
	if (t_handle != 0)
		*t_handle = thr_handle;
#endif /* ! ACE_HAS_WTHREADS */

	// append_thr also put the <new_thr_desc> into Thread_Manager's
	// double-linked list.  Only after this point, can we manipulate
	// double-linked list from a spawned thread's context.
	return this->append_thr (*t_id,
		thr_handle,
		AX_THR_SPAWNED,
		grp_id,
		flags,
		new_thr_desc.release ());
}

int
AX_Thread_Manager::spawn (AX_THR_FUNC func,
						   void *args,
						   long flags,
						   AX_thread_t *t_id,
						   AX_hthread_t *t_handle,
						   long priority,
						   int grp_id,
						   void *stack,
						   size_t stack_size)
{
	printf ("ACE_Thread_Manager::spawn");

	AX_MT(ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1));

	if (grp_id == -1)
		grp_id = this->grp_id_++; // Increment the group id.

	if (priority != AX_DEFAULT_THREAD_PRIORITY)
		AX_CLR_BITS (flags, THR_INHERIT_SCHED);

	if (this->spawn_i (func,
		args,
		flags,
		t_id,
		t_handle,
		priority,
		grp_id,
		stack,
		stack_size) == -1)
		return -1;

	return grp_id;
}
int
AX_Thread_Manager::join (AX_thread_t tid, AX_thr_func_return *status)
{
	printf ("ACE_Thread_Manager::join");

	AX_Thread_Descriptor tdb;
	int found = 0;

	{
		AX_MT(ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1));

#if !defined (ACE_VXWORKS)
		for (AX_Double_Linked_List_Iterator<AX_Thread_Descriptor> biter(this->terminated_thr_list_);
			!biter.done ();
			biter.advance ())
			if (AX_OS::thr_equal (biter.next ()->thr_id_, tid))
			{
				AX_Thread_Descriptor_Base *tdb = biter.advance_and_remove (0);
				if (AX_Thread::join (tdb->thr_handle_, status) == -1)
					return -1;

# if defined (ACE_HAS_PTHREADS_DRAFT4)  &&  defined (ACE_LACKS_SETDETACH)
				// Must explicitly detach threads.  Threads without THR_DETACHED
				// were detached in ACE_OS::thr_create ().
				::pthread_detach (&tdb->thr_handle_);
# endif /* ACE_HAS_PTHREADS_DRAFT4 && ACE_LACKS_SETDETACH */

				delete tdb;
				return 0;
				// return immediately if we've found the thread we want to join.
			}
#endif /* !ACE_VXWORKS */

			for (AX_Double_Linked_List_Iterator<AX_Thread_Descriptor> iter (this->thr_list_);
				!iter.done ();
				iter.advance ())
				// If threads are created as THR_DETACHED or THR_DAEMON, we
				// can't help much.
				if (AX_OS::thr_equal (iter.next ()->thr_id_,tid) &&
					(AX_BIT_DISABLED (iter.next ()->flags_, THR_DETACHED | THR_DAEMON)
					|| AX_BIT_ENABLED (iter.next ()->flags_, THR_JOINABLE)))
				{
					tdb = *iter.next ();
					AX_SET_BITS (iter.next ()->thr_state_, AX_THR_JOINING);
					found = 1;
					break;
				}

				if (found == 0)
					return -1;
				// Didn't find the thread we want or the thread is not joinable.
	}

	if (AX_Thread::join (tdb.thr_handle_, status) == -1)
		return -1;

# if defined (ACE_HAS_PTHREADS_DRAFT4)  &&  defined (ACE_LACKS_SETDETACH)
	// Must explicitly detach threads.  Threads without THR_DETACHED
	// were detached in ACE_OS::thr_create ().

	::pthread_detach (&tdb.thr_handle_);
# endif /* ACE_HAS_PTHREADS_DRAFT4 && ACE_LACKS_SETDETACH */
	return 0;
}
int
AX_Thread_Manager::wait (const timeval *timeout,
						  bool abandon_detached_threads,
						  bool use_absolute_time)
{
	printf ("ACE_Thread_Manager::wait");

	timeval local_timeout;
	// Check to see if we're using absolute time or not.
	if (use_absolute_time == false && timeout != 0)
	{
		local_timeout = *timeout;
		uint64 time = AX_OS::get_time();
		local_timeout.tv_usec = (long)time%1000000;
		local_timeout.tv_sec = (long)time/1000000;
		//local_timeout += AX_OS::getTime();
		timeout = &local_timeout;
	}

#if !defined (ACE_VXWORKS)
	AX_Double_Linked_List<AX_Thread_Descriptor_Base> term_thr_list_copy;
#endif /* ACE_VXWORKS */

#if defined (ACE_HAS_THREADS)
	{
		// Just hold onto the guard while waiting.
		AX_MT(ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, this->lock_, -1));

		if (ACE_Object_Manager::shutting_down () != 1)
		{
			// Program is not shutting down.  Perform a normal wait on threads.
			if (abandon_detached_threads != 0)
			{
				//ACE_ASSERT (this->thr_to_be_removed_.is_empty ());
				for (AX_Double_Linked_List_Iterator<AX_Thread_Descriptor>
					iter (this->thr_list_);
					!iter.done ();
				iter.advance ())
					if (AX_BIT_ENABLED (iter.next ()->flags_,
						THR_DETACHED | THR_DAEMON)
						&& AX_BIT_DISABLED (iter.next ()->flags_, THR_JOINABLE))
					{
						this->thr_to_be_removed_.enqueue_tail (iter.next ());
						AX_SET_BITS (iter.next ()->thr_state_, AX_THR_JOINING);
					}

					if (! this->thr_to_be_removed_.is_empty ())
					{
						AX_Thread_Descriptor *td;
						while (this->thr_to_be_removed_.dequeue_head (td) != -1)
							this->remove_thr (td, 1);
					}
			}

			while (this->thr_list_.size () > 0)
				if (this->zero_cond_.wait (timeout) == -1)
					return -1;
		}
		else
			// Program is shutting down, no chance to wait on threads.
			// Therefore, we'll just remove threads from the list.
			this->remove_thr_all ();

#if !defined (ACE_VXWORKS)
		AX_Thread_Descriptor_Base* item = 0;
		while ((item = this->terminated_thr_list_.delete_head ()) != 0)
		{
			term_thr_list_copy.insert_tail (item);
		}
#endif /* ACE_VXWORKS */
		// Release the guard, giving other threads a chance to run.
	}

#if !defined (ACE_VXWORKS)
	// @@ VxWorks doesn't support thr_join (yet.)  We are working
	//on our implementation.   Chorus'es thr_join seems broken.
	AX_Thread_Descriptor_Base *item = 0;

	while ((item = term_thr_list_copy.delete_head ()) != 0)
	{
		if (AX_BIT_DISABLED (item->flags_, THR_DETACHED | THR_DAEMON)
			|| AX_BIT_ENABLED (item->flags_, THR_JOINABLE))
			// Detached handles shouldn't reached here.
			AX_Thread::join (item->thr_handle_);

# if defined (ACE_HAS_PTHREADS_DRAFT4)  &&  defined (ACE_LACKS_SETDETACH)
		// Must explicitly detach threads.  Threads without
		// THR_DETACHED were detached in ACE_OS::thr_create ().
		::pthread_detach (&item->thr_handle_);
# endif /* ACE_HAS_PTHREADS_DRAFT4 && ACE_LACKS_SETDETACH */
		delete item;
	}

#endif /* ! ACE_VXWORKS */
#else
	//ACE_UNUSED_ARG (timeout);
	//ACE_UNUSED_ARG (abandon_detached_threads);
#endif /* ACE_HAS_THREADS */

	return 0;
}
AX_Thread_Descriptor *
AX_Thread_Manager::find_hthread (AX_hthread_t h_id)
{
	for (AX_Double_Linked_List_Iterator<AX_Thread_Descriptor> iter (this->thr_list_);
		!iter.done ();
		iter.advance ())
		if (AX_OS::thr_cmp (iter.next ()->thr_handle_, h_id))
			return iter.next ();

	return 0;
}
AX_Thread_Descriptor *
AX_Thread_Manager::find_thread (AX_thread_t t_id)
{
	printf ("ACE_Thread_Manager::find_thread");

	//for (ACE_Double_Linked_List_Iterator<AX_Thread_Descriptor> iter (this->thr_list_);
	//	!iter.done ();
	//	iter.advance ())

	for (AX_Double_Linked_List_Iterator<AX_Thread_Descriptor> iter (this->thr_list_);
		!iter.done ();
		iter.advance ())
		if (AX_OS::thr_equal (iter.next ()->thr_id_, t_id))
			return iter.next ();
	return 0;
}

int
AX_Thread_Manager::insert_thr (AX_thread_t t_id,
								AX_hthread_t t_handle,
								int grp_id,
								long flags)
{
	printf ("ACE_Thread_Manager::insert_thr");
	AX_MT(ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1));

	// Check for duplicates and bail out if we're already registered...
//#if defined (ACE_VXWORKS) && !defined (ACE_HAS_PTHREADS)
//	if (this->find_hthread (t_handle) != 0 )
//		return -1;
//#else  /* ! ACE_VXWORKS */
	if (this->find_thread (t_id) != 0 )
		return -1;
//#endif /* ! ACE_VXWORKS */

	if (grp_id == -1)
		grp_id = this->grp_id_++;

	if (this->append_thr (t_id,
		t_handle,
		AX_THR_SPAWNED,
		grp_id,
		flags) == -1)
		return -1;

	return grp_id;
}
int
AX_Thread_Manager::join_thr (AX_Thread_Descriptor *td, int)
{
	printf ("ACE_Thread_Manager::join_thr");
	int result = AX_Thread::join (td->thr_handle_);
	if (result != 0)
	{
		// Since the thread are being joined, we should
		// let it remove itself from the list.

		//      this->remove_thr (td);
		errno = result;
		return -1;
	}

	return 0;
}

int
AX_Thread_Manager::suspend_thr (AX_Thread_Descriptor *td, int)
{
	printf ("ACE_Thread_Manager::suspend_thr");

	int const result = AX_Thread::suspend (td->thr_handle_);
	if (result == -1) {
		if (errno != ENOTSUP)
			this->thr_to_be_removed_.enqueue_tail (td);
		return -1;
	}
	else {
		AX_SET_BITS (td->thr_state_, AX_THR_SUSPENDED);
		return 0;
	}
}

int
AX_Thread_Manager::resume_thr (AX_Thread_Descriptor *td, int)
{
	printf ("ACE_Thread_Manager::resume_thr");

	int const result = AX_Thread::resume (td->thr_handle_);
	if (result == -1) {
		if (errno != ENOTSUP)
			this->thr_to_be_removed_.enqueue_tail (td);
		return -1;
	}
	else {
		AX_CLR_BITS (td->thr_state_, AX_THR_SUSPENDED);
		return 0;
	}
}

//int
//AX_Thread_Manager::cancel_thr (AX_Thread_Descriptor *td, int async_cancel)
//{
//	printf ("ACE_Thread_Manager::cancel_thr");
//	// Must set the state first and then try to cancel the thread.
//	AX_SET_BITS (td->thr_state_, AX_THR_CANCELLED);
//
//	if (async_cancel != 0)
//		// Note that this call only does something relevant if the OS
//		// platform supports asynchronous thread cancellation.  Otherwise,
//		// it's a no-op.
//		return AX_Thread::cancel (td->thr_id_);
//
//	return 0;
//}

int
AX_Thread_Manager::kill_thr (AX_Thread_Descriptor *td, int signum)
{
	printf ("ACE_Thread_Manager::kill_thr");

	AX_thread_t tid = td->thr_id_;
//#if defined (ACE_VXWORKS) && !defined (ACE_HAS_PTHREADS)
//	// Skip over the ID-allocated marker, if present.
//	tid += tid[0] == ACE_THR_ID_ALLOCATED  ?  1  :  0;
//#endif /* ACE_VXWORKS */

	int const result = AX_Thread::kill (tid, signum);

	if (result != 0)
	{
		// Only remove a thread from us when there is a "real" error.
		if (errno != ENOTSUP)
			this->thr_to_be_removed_.enqueue_tail (td);

		return -1;
	}

	return 0;
}

// Run the registered hooks when the thread exits.

void
AX_Thread_Manager::run_thread_exit_hooks (int i)
{
#if 0 // currently unused!
	ACE_TRACE ("ACE_Thread_Manager::run_thread_exit_hooks");

	// @@ Currently, we have just one hook.  This should clearly be
	// generalized to support an arbitrary number of hooks.

	ACE_Thread_Descriptor *td = this->thread_desc_self ();
	if (td != 0 && td->cleanup_info.cleanup_hook_ != 0)
	{
		(*td->cleanup_info_.cleanup_hook_)
			(td->cleanup_info_.object_,
			td->cleanup_info_.param_);

		td->cleanup_info_.cleanup_hook_ = 0;
	}
	ACE_UNUSED_ARG (i);
#else
	//ACE_UNUSED_ARG (i);
#endif /* 0 */
}

// Remove a thread from the pool.  Must be called with locks held.

void
AX_Thread_Manager::remove_thr (AX_Thread_Descriptor *td,
								int close_handler)
{
	printf ("ACE_Thread_Manager::remove_thr");

#if !defined(ACE_USE_ONE_SHOT_AT_THREAD_EXIT)
	td->tm_ = 0;
#endif /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */
	this->thr_list_.remove (td);


#if defined (_WIN32)
	if (close_handler != 0)
		::CloseHandle (td->thr_handle_);
#endif /* ACE_WIN32 */

	this->thread_desc_freelist_.add (td);

#if defined (ACE_HAS_THREADS)
	// Tell all waiters when there are no more threads left in the pool.
	//if (this->thr_list_.size () == 0)
	if (this->AX_Thread_Descriptor_list.size () == 0)
		this->zero_cond_.broadcast ();
#endif /* ACE_HAS_THREADS */
}

void
AX_Thread_Manager::remove_thr_all (void)
{
	AX_Thread_Descriptor *td = 0;

	while ((td = this->thr_list_.delete_head ()) != 0)
	{
		this->remove_thr (td, 1);
	}
}

ssize_t
AX_Thread_Manager::thread_all_list (AX_thread_t thread_list[],
									 size_t n)
{
	printf ("ACE_Thread_Manager::thread_all_list");
	AX_MT(AX_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1));

	size_t thread_count = 0;

	for (AX_Double_Linked_List_Iterator<AX_Thread_Descriptor> iter (this->thr_list_);
		!iter.done ();
		iter.advance ())
	{
		if (thread_count >= n)
			break;

		thread_list[thread_count] = iter.next ()->thr_id_;
		thread_count ++;
	}

	return (ssize_t)thread_count;
}
AX_Thread_Manager::AX_Thread_Manager(size_t preaolloc /* = 0  */, size_t lwm /* = 0  */, size_t inc /* = 0 */, size_t hwm /* = 0 */)
{

}

AX_Thread_Manager::~AX_Thread_Manager (void)
{
	printf ("ACE_Thread_Manager::~ACE_Thread_Manager");
	this->close ();
}

int
AX_Thread_Manager::close ()
{
	printf ("ACE_Thread_Manager::close");

	// Clean up the thread descriptor list.
	if (this->automatic_wait_)
		this->wait (0, 1);
	else
	{
		AX_MT (ACE_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1));

		this->remove_thr_all ();
	}

	return 0;
}

int
AX_Thread_Manager::cancel_thr (AX_Thread_Descriptor *td, int async_cancel)
{
	printf ("ACE_Thread_Manager::cancel_thr");
	// Must set the state first and then try to cancel the thread.
	AX_SET_BITS (td->thr_state_, AX_THR_CANCELLED);

	if (async_cancel != 0)
		// Note that this call only does something relevant if the OS
		// platform supports asynchronous thread cancellation.  Otherwise,
		// it's a no-op.
		return 1;

	return 0;
}
int
AX_Thread_Manager::append_thr (AX_thread_t t_id,
								AX_hthread_t t_handle,
								int thr_state,
								int grp_id,
								//ACE_Task_Base *task,
								long flags,
								AX_Thread_Descriptor *td)
{
	printf ("ACE_Thread_Manager::append_thr");
	AX_Thread_Descriptor *thr_desc = 0;

	if (td == 0)
	{
		AX_NEW_RETURN (thr_desc,
			AX_Thread_Descriptor,
			-1);
#if !defined(ACE_USE_ONE_SHOT_AT_THREAD_EXIT)
		thr_desc->tm_ = this;
		// Setup the Thread_Manager.
#endif /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */
	}
	else
		thr_desc = td;

	thr_desc->thr_id_ = t_id;
	thr_desc->thr_handle_ = t_handle;
	thr_desc->grp_id_ = grp_id;
	//thr_desc->task_ = task;
	thr_desc->flags_ = flags;

	this->thr_list_.insert_head (thr_desc);
	AX_SET_BITS (thr_desc->thr_state_, thr_state);
	//thr_desc->sync_->release ();

	return 0;
}

int
AX_Thread_Manager::cancel (AX_thread_t t_id, int async_cancel)
{
	printf ("ACE_Thread_Manager::cancel");
	AX_EXECUTE_OP (this->cancel_thr, async_cancel);
}

// Send a signal to a single thread.

int
AX_Thread_Manager::kill (AX_thread_t t_id, int signum)
{
	printf ("ACE_Thread_Manager::kill");
	AX_EXECUTE_OP (this->kill_thr, signum);
}
#endif /* ! defined (ACE_THREAD_MANAGER_LACKS_STATICS) */

