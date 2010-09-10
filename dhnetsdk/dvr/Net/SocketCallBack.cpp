
#include "../def.h"
#include "../dvrdevice/dvrdevice.h"
#include "../dvrdevice/dvrchannel.h"
#include "../ParseString.h"
#include "SocketCallBack.h"


/*
 * 摘要：主连接接收数据处理
 */
int __stdcall OnOtherPacket(unsigned char *pBuf, int nLen, void *userdata)
{
    if (nLen < HEADER_SIZE)
    {
        return -1;	// 包出错
    }

    CDvrDevice *device = (CDvrDevice*)userdata;
    CDvrChannel *pDvrChannel = NULL;

	try
	{
	if (NULL == device)
	{
		return -1;  // 系统出错
	}

    /************************************************************************/

    // 实时监视包
    if (0xBC == (unsigned char)*pBuf)
    {
        int channel = (unsigned char)*(pBuf + 8);

		// 预览通道号 = 总的通道号
        if (channel == device->GetVideoChannelCount())
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_PREVIEW, channel);
        }
		else if (10 == pBuf[24]) // 抓图
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CAPTURE, channel);
		}
        else
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_MEDIA, channel);
        }

        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
        }
    }

    // 下载包
    else if (0xBB == (unsigned char)*pBuf)
    {
        int channel = (unsigned char)*(pBuf + 8);
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_DOWNLOAD, AFK_CHANNEL_DOWNLOAD_RECORD, channel-1);
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			
            // 结束下载
            if (*(unsigned int*)(pBuf + 15) == 0x12 ||
                *(unsigned int*)(pBuf + 4) == 0x00)
            {
                pDvrChannel->OnRespond(0, -1);
            }
        }
    }

	// 导出配置文件
	else if (0xF3 == (unsigned char)*pBuf && (0x02 == (unsigned char)*(pBuf + 8) || 0xFF == (unsigned char)*(pBuf + 8)))
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_DOWNLOAD, AFK_CHANNEL_DOWNLOAD_CONFIGFILE, 16);

		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

    // 报警状态包 and record state
    else if (0xB1 == (unsigned char)*pBuf)
    {
        if (nLen == HEADER_SIZE)
        {
			//if querying
			if (pBuf[8] != 0x0A && pBuf[8] != 0x00)//0x0a为查询音频输入状态,0x00为报警或查询报警
			{
				pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE);
				if (pDvrChannel)
				{
					pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
					pDvrChannel->channel_decRef();
				}
			}
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE,0x10);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
				pDvrChannel->channel_decRef();
			}
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE,0x12);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
				pDvrChannel->channel_decRef();
			}

			//alarm packet
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_ALARM, 0);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
        }
		else if ((nLen > HEADER_SIZE + 10) &&
			((pBuf[13] == 0xbf) || (pBuf[13] == 0x7f))) // 小票信息或者小票裸数据
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_ALARM, 0);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
		}
		else if (0x04 == pBuf[8])
		{
			//burning progress
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
		}
		else if (0x06 == pBuf[8]) 
		{
			//shelter alarm
 			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_ALARM, 0);
            if (pDvrChannel)
            {
                pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
            }	
		}
		else if (0x03 == pBuf[8] || 0x08 == pBuf[8] || 0x09 == pBuf[8])
		{
			//alarm packet-disk full, disk error
 			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_ALARM, 0);
            if (pDvrChannel)
            {
                pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
            }	
		}
        else if (0x01 == pBuf[8])
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_RECORDSTATE);
            if (pDvrChannel)
            {
                pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
            }
        }
		else if (0x0b == pBuf[8])
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char *)pBuf, nLen);
			}
		}
		else if (0x0c == pBuf[8])
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char *)pBuf, nLen);
			}
		}
		else if (0x0d == pBuf[8])//无线网络信号强度
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char *)pBuf, nLen);
			}
		}
		else
		{
#ifdef _DEBUG
			OutputDebugString(" UnKnown subCmd in Cmd 0xb1 \n");		
#endif
		}
    }
	else if (0x69 == pBuf[0] && nLen > HEADER_SIZE) //==HEADER_SIZE是订阅返回包，不做处理。
	{
		//gps报警信息
		if(pBuf[8] == 2 &&  (*(unsigned char *)(pBuf+12)) == 155)
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_GPS,0);
			if(pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char *)pBuf,nLen);
				pDvrChannel->channel_decRef();
			}
		}
		//if querying
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE,0x11);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			pDvrChannel->channel_decRef();
		}
		//alarm packet
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_ALARM, 0);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}
	else if(0x69 == pBuf[0])// && 0xa1 == pBuf[12] && nLen == HEADER_SIZE)
	{
		//alarm packet
		if(0xa1 == (unsigned char)pBuf[12] && 0x01 == (unsigned char)pBuf[8])// && nLen == HEADER_SIZE)
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_ALARM, 0);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
		}
	}
	
    // 录像文件,报警文件,卡号文件
    else if (0xB6 == (unsigned char)*pBuf || 0xB7 == (unsigned char)*pBuf 
        || 0xE5 == (unsigned char)*pBuf)
    {
        //0为查询类型
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_RECORD);
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

    // 日志信息
    else if (0xB2 == (unsigned char)*pBuf)
    {
        //1为查询类型
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_LOG, pBuf[16]);
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

    // 通道名称
    else if (0xB8 == (unsigned char)*pBuf)
    {
        //2为查询类型
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_CHANNELNAME);
		//通过0xA8查询
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
			pDvrChannel->channel_decRef();
        }
		
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_CONFIG, CONFIG_TYPE_TITLE);
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

    // 透明串口
    else if (0x20 == (unsigned char)*pBuf)
    {
        int trans_device_type = (unsigned char)*(pBuf + 8);
		int nCmdType = (unsigned char)*(pBuf + 9);
		if(nCmdType != 5)
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_TRANS, trans_device_type);

			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
		}
		else 
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_TRANSCOM, trans_device_type);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
			}
		}
    }

    // 语音对讲
    else if (0xC0 == (unsigned char)*pBuf)
    {
//      unsigned char respond = (unsigned char)*(pBuf + 8);
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_TALK, 0);
        if (pDvrChannel)
        {
			//delete by zhaojs 10-28 会导致错误
            //pDvrChannel->OnRespond((unsigned char*)respond, 0);
            /*//接收对讲
            if (respond == 1)
            {       
                pDvrChannel->OnRespond((unsigned char*)respond, 0);
            }
            //拒绝对讲(2)
            else
            {
                pDvrChannel->OnRespond((unsigned char*)respond, 0);
            }*/
        }
    }

    // 语音对讲数据
    else if (0x1D == (unsigned char)*pBuf)
    {
        //int id = pBuf[12];
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_TALK, 0);
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
        }
    }

    // 报警设置查询与设置
    else if (0xE3 == (unsigned char)*pBuf)
    {
        if ((unsigned char)*(pBuf + 8) == 0x00)
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_ALARMCTRL_IN);
        }
        else if ((unsigned char)*(pBuf + 8) == 0x01)
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_ALARMCTRL_OUT);
        }
        else if ((unsigned char)*(pBuf + 8) == 0x02)
        {
			//暂不处理
            //pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 0);
        }
        else if ((unsigned char)*(pBuf + 8) == 0x03)
        {
			//暂无意义
            //pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CONTROL, 0);
        }
		else if((unsigned char)*(pBuf + 8) == 0x05)
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_ALARMTRRIGER_MODE);
		}
		
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
        }
    }

    // 设备时间
    else if (0x24 == (unsigned char)*pBuf)
    {
		unsigned char subcmd = pBuf[8];
        pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_DEVTIME);
        if (pDvrChannel && (0 == subcmd))
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

	// 强制I帧
	else if (0x80 == (unsigned char)*pBuf)
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CONFIG, 19);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

	// 设置限制码流
	else if (0x81 == (unsigned char)*pBuf)
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CONFIG, 20);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

    // 串口协议与解码器协议
    else if (0xE4 == (unsigned char)*pBuf)
    {
        int protocoltype = *(unsigned char*)(pBuf + 8);
        if (protocoltype == 0)
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_COMMPROTOCOL);
        }
        else if (protocoltype == 1)
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_DCDPROTOCOL);
        }
        else if(protocoltype == 2)
		{
			pDvrChannel =device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH,AFK_CHANNEL_SEARCH_PTZ);
		}
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

    // 系统信息
    else if (0xB4 == (unsigned char)*pBuf)
    {
		int systemtype = *(unsigned char*)(pBuf + 8);

		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_SYSTEM_INFO, systemtype);
		/*
        if (systemtype >= 0 && systemtype <= 6)
        {
            pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 24 + systemtype);
        }
		else if (7 == systemtype)
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 33);
		}
		else if (11 == systemtype)
		{
			pDvrChannel = device->GetChannel(AFK_CHANNEL_TYPE_SEARCH, 43);
		}
       */

        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

    // 配置返回
    else if (0xB3 == (unsigned char)*pBuf && 0xF7 != (unsigned char)*(pBuf+16))
    {
        int configtype = *(unsigned char*)(pBuf + 16);

		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_CONFIG, configtype);

	//	if (200 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 32);
	//	}
	//	else if (23 == configtype) //音频报警配置
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 36);
	//	}
	//	else if (126 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 40);
	//	}
	//	else if (127 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 35);
	//	}
	//	else if (128 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 38);
	//	}
	//	else if (129 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 39);
	//	}
	//	else if (123 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, CONFIG_TYPE_RECORD_NEW);
	//	}
	//	else 
		if (124 == configtype)
		{
			if (pDvrChannel != NULL)
			{
				pDvrChannel->channel_decRef();
			}
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_CONFIG, pBuf[24]+CONFIG_TYPE_ALARM_BEGIN);
		}
		else if (125 == configtype)
		{
			if (pDvrChannel != NULL)
			{
				pDvrChannel->channel_decRef();
			}
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_CONFIG, CONFIG_TYPE_TIMESHEET);
		}
	//	else if (17 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 41);
	//	}
	//	else if (18 == configtype)
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 42);
	//	}
	//	else
	//	{
	//		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, 5 + configtype);
	//	}

        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

    // 用户操作返回
    else if (0xB5 == (unsigned char)*pBuf)
    {
        int useroperatetype = *(unsigned char*)(pBuf + 8);	
		if (11 == useroperatetype)
		{
			//查询在线用户状态
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVSTATE);
		}
		else
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_USER, useroperatetype);
		}
        
        if (pDvrChannel)
        {
            pDvrChannel->OnRespond((unsigned char*)pBuf , nLen);
        }
    }

	// 查询设备工作状态 & DDNS查询IP返回
	else if (0x82 == (unsigned char)*pBuf)
	{
		//DDNS查询IP返回
		if (device->GetType() == PRODUCT_DVR_DDNS && pBuf[8] == 50)
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_DDNSIP_BYNAME);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
		}
		//查询设备工作状态
		else
		{
			pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHENNEL_SEARCH_DEVWORKSTATE);
			if (pDvrChannel)
			{
				pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
			}
		}
	}

	// 设备控制（重启或关闭）返回 Added by Linjy-2006-11-1
	// 硬盘控制返回 Linjy-2006-12-18
	else if (0x60 == (unsigned char)*pBuf)
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CONTROL, 0);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	} 

	// 设备升级返回 Added by Linjy-2006-11-8
	else if (0XBA == (unsigned char)*pBuf)
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_UPGRADE, AFK_CHANNEL_UPLOAD_UPGRADE);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

	// 导入配置文件
	else if (0xF3 == (unsigned char)*pBuf && 0x03 == (unsigned char)*(pBuf + 8))
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_UPGRADE, AFK_CHANNEL_UPLOAD_CONFIGFILE);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

	// 获取通道编码格式和分辨率的掩码返回 Added by Linjy-2006-11-7
	else if (0x83 == (unsigned char)*pBuf)
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_SEARCH, AFK_CHANNEL_SEARCH_CONFIG, CONFIG_TYPE_DSP_MASK);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

	else if (0xC1 == (unsigned char)*pBuf && 0xF7 != (unsigned char)*(pBuf+16)) 
	{
		// 如果ACK信息里需要设备重启，当作一种事件来处理
		if (0x01 == (unsigned char)*(pBuf+9))
		{
			fDeviceEventCallBack cbDevEvent = device->GetDevEventFunc();
			void* userdata = device->GetDisconnectUserdata();
			
			if (cbDevEvent != NULL)
			{
				cbDevEvent((afk_device_s*)device, EVENT_CONFIG_REBOOT, NULL, userdata);
			}
		}
		
		if((BYTE)*(pBuf+16) == 242)//设备配置发生改变
		{
			fDeviceEventCallBack cbDevEvent = device->GetDevEventFunc();
			void* userdata = device->GetDisconnectUserdata();

			if (cbDevEvent != NULL)
			{
				cbDevEvent((afk_device_s*)device, EVENT_CONFIG_CHANGE, NULL, userdata);
			}
		}
		else
		{
			// 回调配置返回码
			CONFIG_ACK_INFO stuAckInfo = {0};
			stuAckInfo.type = *(pBuf+16);		// 修改参数类别
			stuAckInfo.nResultCode = *(pBuf+8);	// 返回码
			stuAckInfo.bReboot = *(pBuf+9);		// 重启标志

			fDeviceEventCallBack cbDevEvent = device->GetDevEventFunc();
			void* userdata = device->GetDisconnectUserdata();
			
			if (cbDevEvent != NULL)
			{
				cbDevEvent((afk_device_s*)device, EVENT_CONFIG_ACK, &stuAckInfo, userdata);
			}
		}

		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CONFIG, 0);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

	// 如果ACK信息里有设备发起的语音对讲，当作一种事件来处理
	else if (0x1E == (unsigned char)*pBuf)
	{
		//语音对讲 2开始 3结束
		fDeviceEventCallBack cbDevEvent = device->GetDevEventFunc();
		void* userdata = device->GetDisconnectUserdata();
		
		if (cbDevEvent != NULL)
		{
			if (2 == (unsigned char)*(pBuf+8))
			{
				cbDevEvent((afk_device_s*)device, EVENT_TALK_START, NULL, userdata);
			}
			else if (3 == (unsigned char)*(pBuf+8))
			{
				cbDevEvent((afk_device_s*)device, EVENT_TALK_STOP, NULL, userdata);
			}
		}
	}

	else if (0x1A == (unsigned char)*pBuf || 0x1B == (unsigned char)*pBuf)
	{
		pDvrChannel = device->device_get_channel(AFK_CHANNEL_TYPE_CONTROL, 0);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}

	else if (0xf1 == (unsigned char)*pBuf)
	{
		//子连接注册返回。暂不处理
	}

	// 字符串协议
	else if ((0xC1 == (unsigned char)*pBuf || 0xB3 == (unsigned char)*pBuf) && 0xF7 == (unsigned char)*(pBuf+16) && nLen > HEADER_SIZE)
	{
		int nRequestType = 0;
		int nPacketSequence = 0;

		pBuf[nLen-1] = '\0';
		pBuf[nLen-2] = '\0';

		char szValue[64] = {0};
		char *p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "ProtocolType:", "\r\n", szValue, 64);
		if (p)
		{
			if (_stricmp(szValue, "CfgTVCombin") == 0 && 0xC1 == (unsigned char)*pBuf)
			{
				nRequestType = AFK_REQUEST_DECODER_CTRLTV;
			}
			else if (_stricmp(szValue, "CfgDigitalChn") == 0 && 0xC1 == (unsigned char)*pBuf)
			{
				nRequestType = AFK_REQUEST_DECODER_SWITCH;
			}
			else if (_stricmp(szValue, "CfgTVCombin") == 0 && 0xB3 == (unsigned char)*pBuf)
			{
				nRequestType = AFK_REQUEST_DEC_QUERY_TV;
			}
			else if (_stricmp(szValue, "CfgDigitalChn") == 0 && 0xB3 == (unsigned char)*pBuf)
			{
				nRequestType = AFK_REQUEST_DEC_QUERY_CHANNEL;
			}
		}

		p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "PacketSequence:", "\r\n", szValue, 64);
		if (p)
		{
			nPacketSequence = atoi(szValue);
		}
	
		pDvrChannel = device->device_get_channel(AFX_CHANNEL_TYPE_REQUEST, nRequestType, nPacketSequence);
		if (pDvrChannel)
		{
			pDvrChannel->channel_set_info(channel_protocoltype, &nRequestType);
			pDvrChannel->channel_set_info(channel_packetsequence, &nPacketSequence);
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}
	else if (0xF4 == (unsigned char)*pBuf)
	{
		int nRequestType = 0;
		int nPacketSequence = 0;

		pBuf[nLen-1] = '\0';
		pBuf[nLen-2] = '\0';

		char szValue[64] = {0};
		char *p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "ParameterName:", "\r\n", szValue, 64);
		if (p)
		{
			char szMethod[64] = {0};
			p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "Method:", "\r\n", szMethod, 64);
			if (p)
			{
				if (_stricmp(szValue, "Dahua.Device.VideoOut.TV.CfgTv") == 0)
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_QUERY_SYS;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.Cfg") == 0)
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_QUERY_CHNNUM;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.VideoOut.TV.MultiCombin") == 0)
				{
					if (_stricmp(szMethod, "AddObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_ADD_COMBIN;
					}
					else if (_stricmp(szMethod, "DeleteObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_DEL_COMBIN;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_SET_COMBIN;
					}
					else if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_QUERY_COMBIN;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.VideoOut.TV.Scout.CfgScout") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_SET_TOUR;
					}
					else if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_QUERY_TOUR;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.VideoOut.TV.CfgTv") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_SET_TV_ENABLE;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.ControlPlayback.Start") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_PLAYBACKBYDEC;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.ControlPlayback.Progress") == 0)
				{
					if (_stricmp(szMethod, "Inform") == 0)
					{
						NVD_PLAYBACK_EVENT stuAckInfo = {0};

						p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "DisChn:", "\r\n", szValue, 64);
						if (p)
						{
							stuAckInfo.nEncoderNum = atoi(szValue);
						}
						
						p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "RecTotalLength:", "\r\n", szValue, 64);
						if (p)
						{
							stuAckInfo.nTotalSize = atoi(szValue);
						}
							
						p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "CurrentLength:", "\r\n", szValue, 64);
						if (p)
						{
							stuAckInfo.nCurSize = atoi(szValue);
						}

						fDeviceEventCallBack cbDevEvent = device->GetDevEventFunc();
						void* userdata = device->GetDisconnectUserdata();
						if (cbDevEvent != NULL)
						{
							cbDevEvent((afk_device_s*)device, EVENT_NVD_PLAYBACK, &stuAckInfo, userdata);
						}
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.ControlPlayback.Resume") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_CTRLPLAYBACK;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.ControlPlayback.Pause") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_CTRLPLAYBACK;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.ControlPlayback.Stop") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_CTRLPLAYBACK;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Network.IPFilter") == 0) //ip过滤
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_IPFILTER;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_IPFILTER;
					}
					else if (_stricmp(szMethod, "AddObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_ADD_IPFILTER;
					}
					else if (_stricmp(szMethod, "DeleteObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEL_IPFILTER;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Audio.Talkback.Cfg") == 0)//语音编码配置
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_AMRENCODE_CFG;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_AMRENCODE_CFG;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Record.General.RecordLen") == 0)//录像长度配置
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_RECORDLEN_CFG;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_RECORDLEN_CFG;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.VideoOut.General") == 0)//视频制式
				{
					if (_stricmp(szMethod, "GetParameterNamesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_VIDEOFORMAT_INFO;
					}
					else if (_stricmp(szMethod, "SetParameterNamesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_VIDEOOUT;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Network.Mobile.EventMessageSending.Cfg") == 0)//MMS配置
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_MMS_INFO;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_MMS_INFO;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Network.Mobile.SMSActivation.Cfg") == 0)//SMSACTIVATION配置
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SMSACTIVE_INFO;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_SMSACTIVE_INFO;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Network.Mobile.DialInActivation.Cfg") == 0)//DIALINACTIVATION配置
				{
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_DIALIN_INFO;
					}
					else if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_DIALIN_INFO;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Record.Backup.Extra") == 0)//司法刻录
				{
					if( _stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_BURNFILE_TRANS;
					}
					else if( _stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_BURNFILE_TRANS;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Network.ControlConnection.Passive") == 0)
				{
					if( _stricmp(szMethod, "AddObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_CONNECT;
					}
					else if( _stricmp(szMethod, "DeleteObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DISCONNECT;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Network.ControlConnection.Port") == 0)
				{
					if( _stricmp(szMethod, "AddObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_AUTOREGISTER_CONNECT;
					}
					else if( _stricmp(szMethod, "DeleteObjectResponse") == 0)
					{
						nRequestType = AFK_REQUEST_AUTOREGISTER_DISCONNECT;
					}
					else if ( _stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_AUTOREGISTER_GETSERVER;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Network.CtrolRegister.Choose") == 0)//控制注册
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_AUTOREGISTER_REGSERVER;
					}
					if (_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_AUTOREGISTER_GETCONID;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.Backup.Extra.Option") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_BURN_ATTACH;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_BURN_ATTACH;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Network.Sniffer.Cfg.Group") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SNIFFER;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_SNIFFER;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Network.Sniffer.Cfg.Group.Frame") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SNIFFER;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_SNIFFER;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Network.Sniffer.Cfg.Group.Frame.Content") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SNIFFER;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_SNIFFER;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.NetWork.DownloadBitrate.Cfg") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_DOWNLOAD_RATE;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_DOWNLOAD_RATE;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.ChannelState") == 0)//解码通道的状态信息
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_CHANNEL_STATE;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Oem.JunYi.Intelligent.General") == 0)
				{
					if(_stricmp(szMethod, "GetParameterNamesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_POINT_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterNamesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_POINT_CFG;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Oem.WeiKeMu.Video.OSD") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_OSDENABLE_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_OSDENABLE_CFG;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.VideoOut.TV.Cfg.Adjust") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_TVADJUST_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_TVADJUST_CFG;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.General") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_FUNCTION_MASK;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.OnVehicle.General") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_GET_VEHICLE_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_VEHICLE_CFG;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.ATM.Overlap.General") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_ATM_OVERLAY_ABILITY;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.ATM.Overlap.Config") == 0)
				{
					if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_ATM_OVERLAY_CFG;
					}
					else if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_ATM_OVERLAY_CFG;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.Backup.General") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_QUERY_BACKUP_DEV;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.Backup.Device") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_QUERY_BACKUP_DEV_INFO;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.Backup.Control.Start") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_BACKUP_START;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.Backup.Control.Stop") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_BACKUP_STOP;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Record.Backup.Control.Progress") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_BACKUP_FEEDBACK;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.ATM.General") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{	
						nRequestType = AFK_REQUEST_ATM_TRADE_TYPE;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.Decode.TourCfg") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_DECODER_TOUR_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_DECODER_TOUR_CFG;
					}
				}
				else if (_stricmp(szValue, "Dahua.Device.Decode.Control") == 0)
				{
					if (_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_DEC_CONTROL;		
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.StreamMedia.Info") == 0)
				{
					if(_stricmp(szMethod, "GetParameterNamesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SIP_ABILITY;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.StreamMedia.SIP.Cfg") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SIP_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_SIP_CFG;
					}
				}
				else if(_stricmp(szValue, "Dahua.Device.StreamMedia.SIP.State") == 0)
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_SIP_STATE;
					}
				} //jfchen 
				else if ( _stricmp( szValue , "ZenoIntel.Device.OnPos.Pos.Cfg" ) == 0 )
				{
					if(_stricmp(szMethod, "GetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_GET_POS_CFG;
					}
					else if(_stricmp(szMethod, "SetParameterValuesResponse") == 0)
					{
						nRequestType = AFK_REQUEST_SET_POS_CFG;
					}
				}
			}
		}

		p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "TransactionID:", "\r\n", szValue, 64);
		if (p)
		{
			nPacketSequence = atoi(szValue);
		}
	
		pDvrChannel = device->device_get_channel(AFX_CHANNEL_TYPE_REQUEST, nRequestType, nPacketSequence);
		if (pDvrChannel)
		{
			pDvrChannel->channel_set_info(channel_protocoltype, &nRequestType);
			pDvrChannel->channel_set_info(channel_packetsequence, &nPacketSequence);
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}
	else if (0xf5 == (unsigned char)*pBuf)
	{
		int nPacketSequence = 0;
		nPacketSequence = *(int*)(pBuf+8);
		pDvrChannel = device->device_get_channel(AFX_CHANNEL_TYPE_NEWCONFIG, 0, nPacketSequence);
		if (pDvrChannel)
		{
			pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);
		}
	}
	else
	{
#ifdef _DEBUG
	//	printf("Error:命令字:(%x )没有定义-->OnOtherPacket\n",pBuf[0]);
#endif
	}
	
	}
    catch (...)
    {
#ifdef _DEBUG
		OutputDebugString("Error:处理命令出现异常-->OnOtherPacket\n");
#endif
	return -1;
	}
	
	if (pDvrChannel)
	{
		pDvrChannel->channel_decRef();
	}

	return 1;
}


