
#include "dvrsearchchannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include "dvrpacket_dvr2.h"


CDvrSearchChannel::CDvrSearchChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_search_channel_parm = *(afk_search_channel_param_s*)pParam;
    m_nLogSize = 0;
    m_nLogRecSize = 0;
}

CDvrSearchChannel::~CDvrSearchChannel()
{
    
}

/*
 * 摘要：处理命令
 */
int CDvrSearchChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;
	
    if (m_search_channel_parm.base.func != NULL)
    {
        int nSearchType = GetSearchType();
		int nSubtype = GetSearchSubtype();

        // 查询录像
        if (AFK_CHANNEL_SEARCH_RECORD == nSearchType)
        {
            afk_record_file_info_s *pstFileInfo = NULL;
            int nFileCount = 0;

            parseSearchRecordResult_dvr2(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
										&pstFileInfo, nFileCount, pBuf[17] == 6, pBuf[16]);

            nRet = m_search_channel_parm.base.func(this, (unsigned char*)pstFileInfo, 
						nFileCount*sizeof(afk_record_file_info_s), 0, m_search_channel_parm.base.udata);

            if (pstFileInfo != NULL)
            {
                delete[] pstFileInfo;
            }
        }
        // 日志查询
        else if (AFK_CHANNEL_SEARCH_LOG == nSearchType)
        {
            if (pBuf[12] == 0) // 传统的日志查询
			{
				m_nLogSize = *(unsigned int*)(pBuf + 8);
				m_nLogRecSize += nLen - HEADER_SIZE;
				if (m_nLogRecSize == m_nLogSize)
				{
					nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
														(void*)-1, m_search_channel_parm.base.udata);
				}
				else
				{
					nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
														(void*)m_nLogSize, m_search_channel_parm.base.udata);
				}

				// 为了兼容每次发4K包的特殊日志协议，检测到包大小超过2K即再发一个应答包
				if (nLen-HEADER_SIZE > 1024*2)
				{
					sendQueryLog_dvr2(m_pDvrDevice, -1, 0);
				}
			}
			else if(pBuf[12] == 1) // 分页方式的日志查询
			{
				nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
													(void*)-2, m_search_channel_parm.base.udata);
			}
        }
        // 通道名查询
        else if (AFK_CHANNEL_SEARCH_CHANNELNAME == nSearchType)
        {
			int nType = *(pBuf+8);
            nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
												(void*)nType, m_search_channel_parm.base.udata);
        }
        // 查询报警布撤防输入 | 查询报警布撤防输出
        else if (AFK_CHANNEL_SEARCH_ALARMCTRL_IN == nSearchType || AFK_CHANNEL_SEARCH_ALARMCTRL_OUT == nSearchType)
        {
            afk_alarm_info_s *pstAlarmInfo = NULL;
            parseSearchAlarmResult_dvr2(m_pDvrDevice, pBuf, nLen, &pstAlarmInfo);
    
            nRet = m_search_channel_parm.base.func(this, (unsigned char*)pstAlarmInfo, 
												sizeof(afk_alarm_info_s), 0, m_search_channel_parm.base.udata);

            if (pstAlarmInfo != NULL)
            {
                delete[] pstAlarmInfo->state;
                delete pstAlarmInfo;
            }
        }
        else if (AFK_CHANNEL_SEARCH_ALARMTRRIGER_MODE == nSearchType)
		{
			if (*(pBuf+8) == 0x05)	// 0x05:查询报警触发方式
			{
				afk_alarm_trriger_mode_s stuTrrigerMode = {0};
				
				stuTrrigerMode.channel_count = m_pDvrDevice->alarmoutputcount(m_pDvrDevice);
				memcpy(stuTrrigerMode.trriger_mode, pBuf+12, 16*sizeof(unsigned char));
				
				nRet = m_search_channel_parm.base.func(this, (unsigned char*)&stuTrrigerMode, 
							sizeof(afk_alarm_trriger_mode_s), (void *)1, m_search_channel_parm.base.udata);
			}
		}
        // 查询配置
		else if (AFK_CHANNEL_SEARCH_CONFIG == nSearchType) 
		{
			int nEnable = 0;
			if (CONFIG_TYPE_PPPOE == nSubtype || CONFIG_TYPE_DDNS == nSubtype)
			{
				nEnable = pBuf[20]; // PPPoE和DDNS使能标志
			}

            nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
												(void *)nEnable, m_search_channel_parm.base.udata);
		}
        // 查询录像状态
        else if (AFK_CHANNEL_SEARCH_RECORDSTATE == nSearchType)
        {
            nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 
												0, m_search_channel_parm.base.udata);
        }
        // 查询设备时间
        else if (AFK_CHANNEL_SEARCH_DEVTIME == nSearchType)
        {
            nRet = m_search_channel_parm.base.func(this, pBuf, nLen, 0, m_search_channel_parm.base.udata);
        }
        // 查询串口协议
        else if (AFK_CHANNEL_SEARCH_COMMPROTOCOL == nSearchType || AFK_CHANNEL_SEARCH_DCDPROTOCOL == nSearchType)
        {
            int nProtocolMask = *(int*)(pBuf + 10);
            nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, (void*)nProtocolMask, m_search_channel_parm.base.udata);
        }
        // 查询系统信息
        else if (AFK_CHANNEL_SEARCH_SYSTEM_INFO == nSearchType)
        {
			if (SYSTEM_INFO_DEV_ATTR == nSubtype)
			{
				SYSATTR_T *newInfo = (SYSATTR_T *)(pBuf + HEADER_SIZE);
				m_pDvrDevice->SetDeviceInfo(1, newInfo->iAlarmInCaps);	//refresh alarm input count
				m_pDvrDevice->SetDeviceInfo(2, newInfo->iAlarmOutCaps); //refresh alarm output count
			}

//			if(SYSTEM_INFO_DISK_DRIVER == subtype)
//			{
//				m_parent->device_set_info(dit_partition_ver, (void *)&pbuf[18]);
//			}

			if (SYSTEM_INFO_PICTURE == nSubtype || SYSTEM_INFO_MOTIONDETECT == nSubtype || 
				SYSTEM_INFO_VIDEOBLIND == nSubtype || SYSTEM_INFO_WATERMARK == nSubtype)
			{
				nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, (void*)pBuf[16], m_search_channel_parm.base.udata);
			}
			else
			{
				nRet = m_search_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 0, m_search_channel_parm.base.udata);
            }
        }
		// 查询设备工作状态
		else if (AFK_CHENNEL_SEARCH_DEVWORKSTATE == nSearchType)
		{
			nRet = m_search_channel_parm.base.func(this, pBuf, nLen, 0, m_search_channel_parm.base.udata);
		}
		// 查询设备报警、状态
		else if (AFK_CHENNEL_SEARCH_DEVSTATE == nSearchType)
		{
			if (0x69 == pBuf[0] && 0x01 == pBuf[8]) 
			{
				int success = (int)pBuf[16];
				nRet = m_search_channel_parm.base.func(this, pBuf+HEADER_SIZE, nLen-HEADER_SIZE, (void*)success, m_search_channel_parm.base.udata);
			}
			else
			{
				nRet = m_search_channel_parm.base.func(this, pBuf, nLen, 0, m_search_channel_parm.base.udata);
			}
		}
		// 查询云台设备属性
		else if (AFK_CHANNEL_SEARCH_PTZ == nSearchType)
		{
			if (0xe4 == pBuf[0])
			{
				nRet = m_search_channel_parm.base.func(this, pBuf+HEADER_SIZE, nLen-HEADER_SIZE, 0, m_search_channel_parm.base.udata);
			}
				
		}
		else if(AFK_CHANNEL_SEARCH_TRANSCOM == nSearchType)
		{
			if (0x20 == pBuf[0])
			{
				nRet = m_search_channel_parm.base.func(this, pBuf+HEADER_SIZE, nLen-HEADER_SIZE, 0, m_search_channel_parm.base.udata);
			}
		}
		else
		{
#ifdef _DEBUG
			OutputDebugString("can't find cmd in onrespond!\n");
#endif
		}
    }

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrSearchChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

	return TRUE;
}

BOOL CDvrSearchChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrSearchChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrSearchChannel::channel_set_info(int type, void *parm)
{
    return 0;
}


