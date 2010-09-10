// TcpServer.cpp: implementation of the CTcpServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TcpServer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpServer::CTcpServer() : TPTCPServer(this)
{

}

CTcpServer::~CTcpServer()
{

}

int CTcpServer::onDisconnect(int engineId, int connId)
{
	return 0;
}

int CTcpServer::onReconnect(int engineId, int connId)
{
	return 0;
}

/*
 *	摘要：接收数据回调
 */
int CTcpServer::onData(int engineId, int connId, const char* data, int len)
{
	return 0;
}

/*
 *	摘要：发送数据完成回调，从发送队列中取数据继续发送
 */
int CTcpServer::onSendDataAck(int engineId, int connId, int id)
{
	return 0;
}

/*
 *	摘要：作服务器用才回调，有新的client连接，返回值为0表示接受此连接，返回值为1表示拒绝接受
 */
int CTcpServer::onConnect(int engineId, int connId, const char* ip, int port)
{
	return 0;
}

/*
 *	摘要：作服务器用才回调，client退出回调，删除客户端连接列表
 */
int CTcpServer::onClose(int engineId, int connId)
{
	return 0;
}