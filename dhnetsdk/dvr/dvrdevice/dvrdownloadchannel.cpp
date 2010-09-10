
#include "dvrdownloadchannel.h"
#include "dvrdevice.h"
#include "dvrpacket_comm.h"
#include "../def.h"
#include "dvrpacket_dvr2.h"


CDvrDownLoadChannel::CDvrDownLoadChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_download_channel_parm = *(afk_download_channel_param_s*)pParam;
	m_nChannelType = m_download_channel_parm.conn.nInterfaceType;
    m_bDownloading = FALSE;
    m_nFrameRate = 0;
	m_pSubSocket = NULL;
}

CDvrDownLoadChannel::~CDvrDownLoadChannel()
{
}

/*
 * 摘要：处理命令
 */
int CDvrDownLoadChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

	CReadWriteMutexLock lock(m_csDataCallBack);
	
    if (m_download_channel_parm.base.func != 0)
    {
		int nMainType = GetMainType();

		switch(nMainType)
		{
		case AFK_CHANNEL_DOWNLOAD_RECORD:
			{
				if (nLen != -1)
				{
					m_nFrameRate = getframerate_comm(m_pDvrDevice, *(pBuf + 1));
					nRet = m_download_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
														(void*)m_nFrameRate, m_download_channel_parm.base.udata);
				}
				else
				{
					nRet = m_download_channel_parm.base.func(this, 0, (unsigned int)-1, 0, m_download_channel_parm.base.udata);
				}
			}
			break;
		case AFK_CHANNEL_DOWNLOAD_CONFIGFILE:
			{
				if (((unsigned char)*(pBuf + 8) == 0xFF && nLen == HEADER_SIZE) || (unsigned char)*(pBuf + 12) != 0x00)
				{
					nRet = m_download_channel_parm.base.func(this, NULL, (unsigned int)-1, NULL, m_download_channel_parm.base.udata);
				}
				else
				{
					nRet = m_download_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, NULL, m_download_channel_parm.base.udata);
				}
			}
		    break;
		default:
		    break;
		}
    }
	
    if (nLen == 0)
    {
        m_bDownloading = FALSE;
    }

	lock.Unlock();

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrDownLoadChannel::channel_close()
{
	CReadWriteMutexLock lock(m_csDataCallBack);
	m_download_channel_parm.base.func = NULL;
	lock.Unlock();

	if (m_bDownloading)
    {
		int nMainType = GetMainType();

		switch(nMainType)
		{
		case AFK_CHANNEL_DOWNLOAD_RECORD:
			{
				if (sendDownload_dvr2(m_pDvrDevice, m_download_channel_parm.info, false, m_download_channel_parm.nByTime, m_download_channel_parm.conn.nConnectID, m_download_channel_parm.nParam))
				{
					m_bDownloading = FALSE;
				}
			}
			break;
		case AFK_CHANNEL_DOWNLOAD_CONFIGFILE:
			{
				if (sendExportCfgFile_dvr2(m_pDvrDevice, false, 0))
				{
					m_bDownloading = FALSE;
				}
			}
		    break;
		default:
		    break;
		}
    }

	m_pDvrDevice->device_remove_channel(this);

	if (m_pSubSocket != NULL)
	{
		m_pDvrDevice->DestroySubConn(m_download_channel_parm.conn.nConnType, m_pSubSocket, m_download_channel_parm.conn.nConnectID);
		m_pSubSocket = NULL;
	}
	
    return TRUE;
}

/*
 * 摘要：暂停通道
 */
BOOL CDvrDownLoadChannel::channel_pause(BOOL pause)
{
	BOOL bRet = FALSE;

	if (m_bDownloading)
    {
		int nMainType = GetMainType();

		if (AFK_CHANNEL_DOWNLOAD_RECORD == nMainType)
		{
			bRet = sendDownLoad_pause_dvr2(m_pDvrDevice, m_download_channel_parm.info.ch, pause, m_download_channel_parm.conn.nConnectID);
		}
    }

    return bRet;
}

int CDvrDownLoadChannel::channel_get_info(int type, void *parm)
{
    return m_nFrameRate;
}

int CDvrDownLoadChannel::channel_set_info(int type, void *parm)
{
	switch(type) 
	{
	case 0:	// 控制录像回放
		if (m_bDownloading)
		{
			afk_download_control_param_s *control_parm = (afk_download_control_param_s*)parm;
			return sendDownload_control_dvr2(m_pDvrDevice, m_download_channel_parm.info.ch, 
				control_parm->offsettime, control_parm->offsetdata, m_download_channel_parm.conn.nConnectID);
		}
		break;
	case 1:	// 打开录像回放
		{
			if (m_bDownloading)
			{
				sendDownload_dvr2(m_pDvrDevice, m_download_channel_parm.info, false, m_download_channel_parm.nByTime, m_download_channel_parm.conn.nConnectID, m_download_channel_parm.nParam);
				afk_download_channel_param_s* pbrf = (afk_download_channel_param_s*)parm;
				m_download_channel_parm.info = pbrf->info;
				return sendDownload_dvr2(m_pDvrDevice, pbrf->info, true, m_download_channel_parm.nByTime, m_download_channel_parm.conn.nConnectID, m_download_channel_parm.nParam);
			}
		}
		break;
	default:
		break;
	}
	
    return 0;
}

/*
 * 摘要：打开通道
 */
int CDvrDownLoadChannel::channel_open()
{
	int nProtocalVer = 0;
	m_pDvrDevice->device_get_info(dit_protocol_version, &nProtocalVer);
	
	// 先建立短连接
	if (nProtocalVer >= 6 || m_download_channel_parm.conn.nConnType != channel_connect_tcp)
	{
		m_download_channel_parm.conn.userdata = this;
		m_pSubSocket = m_pDvrDevice->CreateSubConn(&m_download_channel_parm.conn);
		if (m_pSubSocket == NULL)
		{
			return -1;
		}
	}

	// 然后发请求包
	switch (m_download_channel_parm.type)
	{
	// 下载录象文件
	case AFK_CHANNEL_DOWNLOAD_RECORD:
		{
			if (!sendDownload_comm(m_pDvrDevice, m_download_channel_parm.info, true, m_download_channel_parm.nByTime, m_download_channel_parm.conn.nConnectID, m_download_channel_parm.nParam))
			{
				if (m_pSubSocket != NULL)
				{
					m_pDvrDevice->DestroySubConn(m_download_channel_parm.conn.nConnType, m_pSubSocket, m_download_channel_parm.conn.nConnectID);
					m_pSubSocket = NULL;
				}
				return -1;
			}
		}
		break;
	// 下载配置文件
	case AFK_CHANNEL_DOWNLOAD_CONFIGFILE:
		{
			if (!sendExportCfgFile_comm(m_pDvrDevice, true, m_download_channel_parm.nParam))
			{
				if (m_pSubSocket != NULL)
				{
					m_pDvrDevice->DestroySubConn(m_download_channel_parm.conn.nConnType, m_pSubSocket, m_download_channel_parm.conn.nConnectID);
					m_pSubSocket = NULL;
				}
				return -1;
			}
		}
		break;
	default:
		break;
	}

	m_bDownloading = TRUE;
	return 0;
}



