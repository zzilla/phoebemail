#include "AX_Event.h"

inline AX_event_t
AX_Event::handle (void) const
{
	return this->handle_;
}

inline void
AX_Event::handle (AX_event_t new_handle)
{
	this->handle_ = new_handle;
}
