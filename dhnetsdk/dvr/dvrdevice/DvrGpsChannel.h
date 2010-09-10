/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：GPS功能交互类
* 摘　要：GPS功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRGPSCHANNEL_H_
#define _DVRGPSCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrGpsChannel : public CDvrChannel
{
public:
	CDvrGpsChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrGpsChannel();
	
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


private:
    afk_gps_channel_param_s m_gps_channel_parm;
};

#endif // _DVRGPSCHANNEL_H_