/*
 * 摘要：主连接数据码流统计
 */
int __stdcall OnReceivePacket(unsigned char *pBuf, int nLen, void *userdata)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)userdata;
	if (pDvrDevice == NULL) 
	{
		return -1; 
	}

    pDvrDevice->statiscing(nLen);

	return 1;
}


/*
 * 摘要：主连接断线
 */
int __stdcall OnDisconnect(void *userdata)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)userdata;
	if (pDvrDevice != NULL) 
	{
		int nIsHaveTask = 0;
		pDvrDevice->set_info(pDvrDevice, dit_device_task, &nIsHaveTask);

		fDisconnectCallBack pfDisconnectFunc = pDvrDevice->GetDisconnectFunc();
		void* userdata = pDvrDevice->GetDisconnectUserdata();
		if (pfDisconnectFunc != NULL)
		{
			pfDisconnectFunc((afk_device_s*)pDvrDevice, NULL, FALSE, pDvrDevice->GetIp(), pDvrDevice->GetPort(), 0, userdata);
		}
	}
	
	return 1;
}


/*
 * 摘要：主连接断线恢复
 */
int	__stdcall OnReConnect(void *userdata)
{
	CDvrDevice *pDvrDevice = (CDvrDevice*)userdata;
	if (pDvrDevice != NULL) 
	{
		int nIsHaveTask = 1;
		pDvrDevice->set_info(pDvrDevice, dit_device_task, &nIsHaveTask);
	}

	return 1;
}


