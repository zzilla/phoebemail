
#include "../StdAfx.h"
#include "dvrdevice.h"
#include "dvrchannel.h"
#include "dvrpacket_comm.h"
#include "../dvrinterface.h"
#include "../Net/TcpSocket.h"
#include "../Net/UdpSocket.h"
#include "../Net/MulticastSocket.h"
#include "../Net/SocketCallBack.h"
#include "dvrmediachannel.h"
#include "dvrsearchchannel.h"
#include "dvrdownloadchannel.h"
#include "dvrupgradechannel.h"
#include "dvrpreviewchannel.h"
#include "dvrcontrolchannel.h"
#include "dvrconfigchannel.h"
#include "dvrstatiscchannel.h"
#include "dvrtalkchannel.h"
#include "dvruserchannel.h"
#include "dvrtranschannel.h"
#include "dvralarmchannel.h"
#include "dvrsnapchannel.h"
#include "DvrGpsChannel.h"
#include "RequestChannel.h"
#include "DvrNewConfigChannel.h"
#include "../devprob.h"


CDvrDevice::CDvrDevice(afk_dvrdevice_info stuDeviceInfo, CTcpSocket *pTcpSocket,
						const char *szIp, int nPort, const char *szUser, const char *szPassword, 
						fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata)
{
    version				= interface_version;
	manufactory			= interface_manufactory;
	product				= interface_product;
    device_ip			= interface_ip;
    device_port			= interface_port;
    device_type			= interface_type;
    channelcount		= interface_channelcount;
    alarminputcount		= interface_alarminputcount;
    alarmoutputcount	= interface_alarmoutputcount;
	close				= interface_close;
    get_channel			= interface_get_channel;
	open_channel		= interface_open_channel;
	channel_is_opened	= interface_channel_is_opened;
	get_info			= interface_get_info;
	set_info			= interface_set_info;
	dev_addref			= interface_dev_addref;
	dev_decref			= interface_dev_decref;

    m_Deviceinfo = stuDeviceInfo;
    m_pTcpSocket = pTcpSocket;
    m_pfDisConnect = cbDisconnect;
	m_pfDevEvent = cbDevEvent;
    m_userdata = userdata;
    m_statisc = 0;
	m_dwSpecial = 0;
	m_nRecordProto = -1;
	m_nLogProto = -1;
    m_enablestatisc = FALSE;
	m_nProtocolVer = 0;
	m_nSessionId = 0;
	m_nIsMutiEther = 0;
	m_nTalkMode = 0;
	m_nCardQuery = 0;
	m_nQuickConfig = 0;
	m_nAlarmInCaps = 0;
	m_dwTalkFormat = 0;
	m_nAudioAlarmInCaps = 0;
	m_nMultiPlayFlag = 0;
	m_nIsHaveTask = 0;
	m_dwLastLoginPacket = 0;
	m_dwLastSubKeep = 0;
	m_emTaskStatus = EM_SEND_LOGINPACKET;
	m_pCurSubTcpSocket = NULL;
	memset(&m_messCallFunc, 0, sizeof(DEVICE_MESS_CALLBACK));
	memset(&m_nvdPlaybackCall, 0, sizeof(DEVICE_MESS_CALLBACK));
	memset(&m_loginType, 0, sizeof(afk_login_device_type));
	m_loginType.nLoginMode = channel_connect_tcp;
	m_nImportantRecord = 0;

	m_stCurTalkFmt.iAudioType = 0xFFFF;
	m_stCurTalkFmt.iAudioBit = 8;
	m_stCurTalkFmt.dwSampleRate = 8000;

	memset(m_dwAlarmListenMode, 0, sizeof(m_dwAlarmListenMode));
	memset(m_dwCurMotionAlarm, 0, sizeof(m_dwCurMotionAlarm));
	memset(m_dwCurVideoLost, 0, sizeof(m_dwCurVideoLost));
	memset(m_dwCurShelter, 0, sizeof(m_dwCurShelter));
	memset(m_dwCurInputAlarm, 0, sizeof(m_dwCurInputAlarm));
	memset(&m_nAuthorityInfo, 0, sizeof(m_nAuthorityInfo));
	memset(&m_stEthernet, 0, 5 * sizeof(CONFIG_ETHERNET));
	int i = 0;
	for (i = 0; i < MAX_AUTHORITY_NUM; i++)
	{
		m_nAuthorityInfo.bAuthorityInfo[i] = 1;
	}

    strcpy(m_szIp, szIp);
    m_nPort = nPort;

    if (szUser != NULL && szPassword != NULL)
    {
        strcpy(m_szUser, szUser);
        strcpy(m_szPassword, szPassword);
    }

    for (i = 0; i < m_Deviceinfo.channelcount; i++)
    {
		m_snap_channel_list.push_back(0);
    }

    for (i = 0; i < 2; i++)
    {
        m_trans_channel_list.push_back(0);
    }

	m_upgrade_channel = NULL;
	m_alarm_channel   = NULL;
	m_audio_channel   = NULL;
	m_user_channel    = NULL;
	m_statisc_channel = NULL;
	m_control_channel = NULL;
	m_gps_channel	  = NULL;
	m_importcfg_channel = NULL;

	InterlockedSetEx(&m_lLifeCount, 1);
}

CDvrDevice::~CDvrDevice()
{
    version				= NULL;
	manufactory			= NULL;
	product				= NULL;
    device_ip			= NULL;
    device_port			= NULL;
    device_type			= NULL;
    channelcount		= NULL;
    alarminputcount		= NULL;
    alarmoutputcount	= NULL;
	close				= NULL;
    get_channel			= NULL;
	open_channel		= NULL;
	channel_is_opened	= NULL;
	get_info			= NULL;
	set_info			= NULL;

	// 如果用户未关闭，释放内存
	{
		std::map<unsigned int, CDvrChannel*>::iterator it = m_media_channel_list.begin();
		while (it != m_media_channel_list.end())
		{
			CDvrChannel *pDvrChannel = (*it).second;
			if (pDvrChannel != NULL)
			{
				delete pDvrChannel;
			}
			it++;
		}
		m_media_channel_list.clear();
	}

	{
		std::map<unsigned int, CDvrChannel*>::iterator it = m_preview_channel_list.begin();
		while (it != m_preview_channel_list.end())
		{
			CDvrChannel *pDvrChannel = (*it).second;
			if (pDvrChannel != NULL)
			{
				delete pDvrChannel;
			}
			it++;
		}
		m_preview_channel_list.clear();
	}
    
	{
		std::map<unsigned int, CDvrChannel*>::iterator it = m_download_channel_list.begin();
		while (it != m_download_channel_list.end())
		{
			CDvrChannel *pDvrChannel = (*it).second;
			if (pDvrChannel != NULL)
			{
				delete pDvrChannel;
			}
			it++;
		}
		m_download_channel_list.clear();
	}

	{
		std::vector<CDvrChannel *>::iterator it = m_trans_channel_list.begin();
		while (it != m_trans_channel_list.end())
		{
			if (*it)
			{
				delete *it;
			}
			it++;
		}
		m_trans_channel_list.clear();
	}
	
	{
		std::list<CDvrChannel *>::iterator it = m_search_channel_list.begin();
		while (it != m_search_channel_list.end())
		{
			if (*it)
			{
				delete *it;
			}
			it++;
		}
		m_search_channel_list.clear();
	}

	{
		std::list<CDvrChannel *>::iterator it = m_config_channel_list.begin();
		while (it != m_config_channel_list.end())
		{
			if (*it)
			{
				delete *it;
			}
			it++;
		}
		m_config_channel_list.clear();
	}

	{
		std::vector<CDvrChannel *>::iterator it = m_snap_channel_list.begin();
		while (it != m_snap_channel_list.end())
		{
			if (*it)
			{
				delete *it;
			}
			it++;
		}
		m_snap_channel_list.clear();
	}
    
	{
		std::list<CDvrChannel*>::iterator it = m_request_channel_list.begin();
		while (it != m_request_channel_list.end())
		{
			if (*it)
			{
				delete *it;
			}
			it++;
		}
		m_request_channel_list.clear();
	}

	{
		std::list<CDvrChannel*>::iterator it = m_newconfig_channel_list.begin();
		while (it != m_newconfig_channel_list.end())
		{
			if (*it)
			{
				delete *it;
			}
			it++;
		}
		m_newconfig_channel_list.clear();
	}

	// 释放连接对象
    if (m_pTcpSocket != NULL)
    {
        delete m_pTcpSocket;
        m_pTcpSocket = NULL;
    }

	CReadWriteMutexLock lock(m_csSubTcp);
	{
		std::map<unsigned int, CTcpSocket*>::iterator it = m_lstSubTcpSockets.begin();
		while (it != m_lstSubTcpSockets.end())
		{
			CTcpSocket *pSubTcpSocket = (*it).second;
			if (pSubTcpSocket != NULL)
			{
				delete pSubTcpSocket;
			}
			it++;
		}
		m_lstSubTcpSockets.clear();
	}
	
	{
		std::map<unsigned int, CUdpSocket*>::iterator it = m_lstSubUdpSockets.begin();
		while (it != m_lstSubUdpSockets.end())
		{
			CUdpSocket *pSubUdpSocket = (*it).second;
			if (pSubUdpSocket != NULL)
			{
				delete pSubUdpSocket;
			}
			it++;
		}
		m_lstSubUdpSockets.clear();
	}

	{
		std::map<unsigned int, CMulticastSocket*>::iterator it = m_lstMulticastSockets.begin();
		while (it != m_lstMulticastSockets.end())
		{
			CMulticastSocket *pSubMultiSocket = (*it).second;
			if (pSubMultiSocket != NULL)
			{
				delete pSubMultiSocket;
			}
			it++;
		}
		m_lstMulticastSockets.clear();
	}
	lock.Unlock();
}

