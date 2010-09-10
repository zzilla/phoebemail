#include "AX_Sched_Params.h"

inline
AX_Sched_Params::AX_Sched_Params (
									const Policy policy,
									const AX_Sched_Priority priority,
									const int scope)
									: policy_ (policy),
									priority_ (priority),
									scope_ (scope)
{
}

inline AX_Sched_Params::~AX_Sched_Params (void)
{
}


inline void
AX_Sched_Params::priority (const AX_Sched_Priority priority)
{
	this->priority_ = priority;
}

inline int
AX_Sched_Params::scope (void) const
{
	return this->scope_;
}

inline void
AX_Sched_Params::scope (const int scope)
{
	this->scope_ = scope;
}

//inline const timeval &
//AX_Sched_Params::quantum (void) const
//{
//	return this->quantum_;
//}
//
//inline void
//AX_Sched_Params::quantum (const timeval &quant)
//{
//	this->quantum_ = quant;
//}

