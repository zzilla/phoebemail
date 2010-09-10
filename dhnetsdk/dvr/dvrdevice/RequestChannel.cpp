// RequestChannel.cpp: implementation of the CRequestChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "RequestChannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include "../ParseString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRequestChannel::CRequestChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
	m_requestParam = *(afk_request_channel_param *)pParam;
	m_dwCreateTime = GetTickCountEx();
	m_nProtocolType = -1;
	m_nPacketSequence = 0;
}

CRequestChannel::~CRequestChannel()
{

}

/*
 * 摘要：处理命令
 */
int CRequestChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	// 调用协议栈解析
	MESS_CALLBACK_DATA stuMessData = {0};
	stuMessData.cbMessFunc = m_requestParam.callbackFunc;
	stuMessData.userdata = m_requestParam.userdata;
	stuMessData.lOperateHandle = (LONG)this;
	stuMessData.userparam = m_requestParam.userparam;
	stuMessData.pBuf = m_requestParam.pRecvBuf;
	stuMessData.nBufLen = m_requestParam.nBufLen;
	stuMessData.nResultCode = -1;
	stuMessData.pRecvEvent = m_requestParam.pRecvEvent;
	stuMessData.pRetLen = m_requestParam.pRetLen;
	stuMessData.pResult = m_requestParam.pResultCode;
	
	char szValue[64] = {0};
	char *p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "Result:", "\r\n", szValue, 64);
	if (p)
	{
		if (_stricmp(szValue, "OK") == 0)
		{
			stuMessData.nResultCode = 0;
		}
		else if(_stricmp(szValue, "OK_Restart") == 0)
		{
			stuMessData.nResultCode = 1;
		}
	}
	else
	{
		p = GetProtocolValue((char*)(pBuf+HEADER_SIZE), "FaultCode:", "\r\n", szValue, 64);
		if (p)
		{
			if (_stricmp(szValue, "OK") == 0)
			{
				stuMessData.nResultCode = 0;
			}
			else if(_stricmp(szValue, "OK_Restart") == 0)
			{
				stuMessData.nResultCode = 1;
			}
		}	
	}

	switch(m_nProtocolType)
	{
	/* 异步设置类型 */
	case AFK_REQUEST_DECODER_CTRLTV:
	case AFK_REQUEST_DECODER_SWITCH:
	case AFK_REQUEST_DEC_PLAYBACKBYDEC:
		{
			// 回调
			CReadWriteMutexLock lock(m_csDataCallBack);
			if (m_requestParam.base.func != NULL)
			{
				m_requestParam.base.func(this, (unsigned char*)&stuMessData, sizeof(MESS_CALLBACK_DATA), (void*)-1, m_requestParam.base.udata);
				m_requestParam.base.func = NULL;
			}
			lock.Unlock();

			// 删除
			m_pDvrDevice->device_remove_channel(this);
			channel_decRef();
			return 1;
		}
		break;
	/* 同步查询类型 */
	case AFK_REQUEST_DEC_QUERY_TV:
	case AFK_REQUEST_DEC_QUERY_CHANNEL:
	case AFK_REQUEST_DEC_QUERY_SYS:
	case AFK_REQUEST_DEC_QUERY_CHNNUM:
	case AFK_REQUEST_DEC_ADD_COMBIN:
	case AFK_REQUEST_DEC_QUERY_COMBIN:
	case AFK_REQUEST_DEC_QUERY_TOUR:
	case AFK_REQUEST_GET_IPFILTER:
	case AFK_REQUEST_GET_AMRENCODE_CFG:
	case AFK_REQUEST_GET_RECORDLEN_CFG:
	case AFK_REQUEST_AUTOREGISTER_GETSERVER:
	case AFK_REQUEST_AUTOREGISTER_GETCONID: 
	case AFK_REQUEST_GET_VIDEOFORMAT_INFO:
	case AFK_REQUEST_GET_MMS_INFO:
	case AFK_REQUEST_GET_SMSACTIVE_INFO:
	case AFK_REQUEST_GET_DIALIN_INFO:
	case AFK_REQUEST_SET_BURNFILE_TRANS:
	case AFK_REQUEST_GET_BURNFILE_TRANS:
	case AFK_REQUEST_CONNECT:
	case AFK_REQUEST_DISCONNECT:
	case AFK_REQUEST_SET_BURN_ATTACH:
	case AFK_REQUEST_GET_BURN_ATTACH:
	case AFK_REQUEST_SET_SNIFFER:
	case AFK_REQUEST_GET_SNIFFER:
	case AFK_REQUEST_SET_DOWNLOAD_RATE:
	case AFK_REQUEST_GET_DOWNLOAD_RATE:
	case AFK_REQUEST_DEC_CHANNEL_STATE:
	case AFK_REQUEST_GET_POINT_CFG:
	case AFK_REQUEST_SET_POINT_CFG:
	case AFK_REQUEST_GET_OSDENABLE_CFG:
	case AFK_REQUEST_SET_OSDENABLE_CFG:
	case AFK_REQUEST_GET_TVADJUST_CFG:
	case AFK_REQUEST_FUNCTION_MASK:
	case AFK_REQUEST_GET_VEHICLE_CFG:
	case AFK_REQUEST_SET_VEHICLE_CFG:
	case AFK_REQUEST_GET_ATM_OVERLAY_CFG:
	case AFK_REQUEST_SET_ATM_OVERLAY_CFG:
	case AFK_REQUEST_GET_ATM_OVERLAY_ABILITY:
	case AFK_REQUEST_QUERY_BACKUP_DEV:
	case AFK_REQUEST_QUERY_BACKUP_DEV_INFO:
	case AFK_REQUEST_BACKUP_FEEDBACK:
	case AFK_REQUEST_ATM_TRADE_TYPE:
	case AFK_REQUEST_SET_DECODER_TOUR_CFG:
	case AFK_REQUEST_GET_DECODER_TOUR_CFG:
	//SIP
	case AFK_REQUEST_GET_SIP_CFG:
	case AFK_REQUEST_SET_SIP_CFG:
	case AFK_REQUEST_GET_SIP_ABILITY:
	case AFK_REQUEST_GET_SIP_STATE:
	case AFK_REQUEST_GET_POS_CFG:
		{
			// 处理
			CReadWriteMutexLock lock(m_csDataCallBack);
			if (m_requestParam.base.func != NULL)
			{
				m_requestParam.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, (void*)-1, &stuMessData);
				m_requestParam.base.func = NULL;
			}
			lock.Unlock();
		}
		break;
	/* 同步设置类型 */
	case AFK_REQUEST_DEC_DEL_COMBIN:
	case AFK_REQUEST_DEC_SET_COMBIN:
	case AFK_REQUEST_DEC_SET_TOUR:
	case AFK_REQUEST_DEC_SET_TV_ENABLE:
	case AFK_REQUEST_SET_IPFILTER:
	case AFK_REQUEST_SET_AMRENCODE_CFG:
	case AFK_REQUEST_SET_RECORDLEN_CFG:
	case AFK_REQUEST_AUTOREGISTER_REGSERVER:
	case AFK_REQUEST_SET_MMS_INFO:
	case AFK_REQUEST_SET_SMSACTIVE_INFO:
	case AFK_REQUEST_SET_DIALIN_INFO:
	case AFK_REQUEST_DEC_CTRLPLAYBACK:
	case AFK_REQUEST_SET_VIDEOOUT:
	case AFK_REQUEST_SET_TVADJUST_CFG:
	case AFK_REQUEST_BACKUP_START:
	case AFK_REQUEST_BACKUP_STOP:
	case AFK_REQUEST_DEC_CONTROL:
		{
			// 回调
			CReadWriteMutexLock lock(m_csDataCallBack);
			if (m_requestParam.base.func != NULL)
			{
				m_requestParam.base.func(this, NULL, 0, (void*)-1, (unsigned char*)&stuMessData);
				m_requestParam.base.func = NULL;
			}
			lock.Unlock();
		}
		break;

	default:
		break;
	}

	return 1;
}

