/*!
**	********************************************************************************
**                                  CServerSet
**							Digital Video Recoder xp
**
**   (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock CO.LTD.
**                            All Rights Reserved
**
**	File		: ServerSet.h
**	Description	: 
**	Modify		: 2008-10-20		yupeng(11653)		Create the file
**	********************************************************************************
*/

#ifndef _SERVER_SET_H_
#define _SERVER_SET_H_

#include "StdAfx.h"
#include "netsdk.h"
#include "../dvr/dvr.h"
#include <map>
using namespace std;

#define ALARM_BUFFER_SIZE (1024*16)
//	接收回调函数
typedef int (CALLBACK *fEventCallBack)(LONG lHandle, int connId, char *szIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, DWORD dwUserData);

class CAlarmClient
{
public:
	CAlarmClient();
	virtual ~CAlarmClient();
	int AddRef()
	{
		if (InterlockedIncrement(&m_lLifeCount) <= 1)
		{
			InterlockedDecrement(&m_lLifeCount);
			return 0;
		}
		
		return m_lLifeCount;
	}
	
	int DecRef()
	{
		if (m_lLifeCount <= 0)
		{
			return 0;
		}
		
		if (InterlockedDecrement(&m_lLifeCount) <= 0)
		{
			delete this;
			return 0;
		}
		
		return m_lLifeCount;
	}

public:
	int GetData(char *szBuf, int nBufLen);
	int PutData(const char* szData, int nDataLen);
public:
	char			m_szIP[64];
	int				m_nPort;

	unsigned int	m_nWritePos;
    unsigned int	m_nReadPos;
	char			m_szBuf[ALARM_BUFFER_SIZE];//客户端缓冲

	DEVMutex			m_csBuf;
private:
	LONG			m_lLifeCount;
};

class CManager;

class CServerSet
{
public:
	CServerSet(CManager *pManager);
	~CServerSet();
	BOOL Init(void);
	BOOL Uninit(void);
	/*成员函数部分*/
public: 
	friend int CALLBACK ServiceCallBack(LONG lHandle, int connId, char *szIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, DWORD dwUserData);
	
	LONG StartServer(WORD wPort, char *pIp, fServiceCallBack pfcb, DWORD dwTimeOut, DWORD dwUserData);//DHSev::

	BOOL StopServer(LONG lHandle);

	void AddTcpServerConnect(int connId, CAlarmClient *pClient);

	void FindTcpServerConnect(int connId, char *szData, int nDataLen);

	void DealTcpServerConnect(int connId);

	void DelTcpServerConnect(int connId);

	int	 DealAlarmPacket(char* szIP, int nPort, char* szBuf, int nBufLen);

	int  DealNewAlarmPacket(char* szIP, int nPort, char* szBuf, int nBufLen);
	//
	LONG GetHandle(void){return m_lHandle;}

	void SetHandle(LONG lHandle){m_lHandle = lHandle;}
private:
	int  GetValueFormNewPacket(char *szBuf, char *szKeyBuf, char *szOutBuf, int nOutBufLen);//从新的报警上传包中获取相应值。
private:
	LONG					m_lHandle;
	fServiceCallBack		m_cbAlarm;
	DWORD					m_dwUserData;

	DEVMutex					m_csClientList;
	map<int, CAlarmClient *> m_clientList;

	CManager*		m_pManager;
};

#endif

