/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：查询功能交互类
* 摘　要：查询功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRSEARCHCHANNEL_H_
#define _DVRSEARCHCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;


class CDvrSearchChannel : public CDvrChannel
{
public:
	CDvrSearchChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrSearchChannel();
	
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
    unsigned int	GetSearchType() { return m_search_channel_parm.type; }
	unsigned int	GetSearchSubtype() { return m_search_channel_parm.subtype; }
    int				GetChannelIndex() { return m_search_channel_parm.no; }


protected:
    afk_search_channel_param_s m_search_channel_parm;
    unsigned int	m_nLogSize;
    unsigned int	m_nLogRecSize;
};


#endif // _DVRSEARCHCHANNEL_H_

