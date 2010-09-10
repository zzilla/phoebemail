
#include "dvruserchannel.h"
#include "dvrdevice.h"


CDvrUserChannel::CDvrUserChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_user_channel_parm = *(afk_user_channel_param_s*)pParam;
}

CDvrUserChannel::~CDvrUserChannel()
{
    
}

/*
 * 摘要：处理命令
 */
int CDvrUserChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;

    if (m_user_channel_parm.base.func != NULL)
    {
        int operatecode = *(unsigned char*)(pBuf + 12);
        nRet = m_user_channel_parm.base.func(this, pBuf, nLen, (void*)operatecode, m_user_channel_parm.base.udata);
    }

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrUserChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrUserChannel::channel_pause(BOOL pause)
{
	return FALSE;
}

int CDvrUserChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrUserChannel::channel_set_info(int type, void *parm)
{
    return 0;
}



