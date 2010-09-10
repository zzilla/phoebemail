// DvrNewConfigChannel.h: interface for the CDvrNewConfigChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVRNEWCONFIGCHANNEL_H__B24695CE_864D_4C8C_B417_B54D0AD5A75C__INCLUDED_)
#define AFX_DVRNEWCONFIGCHANNEL_H__B24695CE_864D_4C8C_B417_B54D0AD5A75C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dvrchannel.h"
class CDvrDevice;

class CDvrNewConfigChannel : public CDvrChannel  
{
public:
	CDvrNewConfigChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam);
	virtual ~CDvrNewConfigChannel();

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
	int				GetPacketSequence() { return m_newconfigParam.nSequence; }

private:
	afk_newconfig_channel_param m_newconfigParam;
};

#endif // !defined(AFX_DVRNEWCONFIGCHANNEL_H__B24695CE_864D_4C8C_B417_B54D0AD5A75C__INCLUDED_)