/*
 * 摘要：子连接接收数据处理
 */
int __stdcall OnSubOtherPacket(unsigned char *pBuf, int nLen, void *userdata)
{
	CDvrChannel *pDvrChannel = (CDvrChannel*)userdata;
	if (nLen < HEADER_SIZE || pDvrChannel == NULL)
    {
        return -1;
    }

    if (0xB1 != (unsigned char)*pBuf)
    {
		pDvrChannel->OnRespond((unsigned char*)pBuf, nLen);

		if (0xBB == (unsigned char)*pBuf)
		{
			// 结束下载
            if (*(unsigned int*)(pBuf + 15) == 0x12 ||
                *(unsigned int*)(pBuf + 4) == 0x00)
            {
#ifdef _DEBUG
				OutputDebugString("DVR: DOWNLOAD END!\n");
#endif
                pDvrChannel->OnRespond(0, -1);
            }
		}
    }

	return 1;
}


/*
 * 摘要：子连接数据码流统计
 */
int __stdcall OnSubReceivePacket(unsigned char *pBuf, int nLen, void *userdata)
{
	CDvrChannel *pDvrChannel = (CDvrChannel*)userdata;
	if (nLen < HEADER_SIZE || pDvrChannel == NULL)
    {
        return -1;
    }

	CDvrDevice *pDvrDevice = pDvrChannel->channel_getdevice();
	if (pDvrDevice == NULL) 
	{
		return -1; 
	}

    pDvrDevice->statiscing(nLen);

	return 1;
}


