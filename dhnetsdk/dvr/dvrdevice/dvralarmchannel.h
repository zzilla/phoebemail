/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：报警功能交互类
* 摘　要：报警功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRALARMCHANNEL_H_
#define _DVRALARMCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrAlarmChannel : public CDvrChannel
{
public:
	CDvrAlarmChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrAlarmChannel();

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
	afk_alarm_channel_param_s  m_alarm_channel_parm;
	afk_state_info_s* m_pstateinfo;
};


#endif // _DVRALARMCHANNEL_H_


