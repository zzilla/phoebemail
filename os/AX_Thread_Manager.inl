
#define AX_FIND(OP,INDEX) \
	AX_Thread_Descriptor *INDEX = OP;
////////////////////////////////////////////////////////////////////////////////////////////////

inline int
AX_At_Thread_Exit::was_applied() const

{
	return was_applied_;
}

inline int
AX_At_Thread_Exit::was_applied (int applied)
{
	was_applied_ = applied;
	if (was_applied_)
		td_ = 0;
	return was_applied_;
}

inline int
AX_At_Thread_Exit::is_owner() const
{
	return is_owner_;
}

inline int
AX_At_Thread_Exit::is_owner (int owner)
{
	is_owner_ = owner;
	return is_owner_;
}

inline void
AX_At_Thread_Exit::do_apply (void)
{
	if (!this->was_applied_ && this->is_owner_)
		td_->at_pop();
}
/////////////////////////////////////////////////////////////////////////////////////////////
inline
AX_Thread_Descriptor_Base::AX_Thread_Descriptor_Base (void)
: AX_OS_Thread_Descriptor (),
thr_id_ (0),
thr_handle_ (0),
grp_id_ (0),
thr_state_ (AX_Thread_Manager::AX_THR_IDLE),
//task_ (0),
next_ (0),
prev_ (0)
{
}

inline
AX_Thread_Descriptor_Base::~AX_Thread_Descriptor_Base (void)
{
}

inline bool
AX_Thread_Descriptor_Base::operator== (
										const AX_Thread_Descriptor_Base &rhs) const
{
	return
		AX_OS::thr_cmp (this->thr_handle_, rhs.thr_handle_)
		&& AX_OS::thr_equal (this->thr_id_, rhs.thr_id_);
}

inline bool
AX_Thread_Descriptor_Base::operator!=(const AX_Thread_Descriptor_Base &rhs) const
{
	return !(*this == rhs);
}

// Group ID.

inline int
AX_Thread_Descriptor_Base::grp_id (void) const
{
	//printf ("ACE_Thread_Descriptor_Base::grp_id");
	return grp_id_;
}

// Current state of the thread.
inline int
AX_Thread_Descriptor_Base::state (void) const
{
	//printf ("ACE_Thread_Descriptor_Base::state");
	return thr_state_;
}

// Reset this base descriptor.
inline void
AX_Thread_Descriptor_Base::reset (void)
{
	//printf ("ACE_Thread_Descriptor_Base::reset");
	this->thr_id_ = 0;
	this->thr_handle_ = 0;
	this->grp_id_ = 0;
	this->thr_state_ = AX_Thread_Manager::AX_THR_IDLE;
	//this->task_ = 0;
	this->flags_ = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void
AX_Thread_Descriptor::self (AX_hthread_t &handle)
{
	//printf ("ACE_Thread_Descriptor::self");
	handle = this->thr_handle_;
}

// Unique kernel-level thread handle.

// Set the <next_> pointer
inline void
AX_Thread_Descriptor::set_next (AX_Thread_Descriptor *td)
{
	//printf("ACE_Thread_Descriptor::set_next");
	this->next_ = td;
}

// Get the <next_> pointer
inline AX_Thread_Descriptor *
AX_Thread_Descriptor::get_next (void) const
{
	printf("ACE_Thread_Descriptor::get_next");
	return static_cast<AX_Thread_Descriptor * > (this->next_);
}

// Reset this thread descriptor
inline void
AX_Thread_Descriptor::reset (AX_Thread_Manager *tm)
{
	//printf ("ACE_Thread_Descriptor::reset");
	this->AX_Thread_Descriptor_Base::reset ();
#if defined(ACE_USE_ONE_SHOT_AT_THREAD_EXIT)
	this->cleanup_info_.cleanup_hook_ = 0;
	this->cleanup_info_.object_ = 0;
	this->cleanup_info_.param_ = 0;
#else /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */
	this->at_exit_list_ = 0;
	// Start the at_exit hook list.
	this->tm_ = tm;
	// Setup the Thread_Manager.
	this->terminated_ = false;
#endif /* !ACE_USE_ONE_SHOT_AT_THREAD_EXIT */
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline int
AX_Thread_Manager::thr_state (AX_thread_t id,
							   int& state)
{
	//printf ("ACE_Thread_Manager::thr_state");
	//AX_MT(AX_GUARD_RETURN (AX_Thread_Mutex, ace_mon, this->lock_, -1));

	int self_check = AX_OS::thr_equal (id, AX_OS::thr_self ());

	// If we're checking the state of our thread, try to get the cached
	// value out of TSS to avoid lookup.
	//if (self_check)
	//{
	//	AX_Thread_Descriptor *desc = ACE_LOG_MSG->thr_desc ();
	//	if (desc == 0)
	//		return 0;               // Always return false.
	//	state = desc->thr_state_;
	//}
	//else
	//{
		// Not calling from self, have to look it up from the list.
	AX_FIND (this->find_thread (id), ptr);
	if (ptr == 0)
		return 0;
	state = ptr->thr_state_;
	//}

	return 1;
}


//inline AX_Task_Base *
//AX_Thread_Manager::task (void)
//{
//	printf ("ACE_Thread_Manager::task");
//
//	AX_Thread_Descriptor *td = this->thread_desc_self () ;
//
//	if (td == 0)
//		return 0;
//	else
//		return td->task ();
//}
inline AX_thr_func_return AX_Thread_Manager::exit(AX_thr_func_return status /* = 0 */, int do_thread_exit /* = 1 */)
{
	return (AX_thr_func_return)0;
}

inline int
AX_Thread_Manager::open (size_t)
{
	// Currently no-op.
	return 0;
}

inline int
AX_Thread_Manager::at_exit (AX_At_Thread_Exit& at)
{
	AX_Thread_Descriptor *td = this->thread_desc_self ();
	if (td == 0)
		return -1;
	else
		return td->at_exit (at);
}

inline int
AX_Thread_Manager::at_exit (void *object,
							 AX_CLEANUP_FUNC cleanup_hook,
							 void *param)
{
	AX_Thread_Descriptor *td = this->thread_desc_self ();
	if (td == 0)
		return -1;
	else
		return td->at_exit (object,
		cleanup_hook,
		param);
}

inline void
AX_Thread_Manager::wait_on_exit (int do_wait)
{
	this->automatic_wait_ = do_wait;
}

inline int
AX_Thread_Manager::wait_on_exit (void)
{
	return this->automatic_wait_;
}

inline int
AX_Thread_Manager::register_as_terminated (AX_Thread_Descriptor *td)
{
	return 0;
}

inline size_t
AX_Thread_Manager::count_threads (void) const
{
	return this->thr_list_.size ();
}
inline void AX_Thread_Manager::dump()
{
	return ;
}
