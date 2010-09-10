// SnapPicture.cpp: implementation of the CSnapPicture class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SnapPicture.h"

#include "../dvr/def.h"
#include "Manager.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSnapPicture::CSnapPicture(CManager *pManager)
	: m_pManager(pManager)
{
	this->m_RecievCallBack = NULL;
}

CSnapPicture::~CSnapPicture()
{

}

bool CSnapPicture::SnapPictureQuery(LONG lLoginID, SNAP_PARAMS par)
{
	afk_device_s *device = (afk_device_s*)lLoginID;
    if (m_pManager->IsDeviceValid(device) < 0)
    {
		m_pManager->SetLastError(NET_INVALID_HANDLE);
        return 0;
    }
	
	afk_channel_s*  channel =  NULL;
	 
	//创建抓图通道
    afk_snap_channel_param_s parm;
    parm.no = (int)par.Channel;
	parm.subtype = 0;
    parm.base.func = CSnapPicture::ReceiveData;
    parm.base.udata = (void*)&par;
	
	parm.dwLoginID = lLoginID;
	parm.dwuser     = m_dwuserdata;
	parm.snapcallback = m_RecievCallBack;

	channel = (afk_channel_s*)device->get_channel(device,AFK_CHANNEL_TYPE_CAPTURE,par.Channel);
	if ( channel != NULL)
	{
		channel->close(channel);
		channel->close(channel);
	}
	
    channel = (afk_channel_s*)
        device->open_channel(device, AFK_CHANNEL_TYPE_CAPTURE, &parm);

    return (channel!=NULL);
}

void CSnapPicture::SetSnapRevCallBack(fSnapRev OnSnapRevMessage, DWORD dwUser)
{
	this->m_RecievCallBack = OnSnapRevMessage;
	this->m_dwuserdata = dwUser;
}

int __stdcall CSnapPicture::ReceiveData(
										afk_handle_t object,	//数据提供者
										unsigned char *data,	//数据体
										unsigned int datalen,	//数据长度
										void *param,			//回调参数
										void *udata)
{

	return 1;
}


