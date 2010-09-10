
#include "dvrconfigchannel.h"
#include "dvrdevice.h"


CDvrConfigChannel::CDvrConfigChannel(CDvrDevice *pDvrDevice, int nMainCommand, void *pParam)
	: CDvrChannel(pDvrDevice, nMainCommand)
{
    m_config_channel_parm = *(afk_config_channel_param_s*)pParam;
}

CDvrConfigChannel::~CDvrConfigChannel()
{
}

/*
 * 摘要：处理命令
 */
int CDvrConfigChannel::OnRespond(unsigned char *pBuf, int nLen)
{
	int nRet = -1;
    int nConfigType = GetConfigType();
    int nConfigResult = 0;
	
	switch(nConfigType)
	{
	case 0:
	case 1: // 设置报警输入、输出
		nConfigResult = (*(pBuf + 9) == 0);
		break;
	case 2: // 设置配置
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 32: // 第三方接入配置
		nConfigResult = pBuf[8];
		break;
	case 17: // 设置录像状态        
	case 18: // 设置设备时间
		break;
	case 19: // 强制I桢
	case 20: // 设置限制码流
		nConfigResult = (*(pBuf + 8) == 0);
		break;
	default:
		return -1;
		break;
	}

	if (m_config_channel_parm.base.func != NULL)
	{
		nRet = m_config_channel_parm.base.func(this, 0, nConfigResult, 0, m_config_channel_parm.base.udata);
	}

	CDvrChannel::OnRespond(pBuf, nLen);

	return nRet;
}

/*
 * 摘要：关闭通道
 */
BOOL CDvrConfigChannel::channel_close()
{
    m_pDvrDevice->device_remove_channel(this);

    return TRUE;
}

BOOL CDvrConfigChannel::channel_pause(BOOL pause)
{
    return FALSE;
}

int CDvrConfigChannel::channel_get_info(int type, void *parm)
{
    return 0;
}

int CDvrConfigChannel::channel_set_info(int type, void *parm)
{
    return 0;
}


