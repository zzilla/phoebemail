
#include "StdAfx.h"
#include "AlarmDeal.h"
#include "Manager.h"
#include "Utils_StrParser.h"
#include "../dvr/ParseString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAlarmDeal::CAlarmDeal(CManager *pManager)
	: m_pManager(pManager)
{

}

CAlarmDeal::~CAlarmDeal()
{

}

int CAlarmDeal::Init(void)
{
	return Uninit();
}

int CAlarmDeal::Uninit(void)
{
	int nRet = 0;

	m_csAlarm.Lock();

	list<st_Alarm_Info*>::iterator it = m_lstAlarm.begin();
	for(; it != m_lstAlarm.end(); ++it)
	{
		if (*it)
		{
			bool b = (*it)->channel->close((*it)->channel);
			if (false == b)
			{
				nRet = -1;
			}
			delete (*it);			
		}
	}
	m_lstAlarm.clear();

	m_csAlarm.UnLock();

	return nRet;
}

#ifdef _DEBUG
int check_jpeg_sign(void *data, int size)
{
	 int i;
	 unsigned int *p = (unsigned int*)data;
	 unsigned int *ptr = (unsigned int*)data;
	 int s = size/4;
	 unsigned int sum = (unsigned int)0xE0FFD8FF;
	 for(p++, i=1; i<s; i++)
	 {
	  sum += *p++;
	 }
	 return *ptr == sum;
}

