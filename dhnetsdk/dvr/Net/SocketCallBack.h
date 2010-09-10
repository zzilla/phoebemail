/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：TCP接收处理类
* 摘　要：对TCP流进行解析、分包，并送往对应的功能模块处理。
*         解包依据：主命令、子命令和包序号。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _SOCKETCALLBACK_H_
#define _SOCKETCALLBACK_H_


// 主连接接收数据处理
int __stdcall OnOtherPacket(unsigned char *pBuf, int nLen, void *userdata);

// 主连接数据码流统计
int __stdcall OnReceivePacket(unsigned char *pBuf, int nLen, void *userdata);

// 主连接断线
int __stdcall OnDisconnect(void *userdata);

// 主连接断线恢复
int	__stdcall OnReConnect(void *userdata);

// 子连接接收数据处理
int __stdcall OnSubOtherPacket(unsigned char *pBuf, int nLen, void *userdata);

// 子连接数据码流统计
int __stdcall OnSubReceivePacket(unsigned char *pBuf, int nLen, void *userdata);

// 子连接断线
int __stdcall OnSubDisconnect(void *userdata);

// 子连接断线恢复
int	__stdcall OnSubReConnect(void *userdata);


#endif // _SOCKETCALLBACK_H_




