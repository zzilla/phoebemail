#include "AX_Event.h"

AX_Event::AX_Event (int manual_reset,
					  int initial_state,
					  int type,
					  const char *name,
					  void *arg)
					  : removed_ (0)
{
	AX_OS::event_init (&this->handle_,
		manual_reset,
		initial_state,
		type,
		name,
		arg);
		//printf("AX_Event::AX_Event failed");
}

AX_Event::~AX_Event (void)
{
	//printf("the event is removed \n");
	this->remove ();
}

int
AX_Event::remove (void)
{
	int result = 0;
	if (this->removed_ == 0)
	{
		this->removed_ = 1;
		result = AX_OS::event_destroy (&this->handle_);
	}
	return result;
}

int
AX_Event::wait (void)
{
	return AX_OS::event_wait (&this->handle_);
}

int
AX_Event::wait (const timeval *abstime, int use_absolute_time)
{
	return AX_OS::event_timedwait (&this->handle_,
		const_cast <timeval *> (abstime));
}

int
AX_Event::signal (void)
{
	return AX_OS::event_signal (&this->handle_);
}

int
AX_Event::pulse (void)
{
	return AX_OS::event_pulse (&this->handle_);
}

int
AX_Event::reset (void)
{
	return AX_OS::event_reset (&this->handle_);
}

void
AX_Event::dump (void) const
{
}