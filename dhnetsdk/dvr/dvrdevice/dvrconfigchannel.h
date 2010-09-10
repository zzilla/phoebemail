/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：配置功能交互类
* 摘　要：配置功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRCONFIGCHANNEL_H_
#define _DVRCONFIGCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrConfigChannel : public CDvrChannel
{
public:
	CDvrConfigChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrConfigChannel();
	
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
    unsigned int	GetConfigType() { return m_config_channel_parm.type; }


private:
    afk_config_channel_param_s m_config_channel_parm;
};


#endif // _DVRCONFIGCHANNEL_H_


