#include "AX_Thread_Guard.h"

template <class AX_LOCK> void AX_Guard<AX_LOCK>::dump (void) const
{
}

template <class AX_LOCK> void
AX_Write_Guard<AX_LOCK>::dump (void) const
{
	AX_Guard<AX_LOCK>::dump ();
}

// ACE_ALLOC_HOOK_DEFINE(ACE_Read_Guard)

template <class AX_LOCK> void
AX_Read_Guard<AX_LOCK>::dump (void) const
{
	AX_Guard<AX_LOCK>::dump ();
}
