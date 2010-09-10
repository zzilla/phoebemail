
#ifndef _AX_TSS_ADAPTER
#define _AX_TSS_ADAPTER

#include "AX_OS.h"

class AX_TSS_Adapter
{
public:
	/// Initialize the adapter.
	AX_TSS_Adapter (void *object, AX_THR_C_FUNC f);

	/// Perform the cleanup operation.
	void cleanup (void);

	//private:

	/// The real TS object.
	void * const ts_obj_;

	/// The real cleanup routine for ts_obj;
	AX_THR_C_FUNC func_;
};

#endif
