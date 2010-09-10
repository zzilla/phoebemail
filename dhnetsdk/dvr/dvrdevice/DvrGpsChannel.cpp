
#include "../StdAfx.h"
#include "DvrGpsChannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include <stdio.h>
#include "memory.h"


CDvrGpsChannel::CDvrGpsChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_gps_channel_parm = *(afk_gps_channel_param_s*)pParam;
}

CDvrGpsChannel::~CDvrGpsChannel()
{
    
}

/*
 * 摘要：处理命令
 */
int CDvrGpsChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;
	
    if (m_gps_channel_parm.callback != NULL)
    {
        LONG bodylen = nLen - HEADER_SIZE;

        nRet = m_gps_channel_parm.base.func(this, pBuf + HEADER_SIZE, bodylen, 0, m_gps_channel_parm.base.udata);

		m_gps_channel_parm.callback(m_gps_channel_parm.LoginId,(char*)(pBuf + HEADER_SIZE), bodylen, m_gps_channel_parm.dwuser);        
    }

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrGpsChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

	return TRUE;
}

BOOL CDvrGpsChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrGpsChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrGpsChannel::channel_set_info(int type, void *parm)
{
    return 0;
}

