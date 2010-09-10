#ifndef _AX_SCHED_PARAMS_H
#define _AX_SCHED_PARAMS_H
#include "AX_OS.h"
#ifndef _WIN32

#include <sched.h>
#endif


/**
* @class ACE_Sched_Params
*
* @brief Container for scheduling-related parameters. 
* 
* ACE_Sched_Params are passed via <ACE_OS::sched_params> to the 
* OS to specify scheduling parameters.  These parameters include 
* scheduling policy, such as FIFO (ACE_SCHED_FIFO), round-robin 
* (ACE_SCHED_RR), or an implementation-defined "OTHER"
* (ACE_SCHED_OTHER), to which many systems default; priority; 
* and a time-slice quantum for round-robin scheduling.  A 
* "scope" parameter specifies whether the ACE_Sched_Params
* applies to the current process, current lightweight process
* (LWP) (on Solaris), or current thread.  Please see the "NOTE"
* below about not all combinations of parameters being legal on
* a particular platform.
* For the case of thread priorities, it is intended that
* <ACE_OS::sched_params> usually be called from <main> before
* any threads have been spawned.  If spawned threads inherit
* their parent's priority (I think that's the default behavior
* for all of our platforms), then this sets the default base
* priority.  Individual thread priorities can be adjusted as
* usual using <ACE_OS::thr_prio> or via the ACE_Thread
* interface.  See the parameter descriptions in the private:
* section below.
* @note This class does not do any checking of parameters.  It
* is just a container class.  If it is constructed with values
* that are not supported on a platform, the call to
* <ACE_OS::sched_params> will fail by returning -1 with EINVAL
* (available through <ACE_OS::last_error>).
*/
//class AX_Sched_Params;
typedef int AX_Sched_Priority;

class AX_Sched_Params
{
	//    NOTE: Solaris 2.5.x threads in the RT class must set the
	//    priority of their LWP.  The only way to do that through ACE is
	//    for the RT thread itself to call <ACE_OS::thr_setprio> with
	//    it's own priority.

	//    OS Scheduling parameters are complicated and often confusing.
	//    Many thanks to Thilo Kielmann
	//    <kielmann@informatik.uni-siegen.de> for his careful review of
	//    this class design, thoughtful comments, and assistance with
	//    implementation, especially for PTHREADS platforms.  Please
	//    send any comments or corrections to the ACE developers.
public:
	typedef int Policy;
	
	// = Initialization and termination methods.
	/// Constructor.
	AX_Sched_Params (const Policy policy =0,
		const AX_Sched_Priority priority =0,
		const int scope =0);

	/// Termination.
	~AX_Sched_Params (void);

	// = Get/Set methods:

	// = Get/Set policy
	Policy policy (void) const;
	void policy (const Policy);

	// = Get/Set priority.
	AX_Sched_Priority priority (void) const;
	void priority (const AX_Sched_Priority);

	// = Get/Set scope.
	int scope (void) const;
	void scope(const int);

	// = Get/Set quantum.
	const timeval &quantum (void) const;
	void quantum (const timeval &);

	// = Accessors for OS-specific priorities.
	// These return priority values for ACE_SCHED_OTHER if the Policy value
	// is invalid.
	static int priority_min (const Policy =0,
		const int scope = 0);
	static int priority_max (const Policy = 0,
		const int scope = 0);

	/**
	* The next higher priority.  "Higher" refers to scheduling priority,
	* not to the priority value itself.  (On some platforms, higher scheduling
	* priority is indicated by a lower priority value.)  If "priority" is
	* already the highest priority (for the specified policy), then it is
	* returned.
	*/
	static int next_priority (const Policy,
		const int priority,
		const int scope = 0);

	/**
	* The previous, lower priority.  "Lower" refers to scheduling priority,
	* not to the priority value itself.  (On some platforms, lower scheduling
	* priority is indicated by a higher priority value.)  If "priority" is
	* already the lowest priority (for the specified policy), then it is
	* returned.
	*/
	static int previous_priority (const Policy,
		const int priority,
		const int scope  = 0);

private:
	/// Scheduling policy.
	Policy policy_;

	/// Default <priority_>: for setting the priority for the process, LWP,
	/// or thread, as indicated by the scope_ parameter.
	AX_Sched_Priority priority_;

	/**
	* <scope_> must be one of the following:
	*   ACE_SCOPE_PROCESS:  sets the scheduling policy for the
	*     process, and the process priority.  On some platforms,
	*     such as Win32, the scheduling policy can _only_ be
	*     set at process scope.
	*   ACE_SCOPE_LWP: lightweight process scope, only used with
	*     Solaris threads.
	*   ACE_SCOPE_THREAD: sets the scheduling policy for the thread,
	*     if the OS supports it, such as with Posix threads, and the
	*     thread priority.
	* NOTE:  I don't think that these are the same as POSIX
	*        contention scope.  POSIX users who are interested in,
	*        and understand, contention scope will have to set it
	*        by using system calls outside of ACE.
	*/
	int scope_;

	///**
	//* The <quantum_> is for time slicing.  An ACE_Time_Value of 0 has
	//* special significance: it means time-slicing is disabled; with
	//* that, a thread that is running on a CPU will continue to run
	//* until it blocks or is preempted.  Currently ignored if the OS
	//* doesn't directly support time slicing, such as on VxWorks, or
	//* setting the quantum (can that be done on Win32?).
	//*/
	//timeval quantum_;
};
#include "AX_Sched_Params.inl"
#endif
