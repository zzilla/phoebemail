inline AX_THR_C_FUNC
AX_Base_Thread_Adapter::entry_point (void)
{
	return this->entry_point_;
}
#ifdef ACE_USES_GPROF
inline timerval*
AX_Base_Thread_Adapter::timerval (void)
{
	return &(this->itimer_);
}
#endif
