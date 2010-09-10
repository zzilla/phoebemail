
#ifndef DVRDEVICEDDNS_H
#define DVRDEVICEDDNS_H

#include "dvrdevice.h"


class CDvrDevice_DDNS : public CDvrDevice
{
public:
	CDvrDevice_DDNS(afk_dvrdevice_info stuDevInfo, CTcpSocket *pTcpSocket,
        const char *szIp, int nPort, const char *szUser, const char *szPassword, 
        fDisconnectCallBack cbDisconnect, fDeviceEventCallBack cbDevEvent, void *userdata);
	virtual ~CDvrDevice_DDNS();


public:							
    /* 打开通道 */
    virtual void*	device_open_channel(int type, void *parm);	
};

#endif

