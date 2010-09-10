/*
* Copyright (c) 2008, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 文件名称：TPMulticastClient.h
* 摘　　要：多播客户端功能类
*
*
* 取代版本：0.1
* 原作者　：Peng Dongfeng
* 完成日期：2008年8月6日
* 修订记录：创建
*/
//////////////////////////////////////////////////////////////////////

#ifndef _TPMulticastClient_H_
#define _TPMulticastClient_H_

#include "TPUDPClient.h"


class TPMulticastClient : public TPUDPClient
{
public:
    TPMulticastClient(ITPListener *callback, int engineId = 0);
    virtual ~TPMulticastClient();


public:	/* 接口 */
	virtual int Close(void);

	virtual int Connect(const char* szIp, int nPort);

	virtual int Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort);


private:
	SOCKET	m_sockMultipoint;
};

#endif // _TPMulticastClient_H_
