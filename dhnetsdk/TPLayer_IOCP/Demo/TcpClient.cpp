// TcpClient.cpp: implementation of the CTcpClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TCPClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpClient::CTcpClient() : TPTCPClient(this)
{

}

CTcpClient::~CTcpClient()
{

}

int CTcpClient::onConnect(int engineId, int connId, const char* ip, int port)
{
	return 0;
}

int CTcpClient::onClose(int engineId, int connId)
{
	return 0;
}

/*
 *	摘要：接收数据回调
 */
int CTcpClient::onData(int engineId, int connId, const char* data, int len)
{
	return 0;
}

/*
 *	摘要：发送数据完成回调，从发送队列中取数据继续发送
 */
int CTcpClient::onSendDataAck(int engineId, int connId, int id)
{
	return 0;
}

/*
 *	摘要：作客户端用才回调，断线回调，内部可能重连，由应用层决定是否close
 */
int CTcpClient::onDisconnect(int engineId, int connId)
{
	return 0;
}

/*
 *	摘要：作客户端用才回调，断线后重连成功回调，恢复所有的业务(操作)
 */
int CTcpClient::onReconnect(int engineId, int connId)
{
	return 0;
}
