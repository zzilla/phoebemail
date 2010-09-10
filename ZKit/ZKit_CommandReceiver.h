#ifndef _ZKit_CommandReceiver_h_
#define _ZKit_CommandReceiver_h_

#include "ZKit_Command.h"

BEGIN_ZKIT
//从网络接收数据, 从中提取命令
//需要处理具体命令的类应该继承此类, 并选择性的覆盖其虚方法.
class CommandReceiver
{
public:
	//判断某个命令是否已经收到应答
	bool IsAckReceived( uint32 sessionId ) const;

	Command ConfirmAck( uint32 sessionId );

protected:
	//网络数据的入口
	void OnData(const char* data, size_t length);

private:
	void ExtractCommand();

	void DispatchCommand(const Command& cmd);

	//留给子类覆盖的钩子函数.
	virtual void OnCmd(const Command& cmd)
	{	

	}

	virtual void OnReq(const Command& cmd)
	{	

	}

	virtual void OnAck(const Command& cmd)
	{

	}

private:
	std::vector<char> m_bytes;//用于存储命令的缓存.
	mutable std::map<uint32, Command> m_acks;//收到的应答.
	ZKit::Mutex m_acksMutex;
};

END_ZKIT
#endif // _ZKit_CommandReceiver_h_
