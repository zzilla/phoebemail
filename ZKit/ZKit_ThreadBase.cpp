#include "ZKit_ThreadBase.h"

BEGIN_ZKIT

void* ThreadBase::threadFunc(void *arg)
{
	ThreadBase *thread = (ThreadBase *)arg;
	if (thread == 0) 
	{
		return 0; // error.
	}

	while (!thread->isTerminated()) 
	{
		if (thread->run() < 0) 
		{
			printf("\nthread=%d exit because of return value!", thread->getThreadId());
			break;
		}
	}

	thread->threadStopped();

	printf("\nthread exit: %d", thread->getThreadId());

	return 0;
}

ThreadBase::ThreadBase(int nStackSize /* = 0 */)
:_stackSize(nStackSize)
{
	_handle = 0;

	_running = false;
	_terminating = true;
}

ThreadBase::~ThreadBase() 
{
	stop();
}

bool ThreadBase::start() 
{
	if (!_terminating||_running)
		return true;

	_terminating = false;
	_running = true;

	int ret = AX_Thread::spawn(threadFunc, this, THR_NEW_LWP | THR_JOINABLE, 
		&_threadId, &_handle, AX_DEFAULT_THREAD_PRIORITY, 0, _stackSize);

	if ( ret < 0 )
	{
		_terminating = true;
		_running = false;
		return false;
	}
		
	return true;
}

bool ThreadBase::stop()
{
	if ( isRunning() )
	{
		_terminating = true;

		AX_thr_func_return ret = 0;
		AX_Thread::join(_handle, &ret);
	}	

	return true;
}

bool ThreadBase::terminate(int signal)
{
	if ( isRunning() )
	{
		AX_Thread::kill(_handle, signal);
		threadStopped();
	}

	return true;
}

void ThreadBase::threadStopped(void)
{
	_terminating = true;
	_running = false;
	_handle = 0;
}

END_ZKIT