/*
 * 摘要：关闭通道
 */
BOOL CRequestChannel::channel_close()
{
	CReadWriteMutexLock lock(m_csDataCallBack);
	m_requestParam.base.func = NULL;
	lock.Unlock();

	m_pDvrDevice->device_remove_channel(this);

	return TRUE;
}

BOOL CRequestChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CRequestChannel::channel_get_info(int type, void *parm)
{
	switch(type)
	{
	case channel_request_param:
		{
			*(afk_request_channel_param **)parm = &m_requestParam;
		}
		break;
	case channel_detect_timeout:
		{
			*(BOOL*)parm = FALSE;
			if (m_requestParam.pRecvEvent == NULL && GetTickCountEx() - m_dwCreateTime > 10000)
			{
				*(BOOL*)parm = TRUE;
				
				// 超时则回调
				MESS_CALLBACK_DATA stuMessData = {0};
				stuMessData.cbMessFunc = m_requestParam.callbackFunc;
				stuMessData.userdata = m_requestParam.userdata;
				stuMessData.lOperateHandle = (LONG)this;
				stuMessData.userparam = m_requestParam.userparam;
				stuMessData.nResultCode = -1;
				stuMessData.pBuf = m_requestParam.pRecvBuf;
				stuMessData.pRetLen = m_requestParam.pRetLen;
				
				CReadWriteMutexLock lock(m_csDataCallBack);
				if (m_requestParam.base.func != NULL)
				{
					m_requestParam.base.func(this, (unsigned char*)&stuMessData, sizeof(MESS_CALLBACK_DATA), (void*)-1, m_requestParam.base.udata);
					m_requestParam.base.func = NULL;
				}
				lock.Unlock();
			}
		}
		break;
	default:
	    break;
	}

	return 0;
}

int CRequestChannel::channel_set_info(int type, void *parm)
{
	switch(type)
	{
	case channel_protocoltype:
		{
			m_nProtocolType = *(int*)parm;
		}
		break;
	case channel_packetsequence:
		{
			m_nPacketSequence = *(int*)parm;
		}
	    break;
	default:
	    break;
	}
	return 0;
}

