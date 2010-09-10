// DvrNewConfigChannel.cpp: implementation of the CDvrNewConfigChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "DvrNewConfigChannel.h"
#include "dvrdevice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDvrNewConfigChannel::CDvrNewConfigChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
	m_newconfigParam = *(afk_newconfig_channel_param *)pParam;
}


CDvrNewConfigChannel::~CDvrNewConfigChannel()
{

}

/*
 * 摘要：处理命令
 */
int CDvrNewConfigChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	// 调用协议栈解析
	MESS_CALLBACK_DATA stuMessData = {0};
	stuMessData.cbMessFunc = m_newconfigParam.callbackFunc;
	stuMessData.userdata = m_newconfigParam.userdata;
	stuMessData.lOperateHandle = (LONG)this;
	stuMessData.userparam = m_newconfigParam.userparam;
	stuMessData.pBuf = m_newconfigParam.pRecvBuf;
	stuMessData.nBufLen = m_newconfigParam.nBufLen;
	stuMessData.nResultCode = -1;
	stuMessData.pRecvEvent = m_newconfigParam.pRecvEvent;
	stuMessData.pRetLen = m_newconfigParam.pRetLen;
	stuMessData.pResult = m_newconfigParam.pResultCode;
	stuMessData.pDvrRestart = m_newconfigParam.pDvrRestart;
	stuMessData.nDvrRestart = 0;

	if (0xf5 == (unsigned char)*pBuf)
	{
		stuMessData.nResultCode = *(unsigned char*)(pBuf + 12);
		stuMessData.nDvrRestart = *(unsigned char*)(pBuf + 13);
	}
		
	// 处理
	CReadWriteMutexLock lock(m_csDataCallBack);
	if (m_newconfigParam.base.func != NULL)
	{
		if (nLen - HEADER_SIZE == 0)//set
		{
			m_newconfigParam.base.func(this, NULL, 0, (void*)-1, (unsigned char*)&stuMessData);
			m_newconfigParam.base.func = NULL;
		}
		else//get
		{
			m_newconfigParam.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, (void*)-1, &stuMessData);
			m_newconfigParam.base.func = NULL;	
		}	
	}
	lock.Unlock();
	
	
	return 1;
}


/*
 * 摘要：关闭通道
 */
BOOL CDvrNewConfigChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrNewConfigChannel::channel_pause(BOOL pause)
{
	return FALSE;
}


int CDvrNewConfigChannel::channel_get_info(int type, void *parm)
{
	return 0;
}

int CDvrNewConfigChannel::channel_set_info(int type, void *parm)
{
	return 0;
}
