
#include "StdAfx.h"
#include "DevConfig.h"
#include "Manager.h"
#include "netsdktypes.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "Utils.h"
#include "../dvr/def.h"
#include "../dvr/kernel/afkinc.h"
#include "Utils_StrParser.h"
#include "DevConfigEx.h"
#include "DecoderDevice.h"
#include "../dvr/ParseString.h"
#include <math.h>

#ifdef WIN32
//Define this in any implementation, before "pugxml.h", to be notified of API campatibility.
#define PUGAPI_VARIANT 0x58475550	//The Pug XML library variant we are using in this implementation.
#define PUGAPI_VERSION_MAJOR 1		//The Pug XML library major version we are using in this implementation.
#define PUGAPI_VERSION_MINOR 2		//The Pug XML library minor version we are using in this implementation.
//Include the Pug XML library.
#include "pugxml.h"
using namespace std;
using namespace pug;
#else	//linux
#endif

#define GET_DEVICE_TYPE(lLoginID)	(((afk_device_s*)lLoginID)->device_type((afk_device_s*)lLoginID))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//static int g_query_wait = 500;

CDevConfig::CDevConfig(CManager *pManager)
	: m_pManager(pManager)
{
	CreateEventEx(m_hRecEvent, TRUE, FALSE);
}

CDevConfig::~CDevConfig()
{
	CloseEventEx(m_hRecEvent);
}

int CDevConfig::Init()
{
	return 0;
}

int CDevConfig::Uninit()
{
	return 0;
}

int CDevConfig::CloseChannelOfDevice(afk_device_s* device)
{
	return 0;
}

int __stdcall QueryConfigFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata)
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data || false == receivedata->addRef())
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}
	
	/**/
	if (CONFIG_TYPE_PPPOE == receivedata->reserved[0] || CONFIG_TYPE_DDNS == receivedata->reserved[0])
	{
		*receivedata->datalen = datalen + 4;
		//缓冲区不够大
		if (datalen + 4 > (unsigned int)receivedata->maxlen)
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
		
		*(int *)receivedata->data = (int)param;
		memcpy(receivedata->data + 4, data, datalen);
		receivedata->result = 0;
		SetEventEx(receivedata->hRecEvt);	
	}
	else
	{
		*receivedata->datalen = datalen;
		//缓冲区不够大
		if (datalen > (unsigned int)receivedata->maxlen)
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
		
		memcpy(receivedata->data, data, datalen);
		receivedata->result = 0;
		SetEventEx(receivedata->hRecEvt);
	}
	return 1;
}

int	CDevConfig::QueryConfig(LONG lLoginID, int nConfigType, int nParam,
                                   char *pConfigbuf, int maxlen, int *nConfigbuflen, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pConfigbuf || !nConfigbuflen)
	{
		return NET_ILLEGAL_PARAM;		
	}
	
	int iRet =-1;
    int iConfigType = AFK_CHANNEL_SEARCH_CONFIG;
	int iSubType = 0;

	switch(nConfigType) 
	{
	case CONFIG_TYPE_GENERAL:		/*普通配置*/
	case CONFIG_TYPE_SERIAL_COMM:	/*串口配置*/
	case CONFIG_TYPE_NET:			/*网络配置*/
	case CONFIG_TYPE_RECORD:		/*定时录象配置*/
	case CONFIG_TYPE_CAPTURE:		/*图像配置*/
	case CONFIG_TYPE_PTZ:			/*云台步长*/
	case CONFIG_TYPE_DETECT:		/*图像检测配置*/
	case CONFIG_TYPE_ALARM:			/*报警配置*/
	case CONFIG_TYPE_DISPLAY:		/*图像显示配置*/
	case CONFIG_TYPE_TITLE:			/*通道名称，设备2.4版本后不再支持*/
	case CONFIG_TYPE_MAIL:			/*邮件配置*/
	case CONFIG_TYPE_PREVIEW:		/*预览配置*/
	case CONFIG_TYPE_PPPOE:			/*PPPoE配置*/
	case CONFIG_TYPE_DDNS:			/*DDNS配置*/
	case CONFIG_TYPE_STORSTATION:   //存储位置
	case CONFIG_TYPE_SNIFFER:		/*网络抓包配置*/
	case CONFIG_TYPE_DSPCAP:		/*编码能力信息*/
	case CONFIG_TYPE_FTP_PRO:		/*FTP配置*/
	case CONFIG_TYPE_DOWNLOAD_STRATEGY:	/*下载策略配置*/
	case CONFIG_TYPE_AUTO_MT:		/*自动维护配置*/
	case CONFIG_TYPE_NTP:			/*NTP配置*/
	case CONFIG_TYPE_LIMIT_BR:		/*限码流配置*/
	case CONFIG_TYPE_WATERMAKE:		/*图象水印配置*/
	case CONFIG_TYPE_VIDEO_MATRIX:
	case CONFIG_TYPE_COVER:			/*视频区域遮挡*/
	case CONFIG_TYPE_DHCP:			/*DHCP配置*/
	case CONFIG_TYPE_WEB_URL:
	case CONFIG_TYPE_CAMERA:		/*摄像头属性配置*/
	case CONFIG_TYPE_ETHERNET:
	case CONFIG_TYPE_DNS:			/*DNS服务器配置*/
	case CONFIG_TYPE_VIDEO_OSD:     /*视频OSD叠加配置*/
	case CONFIG_TYPE_COLOR:			/*颜色配置*/
	case CONFIG_TYPE_CAPTURE_127:	/*双码流图像配置*/
	case CONFIG_TYPE_CAPTURE_128:	/*新的编码配置*/
	case CONFIG_TYPE_AUDIO_CAPTURE: /*音频配置*/
	case CONFIG_TYPE_WLAN:			/*查询无线配置*/
	case CONFIG_TYPE_TRANSFER_STRATEGY:
	case CONFIG_TYPE_WIRELESS_ADDR:
	case CONFIG_TYPE_WLAN_DEVICE:	/*搜索无线设备*/
	case CONFIG_TYPE_INTERVIDEO:
	case CONFIG_TYPE_OEM_INFO:		/*第三方接入配置*/
	case CONFIG_TYPE_DSP_MASK:
	case CONFIG_TYPE_AUTO_REGISTER:	/*设备主动注册*/

	case CONFIG_TYPE_ALARM_LOCALALM:
	case CONFIG_TYPE_ALARM_NETALM:
	case CONFIG_TYPE_ALARM_DECODER:
	case CONFIG_TYPE_ALARM_MOTION:
	case CONFIG_TYPE_ALARM_BLIND:
	case CONFIG_TYPE_ALARM_LOSS:
	case CONFIG_TYPE_ALARM_NODISK:
	case CONFIG_TYPE_ALARM_DISKERR:
	case CONFIG_TYPE_ALARM_DISKFULL:
	case CONFIG_TYPE_ALARM_NETBROKEN: /*各报警配置*/
	case CONFIG_TYPE_ALARM_ENCODER:
	case CONFIG_TYPE_ALARM_WIRELESS:
	case CONFIG_TYPE_ALARM_AUDIODETECT:
	case CONFIG_TYPE_ALARM_DISKNUM:
	case CONFIG_TYPE_ALARM_PANORAMA:
	case CONFIG_TYPE_ALARM_LOSTFOCUS:
	case CONFIG_TYPE_ALARM_IPCOLLISION:
	case CONFIG_TYPE_TIMESHEET:		/*各报警时间表*/
	case CONFIG_TYPE_RECORD_NEW:	/*新录象配置*/	
	case CONFIG_TYPE_MULTI_DDNS:
	case CONFIG_TYPE_ALARM_CENTER_UP: //报警中心上传配置
	case CONFIG_TYPE_POS:
		iSubType = nConfigType;
		break;
	default:
		return NET_ILLEGAL_PARAM;	
		break;
	}

    afk_device_s *device = (afk_device_s*)lLoginID;
	
    afk_search_channel_param_s searchchannel = {0};
    searchchannel.base.func = QueryConfigFunc;
	searchchannel.param = nParam;
	
    *nConfigbuflen = 0;
	
    receivedata_s receivedata;// = {0};
    receivedata.data = pConfigbuf;
    receivedata.datalen = nConfigbuflen;
    receivedata.maxlen = maxlen;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }
	
	searchchannel.type = iConfigType;
	searchchannel.subtype = iSubType;
	
	receivedata.reserved[0] = nConfigType;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
			ResetEventEx(receivedata.hRecEvt);
			bool b = pchannel->close(pchannel);
			if (!b)
			{
				iRet = NET_CLOSE_CHANNEL_ERROR;
			}
			else
			{
				if (dwRet == WAIT_OBJECT_0)
				{    
					if (receivedata.result == -1)
					{
						iRet = NET_DEV_VER_NOMATCH;
					}
					else
					{
						iRet = 0;
					}
				}
				else
				{
					iRet = NET_NETWORK_ERROR;	
				}
			}
        } // if (waittime != 0)
        else
        {
            iRet = 0;
        }
    }//if (pchannel)
	else
	{
		iRet = NET_OPEN_CHANNEL_ERROR;
	}

    return iRet;
}

int __stdcall SetupConfigFunc(
							  afk_handle_t object,	/* 数据提供者 */
							  unsigned char *data,	/* 数据体 */
							  unsigned int datalen,	/* 数据长度 */
							  void *param,			/* 回调参数 */
							  void *udata)
{
	receivedata_s *receivedata = (receivedata_s*)udata;
    if (receivedata == 0 || false == receivedata->addRef())
    {
        return -1;
    }
	
	if (receivedata->data) 
	{
		*(int *)receivedata->data = datalen; //这里datalen存放设置返回
		receivedata->result = 0;
	}
	
    SetEventEx(receivedata->hRecEvt);
	receivedata->decRef();
    return 1;
}

/*协议文档中的设置配置有返回值，但设备仍未实现 (HB2.32)*/
int CDevConfig::SetupConfig(LONG lLoginID, int nConfigType, int nParam, 
                                   char *pConfigbuf, int nConfigbuflen, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (nConfigbuflen <= 0 || !pConfigbuf)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;
    afk_config_channel_param_s configchannel = {0};
    configchannel.base.func = 0;
    int iConfigType = AFK_CHANNEL_CONFIG_CONFIG;
	int iSubType = 0;

	switch(nConfigType)
	{
	case CONFIG_TYPE_GENERAL:			/*普通配置*/
	case CONFIG_TYPE_SERIAL_COMM:		/*串口配置*/
	case CONFIG_TYPE_NET:				/*网络配置*/
	case CONFIG_TYPE_RECORD:			/*定时录象配置*/
	case CONFIG_TYPE_CAPTURE:			/*图像配置*/
	case CONFIG_TYPE_PTZ:				/*云台步长*/
	case CONFIG_TYPE_DETECT:			/*图像检测配置*/
	case CONFIG_TYPE_ALARM:				/*报警配置*/
	case CONFIG_TYPE_DISPLAY:			/*图像显示配置*/
	case CONFIG_TYPE_TITLE:				/*通道名称，设备2.4版本后不再支持*/
	case CONFIG_TYPE_MAIL:				/*邮件配置*/
	case CONFIG_TYPE_PREVIEW:			/*预览配置*/
	case CONFIG_TYPE_PPPOE:				/*PPPoE配置*/
	case CONFIG_TYPE_DDNS:				/*DDNS配置*/
	case CONFIG_TYPE_STORSTATION:		//存储位置
	case CONFIG_TYPE_SNIFFER:			/*网络抓包配置*/
	case CONFIG_TYPE_FTP_PRO:			/*FTP配置*/
	case CONFIG_TYPE_CAMERA:			/*摄像头属性配置*/
	case CONFIG_TYPE_DOWNLOAD_STRATEGY:	/*下载策略配置*/
	case CONFIG_TYPE_AUTO_MT:			/*自动维护配置*/
	case CONFIG_TYPE_NTP:				/*NTP配置*/
	case CONFIG_TYPE_LIMIT_BR:			/*限码流配置*/
	case CONFIG_TYPE_WATERMAKE:
	case CONFIG_TYPE_VIDEO_MATRIX:
	case CONFIG_TYPE_COVER:				/*视频区域遮挡*/
	case CONFIG_TYPE_DHCP:				/*DHCP配置*/
	case CONFIG_TYPE_WEB_URL:
	case CONFIG_TYPE_ETHERNET:
	case CONFIG_TYPE_DNS:				/*DNS服务器配置*/
	case CONFIG_TYPE_VIDEO_OSD:     /*视频OSD叠加配置*/
	case CONFIG_TYPE_COLOR:				/*颜色配置*/
	case CONFIG_TYPE_CAPTURE_127:		/*双码流图像配置*/
	case CONFIG_TYPE_CAPTURE_128:		/*新的编码配置*/
	case CONFIG_TYPE_AUDIO_CAPTURE:		/*音频配置*/
	case CONFIG_TYPE_WLAN:				/*查询无线配置*/
	case CONFIG_TYPE_TRANSFER_STRATEGY:
	case CONFIG_TYPE_WIRELESS_ADDR:     /*无线报警配置*/
	case CONFIG_TYPE_WLAN_DEVICE:		/*选择无线设备*/
	case CONFIG_TYPE_INTERVIDEO:
	case CONFIG_TYPE_OEM_INFO:			/*第三方接入配置*/
	case CONFIG_TYPE_AUTO_REGISTER:
		
	case CONFIG_TYPE_ALARM_LOCALALM:
	case CONFIG_TYPE_ALARM_NETALM:
	case CONFIG_TYPE_ALARM_DECODER:
	case CONFIG_TYPE_ALARM_MOTION:
	case CONFIG_TYPE_ALARM_BLIND:
	case CONFIG_TYPE_ALARM_LOSS:
	case CONFIG_TYPE_ALARM_NODISK:
	case CONFIG_TYPE_ALARM_DISKERR:
	case CONFIG_TYPE_ALARM_DISKFULL:
	case CONFIG_TYPE_ALARM_NETBROKEN:	/*各报警配置*/
	case CONFIG_TYPE_ALARM_ENCODER:
	case CONFIG_TYPE_ALARM_WIRELESS:
	case CONFIG_TYPE_ALARM_AUDIODETECT:
	case CONFIG_TYPE_ALARM_DISKNUM:
	case CONFIG_TYPE_ALARM_PANORAMA:
	case CONFIG_TYPE_ALARM_LOSTFOCUS:
	case CONFIG_TYPE_ALARM_IPCOLLISION:
	case CONFIG_TYPE_TIMESHEET:			/*各报警时间表*/
	case CONFIG_TYPE_RECORD_NEW:		/*新录象配置*/	
	case CONFIG_TYPE_MULTI_DDNS:
	case CONFIG_TYPE_ALARM_CENTER_UP:  //报警中心的配置
	case CONFIG_TYPE_POS:
		iSubType = nConfigType;
		break;
		/*
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 10:
	case 11:
	case 12: 
	case 13:
	case 14:
		configchannel.type = nConfigType + 2;
		break;
	case 200:
		configchannel.type = 32;
		break;
	case 127:
		configchannel.type = 35;
		break;
	case 128:
		configchannel.type = 38;
		break;
	case 129:
		configchannel.type = 39;
		break;
	case 126:
		configchannel.type = 40;
		break;
	case 17:	//FTP配置
		configchannel.type = 41;
		break;
	case 18:	//自动维护配置
		configchannel.type = 42;
		break;
	case CONFIG_TYPE_ALARM_LOCALALM:
	case CONFIG_TYPE_ALARM_NETALM:
	case CONFIG_TYPE_ALARM_DECODER:
	case CONFIG_TYPE_ALARM_MOTION:
	case CONFIG_TYPE_ALARM_BLIND:
	case CONFIG_TYPE_ALARM_LOSS:
	case CONFIG_TYPE_ALARM_NODISK:
	case CONFIG_TYPE_ALARM_DISKERR:
	case CONFIG_TYPE_ALARM_DISKFULL:
	case CONFIG_TYPE_ALARM_NETBROKEN: //各报警配置
	case CONFIG_ALARM_TIMESHEET: //各报警时间表
	case CONFIG_TYPE_RECORD_NEW:			//新录象配置
		configchannel.type = nConfigType;
		break;
	case 9:
		*/
	default:
		return NET_ILLEGAL_PARAM;
		break;
	}
	
	int setupResult = -1;
	receivedata_s receivedata;// = {0};
    receivedata.data = (char *)&setupResult;
    receivedata.datalen = 0;
    receivedata.maxlen = sizeof(int);
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;

	configchannel.type = iConfigType;
	configchannel.subtype = iSubType;
    configchannel.configinfo.configbuf = (unsigned char*)pConfigbuf;
    configchannel.configinfo.buflen = nConfigbuflen;
    configchannel.base.udata = (void *)&receivedata;
	configchannel.base.func = SetupConfigFunc;
	configchannel.param[0] = nParam;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
		/*
		DWORD dwRet = WaitForSingleObject(m_hRecEvent, waittime);
		switch(dwRet) 
		{
		case WAIT_OBJECT_0:
			if (0 == receivedata.result)
			{
				switch(setupResult) {
				case 0: //成功
					nRet = 0;
					break;
				case 1:	//暂时无法设置
					m_pManager->SetLastError(NET_CONFIG_DEVBUSY);
					break;
				case 2: //数据不合法
					m_pManager->SetLastError(NET_CONFIG_DATAILLEGAL);
					break;
				default:
					m_pManager->SetLastError(NET_SYSTEM_ERROR);
					break;
				}
			}
			else
			{
				m_pManager->SetLastError(NET_DEV_VER_NOMATCH);
			}
			break;
		default:
			m_pManager->SetLastError(NET_NETWORK_ERROR);
			break;
		}
		*/
		nRet = 0;
        if (!pchannel->close(pchannel))
		{
			nRet = NET_CLOSE_CHANNEL_ERROR;
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int __stdcall QueryChannelNameFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}

    *receivedata->datalen = datalen/*/32*/;
    //缓冲区不够大
    if (datalen > (unsigned int)receivedata->maxlen)
    {
        SetEventEx(receivedata->hRecEvt);
        return -1;
    }
    receivedata->result = (int)param;
    memcpy(receivedata->data, data, datalen);
    SetEventEx(receivedata->hRecEvt);
    return 1;
}


int	CDevConfig::QueryChannelName(LONG lLoginID, 
                                        char *pChannelName, int maxlen, int *nChannelCount, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pChannelName || maxlen < 16*CHANNEL_NAME_LEN || !nChannelCount)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.type = AFK_CHANNEL_SEARCH_CHANNELNAME;
    searchchannel.base.func = QueryChannelNameFunc;

	memset(pChannelName, 0, maxlen);
    *nChannelCount = 0;

	int bufLen = device->channelcount(device)*(64 + 2);
	bufLen = bufLen>CONFIG_CAPTURE_NUM*CHANNEL_NAME_LEN? bufLen:CONFIG_CAPTURE_NUM*CHANNEL_NAME_LEN;
	int nRetLen = 0;
	char* buf = new char[bufLen]; //支持新通道名协议
	if (buf == NULL)
	{
		return NET_SYSTEM_ERROR;
	}
	memset(buf, 0, bufLen);

    receivedata_s receivedata;// = {0};
    receivedata.data = buf;
    receivedata.datalen = &nRetLen;
    receivedata.maxlen = bufLen;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {
                if (-1 == receivedata.result)
                {
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else if (0 == receivedata.result)
                {
					//旧通道名，长度512
					//兼容旧的配置接口
					*nChannelCount = nRetLen/32;
					for (int i = 0; i < *nChannelCount; i++)
					{
						if (maxlen >= CHANNEL_NAME_LEN*(i+1))
						{
							Change_Utf8_Assic((unsigned char*)(buf+32*i), pChannelName + CHANNEL_NAME_LEN*i);
						}
					}
                    nRet = 0;
                }
				else if (1 == receivedata.result)
				{
					//标志该设备的特殊能力
					DWORD dwSp = SP_NEW_CHANNAME;
					device->set_info(device, dit_special_cap_flag, &dwSp);
					
					//新的通道名协议。长度不定，暂定最大64字节/通道
					char* tmpbuf = new char[device->channelcount(device)*64];
					if (tmpbuf != NULL)
					{
						memset(tmpbuf, 0, device->channelcount(device)*64);
						int ret = ParseChannelNameBuf(buf, nRetLen, tmpbuf, device->channelcount(device)*64);
						if (ret >= 0)
						{
							*nChannelCount = ret;
							for (int i = 0; i < *nChannelCount; i++)
							{
								if (maxlen >= CHANNEL_NAME_LEN*(i+1))
								{
									Change_Utf8_Assic((unsigned char*)(tmpbuf+64*i), pChannelName + CHANNEL_NAME_LEN*i);
								}
							}
							nRet = 0;
						}
						else
						{
							nRet = NET_RETURN_DATA_ERROR;
						}

						delete[] tmpbuf;
					}
					else
					{
						nRet = NET_SYSTEM_ERROR;
					}
				}
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	
	if (buf != NULL)
	{
		delete[] buf;
	}

    return nRet;
}


int __stdcall QueryLogFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    afk_channel_s *channel = (afk_channel_s*)object;
	if (!channel)
	{
		return -1;
	}

    afk_device_s *device = (afk_device_s*)channel->get_device(channel);
	if (!device)
	{
		return -1;
	}

    QueryLogInfo *pLogInfo = (QueryLogInfo*)udata;
    if (pLogInfo && pLogInfo->cbLogData)
    {
        int parm = (int)param;
        if (parm != -1)
        {
            pLogInfo->cbLogData((LONG)device, 
                (char*)data, datalen, parm, 0, pLogInfo->dwUser);
        }
        else
        {
            pLogInfo->cbLogData((LONG)device, 
                (char*)data, datalen, 0, 1, pLogInfo->dwUser);
            delete pLogInfo;
            channel->close(channel);
        }
    }
    return 1;
}


int	CDevConfig::QueryLogCallback(LONG lLoginID, fLogDataCallBack cbLogData, DWORD dwUser)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

    QueryLogInfo *pLogInfo = new QueryLogInfo;
	if (!pLogInfo)
	{
		return NET_SYSTEM_ERROR;
	}
    pLogInfo->cbLogData = cbLogData;
    pLogInfo->dwUser = dwUser;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.type = AFK_CHANNEL_SEARCH_LOG;
    searchchannel.base.func = QueryLogFunc;
    searchchannel.base.udata = pLogInfo;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        return 0;
    }

	if (pLogInfo)
	{
		delete pLogInfo;
	}
    return NET_ERROR;
}

int __stdcall QueryLogWaitFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
	int ret = -1;
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        ret = -1;
		goto END;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        ret = -1;
		goto END;
	}

    int parm;
	parm = (int)param;
    if (parm != -1)
    {
        if (receivedata->maxlen < parm || receivedata->maxlen < (*receivedata->datalen + datalen))
        {
            SetEventEx(receivedata->hRecEvt);
			ret = -1;
			goto END;
        }

        memcpy(receivedata->data + *receivedata->datalen, data, datalen);
        *receivedata->datalen += datalen;
    }
    else
    {
		if (receivedata->maxlen < (*receivedata->datalen + datalen))
		{
			SetEventEx(receivedata->hRecEvt);
			ret = -1;
			goto END;
		}

        receivedata->result = 0;
        memcpy(receivedata->data + *receivedata->datalen, data, datalen);
        *receivedata->datalen += datalen;
        SetEventEx(receivedata->hRecEvt);
		ret = 1;
    }
END:
	receivedata->decRef();
    return ret;
}

/*
 *	对日志查询的能力级判断
 */
 BOOL	CDevConfig::SearchLogProtocol(LONG lLoginID, int waittime)
{
	 if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
	 {
		 m_pManager->SetLastError(NET_INVALID_HANDLE);
		 return FALSE;
	 }
	 
	 //查看能力集
	 BOOL bLogProto = FALSE;
	 
	 afk_device_s *device = (afk_device_s*)lLoginID;
	 int nSpecial = 0;
	 device->get_info(device, dit_logprotocol_type, &nSpecial);
	 if(-1 == nSpecial)	//没获取过
	 {
		 int nSp = 0;
		 char pBuf[sizeof(NEW_LOG_TYPE_SUPPORT_T)] = {0};
		 int nBufSize = sizeof(NEW_LOG_TYPE_SUPPORT_T);
		 int nLen = 0;
		 int nQueryResult = m_pManager->GetDevConfig().QuerySystemInfo(lLoginID, SYSTEM_INFO_NEWLOGTYPE, pBuf, nBufSize, &nLen, waittime);
		 if (nQueryResult >=0 && nLen == sizeof(NEW_LOG_TYPE_SUPPORT_T))
		 {
			 NEW_LOG_TYPE_SUPPORT_T *pEnable_T= (NEW_LOG_TYPE_SUPPORT_T *)pBuf;
			 if(0==nQueryResult && 1==pEnable_T->isSupportNewLogType)
			 {
				 bLogProto = TRUE;
				 nSp = SP_NEW_LOGPROTOCOL;
				 device->set_info(device, dit_logprotocol_type, &nSp);
			 }
			 else
			 {
				 device->set_info(device, dit_logprotocol_type, &nSp);
			 }
		 }
	 }
	 else if(nSpecial == SP_NEW_LOGPROTOCOL)
	 {
		 bLogProto = TRUE;
	 }

	 return bLogProto;
}

#ifdef WIN32
 /*
  *	解析日志XML数据
  */
int CDevConfig::ParseLogXML(char *pXMLFile, char *pOutLogBuffer, int nMaxLen, int *nResultlen)
{
	if(pXMLFile==NULL || pOutLogBuffer==NULL || nResultlen==NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = 0;
	
	xml_parser*xml = new xml_parser(pXMLFile);
	*nResultlen = 0;
	memset(pOutLogBuffer,0,nMaxLen);
	xml_node document = xml->document();
	xml_node itelem = document.first_element_by_path(_T("./result"));
	if(!itelem.empty())
	{
		xml_node::child_iterator i = itelem.children_begin();
		xml_node::child_iterator m = itelem.children_end();
		for(; i < m; ++i)
		{
			NEWLOG_INFO_ITEM LogItem = {0};
			
			xml_attribute itemTime = i->attribute("time");	//time attribute
			int nAttrTime = itemTime.value_size();
			char *pAttrTime = new char[nAttrTime+1];
			memset(pAttrTime,0,nAttrTime+1);
			memcpy(pAttrTime,itemTime.value(),nAttrTime);
			ConvertTimeType(pAttrTime,nAttrTime+1,&(LogItem.time));
			delete[] pAttrTime;
			
			xml_attribute itemType = i->attribute("type");	//type attribute
			int nAttrType = itemType.value_size();
			char *pAttrType = new char[nAttrType+1];
			memset(pAttrType,0,nAttrType+1);
			memcpy(pAttrType,itemType.value(),nAttrType);
			LogItem.type = (WORD)atoi(pAttrType);
			delete[] pAttrType;
			
			xml_attribute itemData = i->attribute("data");	//data attribute
			int nAttrData = itemData.value_size();
			char *pAttrData = new char[nAttrData+1];
			memset(pAttrData,0,nAttrData+1);
			memcpy(pAttrData,itemData.value(),nAttrData);
			LogItem.data = (WORD)atoi(pAttrData);
			delete[] pAttrData;

			xml_attribute itemOperator = i->attribute("username");	//username attribute
			int nAttrOperator = itemOperator.value_size();
			char *pOperatorUtf8 = new char[nAttrOperator+1];
			char *pAttrOperator = new char[nAttrOperator+1];
			memset(pOperatorUtf8,0,nAttrOperator+1);
			memset(pAttrOperator,0,nAttrOperator+1);
			memcpy(pOperatorUtf8,itemOperator.value(),nAttrOperator);
			Change_Utf8_Assic((unsigned char*)pOperatorUtf8,pAttrOperator);
			memcpy(LogItem.szOperator,pAttrOperator,(nAttrOperator>8)?8:nAttrOperator);
			delete[] pOperatorUtf8;
			delete[] pAttrOperator;

			if(i->has_child_nodes())		//get value
			{
				xml_node::child_iterator node = i->children_begin();
				if(node->type_pcdata())
				{
					int nTagSize = node->value_size();
					char *pTagValue = new char[nTagSize+1];
					memset(pTagValue,0,nTagSize+1);
					memcpy(pTagValue,node->value(),nTagSize);

					nTagSize = nTagSize>sizeof(LogItem.context) ? sizeof(LogItem.context) : nTagSize;
					for(int n=0;n<nTagSize/2;n++)
					{
						sscanf(pTagValue+n*2, "%2x", &(LogItem.context[n]));
					}
					delete[] pTagValue;
				}
			}
			
			if (nMaxLen >= (int)(sizeof(NEWLOG_INFO_ITEM)*(*nResultlen + 1)))	//save
			{
				memcpy(pOutLogBuffer + (*nResultlen)*sizeof(NEWLOG_INFO_ITEM), &LogItem, sizeof(NEWLOG_INFO_ITEM));
				(*nResultlen)++;
			}
			else
			{
				SetLastError(NET_INSUFFICIENT_BUFFER);
				break;
			}
		}
	}

	*nResultlen = (*nResultlen)*sizeof(NEWLOG_INFO_ITEM);
	return nRet;
}
#endif

int CDevConfig::QueryLog(LONG lLoginID, char *pLogBuffer, int maxlen, int *nLogBufferlen, void* reserved, int waittime, int type)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pLogBuffer || !nLogBufferlen) 
	{
		return NET_ILLEGAL_PARAM;
	}

	//如果reserved不为空，查询能力集是否支持新的日志格式(XML)
	int nParam = 0;
	if(reserved!=NULL && (*(DWORD*)reserved)==1)
	{
		if(SearchLogProtocol(lLoginID, waittime))
		{
			nParam = 1;
		}
		else
		{
			return NET_DEV_VER_NOMATCH;
		}
	}

	int nRet = -1;
    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
	searchchannel.subtype = type; //日志类型，0-all
	searchchannel.param = nParam; //置1表示按新的日志格式XML返回数据
    searchchannel.type = AFK_CHANNEL_SEARCH_LOG;
    searchchannel.base.func = QueryLogWaitFunc;
    
    *nLogBufferlen = 0;
	memset(pLogBuffer,0,maxlen);

    receivedata_s receivedata;// = {0};
    receivedata.data = pLogBuffer;
    receivedata.datalen = nLogBufferlen;
    receivedata.maxlen = maxlen;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {
                if (receivedata.result == -1)
                {
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else
                {
                    nRet = 0;
					//如果是新的日志格式，要对XML进行解析
					if(1 == nParam)
					{
#ifdef WIN32
						/*
#ifdef _DEBUG
						FILE *stream;
						if( (stream = fopen("D:\\BBBBBBBBBB.xml", "a+b")) != NULL )
						{
							int numwritten = fwrite( pLogBuffer, sizeof( char ), *nLogBufferlen, stream);
							fclose( stream );
						}
#endif
						*/
						nRet = ParseLogXML(pLogBuffer, pLogBuffer, maxlen, nLogBufferlen);
#else	
						//linux下暂对xml解析不支持。
						nRet = NET_ERROR;
#endif
					}
                }
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
    return nRet;
}


int __stdcall QueryRecordStateFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}

    *receivedata->datalen = datalen;
    //缓冲区不够大
    if (datalen > (unsigned int)receivedata->maxlen)
    {
        SetEventEx(receivedata->hRecEvt);
        return -1;
    }
    receivedata->result = 0;
    memcpy(receivedata->data, data, datalen);
    SetEventEx(receivedata->hRecEvt);
    return 1;
}

int CDevConfig::QueryRecordState(LONG lLoginID, char *pRSBuffer, int maxlen, 
                                        int *nRSBufferlen, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pRSBuffer || !nRSBufferlen)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.type = AFK_CHANNEL_SEARCH_RECORDSTATE; 
    searchchannel.base.func = QueryRecordStateFunc;

    *nRSBufferlen = 0;

    receivedata_s receivedata;// = {0};
    receivedata.data = pRSBuffer;
    receivedata.datalen = nRSBufferlen;
    receivedata.maxlen = maxlen;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
			if (dwRet == WAIT_OBJECT_0)
            {	
                if (receivedata.result == -1)
                {
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else
                {
                    nRet = 0;
                }
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int CDevConfig::SetupRecordState(LONG lLoginID, char *pRSBuffer, int nRSBufferlen)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pRSBuffer) 
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_config_channel_param_s configchannel;
    configchannel.base.func = 0;
    configchannel.type = AFK_CHANNEL_CONFIG_RECORDSTATE;
    configchannel.configinfo.configbuf = (unsigned char*)pRSBuffer;
    configchannel.configinfo.buflen = nRSBufferlen;
    configchannel.base.udata = 0;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
        pchannel->close(pchannel);
        nRet = 0;
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int __stdcall QueryTimeFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}

    LPNET_TIME pDeviceTime = (LPNET_TIME)receivedata->data;
	if (!pDeviceTime)
	{
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}

    pDeviceTime->dwYear = *(data + 16) + 2000;
    pDeviceTime->dwMonth = *(data + 17);
    pDeviceTime->dwDay = *(data + 18);
    pDeviceTime->dwHour = *(data + 19);
    pDeviceTime->dwMinute = *(data + 20);
    pDeviceTime->dwSecond = *(data + 21);

    receivedata->result = 0;
    SetEventEx(receivedata->hRecEvt);
    return 1;
}

int CDevConfig::QueryDeviceTime(LONG lLoginID, LPNET_TIME pDeviceTime, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	if (!pDeviceTime)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.type = AFK_CHANNEL_SEARCH_DEVTIME;
    searchchannel.base.func = QueryTimeFunc;

    receivedata_s receivedata;// = {0};
    receivedata.data = (char*)pDeviceTime;
    receivedata.datalen = 0;
    receivedata.maxlen = 0;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {
                if (receivedata.result == -1)
                {
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else
                {
                    nRet = 0;
                }
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}
	
    return nRet;
}

int CDevConfig::SetupDeviceTime(LONG lLoginID, LPNET_TIME pDeviceTime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pDeviceTime)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_config_channel_param_s configchannel;
    configchannel.base.func = 0;
    configchannel.base.udata = 0;
    configchannel.type = AFK_CHANNEL_CONFIG_DEVTIME;
    configchannel.devicetimeinfo.year = pDeviceTime->dwYear;
    configchannel.devicetimeinfo.month = pDeviceTime->dwMonth;
    configchannel.devicetimeinfo.day = pDeviceTime->dwDay;
    configchannel.devicetimeinfo.hour = pDeviceTime->dwHour;
    configchannel.devicetimeinfo.minute = pDeviceTime->dwMinute;
    configchannel.devicetimeinfo.second = pDeviceTime->dwSecond;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
        pchannel->close(pchannel);
        return 0;
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int __stdcall QueryProtocolFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}

	int proType = receivedata->result;
	receivedata->result = -1;

	switch(proType) 
	{
	case 0:
		*receivedata->datalen = datalen;
		//缓冲区不够大
		if (*receivedata->datalen > (unsigned int)receivedata->maxlen)
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
		receivedata->result = 0;
		memcpy(receivedata->data, data, datalen);
		break;
	case 1:
		*receivedata->datalen = datalen + 4;
		//缓冲区不够大
		if (*receivedata->datalen > (unsigned int)receivedata->maxlen)
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
		receivedata->result = 0;
		memcpy(receivedata->data, &param, 4);
		memcpy(receivedata->data + 4, data, datalen);
		break;
	case 2:
		{
			*receivedata->datalen = datalen;
			if(*receivedata->datalen >(unsigned int )receivedata->maxlen)
			{
				SetEventEx(receivedata->hRecEvt);
				return -1;
			}
			receivedata->result =0;
			memcpy(receivedata->data,data,datalen);
		}
		break;
	default:
		break;
	}
    SetEventEx(receivedata->hRecEvt);
    return 1; 
}

int CDevConfig::QueryComProtocol(LONG lLoginID, int nProtocolType, char *pProtocolBuffer, 
                                        int maxlen, int *nProtocollen, int waittime,DWORD dwProtocolIndex)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pProtocolBuffer || !nProtocollen)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
	if (nProtocolType == 0)
    {
        searchchannel.type = AFK_CHANNEL_SEARCH_COMMPROTOCOL;
    }
    else if (nProtocolType == 1)
    {
        searchchannel.type = AFK_CHANNEL_SEARCH_DCDPROTOCOL;
    }
	else if(nProtocolType == 2)
	{
		searchchannel.type = AFK_CHANNEL_SEARCH_PTZ;
		searchchannel.param = dwProtocolIndex;
	}
    else
    {
        return NET_ILLEGAL_PARAM;
    }
    searchchannel.base.func = QueryProtocolFunc;

    receivedata_s receivedata;// = {0};
    receivedata.data = (char*)pProtocolBuffer;
    receivedata.datalen = nProtocollen;
    receivedata.maxlen = maxlen;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = nProtocolType;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {    
                if (receivedata.result == -1)
                {
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else
                {
                    nRet = 0;
                }
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int __stdcall QuerySystemInfoFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}

    *(receivedata->datalen) = datalen;
    //缓冲区不够大
    if (datalen > (unsigned int)receivedata->maxlen)
    {
        SetEventEx(receivedata->hRecEvt);
        return -1;
    }
    receivedata->result = (int)param;
    memcpy(receivedata->data, data, datalen);
    SetEventEx(receivedata->hRecEvt);
    return 1;
}

/*
 *	查询系统信息
 */
int CDevConfig::QuerySystemInfo(LONG lLoginID, int nSystemType, 
                                       char *pSysInfoBuffer, int maxlen, int *nSysInfolen, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pSysInfoBuffer || !nSysInfolen)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;

    afk_search_channel_param_s searchchannel = {0};
	searchchannel.type = AFK_CHANNEL_SEARCH_SYSTEM_INFO;

	switch(nSystemType) 
	{
	case SYSTEM_INFO_GENERAL:			/* 普通信息 */
	case SYSTEM_INFO_DEV_ATTR:			/* 设备属性信息 */
	case SYSTEM_INFO_DISK_DRIVER:		/* 硬盘信息 */
	case SYSTEM_INFO_FILE_SYSTEM:		/* 文件系统信息 */
	case SYSTEM_INFO_VIDEO_ATTR:		/* 视频属性 */
	case SYSTEM_INFO_CHARACTER_SET:		/* 字符集信息 */
	case SYSTEM_INFO_OPTICS_STORAGE:	/* 光存储设备信息 */
	case SYSTEM_INFO_DEV_ID:
	case SYSTEM_INFO_DEV_VER:
	case SYSTEM_INFO_LOGIN_ATTR:
	case SYSTEM_INFO_TALK_ATTR:
	case SYSTEM_INFO_DEV_TYPE:
	case SYSTEM_INFO_PLATFORM:
	case SYSTEM_INFO_SD_CARD:
	case SYSTEM_INFO_MOTIONDETECT:		/* 设备视频动态检测属性信息 */
	case SYSTEM_INFO_VIDEOBLIND:		/* 视频区域遮挡属性信息 */
	case SYSTEM_INFO_CAMERA:			/* 查询摄像头属性信息(IPC) */
	case SYSTEM_INFO_WATERMARK:			/* 查询图象水印能力 */
	case SYSTEM_INFO_WIRELESS:			/* 查询Wireless能力 */
	case SYSTEM_INFO_Language:			/* 查询支持的语言列表 */
	case SYSTEM_INFO_PICTURE:			/* 是否支持新的录像及图片列表查询方式 */
	case SYSTEM_INFO_DEV_ALL:			/* 设备功能列表 */
	case SYSTEM_INFO_INFRARED:			/* 查询无线报警能力 */
	case SYSTEM_INFO_NEWLOGTYPE:		/* 是否支持新的日志格式 */
	case SYSTEM_INFO_OEM_INFO:			/* OEM */
	case SYSTEM_INFO_NET_STATE:			/* 网络运行状态信息 */
	case SYSTEM_INFO_DEV_SNAP:			/* 设备抓图功能能力查询 */
	case SYSTEM_INFO_VIDEO_CAPTURE:		/* 视频前端采集能力查询 */
	case SYSTEM_INFO_DISK_SUBAREA:		/*网络硬盘分区*/
		searchchannel.subtype = nSystemType;
		break;
	default:
		return NET_ILLEGAL_PARAM;
		break;
	}

    searchchannel.base.func = QuerySystemInfoFunc;

    receivedata_s receivedata;// = {0};
    receivedata.data = (char*)pSysInfoBuffer;
    receivedata.datalen = nSysInfolen;
    receivedata.maxlen = maxlen;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {    
                if (receivedata.result == -1)
                {
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else
                {
                    nRet = receivedata.result;
                }
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}
	
    return nRet;
}

int __stdcall QueryUserInfoFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}

    *receivedata->datalen = datalen - HEADER_SIZE;
    //缓冲区不够大
    if (datalen-HEADER_SIZE > (unsigned int)receivedata->maxlen)
    {
        SetEventEx(receivedata->hRecEvt);
        return -1;
    }
    receivedata->result = (int)param;
    memcpy(receivedata->data, data + HEADER_SIZE, datalen - HEADER_SIZE);
    SetEventEx(receivedata->hRecEvt);
    return 1;
}

int CDevConfig::QueryUserInfoEx(LONG lLoginID, USER_MANAGE_INFO_EX *info, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	afk_device_s *device = (afk_device_s*)lLoginID;

	if (NULL == info || NULL == device)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(info, 0, sizeof(USER_MANAGE_INFO_EX));

	int retlen=0;
	int nResult = -1;
	info->byNameMaxLength=8;//old version
	info->byPSWMaxLength=8;
	info->dwFouctionMask=0;

	//modify by cqs 09.09.10
	bool bSupport = false;
	int nRetLen = 0;
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nResult = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nResult >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_LOGIN_ATTRIBUTE] != 0)
		{
			bSupport = true;
		}
	}	
	if (!bSupport)
	{
		char szVersion[64];
		nResult = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_VER, szVersion, 64, &retlen, waittime);
		if (nResult >= 0 && retlen > 0)
		{
			double fVersion=atof(szVersion);
			if(fVersion >=  (double)2.2 )//greater 2.2 version("2.2x.xxxx.x") 
			{
				info->byNameMaxLength =16;//新版本，支持16位的用户名和密码
				info->byPSWMaxLength=16;
				info->dwFouctionMask=2;
			}
			
		}
	}
	else
	{
		//get login attribute
		LOGIN_ATTRIBUTE stLoginAttr = {0};
		nResult = QuerySystemInfo(lLoginID, SYSTEM_INFO_LOGIN_ATTR, (char *)&stLoginAttr, sizeof(LOGIN_ATTRIBUTE), &retlen, waittime);
		if (nResult >= 0 && retlen == sizeof(LOGIN_ATTRIBUTE))
		{
			BYTE byNameLen = stLoginAttr.iUNameMaxLen;
			if (byNameLen > 8) 
			{
				info->byNameMaxLength =16;//新版本，支持16位的用户名和密码
				info->byPSWMaxLength=16;
				info->dwFouctionMask=2;
			}
		}
		
	}
	
	/////////////////////////////////////////////////////////////////////
	int nRet = -1;
//	afk_device_s *device = (afk_device_s*)lLoginID;
    afk_user_channel_param_s userchannel = {0};
    userchannel.base.func = QueryUserInfoFunc;
	///////////////////////////query right list////////////////////
	{	
		userchannel.type = 1; 
		char rListBuf[80*MAX_RIGHT_NUM] = {0};
		
		int rlBufLen = 0;
		receivedata_s receivedata;// = {0};
		receivedata.data = rListBuf;
		receivedata.datalen = &rlBufLen;
		receivedata.maxlen = 80*MAX_RIGHT_NUM;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;
		if (waittime == 0)
		{
			userchannel.base.udata = 0;
		}
		else
		{
			userchannel.base.udata = &receivedata;
		}
		
		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_USER, &userchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					switch(receivedata.result)
					{
					case -1:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					case 0:
						//nRet = ParseListInfoEx(0, rListBuf, rlBufLen, info->rightList, &info->dwRightNum,info->byNameMaxLength);
						{
							//由于权限改用字符串判断，但是2.40以前的版本和2.40以后的版本字符串中有
							//字符不对应，问了兼容以前的版本，SDK这里对目前的六个不同的字符进行更正
							//"GeneralConf","EncodeConf","RecordConf","ComConf","NetConf","AlarmConf"
							
							std::string str = "GeneralConf::EncodeConf::RecordConf::ComConf::NetConf::AlarmConf";
							CStrParse cpa;
							cpa.setSpliter("::");
							cpa.setTrim(true);
							cpa.Parse(str);
							
							
							std::string strdata = &rListBuf[0];
							char* ptrdata = NULL;
							char strcmpdata[3] = {0};
							int npos = 0;
							
							for (int i = 0; i < cpa.Size(); i++)
							{
								ptrdata = strstr(strdata.c_str(), cpa.getWord(i).c_str());
								if (ptrdata != NULL)
								{
									npos = ptrdata - strdata.c_str() + cpa.getWord(i).size();
									
									//判断后面的两个字符是不是“ig”，不是插入
									strncpy(strcmpdata, ptrdata + cpa.getWord(i).size(), 2);
									if (_stricmp(strcmpdata, "ig") != 0)
									{
										strdata.insert(npos, "ig", 2);
									}
								}
								
							}
							
							int Listlength = strdata.size() + 1;
							char *Listdata = new char[Listlength];
							
							strcpy(Listdata, strdata.c_str());
							nRet = ParseRightItemEx(Listdata,Listlength,info->rightList,&info->dwRightNum,info->byNameMaxLength);
							
							delete []Listdata;
							Listdata = NULL;
						}
					//	nRet=ParseRightItemEx(rListBuf,rlBufLen,info->rightList,&info->dwRightNum,info->byNameMaxLength);
						break;
					case 3:
						nRet = NET_EMPTY_LIST;
						break;
					default:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}
		
		if (nRet < 0)
		{
			return nRet;
		}
	}
	//////////////////////end query right list////////////////////
	//////////////////////query group////////////////////
	{
		userchannel.type = 5; 
		char gpListBuf[400*MAX_GROUP_NUM] = {0};

		int glBufLen = 0;
		receivedata_s receivedata;// = {0};
		receivedata.data = gpListBuf;
		receivedata.datalen = &glBufLen;
		receivedata.maxlen = 400*MAX_GROUP_NUM;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;
		if (waittime == 0)
		{
			userchannel.base.udata = 0;
		}
		else
		{
			userchannel.base.udata = &receivedata;
		}
		
		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_USER, &userchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					switch(receivedata.result)
					{
					case -1:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					case 0:
						nRet = ParseGroupItemEx(gpListBuf, glBufLen, info->groupList, &info->dwGroupNum,info->byNameMaxLength);
						break;
					case 3:
						nRet = NET_EMPTY_LIST;
						break;
					case 6:
						nRet = NET_NOT_AUTHORIZED;
						break;
					default:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}


		if (nRet < 0)
		{
			return nRet;
		}
	}

	//////////////////////end query group////////////////////
	//////////////////////query user///////////////////////  
	{
		userchannel.type = 9; 
		char urListBuf[400*MAX_USER_NUM] = {0};

		int ulBufLen= 0;
		receivedata_s receivedata;// = {0};
		receivedata.data = urListBuf;
		receivedata.datalen = &ulBufLen;
		receivedata.maxlen = 400*MAX_USER_NUM;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;
		if (waittime == 0)
		{
			userchannel.base.udata = 0;
		}
		else
		{
			userchannel.base.udata = &receivedata;
		}
		
		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_USER, &userchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					switch(receivedata.result)
					{
					case -1:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					case 0:
						{
							int nReusable = 0;
							nRet = ParseUserItemEx(urListBuf, ulBufLen, info->userList, &info->dwUserNum,&nReusable,info->byNameMaxLength);
							if (nRet >= 0 && nReusable != 0) 
							{
								DWORD dwSp = SP_USER_REUSABLE;
								device->set_info(device, dit_special_cap_flag, &dwSp);
								info->dwFouctionMask = 1;
							}
						}
						break;
					case 3:
						nRet = NET_EMPTY_LIST;
						break;
					case 6:
						nRet = NET_NOT_AUTHORIZED;
						break;
					default:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}

	}
	
	//////////////////////end query user///////////////////
	
    return nRet;
}

int CDevConfig::QueryUserInfo(LONG lLoginID, USER_MANAGE_INFO *info, int waittime)
{

    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!info)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(info, 0, sizeof(USER_MANAGE_INFO));
	int nRet = -1;
	afk_device_s *device = (afk_device_s*)lLoginID;
    afk_user_channel_param_s userchannel = {0};
    userchannel.base.func = QueryUserInfoFunc;
	///////////////////////////query right list////////////////////
	{	
		userchannel.type = 1; 
		char rListBuf[80*MAX_RIGHT_NUM] = {0};
		
		int rlBufLen = 0;
		receivedata_s receivedata;// = {0};
		receivedata.data = rListBuf;
		receivedata.datalen = &rlBufLen;
		receivedata.maxlen = 80*MAX_RIGHT_NUM;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;
		if (waittime == 0)
		{
			userchannel.base.udata = 0;
		}
		else
		{
			userchannel.base.udata = &receivedata;
		}
		
		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_USER, &userchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					switch(receivedata.result)
					{
					case -1:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					case 0:
						{
							//由于权限改用字符串判断，但是2.40以前的版本和2.40以后的版本字符串中有
							//字符不对应，为了兼容以前的版本，SDK这里对目前的六个不同的字符进行更正
							//"GeneralConf","EncodeConf","RecordConf","ComConf","NetConf","AlarmConf"

							std::string str = "GeneralConf::EncodeConf::RecordConf::ComConf::NetConf::AlarmConf";
							CStrParse cpa;
							cpa.setSpliter("::");
							cpa.setTrim(true);
							cpa.Parse(str);
							
							
							std::string strdata = &rListBuf[0];
							char* ptrdata = NULL;
							char strcmpdata[3] = {0};
							int npos = 0;
							
							for (int i = 0; i < cpa.Size(); i++)
							{
								ptrdata = strstr(strdata.c_str(), cpa.getWord(i).c_str());
								if (ptrdata != NULL)
								{
									npos = ptrdata - strdata.c_str() + cpa.getWord(i).size();

									//判断后面的两个字符是不是“ig”，不是插入
									strncpy(strcmpdata, ptrdata + cpa.getWord(i).size(), 2);
									if (_stricmp(strcmpdata, "ig") != 0)
									{
										strdata.insert(npos, "ig", 2);
									}
								}
								
							}

							int Listlength = strdata.size() + 1;
							char *Listdata = new char[Listlength];

							strcpy(Listdata, strdata.c_str());
					
							nRet = ParseListInfo(0, Listdata, Listlength, info->rightList, &info->dwRightNum);

							delete []Listdata;
							Listdata = NULL;
						//	nRet = ParseListInfo(0, rListBuf, rlBufLen, info->rightList, &info->dwRightNum);
							
						}
						
						break;
					case 3:
						nRet = NET_EMPTY_LIST;
						break;
					default:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}
		
		if (nRet < 0)
		{
			return nRet;
		}	
	}
	//////////////////////end query right list////////////////////
	//////////////////////query group////////////////////
	{
		userchannel.type = 5; 
		char gpListBuf[400*MAX_GROUP_NUM] = {0};

		int glBufLen = 0;
		receivedata_s receivedata;// = {0};
		receivedata.data = gpListBuf;
		receivedata.datalen = &glBufLen;
		receivedata.maxlen = 400*MAX_GROUP_NUM;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;
		if (waittime == 0)
		{
			userchannel.base.udata = 0;
		}
		else
		{
			userchannel.base.udata = &receivedata;
		}
		
		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_USER, &userchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					switch(receivedata.result)
					{
					case -1:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					case 0:
						nRet = ParseListInfo(1, gpListBuf, glBufLen, info->groupList, &info->dwGroupNum);
						break;
					case 3:
						nRet = NET_EMPTY_LIST;
						break;
					case 6:
						nRet = NET_NOT_AUTHORIZED;
						break;
					default:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}


		if (nRet < 0)
		{
			return nRet;
		}
	}

	//////////////////////end query group////////////////////
	//////////////////////query user///////////////////////  
	{
		userchannel.type = 9; 
		char urListBuf[400*MAX_USER_NUM] = {0};

		int ulBufLen= 0;
		receivedata_s receivedata;// = {0};
		receivedata.data = urListBuf;
		receivedata.datalen = &ulBufLen;
		receivedata.maxlen = 400*MAX_USER_NUM;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;
		if (waittime == 0)
		{
			userchannel.base.udata = 0;
		}
		else
		{
			userchannel.base.udata = &receivedata;
		}
		
		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_USER, &userchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					switch(receivedata.result)
					{
					case -1:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					case 0:
						{
							int nReusable = 0;
							nRet = ParseListInfo(2, urListBuf, ulBufLen, info->userList, &info->dwUserNum, &nReusable);
							if (nRet >= 0 && nReusable != 0) 
							{
								DWORD dwSp = SP_USER_REUSABLE;
								device->set_info(device, dit_special_cap_flag, &dwSp);
								info->dwSpecial = 1;
							}
						}
						break;
					case 3:
						nRet = NET_EMPTY_LIST;
						break;
					case 6:
						nRet = NET_NOT_AUTHORIZED;
						break;
					default:
						nRet = NET_RETURN_DATA_ERROR;
						break;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}

	}
	
	//////////////////////end query user///////////////////
	
    return nRet;
}

int __stdcall OperateUserInfoFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

    /*if (param != 0)
    {
        SetEvent(receivedata->hRecEvt);
        return -1;
    }*/
    receivedata->result = (int)param;
//    memcpy(receivedata->data, data, datalen);
    SetEventEx(receivedata->hRecEvt);
    return 1;
}

int GetOperateResult(int nOperateType, int nReceiveResult)
{
	int nRet = -1;
	switch(nOperateType)
	{
		case 0:		//	2 增加用户组
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 2)
				{
					nRet = NET_GROUP_EXIST;
				}
				else if(nReceiveResult == 3)
				{
					nRet = NET_GROUP_RIGHTOVER;
				}
				else if(nReceiveResult == 11)
				{
					nRet = NET_GROUP_OVERSUPPORTNUM;
				}
			}
			break;
		case 1:		//	3 删除用户组
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 4)
				{
					nRet = NET_GROUP_NOEXIST;
				}
				else if(nReceiveResult == 5)
				{
					nRet = NET_GROUP_HAVEUSER;
				}
				else if (nReceiveResult == 6)
				{
					nRet = NET_NOT_AUTHORIZED;
				}
			}
			break;
		case 2:		//	4 修改用户组
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 4)
				{
					nRet = NET_GROUP_NOEXIST;
				}
				else if(nReceiveResult == 5)
				{
					nRet = NET_GROUP_RIGHTUSE;
				}
				else if(nReceiveResult == 2)
				{
					nRet = NET_GROUP_SAMENAME;
				}
				else if (nReceiveResult == 6)
				{
					nRet = NET_NOT_AUTHORIZED;
				}
			}
			break;
		case 3:		//	6 增加用户
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 4)
				{
					nRet = NET_GROUP_NOEXIST;
				}
				else if(nReceiveResult == 2)
				{
					nRet = NET_USER_EXIST;
				}
				else if(nReceiveResult == 6)
				{
					nRet = NET_USER_RIGHTOVER;
				}
				else if(nReceiveResult == 12)
				{
					nRet = NET_USER_OVERSUPPORTNUM;
				}
			}
			break;
		case 4:		//	7 删除用户
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 4)
				{
					nRet = NET_USER_NOEXIST;
				}
				else if (nReceiveResult == 6)
				{
					nRet = NET_NOT_AUTHORIZED;
				}
			}
			break;
		case 5:		//	8 修改用户
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 4)
				{
					nRet = NET_USER_NOEXIST;
				}
				else if(nReceiveResult == 2)
				{
					nRet = NET_USER_EXIST;
				}
				else if(nReceiveResult == 6)
				{
					nRet = NET_USER_RIGHTOVER;
				}
			}
			break;
		case 6:		//	10 修改用户密码
			{
				if (nReceiveResult == -1)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else if(nReceiveResult == 0)
				{
					nRet = 0;
				}
				else if(nReceiveResult == 1)
				{
					nRet = NET_ILLEGAL_PARAM;
				}
				else if(nReceiveResult == 10)
				{
					nRet = NET_USER_PWD;
				}
				else if(nReceiveResult == 8)
				{
					nRet = NET_USER_FLASEPWD;
				}
				else if(nReceiveResult == 9)
				{
					nRet = NET_USER_NOMATCHING;
				}
				else if (nReceiveResult == 6)
				{
					nRet = NET_NOT_AUTHORIZED;
				}
				else if (nReceiveResult == 11)
				{
					nRet = NET_USER_FLASEPWD_TRYTIME;
				}
			}
			break;
		default:
			nRet = NET_ILLEGAL_PARAM;
			break;;
	}
	
	return nRet;
}

int CDevConfig::OperateUserInfoEx(LONG lLoginID, int nOperateType, void *opParam, void *subParam, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nRet = -1;
	char buf[400] = {0};
	int buflen = 0;
	afk_user_channel_param_s userchannel = {0};

    afk_device_s *device = (afk_device_s*)lLoginID;

	DWORD dwSp = 0;
	device->get_info(device, dit_special_cap_flag, &dwSp);
	int nReusable = ((dwSp&SP_USER_REUSABLE) != 0) ? 1 : 0;

	int nMaxLength=8;//旧版本
	int retlen = 0;

	//modify by cqs 09.09.10
	bool bSupport = false;
	int nRetLen = 0;
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_LOGIN_ATTRIBUTE] != 0)
		{
			bSupport = true;
		}
	}	
	if (!bSupport)
	{
		int nResult = -1;
		char szVersion[64];
		nResult = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_VER, szVersion, 64, &retlen, waittime);
		if (nResult >= 0 && retlen > 0)
		{
			double fVersion = atof(szVersion);
			if(fVersion >= (double)2.2)//greater 2.2 version(2.20.0000.0)
			{
				nMaxLength =16;//新版本，支持16位的用户名和密码
			}
		}
	}
	else
	{
		int nRet = -1;
		//get login attribute
		LOGIN_ATTRIBUTE stLoginAttr = {0};
		nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_LOGIN_ATTR, (char *)&stLoginAttr, sizeof(LOGIN_ATTRIBUTE), &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(LOGIN_ATTRIBUTE))
		{
			BYTE byNameLen = stLoginAttr.iUNameMaxLen;
			nMaxLength = byNameLen>8?16:8;
		}
		
	}

	int ret = BulidUserInfoBufEx(nOperateType, opParam, subParam, buf, &buflen , 400, nMaxLength,(void*)nReusable);
	if (ret < 0)
	{
		return NET_ILLEGAL_PARAM;
	}	
	switch(nOperateType)
	{
	case 0:		//	2 增加用户组
		userchannel.type = 2;
		break;
	case 1:		//	3 删除用户组
		userchannel.type = 3;
		break;
	case 2:		//	4 修改用户组
		userchannel.type = 4;
		break;
	case 3:		//	6 增加用户
		//reusable? in case that user didn't query a user info
		{
			USER_MANAGE_INFO_EX tmpInfo = {0};
			QueryUserInfoEx(lLoginID, &tmpInfo, waittime);
		}
		userchannel.type = 6;
		break;
	case 4:		//	7 删除用户
		userchannel.type = 7;
		break;
	case 5:		//	8 修改用户
		userchannel.type = 8;
		//reusable? in case that user didn't query a user info
		{
			USER_MANAGE_INFO_EX tmpInfo = {0};
			QueryUserInfoEx(lLoginID, &tmpInfo, waittime);
		}
		break;
	case 6:		//	10 修改用户密码
		userchannel.type = 10;
		break;
	default:
		return NET_ILLEGAL_PARAM;
	}
    userchannel.base.func = OperateUserInfoFunc;
    userchannel.userbuf = buf;
    userchannel.buflen = buflen;
    receivedata_s receivedata;// = {0};
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        userchannel.base.udata = 0;
    }
    else
    {
        userchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_USER, &userchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {
				nRet = GetOperateResult(nOperateType, receivedata.result);
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int CDevConfig::OperateUserInfo(LONG lLoginID, int nOperateType, void *opParam, void *subParam, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	int nRet = -1;
	char buf[400] = {0};
	int buflen = 0;
	afk_user_channel_param_s userchannel = {0};

    afk_device_s *device = (afk_device_s*)lLoginID;
	DWORD dwSp = 0;
	device->get_info(device, dit_special_cap_flag, &dwSp);
	int nReusable = ((dwSp&SP_USER_REUSABLE) != 0) ? 1 : 0;
	int ret = BulidUserInfoBuf(nOperateType, opParam, subParam, buf, &buflen , 400, (void*)nReusable);
	if (ret < 0)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	switch(nOperateType)
	{
	case 0:		//	2 增加用户组
		userchannel.type = 2;
		break;
	case 1:		//	3 删除用户组
		userchannel.type = 3;
		break;
	case 2:		//	4 修改用户组
		userchannel.type = 4;
		break;
	case 3:		//	6 增加用户
		//reusable? in case that user didn't query a user info
		{
			USER_MANAGE_INFO tmpInfo = {0};
			QueryUserInfo(lLoginID, &tmpInfo, waittime);
		}
		userchannel.type = 6;
		break;
	case 4:		//	7 删除用户
		userchannel.type = 7;
		break;
	case 5:		//	8 修改用户
		userchannel.type = 8;
		//reusable? in case that user didn't query a user info
		{
			USER_MANAGE_INFO tmpInfo = {0};
			QueryUserInfo(lLoginID, &tmpInfo, waittime);
		}
		break;
	case 6:		//	10 修改用户密码
		userchannel.type = 10;
		break;
	default:
		return NET_ILLEGAL_PARAM;
	}

    userchannel.base.func = OperateUserInfoFunc;
    userchannel.userbuf = buf;
    userchannel.buflen = buflen;

    receivedata_s receivedata;// = {0};
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        userchannel.base.udata = 0;
    }
    else
    {
        userchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_USER, &userchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {
				nRet = GetOperateResult(nOperateType, receivedata.result);
            }
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}


int __stdcall QueryIOControlStateFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	if (NULL == data)
	{
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}

	if (0 == receivedata->data)		//用户缓冲为空，说明用户在查询所需缓冲空间
	{
		if (receivedata->datalen)	//有效指针
		{
			if((unsigned char)param == 0)//报警输出，输入。
			{
				afk_alarm_info_s *alarminfo = (afk_alarm_info_s*)data;
				*receivedata->datalen = alarminfo->count;
				receivedata->result = 0;
				receivedata->reserved[0] = (alarminfo->bSupportTriggerMode == true) ? 1 : 0;
				SetEventEx(receivedata->hRecEvt);
				return 1;
			}
			else if((unsigned char)param == 1)//报警输出触发方式
			{
				afk_alarm_trriger_mode_s *pTrrigerMode = (afk_alarm_trriger_mode_s *)data;
				*receivedata->datalen = pTrrigerMode->channel_count;
				receivedata->result = 0;
				SetEventEx(receivedata->hRecEvt);
				return 1;
			}
		}
		else		//无效指针
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
	}
	else
	{
		if((unsigned char)param == 0)//报警布防
		{
			afk_alarm_info_s *alarminfo = (afk_alarm_info_s*)data;
			
			if (alarminfo->count > (unsigned int)receivedata->maxlen)
			{
				SetEventEx(receivedata->hRecEvt);
				return -1;
			}
			else
			{
				receivedata->result = 0;
				*(receivedata->datalen) = alarminfo->count;
				receivedata->reserved[0] = (alarminfo->bSupportTriggerMode == true) ? 1 : 0;
				memcpy(receivedata->data, alarminfo->state, alarminfo->count);
				SetEventEx(receivedata->hRecEvt);
				return 1;
			}
		}
		else if((unsigned char)param == 1)//报警输出触发方式
		{
			afk_alarm_trriger_mode_s *pTrrigerMode = (afk_alarm_trriger_mode_s *)data;

			if(pTrrigerMode->channel_count > (unsigned int)receivedata->maxlen)
			{
				SetEventEx(receivedata->hRecEvt);
				return -1;
			}
			else
			{
				receivedata->result = 0;
				*(receivedata->datalen) = pTrrigerMode->channel_count;
				memcpy(receivedata->data, pTrrigerMode->trriger_mode, pTrrigerMode->channel_count);
				SetEventEx(receivedata->hRecEvt);
				return 1;
			}
		}
	}

    return 1;
}

int CDevConfig::QueryIOControlState(LONG lLoginID, IO_CTRL_TYPE emType, 
                                           void *pState, int maxlen, int *nIOCount, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	if (!pState && !nIOCount)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;
    afk_search_channel_param_s searchchannel = {0};
    receivedata_s receivedata;
	int ncnt = 0;
	unsigned char * pstatebuf = 0;

	switch(emType)
	{
	case IOTYPE_ALARMINPUT:	//查询报警输入是否已经打开
		searchchannel.type = AFK_CHANNEL_SEARCH_ALARMCTRL_IN;
		ncnt = maxlen/sizeof(ALARM_CONTROL);
		if (pState) 
		{
			pstatebuf = new unsigned char[ncnt];
			if (!pstatebuf)
			{
				return NET_SYSTEM_ERROR;
			}
		}

		receivedata.data = (char*)pstatebuf;
		receivedata.maxlen = ncnt;
		break;
	case IOTYPE_ALARMOUTPUT:	//查询报警输出是否已经打开
		searchchannel.type = AFK_CHANNEL_SEARCH_ALARMCTRL_OUT;
		ncnt = maxlen/sizeof(ALARM_CONTROL);
		if (pState)
		{
			pstatebuf = new unsigned char[ncnt];
			if (!pstatebuf)
			{
				return NET_SYSTEM_ERROR;
			}
		}
		
		receivedata.data = (char*)pstatebuf;
		receivedata.maxlen = ncnt;
		break;

	case IOTYPE_ALARM_TRIGGER_MODE:	//查询报警触发方式
		{
			int nSupport = 0;
			int nRetLen = 0;
			GetDevFunctionInfo(lLoginID, ABILITY_TRIGGER_MODE, (char *)&nSupport, sizeof(int), &nRetLen);
			if(nSupport <= 0)
			{
				return -1;
			}

			ncnt = maxlen/sizeof(TRIGGER_MODE_CONTROL);
			if (pState)
			{
				pstatebuf = new unsigned char[ncnt];
				if (!pstatebuf)
				{
					return NET_SYSTEM_ERROR;
				}
			}

			searchchannel.type = AFK_CHANNEL_SEARCH_ALARMTRRIGER_MODE;
			receivedata.data = (char*)pstatebuf;
			receivedata.maxlen = ncnt;
		}
		break;

//	case IOTYPE_WIRELESS_ALARMOUT: //查询无线报警端口是否已经打开
//		{
//			searchchannel.type = AFK_CHANNEL_SEARCH_WIRELESS_ALARMOUT;
//			ncnt = maxlen/sizeof(ALARM_CONTROL);
//			if(pState)
//			{
//				pstatebuf = new unsigned char[ncnt];
//				if(!pstatebuf)
//				{
//					return NET_SYSTEM_ERROR;
//				}
//			}
//			receivedata.data = (char *)pstatebuf;
//			receivedata.maxlen = ncnt;
//		}
//		
//		break;
	default:
		{
			return NET_ILLEGAL_PARAM;
		}
		break;
	}

    searchchannel.base.func = QueryIOControlStateFunc;

    receivedata.datalen = nIOCount;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
            pchannel->close(pchannel);
            ResetEventEx(receivedata.hRecEvt);
            if (dwRet == WAIT_OBJECT_0)
            {
                if (receivedata.result == -1)
                {
					if (pstatebuf)
					{
						delete[] pstatebuf;
						pstatebuf = 0;
					}
                    nRet = NET_RETURN_DATA_ERROR;
                }
                else
                {
					if (pState && pstatebuf)
					{ 
						if(emType == IOTYPE_ALARM_TRIGGER_MODE)//报警输出触发方式
						{
							for(int i=0; i < ncnt; ++i)
							{
								((TRIGGER_MODE_CONTROL*)pState)[i].index = i;
								switch(pstatebuf[i]) //（0-手动；1-自动；2-关闭）->（0关闭1手动2自动）
								{
								case 0:
									((TRIGGER_MODE_CONTROL*)pState)[i].mode = 1;
									break;
								case 1:
									((TRIGGER_MODE_CONTROL*)pState)[i].mode = 2;
									break;
								case 2:
									((TRIGGER_MODE_CONTROL*)pState)[i].mode = 0;
									break;
								default:
									((TRIGGER_MODE_CONTROL*)pState)[i].mode = 2;
									break;
								}
								
							}
						}
						else
						{
							for(int i=0; i < ncnt; ++i)
							{
								((ALARM_CONTROL*)pState)[i].index = i;
								((ALARM_CONTROL*)pState)[i].state = pstatebuf[i];
							}
						}
					}

					if (pstatebuf)
					{
						delete[] pstatebuf;
						pstatebuf = 0;
					}

					if(NULL == pState)
					{
						return receivedata.reserved[0];
					}
					
                    nRet = 0;
                }
            }//if (dwRet == WAIT_OBJECT_0)
			else
			{
				nRet = NET_NETWORK_ERROR;
			}
        }//if (waittime != 0)
        else
        {
			if (pstatebuf)
			{
				delete[] pstatebuf;
				pstatebuf = 0;
			}
            nRet = 0;
        }
    }//if (pchannel)
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

	if (pstatebuf)
	{
		delete[] pstatebuf;
		pstatebuf = 0;
	}
    return nRet;
}

int CDevConfig::IOControl(LONG lLoginID, IO_CTRL_TYPE emType, void *pState, int maxlen, int waittime)
{
    if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if ((0 == pState) || maxlen <= 0)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = 0;

	int ncnt = 0;

    afk_device_s *device = (afk_device_s*)lLoginID;

	switch(emType)
	{
	case IOTYPE_ALARMINPUT:	//报警输入
	case IOTYPE_ALARMOUTPUT:	//报警输出
	case IOTYPE_WIRELESS_ALARMOUT://无线报警输出 
	case IOTYPE_POSTKT_MODE:	//串口裸数据发送或停止
		{
			ncnt = maxlen/sizeof(ALARM_CONTROL);
			
			for(int i=0; i < ncnt; ++i)
			{
				afk_config_channel_param_s configchannel;
				configchannel.base.func = 0;
				configchannel.base.udata = 0;
				configchannel.type = AFK_CHANNEL_CONFIG_ALARMCTRL;
				configchannel.alarminfo.type = emType; //(emType == IOTYPE_ALARMINPUT) ? true : false;
				configchannel.alarminfo.alarmid = ((ALARM_CONTROL*)pState)[i].index;
				configchannel.alarminfo.controlflag = (((ALARM_CONTROL*)pState)[i].state > 0) ? true : false;

				afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
					AFK_CHANNEL_TYPE_CONFIG, &configchannel);
				if (pchannel)
				{
					pchannel->close(pchannel);
				}
				else
				{
					nRet = NET_OPEN_CHANNEL_ERROR;
				}
			}
		}
		break;
	case IOTYPE_DECODER_ALARMOUT:
		{
			ncnt = maxlen/sizeof(DECODER_ALARM_CONTROL);
			
			for(int i=0; i < ncnt; ++i)
			{
				afk_config_channel_param_s configchannel;
				
				configchannel.base.func = 0;
				configchannel.base.udata = 0;
				configchannel.type = AFK_CHANNEL_CONFIG_ALARMDCD_OUT;
				configchannel.decoderalarm.decoderNo = ((DECODER_ALARM_CONTROL*)pState)[i].decoderNo;
				configchannel.decoderalarm.alarmChn = ((DECODER_ALARM_CONTROL*)pState)[i].alarmChn;
				configchannel.decoderalarm.alarmState = ((DECODER_ALARM_CONTROL*)pState)[i].alarmState;
				
				afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
					AFK_CHANNEL_TYPE_CONFIG, &configchannel);
				if (pchannel)
				{
					pchannel->close(pchannel);
				}
				else
				{
					nRet = NET_OPEN_CHANNEL_ERROR;
				}
			}
		}
		break;
	case IOTYPE_ALARM_TRIGGER_MODE://报警触发方式
		{
			int nSupport = 0;
			int nRetLen = 0;
			GetDevFunctionInfo(lLoginID, ABILITY_TRIGGER_MODE, (char *)&nSupport, sizeof(int), &nRetLen);
			if(nSupport <= 0)
			{
				return -1;
			}

			ncnt = maxlen/sizeof(TRIGGER_MODE_CONTROL);
			
			TRIGGER_MODE_CONTROL *pTrrigerMode = (TRIGGER_MODE_CONTROL *)pState;
			afk_config_channel_param_s configchannel;
			configchannel.base.func = 0;
			configchannel.base.udata = 0;
			configchannel.type = AFK_CHANNEL_CONFIG_ALARMTRRIGER_MODE;
			configchannel.alarmtrrigermode.channel_count = device->alarmoutputcount(device);

			TRIGGER_MODE_CONTROL stuTrrigerMode[16] = {0};
			int nCount = 0;
			int nRet = QueryIOControlState(lLoginID, emType, (void *)stuTrrigerMode, sizeof(TRIGGER_MODE_CONTROL)*16, &nCount, waittime);
			if(nRet < 0)
			{
				return -1;
			}
			//将原来的值复制回去
			int i = 0;
			for(i=0; i < 16; ++i)
			{	
				configchannel.alarmtrrigermode.trriger_mode[i].mode = (unsigned char)stuTrrigerMode[i].mode;
			}
			//将新的值放在相应的位置上
			for(i=0; i < ncnt; ++i)
			{	
				configchannel.alarmtrrigermode.trriger_mode[pTrrigerMode[i].index].mode = (unsigned char)pTrrigerMode[i].mode;
			} 

			afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
				AFK_CHANNEL_TYPE_CONFIG, &configchannel);
			if (pchannel)
			{
				pchannel->close(pchannel);
			}
			else
			{
				nRet = NET_OPEN_CHANNEL_ERROR;
			}
		}
		break;
	default:
		nRet = NET_ILLEGAL_PARAM;
		break;
	}

    return nRet;
}


int __stdcall QueryDEVWorkStateFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }

	LPNET_DEV_WORKSTATE lpState = (LPNET_DEV_WORKSTATE)receivedata->data;
	if (!lpState)
	{
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}

	if (0 == data)
	{
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}

	if (0 == data[8])	//设备状态
	{
		lpState->dwDeviceStatic = data[HEADER_SIZE];
		int nalarmin = data[HEADER_SIZE+1];
		int nalarmout = data[HEADER_SIZE+2];

		memcpy(lpState->byAlarmInStatic, data+HEADER_SIZE+3, nalarmin);
		memcpy(lpState->byAlarmOutStatic, data+HEADER_SIZE+3+nalarmin, nalarmout);
		lpState->dwLocalDisplay = data[HEADER_SIZE+3+nalarmin+nalarmout];

		receivedata->result = 0;
		SetEventEx(receivedata->hRecEvt);
		return 1;
	}
	else if (1 == data[8])	//设备通道状态查询
	{
		if (0xFF != data[9])
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}
		
		int nChnCnt = data[20];
		int nDspCnt = data[21];

		if (nChnCnt != nDspCnt)
		{
			SetEventEx(receivedata->hRecEvt);
			return -1;
		}

		for(int i=0; i < nChnCnt; ++i)
		{
			lpState->stChanStatic[i].bySignalStatic = data[HEADER_SIZE+i];
			lpState->stChanStatic[i].byHardwareStatic = data[HEADER_SIZE+nChnCnt+i];
		}

		receivedata->result = 0;
		SetEventEx(receivedata->hRecEvt);
		return 1;
	}
	else
	{
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}
	
    return 1;
}

int CDevConfig::GetDEVWorkState(LONG lLoginID, LPNET_DEV_WORKSTATE lpWorkState, int waittime)
{
	if (!lpWorkState)
	{
		return NET_ILLEGAL_PARAM;
	}

	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	int nRet = -1;
	{	
		//取设备状态信息
		memset(lpWorkState, 0x00, sizeof(NET_DEV_WORKSTATE));

		afk_device_s *device = (afk_device_s*)lLoginID;

		afk_search_channel_param_s searchchannel = {0};
		receivedata_s receivedata;// = {0};

		searchchannel.no = 0;
		searchchannel.type = AFK_CHENNEL_SEARCH_DEVWORKSTATE;
		searchchannel.querydevstate = 0; //设备状态
		searchchannel.base.func = QueryDEVWorkStateFunc;

		receivedata.data = (char*)lpWorkState;
		receivedata.maxlen = sizeof(NET_DEV_WORKSTATE);
		receivedata.datalen = 0;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;

		searchchannel.base.udata = &receivedata;

		if (waittime == 0)
		{
			searchchannel.base.udata = 0;
		}
		else
		{
			searchchannel.base.udata = &receivedata;
		}

		afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					if (receivedata.result == -1)
					{
						nRet = NET_RETURN_DATA_ERROR;
					}
					else
					{
						nRet = 0;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}
			
		//取通道状态信息
		if (nRet < 0)
		{
			return nRet;
		}

		nRet = -1;

		searchchannel.no = 0xFF;
		searchchannel.type = AFK_CHENNEL_SEARCH_DEVWORKSTATE;
		searchchannel.querydevstate = 1; //通道状态

		receivedata.data = (char*)lpWorkState;
		receivedata.maxlen = sizeof(NET_DEV_WORKSTATE);
		receivedata.datalen = 0;
		//receivedata.hRecEvt = m_hRecEvent;
		receivedata.result = -1;

		pchannel = (afk_channel_s*)device->open_channel(device, 
			AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
		if (pchannel)
		{
			if (waittime != 0)
			{
				DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
				pchannel->close(pchannel);
				ResetEventEx(receivedata.hRecEvt);
				if (dwRet == WAIT_OBJECT_0)
				{
					if (receivedata.result == -1)
					{
						nRet = NET_RETURN_DATA_ERROR;
					}
					else
					{
						nRet = 0;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;
				}
			}
			else
			{
				nRet = 0;
			}
		}
		else
		{
			nRet = NET_OPEN_CHANNEL_ERROR;
		}

		if (nRet < 0)
		{
			memset(lpWorkState, 0x00, sizeof(NET_DEV_WORKSTATE));
			return nRet;
		}

		//取录象状态信息
		BYTE byteRecordState[MAX_CHANNUM];
		int nReturn = 0;
		nRet = QueryRecordState(lLoginID, (char*)byteRecordState, MAX_CHANNUM, &nReturn,waittime);
		
		if (nRet < 0)
		{
			memset(lpWorkState, 0x00, sizeof(NET_DEV_WORKSTATE));
			return nRet;
		}
		
		for(int i=0; i < MAX_CHANNUM; ++i)
		{
			lpWorkState->stChanStatic[i].byRecordStatic = byteRecordState[i];
		}

		//取硬盘状态信息
		
		nRet = -1;
		int nLen;
		int bufSize = sizeof(IDE_INFO64)+sizeof(DRIVER_INFO)*32;
		//char *pBuf = new char[bufSize];

		char pBuf[sizeof(IDE_INFO64)+sizeof(DRIVER_INFO)*32] = {0};

		nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DISK_DRIVER, pBuf, bufSize, &nLen, waittime);
		if (nRet >=0)
		{
			if (nLen >= sizeof(IDE_INFO) && nLen-sizeof(IDE_INFO)==((IDE_INFO *)pBuf)->ide_num*sizeof(DRIVER_INFO))
			{
				IDE_INFO *ideInfo= (IDE_INFO *)pBuf;
				
				int pos = sizeof(IDE_INFO); //标志当前内存位置
				nLen -= sizeof(IDE_INFO);
				if (ideInfo->ide_num > 32)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else
				{
					for(int index = 0; index < ideInfo->ide_num; index ++)
					{
						if (nLen/sizeof(DRIVER_INFO) <= 0)
						{
							break;
						}
						DRIVER_INFO *dvInfo = (DRIVER_INFO *)(pBuf + pos);
						
						lpWorkState->stHardDiskStatic[index].dwVolume = dvInfo->total_space;
						lpWorkState->stHardDiskStatic[index].dwFreeSpace = dvInfo->remain_space;
						lpWorkState->stHardDiskStatic[index].dwStatus = dvInfo->is_current;

						pos += sizeof(DRIVER_INFO);
						nLen -= sizeof(DRIVER_INFO);
					}
				}
			}
			else if (nLen >= sizeof(IDE_INFO64) && nLen-sizeof(IDE_INFO64)==((IDE_INFO64 *)pBuf)->ide_num*sizeof(DRIVER_INFO))
			{
				IDE_INFO64 *ideInfo= (IDE_INFO64 *)pBuf;
				
				int pos = sizeof(IDE_INFO64); //标志当前内存位置
				nLen -= sizeof(IDE_INFO64);
				if (ideInfo->ide_num > 32)
				{
					nRet = NET_RETURN_DATA_ERROR;
				}
				else
				{
					for(int index = 0; index < ideInfo->ide_num; index ++)
					{
						if (nLen/sizeof(DRIVER_INFO) <= 0)
						{
							break;
						}
						DRIVER_INFO *dvInfo = (DRIVER_INFO *)(pBuf + pos);
						
						lpWorkState->stHardDiskStatic[index].dwVolume = dvInfo->total_space;
						lpWorkState->stHardDiskStatic[index].dwFreeSpace = dvInfo->remain_space;
						lpWorkState->stHardDiskStatic[index].dwStatus = dvInfo->is_current;
						
						pos += sizeof(DRIVER_INFO);
						nLen -= sizeof(DRIVER_INFO);
					}
				}
			}
			else
			{
				nRet = NET_RETURN_DATA_ERROR;
			}
		}
		
		if (nRet < 0)
		{
			memset(lpWorkState, 0, sizeof(NET_DEV_WORKSTATE));
		}
	}

	return nRet;
}

//Add: by zsc(11402) 2008-4-8
int	CDevConfig::SetDevConfig_WLANCfg(LONG lLoginID, DEV_WLAN_INFO *pWlanInfo, int waittime)
{
	if (NULL == pWlanInfo)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRetLen = 0;
	int nRet = -1;
	bool bSupport = false;
	Wireless_Enable_T stuEnable = {0};

	//查看能力
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WIRELESS, (char*)&stuEnable, sizeof(Wireless_Enable_T), &nRetLen, waittime);
	if (nRet ==0 && nRetLen == sizeof(Wireless_Enable_T))
	{
		if(1 == stuEnable.isWirelessEnable)
		{
			bSupport = true;
		}
	}

	if (!bSupport)
	{
		return -1;
	}

	CONFIG_WLAN_INFO CfgWlanInfo = {0};
	CfgWlanInfo.nEnable = pWlanInfo->nEnable;
	CfgWlanInfo.nEncryption = pWlanInfo->nEncryption;
	CfgWlanInfo.nKeyFlag = pWlanInfo->nKeyFlag;
	CfgWlanInfo.nKeyID = pWlanInfo->nKeyID;
	CfgWlanInfo.nKeyType = pWlanInfo->nKeyType;
	CfgWlanInfo.nLinkMode = pWlanInfo->nLinkMode;
	strcpy(CfgWlanInfo.szSSID, pWlanInfo->szSSID);
	if(CfgWlanInfo.nEncryption != 4 && CfgWlanInfo.nEncryption != 5)
	{
		sprintf(CfgWlanInfo.szKeys, "%s&&%s&&%s&&%s", pWlanInfo->szKeys[0], pWlanInfo->szKeys[1], pWlanInfo->szKeys[2], pWlanInfo->szKeys[3]);
	}
	else
	{
		memcpy(CfgWlanInfo.szKeys, pWlanInfo->szWPAKeys, 128);
	}
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_WLAN, 0, (char *)&CfgWlanInfo, sizeof(CONFIG_WLAN_INFO), waittime);
	if (nRet < 0)
	{
		nRet = NET_ERROR_SETCFG_WLAN;
	}

	return nRet;
}

int	CDevConfig::SetDevConfig_WLANDevCfg(LONG lLoginID, DEV_WLAN_DEVICE *pWlanDevInfo, int waittime)
{
	if (NULL == pWlanDevInfo)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRetLen = 0;
	int nRet = -1;
	bool bSupport = false;
	Wireless_Enable_T stuEnable = {0};

	//查看能力
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WIRELESS, (char*)&stuEnable, sizeof(Wireless_Enable_T), &nRetLen, waittime);
	if (nRet ==0 && nRetLen == sizeof(Wireless_Enable_T))
	{
		if(1 == stuEnable.isWirelessEnable)
		{
			bSupport = true;
		}
	}

	if (!bSupport)
	{
		return -1;
	}

	CONFIG_WLAN_DEVICE WlanDev = {0};
	strcpy(WlanDev.szSSID, pWlanDevInfo->szSSID);
	WlanDev.nLinkMode = pWlanDevInfo->nLinkMode;
	WlanDev.nEncryption = pWlanDevInfo->nEncryption;
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_WLAN_DEVICE, 0, (char*)&WlanDev, sizeof(CONFIG_WLAN_DEVICE), waittime);
	if (nRet < 0)
	{
		nRet = NET_ERROR_SETCFG_WLANDEV;
	}

	return nRet;
}

int	CDevConfig::SetDevConfig_AutoRegisterCfg(LONG lLoginID, DEV_REGISTER_SERVER *pRegiSevInfo, int waittime)
{
	if (pRegiSevInfo == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	int nRetLen = 0;
	int nBufPos = 0;
	char buf[1024] = {0};
	bool bSupport = false;
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_AUTO_REGISTER] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	//	将结构体串成字符串
	DEV_REGISTER_SERVER temp = {0};
	if(GetDevConfig_AutoRegisterCfg(lLoginID,&temp, waittime) < 0)
	{
		return -1;
	}
	BYTE bCount = temp.bServerNum;

	if(bCount<0 || bCount>MAX_REGISTER_SERVER_NUM)
	{
		return -1;
	}
	for (int i = 0; i < bCount; i++)
	{
		sprintf(buf+nBufPos, "%s::%d&&", pRegiSevInfo->lstServer[i].szServerIp, pRegiSevInfo->lstServer[i].nServerPort);
		nBufPos = strlen(buf);
	}
	if (bCount > 0)
	{
		buf[nBufPos-1] = '\0';
		buf[nBufPos-2] = '\0';
		nBufPos -= 2;
		sprintf(buf+nBufPos,"||%d|%s",pRegiSevInfo->bEnable,pRegiSevInfo->szDeviceID);
		nBufPos = strlen(buf);
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_AUTO_REGISTER, 0, buf, nBufPos, waittime);
		if (nRet < 0)
		{
			nRet = NET_ERROR_SETCFG_REGISTER;
		}
	}
	
	return nRet;
}

int	CDevConfig::SetDevConfig_ChnCmrCfg(LONG lLoginID, DEVICE_CAMERA_CFG *pstCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (lChannel < -1 || lChannel >= MAX_CHANNUM ||
		(lChannel != -1 && lChannel >= device->channelcount(device)))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	if (NULL == pstCHC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int iChanNum = device->channelcount(device);
	iChanNum = Min(iChanNum,MAX_VIDEO_IN_NUM);
	iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM);
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iChanNum;
	}
	
	DEVICE_CAMERA_CFG *const pDevChaCfg = pstCHC;
	DEVICE_CAMERA_CFG *pTmpChaCfg = pDevChaCfg;
	int buflen = 0;
	char *vbuf = NULL;
	if (buflen < CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAMERA_T))
	{
		buflen = CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAMERA_T);
	}

	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_SYSTEM_ERROR;
	}
	
	memset(vbuf, 0, buflen);
	pTmpChaCfg = pDevChaCfg;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAMERA, 0, vbuf, CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAMERA_T), &retlen, waittime);
	if (nRet >= 0 && retlen > 0 && 0 == (retlen % sizeof(CONFIG_CAMERA_T)))
	{
		CONFIG_CAMERA_T *pCameraInfo = (CONFIG_CAMERA_T *)(vbuf+iChanIdx*sizeof(CONFIG_CAMERA_T));

		int nCount = retlen / sizeof(CONFIG_CAMERA_T);
		nCount = nCount > iChanCount ? iChanCount : nCount;
		
		for (int i = 0; i < nCount; i++)
		{
			pCameraInfo->bExposure = pTmpChaCfg->bExposure;
			pCameraInfo->bBacklight = pTmpChaCfg->bBacklight;
			pCameraInfo->bAutoColor2BW = pTmpChaCfg->bAutoColor2BW;
			pCameraInfo->bMirror = pTmpChaCfg->bMirror;
			pCameraInfo->bFlip = pTmpChaCfg->bFlip;
			pCameraInfo->bLensFunction = ((pCameraInfo->bLensFunction>>4)<<4) | pTmpChaCfg->bLensFunction;
			pCameraInfo->bWhiteBalance = pTmpChaCfg->bWhiteBalance;	
			pCameraInfo->bSignalFormat = pTmpChaCfg->bSignalFormat;
			pCameraInfo->bRotate90 = pTmpChaCfg->bRotate90;
			pCameraInfo->ExposureValue1 = pTmpChaCfg->ExposureValue1;
			pCameraInfo->ExposureValue2 = pTmpChaCfg->ExposureValue2;

			pCameraInfo++;
			pTmpChaCfg++;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_CAMERA, 0, vbuf, nCount*sizeof(CONFIG_CAMERA_T), waittime);
		if (nRet < 0)
		{
			nRet = NET_ERROR_SETCFG_CAMERA;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}

	if (vbuf)
	{
		delete[] vbuf;
	}
	
	return nRet;
}
//End: zsc(11402)

int	CDevConfig::SetDevConfig_DevCfg(LONG lLoginID, DEV_SYSTEM_ATTR_CFG *pstSAC, int waittime)
{
	if (NULL == pstSAC)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	//set general configure
	CONFIG_GENERAL tmpGenCfg = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_GENERAL, 0, (char *)&tmpGenCfg, sizeof(CONFIG_GENERAL), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERAL))
	{
		tmpGenCfg.LocalNo = pstSAC->wDevNo;
		tmpGenCfg.OverWrite = pstSAC->byOverWrite;
		tmpGenCfg.RecLen = pstSAC->byRecordLen;
		tmpGenCfg.VideoFmt = pstSAC->byVideoStandard;
		tmpGenCfg.DateFmt = pstSAC->byDateFormat;
		tmpGenCfg.DateSprtr = pstSAC->byDateSprtr;
		tmpGenCfg.TimeFmt = pstSAC->byTimeFmt;
		tmpGenCfg.Language = pstSAC->byLanguage;
		tmpGenCfg.DST = pstSAC->byDSTEnable; //夏令时 add by cqs(10842) 2008.10.31
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_GENERAL, 0, (char *)&tmpGenCfg, sizeof(CONFIG_GENERAL), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
#ifdef _DEBUG
			OutputDebugString("SetupConfig(GEN_CFG) failed!\n");				
#endif
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("QueryConfig(SYS_ATTR) in SetConfig failed!\n");
#endif
		return -1;
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_GENERAL;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}

int CDevConfig::SetDevConfig_NetCfg(LONG lLoginID, DEVICE_NET_CFG *pstNC, int waittime)
{
	if (NULL == pstNC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	
	/*
	*	关于下面的组织，开始是把ddns、pppoe、mail放在general的后面的，因为在general可能会对设备的ip或者
	*	port进行修改，这个时候设备的网络模块就会重启，为了防止这样操作的影响，先把这三部分信息放在general
	*	的前面进行配置。
	*/
	/* FTP配置不放网络配置中了
	CONFIG_FTP_PROTO_SET stFtpCfg = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_FTP, FALSE, (char *)&stFtpCfg, sizeof(CONFIG_FTP_PROTO_SET), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_FTP_PROTO_SET))
	{
		stFtpCfg.m_isEnable = pstNC->struFtpServer.byEnable;
		stFtpCfg.m_unHostIP = inet_addr(pstNC->struFtpServer.sHostIPAddr);
		stFtpCfg.m_nHostPort = pstNC->struFtpServer.wHostPort;

		int usernamelen = min(MAX_HOSTNAME_LEN, MAX_USERNAME_LEN);
		memcpy(stFtpCfg.m_cUserName, pstNC->struFtpServer.sHostUser, usernamelen);
		int userpswlen = min(MAX_HOSTPSW_LEN, MAX_PASSWORD_LEN);
		memcpy(stFtpCfg.m_cPassword, pstNC->struFtpServer.sHostPassword, userpswlen);
		
		int ftpdirlen = min(FTP_MAXDIRLEN, 240);
		memcpy(stFtpCfg.m_cDirName, pstNC->struFtpParam.sFtpDir, ftpdirlen);
		stFtpCfg.m_iFileLen = pstNC->struFtpParam.dwFileLen;
		stFtpCfg.m_iInterval = pstNC->struFtpParam.dwIterval;
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_FTP, 0, (char *)&stFtpCfg, sizeof(CONFIG_FTP_PROTO_SET), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
#ifdef _DEBUG
			OutputDebugString("SetupConfig(NET_CFG) failed!\n");				
#endif
		}
		else
		{
			Sleep(3*SETUP_SLEEP);
		}
	}
	else
	{
	//	nFailedCount++;
	}
*/
	//set ddns and pppoe configure
	char szData[1024] = {0};
	int  iStrLen = 0;

	iStrLen = DecodeHostString(0, &pstNC->struPppoe, NULL, szData, 1024, lLoginID);
	if (iStrLen > 0 && iStrLen <= 1024)
	{
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_PPPOE, 0, szData, iStrLen, waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(3*SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("SetupConfig(NET_CFG PPPOE) failed!\n");
#endif
	}
	
	iStrLen = DecodeHostString(1, &pstNC->struDdns, pstNC->sDevName, szData, 1024, lLoginID);
	if (iStrLen > 0 && iStrLen <= 1024)
	{
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_DDNS, 0, szData, iStrLen, waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(3*SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("SetupConfig(NET_CFG DDNS) failed!\n");
#endif
	}
	
	//set mail configure
	iStrLen = DecodeMailCfg((void *)&pstNC->struMail, szData, 1024);

	//由于EMAIL的设置中的结构体已经变掉，已经在外面实现，为避免客户端保存出错，如果客户已经调用了新增的接口，
	//这里就没有必要再保存一遍，把MAIL_CFG中的保留字段作为是否保存的标志，非1代表需要保存，1代表不保存
	//modify by cqs (10842)
	unsigned short savemailflag = pstNC->struMail.wReserved;

	if (savemailflag != 1)
	{
		if (iStrLen > 0 && iStrLen <= 1024 )
		{
			char szMailInfo[1024] = {0};
			memcpy(szMailInfo, szData, iStrLen);
			memset(szData, 0, 1024);
			Change_Assic_UTF8(szMailInfo, strlen(szMailInfo), szData, 1024);
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_MAIL, 0, szData, strlen(szData), waittime);
			if (nRet < 0)
			{
				nFailedCount++;
			}
			else
			{
				Sleep(3*SETUP_SLEEP);
			}
		}
		else
		{
			nFailedCount++;
#ifdef _DEBUG
			OutputDebugString("SetupConfig(NET_CFG MAIL) failed!\n");
#endif
		}
	}
	
	//set comm net config
	CONFIG_NET tmpNetCfg = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_NET, 0, (char *)&tmpNetCfg, sizeof(CONFIG_NET), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_NET))
	{
		afk_device_s *device = (afk_device_s*)lLoginID;

		for (int i = 0; i < 2; i++)
		{
			if (0 == pstNC->stEtherNet[i].bTranMedia)
			{
				if (0 != _stricmp(pstNC->stEtherNet[i].sDevIPAddr, ""))
				{
					tmpNetCfg.GateWayIP = inet_addr(pstNC->stEtherNet[i].sGatewayIP);
					tmpNetCfg.HostIP = inet_addr(pstNC->stEtherNet[i].sDevIPAddr);
					tmpNetCfg.Submask = inet_addr(pstNC->stEtherNet[i].sDevIPMask);
				}
				break;
			}
		}

		int nEthernetNum = 1;
		CONFIG_ETHERNET stuEthernet[5] = {0};
		strcpy(stuEthernet[0].szEthernetName, "eth0");
		GetDevConfig_EthernetCfg(lLoginID, stuEthernet, nEthernetNum, waittime);
		
		// 修改前两块网卡
		strcpy(stuEthernet[0].sDevIPAddr, pstNC->stEtherNet[0].sDevIPAddr);
		strcpy(stuEthernet[0].sDevIPMask, pstNC->stEtherNet[0].sDevIPMask);
		strcpy(stuEthernet[0].sGatewayIP, pstNC->stEtherNet[0].sGatewayIP);
		strcpy(stuEthernet[0].byMACAddr, pstNC->stEtherNet[0].byMACAddr);
		stuEthernet[0].bTranMedia = pstNC->stEtherNet[0].bTranMedia;
		stuEthernet[0].bDefaultEth = pstNC->stEtherNet[0].bDefaultEth;
		stuEthernet[0].bValid = pstNC->stEtherNet[0].bValid & 0x01;
		stuEthernet[0].bDHCPEnable = (pstNC->stEtherNet[0].bValid>>1) & 0x01;
		if (1 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "10Mbps");
			strcpy(stuEthernet[0].szDuplex, "Full");
		}
		else if (2 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "10Mbps");
			strcpy(stuEthernet[0].szDuplex, "Auto");
		}
		else if (3 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "10Mbps");
			strcpy(stuEthernet[0].szDuplex, "Half");
		}
		else if (4 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "100Mbps");
			strcpy(stuEthernet[0].szDuplex, "Full");
		}
		else if (5 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "100Mbps");
			strcpy(stuEthernet[0].szDuplex, "Auto");
		}
		else if (6 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "100Mbps");
			strcpy(stuEthernet[0].szDuplex, "Half");
		}
		else if (7 == pstNC->stEtherNet[0].dwNetInterface)
		{
			strcpy(stuEthernet[0].szSpeed, "Auto");
			strcpy(stuEthernet[0].szDuplex, "Auto");
		}
		
		strcpy(stuEthernet[1].sDevIPAddr, pstNC->stEtherNet[1].sDevIPAddr);
		strcpy(stuEthernet[1].sDevIPMask, pstNC->stEtherNet[1].sDevIPMask);
		strcpy(stuEthernet[1].sGatewayIP, pstNC->stEtherNet[1].sGatewayIP);
		strcpy(stuEthernet[1].byMACAddr, pstNC->stEtherNet[1].byMACAddr);
		stuEthernet[1].bTranMedia = pstNC->stEtherNet[1].bTranMedia;
		stuEthernet[1].bDefaultEth = pstNC->stEtherNet[1].bDefaultEth;
		stuEthernet[1].bValid = pstNC->stEtherNet[1].bValid & 0x01;
		stuEthernet[1].bDHCPEnable = (pstNC->stEtherNet[1].bValid>>1) & 0x01;
		if (1 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "10Mbps");
			strcpy(stuEthernet[1].szDuplex, "Full");
		}
		else if (2 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "10Mbps");
			strcpy(stuEthernet[1].szDuplex, "Auto");
		}
		else if (3 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "10Mbps");
			strcpy(stuEthernet[1].szDuplex, "Half");
		}
		else if (4 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "100Mbps");
			strcpy(stuEthernet[1].szDuplex, "Full");
		}
		else if (5 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "100Mbps");
			strcpy(stuEthernet[1].szDuplex, "Auto");
		}
		else if (6 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "100Mbps");
			strcpy(stuEthernet[1].szDuplex, "Half");
		}
		else if (7 == pstNC->stEtherNet[1].dwNetInterface)
		{
			strcpy(stuEthernet[1].szSpeed, "Auto");
			strcpy(stuEthernet[1].szDuplex, "Auto");
		}
		
		SetDevConfig_EthernetCfg(lLoginID, stuEthernet, nEthernetNum, waittime);
		SetDevConfig_EtherDHCPCfg(lLoginID, stuEthernet, nEthernetNum, waittime);

		if(0 != _stricmp(pstNC->struAlarmHost.sHostIPAddr, ""))//ip为空，保持不变
		{
			tmpNetCfg.AlarmServerIP = inet_addr(pstNC->struAlarmHost.sHostIPAddr);
			tmpNetCfg.AlarmServerPort = pstNC->struAlarmHost.wHostPort;
			tmpNetCfg.AlmSvrStat = pstNC->struAlarmHost.byEnable;
		}
		if(0 != _stricmp(pstNC->struLogHost.sHostIPAddr, ""))
		{
			tmpNetCfg.LogServerIP = inet_addr(pstNC->struLogHost.sHostIPAddr);
			tmpNetCfg.LogServerPort = pstNC->struLogHost.wHostPort;
		}
		if(0 != _stricmp(pstNC->struMultiCast.sHostIPAddr, ""))
		{
			tmpNetCfg.McastIP = inet_addr(pstNC->struMultiCast.sHostIPAddr);
			tmpNetCfg.McastPort = pstNC->struMultiCast.wHostPort;
		}
		if(0 != _stricmp(pstNC->struSmtpHost.sHostIPAddr, ""))
		{
			tmpNetCfg.SMTPServerIP = inet_addr(pstNC->struSmtpHost.sHostIPAddr);
			tmpNetCfg.SMTPServerPort = pstNC->struSmtpHost.wHostPort;
		}
		if( 0 != _stricmp(pstNC->struDns.sHostIPAddr, ""))
		{
			tmpNetCfg.DNSIP = inet_addr(pstNC->struDns.sHostIPAddr);
		}

		memcpy(tmpNetCfg.HostName, pstNC->sDevName, MAX_GENERAL_NAME_LEN);
		tmpNetCfg.HttpPort = pstNC->wHttpPort;
		tmpNetCfg.HttpsPort = pstNC->wHttpsPort;
		tmpNetCfg.SSLPort = pstNC->wSslPort;
		tmpNetCfg.TCPMaxConn = pstNC->wTcpMaxConnectNum;
		tmpNetCfg.TCPPort = pstNC->wTcpPort;
		tmpNetCfg.UDPPort = pstNC->wUdpPort;
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_NET, 0, (char *)&tmpNetCfg, sizeof(CONFIG_NET), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
#ifdef _DEBUG
			OutputDebugString("SetupConfig(NET_CFG) failed!\n");				
#endif
		}
		else
		{
			Sleep(3*SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("QueryConfig(NET_CFG) in SetConfig failed!\n");
#endif
	}

	//上面分N个部分对网络配置信息进行配置，有一个失败就返回错误
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_NETCFG;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	return nRet;
}

int CDevConfig::SetDevConfig_ChnCfg(LONG lLoginID, DEV_CHANNEL_CFG *pstCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (lChannel < -1 || lChannel >= MAX_CHANNUM ||
		(lChannel != -1 && lChannel >= device->channelcount(device)))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	if (NULL == pstCHC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	int iChanNum = device->channelcount(device);
	iChanNum = Min(iChanNum,MAX_VIDEO_IN_NUM);
	iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM);
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iChanNum;
	}
	
	DEV_CHANNEL_CFG *const pDevChaCfg = pstCHC;
	DEV_CHANNEL_CFG *pTmpChaCfg = pDevChaCfg;
	int i = 0;
	int iTmpIdx = iChanIdx;
	int iCopyStrLen = 0;
	int buflen = 0;
	char *vbuf = NULL;
	if (buflen < CONFIG_CAPTURE_NUM*CHANNEL_NAME_LEN)
	{
		buflen = CONFIG_CAPTURE_NUM*CHANNEL_NAME_LEN;
	}
	if (buflen < CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE))
	{
		buflen = CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE);
	}
	if (buflen < CONFIG_CAPTURE_NUM*sizeof(NEW_CONFIG_CAPTURE))
	{
		buflen = CONFIG_CAPTURE_NUM*sizeof(NEW_CONFIG_CAPTURE);
	}
	if (buflen < CONFIG_PTZ_NUM*sizeof(CONFIG_PTZ))
	{
		buflen = CONFIG_PTZ_NUM*sizeof(CONFIG_PTZ);
	}
	if (buflen < CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE_V2201))
	{
		buflen = CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE_V2201);
	}
	if (buflen < CONFIG_CAPTURE_NUM*sizeof(AUDIO_CAPTURE_CFG))
	{
		buflen = CONFIG_CAPTURE_NUM*sizeof(AUDIO_CAPTURE_CFG);
	}
	if (buflen < CONFIG_CAPTURE_NUM*sizeof(CONFIG_COLOR))
	{
		buflen = CONFIG_CAPTURE_NUM*sizeof(CONFIG_COLOR);
	}
	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_VIDEO;
	}
	
	//set channel name
	pTmpChaCfg = pDevChaCfg;
	iTmpIdx = iChanIdx;
	memset((void *)vbuf, 0, buflen);
	nRet = QueryChannelName(lLoginID, vbuf, buflen/*CONFIG_CAPTURE_NUM*CHANNEL_NAME_LEN*/, &retlen, waittime);
	if (nRet >= 0)
	{
		for (i = 0; i < iChanCount; i++)
		{
			iCopyStrLen = strlen(pTmpChaCfg->szChannelName)<sizeof(pTmpChaCfg->szChannelName)?strlen(pTmpChaCfg->szChannelName):sizeof(pTmpChaCfg->szChannelName);
			if (iCopyStrLen > 0)
			{
				memset((void *)(vbuf+iTmpIdx*CHANNEL_NAME_LEN), 0, CHANNEL_NAME_LEN);
				memcpy((void *)(vbuf+iTmpIdx*CHANNEL_NAME_LEN), pTmpChaCfg->szChannelName, iCopyStrLen);
			}
			
			iTmpIdx ++;
			pTmpChaCfg++;
		}
		nRet = SetupChannelName(lLoginID, vbuf, 16*CHANNEL_NAME_LEN);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
	}
	
	int iProtocolVersion;
	device->get_info(device, dit_protocol_version, &iProtocolVersion);
	if (iProtocolVersion < 5)
	{
		//set video parameters (old)
		CONFIG_CAPTURE *pCapCfg = NULL;
		/*
		*	这里不管是有没有内容的交叠都要重新获取一次配置，在获取内容的基础上修改，因为在和设备通信的配置中有
		*	版本信息，如果版本不符，设备就不接受这些配置信息
		*/
		pTmpChaCfg = pDevChaCfg;
		iTmpIdx = iChanIdx;
		memset((void *)vbuf, 0, buflen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE, 0, vbuf, CONFIG_CAPTURE_NUM * sizeof(CONFIG_CAPTURE), &retlen, waittime);
		if (nRet >= 0 && retlen == CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE))
		{
			pCapCfg = (CONFIG_CAPTURE *)(vbuf + iTmpIdx*sizeof(CONFIG_CAPTURE));
			for (i = 0; i < iChanCount; i++)
			{
				pCapCfg->AudioEn = pTmpChaCfg->stMainVideoEncOpt[0].byAudioEnable;
				pCapCfg->Brightness = pTmpChaCfg->stColorCfg[0].byBrightness;
				pCapCfg->CifMode = pTmpChaCfg->stMainVideoEncOpt[0].byImageSize;
				pCapCfg->Contrast = pTmpChaCfg->stColorCfg[0].byContrast;
				memcpy((void *)&pCapCfg->Cover, (void *)&pTmpChaCfg->stBlindCover[0].rcRect, sizeof(YW_RECT));
				pCapCfg->CoverEnable = pTmpChaCfg->byBlindEnable;
				pCapCfg->EncodeMode = pTmpChaCfg->stMainVideoEncOpt[0].byBitRateControl;
				pCapCfg->Frames = pTmpChaCfg->stMainVideoEncOpt[0].byFramesPerSec;
				pCapCfg->Gain = pTmpChaCfg->stColorCfg[0].byGain;
				if (0 == pTmpChaCfg->stColorCfg[0].byGainEn)
				{
					pCapCfg->Gain += 128;
				}
				pCapCfg->Hue = pTmpChaCfg->stColorCfg[0].byHue;
				pCapCfg->ImgQlty = pTmpChaCfg->stMainVideoEncOpt[0].byImageQlty;
				pCapCfg->Saturation = pTmpChaCfg->stColorCfg[0].bySaturation;
				pCapCfg->VideoType = pTmpChaCfg->stMainVideoEncOpt[0].byEncodeMode;
				
				pTmpChaCfg++;
				pCapCfg++;
			}
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_CAPTURE, 0, vbuf, CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE), waittime);
			if (nRet < 0)
			{
				nFailedCount++;
			}
			else
			{
				Sleep(SETUP_SLEEP);
			}
		}
		else
		{
			//allow failure!
			//nFailedCount++;
		}
		
		//set video config (128)
		memset(vbuf, 0, buflen);
		pTmpChaCfg = pDevChaCfg;
		iTmpIdx = iChanIdx;
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_128, 0, vbuf, CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE_V2201), &retlen, waittime);
		if (nRet >= 0 && retlen == CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE_V2201))
		{
			CONFIG_CAPTURE_V2201 *pCfgCap = (CONFIG_CAPTURE_V2201 *)(vbuf+iTmpIdx*sizeof(CONFIG_CAPTURE_V2201));
			int iMinIndex = 0;
			for (i = 0; i < iChanCount; i++)
			{
				pCfgCap->VideoEn = pTmpChaCfg->stMainVideoEncOpt[0].byVideoEnable;
				pCfgCap->AudioEn = pTmpChaCfg->stMainVideoEncOpt[0].byAudioEnable;
				pCfgCap->CifMode = pTmpChaCfg->stMainVideoEncOpt[0].byImageSize;
				pCfgCap->EncodeMode = pTmpChaCfg->stMainVideoEncOpt[0].byBitRateControl;
				pCfgCap->Frames = pTmpChaCfg->stMainVideoEncOpt[0].byFramesPerSec;
				pCfgCap->ImgQlty = pTmpChaCfg->stMainVideoEncOpt[0].byImageQlty;
				pCfgCap->VideoType = pTmpChaCfg->stMainVideoEncOpt[0].byEncodeMode;
				
				pCfgCap->CoverEnable = pTmpChaCfg->byBlindEnable;
				memcpy(&pCfgCap->Cover, &pTmpChaCfg->stBlindCover[0].rcRect, sizeof(YW_RECT));
				
				pCfgCap->TimeTilteEn = pTmpChaCfg->stTimeOSD.bShow;
				pCfgCap->TimeTitleL = pTmpChaCfg->stTimeOSD.rcRect.left;
				pCfgCap->TimeTitleR = pTmpChaCfg->stTimeOSD.rcRect.right;
				pCfgCap->TimeTitleT = pTmpChaCfg->stTimeOSD.rcRect.top;
				pCfgCap->TimeTitleB = pTmpChaCfg->stTimeOSD.rcRect.bottom;
				
				pCfgCap->ChTitleEn = pTmpChaCfg->stChannelOSD.bShow;
				pCfgCap->ChannelTitleL = pTmpChaCfg->stChannelOSD.rcRect.left;
				pCfgCap->ChannelTitleR = pTmpChaCfg->stChannelOSD.rcRect.right;
				pCfgCap->ChannelTitleT = pTmpChaCfg->stChannelOSD.rcRect.top;
 				pCfgCap->ChannelTitleB = pTmpChaCfg->stChannelOSD.rcRect.bottom;

				pCfgCap->Brightness = pTmpChaCfg->stColorCfg[0].byBrightness;
				pCfgCap->Contrast = pTmpChaCfg->stColorCfg[0].byContrast;
				pCfgCap->Gain = pTmpChaCfg->stColorCfg[0].byGain;
				if (!pTmpChaCfg->stColorCfg[0].byGainEn)
				{
					pCfgCap->Gain += 128;
				}
				pCfgCap->Hue = pTmpChaCfg->stColorCfg[0].byHue;
				pCfgCap->Saturation = pTmpChaCfg->stColorCfg[0].bySaturation;
				
				pCfgCap++;
				pTmpChaCfg++;
			}
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_CAPTURE_128, 0, vbuf, CONFIG_CAPTURE_NUM*sizeof(CONFIG_CAPTURE_V2201), waittime);
			if (nRet < 0)
			{
			}
			else
			{
				Sleep(SETUP_SLEEP);
			}
		}
	}//if (iProtocolVersion < 5)
	
	//set video config (new)
	NEW_CONFIG_CAPTURE *pNewCapCfg = NULL;
	pTmpChaCfg = pDevChaCfg;
	iTmpIdx = iChanIdx;
	memset((void *)vbuf, 0, buflen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, vbuf, CONFIG_CAPTURE_NUM * sizeof(NEW_CONFIG_CAPTURE), &retlen, waittime);
	if (nRet >= 0  && (retlen == CONFIG_CAPTURE_NUM*sizeof(NEW_CONFIG_CAPTURE)
		 || retlen == device->channelcount(device)*sizeof(NEW_CONFIG_CAPTURE)))
	{
		pNewCapCfg = (NEW_CONFIG_CAPTURE *)(vbuf + iTmpIdx*sizeof(NEW_CONFIG_CAPTURE));
		int j = 0;
		int iMinIndex = 0;
		for (i = 0; i < iChanCount; i++)
		{

			pNewCapCfg->Volume = pTmpChaCfg->bVolume;
			pNewCapCfg->VolumeEnable = pTmpChaCfg->bVolumeEnable;
			pNewCapCfg->Noise = pTmpChaCfg->bNoise;

			iMinIndex = Min(N_ENCODE_AUX_INTER,ENCODE_AUX_NUM);
			for (j = 0; j < iMinIndex; j++)
			{
				pNewCapCfg->AssiOption[j].AVEnable = 0;
				if (pTmpChaCfg->stAssiVideoEncOpt[j].byVideoEnable)
				{
					pNewCapCfg->AssiOption[j].AVEnable |= 0x01;
				}
				if (pTmpChaCfg->stAssiVideoEncOpt[j].byAudioEnable)
				{
					pNewCapCfg->AssiOption[j].AVEnable |= 0x02;
				}
				if (pTmpChaCfg->stAssiVideoEncOpt[j].bAudioOverlay)
				{
					pNewCapCfg->AssiOption[j].AVEnable |= 0x04;
				}
				pNewCapCfg->AssiOption[j].BitRateControl = pTmpChaCfg->stAssiVideoEncOpt[j].byBitRateControl;
				pNewCapCfg->AssiOption[j].Frames = pTmpChaCfg->stAssiVideoEncOpt[j].byFramesPerSec;
				pNewCapCfg->AssiOption[j].ImageSize = pTmpChaCfg->stAssiVideoEncOpt[j].byImageSize;
				pNewCapCfg->AssiOption[j].ImgQlty = pTmpChaCfg->stAssiVideoEncOpt[j].byImageQlty;
				//	Add by zsc(11402) 2008-1-18
				pNewCapCfg->AssiOption[j].usBitRate = pTmpChaCfg->stAssiVideoEncOpt[j].wLimitStream;
				pNewCapCfg->AssiOption[j].IFrameInterval = pTmpChaCfg->stAssiVideoEncOpt[j].bIFrameInterval;

				//  add by cqs(10842) 2009.3.14
				//pNewCapCfg->AssiScanMode[j] = pTmpChaCfg->stAssiVideoEncOpt[j].bScanMode;
				
			}
			iMinIndex = Min(REC_TYPE_NUM,REC_TYP_NUM_INTER);
			for (j = 0; j < iMinIndex; j++)
			{
				pNewCapCfg->MainOption[j].AVEnable = 0;
				if (pTmpChaCfg->stMainVideoEncOpt[j].byVideoEnable)
				{
					pNewCapCfg->MainOption[j].AVEnable |= 0x01;
				}
				if (pTmpChaCfg->stMainVideoEncOpt[j].byAudioEnable)
				{
					pNewCapCfg->MainOption[j].AVEnable |= 0x02;
				}
				if (pTmpChaCfg->stMainVideoEncOpt[j].bAudioOverlay)
				{
					pNewCapCfg->MainOption[j].AVEnable |= 0x04;
				}
				pNewCapCfg->MainOption[j].BitRateControl = pTmpChaCfg->stMainVideoEncOpt[j].byBitRateControl;
				pNewCapCfg->MainOption[j].Frames = pTmpChaCfg->stMainVideoEncOpt[j].byFramesPerSec;
				pNewCapCfg->MainOption[j].ImageSize = pTmpChaCfg->stMainVideoEncOpt[j].byImageSize;
				pNewCapCfg->MainOption[j].ImgQlty = pTmpChaCfg->stMainVideoEncOpt[j].byImageQlty;
				//	Add by zsc(11402) 2008-1-18
				pNewCapCfg->MainOption[j].usBitRate = pTmpChaCfg->stMainVideoEncOpt[j].wLimitStream;
				pNewCapCfg->MainOption[j].IFrameInterval = pTmpChaCfg->stMainVideoEncOpt[j].bIFrameInterval;

				//  add by cqs(10842) 2009.3.14
				//pNewCapCfg->MainScanMode[j] = pTmpChaCfg->stMainVideoEncOpt[j].bScanMode;
			}
			
			pNewCapCfg->ChannelTitle.TitlebgRGBA = pTmpChaCfg->stChannelOSD.rgbaBackground;
			pNewCapCfg->ChannelTitle.TitlefgRGBA = pTmpChaCfg->stChannelOSD.rgbaFrontground;

			pNewCapCfg->ChannelTitle.TitleTop = pTmpChaCfg->stChannelOSD.rcRect.top;
			pNewCapCfg->ChannelTitle.TitleBottom = pTmpChaCfg->stChannelOSD.rcRect.bottom;
			pNewCapCfg->ChannelTitle.TitleLeft = pTmpChaCfg->stChannelOSD.rcRect.left;
			pNewCapCfg->ChannelTitle.TitleRight = pTmpChaCfg->stChannelOSD.rcRect.right;
			pNewCapCfg->ChannelTitle.TitleEnable = pTmpChaCfg->stChannelOSD.bShow;
			
			pNewCapCfg->TimeTitle.TitlebgRGBA = pTmpChaCfg->stTimeOSD.rgbaBackground;
			pNewCapCfg->TimeTitle.TitlefgRGBA = pTmpChaCfg->stTimeOSD.rgbaFrontground;
			pNewCapCfg->TimeTitle.TitleTop = pTmpChaCfg->stTimeOSD.rcRect.top;
			pNewCapCfg->TimeTitle.TitleBottom = pTmpChaCfg->stTimeOSD.rcRect.bottom;
			pNewCapCfg->TimeTitle.TitleLeft = pTmpChaCfg->stTimeOSD.rcRect.left;
			pNewCapCfg->TimeTitle.TitleRight = pTmpChaCfg->stTimeOSD.rcRect.right;
			pNewCapCfg->TimeTitle.TitleEnable = pTmpChaCfg->stTimeOSD.bShow;
			
			pNewCapCfg->CoverEnable = pTmpChaCfg->byBlindEnable;
			
			memcpy((void *)&pNewCapCfg->Cover, (void *)&pTmpChaCfg->stBlindCover[0].rcRect, sizeof(YW_RECT));
			
			pNewCapCfg->Compression = pTmpChaCfg->stMainVideoEncOpt[0].byEncodeMode;
			
			iTmpIdx++;
			pTmpChaCfg++;
			pNewCapCfg++;
		}

		int iSetupLen = retlen;	//返回多长数据就设置回多长的数据
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, vbuf, iSetupLen, waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else
	{
		nRet = NET_NOERROR;//这里不对失败计数，因为有些机器是不支持这个命令查询的
	}
	
	/*
	pTmpChaCfg = pDevChaCfg;
	iTmpIdx = iChanIdx;
	memset(vbuf, 0, buflen);
	//这里没有数据交叉也没有版本验证，所以就不重新查询
	{
		AUDIO_CAPTURE_CFG *pAdoCap = (AUDIO_CAPTURE_CFG *)(vbuf + iTmpIdx*sizeof(AUDIO_CAPTURE_CFG));
		int i = 0, j = 0;
		int iMinIndex = 0;
		for (i = 0; i < iChanCount; i++)
		{
			iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
			for (j = 0; j < iMinIndex; j++)
			{
				pAdoCap->nChannels = pTmpChaCfg->stMainVideoEncOpt[j].nChannels;
				pAdoCap->nSamplesPerSec = pTmpChaCfg->stMainVideoEncOpt[j].nSamplesPerSec;
				pAdoCap->dwBitsPerSample = pTmpChaCfg->stMainVideoEncOpt[j].wBitsPerSample;
				pAdoCap->wFormatTag = pTmpChaCfg->stMainVideoEncOpt[j].wFormatTag;
			}
			iMinIndex = N_ENCODE_AUX_INTER<ENCODE_AUX_NUM?N_ENCODE_AUX_INTER:ENCODE_AUX_NUM;
			for (j = 0; j < iMinIndex; j++)
			{
				pAdoCap->nChannels = pTmpChaCfg->stAssiVideoEncOpt[j].nChannels;
				pAdoCap->nSamplesPerSec = pTmpChaCfg->stAssiVideoEncOpt[j].nSamplesPerSec;
				pAdoCap->dwBitsPerSample = pTmpChaCfg->stAssiVideoEncOpt[j].wBitsPerSample;
				pAdoCap->wFormatTag = pTmpChaCfg->stAssiVideoEncOpt[j].wFormatTag;
			}

			pAdoCap++;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_AUDIO_CAPTURE, 0, vbuf, CONFIG_CAPTURE_NUM*sizeof(AUDIO_CAPTURE_CFG), waittime);
		if (nRet < 0)
		{
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	*/
	if (iProtocolVersion > 2)
	{
		//set color config with time section
		pTmpChaCfg = pDevChaCfg;
		iTmpIdx = iChanIdx;
		memset(vbuf, 0, buflen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_COLOR, 0, vbuf, buflen, &retlen, waittime);
		if(nRet >= 0 && 16*sizeof(CONFIG_COLOR) == retlen)
		{
			CONFIG_COLOR *pColorCfg = (CONFIG_COLOR *)(vbuf + iTmpIdx*sizeof(CONFIG_COLOR));
			int i = 0, j = 0;
			int iMinIndex = 0;
			for (i = 0; i < iChanCount; i++)
			{
				iMinIndex = COL_TIME_SECT_NUM<N_COLOR_SECTION?COL_TIME_SECT_NUM:N_COLOR_SECTION;
				for (j = 0; j < iMinIndex; j++)
				{
					pColorCfg->Color[j].Brightness = pstCHC[i].stColorCfg[j].byBrightness;
					pColorCfg->Color[j].Contrast = pstCHC[i].stColorCfg[j].byContrast;
					pColorCfg->Color[j].Gain = pstCHC[i].stColorCfg[j].byGain;
					if (!pstCHC[i].stColorCfg[j].byGainEn)
					{
						pColorCfg->Color[j].Gain += 128;
					}
					pColorCfg->Color[j].Hue = pstCHC[i].stColorCfg[j].byHue;
					pColorCfg->Color[j].Saturation = pstCHC[i].stColorCfg[j].bySaturation;
					pColorCfg->Color[j].Sector.State = pstCHC[i].stColorCfg[j].stSect.bEnable;
					pColorCfg->Color[j].Sector.StartHour = pstCHC[i].stColorCfg[j].stSect.iBeginHour;
					pColorCfg->Color[j].Sector.StartMin = pstCHC[i].stColorCfg[j].stSect.iBeginMin;
					pColorCfg->Color[j].Sector.StartSec = pstCHC[i].stColorCfg[j].stSect.iBeginSec;
					pColorCfg->Color[j].Sector.HourEnd = pstCHC[i].stColorCfg[j].stSect.iHourEnd;
					pColorCfg->Color[j].Sector.EndMin = pstCHC[i].stColorCfg[j].stSect.iEndMin;
					pColorCfg->Color[j].Sector.EndSec = pstCHC[i].stColorCfg[j].stSect.iEndSec;
				}

				pColorCfg++;
			}
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_COLOR, 0, vbuf, CONFIG_CAPTURE_NUM*sizeof(CONFIG_COLOR), waittime);
			if (nRet < 0)
			{
				nFailedCount++;
			}
			else
			{
				Sleep(SETUP_SLEEP);
			}
		}
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_VIDEO;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	
	return nRet;
}

//Add: zsc(11402) 2008-1-18
/*
 *	设置区域遮挡配置信息
 */
int	CDevConfig::SetDevConfig_ChnCoverCfg(LONG lLoginID, DEV_VIDEOCOVER_CFG *pChnCoverCfg, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (pChnCoverCfg == NULL || lChannel < 0 || lChannel >= MAX_CHANNUM || (lChannel >= device->channelcount(device)))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRet = -1;  //return value
	int retlen = 0; //data length
	int iCopyStrLen = 0;
	int buflen = 0;
	char *vbuf = NULL;
	DEV_VIDEOCOVER_CFG *const pDevChnCfg = pChnCoverCfg;
	DEV_VIDEOCOVER_CFG *pTmpChaCfg = pDevChnCfg;

	if (buflen < sizeof(DEV_VIDEOCOVER_CFG))
	{
		buflen = sizeof(DEV_VIDEOCOVER_CFG);
	}
	
	vbuf = new char[buflen];
	if (vbuf == NULL)
	{
		return NET_SYSTEM_ERROR;
	}
	
	//set video parameters (old)
	pTmpChaCfg = pDevChnCfg;
	memset((void *)vbuf, 0, buflen);
	
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_COVER, lChannel+1, vbuf, buflen, &retlen, waittime);
	if (nRet>=0 && retlen>=sizeof(CONFIG_VIDEO_COVER_T) && (0==(retlen-sizeof(CONFIG_VIDEO_COVER_T))%sizeof(CONFIG_VIDEO_COVER_ATTR)))
	{
		CONFIG_VIDEO_COVER_T *pVideoCover = NULL;
		CONFIG_VIDEO_COVER_ATTR *pCoverAttr = NULL;
		int nCoverCount = 0;
		
		pVideoCover = (CONFIG_VIDEO_COVER_T *)vbuf;
		pVideoCover->iChannel = lChannel + 1;
		pVideoCover->iCoverNum = pTmpChaCfg->bCoverCount;
		
		nCoverCount = pVideoCover->iCoverNum;
		for(int i=0; i<nCoverCount; i++)
		{
			pCoverAttr = (CONFIG_VIDEO_COVER_ATTR *)(vbuf + sizeof(CONFIG_VIDEO_COVER_T) + i*sizeof(CONFIG_VIDEO_COVER_ATTR));
			memcpy(&(pCoverAttr->rcBlock), &(pTmpChaCfg->CoverBlock[i].rcBlock), sizeof(YW_RECT));
			pCoverAttr->Color = pTmpChaCfg->CoverBlock[i].nColor;
			pCoverAttr->iBlockType = pTmpChaCfg->CoverBlock[i].bBlockType;
			pCoverAttr->Encode = pTmpChaCfg->CoverBlock[i].bEncode;
			pCoverAttr->Priview = pTmpChaCfg->CoverBlock[i].bPriview;
		}

		retlen = sizeof(CONFIG_VIDEO_COVER_T) + nCoverCount * sizeof(CONFIG_VIDEO_COVER_ATTR);
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_COVER, lChannel, vbuf, retlen, waittime);
		if (nRet < 0)
		{
			nRet = NET_ERROR_SETCFG_COVER;
			goto END;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_COVER;
		goto END;
	}

END:
	if (vbuf)
	{
		delete[] vbuf;
	}
	
	return nRet;
}
//End: zsc(11402)

//Add: ljy(10858) 2008-3-11
int	CDevConfig::SetDevConfig_TransStrategyCfg(LONG lLoginID, DEV_TRANSFER_STRATEGY_CFG *pTransStrategyCfg, int waittime)
{
	if ( NULL == pTransStrategyCfg)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int buflen = 500;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;
	
	retlen = buflen;
	nRet = BuildTransStrategyCfgString(pTransStrategyCfg, tmpbuf, &retlen);
	if (nRet < 0 || retlen <= 0)
	{
		return nRet;
	}
	
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_TRANSFER_STRATEGY, 0, tmpbuf, retlen, waittime);
	if (nRet)
	{
		nFailedCount++;
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	delete tmpbuf;

	return nRet;
}
//End: ljy(10858)

//Add: ljy(10858) 2008-3-24
int	CDevConfig::SetDevConfig_DownloadStrategyCfg(LONG lLoginID, DEV_DOWNLOAD_STRATEGY_CFG *pDownloadStrategyCfg, int waittime)
{
	if ( NULL == pDownloadStrategyCfg)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int buflen = 500;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;
	
	retlen = buflen;
	nRet = BuildDownloadStrategyCfgString(pDownloadStrategyCfg, tmpbuf, &retlen);
	if (nRet < 0 || retlen <= 0)
	{
		return nRet;
	}
	
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_DOWNLOAD_STRATEGY, 0, tmpbuf, retlen, waittime);
	if (nRet)
	{
		nFailedCount++;
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR;
	}
	else
	{
		nRet = NET_NOERROR;
	}

	delete tmpbuf;
	
	return nRet;
}
//End: ljy(10858)

int	CDevConfig::SetDevConfig_RecCfg(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChannel, int waittime)
{
	if (lChannel < -1 || lChannel >= MAX_CHANNUM || NULL == pstREC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	int iChanNum = device->channelcount(device);
	iChanNum = Min(iChanNum,MAX_VIDEO_IN_NUM);
	iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM);
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iChanNum;
	}
	DEV_RECORD_CFG *const pDevRecCfg = pstREC;
	DEV_RECORD_CFG *pTmpRecCfg = pDevRecCfg;
	int i = 0;
	int iTmpIdx = iChanIdx;
	int iCopyStrLen = 0;
	int buflen = 0;
	char *vbuf = NULL;
	if (buflen < CONFIG_RECORD_NUM*sizeof(CONFIG_RECORD))
	{
		buflen = CONFIG_RECORD_NUM*sizeof(CONFIG_RECORD);
	}
	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_RECORD;
	}
	memset((void *)vbuf, 0, buflen);
	
	CONFIG_RECORD *pRecCfg = NULL;
	pTmpRecCfg = pDevRecCfg;
	memset((void *)vbuf, 0, buflen);
	iTmpIdx = iChanIdx;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD, 0, vbuf, CONFIG_RECORD_NUM*sizeof(CONFIG_RECORD), &retlen, waittime);
	if (nRet >= 0 && retlen == CONFIG_RECORD_NUM*sizeof(CONFIG_RECORD))
	{
		pRecCfg = (CONFIG_RECORD *)(vbuf + iTmpIdx*sizeof(CONFIG_RECORD));
		for (i = 0; i < iChanCount; i++)
		{
			pRecCfg->PreRecord = pTmpRecCfg->byPreRecordLen;
			pRecCfg->Redundancy = pTmpRecCfg->byRedundancyEn;

			for (int weekidx = 0; weekidx < N_WEEKS; weekidx++)
			{
				for (int secidx = 0; secidx < N_TSECT; secidx++)
				{
					pRecCfg->Sector[weekidx][secidx].StartHour = pTmpRecCfg->stSect[weekidx][secidx].iBeginHour;
					pRecCfg->Sector[weekidx][secidx].StartMin = pTmpRecCfg->stSect[weekidx][secidx].iBeginMin;
					pRecCfg->Sector[weekidx][secidx].StartSec = pTmpRecCfg->stSect[weekidx][secidx].iBeginSec;
					pRecCfg->Sector[weekidx][secidx].HourEnd = pTmpRecCfg->stSect[weekidx][secidx].iHourEnd;
					pRecCfg->Sector[weekidx][secidx].EndMin = pTmpRecCfg->stSect[weekidx][secidx].iEndMin;
					pRecCfg->Sector[weekidx][secidx].EndSec = pTmpRecCfg->stSect[weekidx][secidx].iEndSec;
					pRecCfg->Sector[weekidx][secidx].State = 0;
					pRecCfg->Sector[weekidx][secidx].State |= (pTmpRecCfg->stSect[weekidx][secidx].bEnable&0x01)?0x04:0x00;
					pRecCfg->Sector[weekidx][secidx].State |= (pTmpRecCfg->stSect[weekidx][secidx].bEnable&0x02)?0x08:0x00;
					pRecCfg->Sector[weekidx][secidx].State |= (pTmpRecCfg->stSect[weekidx][secidx].bEnable&0x04)?0x02:0x00;
				}
			}
			
			iTmpIdx++;
			pTmpRecCfg++;
			pRecCfg++;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_RECORD, 0, vbuf, CONFIG_RECORD_NUM*sizeof(CONFIG_RECORD), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_RECORD;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_RecCfgNew(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChannel, int waittime)
{	
	if (lChannel < -1 || /*lChannel >= MAX_CHANNUM ||*/ NULL == pstREC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	int iChanNum = device->channelcount(device);
	iChanNum = Min(iChanNum, MAX_VIDEO_IN_NUM_EX);
	iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM_EX);
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)
	{
		iChanIdx = 0;
		iChanCount = iChanNum;
	}

	int iTmpIdx = iChanIdx;

	int buflen = 0;
	char *vbuf = NULL;
	int nbufcount = iChanNum>16?iChanNum:16;
	if (buflen < nbufcount*sizeof(CONFIG_RECORD_NEW))
	{
		buflen = nbufcount*sizeof(CONFIG_RECORD_NEW);
	}
	if (buflen < nbufcount*sizeof(CONFIG_WORKSHEET))
	{
		buflen = nbufcount*sizeof(CONFIG_WORKSHEET);
	}
	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_RECORD;
	}
	memset((void *)vbuf, 0, buflen);
	
	DEV_RECORD_CFG *pTmpRecCfg = pstREC;
	CONFIG_RECORD_NEW *pRecCfgNew = NULL;

	memset((void *)vbuf, 0, buflen);
	iTmpIdx = iChanIdx;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, vbuf, nbufcount*sizeof(CONFIG_RECORD_NEW), &retlen, waittime);
	if (nRet >= 0 && retlen%sizeof(CONFIG_RECORD_NEW) == 0 && iChanIdx < retlen/sizeof(CONFIG_RECORD_NEW))
	{
		int nCount = retlen/sizeof(CONFIG_RECORD_NEW);
		pRecCfgNew = (CONFIG_RECORD_NEW *)(vbuf + iTmpIdx*sizeof(CONFIG_RECORD_NEW));
		for (int i = 0; i < iChanCount; i++)
		{
			pRecCfgNew->iPreRecord = pTmpRecCfg->byPreRecordLen;
			pRecCfgNew->bRedundancy = pTmpRecCfg->byRedundancyEn;
			pRecCfgNew->byRecordType = pTmpRecCfg->byRecordType;
			
			iTmpIdx++;
			pTmpRecCfg++;
			pRecCfgNew++;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, vbuf, nCount*sizeof(CONFIG_RECORD_NEW), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else
	{
		nFailedCount++;
	}
	
	{
		memset(vbuf, 0, buflen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)vbuf;

		if(lChannel == -1)
		{
			for (int i = 0; i < iChanCount; i++,pWSheet++)
			{
				pWSheet->iName = i;
				memcpy(pWSheet->tsSchedule, pstREC[i].stSect, sizeof(pWSheet->tsSchedule));
			}	
		}
		else
		{
			nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_RECORD, (CONFIG_WORKSHEET *)vbuf, waittime, iChanCount);
			if(nRet >= 0)
			{
				pWSheet->iName = lChannel;
				memcpy(pWSheet[lChannel].tsSchedule, pstREC->stSect, sizeof(pWSheet->tsSchedule));
			}
		}
		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_RECORD, (CONFIG_WORKSHEET*)vbuf, waittime, iChanCount);
		if (nRet < 0)
		{
			nFailedCount++;
		}
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_RECORD;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_ComCfg(LONG lLoginID, DEV_COMM_CFG *pstCMC, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0 || NULL == pstCMC)
	{
		return NET_INVALID_HANDLE;
	}

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	DEV_COMM_CFG *pDevComCfg = pstCMC;
	int i = 0;
	int buflen = 0;
	char *vbuf = NULL;
	if (buflen < CONFIG_PTZ_NUM*sizeof(CONFIG_PTZ))
	{
		buflen = CONFIG_PTZ_NUM*sizeof(CONFIG_PTZ);
	}
	if (buflen < sizeof(CONFIG_COMM))
	{
		buflen = sizeof(CONFIG_COMM);
	}
	if (buflen < MAX_COMM_PROTO_NUM*sizeof(DEC_PROTOCOL))
	{
		buflen = MAX_COMM_PROTO_NUM*sizeof(DEC_PROTOCOL);
	}
	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_RECORD;
	}
	memset((void *)vbuf, 0, buflen);
	
	CONFIG_PTZ *pPtzCfg = NULL;
	memset((void *)vbuf, 0, buflen);

	//	此功能解码器除外
	if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
	{
		if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_PTZCONF])
		{
			nRet = QueryConfig(lLoginID, CONFIG_TYPE_PTZ, 0, vbuf, CONFIG_PTZ_NUM*sizeof(CONFIG_PTZ), &retlen, waittime);
			if (nRet >= 0 && retlen == 16*sizeof(CONFIG_PTZ))
			{
				pPtzCfg = (CONFIG_PTZ *)vbuf;
				for (i = 0; i < CONFIG_PTZ_NUM; i++)
				{
					pPtzCfg[i].DestAddr = pDevComCfg->stDecoder[i].wDecoderAddress;
					pPtzCfg[i].Protocol = pDevComCfg->stDecoder[i].wProtocol;
					//			pPtzCfg[i].CamID = pDevComCfg->stDecoder[i].byCamID;
					pPtzCfg[i].byMartixID = pDevComCfg->stDecoder[i].byMartixID;
					pPtzCfg[i].bPTZType = pDevComCfg->stDecoder[i].bPTZType;
					pPtzCfg[i].PTZAttr.baudrate = ChangeBaudRate(pDevComCfg->stDecoder[i].struComm.byBaudRate);
					pPtzCfg[i].PTZAttr.databits = pDevComCfg->stDecoder[i].struComm.byDataBit+5;
					pPtzCfg[i].PTZAttr.parity = pDevComCfg->stDecoder[i].struComm.byParity;
					pPtzCfg[i].PTZAttr.stopbits = pDevComCfg->stDecoder[i].struComm.byStopBit;
				}
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_PTZ, 0, vbuf, 16*sizeof(CONFIG_PTZ), waittime);
				if (nRet < 0)
				{
					nFailedCount++;
				}
				else
				{
					Sleep(SETUP_SLEEP);
				}
			}
			else
			{
				nFailedCount++;
			}
		}
	}
	
	//set 232 comm configure
	/*
			 *	因为这里通过了一次掩码过滤，所以还需要再利用一次掩码，掩码的获取是通过get 232 function name list得到的。
			 */
	//Get 232 function name list
	DWORD dw232FuncMask = 0;
	memset((void *)vbuf, 0, buflen);
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_COMCONF])
	{
		nRet = QueryComProtocol(lLoginID, 1, vbuf, MAX_COMM_PROTO_NUM*sizeof(DEC_PROTOCOL), &retlen, waittime);
		if (nRet >= 0 && (retlen-4)%sizeof(DEC_PROTOCOL) == 0)
		{
			//header 4 byte stands protocol number
			dw232FuncMask = *(DWORD*)vbuf;
			
			CONFIG_COMM *pCommCfg = NULL;
			memset((void *)vbuf, 0, buflen);
			nRet = QueryConfig(lLoginID, CONFIG_TYPE_SERIAL_COMM, 0, vbuf, sizeof(CONFIG_COMM), &retlen, waittime);
			if (nRet >= 0 && retlen == sizeof(CONFIG_COMM))
			{
				pCommCfg = (CONFIG_COMM *)vbuf;
				pCommCfg->BaudBase = ChangeBaudRate(pDevComCfg->st232[0].struComm.byBaudRate);
				pCommCfg->DataBits = pDevComCfg->st232[0].struComm.byDataBit+5;
				pCommCfg->Parity = pDevComCfg->st232[0].struComm.byParity;
				pCommCfg->StopBits = pDevComCfg->st232[0].struComm.byStopBit;
				GetMaskedFuncIndex2Dev(&pCommCfg->Function, pDevComCfg->st232[0].byFunction, dw232FuncMask);
				
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_SERIAL_COMM, 0, vbuf, sizeof(CONFIG_COMM), waittime);
				if (nRet < 0)
				{
					nFailedCount++;
				}
				else
				{
					Sleep(SETUP_SLEEP);
				}
			}
			else
			{
				nFailedCount++;
			}
		}
		else
		{
			nFailedCount++;
		}
		
	}
		
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_485DECODER;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_AlmCfg(LONG lLoginID, DEV_ALARM_SCHEDULE *pstAS, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0 || NULL == pstAS)
	{
		return NET_INVALID_HANDLE;
	}
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	DEV_ALARM_SCHEDULE *pDevAlarmCfg = pstAS;
	int i = 0;
	int buflen = 0;
	char *vbuf = NULL;
	if (buflen < CONFIG_ALARM_NUM*sizeof(CONFIG_ALARM))
	{
		buflen = CONFIG_ALARM_NUM*sizeof(CONFIG_ALARM);
	}
	if (buflen < CONFIG_DETECT_NUM*sizeof(CONFIG_DETECT))
	{
		buflen = CONFIG_DETECT_NUM*sizeof(CONFIG_DETECT);
	}
	if (buflen < sizeof(CONFIG_FTP_PROTO_SET))
	{
		buflen = sizeof(CONFIG_FTP_PROTO_SET);
	}
	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_RECORD;
	}
	memset((void *)vbuf, 0, buflen);
	
	//set alarm in configure
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF])
	{
		CONFIG_ALARM *pAlarmCfg = NULL;
		memset((void *)vbuf, 0, buflen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM, 0, vbuf, CONFIG_ALARM_NUM*sizeof(CONFIG_ALARM), &retlen, waittime);
		if (nRet >= 0 && retlen == CONFIG_ALARM_NUM*sizeof(CONFIG_ALARM))
		{
			pAlarmCfg = (CONFIG_ALARM *)vbuf;
			int j = 0;
			int iMinIndex = MAX_ALARM_IN_NUM <= CONFIG_ALARM_NUM ? MAX_ALARM_IN_NUM : CONFIG_ALARM_NUM;
			for (i = 0; i < iMinIndex; i++)
			{
				pAlarmCfg[i].AlarmMask = 0;
				for (j = 0; j < MAX_ALARM_OUT_NUM; j++)
				{
					if (pDevAlarmCfg->struLocalAlmIn[i].struHandle.byRelAlarmOut[j])
					{
						pAlarmCfg[i].AlarmMask |= (1<<j);
					}
				}
				pAlarmCfg[i].AlarmTipEn = (pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwActionFlag&ALARM_TIP)?1:0;
				pAlarmCfg[i].dbTourEn = (pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwActionFlag&ALARM_TOUR)?1:0;
				pAlarmCfg[i].Mailer = (pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwActionFlag&ALARM_MAIL)?1:0;
				pAlarmCfg[i].RecordMask = 0;
				for (j = 0; j < MAX_VIDEO_IN_NUM; j++)
				{
					if (pDevAlarmCfg->struLocalAlmIn[i].struHandle.byRecordChannel[j])
					{
						pAlarmCfg[i].RecordMask |= (1<<j);
					}
				}
				pAlarmCfg[i].SendAlarmMsg = (pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwActionFlag&ALARM_UPLOAD)?1:0;
				pAlarmCfg[i].SensorType = pDevAlarmCfg->struLocalAlmIn[i].byAlarmType;
				pAlarmCfg[i].TimeDelay = pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwDuration;
				pAlarmCfg[i].TimePreRec = pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwRecLatch;
				int iMinIndex = ALARM_TIME_SECT_NUM<ALARM_SECTS?ALARM_TIME_SECT_NUM:ALARM_SECTS; 
				pAlarmCfg[i].dwSectMask[j] = 0;
				for (j = 0; j < iMinIndex; j++)
				{
					if (pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwActionFlag&ALARM_OUT)
					{
						pAlarmCfg[i].dwSectMask[j] |= 0x02;
					}
					if (pDevAlarmCfg->struLocalAlmIn[i].struHandle.dwActionFlag&ALARM_TIP)
					{
						pAlarmCfg[i].dwSectMask[j] |= 0x01;
					}
				}
			}
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM, 0, vbuf, CONFIG_ALARM_NUM*sizeof(CONFIG_ALARM), waittime);
			if (nRet < 0)
			{
				nFailedCount++;
			}
			else
			{
				Sleep(3*SETUP_SLEEP);
			}
		}
		else
		{
			nFailedCount++;
		}
		
		ALARM_CONTROL tmpAlarmCfg[CONFIG_ALARM_NUM] = {0};
		int iAlarmIn = 0;
		nRet = QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, NULL, 0, &iAlarmIn, waittime);
		if (nRet >= 0)
		{
			iAlarmIn = Min(iAlarmIn, CONFIG_ALARM_NUM);
			for (i = 0; i < iAlarmIn; i++)
			{
				tmpAlarmCfg[i].index = i;
				tmpAlarmCfg[i].state = pDevAlarmCfg->struLocalAlmIn[i].byAlarmEn; //stAS.struLocalAlmIn[i].byAlarmEn;
			}
			nRet = IOControl(lLoginID, IOTYPE_ALARMINPUT, tmpAlarmCfg, iAlarmIn*sizeof(ALARM_CONTROL));
			if (nRet < 0)
			{
				nFailedCount++;
			}
			else
			{
				Sleep(SETUP_SLEEP);
			}
		}
		else
		{
			nFailedCount++;
		}
	}
	//set detect configure
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_VIDEOCONF])
	{
		CONFIG_DETECT *pDetCfg = NULL;
		memset((void *)vbuf, 0, buflen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, vbuf, CONFIG_DETECT_NUM*sizeof(CONFIG_DETECT), &retlen, waittime);
		if (nRet >=0 && retlen == CONFIG_DETECT_NUM*sizeof(CONFIG_DETECT))
		{
			int j = 0;
			pDetCfg = (CONFIG_DETECT *)vbuf;
			int iMinIndex = Min(MAX_VIDEO_IN_NUM,CONFIG_DETECT_NUM);
			for (i = 0; i < iMinIndex; i++)
			{
				pDetCfg[i].BlindAlarmMask = 0;
				pDetCfg[i].MotionAlarmMask = 0;
				pDetCfg[i].LossAlarmMask = 0;
				for (j = 0; j < MAX_ALARM_OUT_NUM; j++)
				{
					if (pDevAlarmCfg->struBlind[i].struHandle.byRelAlarmOut[j])
					{
						pDetCfg[i].BlindAlarmMask |= (1<<j);
					}
					if (pDevAlarmCfg->struMotion[i].struHandle.byRelAlarmOut[j])
					{
						pDetCfg[i].MotionAlarmMask |= (1<<j);
					}
					if (pDevAlarmCfg->struVideoLost[i].struHandle.byRelAlarmOut[j])
					{
						pDetCfg[i].LossAlarmMask |= (1<<j);
					}
				}
				pDetCfg[i].MotionRecordMask = 0;
				for (j = 0; j < MAX_VIDEO_IN_NUM; j++)
				{
					if (pDevAlarmCfg->struMotion[i].struHandle.byRecordChannel[j])
					{
						pDetCfg[i].MotionRecordMask |= (1<<j);
					}
				}
				pDetCfg[i].MotionTimeDelay = pDevAlarmCfg->struMotion[i].struHandle.dwDuration;
				pDetCfg[i].BlindTipEn = (pDevAlarmCfg->struBlind[i].struHandle.dwActionFlag&ALARM_TIP)?1:0;
				pDetCfg[i].LossTipEn = (pDevAlarmCfg->struVideoLost[i].struHandle.dwActionFlag&ALARM_TIP)?1:0;
				pDetCfg[i].Mailer = 0;
				if ((pDevAlarmCfg->struMotion[i].struHandle.dwActionFlag&ALARM_MAIL)
					|| (pDevAlarmCfg->struBlind[i].struHandle.dwActionFlag&ALARM_MAIL)
					|| (pDevAlarmCfg->struVideoLost[i].struHandle.dwActionFlag&ALARM_MAIL))
				{
					pDetCfg[i].Mailer = 1;
				}
				pDetCfg[i].MotionLever = pDevAlarmCfg->struMotion[i].wSenseLevel;
				
				//Begin: Add by zsc(11402)	2008-1-18
				int nMaxRow = Min(pDevAlarmCfg->struMotion[i].wMotionRow, 18);
				nMaxRow		= Min(nMaxRow, MOTION_ROW_NUM);
				int nMaxCol = Min(pDevAlarmCfg->struMotion[i].wMotionCol, 32);
				nMaxCol		= Min(nMaxCol, MOTION_COL_NUM);
				for (j = 0; j < nMaxRow; j++)
				{
					pDetCfg[i].MotionRegion[j] = 0;
					for (int k = 0; k < nMaxCol; k++)
					{
						if (pDevAlarmCfg->struMotion[i].byDetected[j][k])
						{
							pDetCfg[i].MotionRegion[j] |= (1<<k);
						}
					}
				}
				//End: zsc(11402)
				
				pDetCfg[i].SendAlarmMsg = 0;
				if ((pDevAlarmCfg->struMotion[i].struHandle.dwActionFlag&ALARM_UPLOAD)
					||(pDevAlarmCfg->struBlind[i].struHandle.dwActionFlag&ALARM_UPLOAD)
					||(pDevAlarmCfg->struVideoLost[i].struHandle.dwActionFlag&ALARM_UPLOAD))
				{
					pDetCfg[i].SendAlarmMsg = 1;
				}
			}
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_DETECT, 0, vbuf, CONFIG_DETECT_NUM*sizeof(CONFIG_DETECT), waittime);
			if (nRet < 0)
			{
				nFailedCount++;
			}
			else
			{
				Sleep(3*SETUP_SLEEP);
			}
		}
		else
		{
			nFailedCount++;
		}
		
	}
	
	/*
	//set ftp config
	CONFIG_FTP_PROTO_SET* pstFtpCfg = NULL;
	memset((void *)vbuf, 0, buflen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_FTP, 0, vbuf, sizeof(CONFIG_FTP_PROTO_SET), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_FTP_PROTO_SET))
	{
		pstFtpCfg = (CONFIG_FTP_PROTO_SET*)vbuf;
		for (int i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			for (int j = 0; j < DAYS_WEEK; j++)
			{
				for (int k=0; k < TIME_SECTION; k++)
				{
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartHour = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].iBeginHour;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartMin = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].iBeginMin;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartSec = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].iBeginSec;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.HourEnd = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].iHourEnd;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.EndMin = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].iEndMin;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.EndSec = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].iEndSec;
					
					BYTE tmp = pDevAlarmCfg->struLocalAlmIn[i].stSect[j][N_REC_TSECT - k-1].bEnable >> 16;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_MdEn = (tmp & 0x04) ? 1 : 0;
					pstFtpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_AlarmEn = (tmp & 0x08) ? 1 : 0;
				}
			}
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_FTP, 0, vbuf, sizeof(CONFIG_FTP_PROTO_SET), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(3*SETUP_SLEEP);
		}
	}
	else
	{
	//	nFailedCount++;
	}
	*/

	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_ALARMIN;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_Devtime(LONG lLoginID, NET_TIME *pstNT, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0 || NULL == pstNT)
	{
		return NET_INVALID_HANDLE;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	NET_TIME *pNetTime = pstNT;
	nRet = SetupDeviceTime(lLoginID, pNetTime);
	if (nRet < 0)
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("SetupConfig(SYS_TIME) failed!\n");				
#endif
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_SYSTIME;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_PreviewCfg(LONG lLoginID, DEV_PREVIEW_CFG *pstPVC, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0 || NULL == pstPVC)
	{
		return NET_INVALID_HANDLE;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	DEV_PREVIEW_CFG *pPreViewCfg = pstPVC;
	CONFIG_EX_CAPTURE_T stPreViewCfg = {0};
	stPreViewCfg.iQuality = pPreViewCfg->stPreView.byImageQlty;
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_PREVIEW, 0, (char *)&stPreViewCfg, sizeof(CONFIG_EX_CAPTURE_T), waittime);
	if (nRet < 0)
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("SetupConfig(PREVIEW) failed!\n");				
#endif
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_PREVIEW;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_AutoMtCfg(LONG lLoginID, DEV_AUTOMT_CFG *pstAMT, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0 || NULL == pstAMT)
	{
		return NET_INVALID_HANDLE;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	
	CONFIG_AUTO_NET stAutoNet = {0};
	stAutoNet.AutoDeleteFilesTime = pstAMT->byAutoDeleteFilesTime;
	stAutoNet.AutoRebootDay = pstAMT->byAutoRebootDay;
	stAutoNet.AutoRebootTime = pstAMT->byAutoRebootTime;

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_AUTO_MT, 0, (char *)&stAutoNet, sizeof(CONFIG_AUTO_NET), waittime);
	if (nRet < 0)
	{
		nFailedCount++;
#ifdef _DEBUG
		OutputDebugString("SetupConfig(AUTOMT) failed!\n");				
#endif
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_PREVIEW;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfig::SetDevConfig_VdMtrxCfg(LONG lLoginID, DEV_VIDEO_MATRIX_CFG *pstVM, int waittime)
{
	if (pstVM == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	int pos = 0;
	char buf[1024] = {0};
	for (int i = 0; i < pstVM->nMatrixNum; i++)
	{
		sprintf(buf+pos, "%d::%d::%d::%d&&", i, pstVM->struVideoGroup[i].dwChannelMask, 
			pstVM->struVideoGroup[i].bTourEnable?1:0, pstVM->struVideoGroup[i].nTourPeriod);
		pos = strlen(buf);
	}

	if (pos > 0)
	{
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_VIDEO_MATRIX, 1, buf, pos-2, waittime);
	}

	return nRet;
}

int	CDevConfig::SetDevConfig_MultiDdnsCfg(LONG lLoginID, DEV_MULTI_DDNS_CFG *pstMD, int waittime)
{
	if (NULL == pstMD)
	{
		return NET_ILLEGAL_PARAM;
	}

	int buflen = 400*10;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	retlen = buflen;
	//Begin: Modify by li_deming(11517) 2008-2-20
	//nRet = BuildMultiDdnsString(pstMD, tmpbuf, &retlen);
	afk_device_s* device = (afk_device_s*)lLoginID;
	DWORD dwSp = 0;
	device->get_info(device, dit_special_cap_flag, &dwSp);
	
	if (dwSp & SP_NEW_MULTIDDNSPRO)
	{
		nRet = BuildMultiDdnsString(pstMD, tmpbuf, &retlen, 1);
	}
	else
	{
		nRet = BuildMultiDdnsString(pstMD, tmpbuf, &retlen, 0);
	}
	//End:li_deming(11517)
	if (nRet < 0 || retlen <= 0)
	{
		return nRet;
	}

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_MULTI_DDNS, 0, tmpbuf, retlen, waittime);
	if (nRet)
	{
		nFailedCount++;
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_SnapCfg(LONG lLoginID, DEVICE_SNAP_CFG *pstSC, LONG lChnNum, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	int channum = device->channelcount(device);
	//hdvr
	if(device->device_type(device) == PRODUCT_DVR_MIX_DVR)
	{
		channum -= 4;//4个数字通道
	}

	if (!pstSC || lChnNum < channum)
	{
		return -1;
	}
	
	int retlen = 0;
	int nRet = -1;
	int nFailedCount = 0;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;

	int nRecNewNum = 16;
	if(lChnNum > 16)
	{
		nRecNewNum = lChnNum;
	}

	if (iRecvBufLen < 16 * sizeof(NEW_CONFIG_CAPTURE))//127#子命令
	{
		iRecvBufLen = 16 * sizeof(NEW_CONFIG_CAPTURE);
	}
	if (iRecvBufLen < nRecNewNum * sizeof(CONFIG_RECORD_NEW))//录象配置
	{
		iRecvBufLen = nRecNewNum * sizeof(CONFIG_RECORD_NEW);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//get record config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, cRecvBuf, nRecNewNum*sizeof(CONFIG_RECORD_NEW), &retlen, waittime);
	if (nRet >= 0 && retlen == nRecNewNum*sizeof(CONFIG_RECORD_NEW))
	{
		CONFIG_RECORD_NEW *stRecordNew = 0;
		for (int i = 0; i < channum; i++)
		{
			stRecordNew = (CONFIG_RECORD_NEW*)(cRecvBuf+i*sizeof(CONFIG_RECORD_NEW));
			stRecordNew->bSnapShot = pstSC[i].bTimingEnable;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, cRecvBuf, nRecNewNum*sizeof(CONFIG_RECORD_NEW), waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	else //if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		return NET_ERROR_GETCFG_RECORD;
	}
	
	//new capture config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && (retlen == 16 * sizeof(NEW_CONFIG_CAPTURE) || retlen == device->channelcount(device) * sizeof(NEW_CONFIG_CAPTURE)))
	{
		NEW_CONFIG_CAPTURE *stNewPic = 0;
		int i = 0;
		
		for (i = 0; i < channum; i++)
		{
			stNewPic = (NEW_CONFIG_CAPTURE *)(cRecvBuf + i*sizeof(NEW_CONFIG_CAPTURE));
			
			stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable = 0;
			if (pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byVideoEnable)
			{
				stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable |= 0x01;
			}
			if (pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byAudioEnable)
			{
				stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable |= 0x02;
			}
			if (pstSC[i].struSnapEnc[SNAP_TYP_TIMING].bAudioOverlay)
			{
				stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable |= 0x04;
			}
			stNewPic->SnapOption[SNAP_TYP_TIM_INTER].ImageSize = pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byImageSize;
			stNewPic->SnapOption[SNAP_TYP_TIM_INTER].BitRateControl = pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byBitRateControl;
			stNewPic->SnapOption[SNAP_TYP_TIM_INTER].Frames = pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byFramesPerSec;
			stNewPic->SnapOption[SNAP_TYP_TIM_INTER].ImgQlty = pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byImageQlty;
			
			stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable = 0;
			if (pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byVideoEnable)
			{
				stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable |= 0x01;
			}
			if (pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byAudioEnable)
			{
				stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable |= 0x02;
			}
			if (pstSC[i].struSnapEnc[SNAP_TYP_ALARM].bAudioOverlay)
			{
				stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable |= 0x04;
			}
			stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].ImageSize = pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byImageSize;
			stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].BitRateControl = pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byBitRateControl;
			stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].Frames = pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byFramesPerSec;
			stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].ImgQlty = pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byImageQlty;

			//增加抓图时间间隔2009.2.9 
			stNewPic->PicTimeInterval = pstSC[i].PicTimeInterval;//单位为s
			
		}//for (int i = 0; i < chanNum; i++)
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, cRecvBuf, iRecvBufLen, waittime);
		if (nRet < 0)
		{
			nFailedCount++;
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}	
	else
	{
		nRet = NET_ERROR_GETCFG_VIDEO; 
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_VIDEO;
	}
	else
	{
		nRet = NET_NOERROR;
	}

END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int	CDevConfig::SetDevConfig_UrlCfg(LONG lLoginID, DEVWEB_URL_CFG *pstUC, int waittime)
{
	if ( NULL == pstUC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int buflen = 500;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	
	retlen = buflen;
	nRet = BuildUrlCfgString(pstUC, tmpbuf, &retlen);
	if (nRet < 0 || retlen <= 0)
	{
		return nRet;
	}
	
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_WEB_URL, 0, tmpbuf, retlen, waittime);
	if (nRet)
	{
		nFailedCount++;
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	return nRet;
}

int	CDevConfig::SetDevConfig_FtpCfg(LONG lLoginID, DEVICE_FTP_PROTO_CFG *pstFC, int waittime)
{	
	if (NULL == pstFC)
	{
		return NET_ILLEGAL_PARAM;
	}
		
	int retlen = 0;
	int nRet = -1;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_FTP_PROTO_SET))
	{
		iRecvBufLen = sizeof(CONFIG_FTP_PROTO_SET);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//get ftp config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_FTP_PRO, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_FTP_PROTO_SET))
	{
		CONFIG_FTP_PROTO_SET *pftpCfg = (CONFIG_FTP_PROTO_SET*)cRecvBuf;
		pftpCfg->m_isEnable = pstFC->bEnable;
		pftpCfg->m_iFileLen = pstFC->iFileLen;
		pftpCfg->m_iInterval = pstFC->iInterval;
		
		int subLen = (strlen(pstFC->szUserName)>MAX_FTP_USERNAME_LEN) ? MAX_FTP_USERNAME_LEN : strlen(pstFC->szUserName);
		char szUTF8[MAX_FTP_USERNAME_LEN*2+2] = {0};
		Change_Assic_UTF8(pstFC->szUserName, subLen, szUTF8, MAX_FTP_USERNAME_LEN*2+2);
		memcpy(pftpCfg->m_cUserName, szUTF8, MAX_USERNAME_LEN-1);

		memcpy(pftpCfg->m_cPassword, pstFC->szPassword, MAX_PASSWORD_LEN-1);
		
		subLen = (strlen(pstFC->szDirName) > FTP_MAX_PATH-1) ?  FTP_MAX_PATH-1 : strlen(pstFC->szDirName);
		char szUTF8DirName[FTP_MAX_PATH*2+2] = {0};
		Change_Assic_UTF8(pstFC->szDirName, subLen, szUTF8DirName, FTP_MAX_PATH*2+2);
		Cut_UTF8_Tail((unsigned char *)szUTF8DirName, FTP_MAX_PATH-1);
		memcpy(pftpCfg->m_cDirName, szUTF8DirName, FTP_MAX_PATH-1);
		
		pftpCfg->m_nHostPort = pstFC->wHostPort;
		pftpCfg->m_unHostIP = inet_addr(pstFC->szHostIp);

		pftpCfg->m_protocol = pstFC->protocol;

		for (int i=0; i<MAX_CHANNEL_NUM; i++)
		{
			for (int j=0; j<DAYS_WEEK; j++)
			{
				for (int k=0; k<TIME_SECTION; k++)
				{
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartHour = pstFC->struUploadCfg[i][j].struPeriod[k].struSect.iBeginHour;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartMin = pstFC->struUploadCfg[i][j].struPeriod[k].struSect.iBeginMin;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartSec = pstFC->struUploadCfg[i][j].struPeriod[k].struSect.iBeginSec;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.HourEnd = pstFC->struUploadCfg[i][j].struPeriod[k].struSect.iHourEnd;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.EndMin = pstFC->struUploadCfg[i][j].struPeriod[k].struSect.iEndMin;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.EndSec = pstFC->struUploadCfg[i][j].struPeriod[k].struSect.iEndSec;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_AlarmEn = pstFC->struUploadCfg[i][j].struPeriod[k].bAlarmEn;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_MdEn = pstFC->struUploadCfg[i][j].struPeriod[k].bMdEn;
					pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_TimerEn = pstFC->struUploadCfg[i][j].struPeriod[k].bTimerEn;
				}
			}
		}

		//set ftp config
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_FTP_PRO, 0, cRecvBuf, iRecvBufLen, waittime);
		if (nRet < 0)
		{
#ifdef _DEBUG
			OutputDebugString("SetupConfig(CONFIG_TYPE_FTP_PRO) failed!\n");
#endif
			goto END;
		}
		else
		{
			Sleep(2*SETUP_SLEEP);
		}
	}
	else
	{
		nRet = -1;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int	CDevConfig::SetDevConfig_InterVedioCfg(LONG lLoginID, char* pCfg, int cfgLen, LONG lPlatform, int waittime)
{
	if (NULL == pCfg)
	{
		return NET_ILLEGAL_PARAM;
	}

	DEV_INTERVIDEO_UCOM_CFG* pstIUC = NULL;
	DEV_INTERVIDEO_NSS_CFG* pstNSS = NULL;
	DEV_INTERVIDEO_BELL_CFG* pstBLC = NULL;
	switch(lPlatform)
	{
	case 4:		//BELL
		pstBLC = (DEV_INTERVIDEO_BELL_CFG*)pCfg;
		break;
	case 10:	//中兴力维
		pstNSS = (DEV_INTERVIDEO_NSS_CFG*)pCfg;
		break;
	case 11:	//U com
		pstIUC = (DEV_INTERVIDEO_UCOM_CFG*)pCfg;
		break;
	default:
		return NET_ILLEGAL_PARAM;
		break;
	}
	
	int retlen = 0;
	int nRet = -1;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_INTERVIDEO_UCOM))
	{
		iRecvBufLen = sizeof(CONFIG_INTERVIDEO_UCOM);
	}
	if (iRecvBufLen < sizeof(DEV_INTERVIDEO_NSS_CFG))
	{
		iRecvBufLen = sizeof(DEV_INTERVIDEO_NSS_CFG);
	}
	if (iRecvBufLen < sizeof(WEB_VSP_CONFIG))
	{
		iRecvBufLen = sizeof(WEB_VSP_CONFIG);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//get intervedio config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_INTERVIDEO, lPlatform, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0)
	{
		switch(lPlatform)
		{
		case 4:		//BELL
			if (retlen == sizeof(WEB_VSP_CONFIG))
			{
				WEB_VSP_CONFIG* pANVCfg = (WEB_VSP_CONFIG*)cRecvBuf;
				//这里偷一下懒，直接拷贝
				memcpy(pANVCfg, pCfg+sizeof(DWORD), sizeof(WEB_VSP_CONFIG));
				
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_INTERVIDEO, lPlatform, cRecvBuf, sizeof(WEB_VSP_CONFIG), waittime);
				if (nRet < 0)
				{
#ifdef _DEBUG
					OutputDebugString("SetupConfig(CONFIG_TYPE_INTERVIDEO) failed!\n");
#endif
					goto END;
				}
				else
				{
					Sleep(2*SETUP_SLEEP);
				}
			}
			else
			{
				nRet = -1;
			}
			break;
		case 10:	//中兴力维
			if (retlen == sizeof(WEB_NSS_CONFIG))
			{
				WEB_NSS_CONFIG* pNSSCfg = (WEB_NSS_CONFIG*)cRecvBuf;
				//这里偷一下懒，直接拷贝
				memcpy(pNSSCfg, pCfg+sizeof(DWORD), sizeof(WEB_NSS_CONFIG));
				
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_INTERVIDEO, lPlatform, cRecvBuf, sizeof(WEB_NSS_CONFIG), waittime);
				if (nRet < 0)
				{
					goto END;
				}
				else
				{
					Sleep(2*SETUP_SLEEP);
				}
			}
			else
			{
				nRet = -1;
			}
			break;
		case 11:	//U com
			if (retlen == sizeof(CONFIG_INTERVIDEO_UCOM))
			{
				CONFIG_INTERVIDEO_UCOM *pIUCfg = (CONFIG_INTERVIDEO_UCOM*)cRecvBuf;
				pIUCfg->iAliveEnable = pstIUC->bAliveEnable;
				pIUCfg->iFuncEnable = pstIUC->bFuncEnable;
				pIUCfg->iAlivePeriod = pstIUC->dwAlivePeriod;
				memcpy(pIUCfg->strDeviceId, pstIUC->szDeviceId, 32-1);
				memcpy(pIUCfg->strPassWord, pstIUC->szPassWord, 32-1);
				memcpy(pIUCfg->strRegPwd, pstIUC->szRegPwd, 16-1);
				memcpy(pIUCfg->strUserName, pstIUC->szUserName, 32-1);
				pIUCfg->ServerIp = inet_addr(pstIUC->szServerIp);
				pIUCfg->ServerPort = pstIUC->wServerPort;
				
				for (int i=0; i<N_SYS_CH; i++)
				{
					pIUCfg->sChnInfo[i].iChnEn = pstIUC->struChnInfo[i].bChnEn;
					memcpy(pIUCfg->sChnInfo[i].strChnId, pstIUC->struChnInfo[i].szChnId, 32-1);
				}
				
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_INTERVIDEO, lPlatform, cRecvBuf, sizeof(CONFIG_INTERVIDEO_UCOM), waittime);
				if (nRet < 0)
				{
#ifdef _DEBUG
					OutputDebugString("SetupConfig(CONFIG_TYPE_INTERVIDEO) failed!\n");
#endif
					goto END;
				}
				else
				{
					Sleep(2*SETUP_SLEEP);
				}
			}
			else
			{
				nRet = -1;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		nRet = -1;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;

}

int CDevConfig::SetDevConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer,DWORD dwInBufferSize,int waittime)
{
	if (!lpInBuffer)
	{
		return -1;
	}
//	int retlen = 0; //data length
	int nRet = -1;  //return value
//	int nFailedCount = 0;	//在分次处理时，记录失败的次数
	
	afk_device_s* device = (afk_device_s*)lLoginID;
	
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}


	switch(dwCommand)
	{
	case DEV_DEVICECFG:				//设置设备参数
		{
			if (dwInBufferSize < sizeof(DEV_SYSTEM_ATTR_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_SYSTEM_ATTR_CFG *pDevSysAttrCfg = NULL;
			pDevSysAttrCfg = (DEV_SYSTEM_ATTR_CFG *)lpInBuffer;
			
			nRet = SetDevConfig_DevCfg(lLoginID, pDevSysAttrCfg, waittime);
		}
		break;
	case DEV_NETCFG:				//设置网络参数
		{
			if (dwInBufferSize < sizeof(DEVICE_NET_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_NET_CFG *pDevNetCfg = NULL;
			pDevNetCfg = (DEVICE_NET_CFG *)lpInBuffer;

			nRet = SetDevConfig_NetCfg(lLoginID, pDevNetCfg, waittime);
		}
		break;
	case DEV_CHANNELCFG:				//设置图象参数
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				afk_device_s* device = (afk_device_s*)lLoginID;
				if (!device || m_pManager->IsDeviceValid(device) < 0)
				{
					return NET_INVALID_HANDLE;
				}
				int iChanNum = device->channelcount(device);

				if (lChannel < -1 /*|| lChannel >= MAX_CHANNUM*/ || lChannel >= iChanNum)
				{
					return NET_ILLEGAL_PARAM;
				}
				else if (-1 == lChannel)
				{
					if (dwInBufferSize < iChanNum * sizeof(DEV_CHANNEL_CFG))
					{
						if (dwInBufferSize < 16 * sizeof(DEV_CHANNEL_CFG)) 
						{
							return NET_ILLEGAL_PARAM;
						}
					}
				}
				else
				{
					if (dwInBufferSize < sizeof(DEV_CHANNEL_CFG))
					{
						return NET_ILLEGAL_PARAM;
					}
				}
				DEV_CHANNEL_CFG *pDevChnCfg = (DEV_CHANNEL_CFG *)lpInBuffer;
				
				nRet = SetDevConfig_ChnCfg(lLoginID, pDevChnCfg, lChannel, waittime);
				if (iChanNum >= 16) //只保存通道号
				{
					DEV_CHANNEL_CFG *pTmpChaCfg = (DEV_CHANNEL_CFG *)lpInBuffer;
					char *buf = new char[iChanNum*64+32];
					if (!buf)
					{
						return NET_SYSTEM_ERROR;
					}

					int iChanCount = 1;
					int iChanIdx = lChannel;
					
					if (-1 == lChannel)
					{
						iChanIdx = 0;
						iChanCount = iChanNum;
					}
					
					memset(buf, 0, iChanNum*64+32);//支持新通道名协议
					int retlen = 0;
					nRet = QueryChannelName(lLoginID, buf, iChanNum*64+32, &retlen, waittime);
					if (nRet >= 0)
					{	
						for (int i = 0; i < iChanCount; i++)
						{
							int iCopyStrLen = strlen(pTmpChaCfg->szChannelName)<sizeof(pTmpChaCfg->szChannelName)?strlen(pTmpChaCfg->szChannelName):sizeof(pTmpChaCfg->szChannelName);
							if (iCopyStrLen > 0)
							{
								memset((void *)(buf+iChanIdx*CHANNEL_NAME_LEN), 0, CHANNEL_NAME_LEN);
								memcpy((void *)(buf+iChanIdx*CHANNEL_NAME_LEN), pTmpChaCfg->szChannelName, iCopyStrLen);
							}
							
							pTmpChaCfg ++;
							iChanIdx ++;
							
						}
						
						nRet = SetupChannelName(lLoginID, buf, iChanNum*CHANNEL_NAME_LEN);
						if (nRet < 0)
						{
							nRet = NET_ERROR_SETCFG_VIDEO;
						}
						
						delete []buf;
					}
					else
					{
						delete []buf;
						nRet =  NET_ERROR_SETCFG_VIDEO;	
					}	
				}	
			}
		}
		break;
	case DEV_RECORDCFG:				//设置录像时间参数
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				afk_device_s* device = (afk_device_s*)lLoginID;
				if (!device)
				{
					return NET_INVALID_HANDLE;
				}
				int iChanNum = device->channelcount(device);
				int iChanCount = 1;
				
				if (lChannel < -1 /*|| lChannel >= MAX_CHANNUM*/)
				{
					return NET_ILLEGAL_PARAM;
				}
				else if (-1 == lChannel)
				{
					iChanCount = iChanNum;
					if (dwInBufferSize < iChanNum * sizeof(DEV_RECORD_CFG))
					{
						return NET_ILLEGAL_PARAM;
					}
				}
				else
				{
					if (dwInBufferSize < sizeof(DEV_RECORD_CFG))
					{
						return NET_ILLEGAL_PARAM;
					}
				}
				
				DEV_RECORD_CFG *pDevRecCfg = (DEV_RECORD_CFG *)lpInBuffer;
				
				int iProtocolVersion;
				device->get_info(device, dit_protocol_version, &iProtocolVersion);
				if (iProtocolVersion > 2)
				{
					nRet = SetDevConfig_RecCfgNew(lLoginID, pDevRecCfg, lChannel, waittime);
				}
				else
				{
					nRet = SetDevConfig_RecCfg(lLoginID, pDevRecCfg, lChannel, waittime);
				}
			}
		}
		break;
	case DEV_COMMCFG:				//设置串口参数
		{
			if (dwInBufferSize < sizeof(DEV_COMM_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_COMM_CFG *pDevComCfg = (DEV_COMM_CFG *)lpInBuffer;
			nRet = SetDevConfig_ComCfg(lLoginID, pDevComCfg, waittime);
		}
		break;
	case DEV_ALARMCFG:				//设置报警布防参数
		{
			if (dwInBufferSize < sizeof(DEV_ALARM_SCHEDULE))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_ALARM_SCHEDULE *pDevAlmCfg = (DEV_ALARM_SCHEDULE *)lpInBuffer;
			int iProtocolVersion;
			device->get_info(device, dit_protocol_version, &iProtocolVersion);
			if (iProtocolVersion > 2)
			{
				nRet = SetDevConfig_AlmCfgNew(lLoginID, *pDevAlmCfg, waittime);
			}
			else
			{
				nRet = SetDevConfig_AlmCfg(lLoginID, pDevAlmCfg, waittime);
			}
		}
		break;
	case DEV_TIMECFG:				//设置DVR时间
		{
			if (dwInBufferSize < sizeof(NET_TIME))
			{
				return NET_ILLEGAL_PARAM;
			}
			NET_TIME *pNetTime = (NET_TIME *)lpInBuffer;
			nRet = SetDevConfig_Devtime(lLoginID, pNetTime, waittime);
		}
		break;
	case DEV_PREVIEWCFG:			//设置预览参数
		{
			if (dwInBufferSize < sizeof(DEV_PREVIEW_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_PREVIEW_CFG *pPreVCfg = (DEV_PREVIEW_CFG *)lpInBuffer;
			nRet = SetDevConfig_PreviewCfg(lLoginID, pPreVCfg, waittime);	
		}
		break;
	case DEV_POS_CFG:
		{
			if (dwInBufferSize < sizeof(DEVICE_POS_INFO))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_POS_INFO *pPosCfg = (DEVICE_POS_INFO *)lpInBuffer;
			nRet = SetDevConfig_PosCfg(lLoginID, pPosCfg, waittime);	
		}
		break ;
	case DEV_AUTOMTCFG:		//自动维护配置
		{
			if (dwInBufferSize < sizeof(DEV_AUTOMT_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_AUTOMT_CFG *pAmtCfg = (DEV_AUTOMT_CFG *)lpInBuffer;
			nRet = SetDevConfig_AutoMtCfg(lLoginID, pAmtCfg, waittime);	
		}
		break;
	case DEV_VEDIO_MARTIX:
		{
			if (dwInBufferSize < sizeof(DEV_VIDEO_MATRIX_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_VIDEO_MATRIX_CFG *pVdMtrx = (DEV_VIDEO_MATRIX_CFG*)lpInBuffer;
			nRet = SetDevConfig_VdMtrxCfg(lLoginID, pVdMtrx, waittime);
		}
		break;
	case DEV_MULTI_DDNS:
		{
			if (dwInBufferSize < sizeof(DEV_MULTI_DDNS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_MULTI_DDNS_CFG *pMd = (DEV_MULTI_DDNS_CFG*)lpInBuffer;
			nRet = SetDevConfig_MultiDdnsCfg(lLoginID, pMd, waittime);
		}
		break;
	case DEV_SNAP_CFG:
		{
			afk_device_s* device = (afk_device_s*)lLoginID;
			if (!device)
			{
				return NET_INVALID_HANDLE;
			}

			int channum = device->channelcount(device);
			if(device->device_type(device) == PRODUCT_DVR_MIX_DVR)
			{
				channum -= 4;//4个数字通道
			}
			if (dwInBufferSize < sizeof(DEVICE_SNAP_CFG)*channum)
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_SNAP_CFG* pSC = (DEVICE_SNAP_CFG*)lpInBuffer;
			nRet = SetDevConfig_SnapCfg(lLoginID, pSC, device->channelcount(device), waittime);
		}
		break;
	case DEV_WEB_URL_CFG:
		{
			if (dwInBufferSize < sizeof(DEVWEB_URL_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVWEB_URL_CFG *pUC = (DEVWEB_URL_CFG*)lpInBuffer;
			nRet = SetDevConfig_UrlCfg(lLoginID, pUC, waittime);
		}
		break;
	case DEV_FTP_PROTO_CFG:
		{
			if (dwInBufferSize < sizeof(DEVICE_FTP_PROTO_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_FTP_PROTO_CFG *pstFC = (DEVICE_FTP_PROTO_CFG*)lpInBuffer;
			nRet = SetDevConfig_FtpCfg(lLoginID, pstFC, waittime);
		}
		break;
	case DEV_INTERVIDEO_CFG:
		{
			switch(lChannel)
			{
			case 4:		//贝尔
				if (dwInBufferSize < sizeof(DEV_INTERVIDEO_BELL_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				break;
			case 10:	//中兴力维
				if (dwInBufferSize < sizeof(DEV_INTERVIDEO_NSS_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				break;
			case 11:	//U网通
				if (dwInBufferSize < sizeof(DEV_INTERVIDEO_UCOM_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				break;
			default:
				{
					//不支持该配置
					return NET_ILLEGAL_PARAM;
				}
				break;
			}
			
			nRet = SetDevConfig_InterVedioCfg(lLoginID, (char*)lpInBuffer, dwInBufferSize, lChannel, waittime);
		}
		break;
	//Add: by zsc(11402) 2008-1-18
	case DEV_VIDEO_COVER:				//区域遮挡配置
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				afk_device_s* device = (afk_device_s*)lLoginID;
				if (!device || m_pManager->IsDeviceValid(device) < 0)
				{
					return NET_INVALID_HANDLE;
				}
				int iChanNum = device->channelcount(device);
				iChanNum = Min(iChanNum,MAX_VIDEO_IN_NUM);
				iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM);
				int iChanCount = 1;
				if (lChannel < -1 || lChannel >= MAX_CHANNUM)
				{
					return NET_ILLEGAL_PARAM;
				}
				
				if (-1 == lChannel)
				{
					if (dwInBufferSize < iChanNum * sizeof(DEV_VIDEOCOVER_CFG))
					{
						return NET_ILLEGAL_PARAM;
					}
					
					for(int i=0; i<iChanNum; i++)
					{
						DEV_VIDEOCOVER_CFG *pChnCoverCfg = (DEV_VIDEOCOVER_CFG *)((char *)lpInBuffer + i*sizeof(DEV_VIDEOCOVER_CFG));
						nRet = SetDevConfig_ChnCoverCfg(lLoginID, pChnCoverCfg, i, waittime);
						if(nRet < 0)
						{
							break;
						}
					}
				}
				else
				{
					if (dwInBufferSize < sizeof(DEV_VIDEOCOVER_CFG))
					{
						return NET_ILLEGAL_PARAM;
					}
					
					DEV_VIDEOCOVER_CFG *pChnCoverCfg = (DEV_VIDEOCOVER_CFG *)lpInBuffer;
					nRet = SetDevConfig_ChnCoverCfg(lLoginID, pChnCoverCfg, lChannel, waittime);
				}
			}
		}
		break;
	//End: zsc(11402)

	//Add: by ljy(10858) 2008-3-11
	case DEV_TRANS_STRATEGY:
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				if (dwInBufferSize < sizeof(DEV_TRANSFER_STRATEGY_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				
				DEV_TRANSFER_STRATEGY_CFG *pstTSC = (DEV_TRANSFER_STRATEGY_CFG*)lpInBuffer;
				nRet = SetDevConfig_TransStrategyCfg(lLoginID, pstTSC, waittime);
			}
		}
		break;
	//End: ljy(10858)

	//Add: by ljy(10858) 2008-3-24
	case DEV_DOWNLOAD_STRATEGY:
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				if (dwInBufferSize < sizeof(DEV_DOWNLOAD_STRATEGY_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				
				DEV_DOWNLOAD_STRATEGY_CFG *pstDSC = (DEV_DOWNLOAD_STRATEGY_CFG*)lpInBuffer;
				nRet = SetDevConfig_DownloadStrategyCfg(lLoginID, pstDSC, waittime);
			}
		}
		break;
	//End: ljy(10858)

	//Add: by zsc(11402) 2008-4-8
	case DEV_WLAN_CFG:				//无线网络配置
		{
			if (dwInBufferSize < sizeof(DEV_WLAN_INFO))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_WLAN_INFO *pWlanCfg = NULL;
			pWlanCfg = (DEV_WLAN_INFO *)lpInBuffer;

			nRet = SetDevConfig_WLANCfg(lLoginID, pWlanCfg, waittime);
		}
		break;
	case DEV_WLAN_DEVICE_CFG:		//无线设备配置
		{
			if (dwInBufferSize < sizeof(DEV_WLAN_DEVICE))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_WLAN_DEVICE *pWlanDev = NULL;
			pWlanDev = (DEV_WLAN_DEVICE *)lpInBuffer;

			nRet = SetDevConfig_WLANDevCfg(lLoginID, pWlanDev, waittime);
		}
		break;
	case DEV_REGISTER_CFG:			//主动注册参数配置
		{
			if (dwInBufferSize < sizeof(DEV_REGISTER_SERVER))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = SetDevConfig_AutoRegisterCfg(lLoginID, (DEV_REGISTER_SERVER*)lpInBuffer, waittime);
		}
		break;
	case DEV_CAMERA_CFG:				//摄像头属性配置
		{
			afk_device_s* device = (afk_device_s*)lLoginID;
			if (!device || m_pManager->IsDeviceValid(device) < 0)
			{
				return NET_INVALID_HANDLE;
			}

			int iChanNum = device->channelcount(device);
			iChanNum = Min(iChanNum,MAX_VIDEO_IN_NUM);
			iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM);
			int iChanCount = 1;
			if (lChannel < -1 || lChannel >= MAX_CHANNUM)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				iChanCount = iChanNum;
				if (dwInBufferSize < iChanNum * sizeof(DEV_CAMERA_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(DEV_CAMERA_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			DEVICE_CAMERA_CFG *pDevChnCfg = (DEVICE_CAMERA_CFG *)lpInBuffer;

			nRet = SetDevConfig_ChnCmrCfg(lLoginID, pDevChnCfg, lChannel, waittime);
		}
		break;
	//End: zsc(11402)

	//Add: by zsc(11402) 2008-4-17
	case DEV_WATERMAKE_CFG:
		{
			afk_device_s* device = (afk_device_s*)lLoginID;
			if (!device || m_pManager->IsDeviceValid(device) < 0)
			{
				return NET_INVALID_HANDLE;
			}
			int iChanNum = device->channelcount(device);
			iChanNum = Min(iChanNum,MAX_VIDEO_IN_NUM);
			iChanNum = Min(iChanNum, CONFIG_CAPTURE_NUM);
			if (lChannel < -1 || lChannel >= MAX_CHANNUM)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				if (dwInBufferSize < iChanNum * sizeof(DEVICE_WATERMAKE_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}

				DEVICE_WATERMAKE_CFG *pChnWaterCfg = (DEVICE_WATERMAKE_CFG *)lpInBuffer;
				nRet = SetDevConfig_AllWaterMakeCfg(lLoginID, pChnWaterCfg, iChanNum, waittime);
			}
			else
			{
				if (dwInBufferSize < sizeof(DEVICE_WATERMAKE_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}

				DEVICE_WATERMAKE_CFG *pChnWaterCfg = (DEVICE_WATERMAKE_CFG *)lpInBuffer;
				nRet = SetDevConfig_WaterMakeCfg(lLoginID, pChnWaterCfg, lChannel, waittime);
			}
		}
		break;
	case DEV_INFRARED_CFG:			//红外报警参数
		{
			if (dwInBufferSize < sizeof(INFRARED_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			INFRARED_CFG *pInfraredCfg = NULL;
			pInfraredCfg = (INFRARED_CFG*)lpInBuffer;

			nRet = SetDevConfig_InfraredCfg(lLoginID, pInfraredCfg, waittime);
		}
		break;
	case DEV_DNS_CFG:		//DNS服务器配置
		{
			if (dwInBufferSize < sizeof(DEVICE_DNS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_DNS_CFG *pDnsCfg = NULL;
			pDnsCfg = (DEVICE_DNS_CFG*)lpInBuffer;

			nRet = SetDevConfig_DNSCfg(lLoginID, pDnsCfg, waittime);
		}
		break;
	case DEV_NTP_CFG:		//NTP配置
		{
			if (dwInBufferSize < sizeof(DEVICE_NTP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_NTP_CFG *pstNTP = NULL;
			pstNTP = (DEVICE_NTP_CFG*)lpInBuffer;

			nRet = SetDevConfig_NTPCfg(lLoginID, pstNTP, waittime);
		}
		break;
	//End: zsc(11402)

	//Add: by cqs(10842) 2008-5-8
	case DEV_SNIFFER_CFG:
		{
			if (dwInBufferSize < sizeof(DEVICE_SNIFFER_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
				
			DEVICE_SNIFFER_CFG *pSnifferCfg = (DEVICE_SNIFFER_CFG *)lpInBuffer;
			nRet = SetDevConfig_SnifferCfg(lLoginID, pSnifferCfg, waittime);
		}
		break;
	case DEV_AUDIO_DETECT_CFG:
		{
			if (dwInBufferSize < sizeof(AUDIO_DETECT_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			AUDIO_DETECT_CFG *pAudioDetectCfg = (AUDIO_DETECT_CFG *)lpInBuffer;
			nRet = SetDevConfig_AudioDetectCfg(lLoginID, pAudioDetectCfg, waittime);
		}
		break;
	case DEV_STORAGE_STATION_CFG:
		{
			afk_device_s* device = (afk_device_s*)lLoginID;
			if (!device || m_pManager->IsDeviceValid(device) < 0)
			{
				return NET_INVALID_HANDLE;
			}
			int iChanNum = device->channelcount(device);

			if (lChannel < -1 || lChannel >= iChanNum)
			{
				return NET_ILLEGAL_PARAM;
			}
			else if (-1 == lChannel)
			{
				if (dwInBufferSize < iChanNum * sizeof(STORAGE_STATION_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			else
			{
				if (dwInBufferSize < sizeof(STORAGE_STATION_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
			}
			STORAGE_STATION_CFG *pDevStorageCfg = (STORAGE_STATION_CFG *)lpInBuffer;
			
			nRet = SetDevConfig_StorageStateCfg(lLoginID, pDevStorageCfg, lChannel, waittime);
		}
		break;
	case DEV_DST_CFG: //夏令时
		{
			if (dwInBufferSize < sizeof(DEVICE_DST_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_DST_CFG *pDSTCfg = (DEVICE_DST_CFG *)lpInBuffer;

			nRet = SetDevConfig_DSTCfg(lLoginID, pDSTCfg, waittime);		
		}
		break;
	case DEV_VIDEO_OSD_CFG:
		{
			int ChannelNum = device->channelcount(device);
			if ((lChannel >=0 && lChannel < ChannelNum && dwInBufferSize < sizeof(DVR_VIDEO_OSD_CFG)) ||
				(lChannel == -1 && dwInBufferSize < ChannelNum*sizeof(DVR_VIDEO_OSD_CFG)) ||
				(lChannel >= ChannelNum))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DVR_VIDEO_OSD_CFG *pVideoOSDCfg = (DVR_VIDEO_OSD_CFG *)lpInBuffer;
			
			nRet = SetDevConfig_VideoOSDCfg(lLoginID, pVideoOSDCfg, lChannel, waittime);		
		}
		break;
		
	case DEV_CDMAGPRS_CFG: //GPRS/CDMA
		{
			if (dwInBufferSize < sizeof(DEVICE_CDMAGPRS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_CDMAGPRS_CFG *pGPRSCDMACfg = (DEVICE_CDMAGPRS_CFG *)lpInBuffer;
			
			nRet = SetDevConfig_GPRSCDMACfg(lLoginID, pGPRSCDMACfg, waittime);		
		}
		break;
		//End: cqs(10842)
	case DEV_MAIL_CFG:				//邮件配置
		{
			 if (dwInBufferSize < sizeof(DEVICE_MAIL_CFG))
			 {
				 return NET_ILLEGAL_PARAM;
			 }
			 
			 DEVICE_MAIL_CFG *pMailCfg = NULL;
			 pMailCfg = (DEVICE_MAIL_CFG*)lpInBuffer;
			 nRet = SetDevConfig_MailCfg(lLoginID,pMailCfg,waittime);
		 }
		 break;
	 case DEV_ALARM_CENTER_CFG:		//报警中心附加的配置
		 {
			if(dwInBufferSize < sizeof(ALARMCENTER_UP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			ALARMCENTER_UP_CFG *pAlarmCenterCfg = NULL;
			pAlarmCenterCfg = (ALARMCENTER_UP_CFG *)lpInBuffer;
			nRet = SetDevConfig_AlarmCenterCfg(lLoginID, pAlarmCenterCfg, waittime);
		 }
		 break;
	default:
		break;
	}

	return nRet;
}

//function for computing masked list  -Added by Linjy 2007-1-5
/*int GetMaskedList(NAME_NODE *des, char **res, BYTE *deslen, int maxnum, DWORD mask, BYTE *maskedIdx, BYTE realIdx)
{
	if (maxnum > 32 || !deslen || !maskedIdx)
	{
		//illegal parameters
		return -1;
	}

	int desIdx = 0;		//index of the output NAME_NODE array
	int maskedIndex = realIdx;	//store the masked array index
	for (int i = 0; i<maxnum; i++)
	{
		if ((mask >> i)&0x01)
		{
			//this bit is 1
			if (strlen(res[i]) > MAX_NAME_LEN)
			{
				//string overlength 
				return -1;
			}
			strcpy(des[desIdx].name, res[i]);
			des[desIdx].index = desIdx;
			desIdx++;
		}
		else
		{
			//this bit is 0
			if (realIdx == i) 
			{
				//invalid index
				return -1;
			}
			else if (realIdx > i)
			{
				maskedIndex--;
			}
		}
	}
	
	*deslen = desIdx;
	*maskedIdx = maskedIndex;
	return 0;
}
*/

//parse DDNS and PPPoE data strings
/////////////parse returned userinfo///////////////////
int ParseHostString(int hostType, char *buf, int bufLen, void *des, char *subparam, LONG lLoginID)
{
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int itemIdx = 0;	//Stores item indexs
	char tmp[6] = {0};

	REMOTE_HOST_CFG *struHost = (REMOTE_HOST_CFG *)des;
	if (!struHost)
	{
		goto e_out;
	}
	
	if (!des || !buf || !subparam)
	{
		goto e_out;
	}

	//4 header bits for enable, 0 stands enabled!!
	struHost->byEnable = (*(DWORD*)buf)==0;
	startPos = 4;
	pos = 4;

	while (pos < bufLen) 
	{
		if (buf[pos] != '&')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (buf[pos+1] != '&')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				//Parse the item
				switch(itemIdx)
				{
				case 0:	
					if (subLen >= MAX_HOSTNAME_LEN)
					{
						goto e_out;
					}

					unsigned char szUserName[MAX_HOSTNAME_LEN+1];
					memset(szUserName, 0, MAX_HOSTNAME_LEN+1);
					memcpy(szUserName, buf+startPos, subLen);

					switch(hostType)
					{
					case 0: //user name - for PPPoE
						Change_Utf8_Assic(szUserName, struHost->sHostUser);
						struHost->sHostUser[MAX_HOSTNAME_LEN-1] = '\0';
						break;
					case 1: //host name -for DDNS 
						Change_Utf8_Assic(szUserName,subparam);
						subparam[MAX_HOSTNAME_LEN-1] = '\0';
						//memcpy(subparam, buf+startPos, subLen);
						break;
					default:
						goto e_out;
					}
					
					break;
				case 1:	//user password - for DDNS
					if (subLen >= MAX_HOSTPSW_LEN)
					{
						goto e_out;
					}
					memcpy(struHost->sHostPassword, buf+startPos, subLen);
					break;
				case 2: //host ip
					{
						switch(hostType)
						{
						case 0: //PPPoE, host ip
							if (subLen >= MAX_HOSTPSW_LEN)
							{
								goto e_out;
							}
							memcpy(subparam, buf+startPos, subLen);
							break;
						case 1: //host ip -for DDNS 
							if (subLen >= MAX_IP_ADDRESS_LEN) 
							{
								goto e_out;
							}
							memcpy(struHost->sHostIPAddr, buf+startPos, subLen);
							break;
						default:
							goto e_out;
						}	
					}
					break;
				case 3://pppoe ether
					{
						switch(hostType)
						{
						case 0: //ether - for PPPoE
							{	
								char EtherName[64] = {0};
								if (subLen >= 64)
								{
									goto e_out;
								}
								memcpy(EtherName, buf+startPos, subLen);
								//取网卡信息 把网卡名称用有线和无线网卡标识
								CONFIG_ETHERNET stEthernet[5] = {0};
								afk_device_s *device = (afk_device_s*)lLoginID;
								device->get_info(device, dit_ether_info, &stEthernet);
								struHost->byAssistant = 0;
								for (int i = 0; i < MAX_ETHERNET_NUM; i++)
								{
									if (_stricmp(EtherName, stEthernet[i].szEthernetName) == 0)
									{
										struHost->byAssistant = i;
									}
								}	
							}
							break;
						default:
							break;
						}
					}
					break;
				default:
					goto e_out;
				}
				pos += 2;
				startPos = pos;
				subLen = 0;
				itemIdx++;
			}
		}
	}
	
	if (pos != 0) //the last item
	{
		switch(hostType)
		{
 		case 0: //PPPoE, host ip
			if (itemIdx == 2) //兼容以前老的pppoe ip
			{
				if (subLen >= MAX_HOSTPSW_LEN)
				{
					goto e_out;
				}
				memcpy(subparam, buf+startPos, subLen);
			}
			break;
		case 1: //DDNS, host port
			if (subLen >= 6) //port should be lower than 65535
			{
				goto e_out;
			}
			memset(tmp, 0, 6);
			memcpy(tmp, buf + startPos, subLen);
			struHost->wHostPort = (WORD)atoi(tmp);
			break;
		default:
			break;
		}
	}

	return 0;
e_out:
	return -1;
}

/*
 *	下面的函数说明：
 *		iHostType区别PPPOE(0)和DDNS(1)；
 */
int DecodeHostString(int iHostType, void *vHostStru, char *szSubParam, char *szOutBuf, int iMaxOutBufLen, LONG lLoginID)
{
	if (NULL == szOutBuf || 0 == iMaxOutBufLen || NULL == vHostStru)
	{
		return 0;
	}

	REMOTE_HOST_CFG *pDevRemotHost = (REMOTE_HOST_CFG *)vHostStru;
	int iCurPos = 0;
	int iNeedBufLen = 0;
	int iCopyStrLen = 0;

	memset(szOutBuf, 0, iMaxOutBufLen);
	iNeedBufLen = iCurPos + 4;//开始的四个字节表示使能，其实也是值的用法，不是字符串
	if (iMaxOutBufLen < iNeedBufLen)
	{
		goto FAILED;
	}
	if (pDevRemotHost->byEnable)
	{
		*(int *)(szOutBuf+iCurPos) = 0;
	}
	else
	{
		*(int *)(szOutBuf+iCurPos) = 1;
	}
	iCurPos += 4;

	/*
	 *	下面的字符串的追加就算没有长度的考虑也不能用strcat，因为开头的四个字节中已经0了，目标字符串不是纯的字符串
	 */
	switch (iHostType)
	{
	case 1:
		{
			iCopyStrLen = strlen(szSubParam);
			iNeedBufLen = iCurPos + iCopyStrLen + 2;//这里还需要加“&&”
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			memcpy(szOutBuf+iCurPos, szSubParam, iCopyStrLen);
			iCurPos += iCopyStrLen;
			memcpy(szOutBuf+iCurPos, "&&", 2);
			iCurPos += 2;

			iCopyStrLen = strlen(pDevRemotHost->sHostUser)<sizeof(pDevRemotHost->sHostUser)?strlen(pDevRemotHost->sHostUser):sizeof(pDevRemotHost->sHostUser);
			iNeedBufLen = iCurPos + iCopyStrLen + 2;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			memcpy(szOutBuf+iCurPos, pDevRemotHost->sHostUser, iCopyStrLen);
			iCurPos += iCopyStrLen;
			memcpy(szOutBuf+iCurPos, "&&", 2);
			iCurPos += 2;
			
			/*
			iCopyStrLen = strlen(pDevRemotHost->sHostPassword)<sizeof(pDevRemotHost->sHostPassword)?strlen(pDevRemotHost->sHostPassword):sizeof(pDevRemotHost->sHostPassword);
			iNeedBufLen = iCurPos + iCopyStrLen + 2;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			
			memcpy(szOutBuf+iCurPos, pDevRemotHost->sHostPassword, iCopyStrLen);
			iCurPos += iCopyStrLen;
			memcpy(szOutBuf+iCurPos, "&&", 2);
			iCurPos += 2;
			*/
			iCopyStrLen = strlen(pDevRemotHost->sHostIPAddr)<sizeof(pDevRemotHost->sHostIPAddr)?strlen(pDevRemotHost->sHostIPAddr):sizeof(pDevRemotHost->sHostIPAddr);
			iNeedBufLen = iCurPos + iCopyStrLen + 2;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			memcpy(szOutBuf+iCurPos, pDevRemotHost->sHostIPAddr, iCopyStrLen);
			iCurPos += iCopyStrLen;
			memcpy(szOutBuf+iCurPos, "&&", 2);
			iCurPos += 2;

			iNeedBufLen = iCurPos + 6;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			char *pBuf = itoa(pDevRemotHost->wHostPort, szOutBuf+iCurPos, 10);
			iCurPos += strlen(pBuf);
		}
		break;
	case 0:
		{
			iCopyStrLen = strlen(pDevRemotHost->sHostUser)<sizeof(pDevRemotHost->sHostUser)?strlen(pDevRemotHost->sHostUser):sizeof(pDevRemotHost->sHostUser);
			iNeedBufLen = iCurPos + iCopyStrLen + 2;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}

			char szUTF8[MAX_HOSTNAME_LEN*2+2] = {0};
			Change_Assic_UTF8(pDevRemotHost->sHostUser, iCopyStrLen, szUTF8, MAX_HOSTNAME_LEN*2+2);
			iCopyStrLen = strlen(szUTF8);
			memcpy(szOutBuf+iCurPos, szUTF8, iCopyStrLen);
			iCurPos += iCopyStrLen;
			memcpy(szOutBuf+iCurPos, "&&", 2);
			iCurPos += 2;
			
			iCopyStrLen = strlen(pDevRemotHost->sHostPassword)<sizeof(pDevRemotHost->sHostPassword)?strlen(pDevRemotHost->sHostPassword):sizeof(pDevRemotHost->sHostPassword);
			iNeedBufLen = iCurPos + iCopyStrLen;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			memcpy(szOutBuf+iCurPos, pDevRemotHost->sHostPassword, iCopyStrLen);
			iCurPos += iCopyStrLen;

			//add by cqs  增加ip和网卡名称,为了保持协议的统一，ip这项不传内容，但是分割符要保留
			memcpy(szOutBuf+iCurPos, "&&&&", 4);
			iCurPos += 4;
			//取网卡信息
			char EtherName[64] = {0};
			CONFIG_ETHERNET stEthernet[5] = {0};
			afk_device_s *device = (afk_device_s*)lLoginID;
			device->get_info(device, dit_ether_info, &stEthernet);
			if (1 == pDevRemotHost->byAssistant)//无线网卡
			{
				strcpy(EtherName, stEthernet[1].szEthernetName);
			}
			else //有线网卡
			{
				strcpy(EtherName, stEthernet[0].szEthernetName);
			}
			
			iCopyStrLen = strlen(EtherName)<sizeof(EtherName)?strlen(EtherName):sizeof(EtherName);
			iNeedBufLen = iCurPos + iCopyStrLen;
			if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
			{
				goto FAILED;
			}
			memcpy(szOutBuf+iCurPos, EtherName, iCopyStrLen);
			iCurPos += iCopyStrLen;
		}
		break;
	default:
		break;
	}
	
	return iCurPos;
FAILED:
	return 0;
}
//parse mail config data strings
/////////////parse returned userinfo///////////////////
int BulidMailStringEx(char* szNewMailCfg,char *szOldMailCfg,DEVICE_MAIL_CFG *pstNewMailCfg)
{
	if(NULL == szNewMailCfg || NULL == szOldMailCfg || NULL == pstNewMailCfg)
	{
		return NET_ERROR;
	}
	int nSize =0;
	bool bParseResult =false;
	CStrParse parseOldMailCfg;
	parseOldMailCfg.setSpliter("&&");
	bParseResult = parseOldMailCfg.Parse(szOldMailCfg);
	if(!bParseResult)
	{
		return NET_ERROR;
	}

	char	sMailIPAddr[MAX_DOMAIN_NAME_LEN+4] = {0};	// 邮件服务器地址(IP或者域名)
	char	sSubMailIPAddr[MAX_DOMAIN_NAME_LEN+4] = {0};	// 邮件服务器子地址(IP或者域名)
	char	sSenderAddr[MAX_MAIL_ADDRESS_LEN+4] = {0};		// 发送地址
	char	sUserName[MAX_MAIL_USERNAME_LEN+4] = {0};		// 用户名
	char	sUserPsw[MAX_MAIL_USERNAME_LEN+4] = {0};			// 用户密码
	char	sDestAddr[MAX_MAIL_ADDRESS_LEN+4] = {0};		// 目的地址
	char	sCcAddr[MAX_MAIL_ADDRESS_LEN+4] = {0};			// 抄送地址
	char	sBccAddr[MAX_MAIL_ADDRESS_LEN+4] = {0};			// 暗抄地址
	char	sSubject[MAX_MAIL_SUBJECT_LEN+4] = {0};		// 标题

	memcpy(sMailIPAddr, pstNewMailCfg->sMailIPAddr, MAX_DOMAIN_NAME_LEN);
	memcpy(sSubMailIPAddr, pstNewMailCfg->sSubMailIPAddr, MAX_DOMAIN_NAME_LEN);
	memcpy(sSenderAddr, pstNewMailCfg->sSenderAddr, MAX_MAIL_ADDRESS_LEN);
	memcpy(sUserName, pstNewMailCfg->sUserName, MAX_MAIL_USERNAME_LEN);
	memcpy(sUserPsw, pstNewMailCfg->sUserPsw, MAX_MAIL_USERNAME_LEN);
	memcpy(sDestAddr, pstNewMailCfg->sDestAddr, MAX_MAIL_ADDRESS_LEN);
	memcpy(sCcAddr, pstNewMailCfg->sCcAddr, MAX_MAIL_ADDRESS_LEN);
	memcpy(sBccAddr, pstNewMailCfg->sBccAddr, MAX_MAIL_ADDRESS_LEN);
	memcpy(sSubject, pstNewMailCfg->sSubject, MAX_MAIL_SUBJECT_LEN);
	
	char *p = strstr(parseOldMailCfg.getWord(0).c_str(),"|");
	if( NULL== p )//only one Ip
	{
		sprintf(szNewMailCfg
			,"%s:%d&&%s&&%s&&%s&&%s&&%s&&%s&&%s"
			,sMailIPAddr
			,pstNewMailCfg->wMailPort
			,sDestAddr
			,sCcAddr
			,sBccAddr
			,sSenderAddr
			,sUserName
			,sUserPsw
			,sSubject
			);
	}
	else//two Ip
	{
	sprintf(szNewMailCfg
		,"%s:%d|%s:%d&&%s&&%s&&%s&&%s&&%s&&%s&&%s"
		,sMailIPAddr
		,pstNewMailCfg->wMailPort
		,sSubMailIPAddr
		,pstNewMailCfg->wSubMailPort
		,sDestAddr
		,sCcAddr
		,sBccAddr
		,sSenderAddr
		,sUserName
		,sUserPsw
		,sSubject
		);
	}
	nSize = parseOldMailCfg.Size();
	if(nSize <= 13)
	{
		for(int i=8; i<nSize-1 ;i++)
		{
			strcat(szNewMailCfg,"&&");
			strcat(szNewMailCfg,parseOldMailCfg.getWord(i).c_str());
		}
		if(nSize == 12 || nSize == 13)
		{
			strcat(szNewMailCfg, (pstNewMailCfg->bEnable >0 ? "&&true" : "&&false"));
		}
		else
		{
			strcat(szNewMailCfg,"&&");
			strcat(szNewMailCfg, parseOldMailCfg.getWord().c_str());
		}
	}
	else// nSize >= 14
	{
		for(int i=8; i<11 ;i++)
		{
			strcat(szNewMailCfg,"&&");
			strcat(szNewMailCfg,parseOldMailCfg.getWord(i).c_str());
		}

		strcat(szNewMailCfg, (pstNewMailCfg->bEnable > 0 ? "&&true" : "&&false"));
		strcat(szNewMailCfg, (pstNewMailCfg->bSSLEnable > 0 ? "&&1" : "&&0"));
		sprintf(szNewMailCfg+strlen(szNewMailCfg), "&&%d", pstNewMailCfg->wSendInterval);
		
		if(nSize >= 15)
		{
			sprintf(szNewMailCfg+strlen(szNewMailCfg), "&&%s", pstNewMailCfg->bAnonymous == 0 ? "False" : "True");
			sprintf(szNewMailCfg+strlen(szNewMailCfg), "&&%s", pstNewMailCfg->bAttachEnable == 0 ? "False" : "True");
		}
	}
	return 0;
	
}
int ParseMailStringEx(const std::string & buf, void *des)
{
	DEVICE_MAIL_CFG *struMail = (DEVICE_MAIL_CFG *)des;
	if(!struMail)
	{
		return -1;
	}
	memset(des,0,sizeof(DEVICE_MAIL_CFG));
	CStrParse parse;
	parse.setSpliter("&&");
	BOOL bSuccess = parse.Parse(buf);
	if(!bSuccess)
	{
		return -1;
	}
    std::string strIpAddress=parse.getWord(0);
	if(strIpAddress.size() > 0 )
	{
		CStrParse ipParse;
		ipParse.setSpliter("|");
		ipParse.Parse(strIpAddress);
		{
			CStrParse portParse;
			portParse.setSpliter(":");
			
			portParse.Parse(ipParse.getWord(0));
			strcpy(struMail->sMailIPAddr,portParse.getWord(0).c_str() );
			struMail->wMailPort=portParse.getValue(1);
			
			portParse.Parse(ipParse.getWord(1));
			strcpy(struMail->sSubMailIPAddr,portParse.getWord(0).c_str() );
			struMail->wSubMailPort=portParse.getValue(1);
		}	
	}

	strcpy(struMail->sDestAddr, parse.getWord(1).c_str());
	strcpy(struMail->sCcAddr, parse.getWord(2).c_str());
	strcpy(struMail->sBccAddr, parse.getWord(3).c_str());
	strcpy(struMail->sSenderAddr, parse.getWord(4).c_str());
	strcpy(struMail->sUserName, parse.getWord(5).c_str());
	strcpy(struMail->sUserPsw, parse.getWord(6).c_str());
	strcpy(struMail->sSubject, parse.getWord(7).c_str());
	if(parse.Size() <= 13)//之前的邮件协议
	{
		if( _stricmp( parse.getWord(11).c_str(), "false") == 0 || (_stricmp( parse.getWord(12).c_str(), "false") == 0) )
		{
			struMail->bEnable = 0;
		}
		else 
		{
			struMail->bEnable = 1;
		}
	}
	else // parse.Size() > 13
	{
		struMail->bEnable = 1;
		if(_stricmp( parse.getWord(11).c_str(), "false") == 0)
		{
			struMail->bEnable = 0;
		}

		struMail->bSSLEnable = parse.getValue(12);
		struMail->wSendInterval = parse.getValue(13);

		if(parse.Size() > 14)
		{
			struMail->bAnonymous = _stricmp(parse.getWord(14).c_str(), "False") == 0 ? 0 : 1;
			struMail->bAttachEnable = _stricmp(parse.getWord(15).c_str(), "False") == 0 ? 0 : 1;
		}
	}
	return 0;
}
int ParseMailString(char *buf, int bufLen, void *des)
{
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int itemIdx = 0;	//Stores item indexs
	char tmp[6] = {0};

	MAIL_CFG *struMail = (MAIL_CFG *)des;
	if (!struMail)
	{
		goto e_out;
	}
	
	if (!des || !buf)
	{
		goto e_out;
	}

	while (pos < bufLen) 
	{
		if (buf[pos] != '&')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (buf[pos+1] != '&')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				//Parse the item
				switch(itemIdx)
				{
				case 0:	//mail server address
					{
						if (subLen >= (MAX_IP_ADDRESS_LEN + 6)*3)
						{
							goto e_out;
						}
						//get ip and port
						int addrlen = 0;
						int iplen = 0;
						int ippos = 0;
						while (addrlen < subLen) 
						{
							if (*(buf+startPos+addrlen) != '|')
							{
								addrlen ++;
							}
							else
							{
								//get first server
								break;
							}
						}
						while (ippos < addrlen) 
						{
							if (*(buf+startPos+ippos) != ':') 
							{
								ippos++;
								iplen++;
							}
							else //got ip
							{
								if (iplen >= MAX_IP_ADDRESS_LEN)
								{
									goto e_out;
								}
								memcpy(struMail->sMailIPAddr, buf+startPos, iplen);
								//got port
								memcpy(tmp, buf+startPos+iplen+1, addrlen-iplen-1);
								struMail->wMailPort = atoi(tmp);
								break;
							}
						}
					}
					break;
				case 1:	//dest address
					if (subLen >= MAX_MAIL_ADDRESS_LEN)
					{
						goto e_out;
					}
					memcpy(struMail->sDestAddr, buf+startPos, subLen);
					break;
				case 2: //cc address
					if (subLen >= MAX_MAIL_ADDRESS_LEN) 
					{
						goto e_out;
					}
					memcpy(struMail->sCcAddr, buf+startPos, subLen);
					//	memcpy(struHost->sHostIPAddr, buf+startPos, subLen);
					break;
				case 3: //bcc address
					if (subLen >= MAX_MAIL_ADDRESS_LEN) 
					{
						goto e_out;
					}
					memcpy(struMail->sBccAddr, buf+startPos, subLen);
					//	memcpy(struHost->sHostIPAddr, buf+startPos, subLen);
					break;
				case 4: //sender address, ignore
					if (subLen >= MAX_MAIL_ADDRESS_LEN) 
					{
						goto e_out;
					}
					memcpy(struMail->sSenderAddr, buf+startPos, subLen);
					break;
				case 5: //sender name, ignore
					if (subLen >= MAX_GENERAL_NAME_LEN) 
					{
						goto e_out;
					}
					memcpy(struMail->sUserName, buf+startPos, subLen);
					break;
				case 6: //sender password, ignore
					if (subLen >= MAX_GENERAL_NAME_LEN) 
					{
						goto e_out;
					}
					memcpy(struMail->sUserPsw, buf+startPos, subLen);
					break;
				case 7: //subject
					if (subLen >= MAX_MAIL_SUBJECT_LEN)
					{
						goto e_out;
					}
					memcpy(struMail->sSubject, buf+startPos, subLen);
					break;
				case 8: //mail body
					break;
				case 9: //attachment
					break;
				default:
					break;
				}
				pos += 2;
				startPos = pos;
				subLen = 0;
				itemIdx++;
			}
		}
	}
	
	if (itemIdx != 10) //the last item
	{
		goto e_out;	
	}

	return 0;
e_out:
	return -1;
}

int DecodeMailCfg(void *lpMailCfg, char *pOutBuf, int iMaxOutBufLen)
{
	if (NULL == pOutBuf || 0 == iMaxOutBufLen || NULL == lpMailCfg)
	{
		return 0;
	}

	MAIL_CFG *pMailCfg = (MAIL_CFG *)lpMailCfg;
	int iCurPos = 0;
	int iNeedBufLen = 0;
	int iCopyStrLen = 0;

	memset(pOutBuf, 0, iMaxOutBufLen);

	//server addr
	iCopyStrLen = strlen(pMailCfg->sMailIPAddr)<sizeof(pMailCfg->sMailIPAddr)?strlen(pMailCfg->sMailIPAddr):sizeof(pMailCfg->sMailIPAddr);
	iNeedBufLen = iCurPos + iCopyStrLen + 1;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sMailIPAddr, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, ":", 1);
	iCurPos += 1;
	iNeedBufLen = iCurPos + 8;
	if (iMaxOutBufLen < iNeedBufLen)//"65535"+2
	{
		goto FAILED;
	}
	else
	{
		itoa(pMailCfg->wMailPort, pOutBuf+iCurPos, 10);
		iCopyStrLen = strlen(pOutBuf+iCurPos);
		iCurPos += iCopyStrLen;
		memcpy(pOutBuf+iCurPos, "&&", 2);
		iCurPos += 2;
	}
	

	//dest addr
	iCopyStrLen = strlen(pMailCfg->sDestAddr)<sizeof(pMailCfg->sDestAddr)?strlen(pMailCfg->sDestAddr):sizeof(pMailCfg->sDestAddr);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sDestAddr, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	//cc addr
	iCopyStrLen = strlen(pMailCfg->sCcAddr)<sizeof(pMailCfg->sCcAddr)?strlen(pMailCfg->sCcAddr):sizeof(pMailCfg->sCcAddr);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sCcAddr, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	//bcc addr
	iCopyStrLen = strlen(pMailCfg->sBccAddr)<sizeof(pMailCfg->sBccAddr)?strlen(pMailCfg->sBccAddr):sizeof(pMailCfg->sBccAddr);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sBccAddr, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	//sender addr ,ignore
	iCopyStrLen = strlen(pMailCfg->sSenderAddr)<sizeof(pMailCfg->sSenderAddr)?strlen(pMailCfg->sSenderAddr):sizeof(pMailCfg->sSenderAddr);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sSenderAddr, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	//sender usrname
	iCopyStrLen = strlen(pMailCfg->sUserName)<sizeof(pMailCfg->sUserName)?strlen(pMailCfg->sUserName):sizeof(pMailCfg->sUserName);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sUserName, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	//sender psw
	iCopyStrLen = strlen(pMailCfg->sUserPsw)<sizeof(pMailCfg->sUserPsw)?strlen(pMailCfg->sUserPsw):sizeof(pMailCfg->sUserPsw);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sUserPsw, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	//subject
	iCopyStrLen = strlen(pMailCfg->sSubject)<sizeof(pMailCfg->sSubject)?strlen(pMailCfg->sSubject):sizeof(pMailCfg->sSubject);
	iNeedBufLen = iCurPos + iCopyStrLen + 2;
	if (iMaxOutBufLen < iNeedBufLen || iCopyStrLen < 0)
	{
		goto FAILED;
	}
	memcpy(pOutBuf+iCurPos, pMailCfg->sSubject, iCopyStrLen);
	iCurPos += iCopyStrLen;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	/*
	 *	后面还有两个信息没有设置，待加
	 */
	iNeedBufLen = iCurPos + 2;
	memcpy(pOutBuf+iCurPos, "&&", 2);
	iCurPos += 2;

	return iCurPos;
FAILED:
	return 0;
}


// change baud rate between index and value
DWORD ChangeBaudRate(DWORD dwBaud)
{
	switch(dwBaud) {
	case 300: return 0;
	case 600: return 1;
	case 1200: return 2;
	case 2400: return 3;	
	case 4800: return 4;
	case 9600: return 5;	
	case 19200: return 6;
	case 38400: return 7;	
	case 57600: return 8;
	case 115200: return 9;

	case 0: return 300;
	case 1: return 600;
	case 2: return 1200;
	case 3: return 2400;	
	case 4: return 4800;
	case 5: return 9600;	
	case 6: return 19200;
	case 7: return 38400;	
	case 8: return 57600;
	case 9: return 115200;
	default:
		return 0;
	}
}

//get masked 232 function names
void GetMaskedFuncName(char *buf, int buflen, DWORD dwMask, char *namelist, int maxnum, DWORD *getnum)
{
	if (!buf || !namelist ||!getnum)
	{
		return;
	}
	int nameIdx = 0;
	DEC_PROTOCOL *tmpNode = 0;
	for (int i = 0; i< 32; i++)
	{
		if ((dwMask&(0x01<<i)) && ((i+1)*sizeof(DEC_PROTOCOL) <= buflen))
		{
			tmpNode = (DEC_PROTOCOL*)(buf+i*sizeof(DEC_PROTOCOL));
			Change_Utf8_Assic((BYTE*)tmpNode->ProtocolName, namelist+nameIdx*MAX_GENERAL_NAME_LEN);
			//memcpy(namelist+nameIdx*MAX_NAME_LEN, tmpNode->ProtocolName, 12);
			nameIdx++;
			if (nameIdx >= maxnum)
			{
				break;
			}
		}
	}
	*getnum = nameIdx;
}

//get masked 232 function index
void GetMaskedFuncIndex(BYTE *des, BYTE src, DWORD dwMask)
{
	if (!des)
	{
		return;
	}
	BYTE idx = 0;
	for (int i = 0; i< 32; i++)
	{
		if (i >= src)
		{
			break;
		}
		else if ((dwMask&(0x01<<i))) 
		{
			idx++;
		}
	}
	*des = idx;
}

void GetMaskedFuncIndex2Dev(BYTE *des, BYTE src, DWORD dwMask)
{
	if (NULL == des)
	{
		return;
	}
	BYTE idx = 0;
	int i = 0;
	for (i = 0; i < 32; i++)
	{
		if(dwMask&(1<<i))
		{
			if (idx >= src)
			{
				break;
			}
			else
			{
				idx++;
			}
		}
	}
	*des = i;
}

int CDevConfig::GetDevConfig_DevCfg(LONG lLoginID, DEV_SYSTEM_ATTR_CFG &stSAC, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stSAC, 0, sizeof(DEV_SYSTEM_ATTR_CFG));
	stSAC.dwSize = sizeof(DEV_SYSTEM_ATTR_CFG);
	int retlen = 0;
	int nRet = -1;

	//get system attribute
	SYSATTR_T stAT = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ATTR, (char *)&stAT, sizeof(SYSATTR_T), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(SYSATTR_T))
	{
		//软件版本
		stSAC.stVersion.dwSoftwareVersion |= stAT.iVerMaj;
		stSAC.stVersion.dwSoftwareVersion <<= 16;
		stSAC.stVersion.dwSoftwareVersion |= stAT.iVerSub;
		//软件生成日期
		stSAC.stVersion.dwSoftwareBuildDate |= stAT.year;
		stSAC.stVersion.dwSoftwareBuildDate <<= 8;
		stSAC.stVersion.dwSoftwareBuildDate |= stAT.month;
		stSAC.stVersion.dwSoftwareBuildDate <<= 8;
		stSAC.stVersion.dwSoftwareBuildDate |= stAT.day;
		//WEB版本
		stSAC.stVersion.dwWebVersion = *(DWORD *)stAT.iWebVersion;
		
		stSAC.byDevType = device->device_type(device);	//设备类型
		stSAC.byVideoCaptureNum = stAT.iVideoInCaps;	//视频接入口数
		stSAC.byAudioCaptureNum = stAT.iAudioInCaps;	//音频输入口数
		stSAC.byAlarmInNum = stAT.iAlarmInCaps;			//报警输入口数
		stSAC.byAlarmOutNum = stAT.iAlarmOutCaps;		//报警输出口数
		stSAC.byNetIONum = stAT.iAetherNetPortNum;		//网络口
		stSAC.byUsbIONum = stAT.iUsbPortNum;			//USB口
		stSAC.byIdeIONum = stAT.iIdePortNum;			//IDE接口数
		stSAC.byLPTIONum = stAT.iParallelPortNum;		//并口数
		stSAC.byComIONum = stAT.iComPortNum;
		stSAC.byTalkInChanNum = 1;		//NSP
		stSAC.byTalkOutChanNum = 1;		//NSP
		stSAC.byDecodeChanNum = 1;		//NSP
		stSAC.byVgaIONum = 1;			//NSP
		stSAC.byIdeControlNum = 0;		//NSP
		stSAC.byIdeControlType = 0;		//NSP
		stSAC.byMatrixOutNum = stAT.iMatrixOutNum;
	}
	else
	{
		return NET_ERROR_GETCFG_SYSATTR;
	}

	//get dev version
	char szVersion[32] = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_VER, szVersion, 32, &retlen, waittime);
	if (nRet >= 0)
	{
		ParseVersionString(szVersion, retlen, &stSAC.stVersion.dwSoftwareVersion);
	}
	else
	{
		//temporarily allowed
		//return NET_ERROR_GETCFG_SERIAL;
	}
	
	//get serial number
	char szSer[32] = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ID, szSer, 32, &retlen, waittime);
	if (nRet >= 0)
	{
		memcpy(stSAC.szDevSerialNo, szSer, 32);
		stSAC.szDevSerialNo[DEV_SERIAL_NUM_LEN-1] = '\0';
	}
	else
	{
		//temporarily allowed
		//return NET_ERROR_GETCFG_SERIAL;
	}
	
	//get general config
	CONFIG_GENERAL stGen = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_GENERAL, 0, (char *)&stGen, sizeof(CONFIG_GENERAL), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERAL))
	{
		stSAC.wDevNo = stGen.LocalNo;			//本地编号
		stSAC.byOverWrite = stGen.OverWrite;	//硬盘满时　1:覆盖, 0:停止
		stSAC.byRecordLen = stGen.RecLen;		//录象段长度
		//	stSAC.dwLanguage = stGen.Language;	//语言选择
		//  stSAC.byStartChanNo = 0;			//NSP, default 0
		stSAC.byVideoStandard = stGen.VideoFmt;	//制式 :NTSC,PAL等
		stSAC.byDateFormat = stGen.DateFmt;		//日期格式
		stSAC.byDateSprtr = stGen.DateSprtr;	//日期分割符
		stSAC.byTimeFmt = stGen.TimeFmt;		//时间格式
		stSAC.wDevNo = stGen.LocalNo;			//设备编号
		stSAC.byLanguage = stGen.Language;		//语言选择
		stSAC.byDSTEnable = stGen.DST;			//夏令时使能: 1:实行 0:不实行
	}
	else
	{
		return NET_ERROR_GETCFG_GENERAL;
	}
	
	//	此功能解码器除外
	if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
	{
		//get resolution and encode-type mask
		char maskbuf[8] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSP_MASK, 0, maskbuf, 8, &retlen, waittime);
		if (nRet >= 0 && retlen == 8) 
		{
			stSAC.stDspEncodeCap.dwImageSizeMask = *(DWORD *)(maskbuf + 4);
			stSAC.stDspEncodeCap.dwEncodeModeMask = *(DWORD *)(maskbuf);
		}
		else
		{
			stSAC.stDspEncodeCap.dwImageSizeMask = 0;
			stSAC.stDspEncodeCap.dwEncodeModeMask = 0;
			return NET_ERROR_GETCFG_DSPCAP;
		}
		
		//辅码流能力
		int bufLen = 32*sizeof(CAPTURE_EXT_STREAM);
		bufLen = bufLen<sizeof(CAPTURE_EXT_STREAM_NEW) ? sizeof(CAPTURE_EXT_STREAM_NEW) : bufLen;
		char* capBuf = new char[bufLen];
		if (NULL == capBuf)
		{
			return -1;
		}
		memset(capBuf, 0, bufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSP_MASK, 1, capBuf, bufLen, &retlen, waittime);
		if (nRet >= 0 && retlen > 8) 
		{
			CAPTURE_EXT_STREAM* stExt = (CAPTURE_EXT_STREAM*)capBuf;
			stSAC.stDspEncodeCap.dwStreamCap = stExt->ExtraStream;
			memcpy(stSAC.stDspEncodeCap.dwImageSizeMask_Assi, stExt->CaptureSizeMask, sizeof(stSAC.stDspEncodeCap.dwImageSizeMask_Assi));
		}
		//同时支持新的辅码流能力协议，配置版本1
		else if (nRet >= 0 && retlen == sizeof(CAPTURE_EXT_STREAM_NEW))
		{
			CAPTURE_EXT_STREAM_NEW* stExt = (CAPTURE_EXT_STREAM_NEW*)capBuf;
			stSAC.stDspEncodeCap.dwStreamCap = stExt->ExtraStream;
			memcpy(stSAC.stDspEncodeCap.dwImageSizeMask_Assi, stExt->CaptureSizeMask, sizeof(stSAC.stDspEncodeCap.dwImageSizeMask_Assi));
		}
		else
		{
			stSAC.stDspEncodeCap.dwStreamCap = 0x01;
			nRet = 0;
			//return NET_ERROR_GETCFG_DSPCAP;
		}
		delete[] capBuf;
		
		//DSP能力
		CAPTURE_DSPINFO stDsp = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSPCAP, 0, (char*)&stDsp, sizeof(stDsp), &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(stDsp)) 
		{
			stSAC.stDspEncodeCap.dwMaxEncodePower = stDsp.nMaxEncodePower;
			stSAC.stDspEncodeCap.wMaxSupportChannel = stDsp.nMaxSupportChannel;
			stSAC.stDspEncodeCap.wChannelMaxSetSync = stDsp.bChannelMaxSetSync;
		}
		else
		{
			//默认
			stSAC.stDspEncodeCap.dwMaxEncodePower = 0xFFFFFFFF;
			stSAC.stDspEncodeCap.wMaxSupportChannel = 1;
			stSAC.stDspEncodeCap.wChannelMaxSetSync = 0;
			nRet = 0;
			//return NET_ERROR_GETCFG_DSPCAP;
		}
	}

	//stSAC.stDspEncodeCap.dwVideoStandardMask = 0x03; //NSP, default 0x03
	//视频制式能力现在由设备传过来了add by cqs 2009.2.19
	int nRetLen = 0;
	char buffer[512] = {0};
	nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_VIDEOFORMAT_INFO, NULL, buffer, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		char szValue[64] = {0};
		char *p = buffer;
		while (p)
		{
			p = GetProtocolValue(p, "SupportItem:", "\r\n", szValue, 64);
			if (0 == _stricmp(szValue, "PAL"))
			{
				stSAC.stDspEncodeCap.dwVideoStandardMask |= 0x01;
			}
			else if(0 == _stricmp(szValue, "NTSC"))
			{
				stSAC.stDspEncodeCap.dwVideoStandardMask |= 0x02;
			}
			else
			{
				stSAC.stDspEncodeCap.dwVideoStandardMask |= 0x03; //NSP, default 0x03
				break;
			}
		}
	}
	else
	{
		stSAC.stDspEncodeCap.dwVideoStandardMask |= 0x03; //NSP, default 0x03
	}

	//get device type string
	char devtypebuf[DEV_TYPE_STR_LEN] = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_TYPE, devtypebuf, DEV_TYPE_STR_LEN, &retlen, waittime);
	if (nRet >= 0)
	{
		memcpy(stSAC.szDevType, devtypebuf, DEV_TYPE_STR_LEN);
		stSAC.szDevType[DEV_TYPE_STR_LEN-1] = '\0';
	}
	else
	{
		//temporarily allowed
		//return NET_ERROR_GETCFG_SERIAL;
	}

	return 0;
}

//Add: by zsc(11402) 2008-4-7
int CDevConfig::GetDevConfig_WLANCfg(LONG lLoginID, DEV_WLAN_INFO &stuWlanInfo, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return NET_INVALID_HANDLE;
	}
	
	memset(&stuWlanInfo, 0, sizeof(DEV_WLAN_INFO));

	int nRetLen = 0;
	int nRet = -1;
	bool bSupport = false;
	Wireless_Enable_T stuEnable = {0};

	//查看能力
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WIRELESS, (char*)&stuEnable, sizeof(Wireless_Enable_T), &nRetLen, waittime);
	if (nRet ==0 && nRetLen == sizeof(Wireless_Enable_T))
	{
		if(1 == stuEnable.isWirelessEnable)
		{
			bSupport = true;
		}
	}

	if (!bSupport)
	{
		return -1;
	}

	CONFIG_WLAN_INFO WlanCfg = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_WLAN, 0, (char*)&WlanCfg, sizeof(CONFIG_WLAN_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen == sizeof(CONFIG_WLAN_INFO)) 
	{
		stuWlanInfo.nEnable = WlanCfg.nEnable;
		stuWlanInfo.nEncryption = WlanCfg.nEncryption;
		stuWlanInfo.nKeyFlag = WlanCfg.nKeyFlag;
		stuWlanInfo.nKeyID = WlanCfg.nKeyID;
		stuWlanInfo.nKeyType = WlanCfg.nKeyType;
		stuWlanInfo.nLinkMode = WlanCfg.nLinkMode;
		strcpy(stuWlanInfo.szSSID, WlanCfg.szSSID);
		if(stuWlanInfo.nEncryption != 4 && stuWlanInfo.nEncryption != 5)
		{
			CStrParse parse;
			parse.setSpliter("&&");
			bool bSuccess = parse.Parse(WlanCfg.szKeys);
			if (!bSuccess)
			{
			//	return -1;//暂时屏蔽掉
			}

			int nCount = parse.Size();
			nCount = nCount > 4 ? 4 : nCount;
			for (int i = 0; i < nCount; i++)
			{
				strcpy(&stuWlanInfo.szKeys[i][0], parse.getWord(i).c_str());
			}
		}
		else//wap
		{
			memcpy(stuWlanInfo.szWPAKeys, WlanCfg.szKeys, 128);
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_WLAN;
	}

	return nRet;
}

int CDevConfig::GetDevConfig_WLANDevCfg(LONG lLoginID, DEV_WLAN_DEVICE_LIST *pWlanLst, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pWlanLst == NULL)
	{
		return NET_INVALID_HANDLE;
	}

	int nRetLen = 0;
	int nRet = -1;
	bool bSupport = false;
	Wireless_Enable_T stuEnable = {0};

	//查看能力
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WIRELESS, (char*)&stuEnable, sizeof(Wireless_Enable_T), &nRetLen, waittime);
	if (nRet ==0 && nRetLen == sizeof(Wireless_Enable_T))
	{
		if(1 == stuEnable.isWirelessEnable)
		{
			bSupport = true;
		}
	}
	if (!bSupport)
	{
		return -1;
	}

	pWlanLst->dwSize = sizeof(DEV_WLAN_DEVICE_LIST);

	nRetLen = 0;
	CONFIG_WLAN_DEVICE stuWlanDev[32] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_WLAN_DEVICE, 0, (char*)stuWlanDev, 32*sizeof(CONFIG_WLAN_DEVICE), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0 && 0 == nRetLen%sizeof(CONFIG_WLAN_DEVICE)) 
	{
		int nWlanDevCount = nRetLen/sizeof(CONFIG_WLAN_DEVICE);
		if (nWlanDevCount > 10)
		{
			pWlanLst->bWlanDevCount = 10;
		}
		else
		{
			pWlanLst->bWlanDevCount = (BYTE)nWlanDevCount;
		}
		
		for (int i = 0; i < pWlanLst->bWlanDevCount; i++)
		{
			strcpy(pWlanLst->lstWlanDev[i].szSSID, stuWlanDev[i].szSSID);
			pWlanLst->lstWlanDev[i].nLinkMode = stuWlanDev[i].nLinkMode;
			pWlanLst->lstWlanDev[i].nEncryption = stuWlanDev[i].nEncryption;
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_WLANDEV;
	}

	return nRet;
}

int CDevConfig::GetDevConfig_AutoRegisterCfg(LONG lLoginID, DEV_REGISTER_SERVER *pRegisterSev, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pRegisterSev == NULL)
	{
		return NET_INVALID_HANDLE;
	}

	int nRet = -1;
	int nRetLen = 0;
	bool bSupport = false;
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_AUTO_REGISTER] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}

	pRegisterSev->dwSize = sizeof(DEV_REGISTER_SERVER);

	//	查询配置信息
	char buf[1024] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_AUTO_REGISTER, 0, buf, 1024, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		CStrParse parseFirst;
		parseFirst.setSpliter("||");
		parseFirst.Parse(buf);
		std::string strFirstSection = parseFirst.getWord(0);
		std::string strSecondSection = parseFirst.getWord(1);

		CStrParse parse;
		parse.setSpliter("|");
		parse.Parse(strSecondSection);
		pRegisterSev->bEnable = parse.getValue(0);
		strcpy(pRegisterSev->szDeviceID,parse.getWord(1).c_str());
		
		parse.setSpliter("&&");
		bool bSuccess = parse.Parse(strFirstSection);
		if (!bSuccess)
		{
			return -1;
		}

		int nCount = parse.Size();
		if (nCount > 0 && nCount <= MAX_REGISTER_SERVER_NUM)
		{
			pRegisterSev->bServerNum = nCount;
			for (int i = 0; i < nCount; i++)
			{
				std::string registerInfo = parse.getWord();

				CStrParse subparse;
				subparse.setSpliter("::");
				bSuccess = subparse.Parse(registerInfo);
				if (!bSuccess)
				{
					break;
				}

				strcpy(pRegisterSev->lstServer[i].szServerIp, subparse.getWord().c_str());
				pRegisterSev->lstServer[i].nServerPort = atoi(subparse.getWord().c_str());
			}
		}
		else
		{
			nRet = NET_RETURN_DATA_ERROR;
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_REGISTER;
	}

	return nRet;
}

int CDevConfig::GetDevConfig_AllChnCmrCfg(LONG lLoginID, DEVICE_CAMERA_CFG *pstCHC, LONG lChnNum, int waittime)
{
	if (pstCHC == NULL || lChnNum < 0 || lChnNum > 16)
	{
		return -1;
	}

	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(pstCHC, 0, lChnNum*sizeof(DEVICE_CAMERA_CFG));

	for (int i = 0; i < lChnNum; i++)
	{
		pstCHC[i].dwSize = sizeof(DEVICE_CAMERA_CFG);
	}

	int retlen = 0;
	int nRet = -1;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_CAMERA_T))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_CAMERA_T);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = NET_SYSTEM_ERROR;
		goto END;
	}
	
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAMERA, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen > 0 && 0 == (retlen % sizeof(CONFIG_CAMERA_T)))
	{
		CONFIG_CAMERA_T *pCameraInfo = NULL;
		int nCount = retlen / sizeof(CONFIG_CAMERA_T);
		nCount = nCount > lChnNum ? lChnNum : nCount;
		for (int i = 0; i < nCount; i++)
		{
			pCameraInfo = (CONFIG_CAMERA_T *)(cRecvBuf + i*sizeof(CONFIG_CAMERA_T));
			pstCHC[i].bExposure = pCameraInfo->bExposure;
			pstCHC[i].bBacklight = pCameraInfo->bBacklight;
			pstCHC[i].bAutoColor2BW = pCameraInfo->bAutoColor2BW;
			pstCHC[i].bMirror = pCameraInfo->bMirror;
			pstCHC[i].bFlip = pCameraInfo->bFlip;
			pstCHC[i].bLensEn = (pCameraInfo->bLensFunction>>4)&0x01; //高4位代表能力
			pstCHC[i].bLensFunction = (pCameraInfo->bLensFunction)&0x01;//低4位代表自动光圈的开关
			pstCHC[i].bWhiteBalance = pCameraInfo->bWhiteBalance;//增加白平衡功能
			pstCHC[i].bSignalFormat = pCameraInfo->bSignalFormat;//增加信号格式
			pstCHC[i].bRotate90 = pCameraInfo->bRotate90;
			pstCHC[i].ExposureValue1 = pCameraInfo->ExposureValue1;
			pstCHC[i].ExposureValue2 = pCameraInfo->ExposureValue2;
		}
	}
	else
	{
		return NET_ERROR_GETCFG_CAMERA;
	}

END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}

	return nRet;
}

int CDevConfig::GetDevConfig_ChnCmrCfg(LONG lLoginID, DEVICE_CAMERA_CFG &stCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	if (lChannel < 0 || lChannel >= device->channelcount(device))
	{
		return -1;
	}
	
	memset(&stCHC, 0, sizeof(DEVICE_CAMERA_CFG));
	stCHC.dwSize = sizeof(DEVICE_CAMERA_CFG);

	int retlen = 0;
	int nRet = -1;

	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_CAMERA_T))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_CAMERA_T);
	}

	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = NET_SYSTEM_ERROR;
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAMERA, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen > 0 && 0 == (retlen % sizeof(CONFIG_CAMERA_T)) && lChannel < (retlen/sizeof(CONFIG_CAMERA_T)))
	{
		CONFIG_CAMERA_T *pCameraInfo = (CONFIG_CAMERA_T *)(cRecvBuf + lChannel*sizeof(CONFIG_CAMERA_T));
		stCHC.bExposure = pCameraInfo->bExposure;
		stCHC.bBacklight = pCameraInfo->bBacklight;
		stCHC.bAutoColor2BW = pCameraInfo->bAutoColor2BW;
		stCHC.bMirror = pCameraInfo->bMirror;
		stCHC.bFlip = pCameraInfo->bFlip;
		stCHC.bLensEn = (pCameraInfo->bLensFunction>>4)&0x01; //高4位代表能力
		stCHC.bLensFunction = (pCameraInfo->bLensFunction)&0x01;//低4位代表自动光圈的开关
		stCHC.bWhiteBalance = pCameraInfo->bWhiteBalance;//增加白平衡功能
		stCHC.bSignalFormat = pCameraInfo->bSignalFormat;//增加信号格式
		stCHC.bRotate90 = pCameraInfo->bRotate90;
		stCHC.ExposureValue1 = pCameraInfo->ExposureValue1;
		stCHC.ExposureValue2 = pCameraInfo->ExposureValue2;
	}
	else
	{
		return NET_ERROR_GETCFG_CAMERA;
	}

END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}

	return nRet;
}

/*
 *	摘要：获取网卡信息
 */
int CDevConfig::GetDevConfig_EthernetCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int &nEthernetNum, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pEthernetInfo == NULL)
	{
		return -1;
	}

	int nRet = -1;
	int nRetLen = 0;
	BOOL bEthernetEnable = FALSE;
	BOOL bDefaultEnable = FALSE;
	DEV_ENABLE_INFO stDevEn = {0};
	
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_DEFAULTNIC] != 0)
		{
			bDefaultEnable = TRUE;
		}

		if (stDevEn.IsFucEnable[EN_MULTIETHERNET] != 0)
		{
			bEthernetEnable = TRUE;
		}
	}

	int nEthernetFlag = 0;
	device->get_info(device, dit_ether_flag, &nEthernetFlag);

	char buf[1024] = {0};

	// 先查询网卡信息
	if (1 == nEthernetFlag || bEthernetEnable)
	{
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_ETHERNET, 0, buf, 1024, &nRetLen, waittime);
		if (nRet >= 0 && nRetLen > 0)
		{
			CStrParse parser;
			parser.setSpliter("&&");
			BOOL bSuccess = parser.Parse(buf);
			if (!bSuccess)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			nEthernetNum = parser.Size();
			if (nEthernetNum <= 0)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			nEthernetNum = Min(5, nEthernetNum);
			int i = 0;
			if (1 == nEthernetFlag && !bEthernetEnable)
			{
				i = 1;
				nEthernetNum++;
			}
			for (; i < nEthernetNum; i++)
			{
				std::string ethernetInfo = parser.getWord();
				
				CStrParse subParse;
				subParse.setSpliter("::");
				bSuccess = subParse.Parse(ethernetInfo);
				if (!bSuccess)
				{
					break;
				}
				
				strcpy(pEthernetInfo[i].szEthernetName, subParse.getWord(0).c_str());
				strcpy(pEthernetInfo[i].sDevIPAddr, subParse.getWord(1).c_str());
				strcpy(pEthernetInfo[i].sDevIPMask, subParse.getWord(2).c_str());
				strcpy(pEthernetInfo[i].sGatewayIP, subParse.getWord(3).c_str());
				strcpy(pEthernetInfo[i].byMACAddr, subParse.getWord(4).c_str());
				strcpy(pEthernetInfo[i].szMode, subParse.getWord(5).c_str());
				strcpy(pEthernetInfo[i].szDuplex, subParse.getWord(6).c_str());
				strcpy(pEthernetInfo[i].szSpeed, subParse.getWord(7).c_str());
			}
		}
	}

	// 再查询默认网卡信息
	memset(buf, 0, 1024);
	nRetLen = 0;
	if (bDefaultEnable)
	{
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_ETHERNET, 1, buf, 1024, &nRetLen, waittime);
		if (nRet >= 0 && nRetLen > 0)
		{
			CStrParse parser;
			parser.setSpliter("&&");
			BOOL bSuccess = parser.Parse(buf);
			if (!bSuccess)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			int nCount = parser.Size();
			if (nCount != 2)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			char szDefaultEther[64] = {0};
			strcpy(szDefaultEther, parser.getWord(1).c_str());
			std::string defaultEtherInfo = parser.getWord(0);
			
			parser.setSpliter("::");
			bSuccess = parser.Parse(defaultEtherInfo);
			if (!bSuccess)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			nCount = parser.Size();
			if (nCount <= 0)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			nCount = Min(nCount, 5);
			for (int i = 0; i < nCount; i++)
			{
				std::string ether = parser.getWord();
				
				CStrParse subParse;
				subParse.setSpliter(",");
				bSuccess = subParse.Parse(ether);
				if (!bSuccess)
				{
					break;
				}
				
				// 根据网卡名称对应
				for (int j = 0; j < nEthernetNum; j++)
				{
					if (0 == _stricmp(pEthernetInfo[j].szEthernetName, subParse.getWord(0).c_str()))
					{
						pEthernetInfo[j].bTranMedia = atoi(subParse.getWord(1).c_str());
						pEthernetInfo[j].bValid = atoi(subParse.getWord(2).c_str());
						if (0 == _stricmp(pEthernetInfo[j].byMACAddr, ""))
						{
							strcpy(pEthernetInfo[j].byMACAddr, subParse.getWord(3).c_str());
						}
					}
					
					if (0 == _stricmp(pEthernetInfo[j].szEthernetName, szDefaultEther))
					{
						pEthernetInfo[j].bDefaultEth = 1;
					}
					else
					{
						pEthernetInfo[j].bDefaultEth = 0;
					}
				}
			}
		}
	}

	return nRet;
}

/*
 *	摘要：修改网卡信息
 */
int CDevConfig::SetDevConfig_EthernetCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int nEthernetNum, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pEthernetInfo == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;
	int nRetLen = 0;
	BOOL bEthernetEnable = FALSE;
	BOOL bDefaultEnable = FALSE;
	DEV_ENABLE_INFO stDevEn = {0};
	
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_DEFAULTNIC] != 0)
		{
			bDefaultEnable = TRUE;
		}

		if (stDevEn.IsFucEnable[EN_MULTIETHERNET] != 0)
		{
			bEthernetEnable = TRUE;
		}
	}
	
	int nEthernetFlag = 0;
	device->get_info(device, dit_ether_flag, &nEthernetFlag);

	int nPos = 0;
	char buf[1024] = {0};
	
	// 修改网卡信息
	if (1 == nEthernetFlag || bEthernetEnable)
	{
		int i = 0;
		if (1 == nEthernetFlag && !bEthernetEnable)
		{
			i = 1;
		}
		for (; i < nEthernetNum; i++)
		{
			sprintf(buf+nPos, "%s::%s::%s::%s::%s::%s::%s::%s&&", pEthernetInfo[i].szEthernetName, pEthernetInfo[i].sDevIPAddr,
				pEthernetInfo[i].sDevIPMask, pEthernetInfo[i].sGatewayIP, pEthernetInfo[i].byMACAddr, pEthernetInfo[i].szMode, 
				pEthernetInfo[i].szDuplex, pEthernetInfo[i].szSpeed);
			nPos = strlen(buf);
		}
		
		buf[nPos-1] = '\0';
		buf[nPos-2] = '\0';
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ETHERNET, 0, buf, nPos-2, waittime);
	}

	// 修改默认配置
	nPos = 0;
	memset(buf, 0, 1024);
	char szDefaultEther[64] = {0};
	
	if (bDefaultEnable)
	{
		// 协议被修改成只传网卡名称了。
		for (int j = 0; j < nEthernetNum; j++)
		{
//			sprintf(buf+nPos, "%s,%d,%d::", pEthernetInfo[j].szEthernetName, pEthernetInfo[j].bTranMedia, pEthernetInfo[j].bValid);
//			nPos = strlen(buf);
			
			if (1 == pEthernetInfo[j].bDefaultEth)
			{
				strcpy(szDefaultEther, pEthernetInfo[j].szEthernetName);
			}
		}
//		buf[nPos-1] = '&';
//		buf[nPos-2] = '&';

		sprintf(buf+nPos, "%s", szDefaultEther);
		nPos = strlen(buf);
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ETHERNET, 1, buf, nPos, waittime);
	}

	return nRet;
}

/*
 *	摘要：获取DHCP信息
 */
int CDevConfig::GetDevConfig_EtherDHCPCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pEthernetInfo == NULL)
	{
		return -1;
	}

	int nRet = 0;
	int nRetLen = 0;
	BOOL bEnable = FALSE;
	DEV_ENABLE_INFO stDevEn = {0};
	
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_DHCP] != 0)
		{
			bEnable = TRUE;
		}
	}

	if (!bEnable)
	{
		return -1;
	}

	char buf[512] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_DHCP, 0, buf, 512, &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		CStrParse parser;
		parser.setSpliter("&&");
		bool bSuccess = parser.Parse(buf);
		if (!bSuccess)
		{
			return NET_RETURN_DATA_ERROR;
		}

		int nEthernetCount = parser.Size();
		if (nEthernetCount <= 0)
		{
			return NET_RETURN_DATA_ERROR;
		}

		nEthernetCount = nEthernetCount > 5 ? 5 : nEthernetCount;
		for (int i = 0; i < nEthernetCount; i++)
		{
			std::string ethernetInfo = parser.getWord();

			CStrParse subParse;
			subParse.setSpliter("::");
			bSuccess = subParse.Parse(ethernetInfo);
			if (!bSuccess)
			{
				break;
			}

			// 根据网卡名称对应
			for (int j = 0; j < nEthernetCount; j++)
			{
				if (0 == _stricmp(pEthernetInfo[j].szEthernetName, subParse.getWord(0).c_str()))
				{
					pEthernetInfo[j].bDHCPEnable = atoi(subParse.getWord(1).c_str());
					pEthernetInfo[j].bDHCPFunc = 1;
				}
			}
		}
	}

	return nRet;
}

int CDevConfig::SetDevConfig_EtherDHCPCfg(LONG lLoginID, CONFIG_ETHERNET *pEthernetInfo, int nEthernetNum, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pEthernetInfo == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = 0;
	int nRetLen = 0;
	BOOL bEnable = FALSE;
	DEV_ENABLE_INFO stDevEn = {0};
	
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_DHCP] != 0)
		{
			bEnable = TRUE;
		}
	}

	if (!bEnable)
	{
		return -1;
	}

	char buf[512] = {0};
	int nPos = 0;
	for (int i = 0; i < nEthernetNum; i++)
	{
		sprintf(buf+nPos, "%s::%d&&", pEthernetInfo[i].szEthernetName, pEthernetInfo[i].bDHCPEnable);
		nPos = strlen(buf);
	}
	buf[nPos-1] = '\0';
	buf[nPos-2] = '\0';
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_DHCP, 0, buf, nPos-2, waittime);

	return nRet;
}
//modify by cqs (10842)
int CDevConfig::GetDevConfig_InfraredCfg(LONG lLoginID, INFRARED_CFG* pInfraredAlarm, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pInfraredAlarm == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;
	bool bSupport = false;
	int nAlarmInCount = 8;//报警输入通道范围0-8，报警输出通道范围为0-4，遥控器通道范围为0-8 
	int nAlarmOutCount = 4;
	int nRemoteAddrCount = 8;
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	//查看能力
 	char buffer[128] = {0};
 	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_INFRARED, buffer, 128, &retlen, waittime);

	if (nRet ==0 && retlen > 0)
	{
		CStrParse parse;
		parse.setSpliter("::");
		bool bSuccess = parse.Parse(buffer);
		if (!bSuccess)
		{
			return -1;
		}

		int nEnable = atoi(parse.getWord(0).c_str());
		bSupport = nEnable == 1 ? true : false;
		nAlarmInCount = atoi(parse.getWord(1).c_str());//输入个数
		nSheetNum = nAlarmInCount;
		nAlarmOutCount = atoi(parse.getWord(2).c_str());//输出个数
		nRemoteAddrCount = atoi(parse.getWord(3).c_str());//遥控器个数
	}

	if (!bSupport)
	{
		return -1;
	}

	pInfraredAlarm->dwSize = sizeof(INFRARED_CFG);

	//查报警输出的地址配置
	if (nAlarmOutCount>0)
	{
		
		WI_ALARM_OUT stuAlarmOutAddr[16] = {0};
		nRet = QueryConfig(lLoginID,CONFIG_TYPE_WIRELESS_ADDR, 1, (char*)stuAlarmOutAddr,16*sizeof(WI_ALARM_OUT),&retlen,waittime);
		if (nRet>=0 && retlen > 0 && 0 == retlen%sizeof(WI_ALARM_OUT))
		{
			int nAlarmOut = retlen / sizeof(WI_ALARM_OUT);
//			assert(nAlarmOutCount == nAlarmOut);//能力级中得到的报警输出通道数是否相符
			pInfraredAlarm->bAlarmOutNum = nAlarmOutCount;
			for (int i = 0; i< nAlarmOut; i++)
			{
				strcpy((char *)pInfraredAlarm->AlarmOutAddr[i].address,(char *)stuAlarmOutAddr[i].address);
				strcpy((char *)pInfraredAlarm->AlarmOutAddr[i].name,(char *)stuAlarmOutAddr[i].name);			
			}
		}
		else
		{
			nFailedCount++;
		}

	}
	
	//查遥控器的地址配置
	if (nRemoteAddrCount>0)
	{
		CONFIG_ROBOT stuRemoteAddr[16] = {0};
		nRet = QueryConfig(lLoginID,CONFIG_TYPE_WIRELESS_ADDR, 2, (char*)stuRemoteAddr, 16*sizeof(CONFIG_ROBOT), &retlen,waittime);
		if (nRet>=0 && retlen > 0 && 0 == retlen%sizeof(CONFIG_ROBOT))
		{
			int nRemote = retlen / sizeof(CONFIG_ROBOT);
//			assert(nRemoteAddrCount == nRemote);//能力级中得到的遥控的个数是否相符
			pInfraredAlarm->bRobotNum = nRemoteAddrCount;
			for (int i = 0; i< nRemote; i++)
			{
				strcpy((char *)pInfraredAlarm->RobotAddr[i].address,(char *)stuRemoteAddr[i].address);
				strcpy((char *)pInfraredAlarm->RobotAddr[i].name,(char *)stuRemoteAddr[i].name);			
			}
		}
		else
		{
			nFailedCount++;
		}
	}

	CONFIG_ALARM_INFRARED stuInfrared[16] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_WIRELESS, 0, (char*)stuInfrared, 16*sizeof(CONFIG_ALARM_INFRARED), &retlen, waittime);
	if (nRet >= 0 && retlen > 0 && 0 == retlen%sizeof(CONFIG_ALARM_INFRARED))
	{
		int nAlarmCount = retlen / sizeof(CONFIG_ALARM_INFRARED);
//		assert(nAlarmInCount == nAlarmCount);//能力级中得到的报警的输入的个数是否相符
		pInfraredAlarm->bAlarmInNum = nAlarmInCount;
		for (int i = 0; i < nAlarmCount; i++)
		{
			pInfraredAlarm->InfraredAlarm[i].bEnable = stuInfrared[i].enable;
			strcpy(pInfraredAlarm->InfraredAlarm[i].szAlarmName, stuInfrared[i].alarmName);
			strcpy(pInfraredAlarm->InfraredAlarm[i].szAlarmInAddress, stuInfrared[i].alarmInAddress);
//			strcpy(pInfraredAlarm->InfraredAlarm[i].szAlarmOutAddress, stuInfrared[i].alarmOutAddress);
//			strcpy(pInfraredAlarm->InfraredAlarm[i].szRemoteAddress, stuInfrared[i].remoteAddress);
			pInfraredAlarm->InfraredAlarm[i].nAlarmInPattern = stuInfrared[i].alarmInPattern;
			pInfraredAlarm->InfraredAlarm[i].nAlarmOutPattern = stuInfrared[i].alarmOutPattern;
			pInfraredAlarm->InfraredAlarm[i].nSensorType = stuInfrared[i].iSensorType;
			pInfraredAlarm->InfraredAlarm[i].nDefendEfectTime = stuInfrared[i].defendEfectTime;
			pInfraredAlarm->InfraredAlarm[i].nDefendAreaType = stuInfrared[i].defendAreaType;
			pInfraredAlarm->InfraredAlarm[i].nAlarmSmoothTime = stuInfrared[i].alarmSmoothTime;

			GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &pInfraredAlarm->InfraredAlarm[i].struHandle.dwActionMask);
			int index = min(32, MAX_VIDEO_IN_NUM);
			int j = 0;
			for (j = 0; j < index; j++)
			{
				pInfraredAlarm->InfraredAlarm[i].struHandle.struPtzLink[j].iValue = stuInfrared[i].hEvent.PtzLink[j].iValue;
				pInfraredAlarm->InfraredAlarm[i].struHandle.struPtzLink[j].iType = stuInfrared[i].hEvent.PtzLink[j].iType;
				pInfraredAlarm->InfraredAlarm[i].struHandle.byRecordChannel[j] = BITRHT(stuInfrared[i].hEvent.dwRecord,j)&1;
				pInfraredAlarm->InfraredAlarm[i].struHandle.byTour[j] = BITRHT(stuInfrared[i].hEvent.dwTour,j)&1;
				pInfraredAlarm->InfraredAlarm[i].struHandle.bySnap[j] = BITRHT(stuInfrared[i].hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pInfraredAlarm->InfraredAlarm[i].struHandle.byRelAlarmOut[j] = BITRHT(stuInfrared[i].hEvent.dwAlarmOut,j)&1;
				pInfraredAlarm->InfraredAlarm[i].struHandle.byRelWIAlarmOut[j] = BITRHT(stuInfrared[i].hEvent.wiAlarmOut,j)&1;
			}
			pInfraredAlarm->InfraredAlarm[i].struHandle.dwDuration = stuInfrared[i].hEvent.iAOLatch;
			pInfraredAlarm->InfraredAlarm[i].struHandle.dwRecLatch = stuInfrared[i].hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pInfraredAlarm->InfraredAlarm[i].struHandle.dwEventLatch = stuInfrared[i].hEvent.iEventLatch;
			pInfraredAlarm->InfraredAlarm[i].struHandle.bMessageToNet = (BYTE)stuInfrared[i].hEvent.bMessagetoNet;
			pInfraredAlarm->InfraredAlarm[i].struHandle.bMMSEn = stuInfrared[i].hEvent.bMMSEn;
			pInfraredAlarm->InfraredAlarm[i].struHandle.bySnapshotTimes = stuInfrared[i].hEvent.SnapshotTimes;//短信发送图片的张数
			pInfraredAlarm->InfraredAlarm[i].struHandle.bMatrixEn = (BYTE)stuInfrared[i].hEvent.bMatrixEn;
			pInfraredAlarm->InfraredAlarm[i].struHandle.dwMatrix = stuInfrared[i].hEvent.dwMatrix;
			pInfraredAlarm->InfraredAlarm[i].struHandle.bLog = (BYTE)stuInfrared[i].hEvent.bLog;
			GetAlmActionFlag(stuInfrared[i].hEvent, &pInfraredAlarm->InfraredAlarm[i].struHandle.dwActionFlag);
		}

		if (nSheetNum>0)
		{
			CONFIG_WORKSHEET WorkSheet[16] = {0};
			
			nRet = GetDevConfig_WorkSheet(lLoginID, /*WSHEET_LOCAL_ALARM*/WSHEET_WIRELESS_ALARM, WorkSheet, waittime, nSheetNum);
			if (nRet >= 0)
			{
				for (int i = 0; i < nSheetNum; i++)
				{
					memcpy(pInfraredAlarm->InfraredAlarm[i].stSect, WorkSheet[i].tsSchedule, sizeof(WorkSheet[i].tsSchedule));
				}
			}
			else
			{
				nFailedCount++;
			}
		}
		
	}
	else
	{
		nFailedCount++;
	}
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_GETCFG_INFRARED;
		return nRet;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfig::SetDevConfig_InfraredCfg(LONG lLoginID, INFRARED_CFG *pInfrared, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pInfrared == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;
	bool bSupport = false;
	int nAlarmInCount = 8;//报警输入通道范围0-8，报警输出通道范围为0-4，遥控器通道范围为0-8 
	int nAlarmOutCount = 4;
	int nRemoteAddrCount = 8;
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	//查看能力
	char buffer[128] = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_INFRARED, buffer, 128, &retlen, waittime);
	if (nRet ==0 && retlen > 0)
	{
		CStrParse parse;
		parse.setSpliter("::");
		bool bSuccess = parse.Parse(buffer);
		if (!bSuccess)
		{
			return -1;
		}

		int nEnable = atoi(parse.getWord(0).c_str());
		bSupport = nEnable == 1 ? true : false;
		nAlarmInCount = atoi(parse.getWord(1).c_str());//输入个数
		nSheetNum = nAlarmInCount;
		nAlarmOutCount = atoi(parse.getWord(2).c_str());//输出个数
		nRemoteAddrCount = atoi(parse.getWord(3).c_str());//遥控器个数
	}

	if (!bSupport)
	{
		return -1;
	}

	//查报警输出的地址配置
	if (nAlarmOutCount>0)
	{
		WI_ALARM_OUT stuAlarmOutAddr[16] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_WIRELESS_ADDR, 1, (char*)stuAlarmOutAddr, 16*sizeof(WI_ALARM_OUT), &retlen, waittime);
		if (nRet>=0 && retlen > 0 && 0 == retlen%sizeof(WI_ALARM_OUT))
		{
			int nAlarmOut = retlen / sizeof(WI_ALARM_OUT);
//			assert(nAlarmOutCount == nAlarmOut);//能力级中得到的报警输出通道数是否相符
			for (int i = 0; i< nAlarmOut; i++)
			{
				strcpy((char *)stuAlarmOutAddr[i].address,(char *)pInfrared->AlarmOutAddr[i].address);
				strcpy((char *)stuAlarmOutAddr[i].name,(char *)pInfrared->AlarmOutAddr[i].name);			
			}
			
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_WIRELESS_ADDR, 1, (char*)stuAlarmOutAddr, nAlarmOut*sizeof(WI_ALARM_OUT), waittime);
			if (nRet < 0)
			{
				return NET_ERROR_SETCFG_INFRARED;
			}
			
			Sleep(SETUP_SLEEP);
		}
		else
		{
			nFailedCount++;
		}
	}

	//查遥控器的地址配置
	if (nRemoteAddrCount>0)
	{
		CONFIG_ROBOT stuRemoteAddr[16] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_WIRELESS_ADDR, 2, (char*)stuRemoteAddr, 16*sizeof(CONFIG_ROBOT), &retlen, waittime);
		if (nRet>=0 && retlen > 0 && 0 == retlen%sizeof(CONFIG_ROBOT))
		{
			int nRemote = retlen / sizeof(CONFIG_ROBOT);
//			assert(nRemoteAddrCount == nRemote);//能力级中得到的遥控的个数是否相符
			for (int i = 0; i< nRemote; i++)
			{
				strcpy((char *)stuRemoteAddr[i].address,(char *)pInfrared->RobotAddr[i].address);
				strcpy((char *)stuRemoteAddr[i].name,(char *)pInfrared->RobotAddr[i].name);			
			}
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_WIRELESS_ADDR, 2, (char*)stuRemoteAddr, nRemote*sizeof(CONFIG_ROBOT), waittime);
			if (nRet < 0)
			{
				return NET_ERROR_SETCFG_INFRARED;
			}
			
			Sleep(SETUP_SLEEP);
		}
		else
		{
			nFailedCount++;
		}	
	}
	
	CONFIG_ALARM_INFRARED stuInfrared[16] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_WIRELESS, 0, (char*)stuInfrared, 16*sizeof(CONFIG_ALARM_INFRARED), &retlen, waittime);
	if (nRet >= 0 && retlen > 0 && 0 == retlen%sizeof(CONFIG_ALARM_INFRARED))
	{
		int nCount = retlen / sizeof(CONFIG_ALARM_INFRARED);
		for (int i = 0; i < nCount; i++)
		{
			stuInfrared[i].enable = pInfrared->InfraredAlarm[i].bEnable;
			memset(stuInfrared[i].alarmName, 0, ALARM_MAX_NAME);
			memset(stuInfrared[i].alarmInAddress, 0, ALARM_MAX_NAME);
//			memset(stuInfrared[i].alarmOutAddress, 0, ALARM_MAX_NAME);
//			memset(stuInfrared[i].remoteAddress, 0, ALARM_MAX_NAME);
			strcpy(stuInfrared[i].alarmName, pInfrared->InfraredAlarm[i].szAlarmName);
			strcpy(stuInfrared[i].alarmInAddress, pInfrared->InfraredAlarm[i].szAlarmInAddress);
//			strcpy(stuInfrared[i].alarmOutAddress, pInfrared->InfraredAlarm[i].szAlarmOutAddress);
//			strcpy(stuInfrared[i].remoteAddress, pInfrared->InfraredAlarm[i].szRemoteAddress);
			stuInfrared[i].alarmInPattern = pInfrared->InfraredAlarm[i].nAlarmInPattern;
			stuInfrared[i].alarmOutPattern = pInfrared->InfraredAlarm[i].nAlarmOutPattern;
			stuInfrared[i].iSensorType = pInfrared->InfraredAlarm[i].nSensorType;
			stuInfrared[i].defendEfectTime = pInfrared->InfraredAlarm[i].nDefendEfectTime;
			stuInfrared[i].defendAreaType = pInfrared->InfraredAlarm[i].nDefendAreaType;
			stuInfrared[i].alarmSmoothTime = pInfrared->InfraredAlarm[i].nAlarmSmoothTime;

			SetAlmActionFlag(&stuInfrared[i].hEvent, pInfrared->InfraredAlarm[i].struHandle.dwActionFlag);

// 			stuInfrared[i].hEvent.dwRecord = 0;
// 			stuInfrared[i].hEvent.dwTour = 0;
// 			stuInfrared[i].hEvent.dwSnapShot = 0;

			int index = min(32, MAX_VIDEO_IN_NUM);
			int j = 0;
			stuInfrared[i].hEvent.dwSnapShot = 0;
			stuInfrared[i].hEvent.dwTour = 0;
			stuInfrared[i].hEvent.dwRecord = 0;
			for (j = 0; j < index; j++)
			{
				stuInfrared[i].hEvent.dwSnapShot |= (pInfrared->InfraredAlarm[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				stuInfrared[i].hEvent.dwTour |= (pInfrared->InfraredAlarm[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				stuInfrared[i].hEvent.dwRecord |= (pInfrared->InfraredAlarm[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				stuInfrared[i].hEvent.PtzLink[j].iValue = pInfrared->InfraredAlarm[i].struHandle.struPtzLink[j].iValue;
				stuInfrared[i].hEvent.PtzLink[j].iType = pInfrared->InfraredAlarm[i].struHandle.struPtzLink[j].iType;
			}

			index = min(32, MAX_ALARM_OUT_NUM);
			stuInfrared[i].hEvent.dwAlarmOut = 0;
			stuInfrared[i].hEvent.wiAlarmOut = 0;
			for (j = 0; j < index; j++)
			{
				stuInfrared[i].hEvent.dwAlarmOut |= (pInfrared->InfraredAlarm[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				stuInfrared[i].hEvent.wiAlarmOut |= (pInfrared->InfraredAlarm[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
			
			}
			stuInfrared[i].hEvent.iAOLatch = pInfrared->InfraredAlarm[i].struHandle.dwDuration;
			stuInfrared[i].hEvent.iRecordLatch = pInfrared->InfraredAlarm[i].struHandle.dwRecLatch;//把预录时间取消用来表示录像延时，变量名待改
			stuInfrared[i].hEvent.iEventLatch = pInfrared->InfraredAlarm[i].struHandle.dwEventLatch;
			stuInfrared[i].hEvent.bMessagetoNet = (BOOL)pInfrared->InfraredAlarm[i].struHandle.bMessageToNet;
			stuInfrared[i].hEvent.bMMSEn = pInfrared->InfraredAlarm[i].struHandle.bMMSEn;
			stuInfrared[i].hEvent.SnapshotTimes = pInfrared->InfraredAlarm[i].struHandle.bySnapshotTimes;//短信发送图片的张数
			stuInfrared[i].hEvent.bLog = (BOOL)pInfrared->InfraredAlarm[i].struHandle.bLog;
			stuInfrared[i].hEvent.bMatrixEn = (BOOL)pInfrared->InfraredAlarm[i].struHandle.bMatrixEn;
			stuInfrared[i].hEvent.dwMatrix = pInfrared->InfraredAlarm[i].struHandle.dwMatrix;
		}

		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_WIRELESS, 0, (char*)stuInfrared, nCount*sizeof(CONFIG_ALARM_INFRARED), waittime);
		if (nRet < 0)
		{
			return NET_ERROR_SETCFG_INFRARED;
		}

		Sleep(SETUP_SLEEP);
	}
	else
	{
		nFailedCount++;
	}
	if (nFailedCount >0)
	{
		return NET_ERROR_GETCFG_INFRARED;
	}

	CONFIG_WORKSHEET WorkSheet[16] = {0};
	nRet = GetDevConfig_WorkSheet(lLoginID, /*WSHEET_LOCAL_ALARM*/WSHEET_WIRELESS_ALARM, WorkSheet, waittime, nSheetNum);
	if (nRet >= 0)
	{
		for (int i = 0; i < nSheetNum; i++)
		{
			WorkSheet[i].iName = i;
			memcpy(WorkSheet[i].tsSchedule, pInfrared->InfraredAlarm[i].stSect, sizeof(WorkSheet[i].tsSchedule));
		}

		nRet = SetDevConfig_WorkSheet(lLoginID, /*WSHEET_LOCAL_ALARM*/WSHEET_WIRELESS_ALARM, WorkSheet, nSheetNum);
	}
	else
	{
		nRet = -1;
	}

	return nRet;
}

int CDevConfig::GetDevConfig_MailCfg(LONG lLoginID, DEVICE_MAIL_CFG *pstMail, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstMail == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRet = -1;

	unsigned char buffer[1024] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_MAIL, 0, (char*)buffer, 1024, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		char szMailInfo[1024] = {0};
		Change_Utf8_Assic(buffer, szMailInfo);
		return ParseMailStringEx(szMailInfo,(void*)pstMail);
		
	//	ParseMailString(szMailInfo, strlen(szMailInfo), &stNC.struMail);
	}
	else
	{
		//nRet = 
	}

	return nRet;
}
int CDevConfig::SetDevConfig_MailCfg(LONG lLoginID, DEVICE_MAIL_CFG *pstMail, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstMail == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0;
	unsigned char buffer[1024] = {0};
	int nRet = QueryConfig(lLoginID, CONFIG_TYPE_MAIL, 0, (char*)buffer, 1024, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		char szMailInfo[1024] = {0};
		Change_Utf8_Assic(buffer, szMailInfo);
		char szNewMailCfg[1024] ={0};
		int bRtn = BulidMailStringEx(szNewMailCfg,szMailInfo,pstMail);
		if( bRtn < 0 )
		{
			return NET_ERROR;
		}
		char szMailBuf[1024] ={0};
		Change_Assic_UTF8(szNewMailCfg,1024,szMailBuf,1024);
		bRtn = SetupConfig(lLoginID, CONFIG_TYPE_MAIL, 0, szMailBuf,strlen(szMailBuf), waittime);
	}
	else
	{
		nRet = NET_ERROR_GETCFG_MAIL;
	}
	
	return nRet;
}

int CDevConfig::GetDevConfig_DNSCfg(LONG lLoginID, DEVICE_DNS_CFG *pstDNS, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstDNS == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRet = -1;

	char buffer[512] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_DNS, 0, buffer, 512, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		CStrParse parser;
		parser.setSpliter("::");
		bool bSuccess = parser.Parse(buffer);
		if (!bSuccess)
		{
			return NET_RETURN_DATA_ERROR;
		}

		int nCount = parser.Size();
		if (nCount < 2)
		{
			return NET_RETURN_DATA_ERROR;
		}

		std::string priIpInfo = parser.getWord();
		
		CStrParse subParse;
		subParse.setSpliter("=");
		bSuccess = subParse.Parse(priIpInfo);
		if (!bSuccess)
		{
			return NET_RETURN_DATA_ERROR;
		}
		
		if (0 == _stricmp("PrimaryIp", subParse.getWord(0).c_str()))
		{
			strcpy(pstDNS->szPrimaryIp, subParse.getWord(1).c_str());
		}
		
		std::string secIpInfo = parser.getWord();
		
		subParse.setSpliter("=");
		bSuccess = subParse.Parse(secIpInfo);
		if (!bSuccess)
		{
			return NET_RETURN_DATA_ERROR;
		}
		
		if (0 == _stricmp("SecondaryIp", subParse.getWord(0).c_str()))
		{
			strcpy(pstDNS->szSecondaryIp, subParse.getWord(1).c_str());
		}
	}

	return nRet;
}

int CDevConfig::GetDevConfig_NTPCfg(LONG lLoginID, DEVICE_NTP_CFG *pstNTP, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstNTP == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRet = -1;

	memset(pstNTP, 0, sizeof(DEVICE_NTP_CFG));

	//	根据能力的版本号，有两种结构体
	BOOL bSupport = FALSE;
	int	nVer = 0;
	char buf[1024] = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ALL, buf, 1024, &retlen, waittime);
	if (nRet >= 0 && retlen > 0) 
	{
		CStrParse parser;
		parser.setSpliter("&&");
		bool bSuccess = parser.Parse(buf);
		if (!bSuccess) 
		{
			return -1;
		}

		for(int i = 0; i < parser.Size(); i++)
		{
			std::string info = parser.getWord();
			CStrParse subparse;
			subparse.setSpliter(":");
			bSuccess = subparse.Parse(info);
			if (!bSuccess) 
			{
				return -1;
			}

			if (0 == _stricmp(subparse.getWord(0).c_str(), "NTP"))
			{
				bSupport = TRUE;
				nVer = atoi(subparse.getWord(1).c_str());
				break;
			}
		}
	}

	if (!bSupport)
	{
		return -1;
	}

	//	根据不同的版本号，获取NTP信息
	if (nVer == 1)
	{
		CONFIG_NTP_PROTO_SET stNTP = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_NTP, 0, (char*)&stNTP, sizeof(CONFIG_NTP_PROTO_SET), &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(CONFIG_NTP_PROTO_SET))
		{
			pstNTP->bEnable = stNTP.Enable;
			pstNTP->nHostPort = stNTP.unHostPort;
			pstNTP->nTimeZone = stNTP.iTimeZone;
			pstNTP->nUpdateInterval = stNTP.nUpdateInterval;
			Ip2Str(stNTP.unHostIP, pstNTP->szHostIp);
			pstNTP->nType = 0;
		}
		else
		{
			nRet = -1;
		}
	}
	else if (nVer == 2)
	{
		memset(buf, 0, 1024);
		retlen = 0;
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_NTP, 0, buf, 1024, &retlen, waittime);
		if (nRet >= 0 && retlen > 0)
		{
			pstNTP->nType = 1;
			CStrParse parser;
			parser.setSpliter("\r\n");
			bool bSuccess = parser.Parse(buf);
			if (!bSuccess) 
			{
				return -1;
			}

			for(int i = 0; i < parser.Size()-2; i++)
			{
				std::string info = parser.getWord();
				CStrParse subparse;
				subparse.setSpliter(":");
				bSuccess = subparse.Parse(info);
				if (!bSuccess) 
				{
					return -1;
				}

				if (0 == _stricmp(subparse.getWord(0).c_str(), "Enable"))
				{
					if (0 == _stricmp(subparse.getWord(1).c_str(), "true"))
					{
						pstNTP->bEnable = TRUE;
					}
					else if (0 == _stricmp(subparse.getWord(1).c_str(), "false"))
					{
						pstNTP->bEnable = FALSE;
					}
				}

				if (0 == _stricmp(subparse.getWord(0).c_str(), "Host"))
				{
					strcpy(pstNTP->szDomainName, subparse.getWord(1).c_str());
				}

				if (0 == _stricmp(subparse.getWord(0).c_str(), "Port"))
				{
					pstNTP->nHostPort = atoi(subparse.getWord(1).c_str());
				}

				if (0 == _stricmp(subparse.getWord(0).c_str(), "UpdateInterval"))
				{
					pstNTP->nUpdateInterval = atoi(subparse.getWord(1).c_str());
				}

				if (0 == _stricmp(subparse.getWord(0).c_str(), "TimeZone"))
				{
					pstNTP->nTimeZone = atoi(subparse.getWord(1).c_str());
				}
			}
		}
		else
		{
			nRet = -1;
		}
	}
	else
	{
		nRet = -1;
	}
	
	return nRet;
}

int CDevConfig::SetDevConfig_NTPCfg(LONG lLoginID, DEVICE_NTP_CFG *pstNTP, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstNTP == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRet = -1;

	//	根据能力的版本号，有两种结构体
	BOOL bSupport = FALSE;
	int	nVer = 0;
	char buf[1024] = {0};
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ALL, buf, 1024, &retlen, waittime);
	if (nRet >= 0 && retlen > 0) 
	{
		CStrParse parser;
		parser.setSpliter("&&");
		bool bSuccess = parser.Parse(buf);
		if (!bSuccess) 
		{
			return -1;
		}

		for(int i = 0; i < parser.Size(); i++)
		{
			std::string info = parser.getWord();
			CStrParse subparse;
			subparse.setSpliter(":");
			bSuccess = subparse.Parse(info);
			if (!bSuccess) 
			{
				return -1;
			}

			if (0 == _stricmp(subparse.getWord(0).c_str(), "NTP"))
			{
				bSupport = TRUE;
				nVer = atoi(subparse.getWord(1).c_str());
				break;
			}
		}
	}

	if (!bSupport)
	{
		return -1;
	}

	//	根据不同的版本号，采取不同的配置
	if (nVer == 1)
	{
		CONFIG_NTP_PROTO_SET stNTP = {0};
		stNTP.Enable = pstNTP->bEnable ? 1:0;
		stNTP.unHostPort = (unsigned short)pstNTP->nHostPort;
		stNTP.iTimeZone = pstNTP->nTimeZone;
		stNTP.nUpdateInterval = pstNTP->nUpdateInterval;
		stNTP.unHostIP = Str2Ip(pstNTP->szHostIp);
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_NTP, 0, (char*)&stNTP, sizeof(CONFIG_NTP_PROTO_SET), waittime);
	}
	else if (nVer == 2)
	{
		int pos = 0;
		memset(buf, 0, 1024);
		if (pstNTP->bEnable)
		{
			sprintf(buf+pos, "Enable:true\r\n");
		}
		else
		{
			sprintf(buf+pos, "Enable:false\r\n");
		}

		pos = strlen(buf);
		sprintf(buf+pos, "Host:%s\r\n", pstNTP->szDomainName);

		pos = strlen(buf);
		sprintf(buf+pos, "Port:%d\r\n", pstNTP->nHostPort);

		pos = strlen(buf);
		sprintf(buf+pos, "UpdateInterval:%d\r\n", pstNTP->nUpdateInterval);

		pos = strlen(buf);
		sprintf(buf+pos, "TimeZone:%d\r\n", pstNTP->nTimeZone);

		pos = strlen(buf);
		sprintf(buf+pos, "\r\n");

		nRet = SetupConfig(lLoginID, CONFIG_TYPE_NTP, 0, buf, strlen(buf), waittime);
	}
	else
	{
		nRet = -1;
	}

	return nRet;
}

int CDevConfig::SetDevConfig_DNSCfg(LONG lLoginID, DEVICE_DNS_CFG *pstDNS, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstDNS == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRet = -1;

	char buffer[512] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_DNS, 0, buffer, 512, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		CStrParse parser;
		parser.setSpliter("::");
		bool bSuccess = parser.Parse(buffer);
		if (!bSuccess)
		{
			return NET_RETURN_DATA_ERROR;
		}

		int nCount = parser.Size();
		if (nCount < 2)
		{
			return NET_RETURN_DATA_ERROR;
		}

		memset(buffer, 0, 512);
		int nPos = 0;
		sprintf(buffer, "PrimaryIp=%s::SecondaryIp=%s", pstDNS->szPrimaryIp, pstDNS->szSecondaryIp);
		nPos = strlen(buffer);
		for (int i = 2; i < nCount; i++)
		{
			sprintf(buffer+nPos, "::%s", parser.getWord(i).c_str());
			nPos = strlen(buffer);
		}

		//修改
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_DNS, 0, buffer, nPos, waittime);
	}

	return nRet;
}
//End: zsc(11402)

int CDevConfig::GetDevConfig_NetCfg(LONG lLoginID, DEVICE_NET_CFG &stNC, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stNC, 0, sizeof(DEVICE_NET_CFG));
	stNC.dwSize = sizeof(DEVICE_NET_CFG);
	int retlen = 0;
	int nRet = -1;

	// 获取网络配置信息
	CONFIG_NET stNetCfg;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_NET, 0, (char *)&stNetCfg, sizeof(CONFIG_NET), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_NET))
	{
		memcpy(stNC.sDevName, stNetCfg.HostName, MAX_GENERAL_NAME_LEN);
		stNC.wTcpMaxConnectNum = stNetCfg.TCPMaxConn;
		stNC.wTcpPort = stNetCfg.TCPPort;
		stNC.wUdpPort = stNetCfg.UDPPort;
		stNC.wHttpPort = stNetCfg.HttpPort;
		stNC.wSslPort = stNetCfg.SSLPort;
		stNC.wHttpsPort = stNetCfg.HttpsPort;
		//stNC.byAlarmSvrStat = stNetCfg.AlmSvrStat;
		
		// 获取网卡信息
		int nEthernetNum = 1;
		CONFIG_ETHERNET stuEthernet[5] = {0};

		in_addr tmp = {0};
		tmp.s_addr = stNetCfg.HostIP;
		strcpy(stuEthernet[0].sDevIPAddr, inet_ntoa(tmp));
		
		tmp.s_addr = stNetCfg.Submask;
		strcpy(stuEthernet[0].sDevIPMask, inet_ntoa(tmp));
		
		tmp.s_addr = stNetCfg.GateWayIP;
		strcpy(stuEthernet[0].sGatewayIP, inet_ntoa(tmp));
		
		stuEthernet[0].bTranMedia = 0;
		stuEthernet[0].bValid = 1;
		stuEthernet[0].bDefaultEth = 1;
		strcpy(stuEthernet[0].szEthernetName, "eth0");
		strcpy(stuEthernet[0].szSpeed, "100Mbps");
		strcpy(stuEthernet[0].szDuplex, "Auto");
		
		GetDevConfig_EthernetCfg(lLoginID, stuEthernet, nEthernetNum, waittime);
		GetDevConfig_EtherDHCPCfg(lLoginID, stuEthernet, waittime);

		// 取前两块网卡信息
		nEthernetNum = Min(nEthernetNum, 2);
		for (int i = 0; i < nEthernetNum; i++)
		{
			strcpy(stNC.stEtherNet[i].sDevIPAddr, stuEthernet[i].sDevIPAddr);
			strcpy(stNC.stEtherNet[i].sDevIPMask, stuEthernet[i].sDevIPMask);
			strcpy(stNC.stEtherNet[i].sGatewayIP, stuEthernet[i].sGatewayIP);
			strcpy(stNC.stEtherNet[i].byMACAddr, stuEthernet[i].byMACAddr);
			stNC.stEtherNet[i].bTranMedia = stuEthernet[i].bTranMedia;
			stNC.stEtherNet[i].bDefaultEth = stuEthernet[i].bDefaultEth;
			stNC.stEtherNet[i].bValid = stuEthernet[i].bValid | (stuEthernet[i].bDHCPEnable<<1) | (stuEthernet[i].bDHCPFunc<<2);
		
			if (0 == _stricmp(stuEthernet[i].szSpeed, "10Mbps") && 0 == _stricmp(stuEthernet[i].szDuplex, "Full"))
			{
				stNC.stEtherNet[i].dwNetInterface = 1;
			}
			else if (0 == _stricmp(stuEthernet[i].szSpeed, "10Mbps") && 0 == _stricmp(stuEthernet[i].szDuplex, "Auto"))
			{
				stNC.stEtherNet[i].dwNetInterface = 2;
			}
			else if (0 == _stricmp(stuEthernet[i].szSpeed, "10Mbps") && 0 == _stricmp(stuEthernet[i].szDuplex, "Half"))
			{
				stNC.stEtherNet[i].dwNetInterface = 3;
			}
			else if (0 == _stricmp(stuEthernet[i].szSpeed, "100Mbps") && 0 == _stricmp(stuEthernet[i].szDuplex, "Full"))
			{
				stNC.stEtherNet[i].dwNetInterface = 4;
			}
			else if (0 == _stricmp(stuEthernet[i].szSpeed, "100Mbps") && 0 == _stricmp(stuEthernet[i].szDuplex, "Auto"))
			{
				stNC.stEtherNet[i].dwNetInterface = 5;
			}
			else if (0 == _stricmp(stuEthernet[i].szSpeed, "100Mbps") && 0 == _stricmp(stuEthernet[i].szDuplex, "Half"))
			{
				stNC.stEtherNet[i].dwNetInterface = 6;
			}
			else
			{
				stNC.stEtherNet[i].dwNetInterface = 7;
			}
		}

		//add by cqs 090702 记录网卡信息，为了ppoe支持多网卡
		device->set_info(device, dit_ether_info, &stuEthernet);

		//get mac address, not supported yet
		/*	char mac[MACADDR_LEN];
		nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ID, mac, MACADDR_LEN, &retlen, waittime);
		if (nRet >= 0)
		{
		mac[MACADDR_LEN-1] = '\0';
		memcpy(stNC.stEtherNet[0].byMACAddr, mac, MACADDR_LEN);
	}*/				
		//Alarm server info
		tmp.s_addr = stNetCfg.AlarmServerIP;
		strcpy(stNC.struAlarmHost.sHostIPAddr, inet_ntoa(tmp));
		stNC.struAlarmHost.wHostPort = stNetCfg.AlarmServerPort;
		stNC.struAlarmHost.byEnable = stNetCfg.AlmSvrStat;
		
		//Log server info
		tmp.s_addr = stNetCfg.LogServerIP;
		strcpy(stNC.struLogHost.sHostIPAddr, inet_ntoa(tmp));
		stNC.struLogHost.wHostPort = stNetCfg.LogServerPort;
		
		//SMTP server info
		tmp.s_addr = stNetCfg.SMTPServerIP;
		strcpy(stNC.struSmtpHost.sHostIPAddr, inet_ntoa(tmp));
		stNC.struSmtpHost.wHostPort = stNetCfg.SMTPServerPort;
		
		//Multicast server info
		tmp.s_addr = stNetCfg.McastIP;
		strcpy(stNC.struMultiCast.sHostIPAddr, inet_ntoa(tmp));
		stNC.struMultiCast.wHostPort = stNetCfg.McastPort;
		
		//DNS server info
		tmp.s_addr = stNetCfg.DNSIP;
		strcpy(stNC.struDns.sHostIPAddr, inet_ntoa(tmp));
		
		//DDNS
		//PPPoE
		char strPPPoE[128] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_PPPOE, 0, (char *)strPPPoE, 128, &retlen, waittime);
		if (nRet >= 0 && retlen <= 128)
		{
			ParseHostString(0, strPPPoE, retlen, &stNC.struPppoe, stNC.sPppoeIP, lLoginID);
		}
		
		char strDDNS[128] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_DDNS, 0, (char *)strDDNS, 128, &retlen, waittime);
		if (nRet >= 0 && retlen <= 128)
		{
			ParseHostString(1, strDDNS, retlen, &stNC.struDdns, stNC.sDdnsHostName, lLoginID);
		}
		//Mail
		char strMail[1024] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_MAIL, 0, (char *)strMail, 1024, &retlen, waittime);
		if (nRet >= 0 && retlen <= 1024)
		{
			unsigned char szMailInfo[1024] = {0};
			memcpy(szMailInfo, strMail, retlen);
			memset(strMail, 0, 1024);
			Change_Utf8_Assic(szMailInfo, strMail);
			ParseMailString(strMail, strlen(strMail), &stNC.struMail);
		}

		/*
		//FTP
		CONFIG_FTP_PROTO_SET stFtpCfg;
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_FTP, 0, (char *)&stFtpCfg, sizeof(CONFIG_FTP_PROTO_SET), &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(CONFIG_FTP_PROTO_SET))
		{
			stNC.struFtpServer.byEnable = stFtpCfg.m_isEnable;
			in_addr ia;
			ia.s_addr = stFtpCfg.m_unHostIP;
			strcpy(stNC.struFtpServer.sHostIPAddr, inet_ntoa(ia));
			stNC.struFtpServer.wHostPort = stFtpCfg.m_nHostPort;
			int usernamelen = min(MAX_HOSTNAME_LEN, MAX_USERNAME_LEN);
			memcpy(stNC.struFtpServer.sHostUser, stFtpCfg.m_cUserName, usernamelen);
			int userpswlen = min(MAX_HOSTPSW_LEN, MAX_PASSWORD_LEN);
			memcpy(stNC.struFtpServer.sHostPassword, stFtpCfg.m_cPassword, userpswlen);

			int ftpdirlen = min(FTP_MAXDIRLEN, 240);
			memcpy(stNC.struFtpParam.sFtpDir, stFtpCfg.m_cDirName, ftpdirlen);
			stNC.struFtpParam.dwFileLen = stFtpCfg.m_iFileLen;
			stNC.struFtpParam.dwIterval = stFtpCfg.m_iInterval;
		}
		*/
	}
	else
	{
		return NET_ERROR_GETCFG_NETCFG;
	}

	return 0;
}

int	CDevConfig::GetDevConfig_ChnCfg(LONG lLoginID, DEV_CHANNEL_CFG &stCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	if (lChannel < 0 || lChannel >= device->channelcount(device))
	{
		return -1;
	}

	int nMultiPlayFlag = 0;
	device->get_info(device, dit_multiplay_flag, &nMultiPlayFlag);
	
	memset(&stCHC, 0, sizeof(DEV_CHANNEL_CFG));
	stCHC.dwSize = sizeof(DEV_CHANNEL_CFG);
	stCHC.byBlindMask |= nMultiPlayFlag==1? 0x01:0x00;
	stCHC.byBlindMask |= 0x02;
	int retlen = 0;
	int nRet = -1;
	
	//get channel name
	char buf[16*CHANNEL_NAME_LEN] = {0};		
	nRet = QueryChannelName(lLoginID, buf, 16*CHANNEL_NAME_LEN, &retlen, waittime);	
	if (nRet >= 0)
	{
		memcpy(stCHC.szChannelName, &buf[CHANNEL_NAME_LEN*lChannel], CHANNEL_NAME_LEN);
	}
	else
	{
		return NET_ERROR_GETCFG_CHANNAME;
	}
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(NEW_CONFIG_CAPTURE))//127#子命令
	{
		iRecvBufLen = 16 * sizeof(NEW_CONFIG_CAPTURE);
	}
	if (iRecvBufLen < 16*sizeof(CONFIG_CAPTURE_V2201))//128#子命令
	{
		iRecvBufLen = 16*sizeof(CONFIG_CAPTURE_V2201);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_CAPTURE))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_CAPTURE);
	}
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}

	int iProtocolVersion;
	device->get_info(device, dit_protocol_version, &iProtocolVersion);

	if (iProtocolVersion < 5)
	{
		//get video parameters
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && retlen == 16 * sizeof(CONFIG_CAPTURE))
		{
			CONFIG_CAPTURE *stPic = (CONFIG_CAPTURE *)(cRecvBuf + lChannel*sizeof(CONFIG_CAPTURE));
			stCHC.stColorCfg[0].byBrightness = stPic->Brightness;
			stCHC.stColorCfg[0].byContrast = stPic->Contrast;
			if (stPic->Gain >= 128)
			{
				stCHC.stColorCfg[0].byGainEn = 0;
				stCHC.stColorCfg[0].byGain = stPic->Gain-128;
			}
			else
			{
				stCHC.stColorCfg[0].byGainEn = 1;
				stCHC.stColorCfg[0].byGain = stPic->Gain;
			}
			stCHC.stColorCfg[0].byGain = stPic->Gain;
			stCHC.stColorCfg[0].bySaturation = stPic->Saturation;
			stCHC.stColorCfg[0].byHue = stPic->Hue;
			
			stCHC.stMainVideoEncOpt[0].byAudioEnable = (stPic->AudioEn)?1:0;
			stCHC.stMainVideoEncOpt[0].byVideoEnable = 0x01;	//vedio default enabled
			stCHC.stMainVideoEncOpt[0].byImageSize = stPic->CifMode;
			stCHC.stMainVideoEncOpt[0].byBitRateControl = stPic->EncodeMode; //待确定变量名
			stCHC.stMainVideoEncOpt[0].byEncodeMode = stPic->VideoType; //待确定变量名
			stCHC.stMainVideoEncOpt[0].byFramesPerSec = stPic->Frames;
			stCHC.stMainVideoEncOpt[0].byImageQlty = stPic->ImgQlty;
			stCHC.byBlindEnable = stPic->CoverEnable;
			memcpy(&stCHC.stBlindCover[0].rcRect, &stPic->Cover, sizeof(YW_RECT));
		}
		else
		{
			return NET_ERROR_GETCFG_VIDEO;
		}
		
		//视频参数128号子命令
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_128, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && retlen == 16*sizeof(CONFIG_CAPTURE_V2201))
		{
			CONFIG_CAPTURE_V2201 *pCfgCap = (CONFIG_CAPTURE_V2201 *)(cRecvBuf+lChannel*sizeof(CONFIG_CAPTURE_V2201));
			int i = 0, j = 0;
			int iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
			for (i = 0; i < 1/*iMinIndex*/; i++)
			{
				stCHC.stMainVideoEncOpt[i].byVideoEnable = pCfgCap->VideoEn;
				stCHC.stMainVideoEncOpt[i].byAudioEnable = pCfgCap->AudioEn;
				stCHC.stMainVideoEncOpt[i].byImageSize = pCfgCap->CifMode;
				stCHC.stMainVideoEncOpt[i].byBitRateControl = pCfgCap->EncodeMode;
				stCHC.stMainVideoEncOpt[i].byFramesPerSec = pCfgCap->Frames;
				stCHC.stMainVideoEncOpt[i].byImageQlty = pCfgCap->ImgQlty;
				stCHC.stMainVideoEncOpt[i].byEncodeMode = pCfgCap->VideoType;
			}
			
			stCHC.byBlindEnable = pCfgCap->CoverEnable;
			memcpy(&stCHC.stBlindCover[0].rcRect, &pCfgCap->Cover, sizeof(YW_RECT));
			
			stCHC.stTimeOSD.bShow = pCfgCap->TimeTilteEn;
			stCHC.stTimeOSD.rcRect.left = pCfgCap->TimeTitleL;
			stCHC.stTimeOSD.rcRect.right = pCfgCap->TimeTitleR;
			stCHC.stTimeOSD.rcRect.top = pCfgCap->TimeTitleT;
			stCHC.stTimeOSD.rcRect.bottom = pCfgCap->TimeTitleB;
			
			stCHC.stChannelOSD.bShow = pCfgCap->ChTitleEn;
			stCHC.stChannelOSD.rcRect.left = pCfgCap->ChannelTitleL;
			stCHC.stChannelOSD.rcRect.right = pCfgCap->ChannelTitleR;
			stCHC.stChannelOSD.rcRect.top = pCfgCap->ChannelTitleT;
			stCHC.stChannelOSD.rcRect.bottom = pCfgCap->ChannelTitleB;
			
			stCHC.stColorCfg[0].byBrightness = pCfgCap->Brightness;
			stCHC.stColorCfg[0].byContrast = pCfgCap->Contrast;
			if (pCfgCap->Gain >= 128)
			{
				stCHC.stColorCfg[0].byGainEn = 0;
				stCHC.stColorCfg[0].byGain = pCfgCap->Gain-128;
			}
			else
			{
				stCHC.stColorCfg[0].byGainEn = 1;
				stCHC.stColorCfg[0].byGain = pCfgCap->Gain;
			}
			stCHC.stColorCfg[0].byHue = pCfgCap->Hue;
			stCHC.stColorCfg[0].bySaturation = pCfgCap->Saturation;
			
			pCfgCap++;
		}
		else
		{
			nRet = NET_NOERROR;
		}
		
		/*
		//get audio_capture_cfg
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_AUDIO_CAPTURE, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && 16*sizeof(AUDIO_CAPTURE_CFG) == retlen)
		{
		AUDIO_CAPTURE_CFG *pAdoCap = (AUDIO_CAPTURE_CFG *)(cRecvBuf + lChannel*sizeof(NEW_CONFIG_CAPTURE));
		int i = 0, j = 0;
		int iMinIndex = 0;
		
		  iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
		  for (j = 0; j < 1; j++)
		  {
		  stCHC.stMainVideoEncOpt[j].nChannels = pAdoCap->nChannels;
		  stCHC.stMainVideoEncOpt[j].nSamplesPerSec = pAdoCap->nSamplesPerSec;
		  stCHC.stMainVideoEncOpt[j].wBitsPerSample = pAdoCap->dwBitsPerSample;
		  stCHC.stMainVideoEncOpt[j].wFormatTag = pAdoCap->wFormatTag;
		  }
		  }
		*/
	} //if (iProtocolVersion < 5)

	//new capture config, allowed to fail
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && (retlen == 16 * sizeof(NEW_CONFIG_CAPTURE) || retlen == device->channelcount(device) * sizeof(NEW_CONFIG_CAPTURE)))
	{
		NEW_CONFIG_CAPTURE *stNewPic = 0;
		int i = 0;
		int j = 0;
		int iMinIndex = 0;
		
		stNewPic = (NEW_CONFIG_CAPTURE *)(cRecvBuf + lChannel*sizeof(NEW_CONFIG_CAPTURE));
		iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
		for (i = 0; i < iMinIndex; i++)
		{
			stCHC.stMainVideoEncOpt[i].byVideoEnable = stNewPic->MainOption[i].AVEnable & 0x01;
			stCHC.stMainVideoEncOpt[i].byAudioEnable = (stNewPic->MainOption[i].AVEnable>>1) & 0x01;
			stCHC.stMainVideoEncOpt[i].bAudioOverlay = (stNewPic->MainOption[i].AVEnable>>2) & 0x01;
			stCHC.stMainVideoEncOpt[i].byImageSize = stNewPic->MainOption[i].ImageSize;
			stCHC.stMainVideoEncOpt[i].byBitRateControl = stNewPic->MainOption[i].BitRateControl;
			stCHC.stMainVideoEncOpt[i].byFramesPerSec = stNewPic->MainOption[i].Frames;
			stCHC.stMainVideoEncOpt[i].byImageQlty = stNewPic->MainOption[i].ImgQlty;
			stCHC.stMainVideoEncOpt[i].byEncodeMode = stNewPic->Compression;
			//	Add by zsc(11402) 2008-1-18
			stCHC.stMainVideoEncOpt[i].wLimitStream = stNewPic->MainOption[i].usBitRate;
			stCHC.stMainVideoEncOpt[i].bIFrameInterval = stNewPic->MainOption[i].IFrameInterval;
			//stCHC.stMainVideoEncOpt[i].bScanMode = stNewPic->MainScanMode[i];
		}
		iMinIndex = N_ENCODE_AUX_INTER<ENCODE_AUX_NUM?N_ENCODE_AUX_INTER:ENCODE_AUX_NUM;
		for (i = 0; i < iMinIndex; i++)
		{
			stCHC.stAssiVideoEncOpt[i].byVideoEnable = stNewPic->AssiOption[i].AVEnable & 0x01;
			stCHC.stAssiVideoEncOpt[i].byAudioEnable = (stNewPic->AssiOption[i].AVEnable>>1) & 0x01;	
			stCHC.stAssiVideoEncOpt[i].bAudioOverlay = (stNewPic->AssiOption[i].AVEnable>>2) & 0x01;
			stCHC.stAssiVideoEncOpt[i].byImageSize = stNewPic->AssiOption[i].ImageSize;
			stCHC.stAssiVideoEncOpt[i].byBitRateControl = stNewPic->AssiOption[i].BitRateControl;
			stCHC.stAssiVideoEncOpt[i].byFramesPerSec = stNewPic->AssiOption[i].Frames;
			stCHC.stAssiVideoEncOpt[i].byImageQlty = stNewPic->AssiOption[i].ImgQlty;
			stCHC.stAssiVideoEncOpt[i].byEncodeMode = stNewPic->Compression;
			//	Add by zsc(11402) 2008-1-18
			stCHC.stAssiVideoEncOpt[i].wLimitStream = stNewPic->AssiOption[i].usBitRate;
			stCHC.stAssiVideoEncOpt[i].bIFrameInterval = stNewPic->AssiOption[i].IFrameInterval;
			//stCHC.stAssiVideoEncOpt[i].bScanMode = stNewPic->AssiScanMode[i];
		}
		stCHC.byBlindEnable = stNewPic->CoverEnable;
		stCHC.bNoise = stNewPic->Noise;
		stCHC.bVolume = stNewPic->Volume;
		stCHC.bVolumeEnable = stNewPic->VolumeEnable;
		
		memcpy(&stCHC.stBlindCover[0].rcRect, &stNewPic->Cover, sizeof(YW_RECT));
		
		stCHC.stTimeOSD.bShow = stNewPic->TimeTitle.TitleEnable;
		stCHC.stTimeOSD.rgbaFrontground = stNewPic->TimeTitle.TitlefgRGBA;
		stCHC.stTimeOSD.rgbaBackground = stNewPic->TimeTitle.TitlebgRGBA;
		stCHC.stTimeOSD.rcRect.left = stNewPic->TimeTitle.TitleLeft;
		stCHC.stTimeOSD.rcRect.right = stNewPic->TimeTitle.TitleRight;
		stCHC.stTimeOSD.rcRect.top = stNewPic->TimeTitle.TitleTop;
		stCHC.stTimeOSD.rcRect.bottom = stNewPic->TimeTitle.TitleBottom;
		
		stCHC.stChannelOSD.bShow = stNewPic->ChannelTitle.TitleEnable;
		stCHC.stChannelOSD.rgbaFrontground = stNewPic->ChannelTitle.TitlefgRGBA;
		stCHC.stChannelOSD.rgbaBackground = stNewPic->ChannelTitle.TitlebgRGBA;
		stCHC.stChannelOSD.rcRect.left = stNewPic->ChannelTitle.TitleLeft;
		stCHC.stChannelOSD.rcRect.right = stNewPic->ChannelTitle.TitleRight;
		stCHC.stChannelOSD.rcRect.top = stNewPic->ChannelTitle.TitleTop;
		stCHC.stChannelOSD.rcRect.bottom = stNewPic->ChannelTitle.TitleBottom;
	}	
	else
	{
		nRet = NET_NOERROR; //acceptable
	}

	if (iProtocolVersion > 2)
	{
		//get color config with time section
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_COLOR, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if(nRet >= 0 && 16*sizeof(CONFIG_COLOR) == retlen)
		{
			CONFIG_COLOR *pColorCfg = (CONFIG_COLOR *)(cRecvBuf + lChannel*sizeof(CONFIG_COLOR));
			int i = 0, j = 0;
			int iMinIndex = 0;
			iMinIndex = COL_TIME_SECT_NUM<N_COLOR_SECTION?COL_TIME_SECT_NUM:N_COLOR_SECTION;
			for (j = 0; j < iMinIndex; j++)
			{
				stCHC.stColorCfg[j].byBrightness = pColorCfg->Color[j].Brightness;// ->Brightness;
				stCHC.stColorCfg[j].byContrast = pColorCfg->Color[j].Contrast;//pCfgCap->Contrast;
				if (pColorCfg->Color[j].Gain >= 128)
				{
					stCHC.stColorCfg[j].byGainEn = 0;
					stCHC.stColorCfg[j].byGain = pColorCfg->Color[j].Gain-128;
				}
				else
				{
					stCHC.stColorCfg[j].byGainEn = 1;
					stCHC.stColorCfg[j].byGain = pColorCfg->Color[j].Gain;
				}
				stCHC.stColorCfg[j].byHue = pColorCfg->Color[j].Hue;// pCfgCap->Hue;
				stCHC.stColorCfg[j].bySaturation = pColorCfg->Color[j].Saturation;//pCfgCap->Saturation;
				stCHC.stColorCfg[j].stSect.bEnable = pColorCfg->Color[j].Sector.State;
				stCHC.stColorCfg[j].stSect.iBeginHour = pColorCfg->Color[j].Sector.StartHour;
				stCHC.stColorCfg[j].stSect.iBeginMin = pColorCfg->Color[j].Sector.StartMin;
				stCHC.stColorCfg[j].stSect.iBeginSec = pColorCfg->Color[j].Sector.StartSec;
				stCHC.stColorCfg[j].stSect.iHourEnd = pColorCfg->Color[j].Sector.HourEnd;
				stCHC.stColorCfg[j].stSect.iEndMin = pColorCfg->Color[j].Sector.EndMin;
				stCHC.stColorCfg[j].stSect.iEndSec = pColorCfg->Color[j].Sector.EndSec;
			}
		}
		else
		{
			nRet = NET_NOERROR;
		}
	}

END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return 0;
}

int	CDevConfig::GetDevConfig_AllChnCfg(LONG lLoginID, DEV_CHANNEL_CFG *pstCHC, LONG lChnNum, int waittime)
{
	if (!pstCHC || lChnNum < 0 || lChnNum > 16)
	{
		return -1;
	}

	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	int nMultiPlayFlag = 0;
	device->get_info(device, dit_multiplay_flag, &nMultiPlayFlag);
	
	memset(pstCHC, 0, lChnNum*sizeof(DEV_CHANNEL_CFG));
	for (int i = 0; i < lChnNum; i++)
	{
		pstCHC[i].dwSize = sizeof(DEV_CHANNEL_CFG);
		pstCHC[i].byBlindMask |= nMultiPlayFlag==1? 0x01:0x00;
		pstCHC[i].byBlindMask |= 0x02;
	}
	int retlen = 0;
	int nRet = -1;

	//get channel name
//	char buf[16*CHANNEL_NAME_LEN] = {0};
	char buf[16*64+32] = {0};	//支持新通道名协议
	nRet = QueryChannelName(lLoginID, buf, /*16*CHANNEL_NAME_LEN*/16*64+32, &retlen, waittime);	
	if (nRet >= 0)
	{
		for (int i = 0; i < lChnNum; i++)
		{
			memcpy(pstCHC[i].szChannelName, &buf[i*CHANNEL_NAME_LEN], CHANNEL_NAME_LEN);
		}
	}
	else
	{
		return NET_ERROR_GETCFG_CHANNAME;
	}
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(NEW_CONFIG_CAPTURE))//127#子命令
	{
		iRecvBufLen = 16 * sizeof(NEW_CONFIG_CAPTURE);
	}
	if (iRecvBufLen < 16*sizeof(CONFIG_CAPTURE_V2201))//128#子命令
	{
		iRecvBufLen = 16*sizeof(CONFIG_CAPTURE_V2201);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_CAPTURE))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_CAPTURE);
	}
	if (iRecvBufLen < 16*sizeof(AUDIO_CAPTURE_CFG))
	{
		iRecvBufLen = 16*sizeof(AUDIO_CAPTURE_CFG);
	}
	if (iRecvBufLen < 16*sizeof(CONFIG_COLOR))
	{
		iRecvBufLen = 16*sizeof(CONFIG_COLOR);
	}
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	int iProtocolVersion;
	device->get_info(device, dit_protocol_version, &iProtocolVersion);

	if (iProtocolVersion < 5)
	{
		//get video parameters
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && retlen == 16 * sizeof(CONFIG_CAPTURE))
		{
			CONFIG_CAPTURE *stPic = 0;
			for (int i = 0; i < lChnNum; i++)
			{
				stPic = (CONFIG_CAPTURE *)(cRecvBuf + i*sizeof(CONFIG_CAPTURE));
				pstCHC[i].stColorCfg[0].byBrightness = stPic->Brightness;
				pstCHC[i].stColorCfg[0].byContrast = stPic->Contrast;
				if (stPic->Gain >= 128)
				{
					pstCHC[i].stColorCfg[0].byGainEn = 0;
					pstCHC[i].stColorCfg[0].byGain = stPic->Gain-128;
				}
				else
				{
					pstCHC[i].stColorCfg[0].byGainEn = 1;
					pstCHC[i].stColorCfg[0].byGain = stPic->Gain;
				}
				pstCHC[i].stColorCfg[0].bySaturation = stPic->Saturation;
				pstCHC[i].stColorCfg[0].byHue = stPic->Hue;
				
				pstCHC[i].stMainVideoEncOpt[0].byAudioEnable = (stPic->AudioEn)?1:0;
				pstCHC[i].stMainVideoEncOpt[0].byVideoEnable = 0x01;	//vedio default enabled
				pstCHC[i].stMainVideoEncOpt[0].byImageSize = stPic->CifMode;
				pstCHC[i].stMainVideoEncOpt[0].byBitRateControl = stPic->EncodeMode; //待确定变量名
				pstCHC[i].stMainVideoEncOpt[0].byEncodeMode = stPic->VideoType; //待确定变量名
				pstCHC[i].stMainVideoEncOpt[0].byFramesPerSec = stPic->Frames;
				pstCHC[i].stMainVideoEncOpt[0].byImageQlty = stPic->ImgQlty;
				pstCHC[i].byBlindEnable = stPic->CoverEnable;
				memcpy(&pstCHC[i].stBlindCover[0].rcRect, &stPic->Cover, sizeof(YW_RECT));
			}//for (int i = 0; i < chanNum; i++)
		}
		else
		{
			//allowed failure!
			//return NET_ERROR_GETCFG_VIDEO;
		}

		//视频参数128号子命令
		memset(cRecvBuf, 0, iRecvBufLen);
		//	int iii ;
		//	iii = 16*sizeof(CONFIG_CAPTURE_V2201);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_128, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && retlen == 16*sizeof(CONFIG_CAPTURE_V2201))
		{
			CONFIG_CAPTURE_V2201 *pCfgCap = (CONFIG_CAPTURE_V2201 *)cRecvBuf;
			int i = 0, j = 0;
			int iMinIndex = 0;
			for (i = 0; i < lChnNum; i++)
			{
				iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
				for (j = 0; j < 1/*iMinIndex*/; j++)
				{
					pstCHC[i].stMainVideoEncOpt[j].byVideoEnable = pCfgCap->VideoEn;
					pstCHC[i].stMainVideoEncOpt[j].byAudioEnable = pCfgCap->AudioEn;
					pstCHC[i].stMainVideoEncOpt[j].byImageSize = pCfgCap->CifMode;
					pstCHC[i].stMainVideoEncOpt[j].byBitRateControl = pCfgCap->EncodeMode;
					pstCHC[i].stMainVideoEncOpt[j].byFramesPerSec = pCfgCap->Frames;
					pstCHC[i].stMainVideoEncOpt[j].byImageQlty = pCfgCap->ImgQlty;
					pstCHC[i].stMainVideoEncOpt[j].byEncodeMode = pCfgCap->VideoType;
				}
				
				pstCHC[i].byBlindEnable = pCfgCap->CoverEnable;
				memcpy(&pstCHC[i].stBlindCover[0].rcRect, &pCfgCap->Cover, sizeof(YW_RECT));
				
				pstCHC[i].stTimeOSD.bShow = pCfgCap->TimeTilteEn;
				pstCHC[i].stTimeOSD.rcRect.left = pCfgCap->TimeTitleL;
				pstCHC[i].stTimeOSD.rcRect.right = pCfgCap->TimeTitleR;
				pstCHC[i].stTimeOSD.rcRect.top = pCfgCap->TimeTitleT;
				pstCHC[i].stTimeOSD.rcRect.bottom = pCfgCap->TimeTitleB;
				
				pstCHC[i].stChannelOSD.bShow = pCfgCap->ChTitleEn;
				pstCHC[i].stChannelOSD.rcRect.left = pCfgCap->ChannelTitleL;
				pstCHC[i].stChannelOSD.rcRect.right = pCfgCap->ChannelTitleR;
				pstCHC[i].stChannelOSD.rcRect.top = pCfgCap->ChannelTitleT;
				pstCHC[i].stChannelOSD.rcRect.bottom = pCfgCap->ChannelTitleB;
				
				pstCHC[i].stColorCfg[0].byBrightness = pCfgCap->Brightness;
				pstCHC[i].stColorCfg[0].byContrast = pCfgCap->Contrast;
				if (pCfgCap->Gain >= 128)
				{
					pstCHC[i].stColorCfg[0].byGainEn = 0;
					pstCHC[i].stColorCfg[0].byGain = pCfgCap->Gain-128;
				}
				else
				{
					pstCHC[i].stColorCfg[0].byGainEn = 1;
					pstCHC[i].stColorCfg[0].byGain = pCfgCap->Gain;
				}
				pstCHC[i].stColorCfg[0].byHue = pCfgCap->Hue;
				pstCHC[i].stColorCfg[0].bySaturation = pCfgCap->Saturation;
				
				pCfgCap++;
			}//for (int i = 0; i < chanNum; i++)
		}
		else
		{
			nRet = NET_NOERROR;
		}
		
		/*
		//get audio_capture_cfg
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_AUDIO_CAPTURE, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && 16*sizeof(AUDIO_CAPTURE_CFG) == retlen)
		{
			AUDIO_CAPTURE_CFG *pAdoCap = (AUDIO_CAPTURE_CFG *)cRecvBuf;
			int i = 0, j = 0;
			int iMinIndex = 0;
			for (i = 0; i < lChnNum; i++)
			{
				iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
				for (j = 0; j < 1; j++)
				{
					pstCHC[i].stMainVideoEncOpt[j].nChannels = pAdoCap->nChannels;
					pstCHC[i].stMainVideoEncOpt[j].nSamplesPerSec = pAdoCap->nSamplesPerSec;
					pstCHC[i].stMainVideoEncOpt[j].wBitsPerSample = pAdoCap->dwBitsPerSample;
					pstCHC[i].stMainVideoEncOpt[j].wFormatTag = pAdoCap->wFormatTag;
				}
				
				pAdoCap++;
			}
		}
		else
		{
			nRet = NET_NOERROR;
		}
		*/
		
	} //if (iProtocolVersion < 5)

	//new capture config, allowed to fail
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && (retlen == 16 * sizeof(NEW_CONFIG_CAPTURE) || retlen == device->channelcount(device) * sizeof(NEW_CONFIG_CAPTURE)))
	{
		NEW_CONFIG_CAPTURE *stNewPic = 0;
		int i = 0;
		int j = 0;
		int iMinIndex = 0;
		
		for (i = 0; i < lChnNum; i++)
		{
			stNewPic = (NEW_CONFIG_CAPTURE *)(cRecvBuf + i*sizeof(NEW_CONFIG_CAPTURE));
			iMinIndex = REC_TYPE_NUM<REC_TYP_NUM_INTER?REC_TYPE_NUM:REC_TYP_NUM_INTER;
			for (j = 0; j < iMinIndex; j++)
			{
				pstCHC[i].stMainVideoEncOpt[j].byVideoEnable = stNewPic->MainOption[j].AVEnable & 0x01;
				pstCHC[i].stMainVideoEncOpt[j].byAudioEnable = (stNewPic->MainOption[j].AVEnable>>1) & 0x01;							
				pstCHC[i].stMainVideoEncOpt[j].bAudioOverlay = (stNewPic->MainOption[j].AVEnable>>2) & 0x01;							
				pstCHC[i].stMainVideoEncOpt[j].byImageSize = stNewPic->MainOption[j].ImageSize;
				pstCHC[i].stMainVideoEncOpt[j].byBitRateControl = stNewPic->MainOption[j].BitRateControl;
				pstCHC[i].stMainVideoEncOpt[j].byFramesPerSec = stNewPic->MainOption[j].Frames;
				pstCHC[i].stMainVideoEncOpt[j].byImageQlty = stNewPic->MainOption[j].ImgQlty;
				pstCHC[i].stMainVideoEncOpt[j].byEncodeMode = stNewPic->Compression;
				//	Add by zsc(11402) 2008-1-18
				pstCHC[i].stMainVideoEncOpt[j].wLimitStream = stNewPic->MainOption[j].usBitRate;
				pstCHC[i].stMainVideoEncOpt[j].bIFrameInterval = stNewPic->MainOption[j].IFrameInterval;
				//pstCHC[i].stMainVideoEncOpt[j].bScanMode = stNewPic->MainScanMode[j];
			}
			iMinIndex = N_ENCODE_AUX_INTER<ENCODE_AUX_NUM?N_ENCODE_AUX_INTER:ENCODE_AUX_NUM;
			for (j = 0; j < iMinIndex; j++)
			{
				pstCHC[i].stAssiVideoEncOpt[j].byVideoEnable = stNewPic->AssiOption[j].AVEnable & 0x01;
				pstCHC[i].stAssiVideoEncOpt[j].byAudioEnable = (stNewPic->AssiOption[j].AVEnable>>1) & 0x01;			
				pstCHC[i].stAssiVideoEncOpt[j].bAudioOverlay = (stNewPic->AssiOption[j].AVEnable>>2) & 0x01;			
				pstCHC[i].stAssiVideoEncOpt[j].byImageSize = stNewPic->AssiOption[j].ImageSize;
				pstCHC[i].stAssiVideoEncOpt[j].byBitRateControl = stNewPic->AssiOption[j].BitRateControl;
				pstCHC[i].stAssiVideoEncOpt[j].byFramesPerSec = stNewPic->AssiOption[j].Frames;
				pstCHC[i].stAssiVideoEncOpt[j].byImageQlty = stNewPic->AssiOption[j].ImgQlty;
				pstCHC[i].stAssiVideoEncOpt[j].byEncodeMode = stNewPic->Compression;
				//	Add by zsc(11402) 2008-1-18
				pstCHC[i].stAssiVideoEncOpt[j].wLimitStream = stNewPic->AssiOption[j].usBitRate;
				pstCHC[i].stAssiVideoEncOpt[j].bIFrameInterval = stNewPic->AssiOption[j].IFrameInterval;

				//pstCHC[i].stAssiVideoEncOpt[j].bScanMode = stNewPic->AssiScanMode[j];

			}
			pstCHC[i].byBlindEnable = stNewPic->CoverEnable;
			memcpy(&pstCHC[i].stBlindCover[0].rcRect, &stNewPic->Cover, sizeof(YW_RECT));
			
			pstCHC[i].stTimeOSD.bShow = stNewPic->TimeTitle.TitleEnable;
			pstCHC[i].stTimeOSD.rgbaFrontground = stNewPic->TimeTitle.TitlefgRGBA;
			pstCHC[i].stTimeOSD.rgbaBackground = stNewPic->TimeTitle.TitlebgRGBA;
			pstCHC[i].stTimeOSD.rcRect.left = stNewPic->TimeTitle.TitleLeft;
			pstCHC[i].stTimeOSD.rcRect.right = stNewPic->TimeTitle.TitleRight;
			pstCHC[i].stTimeOSD.rcRect.top = stNewPic->TimeTitle.TitleTop;
			pstCHC[i].stTimeOSD.rcRect.bottom = stNewPic->TimeTitle.TitleBottom;
			
			pstCHC[i].stChannelOSD.bShow = stNewPic->ChannelTitle.TitleEnable;
			pstCHC[i].stChannelOSD.rgbaFrontground = stNewPic->ChannelTitle.TitlefgRGBA;
			pstCHC[i].stChannelOSD.rgbaBackground = stNewPic->ChannelTitle.TitlebgRGBA;
			pstCHC[i].stChannelOSD.rcRect.left = stNewPic->ChannelTitle.TitleLeft;
			pstCHC[i].stChannelOSD.rcRect.right = stNewPic->ChannelTitle.TitleRight;
			pstCHC[i].stChannelOSD.rcRect.top = stNewPic->ChannelTitle.TitleTop;
			pstCHC[i].stChannelOSD.rcRect.bottom = stNewPic->ChannelTitle.TitleBottom;

			pstCHC[i].bVolume = stNewPic->Volume;
			pstCHC[i].bVolumeEnable = stNewPic->VolumeEnable;
			pstCHC[i].bNoise = stNewPic->Noise;
		}//for (int i = 0; i < chanNum; i++)
	}	
	else
	{
		nRet = NET_NOERROR; //acceptable
	}
	
	if (iProtocolVersion > 2)
	{
		//get color config with time section
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_COLOR, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if(nRet >= 0 && 16*sizeof(CONFIG_COLOR) == retlen)
		{
			CONFIG_COLOR *pColorCfg = (CONFIG_COLOR *)cRecvBuf;
			int i = 0, j = 0;
			int iMinIndex = 0;
			for (i = 0; i < lChnNum; i++)
			{
				iMinIndex = COL_TIME_SECT_NUM<N_COLOR_SECTION?COL_TIME_SECT_NUM:N_COLOR_SECTION;
				for (j = 0; j < iMinIndex; j++)
				{
					pstCHC[i].stColorCfg[j].byBrightness = pColorCfg->Color[j].Brightness;// ->Brightness;
					pstCHC[i].stColorCfg[j].byContrast = pColorCfg->Color[j].Contrast;//pCfgCap->Contrast;
					if (pColorCfg->Color[j].Gain >= 128)
					{
						pstCHC[i].stColorCfg[j].byGainEn = 0;
						pstCHC[i].stColorCfg[j].byGain = pColorCfg->Color[j].Gain-128;
					}
					else
					{
						pstCHC[i].stColorCfg[j].byGainEn = 1;
						pstCHC[i].stColorCfg[j].byGain = pColorCfg->Color[j].Gain;
					}
					pstCHC[i].stColorCfg[j].byHue = pColorCfg->Color[j].Hue;// pCfgCap->Hue;
					pstCHC[i].stColorCfg[j].bySaturation = pColorCfg->Color[j].Saturation;//pCfgCap->Saturation;
					pstCHC[i].stColorCfg[j].stSect.bEnable = pColorCfg->Color[j].Sector.State;
					pstCHC[i].stColorCfg[j].stSect.iBeginHour = pColorCfg->Color[j].Sector.StartHour;
					pstCHC[i].stColorCfg[j].stSect.iBeginMin = pColorCfg->Color[j].Sector.StartMin;
					pstCHC[i].stColorCfg[j].stSect.iBeginSec = pColorCfg->Color[j].Sector.StartSec;
					pstCHC[i].stColorCfg[j].stSect.iHourEnd = pColorCfg->Color[j].Sector.HourEnd;
					pstCHC[i].stColorCfg[j].stSect.iEndMin = pColorCfg->Color[j].Sector.EndMin;
					pstCHC[i].stColorCfg[j].stSect.iEndSec = pColorCfg->Color[j].Sector.EndSec;
				}
				
				pColorCfg++;
			}
		}
		else
		{
			nRet = NET_NOERROR;
		}
	}
	
	/*
	//get PTZ config
	char ptzBuf[16*sizeof(CONFIG_PTZ)] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_PTZ, 0, ptzBuf, 16*sizeof(CONFIG_PTZ), &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_PTZ))
	{
		CONFIG_PTZ *stPTZ = (CONFIG_PTZ *)(ptzBuf + lChannel*sizeof(CONFIG_PTZ));
		stCHC.dwMonitorAddr = stPTZ->MonitorAddr;
		}
		else
		{
		return NET_ERROR_GETCFG_MONADDR;
	}
	*/
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

//Add: by zsc(11402) 2008-1-18
/*
 *	获取通道的区域遮挡配置信息
 */
int CDevConfig::GetDevConfig_ChnCoverCfg(LONG lLoginID, DEV_VIDEOCOVER_CFG &ChnCoverCfg, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	if (lChannel < 0 || lChannel >= device->channelcount(device))
	{
		return -1;
	}
	
	memset(&ChnCoverCfg, 0, sizeof(DEV_VIDEOCOVER_CFG));
	ChnCoverCfg.dwSize = sizeof(DEV_VIDEOCOVER_CFG);
	int retlen = 0;
	int nRet = -1;
	
	//get channel name
	char buf[16*CHANNEL_NAME_LEN] = {0};		
	nRet = QueryChannelName(lLoginID, buf, 16*CHANNEL_NAME_LEN, &retlen, waittime);	
	if (nRet >= 0)
	{
		memcpy(ChnCoverCfg.szChannelName, &buf[CHANNEL_NAME_LEN*lChannel], CHANNEL_NAME_LEN);
	}
	else
	{
		return NET_ERROR_GETCFG_CHANNAME;
	}
	char *cRecvBuf = NULL;
	int iRecvBufLen = sizeof(CONFIG_VIDEO_COVER_T) + MAX_VIDEO_COVER_NUM * sizeof(CONFIG_VIDEO_COVER_ATTR);

	cRecvBuf = new char[iRecvBufLen];
	if (cRecvBuf == NULL)
	{
		goto END;
	}

	//get total blocks
	ChnCoverCfg.bTotalBlocks = MAX_VIDEO_COVER_NUM;
	retlen = 0;
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_VIDEOBLIND, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet == 0 && retlen == sizeof(VIDEO_COVER_CAPS_T))
	{
		VIDEO_COVER_CAPS_T *pCoverCaps = (VIDEO_COVER_CAPS_T *)cRecvBuf;
		if (1 == pCoverCaps->iEnabled)
		{
			ChnCoverCfg.bTotalBlocks = pCoverCaps->iBlockNum;
		}
	}
	
	//get video parameters
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_COVER, lChannel+1, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet>=0 && retlen>=sizeof(CONFIG_VIDEO_COVER_T) && (0==(retlen-sizeof(CONFIG_VIDEO_COVER_T))%sizeof(CONFIG_VIDEO_COVER_ATTR)))
	{
		CONFIG_VIDEO_COVER_T *pVideoCover = NULL;
		CONFIG_VIDEO_COVER_ATTR *pCoverAttr = NULL;
		int nCoverCount = 0;

		pVideoCover = (CONFIG_VIDEO_COVER_T *)cRecvBuf;
		ChnCoverCfg.bCoverCount = pVideoCover->iCoverNum;
		memcpy(ChnCoverCfg.reserved, pVideoCover->reserved, sizeof(pVideoCover->reserved));
		nCoverCount = pVideoCover->iCoverNum;
		if(nCoverCount == (retlen-sizeof(CONFIG_VIDEO_COVER_T))/sizeof(CONFIG_VIDEO_COVER_ATTR))
		{
			for(int i=0; i<nCoverCount; i++)
			{
				pCoverAttr = (CONFIG_VIDEO_COVER_ATTR *)(cRecvBuf + sizeof(CONFIG_VIDEO_COVER_T) + i*sizeof(CONFIG_VIDEO_COVER_ATTR));
				memcpy(&(ChnCoverCfg.CoverBlock[i].rcBlock), &(pCoverAttr->rcBlock), sizeof(YW_RECT));
				ChnCoverCfg.CoverBlock[i].nColor = pCoverAttr->Color;
				ChnCoverCfg.CoverBlock[i].bBlockType = pCoverAttr->iBlockType;
				ChnCoverCfg.CoverBlock[i].bEncode = pCoverAttr->Encode;
				ChnCoverCfg.CoverBlock[i].bPriview = pCoverAttr->Priview;
			}
		}
		else
		{
			nRet = NET_ERROR_GETCFG_COVER;
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_COVER;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete[] cRecvBuf;
		cRecvBuf = NULL;
	}
	return 0;
}
//End: zsc(11402)

//Add: by zsc(11402) 2008-1-18
/*
 *	获取所有通道的区域遮挡配置信息
 */
int CDevConfig::GetDevConfig_AllChnCoverCfg(const LONG lLoginID, DEV_VIDEOCOVER_CFG *lpChnCoverCfg, LONG lChnNum, int waittime)
{
	if (!lpChnCoverCfg || lChnNum < 0 || lChnNum > 16)
	{
		return -1;
	}
	
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	int i = 0;
	memset(lpChnCoverCfg, 0, lChnNum*sizeof(DEV_VIDEOCOVER_CFG));
	for(i = 0; i < lChnNum; i++)
	{
		lpChnCoverCfg[i].dwSize = sizeof(DEV_VIDEOCOVER_CFG);
	}
	int retlen = 0;
	int nRet = -1;
	
	//get channel name
	char buf[16*CHANNEL_NAME_LEN] = {0};		
	nRet = QueryChannelName(lLoginID, buf, 16*CHANNEL_NAME_LEN, &retlen, waittime);	
	if (nRet >= 0)
	{
		for (i = 0; i < lChnNum; i++)
		{
			memcpy(lpChnCoverCfg[i].szChannelName, &buf[i*CHANNEL_NAME_LEN], CHANNEL_NAME_LEN);
		}
	}
	else
	{
		return NET_ERROR_GETCFG_CHANNAME;
	}
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = sizeof(CONFIG_VIDEO_COVER_T) + MAX_VIDEO_COVER_NUM * sizeof(CONFIG_VIDEO_COVER_ATTR);
	
	cRecvBuf = new char[iRecvBufLen];
	if (cRecvBuf == NULL)
	{
		goto END;
	}

	//get total blocks
	for (i = 0; i < lChnNum; i++)
	{
		lpChnCoverCfg[i].bTotalBlocks = MAX_VIDEO_COVER_NUM;
	}
	retlen = 0;
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_VIDEOBLIND, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet == 0 && retlen == sizeof(VIDEO_COVER_CAPS_T))
	{
		VIDEO_COVER_CAPS_T *pCoverCaps = (VIDEO_COVER_CAPS_T *)cRecvBuf;
		if (1 == pCoverCaps->iEnabled)
		{
			for (i = 0; i < lChnNum; i++)
			{
				lpChnCoverCfg[i].bTotalBlocks = pCoverCaps->iBlockNum;
			}
		}
	}

	//get cover parameters
	memset(cRecvBuf, 0, iRecvBufLen);
	for(i = 0; i < lChnNum; i++)
	{
		DEV_VIDEOCOVER_CFG *pChnCoverCfg = lpChnCoverCfg + i;
		
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_COVER, i+1, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet>=0 && retlen>=sizeof(CONFIG_VIDEO_COVER_T) && (0==(retlen-sizeof(CONFIG_VIDEO_COVER_T))%sizeof(CONFIG_VIDEO_COVER_ATTR)))
		{
			CONFIG_VIDEO_COVER_T *pVideoCover = NULL;
			CONFIG_VIDEO_COVER_ATTR *pCoverAttr = NULL;
			int nCoverCount = 0;
			
			pVideoCover = (CONFIG_VIDEO_COVER_T *)cRecvBuf;
			pChnCoverCfg->bCoverCount = pVideoCover->iCoverNum;
			nCoverCount = pVideoCover->iCoverNum;
			if(nCoverCount == (retlen-sizeof(CONFIG_VIDEO_COVER_T))/sizeof(CONFIG_VIDEO_COVER_ATTR))
			{
				for(int j=0; j<nCoverCount; j++)
				{
					pCoverAttr = (CONFIG_VIDEO_COVER_ATTR *)(cRecvBuf + sizeof(CONFIG_VIDEO_COVER_T) + j*sizeof(CONFIG_VIDEO_COVER_ATTR));
					memcpy(&(pChnCoverCfg->CoverBlock[j].rcBlock), &(pCoverAttr->rcBlock), sizeof(YW_RECT));
					pChnCoverCfg->CoverBlock[j].nColor = pCoverAttr->Color;
					pChnCoverCfg->CoverBlock[j].bBlockType = pCoverAttr->iBlockType;
					pChnCoverCfg->CoverBlock[j].bEncode = pCoverAttr->Encode;
					pChnCoverCfg->CoverBlock[j].bPriview = pCoverAttr->Priview;
				}
			}
			else
			{
				nRet = NET_ERROR_GETCFG_COVER;
				break;
			}
		}
		else
		{
			nRet = NET_ERROR_GETCFG_COVER;
			break;
		}

		memset(cRecvBuf, 0, iRecvBufLen);
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete[] cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}
//End: zsc(11402)

//Add: by zsc(11402) 2008-4-16
int	CDevConfig::GetDevConfig_AllWaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChnNum, int waittime)
{
	if (!pstCHC || lChnNum < 0 || lChnNum > 16)
	{
		return -1;
	}

	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	int retlen = 0;
	int nRet = -1;
	bool bSupport = false;
	WaterMake_Enable_T stuEnable = {0};
	//查看能力
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WATERMARK, (char*)&stuEnable, sizeof(WaterMake_Enable_T), &retlen, waittime);
	if (nRet ==0 && retlen == sizeof(WaterMake_Enable_T))
	{
		if(1 == stuEnable.isSupportWM)
		{
			bSupport = true;
		}
	}

	if (!bSupport)
	{
		return -1;
	}
	
	memset(pstCHC, 0, lChnNum*sizeof(DEVICE_WATERMAKE_CFG));
	for (int i = 0; i < lChnNum; i++)
	{
		pstCHC[i].dwSize = sizeof(DEVICE_WATERMAKE_CFG);
	}
	
	//查询图象水印信息
	char buffer[16*4116] = {0};
	retlen = 0;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_WATERMAKE, 0, buffer, 16*4116, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		CStrParse parse;
		parse.setSpliter("&&");
		bool bSuccess = parse.Parse(buffer);
		if (!bSuccess)
		{
			return -1;
		}

		int nCount = parse.Size();
		if (nCount > 0)
		{
			for (int i = 0; i < nCount; i++)
			{
				std::string watermake = parse.getWord();

				CStrParse subparse;
				subparse.setSpliter("::");
				bool bSuccess = subparse.Parse(watermake);
				if (!bSuccess)
				{
					break;
				}

				int nChannelNum = atoi(subparse.getWord().c_str());
				if (nChannelNum > 0 && nChannelNum <= lChnNum)
				{
					pstCHC[nChannelNum-1].nStream = atoi(subparse.getWord().c_str());
					pstCHC[nChannelNum-1].nEnable = atoi(subparse.getWord().c_str());
					pstCHC[nChannelNum-1].nKey = atoi(subparse.getWord().c_str());

					const char* data = subparse.getWord().c_str();
					if (1 == pstCHC[nChannelNum-1].nKey)
					{
						if (MAX_WATERMAKE_LETTER > strlen(data))
						{
							Change_Utf8_Assic((BYTE*)data, pstCHC[nChannelNum-1].szLetterData);
							pstCHC[nChannelNum-1].szLetterData[MAX_WATERMAKE_LETTER-1] = '\0';
						}
					}
					else if (2 == pstCHC[nChannelNum-1].nKey)
					{
						strcpy(pstCHC[nChannelNum-1].szData, data);
					}
				}
			}
		}
		else
		{
			nRet = NET_RETURN_DATA_ERROR;
		}
	}
	else
	{
		return NET_ERROR_GETCFG_WATERMAKE;
	}

	return nRet;
}

int	CDevConfig::GetDevConfig_WaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	if (lChannel < 0 || lChannel >= device->channelcount(device))
	{
		return -1;
	}

	int retlen = 0;
	int nRet = -1;
	bool bSupport = false;
	WaterMake_Enable_T stuEnable = {0};
	//查看能力
	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WATERMARK, (char*)&stuEnable, sizeof(WaterMake_Enable_T), &retlen, waittime);
	if (nRet ==0 && retlen == sizeof(WaterMake_Enable_T))
	{
		if(1 == stuEnable.isSupportWM)
		{
			bSupport = true;
		}
	}

	if (!bSupport)
	{
		return -1;
	}
	
	memset(pstCHC, 0, sizeof(DEVICE_WATERMAKE_CFG));
	pstCHC->dwSize = sizeof(DEVICE_WATERMAKE_CFG);
	
	//查询图象水印信息
	char buffer[16*4116] = {0};
	retlen = 0;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_WATERMAKE, 0, buffer, 16*4116, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		CStrParse parse;
		parse.setSpliter("&&");
		bool bSuccess = parse.Parse(buffer);
		if (!bSuccess)
		{
			return -1;
		}

		int nCount = parse.Size();
		if (nCount > 0)
		{
			for (int i = 0; i < nCount; i++)
			{
				std::string watermake = parse.getWord();

				CStrParse subparse;
				subparse.setSpliter("::");
				bool bSuccess = subparse.Parse(watermake);
				if (!bSuccess)
				{
					break;
				}
				
				int nChannelNum = atoi(subparse.getWord().c_str());
				if ((nChannelNum-1) == lChannel)
				{
					pstCHC->nStream = atoi(subparse.getWord().c_str());
					pstCHC->nEnable = atoi(subparse.getWord().c_str());
					pstCHC->nKey = atoi(subparse.getWord().c_str());

					const char* data = subparse.getWord().c_str();
					if (1 == pstCHC->nKey)
					{
						if (MAX_WATERMAKE_LETTER > strlen(data))
						{
							Change_Utf8_Assic((BYTE*)data, pstCHC->szLetterData);
							pstCHC->szLetterData[MAX_WATERMAKE_LETTER-1] = '\0';
						}
					}
					else if (2 == pstCHC->nKey)
					{
						strcpy(pstCHC->szData, data);
					}
				}
			}
		}
		else
		{
			nRet = NET_RETURN_DATA_ERROR;
		}
	}
	else
	{
		return NET_ERROR_GETCFG_WATERMAKE;
	}

	return nRet;
}

int	CDevConfig::SetDevConfig_AllWaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChnNum, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (lChnNum <= 0 || lChnNum > MAX_CHANNUM || lChnNum > device->channelcount(device))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	if (NULL == pstCHC)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	//	将结构体串成字符串
	char buffer[16*4116] = {0};
	int nPos = 0;
	DEVICE_WATERMAKE_CFG *pTmpChaCfg = pstCHC;
	for (int i = 0; i < lChnNum; i++)
	{
		if (1 == pTmpChaCfg->nKey)
		{
			char szUTF8[MAX_WATERMAKE_LETTER*2+2] = {0};
			Change_Assic_UTF8(pTmpChaCfg->szLetterData, strlen(pTmpChaCfg->szLetterData), szUTF8, MAX_WATERMAKE_LETTER*2+2);

			sprintf(buffer+nPos, "%d::%d::%d::%d::%s&&", i+1, pTmpChaCfg->nStream, pTmpChaCfg->nEnable, pTmpChaCfg->nKey, szUTF8);
		}
		else if (2 == pTmpChaCfg->nKey)
		{
			sprintf(buffer+nPos, "%d::%d::%d::%d::%s&&", i+1, pTmpChaCfg->nStream, pTmpChaCfg->nEnable, pTmpChaCfg->nKey, pTmpChaCfg->szData);
		}
		else
		{
			return nRet;
		}
		
		nPos = strlen(buffer);
		pTmpChaCfg++;
	}
	buffer[nPos-1] = '\0';
	buffer[nPos-2] = '\0';

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_WATERMAKE, 0, buffer, nPos-2, waittime);
	if (nRet < 0)
	{
		nFailedCount++;
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}

	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_WATERMAKE;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int	CDevConfig::SetDevConfig_WaterMakeCfg(LONG lLoginID, DEVICE_WATERMAKE_CFG *pstCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (lChannel < -1 || lChannel >= MAX_CHANNUM || lChannel >= device->channelcount(device))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	if (NULL == pstCHC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	//	将结构体串成字符串
	char buffer[4116] = {0};
	if (1 == pstCHC->nKey)
	{
		char szUTF8[MAX_WATERMAKE_LETTER*2+2] = {0};
		Change_Assic_UTF8(pstCHC->szLetterData, strlen(pstCHC->szLetterData), szUTF8, MAX_WATERMAKE_LETTER*2+2);

		sprintf(buffer, "%d::%d::%d::%d::%s", lChannel+1, pstCHC->nStream, pstCHC->nEnable, pstCHC->nKey, szUTF8);
	}
	else if (2 == pstCHC->nKey)
	{
		sprintf(buffer, "%d::%d::%d::%d::%s", lChannel+1, pstCHC->nStream, pstCHC->nEnable, pstCHC->nKey, pstCHC->szData);
	}
	else
	{
		return nRet;
	}

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_WATERMAKE, 0, buffer, strlen(buffer), waittime);
	if (nRet < 0)
	{
		nFailedCount++;
	}
	else
	{
		Sleep(SETUP_SLEEP);
	}

	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_WATERMAKE;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}
//End: by zsc(11402)

//Add: by ljy(10858) 2008-3-11
int	CDevConfig::GetDevConfig_TransStrategyCfg(LONG lLoginID, DEV_TRANSFER_STRATEGY_CFG &stTSC, int waittime)
{
	memset(&stTSC, 0, sizeof(DEV_TRANSFER_STRATEGY_CFG));
	stTSC.dwSize = sizeof(DEV_TRANSFER_STRATEGY_CFG);
	
	int retlen = 0;
	int nRet = -1;
	
	int buflen = 500;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_TRANSFER_STRATEGY, 0, tmpbuf, buflen, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		nRet = ParseTransStrategyCfgString(tmpbuf, retlen, &stTSC);
	}
	else
	{
		nRet = -1;
	}

	delete tmpbuf;
	
	return nRet;
}
//End: ljy(10858)

//Add: by ljy(10858) 2008-3-24
int	CDevConfig::GetDevConfig_DownloadStrategyCfg(LONG lLoginID, DEV_DOWNLOAD_STRATEGY_CFG &stDSC, int waittime)
{	
	memset(&stDSC, 0, sizeof(DEV_DOWNLOAD_STRATEGY_CFG));
	stDSC.dwSize = sizeof(DEV_DOWNLOAD_STRATEGY_CFG);
	
	int retlen = 0;
	int nRet = -1;
	
	int buflen = 500;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_DOWNLOAD_STRATEGY, 0, tmpbuf, buflen, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		nRet = ParseDownloadStrategyCfgString(tmpbuf, retlen, &stDSC);
	}
	else
	{
		nRet = -1;
	}
	
	delete tmpbuf;

	return nRet;
}
//End: ljy(10858)

int CDevConfig::GetDevConfig_RecCfg(LONG lLoginID, DEV_RECORD_CFG &stREC, LONG lChannel, int waittime)
{
 // 旧协议
	if (lChannel < 0 || lChannel > 16)
	{
		return -1;
	}
	
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stREC, 0, sizeof(DEV_RECORD_CFG));
	stREC.dwSize = sizeof(DEV_RECORD_CFG);
	int retlen = 0;
	int nRet = -1;

	//get record config
	char rBuf[16*sizeof(CONFIG_RECORD)] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD, 0, rBuf, 16*sizeof(CONFIG_RECORD), &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		CONFIG_RECORD *pstRecord = 0;
		
		pstRecord = (CONFIG_RECORD*)(rBuf+lChannel*sizeof(CONFIG_RECORD));
		
		for (int weekIdx = 0; weekIdx < N_WEEKS; weekIdx++)
		{
			for (int secIdx = 0; secIdx < N_TSECT; secIdx++)
			{
				stREC.stSect[weekIdx][secIdx].bEnable = 0;
				stREC.stSect[weekIdx][secIdx].bEnable |= (pstRecord->Sector[weekIdx][secIdx].State&0x02)?0x04:0x00;
				stREC.stSect[weekIdx][secIdx].bEnable |= (pstRecord->Sector[weekIdx][secIdx].State&0x04)?0x01:0x00;
				stREC.stSect[weekIdx][secIdx].bEnable |= (pstRecord->Sector[weekIdx][secIdx].State&0x08)?0x02:0x00;
				stREC.stSect[weekIdx][secIdx].iBeginHour = pstRecord->Sector[weekIdx][secIdx].StartHour;
				stREC.stSect[weekIdx][secIdx].iBeginMin = pstRecord->Sector[weekIdx][secIdx].StartMin;
				stREC.stSect[weekIdx][secIdx].iBeginSec = pstRecord->Sector[weekIdx][secIdx].StartSec;
				stREC.stSect[weekIdx][secIdx].iHourEnd = pstRecord->Sector[weekIdx][secIdx].HourEnd;
				stREC.stSect[weekIdx][secIdx].iEndMin = pstRecord->Sector[weekIdx][secIdx].EndMin;
				stREC.stSect[weekIdx][secIdx].iEndSec = pstRecord->Sector[weekIdx][secIdx].EndSec;
			}
		}
		
		stREC.byRedundancyEn = pstRecord->Redundancy;
		stREC.byPreRecordLen = pstRecord->PreRecord;
	}
	else //if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		return NET_ERROR_GETCFG_RECORD;
	}

	return 0;
}

int	CDevConfig::GetDevConfig_RecCfgNew(LONG lLoginID, DEV_RECORD_CFG &stREC, LONG lChannel, int waittime)
{
	if (lChannel < 0 || lChannel > 16)
	{
		return -1;
	}
	
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stREC, 0, sizeof(DEV_RECORD_CFG));
	stREC.dwSize = sizeof(DEV_RECORD_CFG);
	int retlen = 0;
	int nRet = -1;
	
	//get record config
	char rBuf[16*sizeof(CONFIG_RECORD_NEW)] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, rBuf, 16*sizeof(CONFIG_RECORD_NEW), &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD_NEW))
	{
		CONFIG_RECORD_NEW *stRecordNew = 0;
		stRecordNew = (CONFIG_RECORD_NEW*)(rBuf+lChannel*sizeof(CONFIG_RECORD_NEW));
		
		stREC.byRedundancyEn = stRecordNew->bRedundancy;
		stREC.byPreRecordLen = stRecordNew->iPreRecord;
		stREC.byRecordType = stRecordNew->byRecordType;
	}
	else //if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		return NET_ERROR_GETCFG_RECORD;
	}
	
	char whBuf[16*sizeof(CONFIG_WORKSHEET)] = {0};
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_RECORD, (CONFIG_WORKSHEET*)whBuf, waittime, lChannel+1);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)whBuf;
		pWSheet = (CONFIG_WORKSHEET *)(whBuf+lChannel*sizeof(CONFIG_WORKSHEET));
		memcpy(stREC.stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
	}
	else
	{
		return NET_ERROR_GETCFG_RECORD;
	}
	
	return 0;
}

int	CDevConfig::GetDevConfig_AllRecCfg(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChnNum, int waittime)
{
	//旧协议
	if (!pstREC || lChnNum < 0 || lChnNum > 16)
	{
		return -1;
	}
	
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(pstREC, 0, lChnNum*sizeof(DEV_RECORD_CFG));
	for (int i = 0; i < lChnNum; i++)
	{
		pstREC[i].dwSize = sizeof(DEV_RECORD_CFG);
	}
	int retlen = 0;
	int nRet = -1;

	//get record config
	char rBuf[16*sizeof(CONFIG_RECORD)] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD, 0, rBuf, 16*sizeof(CONFIG_RECORD), &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		CONFIG_RECORD *pstRecord = 0;

		for (int i = 0; i < lChnNum; i++)
		{
			pstRecord = (CONFIG_RECORD*)(rBuf+i*sizeof(CONFIG_RECORD));

			for (int weekIdx = 0; weekIdx < N_WEEKS; weekIdx++)
			{
				for (int secIdx = 0; secIdx < N_TSECT; secIdx++)
				{
					pstREC[i].stSect[weekIdx][secIdx].bEnable = 0;
					pstREC[i].stSect[weekIdx][secIdx].bEnable |= (pstRecord->Sector[weekIdx][secIdx].State&0x02)?0x04:0x00;
					pstREC[i].stSect[weekIdx][secIdx].bEnable |= (pstRecord->Sector[weekIdx][secIdx].State&0x04)?0x01:0x00;
					pstREC[i].stSect[weekIdx][secIdx].bEnable |= (pstRecord->Sector[weekIdx][secIdx].State&0x08)?0x02:0x00;
					pstREC[i].stSect[weekIdx][secIdx].iBeginHour = pstRecord->Sector[weekIdx][secIdx].StartHour;
					pstREC[i].stSect[weekIdx][secIdx].iBeginMin = pstRecord->Sector[weekIdx][secIdx].StartMin;
					pstREC[i].stSect[weekIdx][secIdx].iBeginSec = pstRecord->Sector[weekIdx][secIdx].StartSec;
					pstREC[i].stSect[weekIdx][secIdx].iHourEnd = pstRecord->Sector[weekIdx][secIdx].HourEnd;
					pstREC[i].stSect[weekIdx][secIdx].iEndMin = pstRecord->Sector[weekIdx][secIdx].EndMin;
					pstREC[i].stSect[weekIdx][secIdx].iEndSec = pstRecord->Sector[weekIdx][secIdx].EndSec;
				}
			}
			
			//memcpy(pstREC[i].stSect, pstRecord->Sector, sizeof(pstRecord->Sector));
			pstREC[i].byRedundancyEn = pstRecord->Redundancy;
			pstREC[i].byPreRecordLen = pstRecord->PreRecord;
		}
	}
	else //if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		return NET_ERROR_GETCFG_RECORD;
	}
	
	return 0;
}

int	CDevConfig::GetDevConfig_AllRecCfgNew(LONG lLoginID, DEV_RECORD_CFG *pstREC, LONG lChnNum, int waittime)
{
	if (!pstREC || lChnNum < 0 /*|| lChnNum > 16*/)
	{
		return -1;
	}
	
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(pstREC, 0, lChnNum*sizeof(DEV_RECORD_CFG));
	for (int i = 0; i < lChnNum; i++)
	{
		pstREC[i].dwSize = sizeof(DEV_RECORD_CFG);
	}
	int retlen = 0;
	int nRet = -1;
	
	//get record config
	int chancount = device->channelcount(device)>16?device->channelcount(device):16;
	
	char* rBuf = NULL; 
	rBuf = new char[chancount * sizeof(CONFIG_RECORD_NEW)];
	if (rBuf == NULL) 
	{
		return -1;
	}
	char* whBuf = NULL;
	whBuf = new char[chancount * sizeof(CONFIG_WORKSHEET)];
	if (whBuf == NULL) 
	{
		goto END;
	}

	memset(rBuf, 0, chancount * sizeof(CONFIG_RECORD_NEW));
	//char rBuf[16*sizeof(CONFIG_RECORD_NEW)] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, rBuf, chancount * sizeof(CONFIG_RECORD_NEW), &retlen, waittime);
	if ((nRet >= 0) && (retlen%sizeof(CONFIG_RECORD_NEW)==0) && (retlen/sizeof(CONFIG_RECORD_NEW) >= lChnNum))
	{
		CONFIG_RECORD_NEW *stRecordNew = 0;
		
		for (int i = 0; i < lChnNum; i++)
		{
			stRecordNew = (CONFIG_RECORD_NEW*)(rBuf+i*sizeof(CONFIG_RECORD_NEW));
			
			pstREC[i].byRedundancyEn = stRecordNew->bRedundancy;
			pstREC[i].byPreRecordLen = stRecordNew->iPreRecord;
			pstREC[i].byRecordType = stRecordNew->byRecordType;
		}
	}
	else //if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		nRet = NET_ERROR_GETCFG_RECORD;
		goto END;
	}
	
	memset(whBuf, 0, chancount * sizeof(CONFIG_WORKSHEET));
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_RECORD, (CONFIG_WORKSHEET*)whBuf, waittime, lChnNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)whBuf;
		for (int i = 0; i < lChnNum; i++, pWSheet++)
		{
			memcpy(pstREC[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_RECORD;
		goto END;
	}
	
END:
	if (rBuf) 
	{
		delete []rBuf;
		rBuf = NULL;
	}
	if (whBuf) 
	{
		delete []whBuf;
		whBuf = NULL;			
	}
	return nRet;
}

/*
 *	获取云台设备属性
 */
int CDevConfig::GetDevConfig_PtzCfg(LONG lLoginID, PTZ_OPT_ATTR * stuPtzAttr,DWORD dwProtocolIndex,int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	memset(stuPtzAttr,0,sizeof(PTZ_OPT_ATTR));
	int retlen = 0;
	int nRet =-1;
	nRet = QueryComProtocol(lLoginID,2,(char *)stuPtzAttr,sizeof(PTZ_OPT_ATTR),&retlen,waittime,dwProtocolIndex);
	if(nRet < 0 || retlen == 0)
	{
		return -1;
	}
	return 0;
}

int CDevConfig::GetDevConfig_ComCfg(LONG lLoginID, DEV_COMM_CFG &stCMC, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}
	
	memset(&stCMC, 0, sizeof(DEV_COMM_CFG));
	stCMC.dwSize = sizeof(DEV_COMM_CFG);
	int retlen = 0;
	int nRet = -1;
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	//	此功能解码器除外
	if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
	{
		//Get 485 protocol name list
		char pBuf[MAX_DECPRO_LIST_SIZE*sizeof(DEC_PROTOCOL)] = {0};
		if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_PTZCONF])
		{
			nRet = QueryComProtocol(lLoginID, 0, pBuf, MAX_DECPRO_LIST_SIZE*sizeof(DEC_PROTOCOL), &retlen, waittime);
			if (nRet >= 0 && retlen%sizeof(DEC_PROTOCOL) == 0)
			{
				DEC_PROTOCOL *tmpNode;
				stCMC.dwDecProListNum = retlen/sizeof(DEC_PROTOCOL);
				for (int i = 0; i < stCMC.dwDecProListNum; i++)
				{
					tmpNode = (DEC_PROTOCOL*)(pBuf + i*sizeof(DEC_PROTOCOL));
					if (tmpNode) 
					{
						memcpy(stCMC.DecProName[i], tmpNode->ProtocolName, 12);
					}
				}
			}
			else
			{
				return NET_ERROR_GETCFG_PRONAME;
			}
		}
	}
	
	//Get 232 function name list
	DWORD dw232FuncMask = 0;
	char fBuf[32*sizeof(DEC_PROTOCOL)] = {0};

	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_COMCONF])
	{
		nRet = QueryComProtocol(lLoginID, 1, fBuf, 32*sizeof(DEC_PROTOCOL), &retlen, waittime);
		if (nRet >= 0 && (retlen-4)%sizeof(DEC_PROTOCOL) == 0)
		{
			//header 4 byte stands protocol number
			dw232FuncMask = *(DWORD*)fBuf;
			DEC_PROTOCOL *tmpNode = 0;
			GetMaskedFuncName(fBuf+4, retlen-4, dw232FuncMask, (char*)stCMC.s232FuncName, MAX_232_FUNCS, &stCMC.dw232FuncNameNum);
		}
		else
		{
			return NET_ERROR_GETCFG_FUNCNAME;
		}
	}
	
	//	此功能解码器除外
	if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
	{
		//Get decoder config
		char dBuf[16*sizeof(CONFIG_PTZ)] = {0};
		if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_PTZCONF])
		{
			nRet = QueryConfig(lLoginID, CONFIG_TYPE_PTZ, 0, dBuf, 16*sizeof(CONFIG_PTZ), &retlen, waittime);
			if (nRet >= 0 && retlen == 16*sizeof(CONFIG_PTZ))
			{
				CONFIG_PTZ *stPTZ = 0;
				for (int i = 0; i < 16; i++)
				{
					stPTZ= (CONFIG_PTZ *)(dBuf + i*sizeof(CONFIG_PTZ));
					stCMC.stDecoder[i].wDecoderAddress = stPTZ->DestAddr;
					//			stCMC.stDecoder[i].byCamID = stPTZ->CamID;
					stCMC.stDecoder[i].byMartixID = stPTZ->byMartixID;
					stCMC.stDecoder[i].wProtocol = stPTZ->Protocol;
					stCMC.stDecoder[i].bPTZType = stPTZ->bPTZType;
					stCMC.stDecoder[i].struComm.byDataBit = stPTZ->PTZAttr.databits-5;
					stCMC.stDecoder[i].struComm.byParity = stPTZ->PTZAttr.parity;
					stCMC.stDecoder[i].struComm.byStopBit = stPTZ->PTZAttr.stopbits;
					stCMC.stDecoder[i].struComm.byBaudRate = (BYTE)ChangeBaudRate(stPTZ->PTZAttr.baudrate);
				}
			}
			else
			{
				return NET_ERROR_GETCFG_485DECODER;
			}
		}
	}
	
	//Get 232 com config
	char cBuf[sizeof(CONFIG_COMM)] = {0};
	
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_COMCONF])
	{
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_SERIAL_COMM, 0, cBuf, sizeof(CONFIG_COMM), &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(CONFIG_COMM))
		{
			CONFIG_COMM *stCOM = (CONFIG_COMM*)cBuf;
			GetMaskedFuncIndex(&stCMC.st232[0].byFunction, stCOM->Function, dw232FuncMask);
			stCMC.st232[0].struComm.byDataBit = stCOM->DataBits - 5;
			stCMC.st232[0].struComm.byParity = stCOM->Parity;
			stCMC.st232[0].struComm.byStopBit = stCOM->StopBits;
			stCMC.st232[0].struComm.byBaudRate = (BYTE)ChangeBaudRate(stCOM->BaudBase);
		}
		else
		{
			return NET_ERROR_GETCFG_232COM;
		}
	}

	return 0;
}

void CDevConfig::GetAlmActionMsk(CONFIG_TYPES almType, DWORD *dwMsk)
{
	switch (almType)
	{
	case CONFIG_TYPE_ALARM_LOCALALM:
	case CONFIG_TYPE_ALARM_NETALM:
	case CONFIG_TYPE_ALARM_MOTION:
	case CONFIG_TYPE_ALARM_BLIND:
	case CONFIG_TYPE_ALARM_LOSS:
	case CONFIG_TYPE_ALARM_NODISK:
	case CONFIG_TYPE_ALARM_DISKFULL:
	case CONFIG_TYPE_ALARM_DISKERR:
	case CONFIG_TYPE_ALARM_NETBROKEN:
	case CONFIG_TYPE_ALARM_PANORAMA:
	case CONFIG_TYPE_ALARM_LOSTFOCUS:
	case CONFIG_TYPE_ALARM_DECODER:
	case CONFIG_TYPE_ALARM_IPCOLLISION:
		*dwMsk = 0;
		*dwMsk |= ALARM_TIP;
		*dwMsk |= ALARM_TOUR;
		*dwMsk |= ALARM_MAIL;
		*dwMsk |= ALARM_UPLOAD;
		*dwMsk |= ALARM_OUT;
		*dwMsk |= ALARM_RECORD;
		*dwMsk |= ALARM_PTZ;
		*dwMsk |= ALARM_FTP_UL;
		*dwMsk |= ALARM_BEEP;
		*dwMsk |= ALARM_VOICE;
		*dwMsk |= ALARM_SNAP;
		break;
	default:
		dwMsk = 0;
	}
}

void CDevConfig::GetAlmActionFlag(EVENT_HANDLER hEvent, DWORD *dwFlag)
{
	if (hEvent.bAlarmOutEn)
	{
		*dwFlag |= ALARM_OUT;
	}
	if (hEvent.bBeep)
	{
		*dwFlag |= ALARM_BEEP;
	}
	if (hEvent.bFtp)
	{
		*dwFlag |= ALARM_FTP_UL;
	}
	if (hEvent.bMail)
	{
		*dwFlag |= ALARM_MAIL;
	}
	if (hEvent.bMessage)
	{
		*dwFlag |= ALARM_UPLOAD;
	}
	if (hEvent.bPtzEn)
	{
		*dwFlag |= ALARM_PTZ;
	}
	if (hEvent.bRecordEn)
	{
		*dwFlag |= ALARM_RECORD;
	}
	if (hEvent.bSnapEn)
	{
		*dwFlag |= ALARM_SNAP;
	}
	if (hEvent.bTip)
	{
		*dwFlag |= ALARM_TIP;
	}
	if (hEvent.bTourEn)
	{
		*dwFlag |= ALARM_TOUR;
	}
	if (hEvent.bVoice)
	{
		*dwFlag |= ALARM_VOICE;
	}
}

void CDevConfig::SetAlmActionFlag(EVENT_HANDLER *pEvent, DWORD dwFlag)
{
	pEvent->bAlarmOutEn = (dwFlag&ALARM_OUT) ? 1 : 0;
	pEvent->bBeep = (dwFlag&ALARM_BEEP) ? 1 : 0;
	pEvent->bFtp = (dwFlag&ALARM_FTP_UL) ? 1 : 0;
	pEvent->bMail = (dwFlag&ALARM_MAIL) ? 1 : 0;
	pEvent->bMessage = (dwFlag&ALARM_UPLOAD) ? 1 : 0;
	pEvent->bPtzEn = (dwFlag&ALARM_PTZ) ? 1 : 0;
	pEvent->bRecordEn = (dwFlag&ALARM_RECORD) ? 1 : 0;
	pEvent->bSnapEn = (dwFlag&ALARM_SNAP) ? 1 : 0;
	pEvent->bTip = (dwFlag&ALARM_TIP) ? 1 : 0;
	pEvent->bTourEn = (dwFlag&ALARM_TOUR) ? 1 : 0;
	pEvent->bVoice = (dwFlag&ALARM_VOICE) ? 1 : 0;
}

int CDevConfig::GetDevConfig_AlmCfgLocalAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pAlmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;

	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_ALARM_X))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_ALARM_X);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_ALARM_X))
	{
		device->get_info(device, dit_alarmin_num, &nSheetNum);

		int j = 0;
		CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)cRecvBuf;
		for (i = 0; i < 16; i++,pCfgAlmX++)
		{
			GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &pAlmCfg[i].struHandle.dwActionMask);
			pAlmCfg[i].byAlarmEn = pCfgAlmX->bEnable;
			pAlmCfg[i].byAlarmType = pCfgAlmX->iSensorType;
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pAlmCfg[i].struHandle.struPtzLink[j].iValue = pCfgAlmX->hEvent.PtzLink[j].iValue;
				pAlmCfg[i].struHandle.struPtzLink[j].iType = pCfgAlmX->hEvent.PtzLink[j].iType;
				pAlmCfg[i].struHandle.byRecordChannel[j] = BITRHT(pCfgAlmX->hEvent.dwRecord,j)&1;
				pAlmCfg[i].struHandle.byTour[j] = BITRHT(pCfgAlmX->hEvent.dwTour,j)&1;
				pAlmCfg[i].struHandle.bySnap[j] = BITRHT(pCfgAlmX->hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pAlmCfg[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.dwAlarmOut,j)&1;
				pAlmCfg[i].struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgAlmX->hEvent.wiAlarmOut,j)&1;
			}
			pAlmCfg[i].struHandle.dwDuration = pCfgAlmX->hEvent.iAOLatch;
			pAlmCfg[i].struHandle.dwRecLatch = pCfgAlmX->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pAlmCfg[i].struHandle.dwEventLatch = pCfgAlmX->hEvent.iEventLatch;//报警延时
			pAlmCfg[i].struHandle.bMessageToNet = (BYTE)pCfgAlmX->hEvent.bMessagetoNet;
			pAlmCfg[i].struHandle.bMMSEn = (BYTE)pCfgAlmX->hEvent.bMMSEn;
			pAlmCfg[i].struHandle.bySnapshotTimes = pCfgAlmX->hEvent.SnapshotTimes;//短信发送图片的张数
			pAlmCfg[i].struHandle.bLog = (BOOL)pCfgAlmX->hEvent.bLog;
			pAlmCfg[i].struHandle.bMatrixEn = (BOOL)pCfgAlmX->hEvent.bMatrixEn;
			pAlmCfg[i].struHandle.dwMatrix = pCfgAlmX->hEvent.dwMatrix;
			GetAlmActionFlag(pCfgAlmX->hEvent, &pAlmCfg[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(cRecvBuf, 0, iRecvBufLen);
	nSheetNum = nSheetNum>16?16:nSheetNum;
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_LOCAL_ALARM, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(pAlmCfg[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::GetDevConfig_AlmCfgNetAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (0 == device || NULL == pAlmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;
	
	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_ALARM_X))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_ALARM_X);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_ALARM_X))
	{
		device->get_info(device, dit_alarmin_num, &nSheetNum);

		int j = 0;
		CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)cRecvBuf;
		for (i = 0; i < 16; i++,pCfgAlmX++)
		{
			GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &pAlmCfg[i].struHandle.dwActionMask);
			pAlmCfg[i].byAlarmEn = pCfgAlmX->bEnable;
			pAlmCfg[i].byAlarmType = pCfgAlmX->iSensorType;
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pAlmCfg[i].struHandle.struPtzLink[j].iValue = pCfgAlmX->hEvent.PtzLink[j].iValue;
				pAlmCfg[i].struHandle.struPtzLink[j].iType = pCfgAlmX->hEvent.PtzLink[j].iType;
				pAlmCfg[i].struHandle.byRecordChannel[j] = BITRHT(pCfgAlmX->hEvent.dwRecord,j)&1;
				pAlmCfg[i].struHandle.byTour[j] = BITRHT(pCfgAlmX->hEvent.dwTour,j)&1;
				pAlmCfg[i].struHandle.bySnap[j] = BITRHT(pCfgAlmX->hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pAlmCfg[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.dwAlarmOut,j)&1;
				pAlmCfg[i].struHandle.byRelWIAlarmOut[j] = BITRHT(pCfgAlmX->hEvent.wiAlarmOut,j)&1;
			}
			pAlmCfg[i].struHandle.dwDuration = pCfgAlmX->hEvent.iAOLatch;
			pAlmCfg[i].struHandle.dwRecLatch = pCfgAlmX->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pAlmCfg[i].struHandle.dwEventLatch = pCfgAlmX->hEvent.iEventLatch;
			pAlmCfg[i].struHandle.bMessageToNet = (BYTE)pCfgAlmX->hEvent.bMessagetoNet;
			pAlmCfg[i].struHandle.bMMSEn = (BYTE)pCfgAlmX->hEvent.bMMSEn;
			pAlmCfg[i].struHandle.bySnapshotTimes = pCfgAlmX->hEvent.SnapshotTimes;//短信发送图片的张数
			pAlmCfg[i].struHandle.bLog = (BYTE)pCfgAlmX->hEvent.bLog;
			pAlmCfg[i].struHandle.bMatrixEn = (BYTE)pCfgAlmX->hEvent.bMatrixEn;
			pAlmCfg[i].struHandle.dwMatrix = pCfgAlmX->hEvent.dwMatrix;
			GetAlmActionFlag(pCfgAlmX->hEvent, &pAlmCfg[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nSheetNum = nSheetNum>16?16:nSheetNum;
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_NET_ALARM, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(pAlmCfg[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int CDevConfig::GetDevConfig_AlmCfgMotion(LONG lLoginID, MOTION_DETECT_CFG *pMtnCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (0 == device || NULL == pMtnCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;

	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_MOTIONDETECT))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_MOTIONDETECT);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}

	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_MOTIONDETECT))
	{
		nSheetNum = device->channelcount(device);

		int j = 0;
		CONFIG_MOTIONDETECT *pCfgMtn = (CONFIG_MOTIONDETECT *)cRecvBuf;
		for (i = 0; i < 16; i++,pCfgMtn++)
		{
			GetAlmActionMsk(CONFIG_TYPE_ALARM_MOTION, &pMtnCfg[i].struHandle.dwActionMask);
			pMtnCfg[i].byMotionEn = pCfgMtn->bEnable;
			pMtnCfg[i].wSenseLevel = pCfgMtn->iLevel;

			//Begin: Add by zsc(11402) 2008-1-18
			//	获取动态检测区域的行数和列数
			int nMaxRow = 12;
			int nMaxCol = 16;
			int nSysInfoLen = 0;
			MOTION_DETECT_CAPS SysInfoMotion = {0};
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_MOTIONDETECT, (char*)&SysInfoMotion, sizeof(MOTION_DETECT_CAPS), &nSysInfoLen, waittime);
			if (0 == nRet && nSysInfoLen == sizeof(MOTION_DETECT_CAPS))
			{
				if (SysInfoMotion.Enabled == 1)
				{
					nMaxRow = SysInfoMotion.GridRows;
					nMaxCol = SysInfoMotion.GridLines;
				}
			}

			pMtnCfg[i].wMotionRow = nMaxRow;
			pMtnCfg[i].wMotionCol = nMaxCol;
			//row*col motion region
			for (int x = 0; x < nMaxRow; x++)
			{
				for (int y = 0; y < nMaxCol; y++)
				{
					pMtnCfg[i].byDetected[x][y] = (pCfgMtn->iRegion[x]&(0x01<<y)) ? 1 : 0;
				}
			}
			//End: zsc(11402)

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pMtnCfg[i].struHandle.struPtzLink[j].iValue = pCfgMtn->hEvent.PtzLink[j].iValue;
				pMtnCfg[i].struHandle.struPtzLink[j].iType = pCfgMtn->hEvent.PtzLink[j].iType;
				pMtnCfg[i].struHandle.byRecordChannel[j] = BITRHT(pCfgMtn->hEvent.dwRecord,j)&1;
				pMtnCfg[i].struHandle.byTour[j] = BITRHT(pCfgMtn->hEvent.dwTour,j)&1;
				pMtnCfg[i].struHandle.bySnap[j] = BITRHT(pCfgMtn->hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pMtnCfg[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgMtn->hEvent.dwAlarmOut,j)&1;
				pMtnCfg[i].struHandle.byRelWIAlarmOut[j] = BITRHT(pCfgMtn->hEvent.wiAlarmOut,j)&1;
			}
			pMtnCfg[i].struHandle.dwDuration = pCfgMtn->hEvent.iAOLatch;
			pMtnCfg[i].struHandle.dwRecLatch = pCfgMtn->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pMtnCfg[i].struHandle.dwEventLatch = pCfgMtn->hEvent.iEventLatch;
			pMtnCfg[i].struHandle.bMessageToNet = (BYTE)pCfgMtn->hEvent.bMessagetoNet;
			pMtnCfg[i].struHandle.bMMSEn = (BYTE)pCfgMtn->hEvent.bMMSEn;
			pMtnCfg[i].struHandle.bySnapshotTimes = pCfgMtn->hEvent.SnapshotTimes;//短信发送图片的张数
			pMtnCfg[i].struHandle.bLog = (BYTE)pCfgMtn->hEvent.bLog;
			pMtnCfg[i].struHandle.bMatrixEn = (BYTE)pCfgMtn->hEvent.bMatrixEn;
			pMtnCfg[i].struHandle.dwMatrix = pCfgMtn->hEvent.dwMatrix;
			GetAlmActionFlag(pCfgMtn->hEvent, &pMtnCfg[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(cRecvBuf, 0, iRecvBufLen);
	nSheetNum = nSheetNum>16?16:nSheetNum;
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_MOTION, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(pMtnCfg[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
	}
	
	return nRet;
}

int CDevConfig::GetDevConfig_AlmCfgLoss(LONG lLoginID, VIDEO_LOST_ALARM_CFG *pLosCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (0 == device || NULL == pLosCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;

	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_LOSSDETECT))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_LOSSDETECT);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_LOSSDETECT))
	{
		nSheetNum = device->channelcount(device);

		int j = 0;
		CONFIG_LOSSDETECT *pCfgLos = (CONFIG_LOSSDETECT *)cRecvBuf;
		for (i = 0; i < 16; i++, pCfgLos++)
		{
			GetAlmActionMsk(CONFIG_TYPE_ALARM_LOSS, &pLosCfg[i].struHandle.dwActionMask);
			pLosCfg[i].byAlarmEn = pCfgLos->bEnable;
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pLosCfg[i].struHandle.struPtzLink[j].iValue = pCfgLos->hEvent.PtzLink[j].iValue;
				pLosCfg[i].struHandle.struPtzLink[j].iType = pCfgLos->hEvent.PtzLink[j].iType;
				pLosCfg[i].struHandle.byRecordChannel[j] = BITRHT(pCfgLos->hEvent.dwRecord,j)&1;
				pLosCfg[i].struHandle.byTour[j] = BITRHT(pCfgLos->hEvent.dwTour,j)&1;
				pLosCfg[i].struHandle.bySnap[j] = BITRHT(pCfgLos->hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pLosCfg[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgLos->hEvent.dwAlarmOut,j)&1;
				pLosCfg[i].struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgLos->hEvent.wiAlarmOut,j)&1;
			}
			pLosCfg[i].struHandle.dwDuration = pCfgLos->hEvent.iAOLatch;
			pLosCfg[i].struHandle.dwRecLatch = pCfgLos->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pLosCfg[i].struHandle.dwEventLatch = pCfgLos->hEvent.iEventLatch;
			pLosCfg[i].struHandle.bMessageToNet = (BYTE)pCfgLos->hEvent.bMessagetoNet;
			pLosCfg[i].struHandle.bMMSEn = (BYTE)pCfgLos->hEvent.bMMSEn;
			pLosCfg[i].struHandle.bySnapshotTimes = pCfgLos->hEvent.SnapshotTimes;//短信发送图片的张数
			pLosCfg[i].struHandle.bLog = (BYTE)pCfgLos->hEvent.bLog;
			pLosCfg[i].struHandle.bMatrixEn = (BYTE)pCfgLos->hEvent.bMatrixEn;
			pLosCfg[i].struHandle.dwMatrix = pCfgLos->hEvent.dwMatrix;
			GetAlmActionFlag(pCfgLos->hEvent, &pLosCfg[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(cRecvBuf, 0, iRecvBufLen);
	nSheetNum = nSheetNum>16?16:nSheetNum;
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_LOSS, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(pLosCfg[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int CDevConfig::GetDevConfig_AlmCfgBlind(LONG lLoginID, BLIND_ALARM_CFG *pBldCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (0 == device || NULL == pBldCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 16;

	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_BLINDDETECT))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_BLINDDETECT);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}

	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_BLINDDETECT))
	{
		nSheetNum = device->channelcount(device);

		int j = 0;
		CONFIG_BLINDDETECT *pCfgBld = (CONFIG_BLINDDETECT *)cRecvBuf;
		for (i = 0; i < 16; i++, pCfgBld++)
		{
			GetAlmActionMsk(CONFIG_TYPE_ALARM_BLIND, &pBldCfg[i].struHandle.dwActionMask);
			pBldCfg[i].byBlindEnable = pCfgBld->bEnable;
			pBldCfg[i].byBlindLevel = pCfgBld->iLevel;
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pBldCfg[i].struHandle.struPtzLink[j].iValue = pCfgBld->hEvent.PtzLink[j].iValue;
				pBldCfg[i].struHandle.struPtzLink[j].iType = pCfgBld->hEvent.PtzLink[j].iType;
				pBldCfg[i].struHandle.byRecordChannel[j] = BITRHT(pCfgBld->hEvent.dwRecord,j)&1;
				pBldCfg[i].struHandle.byTour[j] = BITRHT(pCfgBld->hEvent.dwTour,j)&1;
				pBldCfg[i].struHandle.bySnap[j] = BITRHT(pCfgBld->hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pBldCfg[i].struHandle.byRelAlarmOut[j] = BITRHT(pCfgBld->hEvent.dwAlarmOut,j)&1;
				pBldCfg[i].struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgBld->hEvent.wiAlarmOut,j)&1;
			}
			pBldCfg[i].struHandle.dwDuration = pCfgBld->hEvent.iAOLatch;
			pBldCfg[i].struHandle.dwRecLatch = pCfgBld->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pBldCfg[i].struHandle.dwEventLatch = pCfgBld->hEvent.iEventLatch;
			pBldCfg[i].struHandle.bMessageToNet = (BYTE)pCfgBld->hEvent.bMessagetoNet;
			pBldCfg[i].struHandle.bMMSEn = (BYTE)pCfgBld->hEvent.bMMSEn;
			pBldCfg[i].struHandle.bySnapshotTimes = pCfgBld->hEvent.SnapshotTimes;//短信发送图片的张数
			pBldCfg[i].struHandle.bLog = (BYTE)pCfgBld->hEvent.bLog;
			pBldCfg[i].struHandle.bMatrixEn = (BYTE)pCfgBld->hEvent.bMatrixEn;
			pBldCfg[i].struHandle.dwMatrix = pCfgBld->hEvent.dwMatrix;
			GetAlmActionFlag(pCfgBld->hEvent, &pBldCfg[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	memset(cRecvBuf, 0, iRecvBufLen);
	nSheetNum = nSheetNum>16?16:nSheetNum;
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_BLIND, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
	if (nRet >= 0)
	{
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < nSheetNum; i++,pWSheet++)
		{
			memcpy(pBldCfg[i].stSect, pWSheet->tsSchedule, sizeof(pWSheet->tsSchedule));
		}
	}
	else
	{
		nRet = -1;
		goto END;
	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int CDevConfig::GetDevConfig_AlmCfgDisk(LONG lLoginID, DISK_ALARM_CFG *pDiskAlmCfg, int waittime)
{
	if (0 == lLoginID || NULL == pDiskAlmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;

	int i = 0, x = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_DISKNOSPACE))
	{
		iRecvBufLen = sizeof(CONFIG_DISKNOSPACE);
	}
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	if(iRecvBufLen < sizeof(CONFIG_STORAGE_NUMBER_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_STORAGE_NUMBER_EVENT);
	}

	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKFULL, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_DISKNOSPACE))
	{
		int j = 0;
		CONFIG_DISKNOSPACE *pCfgDiskNoSpace = (CONFIG_DISKNOSPACE *)cRecvBuf;
		GetAlmActionMsk(CONFIG_TYPE_ALARM_DISKFULL, &pDiskAlmCfg->struLCHandle.dwActionMask);
		pDiskAlmCfg->byLowCapEn = pCfgDiskNoSpace->bEnable;
		pDiskAlmCfg->byLowerLimit = pCfgDiskNoSpace->iLowerLimit;
		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struLCHandle.struPtzLink[j].iValue = pCfgDiskNoSpace->hEvent.PtzLink[j].iValue;
			pDiskAlmCfg->struLCHandle.struPtzLink[j].iType = pCfgDiskNoSpace->hEvent.PtzLink[j].iType;
			pDiskAlmCfg->struLCHandle.byRecordChannel[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwRecord,j)&1;
			pDiskAlmCfg->struLCHandle.byTour[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwTour,j)&1;
			pDiskAlmCfg->struLCHandle.bySnap[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwSnapShot,j)&1;
		}
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struLCHandle.byRelAlarmOut[j] = BITRHT(pCfgDiskNoSpace->hEvent.dwAlarmOut,j)&1;
			pDiskAlmCfg->struLCHandle.byRelWIAlarmOut[j] =BITRHT(pCfgDiskNoSpace->hEvent.wiAlarmOut,j)&1;
		}
		pDiskAlmCfg->struLCHandle.dwDuration = pCfgDiskNoSpace->hEvent.iAOLatch;
		pDiskAlmCfg->struLCHandle.dwRecLatch = pCfgDiskNoSpace->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pDiskAlmCfg->struLCHandle.dwEventLatch = pCfgDiskNoSpace->hEvent.iEventLatch;
		pDiskAlmCfg->struLCHandle.bMessageToNet = (BYTE)pCfgDiskNoSpace->hEvent.bMessagetoNet;
		
		pDiskAlmCfg->struLCHandle.bMMSEn = (BYTE)pCfgDiskNoSpace->hEvent.bMMSEn;
		pDiskAlmCfg->struLCHandle.bySnapshotTimes = pCfgDiskNoSpace->hEvent.SnapshotTimes;//短信发送图片的张数
		pDiskAlmCfg->struLCHandle.bLog = (BYTE)pCfgDiskNoSpace->hEvent.bLog;
		pDiskAlmCfg->struLCHandle.bMatrixEn = (BYTE)pCfgDiskNoSpace->hEvent.bMatrixEn;
		pDiskAlmCfg->struLCHandle.dwMatrix = pCfgDiskNoSpace->hEvent.dwMatrix;
		GetAlmActionFlag(pCfgDiskNoSpace->hEvent, &pDiskAlmCfg->struLCHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NODISK, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		int j = 0;
		CONFIG_GENERIC_EVENT *pCfgNoDisk = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		GetAlmActionMsk(CONFIG_TYPE_ALARM_NODISK, &pDiskAlmCfg->struHdvrHandle.dwActionMask);
		pDiskAlmCfg->byNoDiskEn = pCfgNoDisk->bEnable;
		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iValue = pCfgNoDisk->hEvent.PtzLink[j].iValue;
			pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iType = pCfgNoDisk->hEvent.PtzLink[j].iType;
			pDiskAlmCfg->struHdvrHandle.byRecordChannel[j] = BITRHT(pCfgNoDisk->hEvent.dwRecord,j)&1;
			pDiskAlmCfg->struHdvrHandle.byTour[j] = BITRHT(pCfgNoDisk->hEvent.dwTour,j)&1;
			pDiskAlmCfg->struHdvrHandle.bySnap[j] = BITRHT(pCfgNoDisk->hEvent.dwSnapShot,j)&1;
		}
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struHdvrHandle.byRelAlarmOut[j] = BITRHT(pCfgNoDisk->hEvent.dwAlarmOut,j)&1;
			pDiskAlmCfg->struHdvrHandle.byRelWIAlarmOut[j] =BITRHT(pCfgNoDisk->hEvent.dwAlarmOut,j)&1;
		}
		pDiskAlmCfg->struHdvrHandle.dwDuration = pCfgNoDisk->hEvent.iAOLatch;
		pDiskAlmCfg->struHdvrHandle.dwRecLatch = pCfgNoDisk->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pDiskAlmCfg->struHdvrHandle.dwEventLatch = pCfgNoDisk->hEvent.iEventLatch;
		pDiskAlmCfg->struHdvrHandle.bMessageToNet = (BYTE)pCfgNoDisk->hEvent.bMessagetoNet;
		pDiskAlmCfg->struHdvrHandle.bMMSEn = (BYTE)pCfgNoDisk->hEvent.bMMSEn;
		pDiskAlmCfg->struHdvrHandle.bySnapshotTimes = pCfgNoDisk->hEvent.SnapshotTimes;//短信发送图片的张数
		pDiskAlmCfg->struHdvrHandle.bLog = (BYTE)pCfgNoDisk->hEvent.bLog;
		pDiskAlmCfg->struHdvrHandle.bMatrixEn = (BYTE)pCfgNoDisk->hEvent.bMatrixEn;
		pDiskAlmCfg->struHdvrHandle.dwMatrix = pCfgNoDisk->hEvent.dwMatrix;
		GetAlmActionFlag(pCfgNoDisk->hEvent, &pDiskAlmCfg->struHdvrHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKERR, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		int j = 0;
		CONFIG_GENERIC_EVENT *pCfgDiskErr = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		GetAlmActionMsk(CONFIG_TYPE_ALARM_DISKERR, &pDiskAlmCfg->struMsgHandle.dwActionMask);
		pDiskAlmCfg->byDiskErrEn = pCfgDiskErr->bEnable;
		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struMsgHandle.struPtzLink[j].iValue = pCfgDiskErr->hEvent.PtzLink[j].iValue;
			pDiskAlmCfg->struMsgHandle.struPtzLink[j].iType = pCfgDiskErr->hEvent.PtzLink[j].iType;
			pDiskAlmCfg->struMsgHandle.byRecordChannel[j] = BITRHT(pCfgDiskErr->hEvent.dwRecord,j)&1;
			pDiskAlmCfg->struMsgHandle.byTour[j] = BITRHT(pCfgDiskErr->hEvent.dwTour,j)&1;
			pDiskAlmCfg->struMsgHandle.bySnap[j] = BITRHT(pCfgDiskErr->hEvent.dwSnapShot,j)&1;
		}
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pDiskAlmCfg->struMsgHandle.byRelAlarmOut[j] = BITRHT(pCfgDiskErr->hEvent.dwAlarmOut,j)&1;
			pDiskAlmCfg->struMsgHandle.byRelWIAlarmOut[j] =BITRHT(pCfgDiskErr->hEvent.wiAlarmOut,j)&1;
		}
		pDiskAlmCfg->struMsgHandle.dwDuration = pCfgDiskErr->hEvent.iAOLatch;
		pDiskAlmCfg->struMsgHandle.dwRecLatch = pCfgDiskErr->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pDiskAlmCfg->struMsgHandle.dwEventLatch = pCfgDiskErr->hEvent.iEventLatch;
		pDiskAlmCfg->struMsgHandle.bMessageToNet = (BYTE)pCfgDiskErr->hEvent.bMessagetoNet;
		pDiskAlmCfg->struMsgHandle.bMMSEn = (BYTE)pCfgDiskErr->hEvent.bMMSEn;
		pDiskAlmCfg->struMsgHandle.bySnapshotTimes = pCfgDiskErr->hEvent.SnapshotTimes;//短信发送图片的张数
		pDiskAlmCfg->struMsgHandle.bLog = (BYTE)pCfgDiskErr->hEvent.bLog;
		pDiskAlmCfg->struMsgHandle.bMatrixEn = (BYTE)pCfgDiskErr->hEvent.bMatrixEn;
		pDiskAlmCfg->struMsgHandle.dwMatrix = pCfgDiskErr->hEvent.dwMatrix;
		GetAlmActionFlag(pCfgDiskErr->hEvent, &pDiskAlmCfg->struMsgHandle.dwActionFlag);

		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKNUM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if(nRet >=0 && retlen == sizeof(CONFIG_STORAGE_NUMBER_EVENT))
		{
			CONFIG_STORAGE_NUMBER_EVENT *pCfgDiskNum = (CONFIG_STORAGE_NUMBER_EVENT *)cRecvBuf;
			pDiskAlmCfg->bDiskNum = pCfgDiskNum->iHddNumber;
		}
	}
	
	else
	{
		nRet = -1;
		goto END;
	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int CDevConfig::GetDevConfig_AlmCfgNetBroken(LONG lLoginID, NETBROKEN_ALARM_CFG *pNetBrkCfg, int waittime)
{
	if (0 == lLoginID || NULL == pNetBrkCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;

	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETBROKEN, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgNetBrk = (CONFIG_GENERIC_EVENT *)cRecvBuf;
		GetAlmActionMsk(CONFIG_TYPE_ALARM_NETBROKEN, &pNetBrkCfg->struHandle.dwActionMask);
		pNetBrkCfg->byEnable = pCfgNetBrk->bEnable;
		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pNetBrkCfg->struHandle.struPtzLink[j].iValue = pCfgNetBrk->hEvent.PtzLink[j].iValue;
			pNetBrkCfg->struHandle.struPtzLink[j].iType = pCfgNetBrk->hEvent.PtzLink[j].iType;
			pNetBrkCfg->struHandle.byRecordChannel[j] = BITRHT(pCfgNetBrk->hEvent.dwRecord,j)&1;
			pNetBrkCfg->struHandle.byTour[j] = BITRHT(pCfgNetBrk->hEvent.dwTour,j)&1;
			pNetBrkCfg->struHandle.bySnap[j] = BITRHT(pCfgNetBrk->hEvent.dwSnapShot,j)&1;
		}
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pNetBrkCfg->struHandle.byRelAlarmOut[j] = BITRHT(pCfgNetBrk->hEvent.dwAlarmOut,j)&1;
			pNetBrkCfg->struHandle.byRelWIAlarmOut[j] =BITRHT(pCfgNetBrk->hEvent.wiAlarmOut,j)&1;
		}
		pNetBrkCfg->struHandle.dwDuration = pCfgNetBrk->hEvent.iAOLatch;
		pNetBrkCfg->struHandle.dwRecLatch = pCfgNetBrk->hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
		pNetBrkCfg->struHandle.dwEventLatch = pCfgNetBrk->hEvent.iEventLatch;
		pNetBrkCfg->struHandle.bMessageToNet = (BYTE)pCfgNetBrk->hEvent.bMessagetoNet;
		pNetBrkCfg->struHandle.bMMSEn = (BYTE)pCfgNetBrk->hEvent.bMMSEn;
		pNetBrkCfg->struHandle.bySnapshotTimes = pCfgNetBrk->hEvent.SnapshotTimes;//短信发送图片的张数
		pNetBrkCfg->struHandle.bLog = (BYTE)pCfgNetBrk->hEvent.bLog;
		pNetBrkCfg->struHandle.bMatrixEn = (BYTE)pCfgNetBrk->hEvent.bMatrixEn;
		pNetBrkCfg->struHandle.dwMatrix = pCfgNetBrk->hEvent.dwMatrix;
		GetAlmActionFlag(pCfgNetBrk->hEvent, &pNetBrkCfg->struHandle.dwActionFlag);
	}
	else
	{
		nRet = -1;
		goto END;
	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}
/*
 *	yehao(10857) 2007-04-26:
 *	为了程序的清晰和便于维护,把每种报警配置的获取分别做成单独的成员函数
 *	这里把每种报警的配置获取分别做成一个成员函数,参数中把时间表传进去,这样做的原因是:
 *	1.增加程序的清晰性;
 *	2.减少不必要的网络通讯冗余;
 *	3.减少多线程操作时的影响(用参数传入,而不是用类的成员变量);
 */
int CDevConfig::GetDevConfig_AlmCfgNew(LONG lLoginID, DEV_ALARM_SCHEDULE &stAs, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);


	memset(&stAs, 0, sizeof(DEV_ALARM_SCHEDULE));
	stAs.dwSize = sizeof(DEV_ALARM_SCHEDULE);
	int nRet = -1;
	int nAlarmInNum = device->alarminputcount(device);
	
	if ((nAlarmInNum != 0) && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]) )
	{
		nRet = GetDevConfig_AlmCfgLocalAlarm(lLoginID, stAs.struLocalAlmIn, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
		nRet = GetDevConfig_AlmCfgNetAlarm(lLoginID, stAs.struNetAlmIn, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
	}
	
	//	此功能解码器除外
	if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
	{
		if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_VIDEOCONF])
		{
			nRet = GetDevConfig_AlmCfgMotion(lLoginID, stAs.struMotion, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			nRet = GetDevConfig_AlmCfgLoss(lLoginID, stAs.struVideoLost, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			nRet = GetDevConfig_AlmCfgBlind(lLoginID, stAs.struBlind, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
		}
		
		nRet = GetDevConfig_AlmCfgDisk(lLoginID, &stAs.struDiskAlarm, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
		nRet = GetDevConfig_AlmCfgNetBroken(lLoginID, &stAs.struNetBrokenAlarm, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
	}
	
	return 0;
}

int	CDevConfig::SetDevConfig_AlmCfgLocalAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
	if (NULL == device || NULL == pAlmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	int nSheetNum = 0;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_ALARM_X))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_ALARM_X);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);

	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_ALARM_X))
	{
		CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)cRecvBuf;
		for (i = 0; i < 16; i++,pCfgAlmX++)
		{
			SetAlmActionFlag(&pCfgAlmX->hEvent, pAlmCfg[i].struHandle.dwActionFlag);

			pCfgAlmX->bEnable = pAlmCfg[i].byAlarmEn;
			pCfgAlmX->iSensorType = pAlmCfg[i].byAlarmType;

			pCfgAlmX->hEvent.dwRecord = 0;
			pCfgAlmX->hEvent.dwTour = 0;
			pCfgAlmX->hEvent.dwSnapShot = 0;

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgAlmX->hEvent.dwSnapShot |= (pAlmCfg[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.dwTour |= (pAlmCfg[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.dwRecord |= (pAlmCfg[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.PtzLink[j].iValue = pAlmCfg[i].struHandle.struPtzLink[j].iValue;
				pCfgAlmX->hEvent.PtzLink[j].iType = pAlmCfg[i].struHandle.struPtzLink[j].iType;
			}

			pCfgAlmX->hEvent.dwAlarmOut = 0;
			pCfgAlmX->hEvent.wiAlarmOut = 0;
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgAlmX->hEvent.dwAlarmOut |= (pAlmCfg[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.wiAlarmOut |= (pAlmCfg[i].struHandle.byRelWIAlarmOut[j]) ?(0x01<<j) : 0;
			}
			pCfgAlmX->hEvent.iAOLatch = pAlmCfg[i].struHandle.dwDuration;
			pCfgAlmX->hEvent.iRecordLatch = pAlmCfg[i].struHandle.dwRecLatch;//把预录时间取消用来表示录像延时，变量名待改
			pCfgAlmX->hEvent.iEventLatch = pAlmCfg[i].struHandle.dwEventLatch;
			pCfgAlmX->hEvent.bMessagetoNet = (BOOL)pAlmCfg[i].struHandle.bMessageToNet;
			pCfgAlmX->hEvent.bMMSEn = pAlmCfg[i].struHandle.bMMSEn;
			pCfgAlmX->hEvent.SnapshotTimes = pAlmCfg[i].struHandle.bySnapshotTimes;//短信发送图片的张数
			pCfgAlmX->hEvent.bLog = (BOOL)pAlmCfg[i].struHandle.bLog;
			pCfgAlmX->hEvent.bMatrixEn = (BOOL)pAlmCfg[i].struHandle.bMatrixEn;
			pCfgAlmX->hEvent.dwMatrix = pAlmCfg[i].struHandle.dwMatrix;
		}

		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_LOCALALM, 0, cRecvBuf, 16*sizeof(CONFIG_ALARM_X), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}
	
	Sleep(SETUP_SLEEP);

	{
		device->get_info(device, dit_alarmin_num, &nSheetNum);
		memset(cRecvBuf, 0, iRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < nSheetNum; i++,pWSheet++)
		{
			pWSheet->iName = i;
			memcpy(pWSheet->tsSchedule, pAlmCfg[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_LOCAL_ALARM, (CONFIG_WORKSHEET*)cRecvBuf, waittime, nSheetNum);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}

	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgNetAlarm(LONG lLoginID, ALARM_IN_CFG *pAlmCfg, int waittime)
{
	if (0 == lLoginID || NULL == pAlmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_ALARM_X))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_ALARM_X);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_ALARM_X))
	{
		int j = 0;
		CONFIG_ALARM_X *pCfgAlmX = (CONFIG_ALARM_X *)cRecvBuf;
		for (i = 0; i < 16; i++,pCfgAlmX++)
		{
			SetAlmActionFlag(&pCfgAlmX->hEvent, pAlmCfg[i].struHandle.dwActionFlag);
			
			pCfgAlmX->bEnable = pAlmCfg[i].byAlarmEn;
			pCfgAlmX->iSensorType = pAlmCfg[i].byAlarmType;
			
			pCfgAlmX->hEvent.dwRecord = 0;
			pCfgAlmX->hEvent.dwTour = 0;
			pCfgAlmX->hEvent.dwSnapShot = 0;
			
			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgAlmX->hEvent.dwSnapShot |= (pAlmCfg[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.dwTour |= (pAlmCfg[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.dwRecord |= (pAlmCfg[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.PtzLink[j].iValue = pAlmCfg[i].struHandle.struPtzLink[j].iValue;
				pCfgAlmX->hEvent.PtzLink[j].iType = pAlmCfg[i].struHandle.struPtzLink[j].iType;
			}
			
			pCfgAlmX->hEvent.dwAlarmOut = 0;
			pCfgAlmX->hEvent.wiAlarmOut = 0;
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgAlmX->hEvent.dwAlarmOut |= (pAlmCfg[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pCfgAlmX->hEvent.wiAlarmOut |= (pAlmCfg[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
			}
			pCfgAlmX->hEvent.iAOLatch = pAlmCfg[i].struHandle.dwDuration;
			pCfgAlmX->hEvent.iRecordLatch = pAlmCfg[i].struHandle.dwRecLatch;//把预录时间取消用来表示录像延时，变量名待改
			pCfgAlmX->hEvent.iEventLatch = pAlmCfg[i].struHandle.dwEventLatch;
			pCfgAlmX->hEvent.bMessagetoNet = (BOOL)pAlmCfg[i].struHandle.bMessageToNet;
			pCfgAlmX->hEvent.bMMSEn = pAlmCfg[i].struHandle.bMMSEn;
			pCfgAlmX->hEvent.SnapshotTimes = pAlmCfg[i].struHandle.bySnapshotTimes;//短信发送图片的张数
			pCfgAlmX->hEvent.bLog = (BOOL)pAlmCfg[i].struHandle.bLog;
			pCfgAlmX->hEvent.bMatrixEn = (BOOL)pAlmCfg[i].struHandle.bMatrixEn;
			pCfgAlmX->hEvent.dwMatrix = pAlmCfg[i].struHandle.dwMatrix;
		}
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_NETALM, 0, cRecvBuf, 16*sizeof(CONFIG_ALARM_X), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	
	/*
	Sleep(SETUP_SLEEP);
	
	{
		memset(cRecvBuf, 0, iRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < 16; i++,pWSheet++)
		{
			memcpy(pWSheet->tsSchedule, pAlmCfg[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_NET_ALARM, (CONFIG_WORKSHEET*)cRecvBuf,waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	*/
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgMotion(LONG lLoginID, MOTION_DETECT_CFG *pMtnCfg, int waittime)
{
	if (0 == lLoginID || NULL == pMtnCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_MOTIONDETECT))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_MOTIONDETECT);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_MOTIONDETECT))
	{
		CONFIG_MOTIONDETECT *pCfgMtn = (CONFIG_MOTIONDETECT *)cRecvBuf;
		for (i = 0; i < 16; i++,pCfgMtn++)
		{
			SetAlmActionFlag(&pCfgMtn->hEvent, pMtnCfg[i].struHandle.dwActionFlag);
			pCfgMtn->bEnable = pMtnCfg[i].byMotionEn;
			pCfgMtn->iLevel = pMtnCfg[i].wSenseLevel;

			//Begin: Add by zsc(11402)	2008-1-18
			int nMaxRow	= Min(pMtnCfg[i].wMotionRow, 32);
			nMaxRow		= Min(nMaxRow, MOTION_ROW_NUM);
			int nMaxCol	= Min(pMtnCfg[i].wMotionCol, 32);
			nMaxCol		= Min(nMaxCol, MOTION_COL_NUM);
			for (j = 0; j < nMaxRow; j++)
			{
				pCfgMtn->iRegion[j] = 0;
				for (int k = 0; k < nMaxCol; k++)
				{
					if (pMtnCfg[i].byDetected[j][k])
					{
						pCfgMtn->iRegion[j] |= (1<<k);
					}
				}
			}
			//End:	zsc(11402)	2008-1-18

			pCfgMtn->hEvent.dwRecord = 0;
			pCfgMtn->hEvent.dwTour = 0;
			pCfgMtn->hEvent.dwSnapShot = 0;

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgMtn->hEvent.dwSnapShot |= (pMtnCfg[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pCfgMtn->hEvent.dwTour |= (pMtnCfg[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pCfgMtn->hEvent.dwRecord |= (pMtnCfg[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pCfgMtn->hEvent.PtzLink[j].iValue = pMtnCfg[i].struHandle.struPtzLink[j].iValue;
				pCfgMtn->hEvent.PtzLink[j].iType = pMtnCfg[i].struHandle.struPtzLink[j].iType;
			}

			pCfgMtn->hEvent.dwAlarmOut = 0;
			pCfgMtn->hEvent.wiAlarmOut = 0;
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgMtn->hEvent.dwAlarmOut |= (pMtnCfg[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pCfgMtn->hEvent.wiAlarmOut |= (pMtnCfg[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
			}
			pCfgMtn->hEvent.iAOLatch = pMtnCfg[i].struHandle.dwDuration;
			pCfgMtn->hEvent.iRecordLatch = pMtnCfg[i].struHandle.dwRecLatch;
			pCfgMtn->hEvent.iEventLatch = pMtnCfg[i].struHandle.dwEventLatch;
			pCfgMtn->hEvent.bMessagetoNet = (BOOL)pMtnCfg[i].struHandle.bMessageToNet;
			pCfgMtn->hEvent.bMMSEn = pMtnCfg[i].struHandle.bMMSEn;
			pCfgMtn->hEvent.SnapshotTimes = pMtnCfg[i].struHandle.bySnapshotTimes;//短信发送图片的张数
			pCfgMtn->hEvent.bLog = (BOOL)pMtnCfg[i].struHandle.bLog;
			pCfgMtn->hEvent.bMatrixEn = (BOOL)pMtnCfg[i].struHandle.bMatrixEn;
			pCfgMtn->hEvent.dwMatrix = pMtnCfg[i].struHandle.dwMatrix;
		}
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_MOTION, 0, cRecvBuf, 16*sizeof(CONFIG_MOTIONDETECT), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	
	Sleep(SETUP_SLEEP);

	{
		memset(cRecvBuf, 0, iRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < 16; i++,pWSheet++)
		{
			pWSheet->iName = i;
			memcpy(pWSheet->tsSchedule, pMtnCfg[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_MOTION, (CONFIG_WORKSHEET*)cRecvBuf, waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgLoss(LONG lLoginID, VIDEO_LOST_ALARM_CFG *pLosCfg, int waittime)
{
	if (0 == lLoginID || NULL == pLosCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;

	int nRetLen = 0;
	bool bSupport = false;
	DEV_ENABLE_INFO stDevEn = {0};
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_LOSSDETECT))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_LOSSDETECT);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_LOSSDETECT))
	{
		CONFIG_LOSSDETECT *pCfgLos = (CONFIG_LOSSDETECT *)cRecvBuf;
		for (i = 0; i < 16; i++, pCfgLos++)
		{
			SetAlmActionFlag(&pCfgLos->hEvent, pLosCfg[i].struHandle.dwActionFlag);
			pCfgLos->bEnable = pLosCfg[i].byAlarmEn;

			pCfgLos->hEvent.dwRecord = 0;
			pCfgLos->hEvent.dwTour = 0;
			pCfgLos->hEvent.dwSnapShot = 0;

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgLos->hEvent.dwSnapShot |= (pLosCfg[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pCfgLos->hEvent.dwTour |= (pLosCfg[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pCfgLos->hEvent.dwRecord |= (pLosCfg[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pCfgLos->hEvent.PtzLink[j].iValue = pLosCfg[i].struHandle.struPtzLink[j].iValue;
				pCfgLos->hEvent.PtzLink[j].iType = pLosCfg[i].struHandle.struPtzLink[j].iType;
			}
			
			pCfgLos->hEvent.dwAlarmOut = 0;
			pCfgLos->hEvent.wiAlarmOut = 0;
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgLos->hEvent.dwAlarmOut |= (pLosCfg[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pCfgLos->hEvent.wiAlarmOut |= (pLosCfg[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
			}
			pCfgLos->hEvent.iAOLatch = pLosCfg[i].struHandle.dwDuration;
			pCfgLos->hEvent.iRecordLatch = pLosCfg[i].struHandle.dwRecLatch;
			pCfgLos->hEvent.iEventLatch = pLosCfg[i].struHandle.dwEventLatch;
			pCfgLos->hEvent.bMessagetoNet = (BOOL)pLosCfg[i].struHandle.bMessageToNet;
			pCfgLos->hEvent.bMMSEn = pLosCfg[i].struHandle.bMMSEn;
			pCfgLos->hEvent.SnapshotTimes = pLosCfg[i].struHandle.bySnapshotTimes;//短信发送图片的张数
			pCfgLos->hEvent.bLog = (BOOL)pLosCfg[i].struHandle.bLog;
			pCfgLos->hEvent.bMatrixEn = (BOOL)pLosCfg[i].struHandle.bMatrixEn;
			pCfgLos->hEvent.dwMatrix = pLosCfg[i].struHandle.dwMatrix;
		}
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_LOSS, 0, cRecvBuf, 16*sizeof(CONFIG_LOSSDETECT), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}
	
	Sleep(SETUP_SLEEP);

	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_DAVINCIMODULE] != 0)
		{
			bSupport = true;
		}
	}
	
	if (bSupport)
	{
		memset(cRecvBuf, 0, iRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < 16; i++,pWSheet++)
		{
			pWSheet->iName = i;
			memcpy(pWSheet->tsSchedule, pLosCfg[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_LOSS, (CONFIG_WORKSHEET*)cRecvBuf, waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}

	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgBlind(LONG lLoginID, BLIND_ALARM_CFG *pBldCfg, int waittime)
{
	if (0 == lLoginID || NULL == pBldCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;

	int nRetLen = 0;
	bool bSupport = false;
	DEV_ENABLE_INFO stDevEn = {0};
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < 16 * sizeof(CONFIG_BLINDDETECT))
	{
		iRecvBufLen = 16 * sizeof(CONFIG_BLINDDETECT);
	}
	if (iRecvBufLen < 16 * sizeof(CONFIG_WORKSHEET)) 
	{
		iRecvBufLen = 16 * sizeof(CONFIG_WORKSHEET);
	}
	
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == 16*sizeof(CONFIG_BLINDDETECT))
	{
		CONFIG_BLINDDETECT *pCfgBld = (CONFIG_BLINDDETECT *)cRecvBuf;
		for (i = 0; i < 16; i++, pCfgBld++)
		{
			SetAlmActionFlag(&pCfgBld->hEvent, pBldCfg[i].struHandle.dwActionFlag);
			pCfgBld->bEnable = pBldCfg[i].byBlindEnable;
			pCfgBld->iLevel = pBldCfg[i].byBlindLevel;
			
			pCfgBld->hEvent.dwRecord = 0;
			pCfgBld->hEvent.dwTour = 0;
			pCfgBld->hEvent.dwSnapShot = 0;

			int index = min(32, MAX_VIDEO_IN_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgBld->hEvent.dwSnapShot |= (pBldCfg[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
				pCfgBld->hEvent.dwTour |= (pBldCfg[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
				pCfgBld->hEvent.dwRecord |= (pBldCfg[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
				pCfgBld->hEvent.PtzLink[j].iValue = pBldCfg[i].struHandle.struPtzLink[j].iValue;
				pCfgBld->hEvent.PtzLink[j].iType = pBldCfg[i].struHandle.struPtzLink[j].iType;
			}
			
			pCfgBld->hEvent.dwAlarmOut = 0;
			pCfgBld->hEvent.wiAlarmOut = 0;
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pCfgBld->hEvent.dwAlarmOut |= (pBldCfg[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
				pCfgBld->hEvent.wiAlarmOut |= (pBldCfg[i].struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
			}
			pCfgBld->hEvent.iAOLatch = pBldCfg[i].struHandle.dwDuration;
			pCfgBld->hEvent.iRecordLatch = pBldCfg[i].struHandle.dwRecLatch;
			pCfgBld->hEvent.iEventLatch = pBldCfg[i].struHandle.dwEventLatch;
			pCfgBld->hEvent.bMessagetoNet = (BOOL)pBldCfg[i].struHandle.bMessageToNet;
			pCfgBld->hEvent.bMMSEn = pBldCfg[i].struHandle.bMMSEn;
			pCfgBld->hEvent.SnapshotTimes = pBldCfg[i].struHandle.bySnapshotTimes;//短信发送图片的张数
			pCfgBld->hEvent.bLog = (BOOL)pBldCfg[i].struHandle.bLog;
			pCfgBld->hEvent.bMatrixEn = (BOOL)pBldCfg[i].struHandle.bMatrixEn;
			pCfgBld->hEvent.dwMatrix = pBldCfg[i].struHandle.dwMatrix;
		}
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_BLIND, 0, cRecvBuf, 16*sizeof(CONFIG_BLINDDETECT), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}
	
	Sleep(SETUP_SLEEP);
	
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_DAVINCIMODULE] != 0)
		{
			bSupport = true;
		}
	}
	
	if (bSupport)
	{
		memset(cRecvBuf, 0, iRecvBufLen);
		CONFIG_WORKSHEET *pWSheet = (CONFIG_WORKSHEET *)cRecvBuf;
		for (i = 0; i < 16; i++,pWSheet++)
		{
			pWSheet->iName = i;
			memcpy(pWSheet->tsSchedule, pBldCfg[i].stSect, sizeof(pWSheet->tsSchedule));
		}
		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_BLIND, (CONFIG_WORKSHEET*)cRecvBuf, waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
	
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgDisk(LONG lLoginID, DISK_ALARM_CFG *pDiskAlmCfg, int waittime)
{
	if (0 == lLoginID || NULL == pDiskAlmCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;

	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_DISKNOSPACE))
	{
		iRecvBufLen = sizeof(CONFIG_DISKNOSPACE);
	}
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	if (iRecvBufLen < sizeof(CONFIG_STORAGE_NUMBER_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_STORAGE_NUMBER_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKFULL, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_DISKNOSPACE))
	{
		CONFIG_DISKNOSPACE *pCfgDiskNoSpace = (CONFIG_DISKNOSPACE *)cRecvBuf;
		SetAlmActionFlag(&pCfgDiskNoSpace->hEvent, pDiskAlmCfg->struLCHandle.dwActionFlag);
		pCfgDiskNoSpace->bEnable = pDiskAlmCfg->byLowCapEn;
		pCfgDiskNoSpace->iLowerLimit = pDiskAlmCfg->byLowerLimit;
		
		pCfgDiskNoSpace->hEvent.dwRecord = 0;
		pCfgDiskNoSpace->hEvent.dwTour = 0;
		pCfgDiskNoSpace->hEvent.dwSnapShot = 0;

		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgDiskNoSpace->hEvent.dwSnapShot |= (pDiskAlmCfg->struLCHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.dwTour |= (pDiskAlmCfg->struLCHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.dwRecord |= (pDiskAlmCfg->struLCHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.PtzLink[j].iValue = pDiskAlmCfg->struLCHandle.struPtzLink[j].iValue;
			pCfgDiskNoSpace->hEvent.PtzLink[j].iType = pDiskAlmCfg->struLCHandle.struPtzLink[j].iType;
		}

		pCfgDiskNoSpace->hEvent.dwAlarmOut = 0;
		pCfgDiskNoSpace->hEvent.wiAlarmOut = 0;
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgDiskNoSpace->hEvent.dwAlarmOut |= (pDiskAlmCfg->struLCHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			pCfgDiskNoSpace->hEvent.wiAlarmOut |= (pDiskAlmCfg->struLCHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgDiskNoSpace->hEvent.iAOLatch = pDiskAlmCfg->struLCHandle.dwDuration;
		pCfgDiskNoSpace->hEvent.iRecordLatch = pDiskAlmCfg->struLCHandle.dwRecLatch;
		pCfgDiskNoSpace->hEvent.iEventLatch = pDiskAlmCfg->struLCHandle.dwEventLatch;
		pCfgDiskNoSpace->hEvent.bMessagetoNet = (BOOL)pDiskAlmCfg->struLCHandle.bMessageToNet;
		pCfgDiskNoSpace->hEvent.bMMSEn = pDiskAlmCfg->struLCHandle.bMMSEn;
		pCfgDiskNoSpace->hEvent.SnapshotTimes = pDiskAlmCfg->struLCHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgDiskNoSpace->hEvent.bLog = (BOOL)pDiskAlmCfg->struLCHandle.bLog;
		pCfgDiskNoSpace->hEvent.bMatrixEn = (BOOL)pDiskAlmCfg->struLCHandle.bMatrixEn;
		pCfgDiskNoSpace->hEvent.dwMatrix = pDiskAlmCfg->struLCHandle.dwMatrix;
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DISKFULL, 0, cRecvBuf, sizeof(CONFIG_DISKNOSPACE), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	Sleep(SETUP_SLEEP);

	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NODISK, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgNoDisk = (CONFIG_GENERIC_EVENT *)cRecvBuf;

		SetAlmActionFlag(&pCfgNoDisk->hEvent, pDiskAlmCfg->struHdvrHandle.dwActionFlag);
		pCfgNoDisk->bEnable = pDiskAlmCfg->byNoDiskEn;
		
		pCfgNoDisk->hEvent.dwRecord = 0;
		pCfgNoDisk->hEvent.dwTour = 0;
		pCfgNoDisk->hEvent.dwSnapShot = 0;

		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgNoDisk->hEvent.dwSnapShot |= (pDiskAlmCfg->struHdvrHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.dwTour |= (pDiskAlmCfg->struHdvrHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.dwRecord |= (pDiskAlmCfg->struHdvrHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.PtzLink[j].iValue = pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iValue;
			pCfgNoDisk->hEvent.PtzLink[j].iType = pDiskAlmCfg->struHdvrHandle.struPtzLink[j].iType;
		}

		pCfgNoDisk->hEvent.dwAlarmOut = 0;
		pCfgNoDisk->hEvent.wiAlarmOut = 0;
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgNoDisk->hEvent.dwAlarmOut |= (pDiskAlmCfg->struHdvrHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			pCfgNoDisk->hEvent.wiAlarmOut |= (pDiskAlmCfg->struHdvrHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgNoDisk->hEvent.iAOLatch = pDiskAlmCfg->struHdvrHandle.dwDuration;
		pCfgNoDisk->hEvent.iRecordLatch = pDiskAlmCfg->struHdvrHandle.dwRecLatch;
		pCfgNoDisk->hEvent.iEventLatch = pDiskAlmCfg->struHdvrHandle.dwEventLatch;
		pCfgNoDisk->hEvent.bMessagetoNet = (BOOL)pDiskAlmCfg->struHdvrHandle.bMessageToNet;
		pCfgNoDisk->hEvent.bMMSEn = pDiskAlmCfg->struHdvrHandle.bMMSEn;
		pCfgNoDisk->hEvent.SnapshotTimes = pDiskAlmCfg->struHdvrHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgNoDisk->hEvent.bLog = (BOOL)pDiskAlmCfg->struHdvrHandle.bLog;
		pCfgNoDisk->hEvent.bMatrixEn = (BOOL)pDiskAlmCfg->struHdvrHandle.bMatrixEn;
		pCfgNoDisk->hEvent.dwMatrix = pDiskAlmCfg->struHdvrHandle.dwMatrix;
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_NODISK, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	Sleep(SETUP_SLEEP);

	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKERR, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgDiskErr = (CONFIG_GENERIC_EVENT *)cRecvBuf;

		SetAlmActionFlag(&pCfgDiskErr->hEvent, pDiskAlmCfg->struMsgHandle.dwActionFlag);
		pCfgDiskErr->bEnable = pDiskAlmCfg->byDiskErrEn;
		
		pCfgDiskErr->hEvent.dwRecord = 0;
		pCfgDiskErr->hEvent.dwTour = 0;
		pCfgDiskErr->hEvent.dwSnapShot = 0;

		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgDiskErr->hEvent.dwSnapShot |= (pDiskAlmCfg->struMsgHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.dwTour |= (pDiskAlmCfg->struMsgHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.dwRecord |= (pDiskAlmCfg->struMsgHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.PtzLink[j].iValue = pDiskAlmCfg->struMsgHandle.struPtzLink[j].iValue;
			pCfgDiskErr->hEvent.PtzLink[j].iType = pDiskAlmCfg->struMsgHandle.struPtzLink[j].iType;			
		}

		pCfgDiskErr->hEvent.dwAlarmOut = 0;
		pCfgDiskErr->hEvent.wiAlarmOut = 0;
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgDiskErr->hEvent.dwAlarmOut |= (pDiskAlmCfg->struMsgHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			pCfgDiskErr->hEvent.wiAlarmOut |= (pDiskAlmCfg->struMsgHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgDiskErr->hEvent.iAOLatch = pDiskAlmCfg->struMsgHandle.dwDuration;
		pCfgDiskErr->hEvent.iRecordLatch = pDiskAlmCfg->struMsgHandle.dwRecLatch;
		pCfgDiskErr->hEvent.iEventLatch = pDiskAlmCfg->struMsgHandle.dwEventLatch;
		pCfgDiskErr->hEvent.bMessagetoNet = (BOOL)pDiskAlmCfg->struMsgHandle.bMessageToNet;
		pCfgDiskErr->hEvent.bMMSEn = pDiskAlmCfg->struMsgHandle.bMMSEn;
		pCfgDiskErr->hEvent.SnapshotTimes = pDiskAlmCfg->struMsgHandle.bySnapshotTimes;
		pCfgDiskErr->hEvent.bLog = (BOOL)pDiskAlmCfg->struMsgHandle.bLog;
		pCfgDiskErr->hEvent.bMatrixEn = (BOOL)pDiskAlmCfg->struMsgHandle.bMatrixEn;
		pCfgDiskErr->hEvent.dwMatrix = pDiskAlmCfg->struMsgHandle.dwMatrix;
		
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DISKERR, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}

		/*
		 *	先读取配置，再将硬盘数目保存过去。
		 */
		memset(cRecvBuf, 0, iRecvBufLen);
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_DISKNUM, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
		if (nRet >= 0 && retlen == sizeof(CONFIG_STORAGE_NUMBER_EVENT))
		{
			CONFIG_STORAGE_NUMBER_EVENT *pCfgDiskNum = (CONFIG_STORAGE_NUMBER_EVENT *)cRecvBuf;
			pCfgDiskNum->iHddNumber = pDiskAlmCfg->bDiskNum;

			nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_DISKNUM, 0, cRecvBuf, sizeof(CONFIG_STORAGE_NUMBER_EVENT), waittime);
			if(nRet < 0)
			{
				nRet = -1;
				goto END;
			}
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}
	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgNetBroken(LONG lLoginID, NETBROKEN_ALARM_CFG *pNetBrkCfg, int waittime)
{
	if (0 == lLoginID || NULL == pNetBrkCfg)
	{
		return -1;
	}
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, j = 0;
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_GENERIC_EVENT))
	{
		iRecvBufLen = sizeof(CONFIG_GENERIC_EVENT);
	}
	cRecvBuf = new char [iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		nRet = -1;
		goto END;
	}
	memset(cRecvBuf, 0, iRecvBufLen);
//	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_NETBROKEN, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
//	if (nRet >= 0 && retlen == sizeof(CONFIG_GENERIC_EVENT))
	{
		CONFIG_GENERIC_EVENT *pCfgNetBrk = (CONFIG_GENERIC_EVENT *)cRecvBuf;

		SetAlmActionFlag(&pCfgNetBrk->hEvent, pNetBrkCfg->struHandle.dwActionFlag);
		pCfgNetBrk->bEnable = pNetBrkCfg->byEnable;
		
		pCfgNetBrk->hEvent.dwRecord = 0;
		pCfgNetBrk->hEvent.dwTour = 0;
		pCfgNetBrk->hEvent.dwSnapShot = 0;
		
		int index = min(32, MAX_VIDEO_IN_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgNetBrk->hEvent.dwSnapShot |= (pNetBrkCfg->struHandle.bySnap[j]) ? (0x01<<j) : 0;
			pCfgNetBrk->hEvent.dwTour |= (pNetBrkCfg->struHandle.byTour[j]) ? (0x01<<j) : 0;
			pCfgNetBrk->hEvent.dwRecord |= (pNetBrkCfg->struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			pCfgNetBrk->hEvent.PtzLink[j].iValue = pNetBrkCfg->struHandle.struPtzLink[j].iValue;
			pCfgNetBrk->hEvent.PtzLink[j].iType = pNetBrkCfg->struHandle.struPtzLink[j].iType;
		}

		pCfgNetBrk->hEvent.dwAlarmOut = 0;
//		pCfgNetBrk->hEvent.wiAlarmOut = 0;
		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			pCfgNetBrk->hEvent.dwAlarmOut |= (pNetBrkCfg->struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
//			pCfgNetBrk->hEvent.wiAlarmOut |= (pNetBrkCfg->struHandle.byRelWIAlarmOut[j]) ? (0x01<<j) : 0;
		}
		pCfgNetBrk->hEvent.iAOLatch = pNetBrkCfg->struHandle.dwDuration;
		pCfgNetBrk->hEvent.iRecordLatch = pNetBrkCfg->struHandle.dwRecLatch;
//		pCfgNetBrk->hEvent.iEventLatch = pNetBrkCfg->struHandle.dwEventLatch;
		pCfgNetBrk->hEvent.bMessagetoNet = (BOOL)pNetBrkCfg->struHandle.bMessageToNet;
		pCfgNetBrk->hEvent.bMMSEn = pNetBrkCfg->struHandle.bMMSEn;
		pCfgNetBrk->hEvent.SnapshotTimes = pNetBrkCfg->struHandle.bySnapshotTimes;//短信发送图片的张数
		pCfgNetBrk->hEvent.bLog = (BOOL)pNetBrkCfg->struHandle.bLog;
		pCfgNetBrk->hEvent.bMatrixEn = (BOOL)pNetBrkCfg->struHandle.bMatrixEn;
		pCfgNetBrk->hEvent.dwMatrix = pNetBrkCfg->struHandle.dwMatrix;
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_NETBROKEN, 0, cRecvBuf, sizeof(CONFIG_GENERIC_EVENT), waittime);
		if (nRet < 0)
		{
			nRet = -1;
			goto END;
		}
	}
//	else
//	{
//		nRet = -1;
//		goto END;
//	}

	nRet = 0;
END:
	delete []cRecvBuf;
	return nRet;
}

int	CDevConfig::SetDevConfig_AlmCfgNew(LONG lLoginID, DEV_ALARM_SCHEDULE &stAs, int waittime)
{
	afk_device_s *device = (afk_device_s *)lLoginID;
	if (NULL == device)
	{
		return -1;
	}
	
	int nRet = -1;
	int nAlarmInNum = device->alarminputcount(device);

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);

	if (nAlarmInNum != 0 && (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF]))
	{
		nRet = SetDevConfig_AlmCfgLocalAlarm(lLoginID, stAs.struLocalAlmIn, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
		Sleep(SETUP_SLEEP);
		nRet = SetDevConfig_AlmCfgNetAlarm(lLoginID, stAs.struNetAlmIn, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
		Sleep(SETUP_SLEEP);
	}
	
	//	此功能解码器除外
	if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
	{
		if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_VIDEOCONF])
		{
			
			nRet = SetDevConfig_AlmCfgMotion(lLoginID, stAs.struMotion, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			Sleep(SETUP_SLEEP);
			
			nRet = SetDevConfig_AlmCfgLoss(lLoginID, stAs.struVideoLost, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			Sleep(SETUP_SLEEP);
			
			nRet = SetDevConfig_AlmCfgBlind(lLoginID, stAs.struBlind, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			Sleep(SETUP_SLEEP);
		}
		
		nRet = SetDevConfig_AlmCfgDisk(lLoginID, &stAs.struDiskAlarm, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
		Sleep(SETUP_SLEEP);
		nRet = SetDevConfig_AlmCfgNetBroken(lLoginID, &stAs.struNetBrokenAlarm, waittime);
		if (nRet < 0)
		{
			return nRet;
		}
		Sleep(SETUP_SLEEP);
	}
	
	return 0;
}

int CDevConfig::GetDevConfig_AlmCfg(LONG lLoginID, DEV_ALARM_SCHEDULE &stAS, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	DVR_AUTHORITY_INFO nAuthorityInfo = {0};
	device->get_info(device, dit_config_authority_mode, &nAuthorityInfo);
	
	memset(&stAS, 0, sizeof(DEV_ALARM_SCHEDULE));
	stAS.dwSize = sizeof(DEV_ALARM_SCHEDULE);
	int retlen = 0;
	int nRet = -1;
	
	int i = 0, x = 0; //indexs for "for" operator
	//get alarm input config
	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_ALARMCONF])
	{
		char aBuf[16*sizeof(CONFIG_ALARM)] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM, 0, aBuf, 16*sizeof(CONFIG_ALARM), &retlen, waittime);
		if (nRet >= 0 && retlen == 16*sizeof(CONFIG_ALARM))
		{
			CONFIG_ALARM *stAlarm = 0;
			for (i = 0; i < 16; i++)
			{
				stAS.struLocalAlmIn[i].struHandle.dwActionMask = 0;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_TIP;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_TOUR;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_MAIL;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_UPLOAD;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_OUT;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_RECORD;
				stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_FTP_UL;
				//	stAS.struLocalAlmIn[i].struHandle.dwActionMask |= ALARM_PTZ;
				stAlarm = (CONFIG_ALARM *)(aBuf + i*sizeof(CONFIG_ALARM));
				stAS.struLocalAlmIn[i].byAlarmType = stAlarm->SensorType;
				stAS.struLocalAlmIn[i].byAlarmEn = 1; //default enabled
				stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= ALARM_RECORD; //default record enable
				for (x = 0; x < 16; x++)
				{
					stAS.struLocalAlmIn[i].struHandle.byRelAlarmOut[x] = (stAlarm->AlarmMask & (0x01<<x)) ? 1 : 0;
					stAS.struLocalAlmIn[i].struHandle.byRecordChannel[x] = (stAlarm->RecordMask & (0x01<<x)) ? 1 : 0;
				}
				if (stAlarm->AlarmTipEn)
				{
					stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= ALARM_TIP;
				}
				if (stAlarm->dbTourEn)
				{
					stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= ALARM_TOUR;
				}
				if (stAlarm->Mailer)
				{
					stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= ALARM_MAIL;
				}
				if (stAlarm->SendAlarmMsg)
				{
					stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= ALARM_UPLOAD;
				}
				for (x = 0; x < ALARM_TIME_SECT_NUM; x++)
				{
					for (int j = 0; j < DAYS_PER_WEEK; j++)
					{
						//memcpy(&stAS.struLocalAlmIn[i].stSect[j][x], &stAlarm->sAlarmSects[x], sizeof(TSECT));
					}
					//这里的wEvent是一个“提示”和“报警输出”的使能掩码。
					//	stAS.struLocalAlmIn[i].struHandle.stAlarmOutSect[x].wEvent = ((WORD)stAlarm->dwSectMask[x]) & 0x000F;
				}
				//	stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= (stAlarm->dwSectMask[0] & 0x01)?ALARM_TIP:0;
				//	stAS.struLocalAlmIn[i].struHandle.dwActionFlag |= (stAlarm->dwSectMask[0]>>1) & 0x01?ALARM_OUT:0;
				
				stAS.struLocalAlmIn[i].struHandle.dwRecLatch = stAlarm->TimePreRec;
				stAS.struLocalAlmIn[i].struHandle.dwDuration = stAlarm->TimeDelay;
			}
		}
		else
		{
			return NET_ERROR_GETCFG_ALARMIN;
		}
	}

	ALARM_CONTROL tmpAlarmCfg[CONFIG_ALARM_NUM] = {0};
	int iAlarmIn = 0;
	nRet = QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, NULL, 0, &iAlarmIn, waittime);
	if (nRet >= 0)
	{
		if (iAlarmIn <= CONFIG_ALARM_NUM)
		{
			nRet = QueryIOControlState(lLoginID, IOTYPE_ALARMINPUT, tmpAlarmCfg, CONFIG_ALARM_NUM*sizeof(ALARM_CONTROL), &iAlarmIn, waittime);
			iAlarmIn = Min(iAlarmIn, CONFIG_ALARM_NUM);
			if (nRet >= 0)
			{
				for (i = 0; i < iAlarmIn; i++)
				{
					stAS.struLocalAlmIn[i].byAlarmEn = tmpAlarmCfg[i].state;
				}
			}
		}
	}
	else
	{
		//error
	}
	//get detect config: motion,video-lost,blind.

	if (1 == nAuthorityInfo.bAuthorityInfo[DVRG_AUTHORITY_VIDEOCONF])
	{
		char dtBuf[16*sizeof(CONFIG_DETECT)] = {0};
		nRet = QueryConfig(lLoginID, CONFIG_TYPE_DETECT, 0, dtBuf, 16*sizeof(CONFIG_DETECT), &retlen, waittime);
		if (nRet >=0 && retlen == 16*sizeof(CONFIG_DETECT))
		{
			CONFIG_DETECT *stDet = 0;
			for (i = 0; i < MAX_VIDEO_IN_NUM; i++)
			{
				stAS.struMotion[i].struHandle.dwActionMask = 0;
				stAS.struMotion[i].struHandle.dwActionMask |= ALARM_OUT;
				stAS.struMotion[i].struHandle.dwActionMask |= ALARM_UPLOAD;
				//stAS.struMotion[i].struHandle.dwActionMask |= ALARM_TIP;
				stAS.struMotion[i].struHandle.dwActionMask |= ALARM_MAIL;
				stAS.struMotion[i].struHandle.dwActionMask |= ALARM_RECORD;
				
				stAS.struBlind[i].struHandle.dwActionMask = 0;
				stAS.struBlind[i].struHandle.dwActionMask |= ALARM_OUT;
				stAS.struBlind[i].struHandle.dwActionMask |= ALARM_TIP;
				stAS.struBlind[i].struHandle.dwActionMask |= ALARM_MAIL;
				
				stAS.struVideoLost[i].struHandle.dwActionMask = 0;
				stAS.struVideoLost[i].struHandle.dwActionMask |= ALARM_OUT;
				stAS.struVideoLost[i].struHandle.dwActionMask |= ALARM_TIP;
				stAS.struVideoLost[i].struHandle.dwActionMask |= ALARM_MAIL;
				
				stDet = (CONFIG_DETECT*)(dtBuf+i*sizeof(CONFIG_DETECT));
				stAS.struMotion[i].byMotionEn = 1; //default enabled
				stAS.struVideoLost[i].byAlarmEn = 1; //default enabled
				stAS.struBlind[i].byBlindEnable = 1; //default enabled
				stAS.struMotion[i].wSenseLevel = stDet->MotionLever;
				
				//Begin: Add by zsc(11402) 2008-1-18
				//	获取动态检测区域的行数和列数
				int nMaxRow = 12;
				int nMaxCol = 16;
				int nSysInfoLen = 0;
				MOTION_DETECT_CAPS SysInfoMotion = {0};
				nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_MOTIONDETECT, (char*)&SysInfoMotion, sizeof(MOTION_DETECT_CAPS), &nSysInfoLen, waittime);
				if (0 == nRet && nSysInfoLen == sizeof(MOTION_DETECT_CAPS))
				{
					if (SysInfoMotion.Enabled == 1)
					{
						nMaxRow = SysInfoMotion.GridRows>18 ? 18 : SysInfoMotion.GridRows;
						nMaxCol = SysInfoMotion.GridLines;
					}
				}
				
				stAS.struMotion[i].wMotionRow = nMaxRow;
				stAS.struMotion[i].wMotionCol = nMaxCol;
				//row*col motion region
				for (x = 0; x < nMaxRow; x++)
				{
					for (int y = 0; y < nMaxCol; y++)
					{
						stAS.struMotion[i].byDetected[x][y] = (stDet->MotionRegion[x] & (0x01<<y)) ? 1 : 0;
					}
				}
				//End: zsc(11402)
				
				for (x = 0; x < MAX_ALARM_OUT_NUM; x++)
				{
					stAS.struMotion[i].struHandle.byRelAlarmOut[x] = (stDet->MotionAlarmMask & (0x01<<x)) ? 1 : 0;
					stAS.struBlind[i].struHandle.byRelAlarmOut[x] = (stDet->BlindAlarmMask & (0x01<<x)) ? 1 : 0;
					stAS.struVideoLost[i].struHandle.byRelAlarmOut[x] = (stDet->LossAlarmMask & (0x01<<x)) ? 1 : 0;
				}
				for (x = 0; x < MAX_VIDEO_IN_NUM; x++)
				{
					stAS.struMotion[i].struHandle.byRecordChannel[x] = (stDet->MotionRecordMask & (0x01<<x)) ? 1 : 0;
				}
				if (stDet->BlindTipEn) 
				{
					stAS.struBlind[i].struHandle.dwActionFlag |= ALARM_TIP;
				}
				if (stDet->LossTipEn) 
				{
					stAS.struVideoLost[i].struHandle.dwActionFlag |= ALARM_TIP;
				}
				if (stDet->SendAlarmMsg)
				{
					stAS.struMotion[i].struHandle.dwActionFlag |= ALARM_UPLOAD;
				}
				//default output eanbled
				stAS.struMotion[i].struHandle.dwActionFlag |= ALARM_OUT;
				stAS.struBlind[i].struHandle.dwActionFlag |= ALARM_OUT;
				stAS.struVideoLost[i].struHandle.dwActionFlag |= ALARM_OUT;
				
				stAS.struMotion[i].struHandle.dwDuration = stDet->MotionTimeDelay;
				stAS.struMotion[i].struHandle.dwActionFlag |= (stDet->Mailer) ? ALARM_MAIL : 0;
			}
		}
		else
		{
			return NET_ERROR_GETCFG_ALARMDET;
		}
	}

	/*
	//get sound alarm config
	char adBuf[16*sizeof(CONFIG_AUDIO_DETECT)] = {0};
	nRet = QueryConfig(lLoginID, 15, 0, adBuf, 16*sizeof(CONFIG_AUDIO_DETECT), &retlen, waittime);
	if (nRet >=0 && retlen == 16*sizeof(CONFIG_AUDIO_DETECT))
	{
		CONFIG_AUDIO_DETECT *stAudio = 0;
		for (i = 0; i < MAX_VIDEO_IN_NUM; i++)
		{
			stAS.struSoundAlarm[i].struHandle.dwActionMask |= ALARM_TIP;
			stAS.struSoundAlarm[i].struHandle.dwActionMask |= ALARM_TOUR;
			stAS.struSoundAlarm[i].struHandle.dwActionMask |= ALARM_MAIL;
			stAS.struSoundAlarm[i].struHandle.dwActionMask |= ALARM_UPLOAD;
			stAS.struSoundAlarm[i].struHandle.dwActionMask |= ALARM_OUT;
			stAS.struSoundAlarm[i].struHandle.dwActionMask |= ALARM_RECORD;
			
			stAudio = (CONFIG_AUDIO_DETECT*)(adBuf+i*sizeof(CONFIG_AUDIO_DETECT));
			stAS.struSoundAlarm[i].bySoundAlarmEn = stAudio->iEnable;
			stAS.struSoundAlarm[i].byVol = stAudio->iValve;
			
			for (x = 0; x < 16; x++)
			{
				stAS.struSoundAlarm[i].struHandle.byRelAlarmOut[x] = (stAudio->AlarmMask & (0x01<<x)) ? 1 : 0;
				stAS.struSoundAlarm[i].struHandle.byRecordChannel[x] = (stAudio->RecordMask & (0x01<<x)) ? 1 : 0;
			}
			//this should be added.
			if (stAudio->AlarmTipEn) 
			{
				stAS.struSoundAlarm[i].struHandle.dwActionFlag |= ALARM_TIP;
			}
			if (stAudio->dbTourEn)
			{
				stAS.struSoundAlarm[i].struHandle.dwActionFlag |= ALARM_TOUR;
			}
			if (stAudio->Mailer)
			{
				stAS.struSoundAlarm[i].struHandle.dwActionFlag |= ALARM_MAIL;
			}
			if (stAudio->SendAlarmMsg)
			{
				stAS.struSoundAlarm[i].struHandle.dwActionFlag |= ALARM_UPLOAD;
			}
			
			stAS.struSoundAlarm[i].struHandle.dwRecLatch = stAudio->TimePreRec;
			stAS.struSoundAlarm[i].struHandle.dwDuration = stAudio->TimeDelay;
		}
	}
	*/

	return 0;
}

#define WORKSHEET_NUM_EX 32
int CDevConfig::GetDevConfig_WorkSheet(LONG lLoginID, CONFIG_WSHEET_TYPE sheetType, CONFIG_WORKSHEET *pWSheetCfg, int waittime, int nSheetNum, int nChannel)
{
	if (0 == lLoginID || NULL == pWSheetCfg)
	{
		return -1;
	}

	if( nSheetNum == 0)//防止时间表为空的时候错误
	{
		return 0;
	}

	int retlen = 0;
	int nRet = -1;

	int i = 0;

	switch(sheetType) 
	{
	case WSHEET_RECORD:
	case WSHEET_LOCAL_ALARM:
	case WSHEET_NET_ALARM:
	case WSHEET_MOTION:
	case WSHEET_BLIND:
	case WSHEET_LOSS: //16个时间表
	case WSHEET_WIRELESS_ALARM://无线报警时间表
	case WSHEET_PANORAMA_SWITCH:
	case WSHEET_LOST_FOCUS:
		{
		//	int nSheetCount = nSheetNum > WORKSHEET_NUM ? WORKSHEET_NUM:nSheetNum;
			int nSheetCount = nSheetNum > WORKSHEET_NUM_EX ? WORKSHEET_NUM_EX:nSheetNum;
			for (i=0; i < nSheetCount; i++)
			{
				nRet = QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16)|(i+1), (char*)&pWSheetCfg[i], sizeof(CONFIG_WORKSHEET), &retlen, waittime);
				if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
				{
					nRet = -1;
					goto END;
				}
			}
		}
		break;
	case WSHEET_NO_DISK:
	case WSHEET_DISK_ERROR:
	case WSHEET_DISK_FULL:
	case WSHEET_NET_BROKEN:
		{
			nRet = QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16)|i, (char*)pWSheetCfg, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
		}
		break;
	case WSHEET_DECODER_ALARM: //1个时间表
		{
			nRet = QueryConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16) |(nChannel+1), (char*)pWSheetCfg, sizeof(CONFIG_WORKSHEET), &retlen, waittime);
			if (nRet < 0 || retlen != sizeof(CONFIG_WORKSHEET))
			{
				nRet = -1;
				goto END;
			}
		}
		break;
	default:
		nRet = -1;
		goto END;
	}
END:
	return nRet;
}

int	CDevConfig::SetDevConfig_WorkSheet(LONG lLoginID, CONFIG_WSHEET_TYPE sheetType, CONFIG_WORKSHEET *pWSheetCfg, int waittime, int nSheetNum, int nChannel)
{
	if (0 == lLoginID || NULL == pWSheetCfg)
	{
		return -1;
	}

	int nRet = -1;
	
	int i = 0;
	
	switch(sheetType) 
	{
	case WSHEET_RECORD:
	case WSHEET_LOCAL_ALARM:
	case WSHEET_NET_ALARM:
	case WSHEET_MOTION:
	case WSHEET_BLIND:
	case WSHEET_LOSS: //16个时间表
		{
		//	int nSheetCount = nSheetNum > WORKSHEET_NUM ? WORKSHEET_NUM:nSheetNum;
		//  超过16个通道也有可能
			int nSheetCount = nSheetNum;
			for (i=0; i<nSheetCount; i++)
			{
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16)|(i+1), (char*)&pWSheetCfg[i], sizeof(CONFIG_WORKSHEET), waittime);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
				Sleep(20);
			}
		}
		break;
	case WSHEET_NO_DISK:
	case WSHEET_DISK_ERROR:
	case WSHEET_DISK_FULL:
	case WSHEET_NET_BROKEN: //1个时间表
		{
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16)|i, (char*)pWSheetCfg, sizeof(CONFIG_WORKSHEET), waittime);
			if (nRet < 0)
			{
				nRet = -1;
				goto END;
			}
			Sleep(20);
		}
		break;
	case WSHEET_WIRELESS_ALARM:
	case WSHEET_PANORAMA_SWITCH:
	case WSHEET_LOST_FOCUS:
		{
			int nSheetCount = nSheetNum > WORKSHEET_NUM ? WORKSHEET_NUM:nSheetNum;
			for (i=0; i<nSheetCount; i++)
			{
				nRet = SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16)|(i+1), (char*)&pWSheetCfg[i], sizeof(CONFIG_WORKSHEET), waittime);
				if (nRet < 0)
				{
					nRet = -1;
					goto END;
				}
				Sleep(20);
			}
			
		}
		break;
	case WSHEET_DECODER_ALARM: 
		{
			nRet = SetupConfig(lLoginID, CONFIG_TYPE_TIMESHEET, (sheetType<<16) |(nChannel+1), (char*)pWSheetCfg, sizeof(CONFIG_WORKSHEET), waittime);
			if (nRet < 0)
			{
				nRet = -1;
				goto END;
			}
			Sleep(20);
		}
		break;
	default:
		nRet = -1;
		goto END;
	}
END:
	return nRet;
}

int	CDevConfig::GetDevConfig_Devtime(LONG lLoginID, NET_TIME &stNT, int waittime)
{
	memset(&stNT, 0, sizeof(NET_TIME));

	int nRet = -1;
	nRet = QueryDeviceTime(lLoginID, &stNT, waittime);
	if (nRet >= 0)
	{
		
	}
	else
	{
		nRet = NET_ERROR_GETCFG_SYSTIME;
	}

	return nRet;
}

int	CDevConfig::GetDevConfig_PreviewCfg(LONG lLoginID, DEV_PREVIEW_CFG &stPVC, int waittime)
{
	memset(&stPVC, 0, sizeof(DEV_PREVIEW_CFG));
	stPVC.dwSize = sizeof(DEV_PREVIEW_CFG);
	
	int nRet = -1;
	int retlen = 0;

	//Get preview config
	CONFIG_EX_CAPTURE_T stPrv = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_PREVIEW, 0, (char*)&stPrv, sizeof(CONFIG_EX_CAPTURE_T), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_EX_CAPTURE_T))
	{
		stPVC.stPreView.byImageQlty = stPrv.iQuality;
		//others are not supported yet
	}
	else
	{
		nRet = NET_ERROR_GETCFG_PREVIEW;
	}

	return nRet;
}
int	CDevConfig::GetDevConfig_AutoMtCfg(LONG lLoginID, DEV_AUTOMT_CFG &stAMT, int waittime)
{
	memset(&stAMT, 0, sizeof(DEV_AUTOMT_CFG));
	stAMT.dwSize = sizeof(DEV_AUTOMT_CFG);

	int nRet = -1;
	int retlen = 0;
	
	//Get auto maintenance config
	CONFIG_AUTO_NET stAutoNet = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_AUTO_MT, 0, (char*)&stAutoNet, sizeof(CONFIG_AUTO_NET), &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_AUTO_NET))
	{
		stAMT.byAutoRebootDay = stAutoNet.AutoRebootDay;
		stAMT.byAutoRebootTime = stAutoNet.AutoRebootTime;
		stAMT.byAutoDeleteFilesTime = stAutoNet.AutoDeleteFilesTime;
	}
	else
	{
		nRet = NET_ERROR_GETCFG_AUTOMT;
	}

	return nRet;
}

int CDevConfig::GetDevConfig_VdMtrxCfg(LONG lLoginID, DEV_VIDEO_MATRIX_CFG *pstMatrix, int waittime)
{
	if (pstMatrix == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(pstMatrix, 0, sizeof(DEV_VIDEO_MATRIX_CFG));
	pstMatrix->dwSize = sizeof(DEV_VIDEO_MATRIX_CFG);

	int nRet = -1;
	int retlen = 0;
		
	//Get video matrix config
	char buf[512] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_VIDEO_MATRIX, 1, buf, 512, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		CStrParse parser;
		parser.setSpliter("&&");
		bool bSuccess = parser.Parse(buf);
		if (!bSuccess) 
		{
			return NET_ERROR;
		}
		
		if (parser.Size() < 0 || parser.Size() > MAX_MATRIX_OUT)
		{
			return NET_ERROR;
		}
		
		pstMatrix->nMatrixNum = parser.Size();
		for (int i = 0; i < parser.Size(); i++)
		{
			std::string matrixInfo = parser.getWord(i);

			CStrParse subparser;
			subparser.setSpliter("::");
			bSuccess = subparser.Parse(matrixInfo);
			if (!bSuccess)
			{
				return NET_ERROR;
			}

			pstMatrix->struVideoGroup[i].dwChannelMask = atoi(subparser.getWord(1).c_str());
			pstMatrix->struVideoGroup[i].bTourEnable = atoi(subparser.getWord(2).c_str());
			pstMatrix->struVideoGroup[i].nTourPeriod = atoi(subparser.getWord(3).c_str());
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_VIDEOMTRX;
	}

	return nRet;
}

int	CDevConfig::GetDevConfig_MultiDdnsCfg(LONG lLoginID, DEV_MULTI_DDNS_CFG &stMD, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;

	memset(&stMD, 0, sizeof(DEV_MULTI_DDNS_CFG));
	stMD.dwSize = sizeof(DEV_MULTI_DDNS_CFG);

	int nRet = -1;
	int retlen = 0;

	int buflen = 400*10;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_MULTI_DDNS, 0, tmpbuf, buflen, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		//Begin: Modify by li_deming(11517) 2008-2-20
		//nRet = ParseMultiDdnsString(tmpbuf, retlen, &stMD);
		DWORD dwSign = 0;
		nRet = ParseMultiDdnsString(tmpbuf, retlen, &stMD,&dwSign);
		if (dwSign != 0)
		{
			//带保活时间的新协议
			DWORD dwSp = SP_NEW_MULTIDDNSPRO;
			device->set_info(device, dit_special_cap_flag, &dwSp);
		}
		//End:li_deming(11517)
	}

	return nRet;
}

int	CDevConfig::GetDevConfig_SnapCfg(LONG lLoginID, DEVICE_SNAP_CFG *pstSC, LONG lChnNum, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	int channum = device->channelcount(device);
	//hdvr
	if(device->device_type(device) == PRODUCT_DVR_MIX_DVR)
	{
		channum -= 4;//4个数字通道
	}

	if (!pstSC || lChnNum < channum)
	{
		return -1;
	}

	int i = 0;
	
	memset(pstSC, 0, lChnNum*sizeof(DEVICE_SNAP_CFG));
	for (i = 0; i < lChnNum; i++)
	{
		pstSC[i].dwSize = sizeof(DEVICE_SNAP_CFG);
	}
	int retlen = 0;
	int nRet = -1;

	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;

	int RecordNewNum = device->channelcount(device);
	if(RecordNewNum < 16)
	{
		RecordNewNum = 16;
	}
	
	if (iRecvBufLen < 16 * sizeof(NEW_CONFIG_CAPTURE))//127#子命令
	{
		iRecvBufLen = 16 * sizeof(NEW_CONFIG_CAPTURE);
	}
	if (iRecvBufLen < RecordNewNum * sizeof(CONFIG_RECORD_NEW))//录象配置
	{
		iRecvBufLen = RecordNewNum * sizeof(CONFIG_RECORD_NEW);
	}

	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//get record config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_RECORD_NEW, 0, cRecvBuf, RecordNewNum*sizeof(CONFIG_RECORD_NEW), &retlen, waittime);
	if (nRet >= 0 && retlen == RecordNewNum*sizeof(CONFIG_RECORD_NEW))
	{
		CONFIG_RECORD_NEW *stRecordNew = 0;
		for (i = 0; i < lChnNum; i++)
		{
			stRecordNew = (CONFIG_RECORD_NEW*)(cRecvBuf+i*sizeof(CONFIG_RECORD_NEW));
			pstSC[i].bTimingEnable = stRecordNew->bSnapShot;
		}
	}
	else //if (nRet >= 0 && retlen == 16*sizeof(CONFIG_RECORD))
	{
		nRet =  NET_ERROR_GETCFG_RECORD;
		goto END;
	}

	//new capture config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_CAPTURE_127, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && (retlen == 16 * sizeof(NEW_CONFIG_CAPTURE) || retlen == device->channelcount(device) * sizeof(NEW_CONFIG_CAPTURE)))
	{
		NEW_CONFIG_CAPTURE *stNewPic = 0;
		int i = 0;
		
		for (i = 0; i < lChnNum; i++)
		{
			stNewPic = (NEW_CONFIG_CAPTURE *)(cRecvBuf + i*sizeof(NEW_CONFIG_CAPTURE));
			
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byVideoEnable = stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable & 0x01;
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byAudioEnable = (stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable>>1) & 0x01;	
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].bAudioOverlay = (stNewPic->SnapOption[SNAP_TYP_TIM_INTER].AVEnable>>2) & 0x01;	
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byImageSize = stNewPic->SnapOption[SNAP_TYP_TIM_INTER].ImageSize;
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byBitRateControl = stNewPic->SnapOption[SNAP_TYP_TIM_INTER].BitRateControl;
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byFramesPerSec = stNewPic->SnapOption[SNAP_TYP_TIM_INTER].Frames;
			pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byImageQlty = stNewPic->SnapOption[SNAP_TYP_TIM_INTER].ImgQlty;
		//	pstSC[i].struSnapEnc[SNAP_TYP_TIMING].byEncodeMode = stNewPic->Compression;
			
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byVideoEnable = stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable & 0x01;
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byAudioEnable = (stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable>>1) & 0x01;	
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].bAudioOverlay = (stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].AVEnable>>2) & 0x01;	
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byImageSize = stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].ImageSize;
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byBitRateControl = stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].BitRateControl;
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byFramesPerSec = stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].Frames;
			pstSC[i].struSnapEnc[SNAP_TYP_ALARM].byImageQlty = stNewPic->SnapOption[SNAP_TYP_ALARM_INTER].ImgQlty;
			//增加抓图时间间隔2009.2.9 
  			pstSC[i].PicTimeInterval = stNewPic->PicTimeInterval;//单位为s
		}//for (int i = 0; i < chanNum; i++)
	}	
	else
	{
		nRet = NET_ERROR_GETCFG_VIDEO; 
	}

END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int	CDevConfig::GetDevConfig_UrlCfg(LONG lLoginID, DEVWEB_URL_CFG &stUC, int waittime)
{
	memset(&stUC, 0, sizeof(DEVWEB_URL_CFG));
	stUC.dwSize = sizeof(DEVWEB_URL_CFG);

	int retlen = 0;
	int nRet = -1;
	
	int buflen = 500;
	char *tmpbuf = new char[buflen];
	if (NULL == tmpbuf) 
	{
		return -1;
	}
	memset(tmpbuf, 0, buflen);
	
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_WEB_URL, 0, tmpbuf, buflen, &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		nRet = ParseUrlCfgString(tmpbuf, retlen, &stUC);
	}
	else
	{
		nRet = -1;
	}
	
	return nRet;
}


int	CDevConfig::GetDevConfig_FtpCfg(LONG lLoginID, DEVICE_FTP_PROTO_CFG &stFC, int waittime)
{
	memset(&stFC, 0, sizeof(DEVICE_FTP_PROTO_CFG));
	stFC.dwSize = sizeof(DEVICE_FTP_PROTO_CFG);
	
	int retlen = 0;
	int nRet = -1;

	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_FTP_PROTO_SET))
	{
		iRecvBufLen = sizeof(CONFIG_FTP_PROTO_SET);
	}
	
	DEV_ENABLE_INFO stDevEn = {0};
	bool bSupport = false;
	
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &retlen, waittime);
	if (nRet >= 0 && retlen > 0)
	{
		if (stDevEn.IsFucEnable[EN_STORAGE_STATION] != 0)
		{
			stFC.dwFunctionMask = stDevEn.IsFucEnable[EN_STORAGE_STATION];
			bSupport = true;
		}		
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//get ftp config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_FTP_PRO, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_FTP_PROTO_SET))
	{
		if (false == bSupport)
		{
			stFC.dwFunctionMask |=0x01; //能力集读不到，但是配置又能读到，默认为支持FTP，兼容老的设备
		}
		
		CONFIG_FTP_PROTO_SET *pftpCfg = (CONFIG_FTP_PROTO_SET*)cRecvBuf;
		stFC.bEnable = pftpCfg->m_isEnable;
		stFC.iFileLen = pftpCfg->m_iFileLen;
		stFC.iInterval = pftpCfg->m_iInterval;

		unsigned char szUserName[MAX_USERNAME_LEN+1];
		memset(szUserName, 0, MAX_USERNAME_LEN+1);
		memcpy(szUserName, pftpCfg->m_cUserName, MAX_USERNAME_LEN);
		Change_Utf8_Assic(szUserName, stFC.szUserName);
		stFC.szUserName[MAX_FTP_USERNAME_LEN-1] = '\0';

		memcpy(stFC.szPassword, pftpCfg->m_cPassword, MAX_PASSWORD_LEN-1);

		unsigned char szDirName[FTP_MAX_PATH+4];
		memset(szDirName, 0, FTP_MAX_PATH+4);
		memcpy(szDirName, pftpCfg->m_cDirName, FTP_MAX_PATH);
		Change_Utf8_Assic(szDirName, stFC.szDirName);
		Cut_UTF8_Tail((unsigned char *)stFC.szDirName, FTP_MAX_PATH-1);
		stFC.szDirName[FTP_MAX_PATH-1] = '\0';
		
		stFC.wHostPort = pftpCfg->m_nHostPort;
		in_addr tmp = {0};
		tmp.s_addr = pftpCfg->m_unHostIP;
		strcpy(stFC.szHostIp, inet_ntoa(tmp));

		stFC.protocol = pftpCfg->m_protocol;
		stFC.NASVer = pftpCfg->m_NASVer;

		for (int i=0; i<MAX_CHANNEL_NUM; i++)
		{
			for (int j=0; j<DAYS_WEEK; j++)
			{
				for (int k=0; k<TIME_SECTION; k++)
				{
					stFC.struUploadCfg[i][j].struPeriod[k].struSect.iBeginHour = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartHour;
					stFC.struUploadCfg[i][j].struPeriod[k].struSect.iBeginMin = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartMin;
					stFC.struUploadCfg[i][j].struPeriod[k].struSect.iBeginSec = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.StartSec;
					stFC.struUploadCfg[i][j].struPeriod[k].struSect.iHourEnd = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.HourEnd;
					stFC.struUploadCfg[i][j].struPeriod[k].struSect.iEndMin = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.EndMin;
					stFC.struUploadCfg[i][j].struPeriod[k].struSect.iEndSec = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_tSect.EndSec;
					stFC.struUploadCfg[i][j].struPeriod[k].bAlarmEn = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_AlarmEn;
					stFC.struUploadCfg[i][j].struPeriod[k].bMdEn = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_MdEn;
					stFC.struUploadCfg[i][j].struPeriod[k].bTimerEn = pftpCfg->m_stMdAlarmSet[i][j].m_Period[k].m_TimerEn;
				}
			}
		}
	}
	else
	{
		nRet = -1;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int	CDevConfig::GetDevConfig_InterVedioCfg(LONG lLoginID, char* pCfg, int cfgLen, LONG lPlatform, int waittime)
{
	DEV_INTERVIDEO_UCOM_CFG* pstIUC = NULL;
	DEV_INTERVIDEO_NSS_CFG* pstNSS = NULL;
	DEV_INTERVIDEO_BELL_CFG* pstBLC = NULL;
	switch(lPlatform)
	{
	case 4:		//BELL
		pstBLC = (DEV_INTERVIDEO_BELL_CFG*)pCfg;
		memset(pstBLC, 0, sizeof(DEV_INTERVIDEO_BELL_CFG));
		pstBLC->dwSize = sizeof(DEV_INTERVIDEO_BELL_CFG);
		break;
	case 10:	//中兴力维
		pstNSS = (DEV_INTERVIDEO_NSS_CFG*)pCfg;
		memset(pstNSS, 0, sizeof(DEV_INTERVIDEO_NSS_CFG));
		pstNSS->dwSize = sizeof(DEV_INTERVIDEO_NSS_CFG);
		break;
	case 11:	//U com
		pstIUC = (DEV_INTERVIDEO_UCOM_CFG*)pCfg;
		memset(pstIUC, 0, sizeof(DEV_INTERVIDEO_UCOM_CFG));
		pstIUC->dwSize = sizeof(DEV_INTERVIDEO_UCOM_CFG);
		break;
	default:
		return NET_ILLEGAL_PARAM;
		break;
	}
	
	int retlen = 0;
	int nRet = -1;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < sizeof(CONFIG_INTERVIDEO_UCOM))
	{
		iRecvBufLen = sizeof(CONFIG_INTERVIDEO_UCOM);
	}
	if (iRecvBufLen < sizeof(DEV_INTERVIDEO_NSS_CFG))
	{
		iRecvBufLen = sizeof(DEV_INTERVIDEO_NSS_CFG);
	}
	if (iRecvBufLen < sizeof(WEB_VSP_CONFIG))
	{
		iRecvBufLen = sizeof(WEB_VSP_CONFIG);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//get intervedio config
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_INTERVIDEO, lPlatform, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0)
	{
		switch(lPlatform)
		{
		case 4:		//BELL
			if (retlen == sizeof(WEB_VSP_CONFIG))
			{
				WEB_VSP_CONFIG* pANVCfg = (WEB_VSP_CONFIG*)cRecvBuf;
				//这里偷一下懒，直接拷贝
				memcpy(pCfg+sizeof(DWORD), pANVCfg, sizeof(WEB_VSP_CONFIG));
			}
			else
			{
				nRet = -1;
			}
			break;
		case 10:		//中兴力维
			if (retlen == sizeof(WEB_NSS_CONFIG))
			{
				WEB_NSS_CONFIG* pNSSCfg = (WEB_NSS_CONFIG*)cRecvBuf;
				//这里偷一下懒，直接拷贝
				memcpy(pCfg+sizeof(DWORD), pNSSCfg, sizeof(WEB_NSS_CONFIG));
			}
			else
			{
				nRet = -1;
			}
			break;
		case 11:	//U com
			if (retlen == sizeof(CONFIG_INTERVIDEO_UCOM))
			{
				CONFIG_INTERVIDEO_UCOM *pIUCfg = (CONFIG_INTERVIDEO_UCOM*)cRecvBuf;
				
				pstIUC->bAliveEnable = pIUCfg->iAliveEnable;
				pstIUC->bFuncEnable = pIUCfg->iFuncEnable;
				pstIUC->dwAlivePeriod = pIUCfg->iAlivePeriod;
				memcpy(pstIUC->szDeviceId, pIUCfg->strDeviceId, 32-1);
				memcpy(pstIUC->szPassWord, pIUCfg->strPassWord, 32-1);
				memcpy(pstIUC->szRegPwd, pIUCfg->strRegPwd, 16-1);
				memcpy(pstIUC->szUserName, pIUCfg->strUserName, 32-1);
				in_addr tmp = {0};
				tmp.s_addr = pIUCfg->ServerIp;
				strcpy(pstIUC->szServerIp, inet_ntoa(tmp));
				pstIUC->wServerPort = pIUCfg->ServerPort;
				for (int i=0; i<N_SYS_CH; i++)
				{
					pstIUC->struChnInfo[i].bChnEn = pIUCfg->sChnInfo[i].iChnEn;
					memcpy(pstIUC->struChnInfo[i].szChnId, pIUCfg->sChnInfo[i].strChnId, 32-1);
				}
			}
			else
			{
				nRet = -1;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		nRet = -1;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

//function for getting structured config, recoded by Linjy
int CDevConfig::GetDevConfig(LONG lLoginID, DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned, int waittime)
{	
	if (!lpOutBuffer || !lpBytesReturned)
	{
		return NET_ILLEGAL_PARAM;
	}

	if (lChannel < -1 || lChannel >= MAX_CHANNUM)
	{
		return NET_ILLEGAL_PARAM;
	}

	//g_query_wait = waittime;

	*lpBytesReturned = 0;
	memset(lpOutBuffer, 0, dwOutBufferSize);

	afk_device_s* device = (afk_device_s*)lLoginID;

	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int retlen = 0; //data length
	int nRet = -1;  //return value

	switch(dwCommand)
	{
	case DEV_DEVICECFG:				//获取设备参数
		{
			if (dwOutBufferSize < sizeof(DEV_SYSTEM_ATTR_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEV_SYSTEM_ATTR_CFG stSAC;

			nRet = GetDevConfig_DevCfg(lLoginID, stSAC, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stSAC, sizeof(DEV_SYSTEM_ATTR_CFG));
			*lpBytesReturned = sizeof(DEV_SYSTEM_ATTR_CFG);
		}
		break;
	case DEV_NETCFG:				//获取网络参数
		{
			if (dwOutBufferSize < sizeof(DEVICE_NET_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			DEVICE_NET_CFG stNC;
			nRet = GetDevConfig_NetCfg(lLoginID, stNC, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stNC, sizeof(DEVICE_NET_CFG));
			*lpBytesReturned = sizeof(DEVICE_NET_CFG);
		}
		break;
	case DEV_CHANNELCFG:				//获取图象通道参数
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				//-1 is very special here
				int chanNum = device->channelcount(device);

				//超过16路的先取16路
				int CurchanNum = chanNum>16?16:chanNum;
	
				if (lChannel == -1)
				{
					DEV_CHANNEL_CFG *stCHC_ALL= new DEV_CHANNEL_CFG[chanNum];
					memset(stCHC_ALL, 0, chanNum*sizeof(DEV_CHANNEL_CFG));

					if (!stCHC_ALL)
					{
						return NET_SYSTEM_ERROR;
					}
					
					nRet = GetDevConfig_AllChnCfg(lLoginID, stCHC_ALL, CurchanNum, waittime);
					if (nRet < 0)
					{
						delete[] stCHC_ALL;
						return nRet;
					}	

					if (dwOutBufferSize < chanNum*sizeof(DEV_CHANNEL_CFG)) 
					{
						if (dwOutBufferSize < 16*sizeof(DEV_CHANNEL_CFG)) 
						{
							if (stCHC_ALL) 
							{
								delete[] stCHC_ALL;
								stCHC_ALL = NULL;	
							}
							return NET_ILLEGAL_PARAM;
						}
						else //通道超过16路，但是用户缓冲只开16路的
						{
							memcpy(lpOutBuffer, stCHC_ALL, CurchanNum*sizeof(DEV_CHANNEL_CFG));
							*lpBytesReturned = CurchanNum*sizeof(DEV_CHANNEL_CFG);
							if (stCHC_ALL) 
							{
								delete[] stCHC_ALL;
								stCHC_ALL = NULL;
							}	
							return 0;
						}
					}

					if (chanNum > 16) //超过16的只取通道名称
					{
						char *buf = new char[chanNum*64+32];
						if (!buf)
						{
							if (stCHC_ALL) 
							{
								delete[] stCHC_ALL;
								stCHC_ALL = NULL;
							}
							return NET_SYSTEM_ERROR;
						}

						memset(buf, 0, chanNum*64+32);//支持新通道名协议	
						int nChannelCount = 0;
						nRet = QueryChannelName(lLoginID, buf, chanNum*64+32, &nChannelCount, waittime);	
						if (nRet >= 0)
						{
							for (int i = 0; i < chanNum; i++)
							{
								memcpy(stCHC_ALL[i].szChannelName, &buf[i*CHANNEL_NAME_LEN], CHANNEL_NAME_LEN);
							}
						}
						delete []buf;
					}

					memcpy(lpOutBuffer, stCHC_ALL, chanNum*sizeof(DEV_CHANNEL_CFG));
					*lpBytesReturned = chanNum*sizeof(DEV_CHANNEL_CFG);
					if (stCHC_ALL) 
					{
						delete[] stCHC_ALL;
						stCHC_ALL = NULL;
					}
					
				}
				else
				{
					if ((lChannel >= 0 && lChannel < chanNum && dwOutBufferSize < sizeof(DEV_CHANNEL_CFG)) || lChannel > 16)
					{
						return NET_ILLEGAL_PARAM;
					}
					
					DEV_CHANNEL_CFG stCHC = {0};
					
					nRet = GetDevConfig_ChnCfg(lLoginID, stCHC, lChannel, waittime);
					if (nRet < 0)
					{
						return nRet;
					}
					
					memcpy(lpOutBuffer, &stCHC, sizeof(DEV_CHANNEL_CFG));
					*lpBytesReturned = sizeof(DEV_CHANNEL_CFG);
				}
			}
		}
		break;
	case DEV_RECORDCFG:				//获取定时录像参数
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				//-1 is very special here
				int chanNum = device->channelcount(device);
				if ( (lChannel >= 0 && lChannel < chanNum && dwOutBufferSize < sizeof(DEV_RECORD_CFG)) ||
					(lChannel == -1 && dwOutBufferSize < chanNum*sizeof(DEV_RECORD_CFG)) ||
					(lChannel >= chanNum && lChannel != -1) )
				{
					return NET_ILLEGAL_PARAM;
				}
				
				if (-1 == lChannel)
				{
					DEV_RECORD_CFG *stREC_ALL = new DEV_RECORD_CFG[chanNum];
					if (!stREC_ALL)
					{
						return NET_SYSTEM_ERROR;
					}
					
					int iProtocolVersion = 0;
					device->get_info(device, dit_protocol_version, &iProtocolVersion);
					if (iProtocolVersion > 2)
					{
						nRet = GetDevConfig_AllRecCfgNew(lLoginID, stREC_ALL, chanNum, waittime);
						if (nRet < 0)
						{
							delete[] stREC_ALL;
							return nRet;
						}		
					}
					else
					{
						nRet = GetDevConfig_AllRecCfg(lLoginID, stREC_ALL, chanNum, waittime);
						if (nRet < 0)
						{
							delete[] stREC_ALL;
							return nRet;
						}		
					}	
					
					memcpy(lpOutBuffer, stREC_ALL, chanNum*sizeof(DEV_RECORD_CFG));
					*lpBytesReturned = chanNum*sizeof(DEV_RECORD_CFG);
					delete[] stREC_ALL;
				}
				else
				{
					DEV_RECORD_CFG stREC;
					
					int iProtocolVersion;
					device->get_info(device, dit_protocol_version, &iProtocolVersion);
					if (iProtocolVersion > 2)
					{
						nRet = GetDevConfig_RecCfgNew(lLoginID, stREC, lChannel, waittime);
						if (nRet < 0)
						{
							return nRet;
						}
					}
					else
					{
						nRet = GetDevConfig_RecCfg(lLoginID, stREC, lChannel, waittime);
						if (nRet < 0)
						{
							return nRet;
						}
					}
					
					memcpy(lpOutBuffer, &stREC, sizeof(DEV_RECORD_CFG));
					*lpBytesReturned = sizeof(DEV_RECORD_CFG);
				}
			}
		}
		break;		
	case DEV_COMMCFG:				//获取串口参数
		{
			if (dwOutBufferSize < sizeof(DEV_COMM_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_COMM_CFG stCMC;
			
			nRet = GetDevConfig_ComCfg(lLoginID, stCMC, waittime);
			if (nRet < 0) 
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stCMC, sizeof(DEV_COMM_CFG));
			*lpBytesReturned = sizeof(DEV_COMM_CFG);
		}
		break;
	case DEV_ALARMCFG:			//获取报警布防参数
		{
			if (dwOutBufferSize < sizeof(DEV_ALARM_SCHEDULE))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_ALARM_SCHEDULE stAS;
			int iProtocolVersion;
			device->get_info(device, dit_protocol_version, &iProtocolVersion);
			if (iProtocolVersion > 2)
			{
				nRet = GetDevConfig_AlmCfgNew(lLoginID, stAS, waittime);
			}
			else
			{
				nRet = GetDevConfig_AlmCfg(lLoginID, stAS, waittime);
			}

			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stAS, sizeof(DEV_ALARM_SCHEDULE));
			*lpBytesReturned = sizeof(DEV_ALARM_SCHEDULE);
		}
		break;		
	case DEV_TIMECFG:				//获取DVR时间
		{
			if (dwOutBufferSize < sizeof(NET_TIME))
			{
				return NET_ILLEGAL_PARAM;
			}

			NET_TIME stNetTime;
			nRet = GetDevConfig_Devtime(lLoginID, stNetTime, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stNetTime, sizeof(NET_TIME));
			*lpBytesReturned = sizeof(NET_TIME);
		}
		break;
	case DEV_PREVIEWCFG:			//获取预览参数
		{
			if (dwOutBufferSize < sizeof(DEV_PREVIEW_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_PREVIEW_CFG stPRC;
			nRet = GetDevConfig_PreviewCfg(lLoginID, stPRC, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stPRC, sizeof(DEV_PREVIEW_CFG));
			*lpBytesReturned = sizeof(DEV_PREVIEW_CFG);
		}
		break;
	case DEV_AUTOMTCFG:
		{
			if (dwOutBufferSize < sizeof(DEV_AUTOMT_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_AUTOMT_CFG stATC;
			nRet = GetDevConfig_AutoMtCfg(lLoginID, stATC, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stATC, sizeof(DEV_AUTOMT_CFG));
			*lpBytesReturned = sizeof(DEV_AUTOMT_CFG);
		}
		break;
	case DEV_VEDIO_MARTIX:
		{
			if (dwOutBufferSize < sizeof(DEV_VIDEO_MATRIX_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_VIDEO_MATRIX_CFG *pstMatrix = NULL;
			pstMatrix = (DEV_VIDEO_MATRIX_CFG*)lpOutBuffer;
			nRet = GetDevConfig_VdMtrxCfg(lLoginID, pstMatrix, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEV_VIDEO_MATRIX_CFG);
		}
		break;
	case DEV_MULTI_DDNS:
		{
			if (dwOutBufferSize < sizeof(DEV_MULTI_DDNS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_MULTI_DDNS_CFG stMD;
			nRet = GetDevConfig_MultiDdnsCfg(lLoginID, stMD, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stMD, sizeof(DEV_MULTI_DDNS_CFG));
			*lpBytesReturned = sizeof(DEV_MULTI_DDNS_CFG);
		}
		break;
	case DEV_POS_CFG:
		{
			if (dwOutBufferSize < sizeof(DEVICE_POS_INFO))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_POS_INFO stPosCfg ;
			memset( &stPosCfg , 0 , sizeof( stPosCfg ) )  ;
			nRet = GetDevConfig_PosCfg(lLoginID, &stPosCfg, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stPosCfg, sizeof(DEVICE_POS_INFO));
			*lpBytesReturned = sizeof(DEVICE_POS_INFO);
		}
		break ;
	case DEV_SNAP_CFG:
		{
			int channum = device->channelcount(device);
			//hdvr
			if(device->device_type(device) == PRODUCT_DVR_MIX_DVR)
			{
				channum -= 4;//4个数字通道
			}

			if (dwOutBufferSize < channum*sizeof(DEVICE_SNAP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVICE_SNAP_CFG *pstSC = new DEVICE_SNAP_CFG[channum];
			nRet = GetDevConfig_SnapCfg(lLoginID, pstSC, channum, waittime);
			if (nRet < 0)
			{
				delete[] pstSC;
				return nRet;
			}

			memcpy(lpOutBuffer, pstSC, channum*sizeof(DEVICE_SNAP_CFG));
			*lpBytesReturned = channum*sizeof(DEVICE_SNAP_CFG);
			delete[] pstSC;
		}
		break;
	case DEV_WEB_URL_CFG:
		{
			if (dwOutBufferSize < sizeof(DEVWEB_URL_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEVWEB_URL_CFG stUC;
			nRet = GetDevConfig_UrlCfg(lLoginID, stUC, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stUC, sizeof(DEVWEB_URL_CFG));
			*lpBytesReturned = sizeof(DEVWEB_URL_CFG);
		}
		break;
	case DEV_FTP_PROTO_CFG:
		{
			if (dwOutBufferSize < sizeof(DEVICE_FTP_PROTO_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEVICE_FTP_PROTO_CFG stFC;
			nRet = GetDevConfig_FtpCfg(lLoginID, stFC, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			memcpy(lpOutBuffer, &stFC, sizeof(DEVICE_FTP_PROTO_CFG));
			*lpBytesReturned = sizeof(DEVICE_FTP_PROTO_CFG);
		}
		break;
	case DEV_INTERVIDEO_CFG:
		{
			switch(lChannel)
			{
			case 4:		//贝尔
				if (dwOutBufferSize < sizeof(DEV_INTERVIDEO_BELL_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				*lpBytesReturned = sizeof(DEV_INTERVIDEO_BELL_CFG);
				break;
			case 10:	//中兴力维
				if (dwOutBufferSize < sizeof(DEV_INTERVIDEO_NSS_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				*lpBytesReturned = sizeof(DEV_INTERVIDEO_NSS_CFG);
				break;
			case 11:	//U网通
				if (dwOutBufferSize < sizeof(DEV_INTERVIDEO_UCOM_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				*lpBytesReturned = sizeof(DEV_INTERVIDEO_UCOM_CFG);
				break;
			default:
				{
					//不支持该配置
					return NET_ILLEGAL_PARAM;
				}
				break;
			}

			nRet = GetDevConfig_InterVedioCfg(lLoginID, (char*)lpOutBuffer, dwOutBufferSize, lChannel, waittime);
			if (nRet < 0)
			{
				*lpBytesReturned = 0;
				return nRet;
			}
		}
		break;
	//Add: zsc(11402) 2008-1-18
	case DEV_VIDEO_COVER:	//区域遮挡配置
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				int chanNum = device->channelcount(device);
				chanNum = chanNum>16?16:chanNum;

				if ( (lChannel >= 0 && lChannel < chanNum && dwOutBufferSize < sizeof(DEV_VIDEOCOVER_CFG)) ||
					(lChannel == -1 && dwOutBufferSize < chanNum*sizeof(DEV_VIDEOCOVER_CFG)) ||
					(lChannel >= chanNum) )
				{
					return NET_ILLEGAL_PARAM;
				}
				
				if (lChannel == -1)
				{
					DEV_VIDEOCOVER_CFG *stuChnCover_ALL= new DEV_VIDEOCOVER_CFG[chanNum];
					if (stuChnCover_ALL == NULL)
					{
						return NET_SYSTEM_ERROR;
					}
					
					nRet = GetDevConfig_AllChnCoverCfg(lLoginID, stuChnCover_ALL, chanNum,waittime);
					if (nRet < 0)
					{
						delete[] stuChnCover_ALL;
						return nRet;
					}	
					
					memcpy(lpOutBuffer, stuChnCover_ALL, chanNum*sizeof(DEV_VIDEOCOVER_CFG));
					*lpBytesReturned = chanNum*sizeof(DEV_VIDEOCOVER_CFG);
					delete[] stuChnCover_ALL;
				}
				else
				{
					DEV_VIDEOCOVER_CFG stuChnCover = {0};
					
					nRet = GetDevConfig_ChnCoverCfg(lLoginID, stuChnCover, lChannel,waittime);
					if (nRet < 0)
					{
						return nRet;
					}
					
					memcpy(lpOutBuffer, &stuChnCover, sizeof(DEV_VIDEOCOVER_CFG));
					*lpBytesReturned = sizeof(DEV_VIDEOCOVER_CFG);
				}
			}
		}
		break;
	//End: zsc(11402)
		
	//Add: by ljy(10858) 2008-3-11
	case DEV_TRANS_STRATEGY:
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				if (dwOutBufferSize < sizeof(DEV_TRANSFER_STRATEGY_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				
				DEV_TRANSFER_STRATEGY_CFG stTSC;
				nRet = GetDevConfig_TransStrategyCfg(lLoginID, stTSC, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				
				memcpy(lpOutBuffer, &stTSC, sizeof(DEV_TRANSFER_STRATEGY_CFG));
				*lpBytesReturned = sizeof(DEV_TRANSFER_STRATEGY_CFG);
			}
		}
		break;
	//End: ljy(10858)
		
		//Add: by ljy(10858) 2008-3-24
	case DEV_DOWNLOAD_STRATEGY:
		{
			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				if (dwOutBufferSize < sizeof(DEV_DOWNLOAD_STRATEGY_CFG))
				{
					return NET_ILLEGAL_PARAM;
				}
				
				DEV_DOWNLOAD_STRATEGY_CFG stDSC;
				nRet = GetDevConfig_DownloadStrategyCfg(lLoginID, stDSC, waittime);
				if (nRet < 0)
				{
					return nRet;
				}
				
				memcpy(lpOutBuffer, &stDSC, sizeof(DEV_DOWNLOAD_STRATEGY_CFG));
				*lpBytesReturned = sizeof(DEV_DOWNLOAD_STRATEGY_CFG);
			}
		}
		break;
	//End: ljy(10858)

	//Add: by zsc(11402) 2008-4-16
	case DEV_WATERMAKE_CFG:
		{
			int chanNum = device->channelcount(device);
			if ( (lChannel >= 0 && lChannel < chanNum && dwOutBufferSize < sizeof(DEVICE_WATERMAKE_CFG)) ||
				 (lChannel == -1 && dwOutBufferSize < chanNum*sizeof(DEVICE_WATERMAKE_CFG)) ||
				 (lChannel >= chanNum) )
			{
				return NET_ILLEGAL_PARAM;
			}

			if (lChannel == -1)
			{
				nRet = GetDevConfig_AllWaterMakeCfg(lLoginID, (DEVICE_WATERMAKE_CFG*)lpOutBuffer, chanNum, waittime);
				if (nRet < 0)
				{
					return nRet;
				}	
				
				*lpBytesReturned = chanNum*sizeof(DEVICE_WATERMAKE_CFG);
			}
			else
			{
				nRet = GetDevConfig_WaterMakeCfg(lLoginID, (DEVICE_WATERMAKE_CFG*)lpOutBuffer, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}

				*lpBytesReturned = sizeof(DEVICE_WATERMAKE_CFG);
			}
		}
		break;
	case DEV_WLAN_CFG:		//配置无线网络信息
		{
			if (dwOutBufferSize < sizeof(DEV_WLAN_INFO))
			{
				return NET_ILLEGAL_PARAM;
			}

			DEV_WLAN_INFO stuWlanInfo = {0};

			nRet = GetDevConfig_WLANCfg(lLoginID, stuWlanInfo, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			memcpy(lpOutBuffer, &stuWlanInfo, sizeof(DEV_WLAN_INFO));
			*lpBytesReturned = sizeof(DEV_WLAN_INFO);
		}
		break;
	case DEV_WLAN_DEVICE_CFG:		//搜索无线设备
		{
			if (dwOutBufferSize < sizeof(DEV_WLAN_DEVICE_LIST))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_WLANDevCfg(lLoginID, (DEV_WLAN_DEVICE_LIST*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEV_WLAN_DEVICE_LIST);
		}
		break;
	case DEV_REGISTER_CFG:	//主动注册参数配置
		{
			if (dwOutBufferSize < sizeof(DEV_REGISTER_SERVER))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_AutoRegisterCfg(lLoginID, (DEV_REGISTER_SERVER*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEV_REGISTER_SERVER);
		}
		break;
	case DEV_CAMERA_CFG:		//摄像头属性配置
		{
			int chanNum = device->channelcount(device);
			if ( (lChannel >= 0 && lChannel < chanNum && dwOutBufferSize < sizeof(DEVICE_CAMERA_CFG)) ||
				 (lChannel == -1 && dwOutBufferSize < chanNum*sizeof(DEVICE_CAMERA_CFG)) ||
				 (lChannel >= chanNum) )
			{
				return NET_ILLEGAL_PARAM;
			}

			if (lChannel == -1)
			{
				DEVICE_CAMERA_CFG *stCHC_ALL= new DEVICE_CAMERA_CFG[chanNum];
				if (stCHC_ALL == NULL)
				{
					return NET_SYSTEM_ERROR;
				}

				nRet = GetDevConfig_AllChnCmrCfg(lLoginID, stCHC_ALL, chanNum, waittime);
				if (nRet < 0)
				{
					delete[] stCHC_ALL;
					return nRet;
				}	
				
				memcpy(lpOutBuffer, stCHC_ALL, chanNum*sizeof(DEVICE_CAMERA_CFG));
				*lpBytesReturned = chanNum*sizeof(DEVICE_CAMERA_CFG);
				delete[] stCHC_ALL;
			}
			else
			{
				DEVICE_CAMERA_CFG stCHC;
				
				nRet = GetDevConfig_ChnCmrCfg(lLoginID, stCHC, lChannel, waittime);
				if (nRet < 0)
				{
					return nRet;
				}

				memcpy(lpOutBuffer, &stCHC, sizeof(DEVICE_CAMERA_CFG));
				*lpBytesReturned = sizeof(DEVICE_CAMERA_CFG);
			}
		}
		break;
	case DEV_INFRARED_CFG:			//红外报警参数
		{
			if (dwOutBufferSize < sizeof(INFRARED_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_InfraredCfg(lLoginID, (INFRARED_CFG*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(INFRARED_CFG);
		}
		break;
	case DEV_MAIL_CFG:				//邮件配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_MAIL_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_MailCfg(lLoginID, (DEVICE_MAIL_CFG*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			*lpBytesReturned = sizeof(DEV_MAIL_CFG);
		}
		break;
	case DEV_DNS_CFG:			//DNS服务器配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_DNS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_DNSCfg(lLoginID, (DEVICE_DNS_CFG*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEVICE_DNS_CFG);
		}
		break;
	case DEV_NTP_CFG:			//NTP配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_NTP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_NTPCfg(lLoginID, (DEVICE_NTP_CFG*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}

			*lpBytesReturned = sizeof(DEVICE_NTP_CFG);
		}
		break;
	//End: zsc(11402)

	//Add: by cqs(10842) 2008-5-9
	case DEV_SNIFFER_CFG:
		{
			if ( dwOutBufferSize < sizeof(DEVICE_SNIFFER_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			nRet = GetDevConfig_SnifferCfg(lLoginID, (DEVICE_SNIFFER_CFG*)lpOutBuffer, waittime);
			if (nRet < 0)
			{
				return nRet;
			}	
				
			*lpBytesReturned = sizeof(DEVICE_SNIFFER_CFG);
		}
		break;
	case DEV_AUDIO_DETECT_CFG:			//音频检测报警参数
		{
			if (dwOutBufferSize < sizeof(AUDIO_DETECT_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			nRet = GetDevConfig_AudioDetectCfg(lLoginID, (AUDIO_DETECT_CFG*)lpOutBuffer,waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			*lpBytesReturned = sizeof(AUDIO_DETECT_CFG);
		}
		break;
	case DEV_STORAGE_STATION_CFG:
		{
			//-1--ALL
			int ChannelNum = device->channelcount(device);
			if ((lChannel >=0 && lChannel < ChannelNum && dwOutBufferSize < sizeof(STORAGE_STATION_CFG)) ||
					(lChannel == -1 && dwOutBufferSize < ChannelNum*sizeof(STORAGE_STATION_CFG)) ||
					(lChannel >= ChannelNum))
			{
				return NET_ILLEGAL_PARAM;
			}

			if (lChannel == -1) //all
			{
				STORAGE_STATION_CFG *stuChannelAll = new STORAGE_STATION_CFG[ChannelNum];
				if (!stuChannelAll) 
				{
					return NET_SYSTEM_ERROR;
				}
				nRet = GetDevConfig_AllStorageStateCfg(lLoginID,stuChannelAll,ChannelNum, waittime);
				if (nRet < 0) 
				{
					delete[] stuChannelAll;
					return nRet;
				}

				memcpy(lpOutBuffer, stuChannelAll, ChannelNum * sizeof(STORAGE_STATION_CFG));
				*lpBytesReturned = ChannelNum * sizeof(STORAGE_STATION_CFG);
				delete[] stuChannelAll;
			}
			else
			{
				STORAGE_STATION_CFG stState;
				memset(&stState, 0 , sizeof(STORAGE_STATION_CFG));

				nRet = GetDevConfig_StorageStateCfg(lLoginID, stState, lChannel, waittime); 
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer,&stState,sizeof(STORAGE_STATION_CFG));
				*lpBytesReturned = sizeof(STORAGE_STATION_CFG);
			}	
		}
		break;
		/*
	//	delete by yp using CLIENT_GetPtzOptAttr() 

	case DEV_PTZ_OPT_CFG:
		{
			if (dwOutBufferSize < sizeof(PTZ_OPT_ATTR))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			nRet = GetDevConfig_PtzCfg(lLoginID, (PTZ_OPT_ATTR*)lpOutBuffer,2,waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			*lpBytesReturned = sizeof(PTZ_OPT_ATTR);
			
		}
		break;
		*/
	case DEV_DST_CFG: // 夏令时配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_DST_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_DSTCfg(lLoginID, (DEVICE_DST_CFG*)lpOutBuffer, waittime);

			if (nRet < 0)
			{
				return nRet;
			}	
				
			*lpBytesReturned = sizeof(DEVICE_DST_CFG);

		}
		break;
		
	case DEV_CDMAGPRS_CFG: // GPRS/CDMA网络配置
		{
			if (dwOutBufferSize < sizeof(DEVICE_CDMAGPRS_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			nRet = GetDevConfig_GPRSCDMACfg(lLoginID, (DEVICE_CDMAGPRS_CFG*)lpOutBuffer, waittime);
			
			if (nRet < 0)
			{
				return nRet;
			}	
			
			*lpBytesReturned = sizeof(DEVICE_CDMAGPRS_CFG);
			
		}
		break;
	case DEV_VIDEO_OSD_CFG://视频OSD叠加配置
		{
			
			//-1--ALL
			int ChannelNum = device->channelcount(device);
			if ((lChannel >=0 && lChannel < ChannelNum && dwOutBufferSize < sizeof(DVR_VIDEO_OSD_CFG)) ||
				(lChannel == -1 && dwOutBufferSize < ChannelNum*sizeof(DVR_VIDEO_OSD_CFG)) ||
				(lChannel >= ChannelNum))
			{
				return NET_ILLEGAL_PARAM;
			}
			
			if (lChannel == -1) //all
			{
				DVR_VIDEO_OSD_CFG *stuChannelAll = new DVR_VIDEO_OSD_CFG[ChannelNum];
				if (!stuChannelAll) 
				{
					return NET_SYSTEM_ERROR;
				}
				nRet = GetDevConfig_AllVideoOSDCfg(lLoginID,stuChannelAll,ChannelNum, waittime);
				if (nRet < 0) 
				{
					delete[] stuChannelAll;
					return nRet;
				}
				
				memcpy(lpOutBuffer, stuChannelAll, ChannelNum * sizeof(DVR_VIDEO_OSD_CFG));
				*lpBytesReturned = ChannelNum * sizeof(DVR_VIDEO_OSD_CFG);
				delete[] stuChannelAll;
			}
			else
			{
				DVR_VIDEO_OSD_CFG stState;
				memset(&stState, 0 , sizeof(DVR_VIDEO_OSD_CFG));
				
				nRet = GetDevConfig_VideoOSDCfg(lLoginID, stState, lChannel, waittime); 
				if (nRet < 0)
				{
					return nRet;
				}
				memcpy(lpOutBuffer,&stState,sizeof(DVR_VIDEO_OSD_CFG));
				*lpBytesReturned = sizeof(DVR_VIDEO_OSD_CFG);
			}	
		}
		break;
	//End: cqs(10842)
	case DEV_ALARM_CENTER_CFG:
		{
			if(dwOutBufferSize < sizeof(ALARMCENTER_UP_CFG))
			{
				return NET_ILLEGAL_PARAM;
			}

			nRet = GetDevConfig_AlarmCenterCfg(lLoginID, (ALARMCENTER_UP_CFG *)lpOutBuffer, waittime);
			if(nRet < 0)
			{
				return nRet;
			}
			*lpBytesReturned = sizeof(ALARMCENTER_UP_CFG);
		}
		break;
	default:
		break;
	}
	return nRet;
} 
//End of get config!

int CDevConfig::SetupChannelName(LONG lLoginID,char *pbuf, int nbuflen)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pbuf || nbuflen < 16*CHANNEL_NAME_LEN)
	{
		return NET_ILLEGAL_PARAM;
	}

    afk_device_s *device = (afk_device_s*)lLoginID;
	
	BYTE sendBuf[32*64];
	memset(sendBuf,0,sizeof(sendBuf));
	int sendbuflen = 0;

	DWORD dwSp = 0;
	device->get_info(device, dit_special_cap_flag, &dwSp);
	
	if ((dwSp&SP_NEW_CHANNAME) == 0)
	{
		//旧的通道名协议，512字节
		char *pUTFData = new char[32*2+2];
		if (!pUTFData)
		{
			return NET_SYSTEM_ERROR;
		}
		memset(pUTFData, 0 , 32*2+2);
		for(int i = 0; i < 16; i++)
		{
			Change_Assic_UTF8((char *)(pbuf + CHANNEL_NAME_LEN * i), 32, pUTFData, 32*2+2);
			
			memcpy((char *)sendBuf + 32 * i, pUTFData, 32);
			//*(sendBuf+32*(i+1)-1) = '\0';
		}
		delete[] pUTFData;

		sendbuflen = 512;
	}
	else
	{
		//新的通道名协议，不定长
		char *pUTFData = new char[32*2+2];
		if (!pUTFData)
		{
			return NET_SYSTEM_ERROR;
		}
		memset(pUTFData, 0 , 32*2+2);
		int bufPos = 0;
		int channum = ((afk_device_s*)lLoginID)->channelcount(((afk_device_s*)lLoginID));
	//	channum = channum>MAX_CHANNUM ? MAX_CHANNUM : channum;
		channum = channum>32 ? 32 : channum;
		for(int i = 0; i < channum; i++)
		{
			//可能无'\0'
			//为之预留
			char tmpBuf[CHANNEL_NAME_LEN+1] = {0};
			memcpy(tmpBuf, pbuf + CHANNEL_NAME_LEN * i, CHANNEL_NAME_LEN);

			Change_Assic_UTF8(tmpBuf, CHANNEL_NAME_LEN, pUTFData, 32*2+2);
			int length = strlen(pUTFData);
			length = length>64 ? 64 : length;

			memcpy(sendBuf+bufPos, pUTFData, length);
			bufPos += length;
			memset(sendBuf+bufPos, '&', 2);
			bufPos += 2;
		}
		delete[] pUTFData;

		sendbuflen = bufPos-2;//最后两个'&&'不要

//		sendbuflen = 16*CHANNEL_NAME_LEN;
//		int ret = BuildChannelNameBuf((char*)pbuf, channum*CHANNEL_NAME_LEN, (char*)sendBuf, &sendbuflen);
//		if (ret < 0)
//		{
//			return NET_ILLEGAL_PARAM;
//		}
	}

	int nRet = -1;
    //现在没有DVR的返回值,waittime无用

    afk_config_channel_param_s configchannel;
    configchannel.base.func = 0;
    configchannel.type = AFK_CHANNEL_CONFIG_CONFIG;
	configchannel.subtype = CONFIG_TYPE_TITLE;
    configchannel.configinfo.configbuf = (unsigned char*)sendBuf;
    configchannel.configinfo.buflen = sendbuflen;
    configchannel.base.udata = 0;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
        pchannel->close(pchannel);
        nRet = 0;
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}
    return nRet;
}

int __stdcall QueryDevStateFunc(
							  afk_handle_t object,	/* 数据提供者 */
							  unsigned char *data,	/* 数据体 */
							  unsigned int datalen,	/* 数据长度 */
							  void *param,			/* 回调参数 */
							  void *udata)
{
	afk_channel_s *channel = (afk_channel_s*)object;
	if (!channel)
	{
		return -1;
	}
	
	afk_device_s *device = (afk_device_s*)channel->get_device(channel);
	if (!device)
	{
		return -1;
	}

    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }
	
	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}
	
	NET_CLIENT_STATE *pcommState = 0;
	BURNING_PROGRESS *pBurnProgress = NULL;
	DEV_WIRELESS_RSS_INFO *pRSS = 0;
	BYTE		*pShelter = 0;
	BYTE		*pRecord = 0;
	int i = 0;
	
	switch(receivedata->reserved[0])
	{
	case DEVSTATE_COMM_ALARM:
		if (HEADER_SIZE == datalen) 
		{
			//缓冲区不够大
			if (receivedata->maxlen < sizeof(NET_CLIENT_STATE))
			{
				goto e_out;
			}

 			pcommState = (NET_CLIENT_STATE*)receivedata->data;
// 			if (!pcommState/* || pcommState->alarminputcount > 16 || pcommState->channelcount > 16*/)
// 			{
// 				goto e_out;
// 			}
			
			memset(pcommState, 0, sizeof(NET_CLIENT_STATE));

			pcommState->alarminputcount = device->alarminputcount(device);
			pcommState->channelcount = device->channelcount(device);
			
			//外部报警
			DWORD alarm = *(DWORD*)(data+16);
			for (i = 0; i < pcommState->alarminputcount; i++)
			{
				pcommState->alarm[i] = (alarm>>i)%2;
			}
			
			//视频丢失
			DWORD videolost = *(DWORD*)(data+20);
			for (i = 0; i < pcommState->channelcount; i++)
			{
				pcommState->videolost[i] = (videolost>>i)%2;
			}
			
			//动态检测: 
			DWORD motiondection = *(DWORD*)(data + 24);
			for (i = 0; i < pcommState->channelcount; i++)
			{
				pcommState->motiondection[i] = (motiondection>>i)%2;
			}
			
			*(receivedata->datalen) = sizeof(NET_CLIENT_STATE);
		}
		else
		{
			goto e_out;
		}
		break;
	case DEVSTATE_SHELTER_ALARM:
		if (4 == datalen && 0 == (int)param) 
		{
			//缓冲区不够大
			if (receivedata->maxlen < 16)
			{
				goto e_out;
			}
			
			pShelter = (BYTE*)receivedata->data;
			if (!pShelter) 
			{
				goto e_out;
			}

			memset(pShelter, 0, 16);
			
			DWORD shltMask = *(DWORD*)(data);
			for (int i = 0; i < 16; i++)
			{
				pShelter[i] = (shltMask & (0x01<<i))?1:0;
			}

			*(receivedata->datalen) = 16;
		}
		else
		{
			goto e_out;
		}
		break;
	case DEVSTATE_RECORDING:
		if (HEADER_SIZE == datalen) 
		{
			//缓冲区不够大
			if (receivedata->maxlen < 16)
			{
				goto e_out;
			}

			pRecord = (BYTE*)receivedata->data;
			if (!pRecord)
			{
				goto e_out;
			}

			//外部报警
			memset(pRecord, 0, 16);
			DWORD record = *(DWORD*)(data+9);
			for (i = 0; i < 16; i++)
			{
				pRecord[i] = (record>>i)%2;
			}

			*(receivedata->datalen) = 16;
		}
		else
		{
			goto e_out;
		}
		break;
	case DEVSTATE_RESOURCE:
		if (data[0] != 0xb1 || data[8] != 0x0b)
		{
			goto e_out;
		}
		{
		int i = sizeof(SYS_STATUS_T);
		if (datalen == sizeof(SYS_STATUS_T)+HEADER_SIZE && receivedata->maxlen >= 4)
		{
			SYS_STATUS_T *pSysStatus = (SYS_STATUS_T *)(data+HEADER_SIZE);
			*(DWORD *)(receivedata->data) = 0;
			if (pSysStatus->iCPUOcc > 50)
			{
				*(DWORD *)(receivedata->data) = 1;
			}
			*(DWORD *)(receivedata->data + 4) = pSysStatus->iDispStatus_TV;
			*(DWORD *)(receivedata->data + 8) = pSysStatus->iDispStatus_VGA;
			*receivedata->datalen = 3*sizeof(DWORD);
		}
		else
		{
			goto e_out;
		}
		}
		break;
	case DEVSTATE_BITRATE:
		if (data[0] != 0xb1 || data[8] != 0x0c)
		{
			goto e_out;
		}
		{
			DWORD dwDataLen = *(DWORD *)(data+4);
			if (dwDataLen > 1024 || dwDataLen == 0)
			{
				goto e_out;
			}
			char *pstr = new char [dwDataLen+1];
			if (NULL == pstr)
			{
				goto e_out;
			}
			memcpy(pstr, data+HEADER_SIZE, dwDataLen);
			pstr[dwDataLen] = 0;
			if (strlen(pstr) != dwDataLen)
			{
				goto e_out;
			}
			CStrParse parser;
			CStrParse subParser;
			string subString;
			int channel, codeType, bitRate;
			int total = receivedata->maxlen/sizeof(DWORD);
			DWORD *dwRetData = (DWORD *)receivedata->data;
			parser.setSpliter("&&");
			parser.setTrim();
			parser.Parse(pstr);
			subParser.setSpliter(":");
			if (parser.Size() <= 0)
			{
				goto e_out;
			}
			while ((subString = parser.getWord(), subString.compare("") != 0))
			{
				subParser.Parse(subString);
				if (subParser.Size() != 3)
				{
					goto e_out;
				}
				channel = subParser.getValue();
				codeType = subParser.getValue();
				bitRate = subParser.getValue();
				if (channel > total || codeType != 0)
				{
					continue;
				}
				*(dwRetData+channel) = bitRate;
			}
			*receivedata->datalen = min(16,total)*sizeof(DWORD);
		}
		break;
	case DEVSTATE_CONN:
		if (data[0] != 0xb5 || data[8] != 0x0b)
		{
			goto e_out;
		}
		{
			DWORD dwDataLen = *(DWORD *)(data+4);
			if (dwDataLen <= 0)
			{
				goto e_out;
			}
			char *pstr = new char [dwDataLen+1];
			if (NULL == pstr)
			{
				goto e_out;
			}
			memcpy(pstr, data+HEADER_SIZE, dwDataLen);
			pstr[dwDataLen] = 0;
			if (strlen(pstr) != dwDataLen)
			{
				delete []pstr;
				goto e_out;
			}
			CStrParse parser;
			CStrParse subParser;
			string subString;
			memset(receivedata->data, 0, receivedata->maxlen);
			DWORD *pdwCliNum = (DWORD *)receivedata->data;
			int recvNum = receivedata->maxlen/sizeof(DWORD) - 1;
			DWORD *pdwCliIp = (DWORD *)(receivedata->data + 4);
			parser.setSpliter("&&");
			subParser.setSpliter(":");
			parser.Parse(pstr);
			*receivedata->datalen = (recvNum+1)*sizeof(DWORD);
			while (subString = parser.getWord(), subString.compare("") != 0)
			{
				subParser.Parse(subString);
				if (subParser.Size() == 2)
				{
					(*pdwCliNum)++;
					if (recvNum > 0)
					{
						subString = subParser.getWord(1);
						*pdwCliIp = inet_addr(subString.c_str());
						pdwCliIp++;
						recvNum--;
					}
				}
			}
			if (recvNum > 0)
			{
				*receivedata->datalen = ((*pdwCliNum)+1)*sizeof(DWORD);
			}
			delete []pstr;
		}
		break;
	case DEVSTATE_BURNING_PROGRESS:
		{
			if (data[0] != 0xb1 || data[8] != 0x04) 
			{
				goto e_out;
			}
			pBurnProgress = (BURNING_PROGRESS*)receivedata->data;
			memset(pBurnProgress,0,sizeof(BURNING_PROGRESS));
			if (NULL == pBurnProgress)
			{
				goto e_out;
			}
			if(datalen == HEADER_SIZE+24)
			{
				if (*(DWORD*)(data+4) == 0)
				{
					pBurnProgress->bBurning = FALSE;
				}
				pBurnProgress->dwStatus = *(DWORD*)(data+HEADER_SIZE+20);
				
				//test 
				//	char temp[100] = {0};
				//	sprintf(temp, "get status = %d\n", pBurnProgress->dwStatus);
				//	OutputDebugString(temp);

				pBurnProgress->bBurning = pBurnProgress->dwStatus!=0? TRUE : FALSE;
				if (4 == pBurnProgress->dwStatus)
				{
					pBurnProgress->dwStatus = 0; //初始化状态
				}
				if (pBurnProgress->dwStatus == 1) 
				{
					FRAME_TIME *Tmptm = (FRAME_TIME*)(data+HEADER_SIZE);
					pBurnProgress->stTimeStart.dwYear = Tmptm->year;
					pBurnProgress->stTimeStart.dwMonth = Tmptm->month;
					pBurnProgress->stTimeStart.dwDay = Tmptm->day;
					pBurnProgress->stTimeStart.dwHour = Tmptm->hour;
					pBurnProgress->stTimeStart.dwMinute = Tmptm->minute;
					pBurnProgress->stTimeStart.dwSecond = Tmptm->second;
					pBurnProgress->dwTimeElapse = *(DWORD*)(data+HEADER_SIZE+4);
					pBurnProgress->dwTotalSpace = *(DWORD*)(data+HEADER_SIZE+8);
					pBurnProgress->dwRemainSpace = *(DWORD*)(data+HEADER_SIZE+12);
					pBurnProgress->dwBurned = *(DWORD*)(data+HEADER_SIZE+16);
				}
			}
			//新的协议
			else if(datalen == 36+HEADER_SIZE || datalen == 40+HEADER_SIZE)
			{
				BYTE *pData =  data+HEADER_SIZE;
				FRAME_TIME *time = (FRAME_TIME*)pData;
				pBurnProgress->stTimeStart.dwYear = time->year;
				pBurnProgress->stTimeStart.dwMonth = time->month;
				pBurnProgress->stTimeStart.dwDay = time->day;
				pBurnProgress->stTimeStart.dwHour = time->hour;
				pBurnProgress->stTimeStart.dwMinute = time->minute;
				pBurnProgress->stTimeStart.dwSecond = time->second;
				pData += sizeof(FRAME_TIME);
				memcpy(&(pBurnProgress->dwTimeElapse),pData,sizeof(DWORD));
				pData += sizeof(DWORD);
				memcpy(&(pBurnProgress->dwTotalSpace),pData,sizeof(DWORD));
				pData += sizeof(DWORD);
				memcpy(&(pBurnProgress->dwRemainSpace),pData,sizeof(DWORD));
				pData += sizeof(DWORD);
				memcpy(&(pBurnProgress->dwBurned),pData,sizeof(DWORD));
				pData += sizeof(DWORD);	
				pBurnProgress->bType = (*(DWORD*)(pData));
				pData += sizeof(DWORD);
				pBurnProgress->bOperateType = (*(DWORD*)(pData));
				pData += sizeof(DWORD);
				pBurnProgress->bRomType = (*(DWORD*)(pData));
				pData += sizeof(DWORD);
				pBurnProgress->bBurning = (*(DWORD*)(pData));
				pBurnProgress->dwStatus = -1;
				pBurnProgress->wChannelMask = 0;
				if(datalen == 40+HEADER_SIZE)
				{
					pData += sizeof(DWORD);
					pBurnProgress->wChannelMask = (*(WORD*)(pData));
				}
			}
			else
			{
				goto e_out;
			}
			*receivedata->datalen = sizeof(BURNING_PROGRESS);
		}
		break;
	case DEVSTATE_NET_RSSI:
		{
			if (data[0] != 0xb1 || data[8] != 0x0d)
			{
				goto e_out;
			}
			//缓冲区不够大
			if (receivedata->maxlen < sizeof(DEV_WIRELESS_RSS_INFO))
			{
				goto e_out;
			}
			
			pRSS = (DEV_WIRELESS_RSS_INFO*)receivedata->data;
			
			memset(pRSS, 0, sizeof(DEV_WIRELESS_RSS_INFO));
			
			DWORD dwDataLen = *(DWORD *)(data+4);
			if (dwDataLen <= 0)
			{
				goto e_out;
			}
			char *pstr = new char [dwDataLen+1];
			if (NULL == pstr)
			{
				goto e_out;
			}
			memcpy(pstr, data+HEADER_SIZE, dwDataLen);
			pstr[dwDataLen] = 0;
			if (strlen(pstr) != dwDataLen)
			{
				delete []pstr;
				goto e_out;
			}
			
			char szValue[64] = {0};
			char *p = GetProtocolValue(pstr, "TD-SCDMA:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				delete []pstr;
				goto e_out;;
			}
			pRSS->dwTDSCDMA = atoi(szValue);
			
			p = GetProtocolValue(pstr, "TD-SCDMA:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				delete []pstr;
				goto e_out;;
			}
			pRSS->dwTDSCDMA = atoi(szValue);
			
			p = GetProtocolValue(pstr, "WCDMA:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				delete []pstr;
				goto e_out;;
			}
			pRSS->dwWCDMA = atoi(szValue);
			
			p = GetProtocolValue(pstr, "CDMA1x:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				delete []pstr;
				goto e_out;;
			}
			pRSS->dwCDMA1x = atoi(szValue);
			
			p = GetProtocolValue(pstr, "EDGE:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				delete []pstr;
				goto e_out;;
			}
			pRSS->dwEDGE = atoi(szValue);
			
			p = GetProtocolValue(pstr, "EVDO:", "\r\n", szValue, 64);
			if (p == NULL)
			{
				delete []pstr;
				goto e_out;;
			}
			pRSS->dwEVDO = atoi(szValue);

			p = GetProtocolValue(pstr, "NetType:", "\r\n", szValue, 64);
			if(p != NULL)//这为后面增加的
			{
				pRSS->nCurrentType = 0;
				char szType[5][20] = {"TD-SCDMA", "WCDMA", "CDMA1x", "EDGE", "EVDO"};
				for(int i = 0; i < 5; i++)
				{
					if(_stricmp(szValue, szType[i]) == 0)
					{
						pRSS->nCurrentType = i+1;
						break;
					}
				}
			}
			else
			{	
				pRSS->nCurrentType = 0;
			}
			pRSS->dwSize = sizeof(DEV_WIRELESS_RSS_INFO);
			*receivedata->datalen = sizeof(DEV_WIRELESS_RSS_INFO);
			
			delete []pstr;
		}
		break;
	default:
		goto e_out;
	}
	
	receivedata->result = 0;
	SetEventEx(receivedata->hRecEvt);
	return 1;
e_out:
//	有可能是其他报警上传，继续等待
//	SetEvent(receivedata->hRecEvt); 
	return -1;
}

int CDevConfig::GetDevTalkFormatList(LONG lLoginID, DEV_TALK_FORMAT_LIST *pstTalkFormat, int waittime)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	afk_device_s *device = (afk_device_s*)lLoginID;

	if (pstTalkFormat == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(pstTalkFormat, 0, sizeof(DEV_TALK_FORMAT_LIST));

	int nRet = 0;
	int retlen = 0;
	unsigned long dwTalkFormat = 0;

	int	bufLen = sizeof(DIALOG_CAPS)+32*sizeof(AUDIO_ATTR_T);
	char* capBuf = new char[bufLen];
	if (NULL == capBuf)
	{
		return NET_SYSTEM_ERROR;
	}

	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_TALK_ATTR, capBuf, bufLen, &retlen, waittime);
	if (nRet >= 0 && retlen >= sizeof(DIALOG_CAPS) && ((retlen-sizeof(DIALOG_CAPS))%sizeof(AUDIO_ATTR_T))==0) 
	{
		DIALOG_CAPS* pDialogCap = (DIALOG_CAPS*)capBuf;
		int nAudioTypeNum = (retlen-sizeof(DIALOG_CAPS))/sizeof(AUDIO_ATTR_T);
		nAudioTypeNum = pDialogCap->iAudioTypeNum > nAudioTypeNum ? nAudioTypeNum : pDialogCap->iAudioTypeNum;
		pstTalkFormat->nSupportNum = nAudioTypeNum;
		for (int i = 0; i < nAudioTypeNum; i++)
		{
			AUDIO_ATTR_T* pAAT = (AUDIO_ATTR_T*)(capBuf+sizeof(DIALOG_CAPS)+i*sizeof(AUDIO_ATTR_T));

			pstTalkFormat->type[i].encodeType = (TALK_CODING_TYPE)pAAT->iAudioType;
			pstTalkFormat->type[i].dwSampleRate = pAAT->dwSampleRate;
			pstTalkFormat->type[i].nAudioBit = pAAT->iAudioBit;

			dwTalkFormat |= (unsigned long)pow(2,pAAT->iAudioType);
		}
	}
	else
	{
		// 默认无头PCM
		pstTalkFormat->nSupportNum = 1;
		pstTalkFormat->type[0].encodeType = TALK_CODING_DEFAULT;
		pstTalkFormat->type[0].dwSampleRate = 8000;
		pstTalkFormat->type[0].nAudioBit = 8;

		dwTalkFormat = 1;
	}

	if (capBuf != NULL)
	{
		delete capBuf;
	}

	device->set_info(device, dit_talk_encode_list, &dwTalkFormat);

	return 0;
}

/* 主动查询设备状态，包括报警、硬盘、录象状态等。 */
/*
	当 nType  = DEVSTATE_BURNING_PROGRESS 时,pRetLen表示查询设备的id号.
*/
int	CDevConfig::QueryDevState(LONG lLoginID, int nType, char *pBuf, int nBufLen, int *pRetLen, int waittime)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pBuf || !pRetLen)
	{
		return NET_ILLEGAL_PARAM;		
	}
	int nDevID = *pRetLen;
	*pRetLen = 0;
	
	int nRet =-1;
    afk_device_s *device = (afk_device_s*)lLoginID;
	
    afk_search_channel_param_s searchchannel = {0};
    searchchannel.base.func = QueryDevStateFunc;

    int configtype = AFK_CHENNEL_SEARCH_DEVSTATE;	//查询报警、状态
	int statetype = 0;
	switch(nType)
	{
	case DEVSTATE_COMM_ALARM:
		statetype = 0;	//普通报警状态
		searchchannel.subtype = 0x10;
		break;
	case DEVSTATE_SHELTER_ALARM:
		statetype = 1;	//遮挡报警状态
		searchchannel.subtype = 0x11;
		break;
	case DEVSTATE_RECORDING:
		statetype = 2;	//录象状态
		searchchannel.subtype = 0x12;
		break;
	case DEVSTATE_RESOURCE:
		if (nBufLen < 3*sizeof(DWORD))
		{
			return NET_ILLEGAL_PARAM;
		}
		statetype = 3;
		break;
	case DEVSTATE_BITRATE:
		if (device->channelcount(device)*sizeof(DWORD) > nBufLen)
		{
			return NET_ILLEGAL_PARAM;
		}
		statetype = 4;
		break;
	case DEVSTATE_CONN:
		if (nBufLen < sizeof(DWORD))
		{
			return NET_ILLEGAL_PARAM;
		}
		statetype = 5;
		break;
	case DEVSTATE_DISK:	// 硬盘状态
		{
			int nOldStructSize = 4 + 32*sizeof(NET_DEV_DISKSTATE);
			if (nBufLen < nOldStructSize) 
			{
				return NET_ILLEGAL_PARAM;
			}

			// 取硬盘状态信息，DRIVER_INFO为硬盘或分区信息
			int nRetLen = 0;
			int nBufSize = sizeof(IDE_INFO64) + 256*sizeof(DRIVER_INFO);
			char *pDiskBuf = new char[nBufSize];
			if (NULL == pDiskBuf)
			{
				return NET_SYSTEM_ERROR;
			}
			
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DISK_DRIVER, pDiskBuf, nBufSize, &nRetLen, waittime);
			if (nRet < 0)
			{
				delete[] pDiskBuf;
				return nRet;
			}
			
			int nTotalNum = ((IDE_INFO *)pDiskBuf)->ide_num;
			if (nTotalNum > 256 || nTotalNum < 0)
			{
				delete[] pDiskBuf;
				return NET_RETURN_DATA_ERROR;
			}

			HARD_DISK_STATE stuDiskState = {0};
			if (nRetLen >= sizeof(IDE_INFO) && (nTotalNum == (nRetLen-sizeof(IDE_INFO))/sizeof(DRIVER_INFO)))
			{
				IDE_INFO *pIDE_Info = (IDE_INFO *)pDiskBuf;
				stuDiskState.dwDiskNum = pIDE_Info->ide_num;

				for (int i = 0; i < stuDiskState.dwDiskNum; i++)
				{
					DRIVER_INFO *pDisk_Info = (DRIVER_INFO *)(pDiskBuf + sizeof(IDE_INFO) + i*sizeof(DRIVER_INFO));

					stuDiskState.stDisks[i].dwVolume = pDisk_Info->total_space;
					stuDiskState.stDisks[i].dwFreeSpace = pDisk_Info->remain_space;
					stuDiskState.stDisks[i].dwStatus = pDisk_Info->is_current;
					if (1 == pDisk_Info->error_flag)
					{
						stuDiskState.stDisks[i].dwStatus = 2;	// 坏硬盘
					}
					stuDiskState.stDisks[i].bDiskNum = (pDisk_Info->index & 0x0F);
					stuDiskState.stDisks[i].bSubareaNum = (pDisk_Info->index>>4) & 0x07;
					int nSubAreaNum = (pDisk_Info->index>>8) & 0xFF;
					if (nSubAreaNum > 0)
					{
						stuDiskState.stDisks[i].bSubareaNum = nSubAreaNum - 1;
					}
					stuDiskState.stDisks[i].bSignal = (pDisk_Info->index>>7) & 0x01;
				}
			}
			else if (nRetLen >= sizeof(IDE_INFO64) && (nTotalNum == (nRetLen-sizeof(IDE_INFO64))/sizeof(DRIVER_INFO)))
			{
				IDE_INFO64 *pIDE_Info = (IDE_INFO64 *)pDiskBuf;
				stuDiskState.dwDiskNum = pIDE_Info->ide_num;

				for (int i = 0; i < stuDiskState.dwDiskNum; i++)
				{
					DRIVER_INFO *pDisk_Info = (DRIVER_INFO *)(pDiskBuf + sizeof(IDE_INFO64) + i*sizeof(DRIVER_INFO));

					stuDiskState.stDisks[i].dwVolume = pDisk_Info->total_space;
					stuDiskState.stDisks[i].dwFreeSpace = pDisk_Info->remain_space;
					stuDiskState.stDisks[i].dwStatus = pDisk_Info->is_current;
					if (1 == pDisk_Info->error_flag)
					{
						stuDiskState.stDisks[i].dwStatus = 2;	// 坏硬盘
					}
					stuDiskState.stDisks[i].bDiskNum = (pDisk_Info->index & 0x0F);
					stuDiskState.stDisks[i].bSubareaNum = (pDisk_Info->index>>4) & 0x07;
					int nSubAreaNum = (pDisk_Info->index>>8) & 0xFF;
					if (nSubAreaNum > 0)
					{
						stuDiskState.stDisks[i].bSubareaNum = nSubAreaNum - 1;
					}
					stuDiskState.stDisks[i].bSignal = (pDisk_Info->index>>7) & 0x01;
				}
			}
			else
			{
				nRet = NET_RETURN_DATA_ERROR;
			}
			
			if (pDiskBuf != NULL)
			{
				delete[] pDiskBuf;
				pDiskBuf = NULL;
			}
			
			if (nBufLen >= sizeof(HARD_DISK_STATE)) 
			{
				memcpy(pBuf, &stuDiskState, sizeof(HARD_DISK_STATE));
				*pRetLen = sizeof(HARD_DISK_STATE);
			}
			else
			{
				if (stuDiskState.dwDiskNum > 32)
				{
					stuDiskState.dwDiskNum = 32;
				}
				memcpy(pBuf, &stuDiskState, nOldStructSize);
				*pRetLen = nOldStructSize;
			}

			return nRet;
		}
		break;
	case DEVSTATE_PROTOCAL_VER:
		{
			*(int*)pBuf = 1;
			*pRetLen = sizeof(int);
			device->get_info(device, dit_protocol_version, (int*)pBuf);
			return 0;
		}
		break;
	case DEVSTATE_TALK_ECTYPE:
		{
			DEV_TALK_FORMAT_LIST *pstTalkFormat = (DEV_TALK_FORMAT_LIST*)pBuf;
			if (nBufLen < sizeof(DEV_TALK_FORMAT_LIST))
			{
				return NET_INSUFFICIENT_BUFFER;
			}
			
			nRet = GetDevTalkFormatList(lLoginID, pstTalkFormat, waittime);
			if (nRet < 0)
			{
				return NET_EMPTY_LIST;
			}

			*pRetLen = sizeof(DEV_TALK_FORMAT_LIST);
			return 0;
		}
		break;
	case DEVSTATE_SD_CARD:
		{
			if (nBufLen < sizeof(SD_CARD_STATE)) 
			{
				return NET_ILLEGAL_PARAM;
			}
			
			SD_CARD_STATE stSDCardState = {0};
			
			//取SD卡状态信息
			int nLen = 0;
			int bufSize = sizeof(CARD_STORAGE_INFO)+sizeof(CARD_STORAGE_DEVICE_INFO)*32;			
			char cardbuf[sizeof(CARD_STORAGE_INFO)+sizeof(CARD_STORAGE_DEVICE_INFO)*32] = {0};
			
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_SD_CARD, cardbuf, bufSize, &nLen, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			if (nLen < sizeof(CARD_STORAGE_INFO))
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			CARD_STORAGE_INFO *csInfo = (CARD_STORAGE_INFO *)cardbuf;
			
			int pos = sizeof(CARD_STORAGE_INFO); //标志当前内存位置
			nLen -= sizeof(CARD_STORAGE_INFO);
			if (csInfo->DevNum < 0 || csInfo->DevNum > 32)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			stSDCardState.dwDiskNum = csInfo->DevNum;

			for(int i = 0; i < csInfo->DevNum; i ++)
			{
				if (nLen/sizeof(CARD_STORAGE_DEVICE_INFO) <= 0)
				{
					break;
				}
				CARD_STORAGE_DEVICE_INFO *csdInfo = (CARD_STORAGE_DEVICE_INFO *)(cardbuf + pos);
				
				stSDCardState.stDisks[i].dwVolume = csdInfo->TotalSpace;
				stSDCardState.stDisks[i].dwFreeSpace = csdInfo->RemainSpace;
				stSDCardState.stDisks[i].dwStatus = 1;
				pos += sizeof(CARD_STORAGE_DEVICE_INFO);
				nLen -= sizeof(CARD_STORAGE_DEVICE_INFO);
			}
			
			memcpy(pBuf, &stSDCardState, sizeof(SD_CARD_STATE));
			*pRetLen = sizeof(SD_CARD_STATE);
			
			return nRet;
		}
		break;
	case DEVSTATE_BURNING_DEV:
		{
			if (nBufLen < sizeof(DEV_BURNING_INFO)) 
			{
				return NET_ILLEGAL_PARAM;
			}
			
			DEV_BURNING_INFO stBurningDev = {0};
			
			//取刻录状态信息
			int nLen = 0;
			int bufSize = sizeof(BACKUP_DEVICE)*32;			
			char burnbuf[sizeof(BACKUP_DEVICE)*32] = {0};
			
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_OPTICS_STORAGE, burnbuf, bufSize, &nLen, waittime);
			if (nRet < 0)
			{
				return nRet;
			}
			
			stBurningDev.dwDevNum = nLen/sizeof(BACKUP_DEVICE);
			if (stBurningDev.dwDevNum > 32 || nLen%sizeof(BACKUP_DEVICE)>0)
			{
				return NET_RETURN_DATA_ERROR;
			}
			
			for(int i = 0; i < stBurningDev.dwDevNum; i ++)
			{
				BACKUP_DEVICE *curDev = (BACKUP_DEVICE*)(burnbuf+sizeof(BACKUP_DEVICE)*i);
				
				stBurningDev.stDevs[i].dwTotalSpace = curDev->capability;
				stBurningDev.stDevs[i].dwRemainSpace = curDev->remain;
				stBurningDev.stDevs[i].dwDriverType = curDev->type;
				stBurningDev.stDevs[i].dwBusType = curDev->bus;
				memcpy(stBurningDev.stDevs[i].dwDriverName, curDev->name, 32);
			}
			
			memcpy(pBuf, &stBurningDev, sizeof(DEV_BURNING_INFO));
			*pRetLen = sizeof(DEV_BURNING_INFO);
			
			return nRet;
		}
		break;
	case DEVSTATE_BURNING_PROGRESS:
		{
			if (nBufLen < sizeof(BURNING_PROGRESS))
			{
				return NET_INSUFFICIENT_BUFFER;
			}
			searchchannel.param = nDevID;
			statetype = 6;	//刻录进度
		}
		break;
	case DEVSTATE_PLATFORM:		//平台接入能力
		{
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_PLATFORM, pBuf, nBufLen, pRetLen, waittime);
			return nRet;
		}
		break;
	case DEVSTATE_CAMERA:		//查询摄像头属性信息
		{
			if (nBufLen < sizeof(DEVICE_CAMERA_INFO))
			{
				return NET_INSUFFICIENT_BUFFER;
			}
			
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_CAMERA, pBuf, nBufLen, pRetLen, waittime);
			return nRet;
		}
		break;
	case DEVSTATE_SOFTWARE:		//设备软件信息
		{
			DEVSOFTWARE_VERSION_INFO *pstDevVerInfo = (DEVSOFTWARE_VERSION_INFO*)pBuf;
			if (nBufLen < sizeof(DEVSOFTWARE_VERSION_INFO) || pstDevVerInfo == NULL)
			{
				return NET_INSUFFICIENT_BUFFER;
			}

			memset(pBuf, 0, nBufLen);
			int retlen = 0;
			SYSATTR_T stAT = {0};
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ATTR, (char *)&stAT, sizeof(SYSATTR_T), &retlen, waittime);
			if (nRet >= 0 && retlen == sizeof(SYSATTR_T))
			{
				//软件生成日期
				pstDevVerInfo->dwSoftwareBuildDate |= stAT.year;
				pstDevVerInfo->dwSoftwareBuildDate <<= 8;
				pstDevVerInfo->dwSoftwareBuildDate |= stAT.month;
				pstDevVerInfo->dwSoftwareBuildDate <<= 8;
				pstDevVerInfo->dwSoftwareBuildDate |= stAT.day;

				pstDevVerInfo->byDevType = device->device_type(device);	//设备类型

				device->get_info(device, dit_protocol_version, &pstDevVerInfo->nProtocalVer);
			}
			else
			{
				return NET_ERROR_GETCFG_SYSATTR;
			}

			retlen = 0;
			char szVersion[64] = {0};
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_VER, szVersion, 64, &retlen, waittime);
			if (nRet >= 0 && retlen > 0)
			{
				strcpy(pstDevVerInfo->szSoftWareVersion, szVersion);
			}
			
			retlen = 0;
			char szSer[64] = {0};
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ID, szSer, 64, &retlen, waittime);
			if (nRet >= 0 && retlen > 0)
			{
				strcpy(pstDevVerInfo->szDevSerialNo, szSer);
			}

			char devtypebuf[DEV_TYPE_STR_LEN] = {0};
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_TYPE, devtypebuf, DEV_TYPE_STR_LEN, &retlen, waittime);
			if (nRet >= 0 && retlen > 0)
			{
				strcpy(pstDevVerInfo->szDevType, devtypebuf);
			}
			
			*pRetLen = sizeof(DEVSOFTWARE_VERSION_INFO);
			return 0;
		}
		break;
	case DEVSTATE_LANGUAGE:		//设备语言支持信息 add by cqs
		{
			if (nBufLen < sizeof(DEV_LANGUAGE_INFO)) 
			{
				return NET_ILLEGAL_PARAM;
			}
			
			char buf[1024] = {0};
			int nRetLen = 0 ;
			int nLangTypeNum = 0;
			
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_Language, buf, 1024, &nRetLen, waittime);
			if (nRet >= 0 && nRetLen > 0) 
			{
				CStrParse parser;
				parser.setSpliter("&&");
				bool bSuccess = parser.Parse(buf);
				if (!bSuccess) 
				{
					return NET_ERROR;
				}
				
				DEV_LANGUAGE_INFO stuDevLanguage = {0};

				std::string info = parser.getWord();
				CStrParse subparse;
				subparse.setSpliter("|");
				bSuccess = subparse.Parse(info);
				if (!bSuccess) 
				{
					return NET_ERROR;
				}
				
				for(int i = 0; i < subparse.Size(); i++)
				{
					char LanguageType[50][20]={"English","SimpChinese","TradChinese","Italian",
									"Spanish","Japanese","Russian","French","German","Portugal","Turkey",
									"Poland","Romanian","Hungarian","Finnish","Estonian","Korean","Farsi",
									"Dansk","Czechish","Bulgaria","Slovakian","Slovenia",
									"Croatian", "Dutch", "Greek", "Ukrainian", "Swedish",
									"Serbian", "Vietnamese", "Lithuanian", "Filipino", "Arabic",
									"Catalan", "Latvian", "Thai", "Hebrew", "Bosnian"};

					for(int j = 0; j < 50; j++)
					{
						if (0 == _stricmp(subparse.getWord(i).c_str(), LanguageType[j]) && 0 != _stricmp(subparse.getWord(i).c_str(), ""))
						{
							stuDevLanguage.byLanguageType[nLangTypeNum] = j;
							nLangTypeNum ++;
							break;
						}
					}
				}

				stuDevLanguage.dwLanguageNum = nLangTypeNum;
				memcpy(pBuf,&stuDevLanguage, sizeof(DEV_LANGUAGE_INFO));
				*pRetLen = sizeof(DEV_LANGUAGE_INFO);
				
				return 0;
			}
			else
			{
				return -1;
			}
		}	//End: cqs(10842)	
		break;
	case DEVSTATE_DSP:
		{
			DEV_DSP_ENCODECAP *pstDevDspCap = (DEV_DSP_ENCODECAP*)pBuf;
			if (nBufLen < sizeof(DEV_DSP_ENCODECAP))
			{
				return NET_INSUFFICIENT_BUFFER;
			}

			memset(pBuf, 0, nBufLen);

			//	此功能解码器除外
			if (GET_DEVICE_TYPE(lLoginID) != PRODUCT_NVD_SERIAL)
			{
				int retlen = 0;
				
				//get resolution and encode-type mask
				char maskbuf[8] = {0};
				nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSP_MASK, 0, maskbuf, 8, &retlen, waittime);
				if (nRet >= 0 && retlen == 8) 
				{
					pstDevDspCap->dwImageSizeMask = *(DWORD *)(maskbuf + 4);
					pstDevDspCap->dwEncodeModeMask = *(DWORD *)(maskbuf);
				}
				else
				{
					pstDevDspCap->dwImageSizeMask = 0;
					pstDevDspCap->dwEncodeModeMask = 0;
					return NET_ERROR_GETCFG_DSPCAP;
				}
				
				//辅码流能力
				int bufLen = 64*sizeof(CAPTURE_EXT_STREAM);
				bufLen = bufLen<sizeof(CAPTURE_EXT_STREAM_NEW) ? sizeof(CAPTURE_EXT_STREAM_NEW) : bufLen;
				char* capBuf = new char[bufLen];
				if (NULL == capBuf)
				{
					return -1;
				}
				memset(capBuf, 0, bufLen);
				nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSP_MASK, 1, capBuf, bufLen, &retlen, waittime);
				if (nRet >= 0 && retlen > 8) 
				{
					CAPTURE_EXT_STREAM* stExt = (CAPTURE_EXT_STREAM*)capBuf;
					pstDevDspCap->dwStreamCap = stExt->ExtraStream;
					memcpy(pstDevDspCap->dwImageSizeMask_Assi, stExt->CaptureSizeMask, sizeof(pstDevDspCap->dwImageSizeMask_Assi));
				}
				//同时支持新的辅码流能力协议，配置版本1
				else if (nRet >= 0 && retlen == sizeof(CAPTURE_EXT_STREAM_NEW))
				{
					CAPTURE_EXT_STREAM_NEW* stExt = (CAPTURE_EXT_STREAM_NEW*)capBuf;
					pstDevDspCap->dwStreamCap = stExt->ExtraStream;
					memcpy(pstDevDspCap->dwImageSizeMask_Assi, stExt->CaptureSizeMask, sizeof(pstDevDspCap->dwImageSizeMask_Assi));
				}
				else
				{
					pstDevDspCap->dwStreamCap = 0x01;
					nRet = 0;
					//return NET_ERROR_GETCFG_DSPCAP;
				}
				delete[] capBuf;
				
				//DSP能力
				CAPTURE_DSPINFO stDsp = {0};
				nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSPCAP, 0, (char*)&stDsp, sizeof(CAPTURE_DSPINFO), &retlen, waittime);
				if (nRet >= 0 && retlen == sizeof(stDsp)) 
				{
					pstDevDspCap->dwMaxEncodePower = stDsp.nMaxEncodePower;
					pstDevDspCap->wMaxSupportChannel = stDsp.nMaxSupportChannel;
					pstDevDspCap->wChannelMaxSetSync = stDsp.bChannelMaxSetSync;
				}
				else
				{
					//默认
					pstDevDspCap->dwMaxEncodePower = 0xFFFFFFFF;
					pstDevDspCap->wMaxSupportChannel = 1;
					pstDevDspCap->wChannelMaxSetSync = 0;
					nRet = 0;
					//return NET_ERROR_GETCFG_DSPCAP;
				}

				//最大采集帧率
				Prevideo_Capture stuVideoCapture[32] = {0};
				retlen = 0;
				nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_VIDEO_CAPTURE, (char*)stuVideoCapture, 32*sizeof(Prevideo_Capture), &retlen, waittime);
				if (nRet >= 0 && retlen > 0 && (0 == retlen%sizeof(Prevideo_Capture)))
				{
					int nCount = retlen/sizeof(Prevideo_Capture);
					for (int i = 0; i < nCount; i++)
					{
						BYTE bImageSize = stuVideoCapture[i].iCifCaps;
						pstDevDspCap->bMaxFrameOfImageSize[bImageSize] = stuVideoCapture[i].iFrameCaps;
					}
				}
				
				//编码能力算法标识
				SYSATTR_VIDEO_CAPS_T stuVideoAttr = {0};
				retlen = 0;
				nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_VIDEO_ATTR, (char*)&stuVideoAttr, sizeof(SYSATTR_VIDEO_CAPS_T), &retlen, waittime);
				if (nRet >= 0 && retlen == sizeof(SYSATTR_VIDEO_CAPS_T))
				{
					pstDevDspCap->bEncodeCap = stuVideoAttr.iEncodeCap;
				}
			}
				
			//pstDevDspCap->dwVideoStandardMask = 0x03; //NSP, default 0x03
			//视频制式能力现在由设备传过来了add by cqs 2009.2.19
			int nRetLen = 0;
			int nRet = -1;
			char buffer[512] = {0};
			nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_VIDEOFORMAT_INFO, NULL, buffer, 512, &nRetLen, waittime);
			if (nRet >= 0 && nRetLen > 0)
			{
				char szValue[64] = {0};
				char *p = buffer;
				while (p)
				{
					p = GetProtocolValue(p, "SupportItem:", "\r\n", szValue, 64);
					if (0 == _stricmp(szValue, "PAL"))
					{
						pstDevDspCap->dwVideoStandardMask|=0x01;
					}
					else if(0 == _stricmp(szValue, "NTSC"))
					{
						pstDevDspCap->dwVideoStandardMask|=0x02;
					}
					else
					{
						pstDevDspCap->dwVideoStandardMask = 0x03; //NSP, default 0x03
						break;
					}
				}
			}
			else
			{
				pstDevDspCap->dwVideoStandardMask = 0x03; //NSP, default 0x03
			}
			
			*pRetLen = sizeof(DEV_DSP_ENCODECAP);
			return 0;
		}
		break;
	case DEVSTATE_OEM:
		{
			DEV_OEM_INFO *pstDevOEM = (DEV_OEM_INFO*)pBuf;
			if (nBufLen < sizeof(DEV_OEM_INFO))
			{
				return NET_INSUFFICIENT_BUFFER;
			}
			
			memset(pBuf, 0, nBufLen);
			int retlen = 0;
			
			char buffer[1024] = {0};
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_OEM_INFO,buffer, 1024, &retlen, waittime);
			if (nRet >= 0 && retlen > 0)
			{
				CStrParse parser;
				parser.setSpliter("::");
				bool bSuccess = parser.Parse(buffer);
				if (!bSuccess) 
				{
					return NET_ERROR;
				}

				if (parser.Size() < 2)
				{
					return NET_ERROR;
				}
				
				strcpy(pstDevOEM->szVendor, parser.getWord(0).c_str());
				strcpy(pstDevOEM->szType, parser.getWord(1).c_str());

				*pRetLen = sizeof(DEV_OEM_INFO);
			}
			return nRet;
		}
		break;
	case DEVSTATE_NET:
		{
			DEV_TOTAL_NET_STATE *pstNetState = (DEV_TOTAL_NET_STATE*)pBuf;
			if (nBufLen < sizeof(DEV_TOTAL_NET_STATE))
			{
				return NET_INSUFFICIENT_BUFFER;
			}
			
			nRet = QueryNetUserState(lLoginID, pstNetState, waittime);
			if (nRet < 0)
			{
				return NET_EMPTY_LIST;
			}
			
			*pRetLen = sizeof(DEV_TOTAL_NET_STATE);
			return 0;
		}
		break;
	case DEVSTATE_TYPE:
		{
			if(NULL ==pBuf)
			{
				return NET_ERROR;
			}
			char szBuf[256] ={0};
			device->get_info(device,dit_dev_type,szBuf);
			if(strlen(szBuf) > nBufLen)
			{
				return NET_ERROR;
			}
			*pRetLen = strlen(pBuf);
			strcpy(pBuf,szBuf);
			return 0;
		}
		break;
	case DEVSTATE_SNAP:
		{
			if(NULL == pBuf || nBufLen < sizeof(SNAP_ATTR_EN))
			{
				return NET_ERROR;
			}

			char bufSnap[2048] = {0};
			int nReturnLen = 0;
			int nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_SNAP, bufSnap, 2048, &nReturnLen, waittime);
			if (nRet < 0 || nReturnLen <= 0)
			{
				return NET_ERROR;
			}
			
			SNAP_ATTR_EN stuSnap;
			memset(&stuSnap, 0, sizeof(SNAP_ATTR_EN));
			CStrParse parseChannel;
			CStrParse parseAttr;
			CStrParse parseValue;
			parseChannel.setSpliter("&");
			parseAttr.setSpliter("::");
			parseValue.setSpliter(":");
					
			parseChannel.Parse(bufSnap);
			int nChannel = parseChannel.Size();
			if(nChannel > 2)
			{
				stuSnap.nChannelCount = nChannel - 2;
			}
			else
			{
				return -1;
			}
			//找出"SNAP"
			if(_stricmp(parseChannel.getWord(0).c_str(), "SNAP") != 0)
			{ 
				return -1;
			}
			for(int i = 2;i < nChannel; i++)
			{
				parseAttr.Parse(parseChannel.getWord(i));
				int nAttr = parseAttr.Size();
				for(int j = 0; j < nAttr; j++)
				{
					parseValue.Parse(parseAttr.getWord(j));
					int nValue = parseValue.Size();
					if(_stricmp(parseValue.getWord(0).c_str(), "SIZE") == 0)
					{
						DWORD dwTmp = 0x0001;
						for(int k = 1; k < nValue; k++)
						{
							stuSnap.stuSnap[i-2].dwVideoStandardMask |= dwTmp<<parseValue.getValue(k);
						}
						
					}
					else if(_stricmp(parseValue.getWord(0).c_str(), "FREQUENCE") == 0)
					{
						for(int k = 1; k < nValue; k++)
						{
							stuSnap.stuSnap[i-2].Frames[k-1] = parseValue.getValue(k);
						}
						stuSnap.stuSnap[i-2].nFramesCount = nValue-1;
					}
					else if(_stricmp(parseValue.getWord(0).c_str(),"MODE")==0)
					{
						for(int k = 1; k < nValue; k++)
						{
							stuSnap.stuSnap[i-2].SnapMode[k-1] = parseValue.getValue(k);
						}
						stuSnap.stuSnap[i-2].nSnapModeCount = nValue-1;
					}
					else if(_stricmp(parseValue.getWord(0).c_str(),"FORMAT")==0)
					{
						for(int k = 1; k < nValue; k++)
						{
							stuSnap.stuSnap[i-2].PictureFormat[k-1] = parseValue.getValue(k);
						}
						stuSnap.stuSnap[i-2].nPicFormatCount = nValue-1;
					}
					else if(_stricmp(parseValue.getWord(0).c_str(),"QUALITY")==0)
					{
						for(int k = 1; k < nValue; k++)
						{
							stuSnap.stuSnap[i-2].PictureQuality[k-1] = parseValue.getValue(k);
						} 
						stuSnap.stuSnap[i-2].nPicQualityCount = nValue-1;
					}
					else
					{
						if(nValue == 1)
						{
							stuSnap.stuSnap[i-2].nChannelNum = parseValue.getValue(0);
						}
						else
						{
							;
						}
					}
				}
			}
			memcpy(pBuf, &stuSnap, sizeof(SNAP_ATTR_EN));
			*pRetLen = sizeof(SNAP_ATTR_EN);
			return 0;
		}
		break;
	case DEVSTATE_RECORD_TIME:
		{
			DEV_DISK_RECORD_INFO *pDiskRecordInfo = (DEV_DISK_RECORD_INFO *)pBuf;
			if (pDiskRecordInfo == NULL || nBufLen < sizeof(DEV_DISK_RECORD_INFO)) 
			{
				return NET_ILLEGAL_PARAM;
			}

			// 取硬盘信息
			int nRetLen = 0;
			int nBufSize = sizeof(IDE_INFO64) + 256*sizeof(DRIVER_INFO);
			char *pDiskBuf = new char[nBufSize];
			if (NULL == pDiskBuf)
			{
				return NET_SYSTEM_ERROR;
			}

			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DISK_DRIVER, pDiskBuf, nBufSize, &nRetLen, waittime);
			if (nRet < 0)
			{
				delete[] pDiskBuf;
				return nRet;
			}
			
			int nTotalNum = ((IDE_INFO *)pDiskBuf)->ide_num;
			if (nTotalNum > 256 || nTotalNum < 0)
			{
				delete[] pDiskBuf;
				return NET_RETURN_DATA_ERROR;
			}

			FRAME_TIME stuFarTime = {0};
			FRAME_TIME stuNearTime = {0};

			if (nRetLen >= sizeof(IDE_INFO) && (nTotalNum == (nRetLen-sizeof(IDE_INFO))/sizeof(DRIVER_INFO)))
			{
				IDE_INFO *pIDE_Info = (IDE_INFO *)pDiskBuf;
				for (int i = 0; i < pIDE_Info->ide_num; i++)
				{
					DRIVER_INFO *pDisk_Info = (DRIVER_INFO *)(pDiskBuf + sizeof(IDE_INFO) + i*sizeof(DRIVER_INFO));
					if (i == 0)
					{
						stuFarTime = pDisk_Info->start_time1;
						stuNearTime = pDisk_Info->end_time1;
					}

					if (stuFarTime >= pDisk_Info->start_time1)
					{
						stuFarTime = pDisk_Info->start_time1;
					}
					if (pDisk_Info->end_time1 >= stuNearTime)
					{
						stuNearTime = pDisk_Info->end_time1;
					}

					// 有第二段录象，取最早时间
					if (pDisk_Info->two_part)
					{
						if (stuFarTime >= pDisk_Info->start_time2)
						{
							stuFarTime = pDisk_Info->start_time2;
						}
						if (pDisk_Info->end_time2 >= stuNearTime)
						{
							stuNearTime = pDisk_Info->end_time2;
						}
					}
				}
			}
			else if (nRetLen >= sizeof(IDE_INFO64) && (nTotalNum == (nRetLen-sizeof(IDE_INFO64))/sizeof(DRIVER_INFO)))// && nProVer == 0)
			{
				IDE_INFO64 *pIDE_Info = (IDE_INFO64 *)pDiskBuf;
		
				for (int i = 0; i < pIDE_Info->ide_num; i++)
				{
					DRIVER_INFO *pDisk_Info = (DRIVER_INFO *)(pDiskBuf + sizeof(IDE_INFO64) + i*sizeof(DRIVER_INFO));
					if (i == 0)
					{
						stuFarTime = pDisk_Info->start_time1;
						stuNearTime = pDisk_Info->end_time1;
					}
					
					if (stuFarTime >= pDisk_Info->start_time1)
					{
						stuFarTime = pDisk_Info->start_time1;
					}
					if (pDisk_Info->end_time1 >= stuNearTime)
					{
						stuNearTime = pDisk_Info->end_time1;
					}

					// 有第二段录象，取最早时间
					if (pDisk_Info->two_part)
					{
						if (stuFarTime >= pDisk_Info->start_time2)
						{
							stuFarTime = pDisk_Info->start_time2;
						}
						if (pDisk_Info->end_time2 >= stuNearTime)
						{
							stuNearTime = pDisk_Info->end_time2;
						}
					}
				}
			}
			else
			{
				nRet = NET_RETURN_DATA_ERROR;
			}

			if (pDiskBuf != NULL)
			{
				delete[] pDiskBuf;
				pDiskBuf = NULL;
			}
			
			pDiskRecordInfo->stuBeginTime.dwYear = stuFarTime.year + 2000;
			pDiskRecordInfo->stuBeginTime.dwMonth = stuFarTime.month;
			pDiskRecordInfo->stuBeginTime.dwDay = stuFarTime.day;
			pDiskRecordInfo->stuBeginTime.dwHour = stuFarTime.hour;
			pDiskRecordInfo->stuBeginTime.dwMinute = stuFarTime.minute;
			pDiskRecordInfo->stuBeginTime.dwSecond = stuFarTime.second;
			pDiskRecordInfo->stuEndTime.dwYear = stuNearTime.year + 2000;
			pDiskRecordInfo->stuEndTime.dwMonth = stuNearTime.month;
			pDiskRecordInfo->stuEndTime.dwDay = stuNearTime.day;
			pDiskRecordInfo->stuEndTime.dwHour = stuNearTime.hour;
			pDiskRecordInfo->stuEndTime.dwMinute = stuNearTime.minute;
			pDiskRecordInfo->stuEndTime.dwSecond = stuNearTime.second;
			*pRetLen = sizeof(DEV_DISK_RECORD_INFO);

			return nRet;
		}
		break;	
	case DEVSTATE_BURNING_ATTACH:
		{
			BOOL *bAttach = (BOOL *)pBuf;
			if(NULL == bAttach)
			{
				return -1;
			}
			*pRetLen = 0;
			*bAttach = 0;
			
			char szBuf[1024] = {0};
			int nRetLen = 0;
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_BURN_ATTACH, NULL, szBuf, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				char szValue[64] = {0};
				char *p = GetProtocolValue(szBuf, "EnableExBackup:", "\r\n", szValue, 64);
				if (p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				*bAttach = atoi(szValue);
				*pRetLen = sizeof(BOOL);
				return 1;
			}
			else
			{
				return -1;
			}
		}
		break;

	case DEVSTATE_NET_RSSI://无线网络信号强度
		{
			if (nBufLen < sizeof(DEV_WIRELESS_RSS_INFO))
			{
				return NET_INSUFFICIENT_BUFFER;
			}
			statetype = 7;	//无线网络信号强度
		}
		break;

	case DEVSTATE_BACKUP_DEV: // 查询备份设备列表
		{
			if (NULL== pBuf || nBufLen < sizeof(DEV_BACKUP_LIST))
			{
				return NET_ERROR;
			}

			*pRetLen = 0;
			
			char szBuf[4096] = {0};
			int nRetLen = 0;
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_QUERY_BACKUP_DEV, NULL, szBuf, 4096, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				char szValue[32] = {0};
				char *p = szBuf;
				
				LPDEV_BACKUP_LIST lpBackupList = (LPDEV_BACKUP_LIST)pBuf;
				memset(pBuf,0,sizeof(DEV_BACKUP_LIST));
				
				
				lpBackupList->nBackupDevNum = 0;

				for (int i =0; p != NULL; i++)
				{
					char szDeviceName[32] = {0};
					sprintf(szDeviceName,"DeviceName[%d]:",i);

					p = GetProtocolValue(szBuf, szDeviceName, "\r\n", szValue, 32);
					if (p != NULL)
					{
						strncpy(lpBackupList->szBackupDevNames[i],szValue,32);
						lpBackupList->nBackupDevNum++;
					}

				}

				*pRetLen = sizeof(DEV_BACKUP_LIST);
				return 1;
			}
			else
			{
				return -1;
			}
			
		}
		break;
	case DEVSTATE_BACKUP_DEV_INFO: // 查询备份设备详细信息
		{
			if (NULL== pBuf || nBufLen < sizeof(DEV_BACKUP_INFO))
			{
				return NET_ERROR;
			}
			LPDEV_BACKUP_INFO lpBackupInfo = (LPDEV_BACKUP_INFO)pBuf;
			if (lpBackupInfo->szName[0] == '\0')	// 无效的参数
			{
				return NET_ILLEGAL_PARAM;
			}
			

			*pRetLen = 0;
			char szDeviceName[32] = {0};
			char szBuf[1024] = {0};
			int nRetLen = 0;
			strncpy(szDeviceName,lpBackupInfo->szName,32);
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_QUERY_BACKUP_DEV_INFO, szDeviceName, szBuf, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				
				memset(pBuf,0,sizeof(DEV_BACKUP_INFO));
				strncpy(lpBackupInfo->szName,szDeviceName,32);
				
				char szValue[32] = {0};
				// 取介质类型
				char *p = GetProtocolValue(szBuf,"type:","\r\n",szValue,32);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				
	
				lpBackupInfo->byType = atoi(szValue);
				if (lpBackupInfo->byType > BT_CDRW)
				{
					return NET_RETURN_DATA_ERROR;
				}

				//取接口类型
				p = GetProtocolValue(szBuf,"bus:","\r\n",szValue,32);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				lpBackupInfo->byBus = atoi(szValue);
				if (lpBackupInfo->byBus > BB_ESATA)
				{
					return NET_RETURN_DATA_ERROR;
				}
				//取总容量
				p = GetProtocolValue(szBuf,"capability:","\r\n",szValue,32);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				lpBackupInfo->nCapability = atoi(szValue);
				//取剩余容量
				p = GetProtocolValue(szBuf,"remain:","\r\n",szValue,32);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				lpBackupInfo->nRemain = atoi(szValue);
				//取远程备份目录
				char szDirectory[128] = {0};
				p = GetProtocolValue(szBuf,"directory:","\r\n",szDirectory,128);
				if (p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				strncpy(lpBackupInfo->szDirectory,szDirectory,128);

				*pRetLen = sizeof(DEV_BACKUP_INFO);
				return 1;
			}
			else
			{
				return -1;
			}
			
		}
		break;
	case DEVSTATE_BACKUP_FEEDBACK: // 备份进度反馈
		{
			if (NULL== pBuf || nBufLen < sizeof(DEV_BACKUP_FEEDBACK))
			{
				return NET_ERROR;
			}
			LPDEV_BACKUP_FEEDBACK lpBackupInfo = (LPDEV_BACKUP_FEEDBACK)pBuf;
			if (lpBackupInfo->szName[0] == '\0')	//无效的参数
			{
				return NET_ILLEGAL_PARAM;
			}
			

			*pRetLen = 0;
			char szDeviceName[32] = {0};
			char szBuf[1024] = {0};
			int nRetLen = 0;
			strncpy(szDeviceName,lpBackupInfo->szName,32);
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_BACKUP_FEEDBACK, szDeviceName, szBuf, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				
				memset(pBuf,0,sizeof(DEV_BACKUP_FEEDBACK));
				strncpy(lpBackupInfo->szName,szDeviceName,32);
				
				char szValue[32] = {0};
				//取总容量
				char *p = GetProtocolValue(szBuf,"Capability:","\r\n",szValue,32);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				lpBackupInfo->nCapability = atoi(szValue);
				//取当前备份剩余容量
				p = GetProtocolValue(szBuf,"Remain:","\r\n",szValue,32);
				if(p == NULL)
				{
					return NET_RETURN_DATA_ERROR;
				}
				lpBackupInfo->nRemain = atoi(szValue);

				*pRetLen = sizeof(DEV_BACKUP_FEEDBACK);
				return 1;
			}
			else
			{
				return -1;
			}
			
		}
		break;
	case DEVSTATE_ATM_QUERY_TRADE: // 查询ATM设备交易类型
		{
			if (NULL== pBuf || nBufLen < sizeof(ATM_QUERY_TRADE))
			{
				return NET_ERROR;
			}
			
			*pRetLen = 0;
			char *szBuf = new char[256 * 1024]; 
			if (szBuf == NULL)
			{
				return NET_SYSTEM_ERROR;
			}
			memset(szBuf, 0, 256 * 1024);
			int nRetLen = 0;
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_ATM_TRADE_TYPE, NULL, szBuf, 256 * 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				char szValue[64] = {0};
				char *p = szBuf;
				int i;
				
				LPATM_QUERY_TRADE lpTradeList = (LPATM_QUERY_TRADE)pBuf;
				memset(pBuf,0,sizeof(ATM_QUERY_TRADE));
				
				//解析交易类型
				lpTradeList->nTradeTypeNum = 0;
				lpTradeList->nExceptionTypeNum = 0;

				
				for (i =0; p != NULL; i++)
				{
					p = GetProtocolValue(p, "SupportTradeType:", "\r\n", szValue, 64);
					if (p != NULL)
					{
						Change_Utf8_Assic((unsigned char*)szValue,lpTradeList->szSupportTradeType[i]);
					//	memcpy(lpTradeList->szSupportTradeType[i],szValue,64);
						lpTradeList->nTradeTypeNum++;
					}
					
				}
				p = szBuf;
				for (i =0; p != NULL; i++)
				{			
					p = GetProtocolValue(p, "SupportExceptionType:", "\r\n", szValue, 64);
					if (p != NULL)
					{
						Change_Utf8_Assic((unsigned char*)szValue,lpTradeList->szSupportExceptionType[i]);
						//memcpy(lpTradeList->szSupportExceptionType[i],szValue,64);
						lpTradeList->nExceptionTypeNum++;
					}
					
				}
				
				*pRetLen = sizeof(ATM_QUERY_TRADE);
				if (szBuf != NULL)
				{
					delete [] szBuf;
					szBuf = NULL;
				}
				return 1;
			}
			else
			{
				if (szBuf != NULL)
				{
					delete [] szBuf;
					szBuf = NULL;
				}
				return -1;
			}
			
		}
		break;
	case DEVSTATE_SIP:
		{
			if (NULL== pBuf || nBufLen < sizeof(DEV_SIP_STATE))
			{
				return NET_ERROR;
			}
			
			DEV_SIP_STATE* pSIPState = (DEV_SIP_STATE*)pBuf;

			*pRetLen = 0;
			char szSIPState[1024] = {0};
			int nRetLen = 0;
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLoginID, AFK_REQUEST_GET_SIP_STATE, NULL, szSIPState, 1024, &nRetLen, waittime);
			if(nRet >= 0 && nRetLen > 0)
			{
				memset(pSIPState, 0, sizeof(DEV_SIP_STATE));

				char szValue[64] = {0};
				char* p = GetProtocolValue(szSIPState, "RegisteredState:", "\r\n", szValue, 64);
				if(NULL == p)
				{
					return -1;
				}

				pSIPState->bOnline = TRUE;
				if(_stricmp("offline", szValue) == 0)
				{
					pSIPState->bOnline = FALSE;
				}
				*pRetLen = sizeof(DEV_SIP_STATE);
				return 1;
			}
			else
			{
				return -1;
			}

		}
		break;
	default:
		return NET_ILLEGAL_PARAM;
	}
	
	searchchannel.type = configtype;
	searchchannel.statetype = statetype;
	
    *pRetLen = 0;
	
    receivedata_s receivedata;// = {0};
    receivedata.data = pBuf;
    receivedata.datalen = pRetLen;
    receivedata.maxlen = nBufLen;
	//receivedata.hRecEvt = m_hRecEvent;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }
	
	receivedata.reserved[0] = nType;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
			ResetEventEx(receivedata.hRecEvt);
			bool b = pchannel->close(pchannel);
			if (!b)
			{
				nRet = NET_CLOSE_CHANNEL_ERROR;
			}
			else
			{
				if (dwRet == WAIT_OBJECT_0)
				{    
					if (receivedata.result == -1)
					{
						nRet = NET_DEV_VER_NOMATCH;
					}
					else
					{
						nRet = 0;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;	
				}
			} 
        } // if (waittime != 0)
        else
        {
            nRet = 0;
        }
    }//if (pchannel)
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int CDevConfig::SetupChannelOsdString(LONG lLoginID, int nChannelNo, CHANNEL_OSDSTRING* struOsdString, int nbuflen)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }
	
	if (NULL == struOsdString || nbuflen != sizeof(CHANNEL_OSDSTRING) 
		||nChannelNo < 0 || nChannelNo >= MAX_CHANNUM)
	{
		return NET_ILLEGAL_PARAM;
	}
	//字符格式转换	
	char *pUTFData = new char[50];
	if (!pUTFData)
	{
		return NET_SYSTEM_ERROR;
	}
	for(int i = 0; i < MAX_STRING_LINE_LEN; i++)
	{
		memset(pUTFData, 0 , 50);
		if (strlen(struOsdString->szStrings[i]) > 0)
		{
			Change_Assic_UTF8(struOsdString->szStrings[i], MAX_PER_STRING_LEN, pUTFData, 50);
			memcpy(struOsdString->szStrings[i], pUTFData, MAX_PER_STRING_LEN);
		}
	}
	
	int nRet = NET_ERROR;
	
    afk_device_s *device = (afk_device_s*)lLoginID;
	
    afk_config_channel_param_s configchannel = {0};
    configchannel.no = nChannelNo;
    configchannel.type = AFK_CHANNEL_CONFIG_CHANNEL_OSD;		
	configchannel.param[0] = struOsdString->bEnable?1:0;
	configchannel.param[1] = struOsdString->dwPosition[0];
	configchannel.param[2] = struOsdString->dwPosition[1];
	configchannel.param[3] = struOsdString->dwPosition[2];
	configchannel.param[4] = struOsdString->dwPosition[3];
	configchannel.param[5] = struOsdString->dwPosition[4];
	configchannel.param[6] = struOsdString->dwPosition[5];
    configchannel.configinfo.configbuf = (unsigned char*)struOsdString->szStrings;
    configchannel.configinfo.buflen = MAX_STRING_LINE_LEN * MAX_PER_STRING_LEN;
    configchannel.base.udata = 0;
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
        pchannel->close(pchannel);
        nRet = 0;
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

	delete[] pUTFData;
    return nRet;
}

int __stdcall QueryPlatFormFunc(
	afk_handle_t object,	/* 数据提供者 */
	unsigned char *data,	/* 数据体 */
	unsigned int datalen,	/* 数据长度 */
	void *param,			/* 回调参数 */
	void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata)
    {
        return -1;
    }

	if (!receivedata->datalen || !receivedata->data || false == receivedata->addRef())
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}
	
	*receivedata->datalen = datalen;
	//缓冲区不够大
	if (datalen > (unsigned int)receivedata->maxlen)
	{
		SetEventEx(receivedata->hRecEvt);
		return -1;
	}
	
	memcpy(receivedata->data, data, datalen);
	receivedata->result = 0;
	SetEventEx(receivedata->hRecEvt);
	
	return 1;
}

int CDevConfig::GetPlatFormInfo(LONG lLoginID, DWORD dwCommand, int nSubCommand, int nParam, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned,int waittime)
{
	if (lpOutBuffer == NULL || lpBytesReturned == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	*lpBytesReturned = 0;
	memset(lpOutBuffer, 0, dwOutBufferSize);

	afk_device_s* device = (afk_device_s*)lLoginID;

	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int nRet = -1;
	int nRetLen = 0;

    afk_search_channel_param_s searchchannel = {0};
    searchchannel.base.func = QueryPlatFormFunc;
	searchchannel.type = AFK_CHANNEL_SEARCH_CONFIG;
	searchchannel.subtype = dwCommand;
	searchchannel.param = nSubCommand;
	
    receivedata_s receivedata;
    receivedata.data = (char*)lpOutBuffer;
    receivedata.datalen = (int*)lpBytesReturned;
    receivedata.maxlen = dwOutBufferSize;
    receivedata.result = -1;
    if (waittime == 0)
    {
        searchchannel.base.udata = 0;
    }
    else
    {
        searchchannel.base.udata = &receivedata;
    }
	
    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
    if (pchannel)
    {
        if (waittime != 0)
        {
            DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, waittime);
			ResetEventEx(receivedata.hRecEvt);
			bool b = pchannel->close(pchannel);
			if (!b)
			{
				nRet = NET_CLOSE_CHANNEL_ERROR;
			}
			else
			{
				if (dwRet == WAIT_OBJECT_0)
				{    
					if (receivedata.result == -1)
					{
						nRet = NET_DEV_VER_NOMATCH;
					}
					else
					{
						nRet = 0;
					}
				}
				else
				{
					nRet = NET_NETWORK_ERROR;	
				}
			}
        }
        else
        {
            nRet = 0;
        }
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

int CDevConfig::SetPlatFormInfo(LONG lLoginID, DWORD dwCommand, int nSubCommand, int nParam, LPVOID lpInBuffer, DWORD dwInBufferSize, int waittime)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (dwInBufferSize <= 0 || !lpInBuffer)
	{
		return NET_ILLEGAL_PARAM;
	}

	int nRet = -1;

    afk_device_s *device = (afk_device_s*)lLoginID;
    afk_config_channel_param_s configchannel = {0};
	
	int setupResult = -1;
	receivedata_s receivedata;
    receivedata.data = (char *)&setupResult;
    receivedata.datalen = 0;
    receivedata.maxlen = sizeof(int);
    receivedata.result = -1;

	configchannel.type = AFK_CHANNEL_CONFIG_CONFIG;
	configchannel.subtype = dwCommand;
    configchannel.configinfo.configbuf = (unsigned char*)lpInBuffer;
    configchannel.configinfo.buflen = dwInBufferSize;
    configchannel.base.udata = (void *)&receivedata;
	configchannel.base.func = SetupConfigFunc;
	configchannel.param[0] = nSubCommand;

    afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
        AFK_CHANNEL_TYPE_CONFIG, &configchannel);
    if (pchannel)
    {
		nRet = 0;
        if (!pchannel->close(pchannel))
		{
			nRet = NET_CLOSE_CHANNEL_ERROR;
		}
    }
	else
	{
		nRet = NET_OPEN_CHANNEL_ERROR;
	}

    return nRet;
}

//Add: by cqs(10842) 2008-5-9
/*  查询设备能力集 */
int	CDevConfig::GetDevFunctionInfo(LONG lLoginID, int nType, char *pBuf, int nBufLen, int *pRetLen, int waittime)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
    {
        return NET_INVALID_HANDLE;
    }

	if (!pBuf || !pRetLen)
	{
		return NET_ILLEGAL_PARAM;		
	}

	*pRetLen = 0;
	
	int nRet =-1;
	int nRetLen = 0;
	afk_device_s *device = (afk_device_s*)lLoginID;

	switch(nType)
	{
	case ABILITY_CARD_QUERY:      //是否支持卡号查询
		{
			if (nBufLen < sizeof(CARD_QUERY_EN)) 
			{
				return NET_ILLEGAL_PARAM;
			}
			
			CARD_QUERY_EN CardQuery_State = {0};

			device->get_info(device, dit_card_query, &CardQuery_State.IsCardQueryEnable);
			
			memcpy(pBuf,&CardQuery_State,sizeof(CARD_QUERY_EN));

			*pRetLen = sizeof(CARD_QUERY_EN);
			
			return 0;
		}
		break;
	case ABILITY_DEVALL_INFO :	/* 设备功能列表*/
		{
			if (nBufLen < sizeof(DEV_ENABLE_INFO)) 
			{
				return NET_ILLEGAL_PARAM;
			}

			char buf[1024] = {0};

			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DEV_ALL, buf, 1024, &nRetLen, waittime);
			if (nRet >= 0 && nRetLen > 0) 
			{
				CStrParse parser;
				parser.setSpliter("&&");
				bool bSuccess = parser.Parse(buf);
				if (!bSuccess) 
				{
					nRet = -1;
					break;
				}

				DEV_ENABLE_INFO stuDevEnable = {0};
				stuDevEnable.IsFucEnable[EN_SCHEDULE] = 0x00000007;//这是以前设备都具备的能力，现在是要屏蔽这个能力。

				for(int i = 0; i < parser.Size(); i++)
				{
					std::string info = parser.getWord();
					CStrParse subparse;
					subparse.setSpliter(":");
					bSuccess = subparse.Parse(info);
					char szsubinfo[256] = {0};//存储子功能项
					
					if (!bSuccess) 
					{
						break;
					}
					char FunctionType[100][40]={"FTP","SMTP","NTP","AutoMaintain","VideoCover",
												"AutoRegister","DHCP","UPNP","CommSniffer","NetSniffer",
												"Burn","VideoMatrix","AudioDetect","STORE POSITION",
												"IPS SEARCH","SNAP","DEFAULTQUERY","SHOWQUALITY","ConfigImportExport",
												"Log","SCHEDULE","NWDevType","ImportantRecID","ACFControl","ENCODE OPTION",
												"DavinciModule","Dahua.Gps","Dahua.MultiNet","Dahua.a4.9",
												"Dahua.Device.Record.General","Dahua_Config"};

					for(int j = 0; j < 50; j++)
					{
						if (0 == _stricmp(subparse.getWord(0).c_str(), FunctionType[j]) )
						{
							if (subparse.getWord(2)!="") 
							{
								strcpy(szsubinfo,subparse.getWord(2).c_str());
								GetFunctionMsk(j,szsubinfo, &stuDevEnable.IsFucEnable[j], lLoginID,waittime);
							}
							break;
						}
					}
				}
					
				memcpy(pBuf,&stuDevEnable, sizeof(DEV_ENABLE_INFO));
				*pRetLen = sizeof(DEV_ENABLE_INFO);
			}
		}
		break;
	case ABILITY_WIRELESS_CFG: /*wireless 能力*/
		{
			if (nBufLen < sizeof(WIRELESS_EN)) 
			{
				return NET_ILLEGAL_PARAM;
			}
			
			//查看能力
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WIRELESS, pBuf, nBufLen, &nRetLen, waittime);
			*pRetLen = nRetLen;
			
		}	
		break;
	case ABILITY_WATERMARK_CFG:
		{
			if (nBufLen < sizeof(WATERMAKE_EN))
			{
				return NET_ILLEGAL_PARAM;
			}
			//查看能力集
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_WATERMARK, pBuf, nBufLen, &nRetLen, waittime);
			*pRetLen = nRetLen;
			
		}
		break;
	case ABILITY_MULTIPLAY:
		{
			if (nBufLen < sizeof(MULTIPLAY_EN))
			{
				return NET_ILLEGAL_PARAM;
			}

			MULTIPLAY_EN stuMultiPlay = {0};

			device->get_info(device, dit_multiplay_flag, &stuMultiPlay.nEnable);
			
			memcpy(pBuf,&stuMultiPlay,sizeof(MULTIPLAY_EN));

			*pRetLen = sizeof(MULTIPLAY_EN);
			
			return 0;
		}
		break;
	case ABILITY_INFRARED:
		{
			if(nBufLen < sizeof(WIRELESS_ALARM_INFO))
			{
				return -1;
			}

			WIRELESS_ALARM_INFO * pWirelessInfo = (WIRELESS_ALARM_INFO*)pBuf;
			memset(pWirelessInfo,0,sizeof(WIRELESS_ALARM_INFO));
			char buffer[128] = {0};
			int nRet = 0;
			int retlen = 0;
			*pRetLen = 0;
			nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_INFRARED, buffer, 128, &retlen, waittime);
			if (nRet ==0 && retlen > 0)
			{
				CStrParse parse;
				parse.setSpliter("::");
				bool bSuccess = parse.Parse(buffer);
				if (!bSuccess)
				{
					return 0;
				}
				int nEnable = atoi(parse.getWord(0).c_str());
				pWirelessInfo->bSupport= nEnable == 1 ? true : false;
				pWirelessInfo->nAlarmInCount = atoi(parse.getWord(1).c_str());//输入个数
				pWirelessInfo->nAlarmOutCount = atoi(parse.getWord(2).c_str());//输出个数
				pWirelessInfo->nRemoteAddrCount = atoi(parse.getWord(3).c_str());//遥控器个数
				*pRetLen = sizeof(WIRELESS_ALARM_INFO);
			}
			return 0;
		}
		break;
	case ABILITY_TRIGGER_MODE:
		{
			if(NULL == pBuf || nBufLen < sizeof(int))
			{
				return -1;
			}

			int nIOCount = 0;
			int nRet = QueryIOControlState(lLoginID, IOTYPE_ALARMOUTPUT, NULL, 0, &nIOCount, waittime);
			*(int *)pBuf = nRet;
			*pRetLen = sizeof(int);
			
			return 1;
		}
		break;
	case ABILITY_DISK_SUBAREA://硬盘分区能力查询
		{
			if(NULL == pBuf || nBufLen < sizeof(DISK_SUBAREA_EN))
			{
				return -1;
			}

			int nReturn = -1;
			int nRetLen = 0;
			char szBuf[256] = {0};
			*pRetLen = 0;
			int nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_DISK_SUBAREA, szBuf, 256, &nRetLen, waittime);
			if(nRet == 0 && nRetLen > 0 )
			{
				char szValue[256];
				memset(szValue, 0 ,256);
				DISK_SUBAREA_EN* pSubarea = (DISK_SUBAREA_EN *)pBuf;
				memset(pSubarea, 0, sizeof(DISK_SUBAREA_EN));
				*pRetLen = sizeof(DISK_SUBAREA_EN);
				int nRet = GetValueFormNewPacket(szBuf, "support_part", szValue, 256);
				if(nRet > 0)
				{
					pSubarea->bSupported = atoi(szValue);
				}
				//如果支持分区，再解析支持的数目
				if(pSubarea->bSupported > 0)
				{
					nRet = GetValueFormNewPacket(szBuf, "max_partnum", szValue, 256);
					if(nRet > 0)
					{
						pSubarea->nSupportNum = atoi(szValue);
					}	
				}
				nReturn = 1;
			}

			return nReturn;
		}
		break;
	case ABILITY_DSP_CFG:
		{
			if(NULL == pBuf || nBufLen < sizeof(DSP_CFG))
			{
				return -1;
			}

			DSP_CFG stuTmpDspCfg = {0};
			int nCount = device->channelcount(device);
			stuTmpDspCfg.nItemNum = nCount;
			int nRet = -1;
			int nRetLen = 0;
			char szTmpBuf[1024];
			int nParam = 0;

			for(int i = 0; i < nCount; i++)
			{
				memset(szTmpBuf, 0, 1024);
				nParam = ((i+1)<<8)|2;
				nRet = QueryConfig(lLoginID, CONFIG_TYPE_DSP_MASK, nParam, szTmpBuf, 1024, &nRetLen, waittime);
				if (nRet >= 0 && nRetLen > 0) 
				{
					char szValue[64];
					char *p = szTmpBuf;
					while(p != NULL)
					{
						p = GetProtocolValue(p, "MainResolution:", "\r\n", szValue, 64);
						if(p != NULL)
						{
							string strTmp(szValue);
							string strLeft = strTmp.substr(0, strTmp.find("&"));
							string strRight = strTmp.substr(strTmp.find("&")+1, strTmp.size());
							int nIndex = atoi(strLeft.c_str());
							if(nIndex < CAPTURE_SIZE_NR)
							{
								stuTmpDspCfg.stuDspCfgItem[i].bMainFrame[nIndex] = atoi(strRight.c_str());
							}
						}
					}

					p = szTmpBuf;
					while(p != NULL)
					{
						p = GetProtocolValue(p, "ExtraResolution_1:", "\r\n", szValue, 64);
						if(p != NULL)
						{
							string strTmp(szValue);
							string strLeft = strTmp.substr(0, strTmp.find("&"));
							string strRight = strTmp.substr(strTmp.find("&")+1, strTmp.size());
							int nIndex = atoi(strLeft.c_str());
							if(nIndex < CAPTURE_SIZE_NR)
							{
								stuTmpDspCfg.stuDspCfgItem[i].bExtraFrame_1[nIndex] = atoi(strRight.c_str());
							}
						}
					}
				}
				else
				{
					return -1;
				}
			}
			
			memcpy(pBuf, &stuTmpDspCfg, sizeof(DSP_CFG));
			*pRetLen = sizeof(DSP_CFG);
			return 1;
		}
		break;
	case ABILITY_QUICK_QUERY_CFG:      //是否快速查询配置
		{
			if (nBufLen < sizeof(QUICK_QUERY_CFG_EN)) 
			{
				return NET_ILLEGAL_PARAM;
			}
			
			QUICK_QUERY_CFG_EN stQuickQuery = {0};
			
			device->get_info(device, dit_quick_config, &stQuickQuery.IsQuickQueryEnable);
			
			memcpy(pBuf,&stQuickQuery, sizeof(QUICK_QUERY_CFG_EN));
			
			*pRetLen = sizeof(QUICK_QUERY_CFG_EN);
			
			return 0;
		}
		break;
	case ABILITY_STREAM_MEDIA:
		{
			if(nBufLen < sizeof(STREAM_MEDIA_EN))
			{
				return NET_ILLEGAL_PARAM;
			}

			STREAM_MEDIA_EN stuStreamMediaEn = {0};
			int nRet = m_pManager->GetDevConfigEx().GetDevNewConfig_SIPAbility(lLoginID, &stuStreamMediaEn, waittime);
			if(nRet >= 0)
			{
				*pRetLen = sizeof(STREAM_MEDIA_EN);
				memcpy(pBuf, &stuStreamMediaEn, sizeof(STREAM_MEDIA_EN));
				return 1;
			}

			return -1;

		}
		break;
	default:
		//nRet = QuerySystemInfo(lLoginID, nType, pBuf, nBufLen, pRetLen, waittime);
		break;
	}
	
    return nRet;
}
//End: cqs(10842)

/*  查询Sniffer配置 */
//Add: by cqs(10842) 2008-5-9
int	CDevConfig::GetDevConfig_SnifferCfg(LONG lLoginID, DEVICE_SNIFFER_CFG *pstSniffer, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	int retlen = 0;
	int nRet = -1;
	int nRetLen = 0;
	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_COMM_SNIFFER] != 0 || stDevEn.IsFucEnable[EN_NET_SNIFFER] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	memset(pstSniffer, 0, sizeof(DEVICE_SNIFFER_CFG));
	pstSniffer->dwSize = sizeof(DEVICE_SNIFFER_CFG);
	
	//查询Sniffer信息

	CONFIG_ATM_SNIFFER stSnifferCfg[SNIFFER_GROUP_NUM] = {0};

	nRet = QueryConfig(lLoginID, CONFIG_TYPE_SNIFFER, 0, (char *)&stSnifferCfg, SNIFFER_GROUP_NUM*sizeof(CONFIG_ATM_SNIFFER), &retlen, waittime);

	if (nRet >= 0 && retlen == sizeof(CONFIG_ATM_SNIFFER)*SNIFFER_GROUP_NUM)
	{
		in_addr tmp = {0};

		for( int i = 0; i< SNIFFER_GROUP_NUM; i++ )
		{
			tmp.s_addr = stSnifferCfg[i].snifferIp.SnifferSrcIP;
			strcpy(pstSniffer->SnifferConfig[i].SnifferSrcIP, inet_ntoa(tmp));
			pstSniffer->SnifferConfig[i].SnifferSrcPort = stSnifferCfg[i].snifferIp.SnifferSrcPort;

			tmp.s_addr = stSnifferCfg[i].snifferIp.SnifferDestIP;
			strcpy(pstSniffer->SnifferConfig[i].SnifferDestIP, inet_ntoa(tmp));	
			pstSniffer->SnifferConfig[i].SnifferDestPort = stSnifferCfg[i].snifferIp.SnifferDestPort;

			pstSniffer->SnifferConfig[i].displayPosition = stSnifferCfg[i].displayPosition;
			pstSniffer->SnifferConfig[i].recdChannelMask = stSnifferCfg[i].recdChannelMask;

			for( int j =0; j< SNIFFER_FRAMEID_NUM; j++)
			{

				memcpy(&pstSniffer->SnifferConfig[i].snifferFrame[j].snifferFrameId,&(stSnifferCfg[i].snifferFrameId[j]),sizeof(SNIFFER_FRAMEID));
				
				for(int z =0; z< SNIFFER_GROUP_NUM; z++)
				{
					unsigned char szKeyTile[24] = {0};
					memcpy(szKeyTile,stSnifferCfg[i].snifferContent[j][z].KeyTitle,24);
					memset(stSnifferCfg[i].snifferContent[j][z].KeyTitle,0,24);
					Change_Utf8_Assic(szKeyTile,stSnifferCfg[i].snifferContent[j][z].KeyTitle);

					memcpy(&pstSniffer->SnifferConfig[i].snifferFrame[j].snifferContent[z],&stSnifferCfg[i].snifferContent[j][z],sizeof(SNIFFER_CONTENT));			
				}

			}
		}
	}
	else
	{
		nRet = NET_RETURN_DATA_ERROR;
	}

	return nRet;
}

int	CDevConfig::SetDevConfig_SnifferCfg(LONG lLoginID, DEVICE_SNIFFER_CFG *pstSniffer, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	if (NULL == pstSniffer)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0; //data length
	int nRet = -1;  //return value
	int nFailedCount = 0;	//在分次处理时，记录失败的次数

	int nRetLen = 0;
	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_COMM_SNIFFER] != 0 || stDevEn.IsFucEnable[EN_NET_SNIFFER] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	
	//set sniffer config
	
	CONFIG_ATM_SNIFFER tmpSnifferCfg[SNIFFER_GROUP_NUM] = {0};

	nRet = QueryConfig(lLoginID, CONFIG_TYPE_SNIFFER, 0, (char *)&tmpSnifferCfg, sizeof(CONFIG_ATM_SNIFFER)*SNIFFER_GROUP_NUM, &retlen, waittime);
	if (nRet >= 0 && retlen == sizeof(CONFIG_ATM_SNIFFER)*SNIFFER_GROUP_NUM)
	{
			
		for( int i = 0; i< SNIFFER_GROUP_NUM; i++ )
		{	
		//	memcpy(tmpSnifferCfg,pstSniffer->SnifferConfig, retlen);
			tmpSnifferCfg[i].snifferIp.SnifferSrcIP = inet_addr(pstSniffer->SnifferConfig[i].SnifferSrcIP);
			tmpSnifferCfg[i].snifferIp.SnifferSrcPort = pstSniffer->SnifferConfig[i].SnifferSrcPort;
			tmpSnifferCfg[i].snifferIp.SnifferDestIP = inet_addr(pstSniffer->SnifferConfig[i].SnifferDestIP);
			tmpSnifferCfg[i].snifferIp.SnifferDestPort = pstSniffer->SnifferConfig[i].SnifferDestPort;
			
			tmpSnifferCfg[i].displayPosition = pstSniffer->SnifferConfig[i].displayPosition; 
			tmpSnifferCfg[i].recdChannelMask = pstSniffer->SnifferConfig[i].recdChannelMask;
			
			for( int j =0; j< SNIFFER_FRAMEID_NUM; j++)
			{
				
				memcpy(&(tmpSnifferCfg[i].snifferFrameId[j]),&pstSniffer->SnifferConfig[i].snifferFrame[j].snifferFrameId,sizeof(SNIFFER_FRAMEID));
				
				for(int z =0; z< SNIFFER_GROUP_NUM; z++)
				{
					char szKeyTile[24] = {0};
					
					memcpy(szKeyTile,pstSniffer->SnifferConfig[i].snifferFrame[j].snifferContent[z].KeyTitle,24);
					memset(pstSniffer->SnifferConfig[i].snifferFrame[j].snifferContent[z].KeyTitle,0,24);
					Change_Assic_UTF8(szKeyTile, sizeof(szKeyTile), pstSniffer->SnifferConfig[i].snifferFrame[j].snifferContent[z].KeyTitle, 24);
					
					memcpy(&tmpSnifferCfg[i].snifferContent[j][z],&pstSniffer->SnifferConfig[i].snifferFrame[j].snifferContent[z],sizeof(SNIFFER_CONTENT));
					
				}
				
			}
		}

		nRet = SetupConfig(lLoginID, CONFIG_TYPE_SNIFFER, 0, (char *)&tmpSnifferCfg, (sizeof(CONFIG_ATM_SNIFFER)*SNIFFER_GROUP_NUM), waittime);
		
		if (nRet < 0)
		{
			nFailedCount++;
#ifdef _DEBUG
			OutputDebugString("SetupConfig(SNIFERR_CFG) failed!\n");				
#endif
		}
		else
		{
			Sleep(SETUP_SLEEP);
		}
	}
	
	if (nFailedCount > 0)
	{
		nRet = NET_ERROR_SETCFG_WATERMAKE;
	}
	else
	{
		nRet = NET_NOERROR;
	}
	
	return nRet;
}

int CDevConfig::GetDevConfig_AudioDetectCfg(LONG lLoginID, AUDIO_DETECT_CFG* pAudioDetectAlarm, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pAudioDetectAlarm == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0;
	int nRetLen = 0;
	int nRet = -1;
	int nSheetNum = 16;
	int nAlarmCount = 0;
	bool bSupport = false;

	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_AUDIO_DETECT] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}

	pAudioDetectAlarm->dwSize = sizeof(AUDIO_DETECT_CFG);

	CONFIG_ALARM_AUDIO stuAudioDetect[16] = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_AUDIODETECT, 0, (char*)stuAudioDetect, 16*sizeof(CONFIG_ALARM_AUDIO), &retlen, waittime);
	if (nRet >= 0 && retlen > 0 && 0 == retlen%sizeof(CONFIG_ALARM_AUDIO))
	{
		nAlarmCount = retlen / sizeof(CONFIG_ALARM_AUDIO);
		device->set_info(device,dit_audio_alarmin_num,&nAlarmCount);

		pAudioDetectAlarm->AlarmNum = nAlarmCount > nSheetNum ? nSheetNum:nAlarmCount;
		for (int i = 0; i < nAlarmCount; i++)
		{
			pAudioDetectAlarm->AudioDetectAlarm[i].bEnable = stuAudioDetect[i].enable;
			pAudioDetectAlarm->AudioDetectAlarm[i].Volume_max = stuAudioDetect[i].Volume_max;
			pAudioDetectAlarm->AudioDetectAlarm[i].Volume_min = stuAudioDetect[i].Volume_min;
			
			GetAlmActionMsk(CONFIG_TYPE_ALARM_LOCALALM, &pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwActionMask);
			int index = min(32, MAX_VIDEO_IN_NUM);
			int j = 0;
			for (j = 0; j < index; j++)
			{
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.struPtzLink[j].iValue = stuAudioDetect[i].hEvent.PtzLink[j].iValue;
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.struPtzLink[j].iType = stuAudioDetect[i].hEvent.PtzLink[j].iType;
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byRecordChannel[j] = BITRHT(stuAudioDetect[i].hEvent.dwRecord,j)&1;
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byTour[j] = BITRHT(stuAudioDetect[i].hEvent.dwTour,j)&1;
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bySnap[j] = BITRHT(stuAudioDetect[i].hEvent.dwSnapShot,j)&1;
			}
			index = min(32, MAX_ALARM_OUT_NUM);
			for (j = 0; j < index; j++)
			{
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byRelAlarmOut[j] = BITRHT(stuAudioDetect[i].hEvent.dwAlarmOut,j)&1;
				pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byRelWIAlarmOut[j] = BITRHT(stuAudioDetect[i].hEvent.wiAlarmOut,j)&1;
			}
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwDuration = stuAudioDetect[i].hEvent.iAOLatch;
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwRecLatch = stuAudioDetect[i].hEvent.iRecordLatch;//把预录时间取消空间用来表示录像延时，变量名待改
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwEventLatch = stuAudioDetect[i].hEvent.iEventLatch;
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bMessageToNet = (BYTE)stuAudioDetect[i].hEvent.bMessagetoNet;
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bMMSEn = (BYTE)stuAudioDetect[i].hEvent.bMMSEn;
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bySnapshotTimes = stuAudioDetect[i].hEvent.SnapshotTimes;//短信发送图片的张数
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bLog = (BYTE)stuAudioDetect[i].hEvent.bLog;
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bMatrixEn = (BYTE)stuAudioDetect[i].hEvent.bMatrixEn;
			pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwMatrix = stuAudioDetect[i].hEvent.dwMatrix;
			GetAlmActionFlag(stuAudioDetect[i].hEvent, &pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwActionFlag);
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_SOUNDALARM;
		return nRet;
	}
	
	CONFIG_WORKSHEET WorkSheet[16] = {0};
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_MOTION, WorkSheet, waittime, nAlarmCount);
	if (nRet >= 0)
	{
		for (int i = 0; i < nAlarmCount; i++)
		{
			memcpy(pAudioDetectAlarm->AudioDetectAlarm[i].stSect, WorkSheet[i].tsSchedule, sizeof(WorkSheet[i].tsSchedule));
		}
	}
	else
	{
		nRet = -1;
	}
	
	return nRet;
}

int CDevConfig::SetDevConfig_AudioDetectCfg(LONG lLoginID, AUDIO_DETECT_CFG* pAudioDetectAlarm, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pAudioDetectAlarm == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	int retlen = 0;
	int nRetLen = 0;
	int nRet = -1;
	int nSheetNum = 0;
	device->get_info(device, dit_audio_alarmin_num, &nSheetNum);
	if (nSheetNum == 0) 
	{
		nSheetNum = 16;
	}

	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_AUDIO_DETECT] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	
	CONFIG_ALARM_AUDIO stuAudioDetect[16] = {0};

	for (int i = 0; i < nSheetNum; i++)
	{
		stuAudioDetect[i].enable = pAudioDetectAlarm->AudioDetectAlarm[i].bEnable;
		stuAudioDetect[i].Volume_max = pAudioDetectAlarm->AudioDetectAlarm[i].Volume_max;
		stuAudioDetect[i].Volume_min = pAudioDetectAlarm->AudioDetectAlarm[i].Volume_min;

		SetAlmActionFlag(&stuAudioDetect[i].hEvent, pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwActionFlag);

		int index = min(32, MAX_VIDEO_IN_NUM);
		int j = 0;
		for (j = 0; j < index; j++)
		{
			stuAudioDetect[i].hEvent.dwSnapShot |= (pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bySnap[j]) ? (0x01<<j) : 0;
			stuAudioDetect[i].hEvent.dwTour |= (pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byTour[j]) ? (0x01<<j) : 0;
			stuAudioDetect[i].hEvent.dwRecord |= (pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byRecordChannel[j]) ? (0x01<<j) : 0;
			stuAudioDetect[i].hEvent.PtzLink[j].iValue = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.struPtzLink[j].iValue;
			stuAudioDetect[i].hEvent.PtzLink[j].iType = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.struPtzLink[j].iType;
		}

		index = min(32, MAX_ALARM_OUT_NUM);
		for (j = 0; j < index; j++)
		{
			stuAudioDetect[i].hEvent.dwAlarmOut |= (pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byRelAlarmOut[j]) ? (0x01<<j) : 0;
			stuAudioDetect[i].hEvent.wiAlarmOut |= (pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.byRelWIAlarmOut[j])?(0x01<<j) : 0;
		}
		stuAudioDetect[i].hEvent.iAOLatch = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwDuration;
		stuAudioDetect[i].hEvent.iRecordLatch = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwRecLatch;//把预录时间取消用来表示录像延时，变量名待改
		stuAudioDetect[i].hEvent.iEventLatch = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwEventLatch;
		stuAudioDetect[i].hEvent.bMessagetoNet = (BOOL)pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bMessageToNet;
		stuAudioDetect[i].hEvent.bMMSEn = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bMMSEn;
		stuAudioDetect[i].hEvent.SnapshotTimes = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bySnapshotTimes;//短信发送图片的张数
		stuAudioDetect[i].hEvent.bLog = (BOOL)pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bLog;
		stuAudioDetect[i].hEvent.bMatrixEn = (BOOL)pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.bMatrixEn;
		stuAudioDetect[i].hEvent.dwMatrix = pAudioDetectAlarm->AudioDetectAlarm[i].struHandle.dwMatrix;
	}

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_ALARM_AUDIODETECT, 0, (char*)stuAudioDetect, nSheetNum*sizeof(CONFIG_ALARM_AUDIO), waittime);
	if (nRet < 0)
	{
		return NET_ERROR_SETCFG_SOUNDALARM;
	}

	Sleep(SETUP_SLEEP);

	CONFIG_WORKSHEET WorkSheet[16] = {0};
	nRet = GetDevConfig_WorkSheet(lLoginID, WSHEET_LOCAL_ALARM, WorkSheet, waittime, nSheetNum);
	if (nRet >= 0)
	{
		for (int i = 0; i < nSheetNum; i++)
		{
			WorkSheet[i].iName = i;
			memcpy(WorkSheet[i].tsSchedule, pAudioDetectAlarm->AudioDetectAlarm[i].stSect, sizeof(WorkSheet[i].tsSchedule));
		}

		nRet = SetDevConfig_WorkSheet(lLoginID, WSHEET_MOTION, WorkSheet, waittime, nSheetNum);
	}
	else
	{
		nRet = -1;
	}

	return nRet;
}

int	CDevConfig::GetDevConfig_AllStorageStateCfg(LONG lLoginID, STORAGE_STATION_CFG *pstStorageCHC, LONG lChnNum, int waittime)
{
	if (!pstStorageCHC || lChnNum < 0 || lChnNum > 16)
	{
		return -1;
	}

	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	int retlen = 0;
	int nRet = -1;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	int nRetLen = 0;
	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_STORAGE_STATION] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	
	memset(pstStorageCHC, 0, lChnNum*sizeof(STORAGE_STATION_CFG));
	for (int i = 0; i < lChnNum; i++)
	{
		pstStorageCHC[i].dwSize = sizeof(STORAGE_STATION_CFG);
	}

	if (iRecvBufLen < lChnNum * sizeof(CONFIG_STORAGE_STATION)) 
	{
		iRecvBufLen = lChnNum * sizeof(CONFIG_STORAGE_STATION);
	}
	cRecvBuf = new char[iRecvBufLen];

	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
   //获取存储位置的配置
	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_STORSTATION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == lChnNum * sizeof(CONFIG_STORAGE_STATION))
	{
		CONFIG_STORAGE_STATION *stStorage = 0;
		for (int i = 0; i < lChnNum; i++)
		{
			stStorage = (CONFIG_STORAGE_STATION *)(cRecvBuf + i*sizeof(CONFIG_STORAGE_STATION));
			pstStorageCHC[i].dwLocalMask = stStorage->dwLocalMask;
			pstStorageCHC[i].dwMobileMask = stStorage->dwMobileMask;
			pstStorageCHC[i].dwRemoteMask = stStorage->dwRemoteMask;
			pstStorageCHC[i].dwRemoteSecondSelLocal = stStorage->dwRemoteSecondSelLocal;
			pstStorageCHC[i].dwRemoteSecondSelMobile = stStorage->dwRemoteSecondSelMobile;
			pstStorageCHC[i].RemoteType = stStorage->RemoteType;
			memcpy(pstStorageCHC[i].SubRemotePath,stStorage->SubRemotePath,MAX_PATH_STOR);
			pstStorageCHC[i].dwFunctionMask = stStorage->dwFunctionMask;
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_STORAGE;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int	CDevConfig::GetDevConfig_StorageStateCfg(LONG lLoginID, STORAGE_STATION_CFG &stuStorageCHA, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return -1;
	}

	if (lChannel < 0 || lChannel >= device->channelcount(device))
	{
		return -1;
	}
	
	int retlen = 0;
	int nRet = -1;
	int nRetLen = 0;
	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_STORAGE_STATION] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	memset(&stuStorageCHA, 0, sizeof(STORAGE_STATION_CFG));
	stuStorageCHA.dwSize = sizeof(STORAGE_STATION_CFG);
	
	int nChannelnum = device->channelcount(device);
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < nChannelnum * sizeof(CONFIG_STORAGE_STATION))
	{
		iRecvBufLen = nChannelnum * sizeof(CONFIG_STORAGE_STATION);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_STORSTATION, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);
	if (nRet >= 0 && retlen == nChannelnum * sizeof(CONFIG_STORAGE_STATION))
	{
		CONFIG_STORAGE_STATION *stStorage = (CONFIG_STORAGE_STATION *)(cRecvBuf + lChannel*sizeof(CONFIG_STORAGE_STATION));
		
		stuStorageCHA.dwLocalMask = stStorage->dwLocalMask;
		stuStorageCHA.dwMobileMask = stStorage->dwMobileMask;
		stuStorageCHA.dwRemoteMask = stStorage->dwRemoteMask;
		stuStorageCHA.dwRemoteSecondSelLocal = stStorage->dwRemoteSecondSelLocal;
		stuStorageCHA.dwRemoteSecondSelMobile = stStorage->dwRemoteSecondSelMobile;
		stuStorageCHA.RemoteType = stStorage->RemoteType;
		memcpy(stuStorageCHA.SubRemotePath,stStorage->SubRemotePath,MAX_PATH_STOR);
		stuStorageCHA.dwFunctionMask = stStorage->dwFunctionMask;
	}
	else
	{
		return NET_ERROR_GETCFG_STORAGE;
	}
		
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_StorageStateCfg(LONG lLoginID, STORAGE_STATION_CFG *pstStorageCHC, LONG lChannel, int waittime)
{
	afk_device_s* device = (afk_device_s*)lLoginID;
	
	int iChanNum = device->channelcount(device);
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (lChannel < -1 || lChannel >= MAX_CHANNUM ||
		(lChannel != -1 && lChannel >= iChanNum))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	if (NULL == pstStorageCHC)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	STORAGE_STATION_CFG *const pStorageChaCfg = pstStorageCHC;
	STORAGE_STATION_CFG *pTmpStateChaCfg = pStorageChaCfg;

	int retlen = 0; 
	int nRet = -1;  
	int nRetLen = 0;
	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_STORAGE_STATION] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}
	int iChanCount = 1;
	int iChanIdx = lChannel;
	
	if (-1 == lChannel)//all
	{
		iChanIdx = 0;
		iChanCount = iChanNum;
	}

	int buflen = 0;
	char *vbuf = NULL;

	if (buflen < iChanNum*sizeof(CONFIG_STORAGE_STATION))
	{
		buflen = iChanNum*sizeof(CONFIG_STORAGE_STATION);
	}

	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_STORAGE;
	}
	
	//设置存储位置配置
	CONFIG_STORAGE_STATION *pStorageCfg = NULL;	
	pTmpStateChaCfg = pStorageChaCfg;

	memset((void *)vbuf, 0, buflen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_STORSTATION, 0, vbuf, buflen, &retlen, waittime);
	if (nRet >= 0 && retlen == iChanNum*sizeof(CONFIG_STORAGE_STATION))
	{
		pStorageCfg = (CONFIG_STORAGE_STATION *)(vbuf + iChanIdx*sizeof(CONFIG_STORAGE_STATION));
		for (int i = 0; i < iChanCount; i++)
		{
			pStorageCfg->dwLocalMask = pTmpStateChaCfg->dwLocalMask;
			pStorageCfg->dwMobileMask = pTmpStateChaCfg->dwMobileMask;
			pStorageCfg->dwRemoteMask = pTmpStateChaCfg->dwRemoteMask;
			pStorageCfg->dwRemoteSecondSelLocal = pTmpStateChaCfg->dwRemoteSecondSelLocal;
			pStorageCfg->dwRemoteSecondSelMobile = pTmpStateChaCfg->dwRemoteSecondSelMobile;
			pStorageCfg->RemoteType = pTmpStateChaCfg->RemoteType;
			memcpy(pStorageCfg->SubRemotePath,pTmpStateChaCfg->SubRemotePath,MAX_PATH_STOR);
			
			pTmpStateChaCfg++;
			pStorageCfg++;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_STORSTATION, 0, vbuf, iChanNum*sizeof(CONFIG_STORAGE_STATION), waittime);
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	
	return nRet;
}

int CDevConfig::GetDevConfig_DSTCfg(LONG lLoginID, DEVICE_DST_CFG *pstDSTInfo, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return NET_INVALID_HANDLE;
	}
	
	int retlen = 0;
	int nRet = -1;

	memset(pstDSTInfo, 0, sizeof(DEVICE_DST_CFG));
	pstDSTInfo->dwSize = sizeof(DEVICE_DST_CFG);
	
	//查询DST信息
	char buf[1024] = {0};	
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_GENERAL, 1, buf, sizeof(buf), &retlen, waittime);

	if (nRet >= 0 && retlen > 0)
	{
		CStrParse parser;
		parser.setSpliter("&&");
		bool bSuccess = parser.Parse(buf);
		if (!bSuccess) 
		{
			return NET_RETURN_DATA_ERROR;
		}

		//en&&year，month，week，dayorweek，hour，minute::year，month，week，dayorweek，hour，minute
		//比如：1&&2000，4，16，1，14，28::2008，4，16，1，14，28	
		//说明：en使能，后面是开始时间和结束时间
		//week = 0时表示设置的是天，dayorweek表示月的哪一天(【1－31】)。
		//当week不等于0时，表示设置的是周，正数为正数第几周，从1开始，负数表示倒数，－1最示最后一周，-2表示倒数第二周，目前只做了1,2,3,4,－1. dayorweek表示周的哪一天（星期天为0）。

		pstDSTInfo->dwReserved[0] = atoi(parser.getWord(0).c_str());//使能暂时不用

		std::string info = parser.getWord(1); //夏令时的配置信息
		CStrParse subparse;
		subparse.setSpliter("::");
		bSuccess = subparse.Parse(info);		
		if (bSuccess) 
		{
			//分析夏令时配置
			bool bSubSuccess = false;
			CStrParse subinfoparse;
			subinfoparse.setSpliter(",");
			std::string subinfo = subparse.getWord(0);	
			bSubSuccess = subinfoparse.Parse(subinfo);

			if (bSubSuccess)//夏令时开始
			{
				for (int j = 0; j < subinfoparse.Size(); j++)
				{
					pstDSTInfo->stDSTStart.nYear = atoi(subinfoparse.getWord(0).c_str());
					pstDSTInfo->stDSTStart.nMonth = atoi(subinfoparse.getWord(1).c_str());
					pstDSTInfo->stDSTStart.nWeekOrDay = atoi(subinfoparse.getWord(2).c_str());
					if (0 != pstDSTInfo->stDSTStart.nWeekOrDay)//按周的方式
					{
						pstDSTInfo->nDSTType = 1;
						pstDSTInfo->stDSTStart.iWeekDay = atoi(subinfoparse.getWord(3).c_str());
					}
					else //按日期方式
					{
						pstDSTInfo->nDSTType = 0;
						pstDSTInfo->stDSTStart.iDay = atoi(subinfoparse.getWord(3).c_str());
					}
					pstDSTInfo->stDSTStart.nHour = atoi(subinfoparse.getWord(4).c_str());
					pstDSTInfo->stDSTStart.nMinute = atoi(subinfoparse.getWord(5).c_str());
				}
			}

			subinfo = subparse.getWord(1);
			bSubSuccess = bSubSuccess = subinfoparse.Parse(subinfo);
			if (bSubSuccess)//夏令时结束
			{
				for (int j = 0; j < subinfoparse.Size(); j++)
				{
					pstDSTInfo->stDSTEnd.nYear = atoi(subinfoparse.getWord(0).c_str());
					pstDSTInfo->stDSTEnd.nMonth = atoi(subinfoparse.getWord(1).c_str());
					pstDSTInfo->stDSTEnd.nWeekOrDay = atoi(subinfoparse.getWord(2).c_str());
					if (0 != pstDSTInfo->stDSTEnd.nWeekOrDay)//按周的方式
					{
					//	pstDSTInfo->nDSTType = 1;
						pstDSTInfo->stDSTEnd.iWeekDay = atoi(subinfoparse.getWord(3).c_str());
					}
					else //按日期方式
					{
					//	pstDSTInfo->nDSTType = 0;
						pstDSTInfo->stDSTEnd.iDay = atoi(subinfoparse.getWord(3).c_str());
					}
					pstDSTInfo->stDSTEnd.nHour = atoi(subinfoparse.getWord(4).c_str());
					pstDSTInfo->stDSTEnd.nMinute = atoi(subinfoparse.getWord(5).c_str());
				}
			}	
		}
		else //不是夏令时的配置
		{
			return NET_ERROR_GETCFG_DST;
		}
	}
	else
	{
		return NET_ERROR_GETCFG_DST;
	}
	
	return nRet;	
}

int CDevConfig::SetDevConfig_DSTCfg(LONG lLoginID, DEVICE_DST_CFG *pstDSTInfo, int waittime)
{
	if (NULL == pstDSTInfo)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRetLen = 0;
	int nRet = -1;
	int nEnable = 1;//默认有使能

// 	//test
// 	pstDSTInfo->stDSTStart.nYear = 2005;
// 	pstDSTInfo->stDSTStart.nMonth = 1;
// 	pstDSTInfo->stDSTStart.nHour = 12;
// 	pstDSTInfo->stDSTStart.nMinute = 30;
// 	pstDSTInfo->stDSTStart.nWeekOrDay = 0;
// 	pstDSTInfo->stDSTEnd.iDay = 22;
// 	pstDSTInfo->stDSTEnd.nYear = 2010;
// 	pstDSTInfo->stDSTEnd.nMonth = 12;
// 	pstDSTInfo->stDSTEnd.nHour = 1;
// 	pstDSTInfo->stDSTEnd.nMinute = 20;
// 	pstDSTInfo->stDSTEnd.nWeekOrDay = 0;
// 	pstDSTInfo->stDSTEnd.iDay = 1;
	
	char szData[1024] = {0};

	sprintf(szData, "%d&&%d,%d,%d,%d,%d,%d::%d,%d,%d,%d,%d,%d", nEnable, pstDSTInfo->stDSTStart.nYear,
			pstDSTInfo->stDSTStart.nMonth, pstDSTInfo->stDSTStart.nWeekOrDay, pstDSTInfo->stDSTStart.iWeekDay,
			pstDSTInfo->stDSTStart.nHour, pstDSTInfo->stDSTStart.nMinute, pstDSTInfo->stDSTEnd.nYear,
			pstDSTInfo->stDSTEnd.nMonth, pstDSTInfo->stDSTEnd.nWeekOrDay, pstDSTInfo->stDSTEnd.iWeekDay,
			pstDSTInfo->stDSTEnd.nHour, pstDSTInfo->stDSTEnd.nMinute);

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_GENERAL, 1, szData, strlen(szData), waittime);

	if (nRet < 0)
	{
		nRet = NET_ERROR_SETCFG_DST;
	}
	
	return nRet;
}

//视频OSD叠加配置

int CDevConfig::GetDevConfig_AllVideoOSDCfg(LONG lLoginID, DVR_VIDEO_OSD_CFG *pstVideoOSD, LONG lChnNum, int waittime)
{
	if (!pstVideoOSD || lChnNum < 0 || lChnNum > MAX_CHANNUM)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0;
	int nRet = -1;
	
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	
	memset(pstVideoOSD, 0, lChnNum*sizeof(DVR_VIDEO_OSD_CFG));

	for (int i = 0; i < lChnNum; i++)
	{
		pstVideoOSD[i].dwSize = sizeof(DVR_VIDEO_OSD_CFG);
	}
	
	if (iRecvBufLen < lChnNum * sizeof(DVR_CONFIG_VIDEOOSD)) 
	{
		iRecvBufLen = lChnNum * sizeof(DVR_CONFIG_VIDEOOSD);
	}
	cRecvBuf = new char[iRecvBufLen];
	
	if (NULL == cRecvBuf)
	{
		goto END;
	}
	
	//获取视频OSD叠加配置
	memset(cRecvBuf, 0, iRecvBufLen);

	nRet = QueryConfig(lLoginID, CONFIG_TYPE_VIDEO_OSD, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);

	if (nRet >= 0 && retlen > 0 && 0 == retlen%sizeof(DVR_CONFIG_VIDEOOSD))
	{
		int nOSDChannelCount = retlen / sizeof(DVR_CONFIG_VIDEOOSD);
		int nChannelCount = nOSDChannelCount > lChnNum ? lChnNum:nOSDChannelCount;	

		DVR_CONFIG_VIDEOOSD *pVideoOSD = NULL;

		for (int i = 0; i < nChannelCount; i++)
		{
			pVideoOSD = (DVR_CONFIG_VIDEOOSD *)(cRecvBuf + i*sizeof(DVR_CONFIG_VIDEOOSD));

			for (int j = 0; j < VIDEO_CUSTOM_OSD_NUM; j++)
			{
				pstVideoOSD[i].StOSDTitleOpt[j].StOSD_POS.bEncodeBlend = pVideoOSD->OSD_TITLE[j].OSD_POS.bEncodeBlend;
				pstVideoOSD[i].StOSDTitleOpt[j].StOSD_POS.bPreviewBlend = pVideoOSD->OSD_TITLE[j].OSD_POS.bPreviewBlend;
				pstVideoOSD[i].StOSDTitleOpt[j].StOSD_POS.rgbaFrontground = pVideoOSD->OSD_TITLE[j].OSD_POS.rgbaFrontground;
				pstVideoOSD[i].StOSDTitleOpt[j].StOSD_POS.rgbaBackground = pVideoOSD->OSD_TITLE[j].OSD_POS.rgbaBackground;
				memcpy(&pstVideoOSD[i].StOSDTitleOpt[j].StOSD_POS.rcRelativePos, &pVideoOSD->OSD_TITLE[j].OSD_POS.rcRelativePos, sizeof(RECT));
				
				char SzName[VIDEO_OSD_NAME_NUM] = {0} ;
				Change_Utf8_Assic((unsigned char *)pVideoOSD->OSD_TITLE[j].OSD_NAME, SzName);
				strcpy(pstVideoOSD[i].StOSDTitleOpt[j].SzOSD_Name, SzName);		
			}	
		}
	}
	else
	{
		nRet = NET_ERROR_GETCFG_VIDEO_OSD;
	}
	
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
	
}


int	CDevConfig::GetDevConfig_VideoOSDCfg(LONG lLoginID, DVR_VIDEO_OSD_CFG &stVideoOSD, LONG IChannel, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return NET_ILLEGAL_PARAM;
	}

	if (IChannel < 0 || IChannel >= device->channelcount(device))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int retlen = 0;
	int nRet = -1;

	memset(&stVideoOSD, 0, sizeof(DVR_VIDEO_OSD_CFG));
	stVideoOSD.dwSize = sizeof(DVR_VIDEO_OSD_CFG);
	
	int nChannelnum = device->channelcount(device);
	char *cRecvBuf = NULL;
	int iRecvBufLen = 0;
	if (iRecvBufLen < nChannelnum * sizeof(DVR_CONFIG_VIDEOOSD))
	{
		iRecvBufLen = nChannelnum * sizeof(DVR_CONFIG_VIDEOOSD);
	}
	
	cRecvBuf = new char[iRecvBufLen];
	if (NULL == cRecvBuf)
	{
		goto END;
	}

	memset(cRecvBuf, 0, iRecvBufLen);

	nRet = QueryConfig(lLoginID, CONFIG_TYPE_VIDEO_OSD, 0, cRecvBuf, iRecvBufLen, &retlen, waittime);

	if (nRet >= 0 && retlen > 0 && 0 == retlen%sizeof(DVR_CONFIG_VIDEOOSD))
	{
		int nOSDChannelCount = retlen / sizeof(DVR_CONFIG_VIDEOOSD);
		
		if (nOSDChannelCount < IChannel)//设备传过来的结构体不够
		{
			return NET_RETURN_DATA_ERROR;
		}
	
		DVR_CONFIG_VIDEOOSD *pVideoOSD = (DVR_CONFIG_VIDEOOSD *)(cRecvBuf + IChannel*sizeof(DVR_CONFIG_VIDEOOSD));
		
		for (int j = 0; j < VIDEO_CUSTOM_OSD_NUM; j++)
		{
			stVideoOSD.StOSDTitleOpt[j].StOSD_POS.bEncodeBlend = pVideoOSD->OSD_TITLE[j].OSD_POS.bEncodeBlend;
			stVideoOSD.StOSDTitleOpt[j].StOSD_POS.bPreviewBlend = pVideoOSD->OSD_TITLE[j].OSD_POS.bPreviewBlend;
			stVideoOSD.StOSDTitleOpt[j].StOSD_POS.rgbaFrontground = pVideoOSD->OSD_TITLE[j].OSD_POS.rgbaFrontground;
			stVideoOSD.StOSDTitleOpt[j].StOSD_POS.rgbaBackground = pVideoOSD->OSD_TITLE[j].OSD_POS.rgbaBackground;
			memcpy(&stVideoOSD.StOSDTitleOpt[j].StOSD_POS.rcRelativePos, &pVideoOSD->OSD_TITLE[j].OSD_POS.rcRelativePos, sizeof(RECT));
			
			char SzName[VIDEO_OSD_NAME_NUM] = {0} ;
			Change_Utf8_Assic((unsigned char *)pVideoOSD->OSD_TITLE[j].OSD_NAME, SzName);
			strcpy(stVideoOSD.StOSDTitleOpt[j].SzOSD_Name, SzName);		
		}	
	}
	else
	{
		return NET_ERROR_GETCFG_VIDEO_OSD;
	}
		
END:
	if (cRecvBuf != NULL)
	{
		delete []cRecvBuf;
		cRecvBuf = NULL;
	}
	return nRet;
}

int CDevConfig::SetDevConfig_VideoOSDCfg(LONG lLoginID, DVR_VIDEO_OSD_CFG *pstVideoOSD, LONG IChannel, int waittime)
{

	afk_device_s* device = (afk_device_s*)lLoginID;
	
	int iChanNum = device->channelcount(device);
	if (!device || m_pManager->IsDeviceValid(device) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	if (IChannel < -1 || IChannel >= MAX_CHANNUM || (IChannel != -1 && IChannel >= iChanNum))
	{
		return NET_ILLEGAL_PARAM;
	}
	
	if (NULL == pstVideoOSD)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	DVR_VIDEO_OSD_CFG *const pVideoOSDChaCfg = pstVideoOSD;
	DVR_VIDEO_OSD_CFG *pTmpVideoOSDChaCfg = pVideoOSDChaCfg;

	int retlen = 0; 
	int nRet = -1;  

	int iChanCount = 1;
	int iChanIdx = IChannel;
	
	if (-1 == IChannel)//all
	{
		iChanIdx = 0;
		iChanCount = iChanNum;
	}

	int buflen = 0;
	char *vbuf = NULL;

	if (buflen < iChanNum*sizeof(DVR_CONFIG_VIDEOOSD))
	{
		buflen = iChanNum*sizeof(DVR_CONFIG_VIDEOOSD);
	}

	vbuf = new char[buflen];
	if (NULL == vbuf)
	{
		return NET_ERROR_SETCFG_VIDEO_OSD;
	}

	char pUTFName[VIDEO_OSD_NAME_NUM];
	
	//设置视频OSD叠加配置

	DVR_CONFIG_VIDEOOSD *pVideoOSDCfg = NULL;	
	pTmpVideoOSDChaCfg = pVideoOSDChaCfg;

	memset((void *)vbuf, 0, buflen);
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_VIDEO_OSD, 0, vbuf, buflen, &retlen, waittime);

	if (nRet >= 0 && retlen > 0 && 0 == retlen%sizeof(DVR_CONFIG_VIDEOOSD))
	{
		int nOSDChannelCount = retlen / sizeof(DVR_CONFIG_VIDEOOSD);
		
		if (nOSDChannelCount < iChanIdx)//设备传过来的结构体不够
		{
			return NET_RETURN_DATA_ERROR;
		}

		pVideoOSDCfg = (DVR_CONFIG_VIDEOOSD *)(vbuf + iChanIdx*sizeof(DVR_CONFIG_VIDEOOSD));

		for (int i = 0; i < iChanCount; i++)
		{
			for (int j = 0; j < VIDEO_CUSTOM_OSD_NUM; j++)
			{
				pVideoOSDCfg->OSD_TITLE[j].OSD_POS.bEncodeBlend = pTmpVideoOSDChaCfg->StOSDTitleOpt[j].StOSD_POS.bEncodeBlend;
				pVideoOSDCfg->OSD_TITLE[j].OSD_POS.bPreviewBlend = pTmpVideoOSDChaCfg->StOSDTitleOpt[j].StOSD_POS.bPreviewBlend;
				pVideoOSDCfg->OSD_TITLE[j].OSD_POS.rgbaFrontground = pTmpVideoOSDChaCfg->StOSDTitleOpt[j].StOSD_POS.rgbaFrontground;
				pVideoOSDCfg->OSD_TITLE[j].OSD_POS.rgbaBackground = pTmpVideoOSDChaCfg->StOSDTitleOpt[j].StOSD_POS.rgbaBackground;
				memcpy(&pVideoOSDCfg->OSD_TITLE[j].OSD_POS.rcRelativePos, &pTmpVideoOSDChaCfg->StOSDTitleOpt[j].StOSD_POS.rcRelativePos, sizeof(RECT));
				
				memset(pUTFName, 0, sizeof(pUTFName));
				Change_Assic_UTF8((char *)(pTmpVideoOSDChaCfg->StOSDTitleOpt[j].SzOSD_Name), VIDEO_OSD_NAME_NUM, pUTFName, VIDEO_OSD_NAME_NUM);
				
				strcpy(pVideoOSDCfg->OSD_TITLE[j].OSD_NAME, pUTFName);
			}

			pTmpVideoOSDChaCfg++;
			pVideoOSDCfg++;
		}
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_VIDEO_OSD, 0, vbuf, nOSDChannelCount*sizeof(DVR_CONFIG_VIDEOOSD), waittime);
	}
	if (vbuf)
	{
		delete []vbuf;
	}
	
	if (nRet < 0)
	{
		nRet = NET_ERROR_SETCFG_VIDEO_OSD;
	}
	return nRet;
}

//GPRS/CDMA配置读取
int CDevConfig::GetDevConfig_GPRSCDMACfg(LONG lLoginID, DEVICE_CDMAGPRS_CFG* pstGPRSCDMAInfo, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (!device)
	{
		return NET_INVALID_HANDLE;
	}
	
	int retlen = 0;
	int nRet = -1;

	int nRetLen = 0;
	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_NETWORK_TYPE] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}

	memset(pstGPRSCDMAInfo, 0, sizeof(DEVICE_CDMAGPRS_CFG));
	pstGPRSCDMAInfo->dwSize = sizeof(DEVICE_CDMAGPRS_CFG);

	char szValueBuf[256] = {0};//为了解析协议用
	int	 nLen = 0;
	
	//查询GPRS/CDMA网络信息
	char buf[4096] = {0};	
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_WLAN, 1, buf, sizeof(buf), &retlen, waittime);

	if (nRet >= 0 && retlen > 0)
	{
		memset(szValueBuf, 0 , 256);
		//使能
		nLen = GetValueFormNewPacket(buf, "iEnable", szValueBuf, 256);
		if(nLen > 0)
		{
			pstGPRSCDMAInfo->bEnable = atoi(szValueBuf);			
		}
		else //不是GPRS/CDMA的配置
		{
			return NET_ERROR_GETCFG_GPRSCDMA;
		}

		//设备支持网络类型
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "TypeMask", szValueBuf, 256);
		if(nLen > 0)
		{
			pstGPRSCDMAInfo->dwTypeMask = atoi(szValueBuf);			
		}

		//当前支持的网络类型
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "iNetType", szValueBuf, 256);
		if(nLen > 0)
		{
			pstGPRSCDMAInfo->dwNetType = atoi(szValueBuf);			
		}

		//接入点名称
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szAPN", szValueBuf, 256);
		if(nLen < 128 && nLen > 0)
		{
			Change_Utf8_Assic((unsigned char *)szValueBuf, pstGPRSCDMAInfo->szAPN);
		}

		//拨号码
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szDialNum", szValueBuf, 256);
		if(nLen < 128 && nLen > 0)
		{
			strcpy(pstGPRSCDMAInfo->szDialNum, szValueBuf);
		}	

		//拨号用户名
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szUserName", szValueBuf, 256);
		if(nLen < 128 && nLen > 0)
		{
			Change_Utf8_Assic((unsigned char *)szValueBuf, pstGPRSCDMAInfo->szUserName);
		}

		//拨号密码
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szPWD", szValueBuf, 256);
		if( nLen > 0)
		{
			strcpy(pstGPRSCDMAInfo->szPWD, szValueBuf);
		}

		//无线网络注册状态
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "iAccessStat", szValueBuf, 256);
		if(nLen > 0)
		{
			pstGPRSCDMAInfo->iAccessStat = atoi(szValueBuf);
		}

		//前端设备拨号IP
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szDevIP", szValueBuf, 256);
		if(nLen < 16 && nLen > 0)
		{
			strcpy(pstGPRSCDMAInfo->szDevIP, szValueBuf);
		}

		//前端设备拨号子网掩码
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szSubNetMask", szValueBuf, 256);
		if(nLen < 16 && nLen > 0)
		{
			strcpy(pstGPRSCDMAInfo->szSubNetMask, szValueBuf);
		}

		//前端设备拨号网关
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "szGateWay", szValueBuf, 256);
		if(nLen < 16 && nLen > 0)
		{
			strcpy(pstGPRSCDMAInfo->szGateWay, szValueBuf);
		}	

		//保活时间 add by cqs 090404
		memset(szValueBuf, 0 , 256);
		nLen = GetValueFormNewPacket(buf, "iKeepLive", szValueBuf, 256);
		if(nLen > 0)
		{
			pstGPRSCDMAInfo->iKeepLive = atoi(szValueBuf);
		}

		//3G拨号时间段
		int nWeekIndex = 0;
		int nDayIndex = 0;

		char *p = buf;//解析时间段
		{
			for (int i = 0; i < DAYS_PER_WEEK*TIME_SECT_NUM; i++)
			{
				memset(szValueBuf, 0 , 256);
				p = GetProtocolValue(p, "TimeSection:", "\r\n", szValueBuf, 256);
				if (p == NULL)
				{
					break;
				}
				
				CStrParse parser;
				parser.setSpliter(" ");
				bool bSuccess = parser.Parse(szValueBuf);
				if (!bSuccess)
				{
					return NET_RETURN_DATA_ERROR;
				}

				nWeekIndex = i / TIME_SECT_NUM;
				nDayIndex = i % TIME_SECT_NUM;

				if (parser.Size() <= 1)//简单判断字符串的是否有两项
				{
					continue;
				}
				
				//使能
				pstGPRSCDMAInfo->stSect[nWeekIndex][nDayIndex].bEnable = atoi(parser.getWord(0).c_str());
				char szDateBuf[256] = {0};//为了解析时间段用
				strcpy(szDateBuf, parser.getWord(1).c_str());
				Convert3GDateType(szDateBuf, strlen(szDateBuf), &pstGPRSCDMAInfo->stSect[nWeekIndex][nDayIndex]);
			}
		}

		p = buf;//解析是否已经被语音或短信激活
		memset(szValueBuf, 0 , 256);
		p = GetProtocolValue(p, "Activate:", "\r\n", szValueBuf, 256);
		if (p)
		{
			if (_stricmp("True", szValueBuf) == 0)
			{
				pstGPRSCDMAInfo->byActivate = 1;
			}
			else
			{
				pstGPRSCDMAInfo->byActivate = 0;
			}
			
		}	
	}
	else
	{
		return NET_ERROR_GETCFG_GPRSCDMA;
	}
	
	return nRet;
}

int  CDevConfig::GetDevConfig_PosCfg( LONG lLoginID , DEVICE_POS_INFO* pPosCfg , int waittime ) 
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pPosCfg == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(pPosCfg, 0, sizeof(DEVICE_POS_INFO));
	int nRetLen = 0;
	int nRet = -1;
	
	DEVICE_POS_INFO stPosCfg;

	memset( &stPosCfg , 0 , sizeof( stPosCfg ) ) ;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_POS, 0, (char *)&stPosCfg, sizeof(DEVICE_POS_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen == sizeof(DEVICE_POS_INFO))
	{
		strcpy( pPosCfg->posAlarmHostAddr , stPosCfg.posAlarmHostAddr ) ;
		pPosCfg->posAlarmHostPort = stPosCfg.posAlarmHostPort ;
		pPosCfg->fontX = stPosCfg.fontX ;
		pPosCfg->fontY = stPosCfg.fontY ;
		pPosCfg->fontColor = stPosCfg.fontColor ;
		pPosCfg->fontSize  = stPosCfg.fontSize ;
		pPosCfg->show      = stPosCfg.show ;
		pPosCfg->holdTime   = stPosCfg.holdTime ;
	}
	else//如果旧的部分都没有就失败
	{
		return NET_ERROR_GET_POS_CFG ;
	}

	return 0 ;
}


int  CDevConfig::SetDevConfig_PosCfg( LONG lLoginID , DEVICE_POS_INFO* pPosCfg , int waittime ) 
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pPosCfg == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRetLen = 0;
	int nRet = -1;
	
	nRet = SetupConfig(lLoginID, CONFIG_TYPE_POS, 0, (char *)pPosCfg , sizeof(DEVICE_POS_INFO), waittime);
	if (nRet < 0)
	{
		nRet = NET_ERROR_SET_POS_CFG;
	}
	
	return nRet;
}


//GPRS/CDMA配置设置
int CDevConfig::SetDevConfig_GPRSCDMACfg(LONG lLoginID, DEVICE_CDMAGPRS_CFG* pstGPRSCDMAInfo, int waittime)
{
	if (NULL == pstGPRSCDMAInfo)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRetLen = 0;
	int nRet = -1;

	bool bSupport = false;
	
	DEV_ENABLE_INFO stDevEn = {0};
	//查看能力
	nRet = GetDevFunctionInfo(lLoginID, ABILITY_DEVALL_INFO, (char*)&stDevEn, sizeof(DEV_ENABLE_INFO), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen > 0)
	{
		if (stDevEn.IsFucEnable[EN_NETWORK_TYPE] != 0)
		{
			bSupport = true;
		}
	}
	
	if (!bSupport)
	{
		return -1;
	}

	//构建字符串
	char szData[4096] = {0};
	int nDataLen = 0;

	sprintf(szData, "iEnable:%d\r\niNetType:%d\r\nszAPN:%s\r\nszDialNum:%s\r\nszUserName:%s\r\nszPWD:%s\r\niKeepLive:%d\r\nActivate:%s\r\n", 
			pstGPRSCDMAInfo->bEnable, pstGPRSCDMAInfo->dwNetType, pstGPRSCDMAInfo->szAPN, pstGPRSCDMAInfo->szDialNum,
			pstGPRSCDMAInfo->szUserName, pstGPRSCDMAInfo->szPWD, pstGPRSCDMAInfo->iKeepLive, pstGPRSCDMAInfo->byActivate==1?"True":"False");

	nDataLen = strlen(szData);

	for (int i = 0; i < DAYS_PER_WEEK; i++)
	{
		for(int j =0; j < TIME_SECT_NUM; j++)
		{
			sprintf((char*)(szData+nDataLen), "TimeSection:%d %d:%d:%d-%d:%d:%d\r\n",  pstGPRSCDMAInfo->stSect[i][j].bEnable,
				pstGPRSCDMAInfo->stSect[i][j].bBeginHour, pstGPRSCDMAInfo->stSect[i][j].bBeginMin, pstGPRSCDMAInfo->stSect[i][j].bBeginSec,
				pstGPRSCDMAInfo->stSect[i][j].bHourEnd, pstGPRSCDMAInfo->stSect[i][j].bEndMin, pstGPRSCDMAInfo->stSect[i][j].bEndSec);
			nDataLen = strlen((char*)(szData));
		}	
	}

	sprintf((char*)(szData+nDataLen), "\r\n");
	nDataLen = strlen((char*)(szData));

	nRet = SetupConfig(lLoginID, CONFIG_TYPE_WLAN, 1, szData, strlen(szData), waittime);

	if (nRet < 0)
	{
		nRet = NET_ERROR_SETCFG_GPRSCDMA;
	}
	
	return nRet;
}


void CDevConfig::GetFunctionMsk(int EnableType, const char *strSource ,DWORD *dwMsk, LONG lLogin, int waittime)
{
	char buf[256]= {0};
	strcpy(buf,strSource);
	char FunctionType[50][20] = {0};
	*dwMsk = 0;

	switch (EnableType)
	{
	case EN_FTP:
		{
			char TmpChar[50][20] = {"Record","Snap"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}			
		break;
	case EN_SMTP:
		{
			char TmpChar[50][20] = {"AlarmText","AlarmSnap"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_NTP:
		{
			char TmpChar[50][20] = {"AdjustSysTime"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_AUTO_MAINTAIN:
		{
			char TmpChar[50][20] = {"Reboot","ShutDown","DeleteFiles"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_VIDEO_COVER:
		{
			char TmpChar[30][20] = {"MutiCover"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_AUTO_REGISTER:
		{
			char TmpChar[50][20] = {"Login"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_DHCP:
		{
			char TmpChar[50][20] = {"RequestIP"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_UPNP:
		{
			char TmpChar[50][20] = {"SearchDevice"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_COMM_SNIFFER:
		{
			char TmpChar[50][20] = {"CommATM"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_NET_SNIFFER:
		{
			char TmpChar[30][20] = {"NetSniffer"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_BURN:
		{
			char TmpChar[50][20] = {"QueryBurnState"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_VIDEO_MATRIX:
		{
			char TmpChar[50][20] = {"HasMatrixBoard"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_AUDIO_DETECT:
		{
			char TmpChar[50][20] = {"AudioDetect"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_STORAGE_STATION:
		{
			char TmpChar[50][20] = {"FTP","SBM","NFS","?","?","?","?","?","?","?","?","?","?","?","?","?",
				                    "DISK","U"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_IPSSEARCH:
		{
			char TmpChar[50][20] = {"IPS"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_SNAP:
		{
			char TmpChar[50][20] ={"SIZE","FREQUENCE","MODE","FORMAT","QUALITY"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_DEFAULTNIC:
		{
			char TmpChar[50][20] ={"DQUERY"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;	
	case EN_SHOWQUALITY:
		{
			char TmpChar[50][20] ={"SHOW"};
			memcpy(&FunctionType,&TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_CONFIG_IMEXPORT:
		{
			char TmpChar[30][20] = {"SetConfig"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_LOG:
		{
			char TmpChar[50][20] = {"PageForPageLog"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_SCHEDULE:
		{
			char TmpChar[50][20] = {"Redundancy", "Prerecord", "Periodinfo"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_NETWORK_TYPE:
		{
			char TmpChar[50][20] = {"NWDEV_ETH0", "NWDEV_WLAN", "NWDEV_3G"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_MARK_IMPORTANTRECORD:
		{
			char TmpChar[50][20] = {"RECID"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}	
		break;
	case EN_ACFCONTROL:
		{
			char TmpChar[50][20] = {"ACF"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_MULTIASSIOPTION:
		{
			char TmpChar[50][20] = {"MultiAssiOption"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_DAVINCIMODULE:
		{
			char TmpChar[50][20] = {"WorkSheetCFGApart", "StandardGOP"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_GPS:
		{
			char TmpChar[30][20] = {"Locate"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_MULTIETHERNET:
		{
			char TmpChar[50][20] ={"Config"};
			memcpy(&FunctionType, &TmpChar,sizeof(TmpChar));
		}
		break;
	case EN_LOGIN_ATTRIBUTE:
		{
			char TmpChar[50][20] = {"Login"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	case EN_RECORD_GENERAL:
		{
			DWORD *lpwMask = dwMsk;
			*lpwMask = 0;
			int nRetLen = 0;
			char pszRequest[512] = {0};
			sprintf(pszRequest, "Method:GetParameterValues\r\nParameterName:Dahua.Device.Record.General\r\n\r\n");
			char pszAck[1024] = {0};
			int nRet = m_pManager->GetDecoderDevice().SysQueryInfo(lLogin, AFK_REQUEST_FUNCTION_MASK, (void*)pszRequest, pszAck, 1024, &nRetLen, waittime);
			if (nRet >= 0 && nRetLen > 0)
			{	
				BYTE bRet = 0;
				char szValue[64] = {0};
				char *p = GetProtocolValue(pszAck, "IsGeneralRecord:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x01 : 0;

				p = GetProtocolValue(pszAck, "IsAlarmRecord:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x02 : 0;

				p = GetProtocolValue(pszAck, "IsMoveDetectRecord:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x04 : 0;

				p = GetProtocolValue(pszAck, "IsLocalStore:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x08 : 0;

				p = GetProtocolValue(pszAck, "IsRemoteStore:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x10 : 0;

				p = GetProtocolValue(pszAck, "IsRedunancyStore:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x20 : 0;

				p = GetProtocolValue(pszAck, "IsLocalurgentStore:", "\r\n", szValue, 64);
				if(p == NULL)
				{
					return;
				}
				bRet = atoi(szValue);
				*lpwMask |= (bRet > 0) ? 0x40 : 0;
			}

			return;
		}
		break;
	case EN_JSON_CONFIG:
		{
			char TmpChar[50][20] = {"Json"};
			memcpy(&FunctionType, &TmpChar, sizeof(TmpChar));
		}
		break;
	default:
		break;
	}

	CStrParse parser;
	parser.setSpliter(",");
	bool bSuccess = parser.Parse(buf);
	if (!bSuccess) 
	{
		return;
	}
	
	for(int i = 0; i < parser.Size(); i++)
	{
		for(int j = 0; j < 50; j++)
		{
			if (0 == _stricmp(parser.getWord(i).c_str(), FunctionType[j]) && 0 != _stricmp(parser.getWord(i).c_str(), ""))
			{
				*dwMsk |= (1<<j);
				break;
			}
		}
	}
}
//End: by cqs(10842)

/*
 *	网络运行状态信息。
 */
int	CDevConfig::QueryNetUserState(LONG lLoginID, DEV_TOTAL_NET_STATE *pstNetUserState, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pstNetUserState == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}

	memset(pstNetUserState,0,sizeof(DEV_TOTAL_NET_STATE));
	int retlen = 0;
	int nRet   =NET_ERROR;
	char szSer[2048] = {0};//用来储存返回信息.

	nRet = QuerySystemInfo(lLoginID, SYSTEM_INFO_NET_STATE, szSer, 2048, &retlen, waittime);
	//the format of szSer  :"hello::admin::7/16 10-47-31::2::0:monitor:440::16:playback:0&&123::admin::7/16 10-49-3::2::0:monitor:440::16:download:0"
	
	//data for test
	//strcpy(szSer,"与::admin::7/16 10-47-31::2::0:monitor:440::16:playback:0");
	if(nRet<0 || retlen == 0)
	{
		return NET_ERROR;
	}
	/*
	if (nRet >= 0 && retlen > 0)
	{
	*/
		bool bResult;
		int  strLen=0;
		int nSize = 0;
		CStrParse parse;
		CStrParse parseNetInfo;
		CStrParse parseChannel;
		CStrParse parseTime;
		parse.setSpliter("&&");
		parseNetInfo.setSpliter("::");
		parseChannel.setSpliter(":");

		bResult = parse.Parse(szSer);
		if( false == bResult)
		{
			return NET_ERROR;
		}
		nSize = parse.Size();
		pstNetUserState->nUserCount = nSize;
		//begin to parse UserNetInfo one by one 
		for(int i=0;i<nSize;i++)
		{
			bResult = parseNetInfo.Parse(parse.getWord(i));
			if(!bResult)
			{
				return NET_ERROR;
			}
			//name
			strLen = parseNetInfo.getWord(0).size();
			if(strLen > 31)
			{
				return NET_ERROR;
			}
			unsigned char szNameBuf[32]={0};
			memcpy(szNameBuf ,parseNetInfo.getWord(0).c_str() ,strLen);
			Change_Utf8_Assic(szNameBuf, pstNetUserState->stuUserInfo[i].szUserName);
			pstNetUserState->stuUserInfo[i].szUserName[31] = '\0';
			//group name
			strLen = parseNetInfo.getWord(1).size();
			if(strLen > 31)
			{
				return NET_ERROR;
			}
			memset(szNameBuf,0,32);
			memcpy(szNameBuf ,parseNetInfo.getWord(1).c_str() ,strLen);
			Change_Utf8_Assic(szNameBuf, pstNetUserState->stuUserInfo[i].szUserGroup);
			pstNetUserState->stuUserInfo[i].szUserGroup[31] = '\0';
			//memcpy(pstNetUserState->stuUserInfo[i].szUserGroup,parseNetInfo.getWord(1).c_str(),strLen);

			//login time  
			//the format of time : "7/16 10-47-31"
			{
				CStrParse parseLeft;
				CStrParse parseRight;
				parseLeft.setSpliter("/");
				parseRight.setSpliter("-");
				parseTime.setSpliter(" ");
				bResult = parseTime.Parse(parseNetInfo.getWord(2));
				if(!bResult)
				{
					return NET_ERROR;
				}
				bResult = parseLeft.Parse(parseTime.getWord(0));
				if(false == bResult)
				{
					return NET_ERROR;
				}
				
				bResult = parseRight.Parse(parseTime.getWord(1));
				if(!bResult)
				{
					return NET_ERROR;
				}
				pstNetUserState->stuUserInfo[i].time.dwMonth = parseLeft.getValue(0);
				pstNetUserState->stuUserInfo[i].time.dwDay = parseLeft.getValue(1);

				pstNetUserState->stuUserInfo[i].time.dwHour = parseRight.getValue(0);
				pstNetUserState->stuUserInfo[i].time.dwMinute  = parseRight.getValue(1);
				pstNetUserState->stuUserInfo[i].time.dwSecond = parseRight.getValue(2);
	
			}//end of  time parse
			
			//channel num
			int nChannelNum = parseNetInfo.getValue(3);
			pstNetUserState->stuUserInfo[i].nOpenedChannelNum = nChannelNum;
			
			for (int j=0;j<nChannelNum;j++)
			{
				//parse channel info
				bResult = parseChannel.Parse(parseNetInfo.getWord(4+j));
				if(!bResult)
				{
					return NET_ERROR;
				}
				//channel num
				pstNetUserState->stuUserInfo[i].channelInfo[j].nChannelNum = parseChannel.getValue(0);
				//channel use type
				strLen = parseChannel.getWord(1).size();
				if(strLen > 31)
				{
					return NET_ERROR;
				}
				memcpy(pstNetUserState->stuUserInfo[i].channelInfo[j].szUseType,parseChannel.getWord(1).c_str(),strLen);
				pstNetUserState->stuUserInfo[i].channelInfo[j].dwStreamSize = parseChannel.getValue(2);
				
			}//end of for(j)
		

		}//end of for(i)

	/*}*/
	return 0;
}

//报警中心
int CDevConfig::GetDevConfig_AlarmCenterCfg(LONG lLoginID, ALARMCENTER_UP_CFG *pAlarmCenterCfg, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pAlarmCenterCfg == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	memset(pAlarmCenterCfg, 0, sizeof(ALARMCENTER_UP_CFG));
	int nRetLen = 0;
	int nRet = -1;

	//旧的报警中心部分
	CONFIG_NET stNetCfg;
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_NET, 0, (char *)&stNetCfg, sizeof(CONFIG_NET), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen == sizeof(CONFIG_NET))
	{
		in_addr tmp ={0};
		tmp.s_addr = stNetCfg.AlarmServerIP;
		strcpy(pAlarmCenterCfg->sHostIPAddr, inet_ntoa(tmp));//ip
		pAlarmCenterCfg->wHostPort = stNetCfg.AlarmServerPort;//port
		pAlarmCenterCfg->byEnable = stNetCfg.AlmSvrStat;//连接使能
	}
	else//如果旧的部分都没有就失败
	{
		return -1;
	}

	//新的部分
	nRetLen = 0;
	nRet = -1;
	ALARM_CENTER_CFG stuAlarmCenterCfg ={0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_CENTER_UP, 0, 
		(char *)&stuAlarmCenterCfg, sizeof(ALARM_CENTER_CFG), &nRetLen, waittime);
	if(nRet >= 0 && nRetLen == sizeof(ALARM_CENTER_CFG))
	{
		pAlarmCenterCfg->nByTimeEn = 1;
		pAlarmCenterCfg->nUploadDay = stuAlarmCenterCfg.iOnTimeUpDay;
		pAlarmCenterCfg->nUploadTime = stuAlarmCenterCfg.iOnTimeUpHour;
	}
	
	return 0;//只要旧的部分有就成功，新的部分用nEnable表示了。
}

int CDevConfig::SetDevConfig_AlarmCenterCfg(LONG lLoginID, ALARMCENTER_UP_CFG *pAlarmCenterCfg, int waittime)
{
	afk_device_s* device = (afk_device_s *)lLoginID;
	if (device == NULL || pAlarmCenterCfg == NULL)
	{
		return NET_ILLEGAL_PARAM;
	}
	
	int nRetLen = 0;
	int nRet = -1;
	
	//旧的报警中心部分
	CONFIG_NET stNetCfg = {0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_NET, 0, (char *)&stNetCfg, sizeof(CONFIG_NET), &nRetLen, waittime);
	if (nRet >= 0 && nRetLen == sizeof(CONFIG_NET))
	{
		stNetCfg.AlarmServerIP = inet_addr(pAlarmCenterCfg->sHostIPAddr);//ip
		stNetCfg.AlarmServerPort = pAlarmCenterCfg->wHostPort;//port
		stNetCfg.AlmSvrStat = pAlarmCenterCfg->byEnable;//连接使能
		nRet = SetupConfig(lLoginID, CONFIG_TYPE_NET, 0, (char *)&stNetCfg, sizeof(CONFIG_NET), waittime);
		if(nRet < 0)//如果失败，不进行下面的处理
		{
			return nRet;
		}
	}

	//先查询一下设备有没有使能。
	nRetLen = 0;
	nRet = -1;
	ALARM_CENTER_CFG stuAlarmCenterCfg ={0};
	nRet = QueryConfig(lLoginID, CONFIG_TYPE_ALARM_CENTER_UP, 0, 
		(char *)&stuAlarmCenterCfg, sizeof(ALARM_CENTER_CFG), &nRetLen, waittime);
	if(nRet >= 0 && nRetLen == sizeof(ALARM_CENTER_CFG))//如果有能力
	{	
		stuAlarmCenterCfg.iOnTimeUpDay = pAlarmCenterCfg->nUploadDay;
		stuAlarmCenterCfg.iOnTimeUpHour = pAlarmCenterCfg->nUploadTime;

		SetupConfig(lLoginID, CONFIG_TYPE_ALARM_CENTER_UP, 0, (char *)&stuAlarmCenterCfg, sizeof(ALARM_CENTER_CFG), waittime);
	}
	return 0;//成功 
}



int __stdcall QueryTransComFunc(
								afk_handle_t object,	/* 数据提供者 */
								unsigned char *data,	/* 数据体 */
								unsigned int datalen,	/* 数据长度 */
								void *param,			/* 回调参数 */
								void *udata)
{
    receivedata_s *receivedata = (receivedata_s*)udata;
    if (!receivedata || false == receivedata->addRef())
    {
        return -1;
    }
	
	if (!receivedata->datalen || !receivedata->data)
	{
		SetEventEx(receivedata->hRecEvt);
        return -1;
	}
	
	receivedata->result = 0;
	memcpy(receivedata->data, data, datalen);

    SetEventEx(receivedata->hRecEvt);
    return 1; 
}

/* 查询串口参数 */
int		CDevConfig::QueryTransComParams(LONG lLoginID, 
										 int TransComType,
										 COMM_STATE* pCommState,
										 int nWaitTime)
{
	
	   if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
	   {
		   return NET_INVALID_HANDLE;
	   }
	   
	   if (NULL == pCommState)
	   {
		   return NET_ILLEGAL_PARAM;
	   }
	   
	   int nRet = -1;
	   
	   afk_device_s *device = (afk_device_s*)lLoginID;
	   
	   afk_search_channel_param_s searchchannel = {0};
	   searchchannel.type = AFK_CHANNEL_SEARCH_TRANSCOM;
	   searchchannel.subtype = TransComType;
	   searchchannel.param = TransComType;
	   searchchannel.base.func = QueryTransComFunc;
	   
	   char szRecBuf[256] = {0};
	   int nRetLen = 0;
	   receivedata_s receivedata;// = {0};
	   receivedata.data = szRecBuf;
	   receivedata.datalen = &nRetLen;
	   receivedata.maxlen = 256;
	   //receivedata.hRecEvt = m_hRecEvent;
	   receivedata.result = -1;
	   if (nWaitTime == 0)
	   {
		   searchchannel.base.udata = 0;
	   }
	   else
	   {
		   searchchannel.base.udata = &receivedata;
	   }
	   
	   afk_channel_s *pchannel = (afk_channel_s*)device->open_channel(device, 
		   AFK_CHANNEL_TYPE_SEARCH, &searchchannel);
	   if (pchannel)
	   {
		   if (nWaitTime != 0)
		   {
			   DWORD dwRet = WaitForSingleObjectEx(receivedata.hRecEvt, nWaitTime);
			   pchannel->close(pchannel);
			   ResetEventEx(receivedata.hRecEvt);
			   if (dwRet == WAIT_OBJECT_0)
			   {    
				   if (receivedata.result == -1)
				   {
					   nRet = NET_RETURN_DATA_ERROR;
				   }
				   else if(receivedata.result == 0)
				   {
					   memset(pCommState, 0, sizeof(COMM_STATE));
					   char szValue[64] = {0};
					   char *p = GetProtocolValue(szRecBuf, "States:", "\r\n", szValue, 64);
					   if(p == NULL)
					   {
						   return NET_ERROR_SEARCH_TRANSCOM;
					   }
					   pCommState->uBeOpened = (atoi(szValue) != 0) ? 1 : 0;
					   if(pCommState->uBeOpened > 0)
					   {
						   p = GetProtocolValue(szRecBuf, "BaudBase:", "\r\n", szValue, 64);
						   if(p == NULL)
						   {
							   return NET_ERROR_SEARCH_TRANSCOM;
						   }
						   pCommState->uBaudRate = atoi(szValue);

						   p = GetProtocolValue(szRecBuf, "DataBits:", "\r\n", szValue, 64);
						   if(p == NULL)
						   {
							   return NET_ERROR_SEARCH_TRANSCOM;
						   }
						   pCommState->uDataBites = atoi(szValue);

						   p = GetProtocolValue(szRecBuf, "StopBits:", "\r\n", szValue, 64);
						   if(p == NULL)
						   {
							   return NET_ERROR_SEARCH_TRANSCOM;
						   }
						   pCommState->uStopBits = atoi(szValue);
						   
						   p = GetProtocolValue(szRecBuf, "Parity:", "\r\n", szValue, 64);
						   if(p == NULL)
						   {
							   return NET_ERROR_SEARCH_TRANSCOM;
						   }
						   pCommState->uParity = atoi(szValue);
					   }
					   nRet = 0;
				   }
			   }
			   else
			   {
				   nRet = NET_NETWORK_ERROR;
			   }
		   }
		   else
		   {
			   nRet = 0;
		   }
	   }
	   else
	   {
		   nRet = NET_OPEN_CHANNEL_ERROR;
	   }
	   
	   return nRet;
	
}






