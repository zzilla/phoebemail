/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：语音对讲功能交互类
* 摘　要：语音对讲功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRTALKCHANNEL_H_
#define _DVRTALKCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrTalkChannel : public CDvrChannel
{
public:
	CDvrTalkChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrTalkChannel();
	
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
    afk_talk_channel_param_s m_talk_channel_parm;
    BOOL            m_bWorking;
};


#endif // _DVRTALKCHANNEL_H_

