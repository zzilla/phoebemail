#ifndef _AX_BASE_THREAD_H
#define _AX_BASE_THREAD_H

#include "AX_OS.h"
/**
* @class ACE_OS_Thread_Descriptor
*
* @brief Parent class of all ACE_Thread_Descriptor classes.
* =
* Container for ACE_Thread_Descriptor members that are
* used in ACE_OS.
*/
class  AX_OS_Thread_Descriptor
{
public:
	/// Get the thread creation flags.
	long flags (void) const;

protected:
	/// For use by ACE_Thread_Descriptor.
	AX_OS_Thread_Descriptor (long flags = 0);

	/**
	* Keeps track of whether this thread was created "detached" or not.
	* If a thread is *not* created detached then if someone calls
	* <ACE_Thread_Manager::wait>, we need to join with that thread (and
	* close down the handle).
	*/
	long flags_;
};
#include "AX_Base_Thread.inl"

#endif
