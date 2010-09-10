#include "AX_Thread_Exit.h"


u_int AX_Thread_Exit::is_constructed_ = 0;

void
AX_Thread_Exit::cleanup (void *instance)
{
	//printf ("ACE_Thread_Exit::cleanup");

	delete ( AX_Thread_Exit*) instance;

	// Set the thr_exit_ static to null to keep things from crashing if
	// ACE::fini() is enabled here.
	//AX_Thread_Manager::thr_exit_ = 0;

	AX_Thread_Exit::is_constructed_ = 0;
	// All TSS objects have been destroyed.  Reset this flag so
	// ACE_Thread_Exit singleton can be created again.
}

// NOTE: this preprocessor directive should match the one in
// ACE_Task_Base::svc_run () below.  This prevents the two statics
// from being defined.
// 
// AX_Thread_Exit *
// AX_Thread_Exit::instance (void)
// {
// #if defined (ACE_HAS_THREAD_SPECIFIC_STORAGE) || defined (ACE_HAS_TSS_EMULATION)
// 	printf("ACE_Thread_Exit::instance");
// 
// 	// Determines if we were dynamically allocated.
// 	static AX_Thread_Exit *instance_;
// 
// 	// Implement the Double Check pattern.
// 
// 	if (AX_Thread_Exit::is_constructed_ == 0)
// 	{
// 		//ACE_MT (ACE_Thread_Mutex *lock =
// 		//	ACE_Managed_Object<ACE_Thread_Mutex>::get_preallocated_object
// 		//	(ACE_Object_Manager::ACE_THREAD_EXIT_LOCK);
// 		//ACE_GUARD_RETURN (ACE_Thread_Mutex, ace_mon, *lock, 0));
// 		AX_Thread_Mutex ace_mon(*lock);
// 
// 		if (AX_Thread_Exit::is_constructed_ == 0)
// 		{
// 			//ACE_NEW_RETURN (instance_,
// 			//	AX_Thread_Exit,
// 			//	0);
// 			instance_ = new AX_Thread_Exit;
// 			if (instance_ == NULL)
// 			{
// 				return NULL;
// 			}
// 			
// 
// 			AX_Thread_Exit::is_constructed_ = 1;
// 
// 			AX_Thread_Manager::set_thr_exit (instance_);
// 		}
// 	}
// 
// 	return (AX_Thread_Exit)instance_;
// #else
// 	return 0;
// #endif /* ACE_HAS_THREAD_SPECIFIC_STORAGE || ACE_HAS_TSS_EMULATION */
// }

// Grab hold of the Task * so that we can close() it in the
// destructor.

AX_Thread_Exit::AX_Thread_Exit (void)
{
	//printf ("ACE_Thread_Exit::ACE_Thread_Exit");
#ifndef _WIN32
	size_ = 0;
#endif
}

// Set the this pointer...

//void
//AX_Thread_Exit::thr_mgr (AX_Thread_Manager *tm)
//{
//	printf ("ACE_Thread_Exit::thr_mgr");
//
//	if (tm != 0)
//		this->thread_control_.insert (tm, 0);
//}

// When this object is destroyed the Task is automatically closed
// down!

AX_Thread_Exit::~AX_Thread_Exit (void)
{
	//printf ("ACE_Thread_Exit::~ACE_Thread_Exit");
}

//ACE_Thread_Exit_Maybe::ACE_Thread_Exit_Maybe (int flag)
//: instance_ (0)
//{
//	if (flag)
//	{
//		ACE_NEW (instance_, ACE_Thread_Exit);
//	}
//}
//
//ACE_Thread_Exit_Maybe::~ACE_Thread_Exit_Maybe (void)
//{
//	delete this->instance_;
//}
//
//ACE_Thread_Exit *
//ACE_Thread_Exit_Maybe::operator -> (void) const
//{
//	return this->instance_;
//}
//
//ACE_Thread_Exit *
//ACE_Thread_Exit_Maybe::instance (void) const
//{
//	return this->instance_;
//}
int AX_Thread_Exit::register_to_exit(AX_Exit_Routine  exitRoutine)
{
//#ifdef _WIN32
	Exit_Stack.push(exitRoutine);
	return 1;
//#else
//	pthread_cleanup_push(exitRoutine.func,exitRoutine.arg);
//	size_++;
//	return size_;
//#endif
}

void AX_Thread_Exit::remove(void)
{
//#ifdef _WIN32
	Exit_Stack.pop();
	return;
//#else
//	pthread_cleanup_pop(0);
//	size_ --;
//	return;
//#endif
}

int AX_Thread_Exit::size()
{
//#ifdef _WIN32
	return (int)Exit_Stack.size();
//#else
//	return size_;
//#endif
}

int AX_Thread_Exit::run()
{
//#ifdef _WIN32
	while(!Exit_Stack.empty())
	{
		AX_Exit_Routine routine = Exit_Stack.top();
		(routine.func)(routine.arg);
		Exit_Stack.pop();
	}
	return 1;
//	return 1;
//#else
//	while(size_>0)
//	{
//		pthread_cleanup_pop(1);
//		size_ --;
//	}
//	return 1;
//#endif
}
