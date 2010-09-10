// AutoRegister.h: interface for the CAutoRegister class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOREGISTER_H__55EF1893_48EB_442C_A58F_7F4BCCAD48D6__INCLUDED_)
#define AFX_AUTOREGISTER_H__55EF1893_48EB_442C_A58F_7F4BCCAD48D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "netsdk.h"

class CManager;


class CAutoRegister  
{
public:
	CAutoRegister(CManager *pManager);
	virtual ~CAutoRegister();
	
public:
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 关闭一个设备的所有通道 */
	int							CloseChannelOfDevice(afk_device_s* device);

	/* 主动注册重定向功能*/
	// 使设备定向主动连接服务器
	LONG ConnectRegServer(LONG lLoginID, char* RegServerIP, WORD RegServerPort, int TimeOut);
	// 使设备主动注册接服务器
	int ControlRegister(LONG lLoginID, LONG ConnectionID, int waittime);
	// 使设备断开主动注册服务机器
	int DisConnectRegServer(LONG lLoginID, LONG ConnectionID);
	// 查询设备主动注册服务器的信息
	int QueryRegServerInfo(LONG lLoginID, LPDEV_SERVER_AUTOREGISTER lpRegServerInfo, int waittime);

private:
	CManager*		m_pManager;

};


#endif // !defined(AFX_AUTOREGISTER_H__55EF1893_48EB_442C_A58F_7F4BCCAD48D6__INCLUDED_)
