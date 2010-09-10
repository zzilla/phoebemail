/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：透明串口功能交互类
* 摘　要：透明串口功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRTRANSCHANNEL_H_
#define _DVRTRANSCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrTransChannel : public CDvrChannel
{
public:
	CDvrTransChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrTransChannel();
	
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
    int				GetChannelIndex() { return (int)m_trans_channel_parm.trans_device_type; }


private:
    afk_trans_channel_param_s m_trans_channel_parm;
};


#endif // _DVRTRANSCHANNEL_H_

