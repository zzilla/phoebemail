
#include "dvrinterface.h"
#include "dvrdevice/dvrdevice.h"
#include "dvrdevice/dvrchannel.h"


/* 版本信息 */
char* interface_version(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_version();
}

/* 制造商信息 */							
char* interface_manufactory(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_manufactory();
}

/* 产品信息 */
char* interface_product(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_product();
}

/* IP地址 */
char* interface_ip(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->GetIp();
}

/* 端口 */
int interface_port(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->GetPort();
}

/* 类型 */
int interface_type(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->GetType();
}

/* 通道个数 */
int interface_channelcount(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->GetVideoChannelCount();
}

/* 报警输入个数 */                        
int interface_alarminputcount(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->GetAlarminputCount();
}

/* 报警输出个数 */                     
int interface_alarmoutputcount(afk_device_s *device)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->GetAlarmoutputCount();
}

/* 关闭设备 */
int interface_close(afk_device_s *device)
{
	int nRet = -1;
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
	if (pDvrDevice)
	{
		nRet = pDvrDevice->device_close();
	}

    return nRet;
}

/* 得到通道 */
void* interface_get_channel(afk_device_s *device, int type, unsigned int parm)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return (afk_channel_s*)pDvrDevice->device_get_channel(type, parm);
}         

/* 打开通道 */
void* interface_open_channel(afk_device_s *device, int type, void *parm)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_open_channel(type, parm);
}

/* 是否通道打开 */
BOOL interface_channel_is_opened(afk_device_s *device, int type, void *parm)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_channel_is_opened(type, parm);
}
	
/* 获取信息 */
int interface_get_info(afk_device_s *device, int type, void *parm)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_get_info(type, parm);
}

/* 设置信息 */
int interface_set_info(afk_device_s *device, int type, void *parm)
{
    CDvrDevice *pDvrDevice = (CDvrDevice*)device;
    return pDvrDevice->device_set_info(type, parm);
}

/* 设备资源生命计数 */
int interface_dev_addref(afk_device_s *device)
{
	CDvrDevice *pDvrDevice = (CDvrDevice *)device;
	return pDvrDevice->device_AddRef();
}

int interface_dev_decref(afk_device_s *device)
{
	CDvrDevice *pDvrDevice = (CDvrDevice *)device;
	return pDvrDevice->device_DecRef();
}

/* 得到设备 */
void* interface_get_device(afk_channel_s *channel)
{
    CDvrChannel *pDvrChannel = (CDvrChannel*)channel;
    return (afk_device_s*)pDvrChannel->channel_getdevice();
}

/* 关闭通道 */
BOOL interface_close_ch(afk_channel_s *channel)
{
	BOOL ret = FALSE;
    CDvrChannel *pDvrChannel = (CDvrChannel*)channel;
	if (pDvrChannel)
	{
		ret = pDvrChannel->channel_close();
		if (!ret)
		{
			//...this can not happen for now
		}
		pDvrChannel->channel_decRef();
	}

	return ret;
}

/* 暂停通道 */
BOOL interface_pause_ch(afk_channel_s *channel, BOOL pause)
{
    CDvrChannel *pDvrChannel = (CDvrChannel*)channel;
    return pDvrChannel->channel_pause(pause);
}

/* 获取信息 */
int interface_get_info_ch(afk_channel_s *channel, int type, void *parm)
{
    CDvrChannel *pDvrChannel = (CDvrChannel*)channel;
    return pDvrChannel->channel_get_info(type, parm);
}

/* 设置信息 */
int interface_set_info_ch(afk_channel_s *channel, int type, void *parm)
{
    CDvrChannel *pDvrChannel = (CDvrChannel*)channel;
    return pDvrChannel->channel_set_info(type, parm);
}