/*
 * 摘要：子连接断线
 */
int __stdcall OnSubDisconnect(void *userdata)
{
	CDvrChannel *pDvrChannel = (CDvrChannel*)userdata;
	if (pDvrChannel == NULL)
    {
        return -1;
    }

	CDvrDevice *pDvrDevice = pDvrChannel->channel_getdevice();
	if (pDvrDevice == NULL) 
	{
		return -1; 
	}

	if (pDvrDevice != NULL) 
	{
		int nIsHaveTask = 0;
		pDvrDevice->set_info(pDvrDevice, dit_device_task, &nIsHaveTask);

		fDisconnectCallBack pfDisconnectFunc = pDvrDevice->GetDisconnectFunc();
		void* userdata = pDvrDevice->GetDisconnectUserdata();
		if (pfDisconnectFunc != NULL)
		{
			pfDisconnectFunc((afk_device_s*)pDvrDevice, pDvrChannel, FALSE, pDvrDevice->GetIp(), pDvrDevice->GetPort(), pDvrChannel->GetChannelType(), userdata);
		}
	}
	
	return 1;
}


/*
 * 摘要：子连接断线恢复
 */
int	__stdcall OnSubReConnect(void *userdata)
{
// 	CDvrChannel *pDvrChannel = (CDvrChannel*)userdata;
// 	if (pDvrChannel == NULL)
//     {
//         return -1;
//     }
// 
// 	CDvrDevice *pDvrDevice = pDvrChannel->channel_getdevice();
// 	if (pDvrDevice == NULL) 
// 	{
// 		return -1; 
// 	}
// 	
// 	if (pDvrDevice != NULL) 
// 	{
// 		int nIsHaveTask = 1;
// 		pDvrDevice->set_info(pDvrDevice, dit_device_task, &nIsHaveTask);
// 	}

	return 1;
}






























