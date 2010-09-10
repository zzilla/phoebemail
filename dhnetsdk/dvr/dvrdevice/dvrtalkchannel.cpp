
#include "dvrtalkchannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include "dvrpacket_dvr2.h"


CDvrTalkChannel::CDvrTalkChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_talk_channel_parm = *(afk_talk_channel_param_s*)pParam;
    m_bWorking = TRUE;
}

CDvrTalkChannel::~CDvrTalkChannel()
{
    
}

/*
 * 摘要：处理命令
 */
int CDvrTalkChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

	CReadWriteMutexLock lock(m_csDataCallBack);
    if (m_talk_channel_parm.base.func != NULL)
    {
        nRet = m_talk_channel_parm.base.func(this, pBuf + HEADER_SIZE, nLen - HEADER_SIZE, 0, m_talk_channel_parm.base.udata);
    }
	lock.Unlock();

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrTalkChannel::channel_close()
{
	CReadWriteMutexLock lock(m_csDataCallBack);
	m_talk_channel_parm.base.func = NULL;
	lock.Unlock();

	if (m_bWorking)
    {
        if (!sendTalkRequest_dvr2(m_pDvrDevice, m_talk_channel_parm.no, false, m_talk_channel_parm.nEncodeType))
        {
#ifdef _DEBUG
			// OutputDebugString("Error:发送关闭对讲命令出错-->CDvrTalkChannel_dvr2::channel_close()\n");
#endif
        }
        m_bWorking = FALSE;
    }

	m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrTalkChannel::channel_pause(BOOL pause)
{
	return FALSE;
}

int CDvrTalkChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrTalkChannel::channel_set_info(int type, void *parm)
{
    afk_talk_info_s *talkinfo = (afk_talk_info_s*)parm;
	
    if (sendTalkData_dvr2(m_pDvrDevice, m_talk_channel_parm.no, talkinfo->data, talkinfo->datalen, m_talk_channel_parm.nEncodeType))
    {
        return 1;
    }

    return 0;
}


