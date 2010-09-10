/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：多画面预览功能交互类
* 摘　要：多画面预览功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRPREVIEWCHANNEL_H_
#define _DVRPREVIEWCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrPreviewChannel : public CDvrChannel
{
public:
	CDvrPreviewChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrPreviewChannel();

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
    void			GetPreviewType(int *type1, int *type2);
	unsigned int	GetChannelConnectID() { return m_preview_channel_parm.conn.nConnectID; }


private:
	afk_preview_channel_param_s m_preview_channel_parm;
	void*			m_pSubSocket;
    BOOL			m_bWorking;
};


#endif // _DVRPREVIEWCHANNEL_H_


