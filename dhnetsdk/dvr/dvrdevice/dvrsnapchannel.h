/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：抓图功能交互类
* 摘　要：抓图功能数据交互。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRSNAPCHANNEL_H_
#define _DVRSNAPCHANNEL_H_

#include "dvrchannel.h"
class CDvrDevice;

#define MAX_SNAPBUFFER_LEN  1048576 //1M bytes


class CDvrSnapChannel : public CDvrChannel
{
public:
	CDvrSnapChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrSnapChannel();
	
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
    int				GetChannelIndex() { return m_snap_channel_parm.no; }
	int				GetChannelSubtype() { return m_snap_channel_parm.subtype; }


protected:
	unsigned char	m_SnapPicBytes[MAX_SNAPBUFFER_LEN]; // 抓图返回数据缓存
    afk_snap_channel_param_s m_snap_channel_parm;
    BOOL            m_bWorking;
	LONG			m_Pos;			// 图片暂存缓存位置
};


#endif // _DVRSNAPCHANNEL_H_

