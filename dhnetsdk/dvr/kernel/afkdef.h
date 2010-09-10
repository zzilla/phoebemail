
#ifndef _AFKDEF_H_
#define _AFKDEF_H_

#include "../StdAfx.h"


/************************************************************************
 ** 设备类接口定义
 ***********************************************************************/
struct afk_device_s
{
	virtual	~afk_device_s(){}
	char*	(*version)(afk_device_s *device);							// 版本信息
	char*	(*manufactory)(afk_device_s *device);						// 制造商信息
	char*	(*product)(afk_device_s *device);							// 产品信息
	char*	(*device_ip)(afk_device_s *device);							// ip地址
    int		(*device_port)(afk_device_s *device);						// 端口
    int		(*device_type)(afk_device_s *device);						// 类型
    int		(*close)(afk_device_s *device);								// 关闭设备
    int		(*channelcount)(afk_device_s *device);						// 通道个数
    int		(*alarminputcount)(afk_device_s *device);					// 报警输入个数
    int		(*alarmoutputcount)(afk_device_s *device);					// 报警输出个数

    void*	(*get_channel)(afk_device_s *device, int type, unsigned int parm);	// 得到通道
	void*	(*open_channel)(afk_device_s *device, int type, void *parm);		// 打开通道
	BOOL	(*channel_is_opened)(afk_device_s *device, int type, void *parm);	// 是否通道已打开

	int		(*get_info)(afk_device_s *device, int type, void *parm);	// 获取信息
	int		(*set_info)(afk_device_s *device, int type, void *parm);	// 设置信息

	int		(*dev_addref)(afk_device_s *device);
	int		(*dev_decref)(afk_device_s *device);
};


/************************************************************************
 ** 抽象通道类接口定义
 ***********************************************************************/
struct afk_channel_s
{
	virtual	~afk_channel_s(){}
    void*	(*get_device)(afk_channel_s *channel);						// 得到设备
	BOOL	(*close)(afk_channel_s *channel);							// 关闭通道
    BOOL	(*pause)(afk_channel_s *channel, BOOL pause);				// 暂停通道
	int		(*get_info)(afk_channel_s *channel, int type, void *parm);	// 获取信息
	int		(*set_info)(afk_channel_s *channel, int type, void *parm);	// 设置信息
};


#endif // _AFKDEF_H_

