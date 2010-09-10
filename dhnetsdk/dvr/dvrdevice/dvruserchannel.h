/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：用户管理功能交互类
* 摘　要：用户管理功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRUSERCHANNEL_H_
#define _DVRUSERCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrUserChannel : public CDvrChannel
{
public:
	CDvrUserChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrUserChannel();

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
    unsigned int	GetUserType() { return m_user_channel_parm.type; }


private:
    afk_user_channel_param_s m_user_channel_parm;
};


#endif // _DVRUSERCHANNEL_H_

