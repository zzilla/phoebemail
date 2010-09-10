/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：流量统一类
* 摘　要：流量统计。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRSTATISCCHANNEL_H_
#define _DVRSTATISCCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrStatiscChannel : public CDvrChannel
{
public:
	CDvrStatiscChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrStatiscChannel();
	
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
};


#endif // _DVRSTATISCCHANNEL_H_

