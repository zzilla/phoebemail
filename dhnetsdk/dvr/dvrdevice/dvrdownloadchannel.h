/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：下载功能交互类
* 摘　要：下载功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRDOWNLOADCHANNEL_H_
#define _DVRDOWNLOADCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrDownLoadChannel : public CDvrChannel
{
public:
	CDvrDownLoadChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrDownLoadChannel();

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
    int				GetChannelIndex() { return m_download_channel_parm.info.ch; }
	unsigned int	GetMainType() { return m_download_channel_parm.type; }
	unsigned int	GetSubType() { return m_download_channel_parm.subtype; }
	unsigned int	GetChannelConnectID() { return m_download_channel_parm.conn.nConnectID; }


private:
	afk_download_channel_param_s m_download_channel_parm;
	BOOL			m_bDownloading;
    unsigned char	m_nFrameRate;
	void*			m_pSubSocket;
};


#endif // _DVRDOWNLOADCHANNEL_H_

