/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：ITPListener.h
* 摘　　要：传输层监听器接口
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年6月16日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef _ITPLISTENER_H_
#define _ITPLISTENER_H_

class ITPListener
{
public:
	//	接收到数据后往缓冲里仍，该函数的处理时间会影响接收性能
	virtual int onData(int nEngineId, int nConnId, unsigned char* data, int nLen) = 0;

	//	处理缓冲数据(解析数据)，返回值：0：忙；1：空闲
	virtual int	onDealData(int nEngineId, int nConnId, unsigned char* buffer, int nLen) = 0;

	//	发送完成回调，可以用来删除上层的发送队列
	virtual int onSendDataAck(int nEngineId, int nConnId, int nId) = 0;

    //	返回值为0表示接受此连接，返回值为1表示拒绝接受
	virtual int onConnect(int nEngineId, int nConnId, char* szIp, int nPort) = 0;

	//	作为服务器类才回调，表示客户端连接断开
	virtual int onClose(int nEngineId, int nConnId) = 0;

	//	作为客户端类才回调，表示连接断开
	virtual int onDisconnect(int nEngineId, int nConnId) = 0;

	//	作为客户端类才回调，表示已重新连接
	virtual int onReconnect(int nEngineId, int nConnId) = 0;


};
#endif	//_ITPLISTENER_H_