int CDvrDevice::device_AddRef()
{
	int nLifeCount = InterlockedIncrementEx(&m_lLifeCount);
	if (nLifeCount <= 1)
	{
		InterlockedDecrementEx(&m_lLifeCount);
		return 0;
	}
	
	return nLifeCount;
}

int CDvrDevice::device_DecRef()
{
	int nLifeCount = InterlockedDecrementEx(&m_lLifeCount);
	if (nLifeCount <= 0)
	{
		delete this;
		return 0;
	}
	
	return nLifeCount;
}

CDvrChannel* CDvrDevice::device_get_channel(int type, unsigned int param, unsigned int subparam)
{
	CDvrChannel *p = NULL;
	
    switch (type)
    {
    /* 普通的实时媒体通道 */
    case AFK_CHANNEL_TYPE_MEDIA:
		{
 			CReadWriteMutexLock lock(m_csmedia, false, true, false);

			std::map<unsigned int, CDvrChannel*>::iterator it = m_media_channel_list.find(param);
			if (it != m_media_channel_list.end())
			{
				p = (*it).second;
				if (p != NULL)
				{
					p->channel_addRef();
				}
			}
		}
		break;
    /* 查询通道 */
    case AFK_CHANNEL_TYPE_SEARCH:
		{
			CReadWriteMutexLock lock(m_cssearch, false, true, false);

			CDvrSearchChannel *searchchannel = 0;

			std::list<CDvrChannel*>::iterator it = m_search_channel_list.begin();
			for (; it != m_search_channel_list.end(); it++)
			{
				searchchannel = (CDvrSearchChannel*)(*it);
				if (searchchannel && (searchchannel->GetSearchType() == param) 
					&& (searchchannel->GetSearchSubtype() == subparam))
				{
					p = searchchannel;
					break;
				}
			}
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 下载通道 */
    case AFK_CHANNEL_TYPE_DOWNLOAD:
		{
			CReadWriteMutexLock lock(m_csdownload, false, true, false);

			std::map<unsigned int, CDvrChannel*>::iterator it = m_download_channel_list.find(subparam);
			if (it != m_download_channel_list.end())
			{
				p = (*it).second;
				if (p != NULL)
				{
					p->channel_addRef();
				}
			}
		}
		break;
    /* 在线更新通道 */
    case AFK_CHANNEL_TYPE_UPGRADE:
		{
			if (AFK_CHANNEL_UPLOAD_UPGRADE == param)
			{
				CReadWriteMutexLock lock(m_csupgrade, false, true, false);
				
				p = m_upgrade_channel;
				
				if (p != NULL)
				{
					p->channel_addRef();
				}
			}
			else if (AFK_CHANNEL_UPLOAD_CONFIGFILE == param)
			{
				CReadWriteMutexLock lock(m_csimportcfg, false, true, false);
				
				p = m_importcfg_channel;
				
				if (p != NULL)
				{
					p->channel_addRef();
				}
			}
		}
		break;
    /* 预览通道 */
    case AFK_CHANNEL_TYPE_PREVIEW:
		{
			CReadWriteMutexLock lock(m_cspreview, false, true, false);
			
			std::map<unsigned int, CDvrChannel*>::iterator it = m_preview_channel_list.find(param);
			if (it != m_preview_channel_list.end())
			{
				p = (*it).second;
				if (p != NULL)
				{
					p->channel_addRef();
				}
			}
		}
		break;
    /* 报警数据通道 */
    case AFK_CHANNEL_TYPE_ALARM:
		{
			CReadWriteMutexLock lock(m_csalarm, false, true, false);

			p = m_alarm_channel;
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 控制通道 */
    case AFK_CHANNEL_TYPE_CONTROL:
		{
			CReadWriteMutexLock lock(m_cscontrol, false, true, false);
			
			/* 只有设备控制支持返回值处理 */
			p = m_control_channel;

			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 配置通道 */
    case AFK_CHANNEL_TYPE_CONFIG:
		{
			CReadWriteMutexLock lock(m_csconfig, false, true, false);
			
			CDvrConfigChannel *configchannel = 0;

			std::list<CDvrChannel*>::iterator it = m_config_channel_list.begin();
			for (; it != m_config_channel_list.end(); it++)
			{
				configchannel = (CDvrConfigChannel*)(*it);
				if (configchannel && (configchannel->GetConfigType() == param))
				{
					p = configchannel;
					break;
				}
			}			
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 统计信息通道 */
    case AFK_CHANNEL_TYPE_STATISC:
		{
			CReadWriteMutexLock lock(m_csstatisc, false, true, false);
			
			p = m_statisc_channel;
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 透明通道 */
    case AFK_CHANNEL_TYPE_TRANS:
		{
			CReadWriteMutexLock lock(m_cstrans, false, true, false);
			
			if (param >= 0 && param < m_trans_channel_list.size())
			{
				p = m_trans_channel_list.at(param);
			}
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 语音通道 */
    case AFK_CHANNEL_TYPE_TALK:
		{
			CReadWriteMutexLock lock(m_csaudio, false, true, false);
			
			p = m_audio_channel;
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 用户通道 */
    case AFK_CHANNEL_TYPE_USER:
		{
			CReadWriteMutexLock lock(m_csuser, false, true, false);
			
			p = m_user_channel;
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    /* 解码通道 */
    case AFK_CHANNEL_TYPE_DECODE:
        break;
    /* 渲染通道 */
    case AFK_CHANNEL_TYPE_RENDER:
        break;
    /* 用户数据 */
    case AFK_CHANNEL_TYPE_USERMSG:
        break;
	case AFK_CHANNEL_TYPE_GPS:
		{
			CReadWriteMutexLock lock(m_csgps, false, true, false);
			
			p = m_gps_channel;
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
	case AFK_CHANNEL_TYPE_CAPTURE:
		{
			CReadWriteMutexLock lock(m_cssnap, false, true, false);
			
			if (param >= 0 && param < m_snap_channel_list.size())
			{
				p = m_snap_channel_list.at(param);
			}
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
	/* 请求通道 */
    case AFX_CHANNEL_TYPE_REQUEST:
		{
			CReadWriteMutexLock lock(m_csRequest, false, true, false);

			CRequestChannel *pRequestchannel = NULL;

			std::list<CDvrChannel*>::iterator it = m_request_channel_list.begin();
			for (; it != m_request_channel_list.end(); it++)
			{
				pRequestchannel = (CRequestChannel*)(*it);
				if (pRequestchannel != NULL && (pRequestchannel->GetRequestType() == param) 
					&& (pRequestchannel->GetPacketSequence() == subparam))
				{
					p = pRequestchannel;
					break;
				}
			}
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
	/* 新配置通道 */
    case AFX_CHANNEL_TYPE_NEWCONFIG:
		{
			CReadWriteMutexLock lock(m_csnewconfig, false, true, false);

			CRequestChannel *pNewConfigchannel = NULL;

			std::list<CDvrChannel*>::iterator it = m_newconfig_channel_list.begin();
			for (; it != m_newconfig_channel_list.end(); it++)
			{
				pNewConfigchannel = (CRequestChannel*)(*it);
				if ((pNewConfigchannel != NULL) && (pNewConfigchannel->GetPacketSequence() == subparam))
				{
					p = pNewConfigchannel;
					break;
				}
			}
			
			if (p != NULL)
			{
				p->channel_addRef();
			}
		}
		break;
    default:
        break;
    }

    return p;
}

void CDvrDevice::device_remove_channel(CDvrChannel *pDvrChannel)
{
    int nMainCommand = pDvrChannel->GetMainCommand();

    switch(nMainCommand)
    {
    /* 普通的实时媒体通道 */
    case AFK_CHANNEL_TYPE_MEDIA:
		{
			CReadWriteMutexLock lock(m_csmedia);

			CDvrMediaChannel *pMediaChannel = (CDvrMediaChannel*)pDvrChannel;
			unsigned int nConnectID = pMediaChannel->GetChannelConnectID();
			m_media_channel_list.erase(nConnectID);
		}
		break;
    /* 查询通道 */
    case AFK_CHANNEL_TYPE_SEARCH:
		{
			CReadWriteMutexLock lock(m_cssearch);
			
			m_search_channel_list.remove(pDvrChannel);
		}
		break;
    /* 下载通道 */
    case AFK_CHANNEL_TYPE_DOWNLOAD:
		{
			CReadWriteMutexLock lock(m_csdownload);

			CDvrDownLoadChannel *pDownloadChannel = (CDvrDownLoadChannel*)pDvrChannel;
			unsigned int nConnectID = pDownloadChannel->GetChannelConnectID();
			m_download_channel_list.erase(nConnectID);
		}
		break;
    /* 在线更新通道 */
    case AFK_CHANNEL_TYPE_UPGRADE:
		{
			CReadWriteMutexLock lock(m_csupgrade);
			
			if (pDvrChannel == m_upgrade_channel)
			{
				m_upgrade_channel = NULL;
			}
			lock.Unlock();

			CReadWriteMutexLock lock1(m_csimportcfg);
			
			if (pDvrChannel == m_importcfg_channel)
			{
				m_importcfg_channel = NULL;
			}
			lock1.Unlock();
		}
		break;
    /* 预览通道 */
    case AFK_CHANNEL_TYPE_PREVIEW:
		{
			CReadWriteMutexLock lock(m_cspreview);
			
			CDvrPreviewChannel *pPreviewChannel = (CDvrPreviewChannel*)pDvrChannel;
			unsigned int nConnectID = pPreviewChannel->GetChannelConnectID();
			m_preview_channel_list.erase(nConnectID);
		}
		break;
    /* 报警数据通道 */
    case AFK_CHANNEL_TYPE_ALARM:
		{
			CReadWriteMutexLock lock(m_csalarm);
			
			m_alarm_channel = NULL;
		}
		break;
    /* 控制通道 */
    case AFK_CHANNEL_TYPE_CONTROL:
		{
			CReadWriteMutexLock lock(m_cscontrol);
			
			CDvrControlChannel *pControlChannel = (CDvrControlChannel*)pDvrChannel;
			/* 只有设备控制支持返回值处理 */
			if (pControlChannel && (AFK_CONTROL_TYPE_DEVCONTROL == pControlChannel->GetControlType1()))
			{
				m_control_channel = NULL;
			}
		}
		break;
    /* 配置通道 */
    case AFK_CHANNEL_TYPE_CONFIG:
		{
			CReadWriteMutexLock lock(m_csconfig);
			
			m_config_channel_list.remove(pDvrChannel);
		}
		break;
    /* 统计信息通道 */
    case AFK_CHANNEL_TYPE_STATISC:
		{
			CReadWriteMutexLock lock(m_csstatisc);

			m_statisc_channel = NULL;
		}
		break;
    /* 透明通道 */
    case AFK_CHANNEL_TYPE_TRANS:
		{
			CReadWriteMutexLock lock(m_cstrans);
			
			CDvrTransChannel *pTransChannel = (CDvrTransChannel*)pDvrChannel;
			size_t no = pTransChannel->GetChannelIndex();
			if (no < m_trans_channel_list.size())
			{
				m_trans_channel_list[no] = NULL;
			}
			else
			{
				//have error
			}
		}
		break;
    /* 语音通道 */
    case AFK_CHANNEL_TYPE_TALK:
		{
			CReadWriteMutexLock lock(m_csaudio);
			
			m_audio_channel = NULL;
		}
		break;
    /* 用户通道 */
    case AFK_CHANNEL_TYPE_USER:
		{
			CReadWriteMutexLock lock(m_csuser);
			
			m_user_channel = NULL;
		}
		break;
    /* 解码通道 */
    case AFK_CHANNEL_TYPE_DECODE:
        break;
    /* 渲染通道 */
    case AFK_CHANNEL_TYPE_RENDER:
        break;
    /* 用户数据 */
    case AFK_CHANNEL_TYPE_USERMSG:
        break;
	case AFK_CHANNEL_TYPE_CAPTURE:
		{
			CReadWriteMutexLock lock(m_cssnap);
			
			CDvrSnapChannel *pSnapChannel = (CDvrSnapChannel*)pDvrChannel;
			
			size_t no = pSnapChannel->GetChannelIndex();
			if (no < m_snap_channel_list.size())
			{
				m_snap_channel_list[no] = NULL;
			}
			else
			{
				//have error 
			}
		}
		break;
	case AFK_CHANNEL_TYPE_GPS:
		{
			CReadWriteMutexLock lock(m_csgps);

			m_gps_channel = NULL;
		}
		break;
	case AFX_CHANNEL_TYPE_REQUEST:
		{
			CReadWriteMutexLock lock(m_csRequest);
			m_request_channel_list.remove(pDvrChannel);
		}
		break;
	case AFX_CHANNEL_TYPE_NEWCONFIG:
		{
			CReadWriteMutexLock lock(m_csnewconfig);
			m_newconfig_channel_list.remove(pDvrChannel);
		}
		break;
    default:
        break;
    }
}

BOOL CDvrDevice::device_channel_is_opened(int type, void *parm)
{
    afk_media_channel_param_s *mc_parm = NULL;
	afk_preview_channel_param_s *pc_parm = NULL;
    afk_search_channel_param_s *sc_parm = NULL;
    afk_trans_channel_param_s *tc_parm = NULL;
    afk_download_channel_param_s *dc_parm = NULL;
	afk_config_channel_param_s *cc_parm = NULL;
	afk_control_channel_param_s *ctrl_parm = NULL;
	afk_snap_channel_param_s *sp_parm = NULL;
	afk_upgrade_channel_param_s *up_parm = NULL;

	BOOL bRet = FALSE;
	CDvrChannel *pDvrChannel = NULL;
	
    switch (type)
    {
    //普通的实时媒体通道
    case AFK_CHANNEL_TYPE_MEDIA:
        mc_parm = (afk_media_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, mc_parm->conn.nConnectID);
        break;
    //查询通道
    case AFK_CHANNEL_TYPE_SEARCH:
		sc_parm = (afk_search_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, sc_parm->type);
        break;
    //下载通道
    case AFK_CHANNEL_TYPE_DOWNLOAD:
        dc_parm = (afk_download_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, dc_parm->type, dc_parm->conn.nConnectID);
        break;
    //在线更新通道
    case AFK_CHANNEL_TYPE_UPGRADE:
		up_parm = (afk_upgrade_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, up_parm->type);
        break;
    //预览通道
    case AFK_CHANNEL_TYPE_PREVIEW:
		pc_parm = (afk_preview_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, pc_parm->conn.nConnectID);
        break;
    //报警数据通道
    case AFK_CHANNEL_TYPE_ALARM:
        pDvrChannel = device_get_channel(type, 0);
        break;
    //控制通道
    case AFK_CHANNEL_TYPE_CONTROL:
		ctrl_parm = (afk_control_channel_param_s*)parm;
		if (AFK_CONTROL_TYPE_DEVCONTROL == ctrl_parm->type1)
		{
			pDvrChannel = device_get_channel(type, ctrl_parm->type1);
		}
		else
		{
			return FALSE;
		}
        break;
    //统计信息通道
    case AFK_CHANNEL_TYPE_STATISC:
        pDvrChannel = device_get_channel(type, 0);
        break;
    /* 透明通道 */
    case AFK_CHANNEL_TYPE_TRANS:
        tc_parm = (afk_trans_channel_param_s*)parm;
        pDvrChannel = device_get_channel(type, tc_parm->trans_device_type);
        break;
    /* 语音通道 */
    case AFK_CHANNEL_TYPE_TALK:
        pDvrChannel = device_get_channel(type, 0);
        break;
    /* 用户通道 */
    case AFK_CHANNEL_TYPE_USER:
        pDvrChannel = device_get_channel(type, 0);
        break;
    //解码通道
    case AFK_CHANNEL_TYPE_DECODE:
        break;
    //渲染通道
    case AFK_CHANNEL_TYPE_RENDER:
        break;
    //用户数据
    case AFK_CHANNEL_TYPE_USERMSG:
        break;
	case AFK_CHANNEL_TYPE_DDNS:
		pDvrChannel = device_get_channel(type, 0);
		break;
	case AFK_CHANNEL_TYPE_CONFIG:
		cc_parm = (afk_config_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, cc_parm->type);
		break;
	//抓图通道
	case AFK_CHANNEL_TYPE_CAPTURE:		
        sp_parm = (afk_snap_channel_param_s*)parm;
		pDvrChannel = device_get_channel(type, sp_parm->no);
        break;
	//GPS数据通道
    case AFK_CHANNEL_TYPE_GPS:
        pDvrChannel = device_get_channel(type, 0);
        break;
	case AFX_CHANNEL_TYPE_REQUEST:
		{
			pDvrChannel = NULL;
		}
		break;
	case AFX_CHANNEL_TYPE_NEWCONFIG:
		{
			pDvrChannel = NULL;
		}
		break;
    default:
        break;
    }
	
	bRet = (pDvrChannel != 0);
	if (pDvrChannel)
	{
		pDvrChannel->channel_decRef();
	}

    return bRet; 
}

void CDvrDevice::SetDeviceInfo(int type, DWORD dwParm)
{
	switch(type)
	{
	case 1:	//set alarm input count
		m_Deviceinfo.alarminputcount = dwParm;
		break;
	case 2: //set alarm output count
		m_Deviceinfo.alarmoutputcount = dwParm;
		break;
	default:
		break;
	}
}

void CDvrDevice::startstatisc()
{
    m_enablestatisc = TRUE;
    m_statisc = 0;
}

void CDvrDevice::statiscing(unsigned int nlen)
{
    if (m_enablestatisc)
    {
        m_statisc += nlen;
    }
}

unsigned int CDvrDevice::statisc()
{
    unsigned int statisc = m_statisc; 
    m_enablestatisc = FALSE;
    m_statisc = 0;
    return statisc;
}

int CDvrDevice::device_close()
{
	int nRet = 0;

	if (m_pTcpSocket != NULL)
	{
		if (1 == m_nQuickConfig)
		{
			if (sendRequestPacket_comm(this, AFK_REQUEST_LOGOUT, 0, 0, NULL))
			{
				WaitForSingleObjectEx(m_pTcpSocket->m_hRecEvent, 500);
			}
		}

		m_pTcpSocket->Disconnect();
	}
	
	CReadWriteMutexLock lock(m_csSubTcp, false, true, false);
	{
		std::map<unsigned int, CTcpSocket*>::iterator it = m_lstSubTcpSockets.begin();
		while (it != m_lstSubTcpSockets.end())
		{
			CTcpSocket *pSubTcpSocket = (*it).second;
			if (pSubTcpSocket != NULL)
			{
				pSubTcpSocket->Disconnect();
			}
			it++;
		}
	}
	
	{
		std::map<unsigned int, CUdpSocket*>::iterator it = m_lstSubUdpSockets.begin();
		while (it != m_lstSubUdpSockets.end())
		{
			CUdpSocket *pSubUdpSocket = (*it).second;
			if (pSubUdpSocket != NULL)
			{
				pSubUdpSocket->Disconnect();
			}
			it++;
		}
		m_lstSubUdpSockets.clear();
	}

	{
		std::map<unsigned int, CMulticastSocket*>::iterator it = m_lstMulticastSockets.begin();
		while (it != m_lstMulticastSockets.end())
		{
			CMulticastSocket *pMulticastSocket = (*it).second;
			if (pMulticastSocket != NULL)
			{
				pMulticastSocket->Disconnect();
			}
			it++;
		}
		m_lstMulticastSockets.clear();
	}
	lock.Unlock();

	CleanDisConn();

    return nRet;
}


/* 打开通道 */
void* CDvrDevice::device_open_channel(int type, void *parm)
{
    if (device_channel_is_opened(type, parm))
    {
        return NULL;
    }

    CDvrMediaChannel *dvr2_media_channel = NULL;
    afk_media_channel_param_s *mc_parm = NULL;
    
    CDvrControlChannel *dvr2_control_channel = NULL;
    afk_control_channel_param_s *cc_parm = NULL;

    CDvrSearchChannel *dvr2_search_channel = NULL;
    afk_search_channel_param_s *sc_parm = NULL;
    
    CDvrDownLoadChannel *dvr2_download_channel = NULL;
    afk_download_channel_param_s *dc_parm = NULL;

    CDvrUpgradeChannel *dvr2_upgrade_channel = NULL;
    afk_upgrade_channel_param_s *uc_parm = NULL;

    CDvrPreviewChannel *dvr2_preview_channel = NULL;
    afk_preview_channel_param_s *pc_parm = NULL;

    CDvrAlarmChannel *dvr2_alarm_channel = NULL;
    afk_alarm_channel_param_s *ac_parm = NULL;

    CDvrConfigChannel *dvr2_config_channel = NULL;
    afk_config_channel_param_s *fc_parm = NULL;

    CDvrStatiscChannel *dvr2_statisc_channel = NULL;

    CDvrTransChannel *dvr2_trans_channel = NULL;
    afk_trans_channel_param_s *tc_parm = NULL;

    CDvrTalkChannel *dvr2_talk_channel = NULL;
    afk_talk_channel_param_s  *ad_parm = NULL;

    CDvrUserChannel *dvr2_user_channel = NULL;
    afk_user_channel_param_s *uo_parm = NULL;

	CDvrSnapChannel *dvr2_snap_channel = NULL;
	afk_snap_channel_param_s *sp_parm = NULL;

	CDvrGpsChannel *mobile_gps_channel = NULL;
	afk_gps_channel_param_s *gps_parm = NULL;

    switch (type)
    {
    //普通的实时媒体通道
    case AFK_CHANNEL_TYPE_MEDIA:
        {
			mc_parm = (afk_media_channel_param_s*)parm;
			if (mc_parm->no < m_Deviceinfo.channelcount && mc_parm->no >= 0)
			{
				// 对于向动态多连接过渡的设备，第一次打开时建立连接
				int nIsQuickConfig = 0;
				device_get_info(dit_quick_config, &nIsQuickConfig);
				if (5 == m_nProtocolVer && 1 == nIsQuickConfig && channel_connect_tcp == mc_parm->conn.nConnType && !m_Deviceinfo.bAutoRegister)
				{
					if (*(m_pTcpSocket->m_registerAck + 9) != 8)
					{
						mc_parm->conn.nParam = 0;
					}

					CTcpSocket *pSubTcpSocket = NULL;
#ifdef WIN32
					CReadWriteMutexLock lock(m_csSubTcp);
#endif
					std::map<unsigned int, CTcpSocket*>::iterator it = m_lstSubTcpSockets.find(mc_parm->conn.nConnectID);
					if (it != m_lstSubTcpSockets.end())
					{
						pSubTcpSocket = (*it).second;
					}
					else
					{
						pSubTcpSocket = (CTcpSocket*)CreateSubConn(&mc_parm->conn);
					}
#ifdef WIN32
					lock.Unlock();
#endif
					
					if (NULL == pSubTcpSocket)
					{
						return NULL;
					}

					if (0 == mc_parm->conn.nParam)
					{
						Sleep(50);
					}
				}

 				// 创建交互通道
 				dvr2_media_channel = new CDvrMediaChannel(this, AFK_CHANNEL_TYPE_MEDIA, parm);
 				if (dvr2_media_channel == NULL)
 				{
 					return NULL;
 				}

				// 开启监视
				int ret = dvr2_media_channel->channel_open();
				if (ret < 0)
				{
					dvr2_media_channel->close(dvr2_media_channel);
					return NULL;
				}

				CReadWriteMutexLock lock(m_csmedia);
				m_media_channel_list[mc_parm->conn.nConnectID] = dvr2_media_channel;
				lock.Unlock();

				return dvr2_media_channel;
			}
		}
        break;
    //查询通道
    case AFK_CHANNEL_TYPE_SEARCH:
		{
			CReadWriteMutexLock lock(m_cssearch);
			
			sc_parm = (afk_search_channel_param_s*)parm;
			
			switch (sc_parm->type)
			{
				//查询录像
			case AFK_CHANNEL_SEARCH_RECORD:
				if ((sc_parm->no < m_Deviceinfo.channelcount && sc_parm->no >= 0) || sc_parm->no == 255 || sc_parm->no == 16)
				{
					if (sendQueryRecord_comm(this, sc_parm->queryrecord, sc_parm->param))
					{
						dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
					}
				}
				break;
				//查询日志
			case AFK_CHANNEL_SEARCH_LOG:
				if (sendQueryLog_comm(this, sc_parm->subtype, sc_parm->param, sc_parm->webName, sc_parm->webNamelen))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询通道名
			case AFK_CHANNEL_SEARCH_CHANNELNAME:
				if (sendQueryChannelName_comm(this))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询报警布撤防输入
			case AFK_CHANNEL_SEARCH_ALARMCTRL_IN:
				if (sendQueryAlarmInput_comm(this))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询报警布撤防输出
			case AFK_CHANNEL_SEARCH_ALARMCTRL_OUT:
				if (sendQueryAlarmOutput_comm(this))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询配置信息
			case AFK_CHANNEL_SEARCH_CONFIG:
				if (sendQueryConfig_comm(this, sc_parm->subtype, sc_parm->param))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询录象状态
			case AFK_CHANNEL_SEARCH_RECORDSTATE:
				if (sendAlarmQuery_comm(this, true))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询设备时间
			case AFK_CHANNEL_SEARCH_DEVTIME:
				if (sendQueryDeviceTime_comm(this))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询串口协议
			case AFK_CHANNEL_SEARCH_COMMPROTOCOL:
				if (sendQueryProtocol_comm(this, 0))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询解码器协议
			case AFK_CHANNEL_SEARCH_DCDPROTOCOL:
				if (sendQueryProtocol_comm(this, 1))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
			case AFK_CHANNEL_SEARCH_SYSTEM_INFO:
				if (sendQuerySystemInfo_comm(this, sc_parm->subtype, sc_parm->param))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
			case AFK_CHENNEL_SEARCH_DEVWORKSTATE:
				if (sendQuerydevstate_comm(this, sc_parm->no, sc_parm->querydevstate))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
			case AFK_CHENNEL_SEARCH_DEVSTATE:
				if (sendQueryAlarmState_comm(this, sc_parm->statetype, sc_parm->param))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询云台属性
			case AFK_CHANNEL_SEARCH_PTZ:
				if (sendQueryProtocol_comm(this, 2,sc_parm->param)) /*2 表示查询ptz协议属性*/
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
				//查询无线报警输出
			case AFK_CHANNEL_SEARCH_WIRELESS_ALARMOUT:
				if(sendQueryWirelessAlarmOutput_comm(this))
				{
					dvr2_search_channel = new CDvrSearchChannel(this,AFK_CHANNEL_TYPE_SEARCH,parm);
				}
				break;
				//查询报警触发方式
			case AFK_CHANNEL_SEARCH_ALARMTRRIGER_MODE:
				if (sendQueryAlarmTrrigerMode_comm(this))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
			case AFK_CHANNEL_SEARCH_TRANSCOM:
				if(sendQueryTransCom_comm(this, sc_parm->param))
				{
					dvr2_search_channel = new CDvrSearchChannel(this, AFK_CHANNEL_TYPE_SEARCH, parm);
				}
				break;
			default:
				break;
			}
				
			if (dvr2_search_channel)
			{
				m_search_channel_list.push_back(dvr2_search_channel);
			}
			
			return dvr2_search_channel;
		}
		break;
    //下载通道
    case AFK_CHANNEL_TYPE_DOWNLOAD:
        {
			dc_parm = (afk_download_channel_param_s*)parm;
			if ((dc_parm->info.ch < (unsigned int)m_Deviceinfo.channelcount || 16 == dc_parm->info.ch || 255 == dc_parm->info.ch) && dc_parm->info.ch >= 0)
			{
 				// 创建交互通道
 				dvr2_download_channel = new CDvrDownLoadChannel(this, AFK_CHANNEL_TYPE_DOWNLOAD, parm);
 				if (dvr2_download_channel == NULL)
 				{
 					return NULL;
 				}

				// 开启下载
				int ret = dvr2_download_channel->channel_open();
				if (ret < 0)
				{
					dvr2_download_channel->close(dvr2_download_channel);
					return NULL;
				}

				CReadWriteMutexLock lock(m_csdownload);
				m_download_channel_list[dc_parm->conn.nConnectID] = dvr2_download_channel;
				lock.Unlock();

				return dvr2_download_channel;
			}
		}
        break;
    //上载通道
    case AFK_CHANNEL_TYPE_UPGRADE:
        {
			uc_parm = (afk_upgrade_channel_param_s*)parm;

			switch (uc_parm->type)
			{
				//上载升级文件
			case AFK_CHANNEL_UPLOAD_UPGRADE:
				{
					if (sendUpgradeControl_comm(this, uc_parm))
					{
						dvr2_upgrade_channel = new CDvrUpgradeChannel(this, AFK_CHANNEL_TYPE_UPGRADE, parm);
						if (dvr2_upgrade_channel)
						{
							CReadWriteMutexLock lock(m_csupgrade);

							m_upgrade_channel = dvr2_upgrade_channel;
						}

						return dvr2_upgrade_channel;
					}
				}
				break;
				//上载配置文件
			case AFK_CHANNEL_UPLOAD_CONFIGFILE:
				{
					if (sendImportCfgFile_comm(this, uc_parm))
					{
						dvr2_upgrade_channel = new CDvrUpgradeChannel(this, AFK_CHANNEL_TYPE_UPGRADE, parm);
						if (dvr2_upgrade_channel)
						{
							CReadWriteMutexLock lock(m_csimportcfg);

							m_importcfg_channel = dvr2_upgrade_channel;
						}

						return dvr2_upgrade_channel;
					}
				}
				break;
			default:
				break;
			}
		}
        break;
    //预览通道
    case AFK_CHANNEL_TYPE_PREVIEW:
        {
			pc_parm = (afk_preview_channel_param_s*)parm;
			if (m_Deviceinfo.ispreview == false)
			{
				break;
			}

			// 创建交互通道
			dvr2_preview_channel = new CDvrPreviewChannel(this, AFK_CHANNEL_TYPE_PREVIEW, parm);
			if (dvr2_preview_channel == NULL)
			{
				return NULL;
			}
			
			// 开启预览
			int ret = dvr2_preview_channel->channel_open();
			if (ret < 0)
			{
				dvr2_preview_channel->close(dvr2_preview_channel);
				return NULL;
			}
			
			CReadWriteMutexLock lock(m_cspreview);
			m_preview_channel_list[pc_parm->conn.nConnectID] = dvr2_preview_channel;
			lock.Unlock();
			
			return dvr2_preview_channel;
		}
        break;
    //报警数据通道
    case AFK_CHANNEL_TYPE_ALARM:
        {
			ac_parm = (afk_alarm_channel_param_s*)parm;
			if (0 == ac_parm->parm)		//老报警协议
			{
				if (sendAlarmQuery_comm(this, false, 0))
				{
					dvr2_alarm_channel = new CDvrAlarmChannel(this, AFK_CHANNEL_TYPE_ALARM, parm);
					{
						CReadWriteMutexLock lock(m_csalarm);

						m_alarm_channel = dvr2_alarm_channel;
					}

					return dvr2_alarm_channel;
				}
			}
			else if (1 == ac_parm->parm)	//新报警协议
			{
				if (sendAlarmQuery_comm(this, false, 1))
				{
					dvr2_alarm_channel = new CDvrAlarmChannel(this, AFK_CHANNEL_TYPE_ALARM, parm);
					{
						CReadWriteMutexLock lock(m_csalarm);

						m_alarm_channel = dvr2_alarm_channel;
					}

					return dvr2_alarm_channel;
				}
			}
		}
        break;
    //控制通道
    case AFK_CHANNEL_TYPE_CONTROL:
        {
			cc_parm = (afk_control_channel_param_s*)parm;
			if (sendControl_comm(this, cc_parm->no, cc_parm->type1, cc_parm->type2, cc_parm->parm1, cc_parm->parm2, cc_parm->parm3, cc_parm->pParm4))
			{
				dvr2_control_channel = new CDvrControlChannel(this, AFK_CHANNEL_TYPE_CONTROL, parm);
				if (AFK_CONTROL_TYPE_DEVCONTROL == cc_parm->type1)
				{
					CReadWriteMutexLock lock(m_cscontrol);

					m_control_channel = dvr2_control_channel;
				}

				return dvr2_control_channel;
			}
		}
        break;
    //配置通道
    case AFK_CHANNEL_TYPE_CONFIG:   
		{
			CReadWriteMutexLock lock(m_csconfig);
			
			fc_parm = (afk_config_channel_param_s*)parm;
			switch (fc_parm->type)
			{
			//设置报警输入输出
			case AFK_CHANNEL_CONFIG_ALARMCTRL:
				if (sendSettingAlarm_comm(this, fc_parm->alarminfo))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			//设置报警触发方式
			case AFK_CHANNEL_CONFIG_ALARMTRRIGER_MODE:
				if(sendSettingAlarmTrrigerMode_comm(this, fc_parm->alarmtrrigermode))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			//设置配置信息
			case AFK_CHANNEL_CONFIG_CONFIG:
				if (sendSetupConfig_comm(this, fc_parm->subtype, fc_parm->param[0], fc_parm->configinfo.configbuf, fc_parm->configinfo.buflen))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			//设置录象状态
			case AFK_CHANNEL_CONFIG_RECORDSTATE:
				if (sendSetupRecordState_comm(this, fc_parm->configinfo.configbuf, fc_parm->configinfo.buflen))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			//设置设备时间
			case AFK_CHANNEL_CONFIG_DEVTIME:
				if (sendSetupDeviceTime_comm(this, fc_parm->devicetimeinfo.year,
					fc_parm->devicetimeinfo.month, fc_parm->devicetimeinfo.day,
					fc_parm->devicetimeinfo.hour, fc_parm->devicetimeinfo.minute,
					fc_parm->devicetimeinfo.second))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			//强制I桢
			case AFK_CHANNEL_CONFIG_FORCE_IFRAME:
				if (sendForceIframe_comm(this, fc_parm->no, fc_parm->chngetIframe.subchn))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			//设置限制码流
			case AFK_CHANNEL_CONFIG_LIMIT_FLUX:
				if (sendLimitFlux_comm(this, fc_parm->limitflux))
				{
					dvr2_config_channel = new CDvrConfigChannel(this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			case AFK_CHANNEL_CONFIG_CHANNEL_OSD:
				if (sendSetupChannelOsdString_comm(this, fc_parm->no, fc_parm->param[0], &fc_parm->param[1],
					fc_parm->configinfo.configbuf, fc_parm->configinfo.buflen))
				{
					dvr2_config_channel = new CDvrConfigChannel((CDvrDevice *)this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			case AFK_CHANNEL_CONFIG_ALARMDCD_OUT:
				if(sendSetingDecoderAlarm_comm(this, fc_parm->decoderalarm))
				{
					dvr2_config_channel = new CDvrConfigChannel((CDvrDevice *)this, AFK_CHANNEL_TYPE_CONFIG, parm);
				}
				break;
			default:
				break;
			}
			
			if (dvr2_config_channel) 
			{
				m_config_channel_list.push_back(dvr2_config_channel);
			}

			return dvr2_config_channel;
		}
		break;
    //统计信息通道
    case AFK_CHANNEL_TYPE_STATISC:
        {
			dvr2_statisc_channel = new CDvrStatiscChannel(this, AFK_CHANNEL_TYPE_STATISC, 0);
			{
				CReadWriteMutexLock lock(m_csstatisc);

				m_statisc_channel = dvr2_statisc_channel;
			}

			return dvr2_statisc_channel;
		}
        break;
    //透明通道
    case AFK_CHANNEL_TYPE_TRANS:
        tc_parm = (afk_trans_channel_param_s*)parm;
        if (tc_parm->trans_device_type <= 2 &&
            tc_parm->trans_device_type >= 0)
        {
            if (sendSettingSerial_comm(this, *tc_parm))
            {
                dvr2_trans_channel = new CDvrTransChannel(this, AFK_CHANNEL_TYPE_TRANS, parm);
				if (dvr2_trans_channel)
				{
					CReadWriteMutexLock lock(m_cstrans);

					if (tc_parm->trans_device_type < m_trans_channel_list.size())
					{
						m_trans_channel_list[tc_parm->trans_device_type] = dvr2_trans_channel;
					}
					else
					{
						delete dvr2_trans_channel;
						dvr2_trans_channel = NULL;
					}
				}
				else
				{
					//have error
				}

                return dvr2_trans_channel;
            }
        }
        break;
    //语音通道
    case AFK_CHANNEL_TYPE_TALK:
        {
			if (!m_Deviceinfo.istalk)
			{
				break;
			}
			
			ad_parm = (afk_talk_channel_param_s*)parm;
			if (sendTalkRequest_comm(this, ad_parm->no, true, ad_parm->nEncodeType))
			{
				dvr2_talk_channel = new CDvrTalkChannel(this, AFK_CHANNEL_TYPE_TALK, parm);
				{
					CReadWriteMutexLock lock(m_csaudio);

					m_audio_channel = dvr2_talk_channel;
				}

				return dvr2_talk_channel;
			}
		}
        break;
    //用户通道
    case AFK_CHANNEL_TYPE_USER:
        {
			uo_parm = (afk_user_channel_param_s*)parm;
			if (sendUserOperate_comm(this, uo_parm->type, uo_parm->userbuf, uo_parm->buflen))
			{
				dvr2_user_channel = new CDvrUserChannel(this, AFK_CHANNEL_TYPE_USER, parm);
				{
					CReadWriteMutexLock lock(m_csuser);

					m_user_channel = dvr2_user_channel;
				}

				return dvr2_user_channel;
			}
		}
        break;
    //解码通道
    case AFK_CHANNEL_TYPE_DECODE:
        break;
    //渲染通道
    case AFK_CHANNEL_TYPE_RENDER:
        break;
    //用户数据
    case AFK_CHANNEL_TYPE_USERMSG:
        break;
	//抓图通道
    case AFK_CHANNEL_TYPE_CAPTURE:
		{
			sp_parm = (afk_snap_channel_param_s*)parm;
			if (sp_parm->no < m_Deviceinfo.channelcount && sp_parm->no >= 0)
			{
				if (sendSnap_comm(this,sp_parm->base.udata,1))
				{
					dvr2_snap_channel = new CDvrSnapChannel(this, AFK_CHANNEL_TYPE_CAPTURE, parm);
					if (dvr2_snap_channel)
					{
						CReadWriteMutexLock lock(m_cssnap);

						if ((unsigned int)sp_parm->no < m_snap_channel_list.size())
						{
							m_snap_channel_list[sp_parm->no] = dvr2_snap_channel;
						}
						else
						{
							delete dvr2_snap_channel;
							dvr2_snap_channel = NULL;
						}
					}
					else
					{
						//new a channel failed
					}
					
					return dvr2_snap_channel;
				}
			}
		}
		break;
	//GPS数据通道
    case AFK_CHANNEL_TYPE_GPS:
		{
			gps_parm = (afk_gps_channel_param_s*)parm;
			if (sendGpsAlarm_comm(this, gps_parm))
			{
				mobile_gps_channel = new CDvrGpsChannel((CDvrDevice*)this, AFK_CHANNEL_TYPE_GPS, parm);
				{
					CReadWriteMutexLock lock(m_csgps);
					
					m_gps_channel = mobile_gps_channel;
				}
				
				return mobile_gps_channel;
			}
		}
        break;
	//请求通道
	case AFX_CHANNEL_TYPE_REQUEST:
		{
			afk_request_channel_param *pRequestParam = (afk_request_channel_param *)parm;
			if (sendRequestPacket_comm(this, pRequestParam->nType, pRequestParam->nSequence, pRequestParam->nSubType, pRequestParam->pCondition))
			{
				CRequestChannel *pRequestChannel = new CRequestChannel(this, AFX_CHANNEL_TYPE_REQUEST, parm);
				if (pRequestChannel != NULL)
				{
					CReadWriteMutexLock lock(m_csRequest);
					m_request_channel_list.push_back(pRequestChannel);
				}

				return pRequestChannel;
			}
		}
		break;
	//新配置通道
	case AFX_CHANNEL_TYPE_NEWCONFIG:
		{
			afk_newconfig_channel_param *pNewConfigParam = (afk_newconfig_channel_param *)parm;
			if (sendNewConfigPacket_comm(this, pNewConfigParam->nSequence, pNewConfigParam->pCondition))
			{
				CDvrNewConfigChannel *pNewConfigChannel = new CDvrNewConfigChannel(this, AFX_CHANNEL_TYPE_NEWCONFIG, parm);
				if (pNewConfigChannel != NULL)
				{
					CReadWriteMutexLock lock(m_csnewconfig);
					m_newconfig_channel_list.push_back(pNewConfigChannel);
				}

				return pNewConfigChannel;
			}
		}
		break;
    default:
        break;
    }

    return 0;
}

/* 版本信息 */
char* CDvrDevice::device_version()
{
	return NULL;
}

/* 制造商信息 */							
char* CDvrDevice::device_manufactory()
{
	return NULL;
}

/* 产品信息 */
char* CDvrDevice::device_product()
{
	return NULL;
}

/* 获取信息 */
int CDvrDevice::device_get_info(int type, void *parm)
{
	if (NULL == parm)
	{
		return -1;
	}
	
	switch(type)
	{
	case dit_special_cap_flag:	//特殊能力标志,param = DWORD*　，按位表示
		{
			CReadWriteMutexLock lock(m_csSpecial);

			*(DWORD*)parm = m_dwSpecial;
		}
		break;
	case dit_protocol_version:
		{
			*(int*)parm = m_nProtocolVer;
		}
		break;
	case dit_talk_encode_type:
		{
			memcpy(parm, &m_stCurTalkFmt, sizeof(AUDIO_ATTR_T));
		}
		break;
	case dit_recordprotocol_type:
		{
			*(int*)parm = m_nRecordProto;
		}
		break;
	case dit_logprotocol_type:
		{
			*(int*)parm = m_nLogProto;
		}
		break;
	case dit_session_id:
		{
			*(int*)parm = m_nSessionId;
		}
		break;
	case dit_ether_flag:
		{
			*(int*)parm = m_nIsMutiEther;
		}
		break;
	case dit_talk_use_mode:
		{
			*(int*)parm = m_nTalkMode;
		}
		break;
	case dit_card_query:
		{
			*(int*)parm = m_nCardQuery;
		}
		break;
	case dit_alarmin_num:
		{
			*(int*)parm = m_nAlarmInCaps;
		}
		break;
	case dit_talk_encode_list:
		{
			*(unsigned long*)parm = m_dwTalkFormat;
		}
		break;
	case dit_audio_alarmin_num:
		{
			*(int*)parm = m_nAudioAlarmInCaps;
		}
		break;
	case dit_dev_type:
		{
			strcpy((char*)parm, m_Deviceinfo.szDevType);
		}
		break;
	case dit_alarm_listen_use_mode:
		{
			memcpy(parm, m_dwAlarmListenMode, sizeof(m_dwAlarmListenMode));
		}
		break;
	case dit_multiplay_flag:
		{
			*(int*)parm = m_nMultiPlayFlag;
		}
		break;
	case dit_device_task:
		{
			*(int*)parm = m_nIsHaveTask;
		}
		break;
	case dit_online_flag:
		{
			int nIsOnline = 0;
			if (m_pTcpSocket != NULL)
			{
				nIsOnline = m_pTcpSocket->GetIsOnline();
			}

			*(int*)parm = nIsOnline;
		}
		break;
	case dit_user_name:
		{
			strcpy((char *)parm, m_szUser);
		}
		break;
	case dit_config_authority_mode:
		{
			memcpy(parm, &m_nAuthorityInfo, sizeof(DVR_AUTHORITY_INFO));
		}
		break;
	case dit_set_callback:
		{
			DEVICE_MESS_CALLBACK *pMessFunc = (DEVICE_MESS_CALLBACK *)parm;
			pMessFunc->fun = m_messCallFunc.fun;
			pMessFunc->data = m_messCallFunc.data;
		}
		break;
//	case dit_partition_ver:
//		{
//			*((int*)parm) = m_nPartVer; 
//		}
//		break;
//	case dit_support_partition:		//支持分区个数
//		{
//			*((int*)parm) = m_nSupprotPartNum;
//		}
//		break;
		
	case dit_firstqueryrecord_flag:
		{
			*((int *)parm) = m_nImportantRecord;
		}
		break;
	case dit_nvd_playback_flag:
		{
			DEVICE_MESS_CALLBACK *pMessFunc = (DEVICE_MESS_CALLBACK *)parm;
			pMessFunc->fun = m_nvdPlaybackCall.fun;
			pMessFunc->data = m_nvdPlaybackCall.data;
		}
		break;
	case dit_svr_alarm_flag:
		{
			switch((int)parm)
			{
			case 1:	// 外部报警
				return (int)m_dwCurInputAlarm;
				break;
			case 2: // 动态检测
				return (int)m_dwCurMotionAlarm;
			    break;
			case 3:	// 视频丢失
				return (int)m_dwCurVideoLost;
				break;
			case 4:	// 视频遮挡
				return (int)m_dwCurShelter;
				break;
			default:
			    break;
			}
		}
		break;
	case dit_login_device_flag:
		{
			memcpy(parm, &m_loginType, sizeof(afk_login_device_type));
		}
		break;
	case dit_ether_info:
		{
			memcpy(parm, &m_stEthernet, 5 * sizeof(CONFIG_ETHERNET));
		}
		break;
	case dit_quick_config:
		{
			*(int*)parm = m_nQuickConfig;
		}
		break;
	default:
		break;
	}
	return 0;
}

/* 设置信息 */
int CDvrDevice::device_set_info(int type, void *parm)
{
	switch(type)
	{
	case dit_special_cap_flag:	//特殊能力标志,param = DWORD*　，按位表示
		{
			CReadWriteMutexLock lock(m_csSpecial);

			m_dwSpecial |= *(DWORD*)parm;
		}
		break;
	case dit_protocol_version:
		{
			m_nProtocolVer = *(int*)parm;
		}
		break;
	case dit_talk_encode_type:
		{
			memcpy(&m_stCurTalkFmt, parm, sizeof(AUDIO_ATTR_T));
		}
		break;
	case dit_recordprotocol_type:
		{
			m_nRecordProto = *(int*)parm;
		}
		break;
	case dit_logprotocol_type:
		{
			m_nLogProto = *(int*)parm;
		}
		break;
	case dit_session_id:
		{
			m_nSessionId = *(unsigned int*)parm;
		}
		break;
	case dit_talk_use_mode:
		{
			m_nTalkMode = *(int*)parm;
		}
		break;
	case dit_ether_flag:
		{
			m_nIsMutiEther = *(int*)parm;
		}
		break;
	case dit_card_query:
		{
			m_nCardQuery = *(int*)parm;
		}
		break;
	case dit_alarmin_num:
		{
			m_nAlarmInCaps = *(int*)parm;
		}
		break;
	case dit_talk_encode_list:
		{
			m_dwTalkFormat = *(unsigned long*)parm;
		}
		break;
	case dit_audio_alarmin_num:
		{
			m_nAudioAlarmInCaps = *(int*)parm;
		}
		break;
	case dit_alarm_listen_use_mode:
		{
			// 高两位：通道掩码起始值；低两位：订阅通道掩码
			DWORD dwAlarmMask = *(DWORD*)parm;
			int nNum = dwAlarmMask >> 16;
			dwAlarmMask &= 0xFFFF;
			if (nNum%2 != 0)
			{
				m_dwAlarmListenMode[nNum/2] &= 0xFFFF;
				m_dwAlarmListenMode[nNum/2] |= dwAlarmMask<<16;
			}
			else
			{
				m_dwAlarmListenMode[nNum/2] &= 0xFFFF0000;
				m_dwAlarmListenMode[nNum/2] |= dwAlarmMask;
			}
		}
		break;
	case dit_multiplay_flag:
		{
			m_nMultiPlayFlag = *(int*)parm;
		}
		break;
	case dit_heart_beat:
		{
			if (m_pTcpSocket != NULL)
			{
				m_pTcpSocket->Heartbeat();
			}

			if (m_nProtocolVer >= 6 && GetTickCountEx() - m_dwLastSubKeep >= 10000)
			{
				m_dwLastSubKeep = GetTickCountEx();
				
				CReadWriteMutexLock lock(m_csSubTcp, false, true, false);
				std::map<unsigned int, CTcpSocket*>::iterator it = m_lstSubTcpSockets.begin();
				while (it != m_lstSubTcpSockets.end())
				{
					CTcpSocket *pSubTcpSocket = (*it).second;
					if (pSubTcpSocket != NULL)
					{
						pSubTcpSocket->Heartbeat();
					}
					it++;
				}
			}
		}
		break;
	case dit_cleanres_flag:
		{
			CleanDisConn();
		}
		break;
	case dit_device_task:
		{
			m_nIsHaveTask = *(int*)parm;
			m_emTaskStatus = EM_SEND_LOGINPACKET;
		}
		break;
	case dit_execute_task:
		{
			if (m_pTcpSocket != NULL)
			{
				ExecuteAllTask();
			}
		}
		break;
	case dit_config_authority_mode:
		{
			memcpy(&m_nAuthorityInfo, parm, sizeof(DVR_AUTHORITY_INFO));
		}
		break;
	case dit_set_callback:
		{
			DEVICE_MESS_CALLBACK *pMessFunc = (DEVICE_MESS_CALLBACK *)parm;
			m_messCallFunc.fun = pMessFunc->fun;
			m_messCallFunc.data = pMessFunc->data;
		}
		break;
	case dit_asyn_timeout:
		{
			CReadWriteMutexLock lock(m_csRequest);

			CRequestChannel *pRequestchannel = NULL;
			std::list<CDvrChannel*>::iterator it = m_request_channel_list.begin();
			while (it != m_request_channel_list.end())
			{
				pRequestchannel = (CRequestChannel*)(*it);
				if (pRequestchannel != NULL)
				{
					BOOL bTimeout = FALSE;
					pRequestchannel->channel_get_info(channel_detect_timeout, &bTimeout);
					if (bTimeout)
					{
						m_request_channel_list.erase(it++);
						pRequestchannel->channel_decRef();
						continue;
					}
				}

				it++;
			}

			lock.Unlock();
		}
		break;
//	case dit_partition_ver:
//		{
//			m_nPartVer = *(int *)parm;
//		}
//		break;
//	case dit_support_partition:		//支持分区个数
//		{
//			m_nSupprotPartNum = *(int *)parm;
//		}
//		break;
	case dit_firstqueryrecord_flag:
		{
			m_nImportantRecord = *(int *)parm;
		}
		break;
	case dit_nvd_playback_flag:
		{
			DEVICE_MESS_CALLBACK *pMessFunc = (DEVICE_MESS_CALLBACK *)parm;
			m_nvdPlaybackCall.fun = pMessFunc->fun;
			m_nvdPlaybackCall.data = pMessFunc->data;
		}
		break;
	case dit_login_device_flag:
		{
			memcpy(&m_loginType, parm, sizeof(afk_login_device_type));
		}
		break;
	case dit_ether_info:
		{
			memcpy(&m_stEthernet, parm, 5 * sizeof(CONFIG_ETHERNET));
		}
		break;
	case dit_quick_config:
		{
			m_nQuickConfig = *(int *)parm;
		}
		break;	
	default:
		break;
	}
	
	return 0;
}

/*
 * 摘要：创建动态子连接
 */
void* CDvrDevice::CreateSubConn(afk_connect_param_t *pConnParam)
{
	if (pConnParam == NULL)
	{
		return NULL;
	}

	switch(pConnParam->nConnType)
	{
	case channel_connect_tcp:
		{
			// 建立子连接
			CTcpSocket *pSubTcpSocket = device_create_connect<CTcpSocket>(pConnParam);
			if (pSubTcpSocket != NULL)
			{
				pSubTcpSocket->SetIsReConn(0);
		
				// 注册子连接
				if (m_nProtocolVer >= 6)
				{
					unsigned char heartbeat[HEADER_SIZE];
					memset(heartbeat, 0, HEADER_SIZE);
					heartbeat[0] = 0xA1;
#ifdef NETSDK_VERSION_INVERSE
					for (int i = 0; i < HEADER_SIZE; i++)
					{
						unsigned char *p = (unsigned char*)(heartbeat + i);
						*p = ~(*p);
					}
#endif
					pSubTcpSocket->SetKeepLife(heartbeat, HEADER_SIZE);
					pSubTcpSocket->SetIsDetectDisconn(1);

					int nExtLen = 0;
					unsigned char data[HEADER_SIZE + 256] = {0};
					data[0] = 0xF4;
					
					sprintf((char*)(data+HEADER_SIZE), "TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.ControlConnection.AckSubChannel\r\nSessionID:%d\r\nConnectionID:%d\r\n\r\n", m_nSessionId, pConnParam->nConnectID);
					nExtLen = strlen((char*)(data+HEADER_SIZE));
				
					*(DWORD*)(data + 4) = nExtLen;
					pSubTcpSocket->WriteData((char*)data, HEADER_SIZE+nExtLen);

					DWORD dwRet = WaitForSingleObjectEx(pSubTcpSocket->m_hRecEvent, pConnParam->nParam);
					ResetEventEx(pSubTcpSocket->m_hRecEvent);
					if (dwRet != WAIT_OBJECT_0 || pSubTcpSocket->m_registerAck[8] != 0)
					{
						pSubTcpSocket->Disconnect();
						delete pSubTcpSocket;
						return NULL;
					}
					
					pSubTcpSocket->SetCallBack(OnSubDisconnect, OnSubReConnect, OnSubOtherPacket, OnSubReceivePacket, pConnParam->userdata);
				}
				else
				{
					pSubTcpSocket->SetIsDetectDisconn(0);

#ifdef NETSDK_VERSION_BOGUSSSL
					pSubTcpSocket->m_nSSL = 7;
#endif

					unsigned char data[HEADER_SIZE] = {0};
					data[0] = 0xf1;	//注册命令0xf1
					*(unsigned int*)(data + 8) = m_nSessionId;
					data[12] = (pConnParam->nConnectID == GetVideoChannelCount()) ? 2 : 1;
					data[13] = pConnParam->nConnectID + 1;
					
					pSubTcpSocket->WriteData((char*)data, HEADER_SIZE);

					if (pConnParam->nParam > 0)
					{
						DWORD dwRet = WaitForSingleObjectEx(pSubTcpSocket->m_hRecEvent, pConnParam->nParam);
						ResetEventEx(pSubTcpSocket->m_hRecEvent);
						if (dwRet != WAIT_OBJECT_0 || pSubTcpSocket->m_registerAck[0] != 0)
						{
							pSubTcpSocket->Disconnect();
							delete pSubTcpSocket;
							return NULL;
						}
					}

					pSubTcpSocket->SetCallBack(NULL, NULL, OnOtherPacket, OnReceivePacket, this);
				}
				
				CReadWriteMutexLock lock(m_csSubTcp);
				m_lstSubTcpSockets[pConnParam->nConnectID] = pSubTcpSocket;
				lock.Unlock();
				
				return pSubTcpSocket;
			}
		}
		break;
	case channel_connect_udp:
		{
 			CUdpSocket *pSubUdpSocket = device_create_connect<CUdpSocket>(pConnParam);
			if (pSubUdpSocket != NULL)
			{
				pSubUdpSocket->SetCallBack(OnSubOtherPacket, OnSubReceivePacket, pConnParam->userdata);

				CReadWriteMutexLock lock(m_csSubTcp);
				m_lstSubUdpSockets[pConnParam->nConnectID] = pSubUdpSocket;
				lock.Unlock();
				
				pSubUdpSocket->GetSockLocalAddr(pConnParam->szLocalIp, pConnParam->nLocalPort);

				return pSubUdpSocket;
			}
		}
		break;
	case channel_connect_multicast:
		{
			CMulticastSocket *pSubMulticast = device_create_connect<CMulticastSocket>(pConnParam);
			if (pSubMulticast != NULL)
			{
				pSubMulticast->SetCallBack(OnSubOtherPacket, OnSubReceivePacket, pConnParam->userdata);

				CReadWriteMutexLock lock(m_csSubTcp);
				m_lstMulticastSockets[pConnParam->nConnectID] = pSubMulticast;
				lock.Unlock();
				
				return pSubMulticast;
			}
		}
	    break;
	default:
	    break;
	}

	return NULL;
}

/*
 * 摘要：删除动态子连接
 */
void CDvrDevice::DestroySubConn(int type, void *pSubSocket, int nConnectID)
{
	if (pSubSocket == NULL)
	{
		return;
	}

	switch(type)
	{
	case channel_connect_tcp:
		{
			CTcpSocket *pSubTcpSocket = (CTcpSocket*)pSubSocket;
			if (pSubTcpSocket != NULL)
			{
				pSubTcpSocket->SetCallBack(NULL, NULL, NULL, NULL, NULL);

				CReadWriteMutexLock lock(m_csSubTcp);
				m_lstSubTcpSockets.erase(nConnectID);
				lock.Unlock();

				CReadWriteMutexLock lock1(m_csDisTcp);
				m_lstDisTcpSockets.push(pSubTcpSocket);
				lock1.Unlock();

				if (m_pfDevEvent != NULL)
				{
					m_pfDevEvent(this, EVENT_DEVICE_DEAL, this, m_userdata);
				}
			}
		}
		break;
	case channel_connect_udp:
		{
			CUdpSocket *pSubUdpSocket = (CUdpSocket*)pSubSocket;
			if (pSubUdpSocket != NULL)
			{
				pSubUdpSocket->SetCallBack(NULL, NULL, NULL);

				CReadWriteMutexLock lock(m_csSubTcp);
				m_lstSubUdpSockets.erase(nConnectID);
				lock.Unlock();

				CReadWriteMutexLock lock1(m_csDisTcp);
				m_lstDisUdpSockets.push(pSubUdpSocket);
				lock1.Unlock();

				if (m_pfDevEvent != NULL)
				{
					m_pfDevEvent(this, EVENT_DEVICE_DEAL, this, m_userdata);
				}
			}
		}
		break;
	case channel_connect_multicast:
		{
			CMulticastSocket *pSubMulticast = (CMulticastSocket*)pSubSocket;
			if (pSubMulticast != NULL)
			{
				pSubMulticast->SetCallBack(NULL, NULL, NULL);

				CReadWriteMutexLock lock(m_csSubTcp);
				m_lstMulticastSockets.erase(nConnectID);
				lock.Unlock();

				CReadWriteMutexLock lock1(m_csDisTcp);
				m_lsDistMultiSockets.push(pSubMulticast);
				lock1.Unlock();

				if (m_pfDevEvent != NULL)
				{
					m_pfDevEvent(this, EVENT_DEVICE_DEAL, this, m_userdata);
				}
			}
		}
	    break;
	default:
	    break;
	}
}

/*
 * 摘要：单独线程删除连接资源
 */
void CDvrDevice::CleanDisConn()
{
	BOOL bExit = FALSE;
	while (!bExit)
	{
		CTcpSocket *pTcpSocket = NULL;

		CReadWriteMutexLock lock(m_csDisTcp);
		if (!m_lstDisTcpSockets.empty())
		{
			pTcpSocket = m_lstDisTcpSockets.front();
			m_lstDisTcpSockets.pop();
		}
		else
		{
			bExit = TRUE;
		}
		lock.Unlock();
		
		if (pTcpSocket != NULL)
		{
			pTcpSocket->Disconnect();
			delete pTcpSocket;
		}
	}

	bExit = FALSE;
	while (!bExit)
	{
		CUdpSocket *pUdpSocket = NULL;

		CReadWriteMutexLock lock(m_csDisTcp);
		if (!m_lstDisUdpSockets.empty())
		{
			pUdpSocket = m_lstDisUdpSockets.front();
			m_lstDisUdpSockets.pop();
		}
		else
		{
			bExit = TRUE;
		}
		lock.Unlock();
		
		if (pUdpSocket != NULL)
		{
			pUdpSocket->Disconnect();
			delete pUdpSocket;
		}
	}

	bExit = FALSE;
	while (!bExit)
	{
		CMulticastSocket *pMultiSocket = NULL;

		CReadWriteMutexLock lock(m_csDisTcp);
		if (!m_lsDistMultiSockets.empty())
		{
			pMultiSocket = m_lsDistMultiSockets.front();
			m_lsDistMultiSockets.pop();
		}
		else
		{
			bExit = TRUE;
		}
		lock.Unlock();
		
		if (pMultiSocket != NULL)
		{
			pMultiSocket->Disconnect();
			delete pMultiSocket;
		}
	}
}

void CDvrDevice::ExecuteAllTask()
{
	if (m_nIsHaveTask == 0)
	{
		return;
	}

	// 状态机，单线程处理
	switch(m_emTaskStatus)
	{
	case EM_SEND_LOGINPACKET:
		{
			//	发送登入包
			unsigned int  commandlen = 0;
			unsigned char command[160] = {0};
			unsigned char ver = 5;
			
			build_login_packet(command, m_szUser, m_szPassword, 0, 0, &commandlen);	
			m_pTcpSocket->WriteData((char*)command, HEADER_SIZE+commandlen);

			m_dwLastLoginPacket = GetTickCountEx();
			m_emTaskStatus = EM_WAIT_LOGINACK;
		}
		break;
	case EM_WAIT_LOGINACK:
		{
			//	等待返回包
			DWORD dwRet = WaitForSingleObjectEx(m_pTcpSocket->m_hRecEvent, 0);
			if (dwRet != WAIT_OBJECT_0)
			{
				if (GetTickCountEx() - m_dwLastLoginPacket >= 5000)
				{
					m_emTaskStatus = EM_SEND_LOGINPACKET;
				}
				break;
			}
			else
			{
				m_emTaskStatus = EM_PARSE_LOGINACK;
			}
		}
//		break;
	case EM_PARSE_LOGINACK:
		{
			//	分析返回包
			if (*(m_pTcpSocket->m_registerAck + 8) != 0)
			{
				m_emTaskStatus = EM_SEND_LOGINPACKET;
				break;
			}
			else
			{
				m_nSessionId = *(unsigned int*)(m_pTcpSocket->m_registerAck + 16);
				if (m_nProtocolVer >= 6)
				{
					m_emTaskStatus = EM_DYNAMIC_SUBCONN;
					goto e_dynamic_reconn;
				}
				else
				{
					// 多连接模型下的子连接重连
					CReadWriteMutexLock lock(m_csSubTcp, false, true, false);
					
					std::map<unsigned int, CTcpSocket*>::iterator it = m_lstSubTcpSockets.begin();
					while (it != m_lstSubTcpSockets.end())
					{
						CTcpSocket *pSubTcpSocket = (*it).second;
						if (pSubTcpSocket != NULL)
						{
							pSubTcpSocket->CloseSubConn();
						}
						it++;
					}
					
					m_pCurSubTcpSocket = m_lstSubTcpSockets.begin();
					
					lock.Unlock();
					
					m_emTaskStatus = EM_RECONN_SUBCONN;
				}
			}
		}
//	    break;
	case EM_RECONN_SUBCONN:
		{
			//	子连接重连
e_reconn_subconn:
			CReadWriteMutexLock lock(m_csSubTcp, false, true, false);

			if (m_pCurSubTcpSocket != m_lstSubTcpSockets.end())
			{
				CTcpSocket *pSubTcpSocket = (*m_pCurSubTcpSocket).second;
				if (pSubTcpSocket->ConnectSubConn() > 0)
				{
					// 发送子连接注册命令
					unsigned char command[HEADER_SIZE] = {0};
					command[0] = 0xF1;	//注册命令0xF1
					memcpy(command + 8, m_pTcpSocket->m_registerAck + 16, 4);
					command[12] = ((*m_pCurSubTcpSocket).first == m_Deviceinfo.channelcount) ? 2 : 1;
					command[13] = (*m_pCurSubTcpSocket).first + 1;
					
					pSubTcpSocket->WriteData((char*)command, HEADER_SIZE);
					
					if (*(m_pTcpSocket->m_registerAck + 9) != 8)
					{
						m_pCurSubTcpSocket++;
					}
					else
					{
						m_dwLastLoginPacket = GetTickCountEx();
						m_emTaskStatus = EM_WAIT_SUBCONN;
					}
				}

				break;
			}
			else
			{
				m_emTaskStatus = EM_SEND_REALPACKET;
			}
		}
//	    break;
	case EM_WAIT_SUBCONN:
		{
			CReadWriteMutexLock lock(m_csSubTcp, false, true, false);

			if (m_pCurSubTcpSocket != m_lstSubTcpSockets.end())
			{
				CTcpSocket *pSubTcpSocket = (*m_pCurSubTcpSocket).second;
				DWORD dwRet = WaitForSingleObjectEx(pSubTcpSocket->m_hRecEvent, 0);
				if (dwRet == WAIT_OBJECT_0 || GetTickCountEx() - m_dwLastLoginPacket >= 3000)
				{
					ResetEventEx(pSubTcpSocket->m_hRecEvent);
					m_pCurSubTcpSocket++;
					m_emTaskStatus = EM_RECONN_SUBCONN;
					goto e_reconn_subconn;
				}
				break;
			}
			else
			{
				m_emTaskStatus = EM_SEND_REALPACKET;
			}
		}
//		break;
	case EM_SEND_REALPACKET:
		{
			//	打开监视
			CReadWriteMutexLock lock(m_csmedia, false, true, false);
			
			std::map<unsigned int, CDvrChannel*>::iterator it = m_media_channel_list.begin();
			while (it != m_media_channel_list.end())
			{
				CDvrMediaChannel *pMediaChannel = (CDvrMediaChannel*)(*it).second;
				if (pMediaChannel)
				{
					sendMonitor_comm(this, pMediaChannel->GetChannelIndex(), pMediaChannel->GetChannelSubtype(), true, pMediaChannel->GetConnectType(), pMediaChannel->GetChannelConnectID(), pMediaChannel->GetConnLocolIp(), pMediaChannel->GetConnLocolPort());
				}
				it++;
			}
			
			lock.Unlock();
			
			m_nIsHaveTask = 0;

			fDisconnectCallBack pfDisconnectFunc = GetDisconnectFunc();
			void *userdata = GetDisconnectUserdata();
			if (pfDisconnectFunc != NULL)
			{
				pfDisconnectFunc(this, NULL, TRUE, m_szIp, m_nPort, 0, userdata);
			}
		}
		break;
	case EM_DYNAMIC_SUBCONN:
		{
e_dynamic_reconn:
			m_nIsHaveTask = 0;
			
			fDisconnectCallBack pfDisconnectFunc = GetDisconnectFunc();
			void *userdata = GetDisconnectUserdata();
			if (pfDisconnectFunc != NULL)
			{
				pfDisconnectFunc(this, NULL, TRUE, m_szIp, m_nPort, 0, userdata);
			}
		}
		break;
	default:
	    break;
	}
}




