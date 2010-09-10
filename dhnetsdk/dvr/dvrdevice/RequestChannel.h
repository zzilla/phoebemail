/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：新协议交互类
* 摘　要：适合加包序号的字符串协议，每个交互过程利用包序号标识，无须再区分功能。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_REQUESTCHANNEL_H__0053C362_A0DF_42F5_9EFB_AB1D4D2AA702__INCLUDED_)
#define AFX_REQUESTCHANNEL_H__0053C362_A0DF_42F5_9EFB_AB1D4D2AA702__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dvrchannel.h"
class CDvrDevice;


class CRequestChannel : public CDvrChannel
{
public:
	CRequestChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CRequestChannel();

public:
	/* 关闭通道 */
    virtual BOOL	channel_close();

    /* 暂停通道 */
    virtual BOOL	channel_pause(BOOL pause);

    /* 获取信息 */
    virtual int		channel_get_info(int type, void *parm);

    /* 设置信息 */
    virtual int		channel_set_info(int type, void *parm);

	/* 处理命令 */
	virtual int		OnRespond(unsigned char *pBuf, int nLen);


public:
	int				GetRequestType() { return m_requestParam.nType; }
	int				GetPacketSequence() { return m_requestParam.nSequence; }

private:
	afk_request_channel_param m_requestParam;
	int				m_nProtocolType;
	int				m_nPacketSequence;
	DWORD			m_dwCreateTime;	// 异步未收到应答包则删除
};


#endif // !defined(AFX_REQUESTCHANNEL_H__0053C362_A0DF_42F5_9EFB_AB1D4D2AA702__INCLUDED_)