//这个函数返回数据的校验值，你把该校验值也一并写到jpeg文件的末尾
unsigned int restore_jpeg_sign(void *data, int size)
{
	unsigned int *p = (unsigned int*)data;
	unsigned int ret = *p;
	*p = 0xE0FFD8FF;
	return ret;
}
#endif
#ifdef _DEBUG
int g_TicketCount = 0;
#endif
int __stdcall CAlarmDeal::DeviceStateFunc(
									afk_handle_t object,	/* 数据提供者 */
									unsigned char *data,	/* 数据体 */
									unsigned int datalen,	/* 数据长度 */
									void *param,			/* 回调参数 */
									void *userdata)
{
	CManager *pManager = (CManager*)userdata;
    afk_channel_s *channel = (afk_channel_s*)object;
	if (pManager == NULL || channel == NULL)
	{
		return -1;
	}

	AFK_ALARM_DATA *pstAlarmData = new AFK_ALARM_DATA;
	if (pstAlarmData == NULL)
	{
		return -1;
	}

	memset(pstAlarmData, 0, sizeof(AFK_ALARM_DATA));
	pstAlarmData->device = (afk_device_s*)channel->get_device(channel);
	strcpy(pstAlarmData->szIp, pstAlarmData->device->device_ip(pstAlarmData->device));
	pstAlarmData->nPort = pstAlarmData->device->device_port(pstAlarmData->device);

	//	类型号在100以内为老报警协议
	if ((int)param < 100) 
	{
		switch((int)param)
		{
		case 0: //common
			{
				if (datalen != sizeof(NET_CLIENT_STATE) || data == NULL) 
				{
					goto e_clear;
				}

				NET_CLIENT_STATE * pCommAlarmData = new NET_CLIENT_STATE;
				if (pCommAlarmData == NULL)
				{
					goto e_clear;
				}

				memcpy(pCommAlarmData, data, sizeof(NET_CLIENT_STATE));
				pstAlarmData->dwAlarmType = COMM_ALARM;
				pstAlarmData->data = (void*)pCommAlarmData;
				pstAlarmData->datalen = sizeof(NET_CLIENT_STATE);
			}
			break;
		case 1: //disk error
			{
				if (datalen != sizeof(DWORD) || data == NULL) 
				{
					goto e_clear;
				}

				DWORD *pDiskAlarmData = new DWORD;
				if (pDiskAlarmData == NULL)
				{
					goto e_clear;
				}

				*pDiskAlarmData = *(DWORD*)data;
				pstAlarmData->dwAlarmType = DISK_ERROR_ALARM;
				pstAlarmData->data = (void *)pDiskAlarmData;
				pstAlarmData->datalen = sizeof(DWORD);
			}
			break;
		case 2: //disk full
			{
				if (datalen != sizeof(BYTE) || data == NULL) 
				{
					goto e_clear;
				}

				DWORD *pDiskFullAlarm = new DWORD;
				if (pDiskFullAlarm == NULL)
				{
					goto e_clear;
				}

				*pDiskFullAlarm = (DWORD)(*data);
				pstAlarmData->dwAlarmType = DISK_FULL_ALARM;
				pstAlarmData->data = (void *)pDiskFullAlarm;
				pstAlarmData->datalen = sizeof(DWORD);
			}
			break;
		case 3: //shelter
			{
				if (datalen != 16 || data == NULL) 
				{
					goto e_clear;
				}

				BYTE *pShelterAlarm = new BYTE[16];
				if (pShelterAlarm == NULL)
				{
					goto e_clear;
				}

				memcpy(pShelterAlarm, data, 16);
				pstAlarmData->dwAlarmType = SHELTER_ALARM;
				pstAlarmData->data = (void *)pShelterAlarm;
				pstAlarmData->datalen = 16;
			}
			break;
		case 4: //sound detect
			{
				if (datalen != 2 || data == NULL) 
				{
					goto e_clear;
				}

				WORD sdMask = *(WORD*)data;
				BYTE *pSoundAlarm = new BYTE[16];
				if (pSoundAlarm == NULL)
				{
					goto e_clear;
				}

				memset(pSoundAlarm, 0, 16);
				for (int i = 0; i < 16; i++)
				{
					if (sdMask & (0x01<<i)) 
					{
						pSoundAlarm[i] = 1;
					}
				}
				pstAlarmData->dwAlarmType = SOUND_DETECT_ALARM;
				pstAlarmData->data = (void *)pSoundAlarm;
				pstAlarmData->datalen = 16;
			}
			break;
		case 5:
			{
				if (datalen != 16 || data == NULL)
				{
					goto e_clear;
				}

				BYTE *pDecoderAlarm = new BYTE[16];
				if (pDecoderAlarm == NULL)
				{
					goto e_clear;
				}

				memcpy(pDecoderAlarm, data, 16);
				pstAlarmData->dwAlarmType = ALARMDECODER_ALARM;
				pstAlarmData->data = (void *)pDecoderAlarm;
				pstAlarmData->datalen = 16;
			}
			break;
		case 6: //POS小票信息
			{
				if (datalen < 32 || data == NULL)
				{
					goto e_clear;
				}

#define POS_RECEIPT_LENGTH 10*1024
				
				BYTE *pPosTicketAlarm = new BYTE[POS_RECEIPT_LENGTH] ; // new BYTE[2048];
				memset(pPosTicketAlarm, 0, POS_RECEIPT_LENGTH);
				if (pPosTicketAlarm == NULL)
				{
					goto e_clear;
				}
				
				memcpy(pPosTicketAlarm, data, POS_RECEIPT_LENGTH);
				pstAlarmData->dwAlarmType = POS_TICKET_ALARM;
				pstAlarmData->data = (void *)pPosTicketAlarm;
				pstAlarmData->datalen = datalen;
			}
			break;
		case 7: //小票裸数据
			{
				if (datalen < 32 || data == NULL)
				{
					goto e_clear;
				}
				
				BYTE *pPosTicketAlarm = new BYTE[1024];
				memset(pPosTicketAlarm, 0, sizeof(pPosTicketAlarm));
				if (pPosTicketAlarm == NULL)
				{
					goto e_clear;
				}
				
				memcpy(pPosTicketAlarm, data, 1024);
				pstAlarmData->dwAlarmType = POS_TICKET_RAWDATA;
				pstAlarmData->data = (void *)pPosTicketAlarm;
				pstAlarmData->datalen = datalen;
			}
			break;
		default:
			goto e_clear;
		}
	}
	//	新报警协议
	else
	{
		//新的音频检测报警 add by cqs(10842)
		if ((int)param == 354) 
		{
			if (datalen == 0)
			{
				goto e_clear;
			}
			
			char* buf = new char[datalen + 1];
			if (!buf)
			{
				goto e_clear;
			}
			memcpy(buf, data, datalen);
			buf[datalen] = '\0';

			CStrParse parser;
			parser.setSpliter("&&");
			bool bSuccess = parser.Parse(buf);
			if (!bSuccess) 
			{
				delete[] buf;
				goto e_clear;
			}
		
			NEW_SOUND_ALARM_STATE  *pstNewSoundState = new NEW_SOUND_ALARM_STATE;

			memset(pstNewSoundState,0,sizeof(NEW_SOUND_ALARM_STATE));
	
			pstNewSoundState->channelcount = parser.Size();

			for(int i = 0; i < parser.Size(); i++)
			{
				std::string info = parser.getWord();
				CStrParse subparse;
				subparse.setSpliter("::");
				bSuccess = subparse.Parse(info);
				if (!bSuccess) 
				{
					delete[] buf;
					goto e_clear;
				}

				pstNewSoundState->SoundAlarmInfo[i].channel = atoi(subparse.getWord(0).c_str());
				if (subparse.Size()>1) 
				{
					if (0 == _stricmp(subparse.getWord(1).c_str(), "Audio-High"))
					{
						pstNewSoundState->SoundAlarmInfo[i].alarmType = 1;
					}
					
					//stuNewSoundState.SoundAlarmInfo[i].volume = atoi(subparse.getWord(2).c_str());
				}	
			}

			pstAlarmData->dwAlarmType = NEW_SOUND_DETECT_ALARM_EX;
			pstAlarmData->data = (void*)pstNewSoundState;
			pstAlarmData->datalen = sizeof(NEW_SOUND_ALARM_STATE);

			delete[] buf;
		}
		else if( (int)param == 256) //报警解码器报警
		{
			ALARM_DECODER_ALARM *pAlarmDecoderAlarm = new ALARM_DECODER_ALARM;
			memset(pAlarmDecoderAlarm, 0, sizeof(ALARM_DECODER_ALARM));
			if( (datalen % sizeof(ALARM_DECODER)) == 0)
			{
				pAlarmDecoderAlarm->nAlarmDecoderNum = datalen/sizeof(ALARM_DECODER);
				memcpy(pAlarmDecoderAlarm->stuAlarmDecoder, data, datalen);

				pstAlarmData->datalen = sizeof(ALARM_DECODER_ALARM);
				pstAlarmData->data = (void*)pAlarmDecoderAlarm;
				pstAlarmData->dwAlarmType = ALARM_DECODER_ALARM_EX;
			}
			else
			{
				delete pAlarmDecoderAlarm;
				pAlarmDecoderAlarm = NULL;
				return -1;
			}
		}
		else if((int)param == 261)
		{
			BYTE *bData = new BYTE;
			*bData = data[15];
			pstAlarmData->datalen = 1;
			pstAlarmData->data = bData;
			pstAlarmData->dwAlarmType = OEMSTATE_EX;
		}
		else if((int)param == 262)//ATM和pos机断开报警
		{
			char szValue[64] = {0};
			char *p = GetProtocolValue((char*)data, "PosState:", "\r\n", szValue, 64);
			if (p)
			{
				BYTE *bData = new BYTE;
				*bData = atoi(szValue);
				pstAlarmData->datalen = 1;
				pstAlarmData->data = bData;
				pstAlarmData->dwAlarmType = ATMPOS_BROKEN_EX;
			}
			else
			{
				goto e_clear;
			}
		}
		else if ((int)param == 113)	// 解码器能力不足报警
		{
			BYTE *alarm = new BYTE;
			if (alarm == NULL)
			{
				goto e_clear;
			}
			*alarm = 0;
			pstAlarmData->datalen = 1;
			pstAlarmData->data = (void*)alarm;
			pstAlarmData->dwAlarmType = DECODER_DECODE_ABILITY;

			if (datalen > 2)
			{
				data[datalen-1] = '\0';
				data[datalen-2] = '\0';
				char szValue[64] = {0};
				char *p = GetProtocolValue((char*)data, "DecoderAbility:", "\r\n", szValue, 64);
				if (p)
				{
					*alarm = atoi(szValue);
				}
			}
		}
		else if((int)param == 114)
		{
			ALARM_FDDI_ALARM *alarm = new ALARM_FDDI_ALARM;
			if (alarm == NULL)
			{
				goto e_clear;
			}

			memset(alarm, 0, sizeof(ALARM_FDDI_ALARM));
			pstAlarmData->datalen = sizeof(ALARM_FDDI_ALARM);
			pstAlarmData->data = (void*)alarm;
			pstAlarmData->dwAlarmType = FDDI_DECODER_ABILITY;

			char szValue[64] = {0};
			char *p = (char *)data;
			{
				for (int i = 0; i < 256; i++)
				{
					p = GetProtocolValue(p, "FiberState:", "\r\n", szValue, 64);
					if (p == NULL)
					{
						break;
					}
					alarm->nFDDINum += 1;
					alarm->bAlarm[i] = atoi(szValue);
				}
			}
			
			pstAlarmData->dwAlarmType =	FDDI_DECODER_ABILITY;

		}
		else if((int)param == 117)
		{
			DSP_ALARM *alarm = new DSP_ALARM;
			if(alarm == NULL)
			{
				goto e_clear;
			}

			memset(alarm, 0, sizeof(DSP_ALARM));
			pstAlarmData->datalen = sizeof(DSP_ALARM);
			pstAlarmData->data = (void *)alarm;
			pstAlarmData->dwAlarmType = DSP_ALARM_EX;

			char szValue[64] = {0};
			char *p = (char *)data;

			p = GetProtocolValue(p, "DSPState:", "\r\n", szValue, 64);
			if(p != NULL)
			{
				alarm->bError = atoi(szValue);
			}
			if(alarm->bError > 0)
			{
				p = GetProtocolValue(p, "ErrorID:", "\r\n", szValue, 64);
				if(p != NULL)
				{
					DWORD dwMask = 0x00000001;
					CStrParse parser;
					parser.setSpliter("&");
					parser.Parse(szValue);
					for(int i = 0; i < parser.Size(); i++)
					{
						DWORD dwErrorID = parser.getValue();
						alarm->dwErrorMask |= (dwMask<<dwErrorID);
					}
				}
			}
			//DecodeChannel
			if(alarm->bError > 0)
			{
				p = GetProtocolValue(p, "DecodeChannel:", "\r\n", szValue, 64);
				if(p != NULL)
				{
					DWORD dwChannelMask = 0x00000001;
					CStrParse subparser;
					subparser.setSpliter("&");
					bool bSuccess = subparser.Parse(szValue);
					if (bSuccess)
					{
						for(int i = 0; i < subparser.Size(); i++)
						{
							DWORD dwChannelID = subparser.getValue();
							if (dwChannelID > 0)
							{
								alarm->dwDecChnnelMask |= (dwChannelMask<<(dwChannelID-1));
							}
						}
					}
					
				}
			}	
		}
		else if ((int)param == 118) // 录像状态变化报警
		{
			pstAlarmData->dwAlarmType = RECORD_CHANGED_EX;

			int alarmlen = 0;
			ALARM_RECORDING_CHANGED *alarm = NULL;

			if (datalen > 2)
			{
				data[datalen-1] = '\0';
				data[datalen-2] = '\0';
				char szValue[64] = {0};
				char *p = (char*)data;

				int nDevChannel = pstAlarmData->device->channelcount(pstAlarmData->device);
				alarm = new ALARM_RECORDING_CHANGED[nDevChannel];
				if (alarm != NULL)
				{
					memset(alarm, 0, nDevChannel*sizeof(ALARM_RECORDING_CHANGED));
					while (TRUE)
					{
						p = GetProtocolValue(p, "Channel:", "\r\n", szValue, 64);
						if (p)
						{
							alarm[alarmlen++].nChannel = atoi(szValue);
						}
						else
						{
							break;
						}
					}

					if (0 == alarmlen)
					{
						delete[] alarm;
					}
				}
			}

			pstAlarmData->data = (void *)alarm;
			pstAlarmData->datalen = alarmlen*sizeof(ALARM_RECORDING_CHANGED);
		}
		else if ((int)param == 119) // 设备重启报警
		{
			pstAlarmData->dwAlarmType = DEVICE_REBOOT_EX;
			pstAlarmData->data = NULL;
			pstAlarmData->datalen = 0;
		}
		else if ((int)param == 120) // 配置发生变化报警
		{
			pstAlarmData->dwAlarmType = CONFIG_CHANGED_EX;
			pstAlarmData->data = NULL;
			pstAlarmData->datalen = 0;
		}
		else if ((int)param == 121) //pos机小票信息
		{
			pstAlarmData->dwAlarmType = POS_TICKET_ALARM_EX;
			pstAlarmData->data = data;
			pstAlarmData->datalen = datalen;
		}
		else if ((int)param == 122) //pos机小票裸数据
		{
			pstAlarmData->dwAlarmType = POS_TICKET_RADATA_EX;
			pstAlarmData->data = data;
			pstAlarmData->datalen = datalen;
		}
		else
		{
			if (datalen != 4)
			{
				goto e_clear;
			}

			DWORD dwMask = *(DWORD*)data;
			BYTE byMask[32] = {0};
			for (int i = 0; i < 32; i++)
			{
				if (dwMask & (0x01<<i))
				{
					byMask[i] = 1;
				}
			}

			int alarmlen = 0;
			BYTE *alarm = NULL;
			
			switch((int)param - 100)
			{
			case 1:	// 外部报警
			case 2:	// 动态检测报警
			case 3:	// 视频丢失报警
			case 4:	// 遮挡报警
				{
					pstAlarmData->dwAlarmType = OUT_ALARM_EX + ((int)param - 100) - 1;
					int nDevChannel = pstAlarmData->device->channelcount(pstAlarmData->device);
					if (nDevChannel <= 16)
					{
						alarmlen = 16;
						alarm = new BYTE[alarmlen];
						if (alarm == NULL)
						{
							goto e_clear;
						}
						memcpy(alarm, byMask, alarmlen);
					}
					else
					{
						// 把报警状态记录下来
						BYTE bNum = *(data - 32 + 13);
						if (bNum > 32)
						{
							goto e_clear;
						}
						DWORD *pAlarmStatus = (DWORD*)pstAlarmData->device->get_info(pstAlarmData->device, dit_svr_alarm_flag, (void*)((int)param - 100));
						pAlarmStatus[bNum] = dwMask;
						
						if (pstAlarmData->dwAlarmType == OUT_ALARM_EX)
						{
							alarmlen = pstAlarmData->device->alarminputcount(pstAlarmData->device);
						}
						else
						{
							alarmlen = pstAlarmData->device->channelcount(pstAlarmData->device);
						}
						alarm = new BYTE[alarmlen];
						if (alarm == NULL)
						{
							goto e_clear;
						}
						
						for (int i = 0; i < alarmlen; i++)
						{
							dwMask = pAlarmStatus[i/32];
							if (dwMask & (0x01<<(i%32)))
							{
								alarm[i] = 1;
							}
							else
							{
								alarm[i] = 0;
							}
						}
					}
				}
				break;
			case 5: // 音频检测报警
				{
					pstAlarmData->dwAlarmType = SOUND_DETECT_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 6:	// 硬盘满报警
				{
					pstAlarmData->dwAlarmType = DISKFULL_ALARM_EX;
					alarmlen = 1;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 7:	// 坏硬盘报警
				{
					pstAlarmData->dwAlarmType = DISKERROR_ALARM_EX;
					alarmlen = 32;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 10: // 编码器报警
				{
					pstAlarmData->dwAlarmType = ENCODER_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 11: // 紧急报警
				{
					pstAlarmData->dwAlarmType = URGENCY_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 12: // 无线报警
				{
					pstAlarmData->dwAlarmType = WIRELESS_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 15: // 切换场景报警
				{
					pstAlarmData->dwAlarmType = PANORAMA_SWITCH_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 16: // 失去焦点报警
				{
					pstAlarmData->dwAlarmType = LOSTFOCUS_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
				
			case 17: //pos小票信息
				{
					pstAlarmData->dwAlarmType = POS_TICKET_ALARM_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			case 18: //pos小票裸数据
				{
					pstAlarmData->dwAlarmType = POS_TICKET_RADATA_EX;
					alarmlen = 16;
					alarm = new BYTE[alarmlen];
					if (alarm == NULL)
					{
						goto e_clear;
					}
					memcpy(alarm, byMask, alarmlen);
				}
				break;
			default:
				goto e_clear;
			}
			
			pstAlarmData->data = (void *)alarm;
			pstAlarmData->datalen = alarmlen;
		}
	}
	
	{
		pManager->GetAlarmDataCS().Lock();
		pManager->GetAlarmDataList().push_back(pstAlarmData);
		pManager->GetAlarmDataCS().UnLock();
		
#ifdef _DEBUG
		char *tmpstr = new char [2048+512];
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr,"\nTicketCount:%d---IP:%s--Port:%d---AlarmType:0x%x----Datalen:%d---Data:%s",
			g_TicketCount++,
			pstAlarmData->szIp,
			pstAlarmData->nPort,
			pstAlarmData->dwAlarmType,
			pstAlarmData->datalen,
			pstAlarmData->data);
		OutputDebugString(tmpstr);
		delete tmpstr;
		tmpstr = NULL;
#endif
		SetEventEx(pManager->m_hAlarmDataEvent);
	}
	
    return 1;

e_clear:
	if (pstAlarmData != NULL)
	{
		delete pstAlarmData;
	}
	return -1;
}

int CAlarmDeal::StartListen(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
	{
		return NET_INVALID_HANDLE;
	}
	
	int nRet = NET_ERROR;
	afk_device_s *device = NULL;
	afk_channel_s *alarmch = NULL;
	
	m_csAlarm.Lock();

	list<st_Alarm_Info*>::iterator it = 
		find_if(m_lstAlarm.begin(),m_lstAlarm.end(),SearchAIbydevice(lLoginID));
	
	if (it != m_lstAlarm.end())
	{
		nRet =  NET_NOERROR;
		goto END;
	}

	device = (afk_device_s*)lLoginID;

	afk_alarm_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_alarm_channel_param_s));
	parm.base.func = DeviceStateFunc;
	parm.base.udata = m_pManager;
	parm.parm = 0; //老报警协议

	alarmch = (afk_channel_s*)device->open_channel(device, 
		AFK_CHANNEL_TYPE_ALARM, &parm);
	
	if (alarmch != 0)
	{
		st_Alarm_Info* pAI = new st_Alarm_Info;
		if (!pAI)
		{
			nRet = NET_SYSTEM_ERROR;
		}
		else
		{
			memset(pAI, 0, sizeof(st_Alarm_Info));
			pAI->channel = alarmch;
			m_lstAlarm.push_back(pAI);
			nRet = NET_NOERROR;
		}
	}
	
END:
	m_csAlarm.UnLock();
    return nRet;
}

int CAlarmDeal::StartListenEx(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
	{
		return NET_INVALID_HANDLE;
	}
	
	int nRet = NET_ERROR;
	afk_device_s *device = NULL;
	afk_channel_s *alarmch = NULL;
	
	m_csAlarm.Lock();
	
	list<st_Alarm_Info*>::iterator it = 
		find_if(m_lstAlarm.begin(),m_lstAlarm.end(),SearchAIbydevice(lLoginID));
	
	if (it != m_lstAlarm.end())
	{
		nRet =  NET_NOERROR;
		goto END;
	}

	device = (afk_device_s*)lLoginID;
	
	afk_alarm_channel_param_s parm;
	memset(&parm, 0, sizeof(afk_alarm_channel_param_s));
	parm.base.func = DeviceStateFunc;
	parm.base.udata = m_pManager;
	parm.parm = 1; //新报警协议
	
	alarmch = (afk_channel_s*)device->open_channel(device, 
		AFK_CHANNEL_TYPE_ALARM, &parm);
	
	if (alarmch != 0)
	{
		st_Alarm_Info* pAI = new st_Alarm_Info;
		if (!pAI)
		{
			nRet = NET_SYSTEM_ERROR;
		}
		else
		{
			memset(pAI, 0, sizeof(st_Alarm_Info));
			pAI->channel = alarmch;
			m_lstAlarm.push_back(pAI);
			nRet = NET_NOERROR;
		}
	}
	
END:
	m_csAlarm.UnLock();
    return nRet;
}

int CAlarmDeal::StopListen(LONG lLoginID)
{
	if (m_pManager->IsDeviceValid((afk_device_s*)lLoginID) < 0)
	{
		return NET_INVALID_HANDLE;
	}

	int nRet = NET_ERROR;
	
	m_csAlarm.Lock();

	list<st_Alarm_Info*>::iterator it = 
		find_if(m_lstAlarm.begin(),m_lstAlarm.end(),SearchAIbydevice(lLoginID));

	if (it != m_lstAlarm.end())
	{
		if ((*it) && (*it)->channel)
		{
			bool b = (*it)->channel->close((*it)->channel);
			if (!b)
			{
				nRet = NET_CLOSE_CHANNEL_ERROR;
			}
			else
			{
				nRet = 0;
			}
			delete (*it);
			m_lstAlarm.erase(it);
		}
		else
		{
			nRet = NET_SYSTEM_ERROR;
		}
	}
	else
	{
		nRet = NET_INVALID_HANDLE;
	}
	
	m_csAlarm.UnLock();

	return nRet;
}



