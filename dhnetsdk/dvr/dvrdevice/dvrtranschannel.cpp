
#include "dvrtranschannel.h"
#include "dvrdevice.h"
#include "../def.h"
#include "dvrpacket_dvr2.h"


CDvrTransChannel::CDvrTransChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_trans_channel_parm = *(afk_trans_channel_param_s*)pParam;
}

CDvrTransChannel::~CDvrTransChannel()
{
    
}

/*
 * 摘要：处理命令
 */
int CDvrTransChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

    if (m_trans_channel_parm.base.func != NULL)
    {
        int extlen = *((int*)(pBuf + 4));

        unsigned char type = *(pBuf + 9);
        unsigned char ret  = *(pBuf + 10);

        unsigned short parm = (unsigned short)((unsigned short)ret | ((unsigned short)type) << 8);

        nRet = m_trans_channel_parm.base.func(this, pBuf + HEADER_SIZE, extlen, (void*)parm, m_trans_channel_parm.base.udata);
    }

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrTransChannel::channel_close()
{
	sendSettingSerial_dvr2(m_pDvrDevice, m_trans_channel_parm, false);

    m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrTransChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrTransChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrTransChannel::channel_set_info(int type, void *parm)
{
    afk_trans_info_s *transinfo = (afk_trans_info_s*)parm;
    
    if (sendTransSerialData_dvr2(m_pDvrDevice, m_trans_channel_parm.trans_device_type, transinfo->data, transinfo->datalen))
    {
        return 1;
    }

    return 0;
}

