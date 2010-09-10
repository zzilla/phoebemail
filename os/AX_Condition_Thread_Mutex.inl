
inline AX_Condition_Attributes::AX_Condition_Attributes(int type)
{
	AX_OS::condattr_init(this->attributes_,type);
}

inline AX_Condition_Attributes::~AX_Condition_Attributes (void)
{
	AX_OS::condattr_destroy (this->attributes_);
}


inline AX_Thread_Mutex& AX_Condition_Thread_Mutex::mutex(void)
{
	return this->mutex_;
}
