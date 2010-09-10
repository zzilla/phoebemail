/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：实时监视交互类
* 摘　要：实时监视功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRMEDIACHANNEL_H_
#define _DVRMEDIACHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrMediaChannel : public CDvrChannel
{
public:
	CDvrMediaChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrMediaChannel();

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

	/* 打开通道 */
	int				channel_open();


public:
    int				GetChannelIndex() { return m_media_channel_parm.no; }
	int				GetChannelSubtype() { return m_media_channel_parm.subtype; }
	unsigned int	GetChannelConnectID() { return m_media_channel_parm.conn.nConnectID; }
	int				GetConnectType() { return m_media_channel_parm.conn.nConnType; }
	char*			GetConnLocolIp() { return m_media_channel_parm.conn.szLocalIp; }
	int				GetConnLocolPort() { return m_media_channel_parm.conn.nLocalPort; }
	

private:
	afk_media_channel_param_s  m_media_channel_parm;
	void*			m_pSubSocket;
    BOOL            m_bWorking;
};


#endif // _DVRMEDIACHANNEL_H_

