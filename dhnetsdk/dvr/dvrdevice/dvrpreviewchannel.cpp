
#include "dvrpreviewchannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include "dvrpacket_dvr2.h"


CDvrPreviewChannel::CDvrPreviewChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_preview_channel_parm = *(afk_preview_channel_param_s*)pParam;
	m_nChannelType = m_preview_channel_parm.conn.nInterfaceType;
	m_pSubSocket = NULL;
    m_bWorking = FALSE;
}

CDvrPreviewChannel::~CDvrPreviewChannel()
{
}

/*
 * 摘要：处理命令
 */
int CDvrPreviewChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

	CReadWriteMutexLock lock(m_csDataCallBack);
    if (m_preview_channel_parm.base.func != NULL)
    {
		nRet = m_preview_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 0, m_preview_channel_parm.base.udata);
    }
	lock.Unlock();

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrPreviewChannel::channel_close()
{
	CReadWriteMutexLock lock(m_csDataCallBack);
	m_preview_channel_parm.base.func = NULL;
	lock.Unlock();

	if (m_bWorking)
    {
        if (!sendPreview_dvr2(m_pDvrDevice, 0, 0, false, m_preview_channel_parm.conn.nConnType, m_preview_channel_parm.conn.nConnectID, m_preview_channel_parm.conn.szLocalIp, m_preview_channel_parm.conn.nLocalPort))
        {
#ifdef _DEBUG
			// OutputDebugString("Error:发送关闭实时预览命令出错-->CDvrPreviewChannel_dvr2::channel_close()\n");
#endif
        }
        m_bWorking = FALSE;
    }

	m_pDvrDevice->device_remove_channel(this);
    
	if (m_pSubSocket != NULL)
	{
		m_pDvrDevice->DestroySubConn(m_preview_channel_parm.conn.nConnType, m_pSubSocket, m_preview_channel_parm.conn.nConnectID);
		m_pSubSocket = NULL;
	}

	return TRUE;
}

BOOL CDvrPreviewChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrPreviewChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrPreviewChannel::channel_set_info(int type, void *parm)
{
    return 0;
}

void CDvrPreviewChannel::GetPreviewType(int *type1, int *type2)
{
    *type1 = m_preview_channel_parm.parm1;
    *type2 = m_preview_channel_parm.parm2;
}

/*
 * 摘要：打开通道
 */
int CDvrPreviewChannel::channel_open()
{
	int nProtocalVer = 0;
	m_pDvrDevice->device_get_info(dit_protocol_version, &nProtocalVer);
	
	// 先建立短连接
	if (nProtocalVer >= 6 || m_preview_channel_parm.conn.nConnType != channel_connect_tcp)
	{
		m_preview_channel_parm.conn.userdata = this;
		m_pSubSocket = m_pDvrDevice->CreateSubConn(&m_preview_channel_parm.conn);
		if (m_pSubSocket == NULL)
		{
			return -1;
		}
	}

	// 然后请求视频
	if (!sendPreview_dvr2(m_pDvrDevice, m_preview_channel_parm.parm1, m_preview_channel_parm.parm2, true, m_preview_channel_parm.conn.nConnType, m_preview_channel_parm.conn.nConnectID, m_preview_channel_parm.conn.szLocalIp, m_preview_channel_parm.conn.nLocalPort))
	{
		if (m_pSubSocket != NULL)
		{
			m_pDvrDevice->DestroySubConn(m_preview_channel_parm.conn.nConnType, m_pSubSocket, m_preview_channel_parm.conn.nConnectID);
			m_pSubSocket = NULL;
		}
		return -1;
	}

	m_bWorking = TRUE;
	return 0;
}
