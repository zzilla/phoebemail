
#include "dvrstatiscchannel.h"
#include "dvrdevice.h"


CDvrStatiscChannel::CDvrStatiscChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
}

CDvrStatiscChannel::~CDvrStatiscChannel()
{
}

/*
 * 摘要：处理命令
 */
int CDvrStatiscChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	return -1;
}

BOOL CDvrStatiscChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrStatiscChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrStatiscChannel::channel_get_info(int type, void *parm)
{
	// 设备总流量
	if (parm == 0)
	{
		int statisc = m_pDvrDevice->statisc();
		m_pDvrDevice->startstatisc();
		return statisc;
	}
	else
	{
		afk_channel_s *channel_t = (afk_channel_s*)parm;
		CDvrChannel *channel = (CDvrChannel*)channel_t;
		if (channel)
		{
			int statisc = (int)channel->statisc();
			channel->startstatisc();
			return statisc;
		}
	}
	
    return -1;
}

int CDvrStatiscChannel::channel_set_info(int type, void *parm)
{
    return 0;
}

