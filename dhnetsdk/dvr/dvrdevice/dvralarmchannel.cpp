
#include "dvralarmchannel.h"
#include "dvrdevice.h"
#include "../def.h"


CDvrAlarmChannel::CDvrAlarmChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_alarm_channel_parm = *(afk_alarm_channel_param_s*)pParam;
	
	m_pstateinfo = new afk_state_info_s;
	if (m_pstateinfo && pDvrDevice)
	{	
		memset(m_pstateinfo, 0, sizeof(afk_state_info_s));
		m_pstateinfo->channelcount = pDvrDevice->GetDvrInfo().channelcount;
		m_pstateinfo->alarminputcount = pDvrDevice->GetDvrInfo().alarminputcount;
	}
	else
	{
		throw 0xFFFFFFFF;
	}
}

CDvrAlarmChannel::~CDvrAlarmChannel()
{
	if (m_pstateinfo != NULL)
	{
		delete m_pstateinfo;
	}
}

/*
 * 摘要：处理命令
 */
int CDvrAlarmChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

    if (m_alarm_channel_parm.base.func != NULL)
    {
		if (0 == m_alarm_channel_parm.parm && 0xb1 == pBuf[0]) // 老报警协议
		{
			// 普通报警
			if (nLen == HEADER_SIZE && *(pBuf + 8) != 0x0A)
			{
				// 外部报警:*(alarmresult + 13) % 2
				int i = 0;
				memset(m_pstateinfo->alarm, 0, 16);
				unsigned long alarm = *(unsigned long*)(pBuf + 16);
				for (i=0; i<m_pstateinfo->alarminputcount; i++)
				{
					m_pstateinfo->alarm[i] = (alarm>>i)%2;
				}
				
				// 视频丢失: (*(alarmresult + 13) >> 1) % 2
				memset(m_pstateinfo->videolost, 0, 16);
				unsigned long videolost = *(unsigned long*)(pBuf + 20);
				for (i=0; i<m_pstateinfo->channelcount; i++)
				{
					m_pstateinfo->videolost[i] = (videolost>>i)%2;
				}
				
				// 动态检测: (*(alarmresult + 13) >> 2) % 2
				memset(m_pstateinfo->motiondection, 0, 16);
				unsigned long motiondection = *(unsigned long*)(pBuf + 24);
				for (i=0; i<m_pstateinfo->channelcount; i++)
				{
					m_pstateinfo->motiondection[i] = (motiondection>>i)%2;
				}

				nRet = m_alarm_channel_parm.base.func(this, (unsigned char*)m_pstateinfo, sizeof(afk_state_info_s), (void *)0/*comm*/, m_alarm_channel_parm.base.udata);
			}
			if ((nLen >= HEADER_SIZE) && (*(pBuf + 8) != 0x0A))//POS机小票信息,这里按照新协议处理报警信息
			{
				if (pBuf[13] == 0xbf)
				{
					//小票信息
					nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, (void *)6/*PosTicket*/, m_alarm_channel_parm.base.udata);
				}
				if (pBuf[13] == 0x7f)
				{
					//裸数据
					nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, (void *)7/*RawPosTicket*/, m_alarm_channel_parm.base.udata);
				}
			}
			// 坏硬盘报警
			else if (*(pBuf + 8) == 0x09)
			{
				nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, 4, (void *)1/*disk error*/, m_alarm_channel_parm.base.udata);
			}
			// 硬盘满报警
			else if (*(pBuf + 8) == 0x08) 
			{
				nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, 1, (void *)2/*disk full*/, m_alarm_channel_parm.base.udata);
			}
			// 遮挡报警
			else if (*(pBuf + 8) == 0x06)
			{
				nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen-HEADER_SIZE, (void *)3/*shelter*/, m_alarm_channel_parm.base.udata);
			}
			// 音频报警
			else if (*(pBuf + 8) == 0x0A)
			{
				nRet = m_alarm_channel_parm.base.func(this, pBuf + 29, 2, (void *)4/*sound detect*/, m_alarm_channel_parm.base.udata);
			}
			// 解码器报警
			else if (*(pBuf + 8) == 0x03)
			{
				nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, 16, (void *)5/*alarm decoder*/, m_alarm_channel_parm.base.udata);
			}
// 			else if (*(pBuf + 8) == 0xBF)
// 			{
// 				nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, 16, (void *)6/*alarm pos ticket*/, m_alarm_channel_parm.base.udata);
// 			}
		}
        else if (1 == m_alarm_channel_parm.parm && 0x69 == pBuf[0]) // 新报警协议
		{
			int alarmtype = (int)pBuf[12];

			if (*(pBuf + 12) == 0x05 && *(pBuf + 13) == 0x01) // 新的音频检测报警，按照字符串解析
			{
				alarmtype = 254;
			}

			switch(alarmtype) 
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 156:		// 报警解码器报警
			case 162:		// atm和pos断开报警
			case 254:		// 新的音频检测报警
				nRet = m_alarm_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen-HEADER_SIZE, 
												(void *)(alarmtype+100), m_alarm_channel_parm.base.udata);
				break;
			case 161:
				nRet = m_alarm_channel_parm.base.func(this, pBuf , HEADER_SIZE, 
												(void *)(alarmtype+100), m_alarm_channel_parm.base.udata);
				break;
			default:
				break;
			}
        }
    }

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrAlarmChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrAlarmChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrAlarmChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrAlarmChannel::channel_set_info(int type, void *parm)
{
    return 0;
}






















