#ifndef _ZKit_ThreadBase_h_
#define _ZKit_ThreadBase_h_

#include "AX_Thread.h"
#include "ZKit_Config.h"

BEGIN_ZKIT

class ThreadBase
{
public: 
	ThreadBase(int nStackSize = 0);
	virtual ~ThreadBase(void);

	virtual bool start(void);
	virtual bool stop(void);
	virtual bool terminate(int signal); //最好用stop正常退出线程，除非是事先知道必须强制退出

public:
	AX_hthread_t getThreadHandle(void) const { return _handle; }
	AX_thread_t getThreadId(void) const { return _threadId; }

	bool isRunning(void) const { return _running; }
	bool isTerminated(void) const { return _terminating; }

protected:
	void threadStopped(void);

	//线程执行函数,返回值<0就退出线程
	virtual int run(void) = 0;

private:
    ThreadBase(const ThreadBase &);
    ThreadBase & operator=(const ThreadBase &) { return *this; }

    static void * threadFunc(void * arg);

protected:
	int _stackSize;
	bool _running;
	bool _terminating;

	AX_hthread_t _handle;
	AX_thread_t _threadId;
};

END_ZKIT
#endif // _ZKit_ThreadBase_h_
