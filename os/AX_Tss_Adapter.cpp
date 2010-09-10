#include "AX_Tss_Adapter.h"

AX_TSS_Adapter::AX_TSS_Adapter (void *object, AX_THR_C_FUNC f)
: ts_obj_ (object),
func_ (f)
{
	// ACE_TRACE ("ACE_TSS_Adapter::ACE_TSS_Adapter");
}

void
AX_TSS_Adapter::cleanup (void)
{
	// ACE_TRACE ("ACE_TSS_Adapter::cleanup");
	(*this->func_)(this->ts_obj_);  // call cleanup routine for ts_obj_
}

extern "C" void
AX_TSS_C_cleanup (void *object)
{
	// ACE_TRACE ("ACE_TSS_C_cleanup");
	if (object != 0)
	{
		AX_TSS_Adapter * const tss_adapter = (AX_TSS_Adapter *) object;
		// Perform cleanup on the real TS object.
		tss_adapter->cleanup ();
		// Delete the adapter object.
		delete tss_adapter;
	}
}

