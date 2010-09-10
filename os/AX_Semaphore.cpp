#include "AX_Semaphore.h"

void
AX_Semaphore::dump (void) const
{
}

AX_Semaphore::AX_Semaphore (unsigned int count,
							  int type,
							  const char *name,
							  void *arg,
							  int max)
							  : removed_ (0)
{
	//printf ("ACE_Semaphore::ACE_Semaphore");
	AX_OS::sema_init (&this->semaphore_, count, type,
		name, arg, max);
		//printf ("ACE_Semaphore::success");
}

AX_Semaphore::~AX_Semaphore (void)
{
	// ACE_TRACE ("ACE_Semaphore::~ACE_Semaphore");

	this->remove ();
}

