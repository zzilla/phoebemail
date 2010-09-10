
#include "dvrmediachannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include "dvrpacket_dvr2.h"


CDvrMediaChannel::CDvrMediaChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_media_channel_parm = *(afk_media_channel_param_s*)pParam;
	m_nChannelType = m_media_channel_parm.conn.nInterfaceType;
	m_pSubSocket = NULL;
    m_bWorking = FALSE;
}

CDvrMediaChannel::~CDvrMediaChannel()
{
	
}

/*
 * 摘要：处理命令
 */
int CDvrMediaChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

	CReadWriteMutexLock lock(m_csDataCallBack);
    if (m_media_channel_parm.base.func != NULL)
    {
		nRet = m_media_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 0, m_media_channel_parm.base.udata);
    }
	lock.Unlock();

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrMediaChannel::channel_close()
{
	// 目的停止回调，防止对象被删除了回调造成异常
	CReadWriteMutexLock lock(m_csDataCallBack);
	m_media_channel_parm.base.func = NULL;
	lock.Unlock();

	// 如果视频被打开了，需要关闭视频
	if (m_bWorking)
    {
        if (!sendMonitor_dvr2(m_pDvrDevice, m_media_channel_parm.no, m_media_channel_parm.subtype, false, m_media_channel_parm.conn.nConnType, m_media_channel_parm.conn.nConnectID, m_media_channel_parm.conn.szLocalIp, m_media_channel_parm.conn.nLocalPort))
        {
#ifdef _DEBUG
			// OutputDebugString("Error:发送关闭实时预览命令出错-->CDvrMediaChannel_dvr2::channel_close()\n");
#endif
        }
        m_bWorking = false;
    }
    
	m_pDvrDevice->device_remove_channel(this);
	
	if (m_pSubSocket != NULL)
	{
		m_pDvrDevice->DestroySubConn(m_media_channel_parm.conn.nConnType, m_pSubSocket, m_media_channel_parm.conn.nConnectID);
		m_pSubSocket = NULL;
	}

	return TRUE;
}

BOOL CDvrMediaChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrMediaChannel::channel_get_info(int type, void *parm)
{
	return 0;
}

int CDvrMediaChannel::channel_set_info(int type, void *parm)
{
	return 0;
}

/*
 * 摘要：打开通道
 */
int CDvrMediaChannel::channel_open()
{
	int nProtocalVer = 0;
	m_pDvrDevice->device_get_info(dit_protocol_version, &nProtocalVer);
	
	// 先建立短连接
	if (nProtocalVer >= 6 || m_media_channel_parm.conn.nConnType != channel_connect_tcp)
	{
		m_media_channel_parm.conn.userdata = this;
		m_pSubSocket = m_pDvrDevice->CreateSubConn(&m_media_channel_parm.conn);
		if (m_pSubSocket == NULL)
		{
			return -1;
		}
	}

	// 然后请求视频
	if (!sendMonitor_dvr2(m_pDvrDevice, m_media_channel_parm.no, m_media_channel_parm.subtype, true, m_media_channel_parm.conn.nConnType, m_media_channel_parm.conn.nConnectID, m_media_channel_parm.conn.szLocalIp, m_media_channel_parm.conn.nLocalPort))
	{
		if (m_pSubSocket != NULL)
		{
			m_pDvrDevice->DestroySubConn(m_media_channel_parm.conn.nConnType, m_pSubSocket, m_media_channel_parm.conn.nConnectID);
			m_pSubSocket = NULL;
		}
		return -1;
	}

	m_bWorking = TRUE;
	return 0;
}


