inline long
AX_OS_Thread_Descriptor::flags (void) const
{
	return flags_;
}

inline
AX_OS_Thread_Descriptor::AX_OS_Thread_Descriptor (long flags)
: flags_ (flags)
{
}
