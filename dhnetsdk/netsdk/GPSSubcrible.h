// GPSSubcrible.h: interface for the CGPSSubcrible class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GPSSUBCRIBLE_H__A239C645_A7B0_48C3_B905_79BC1F6DBCE0__INCLUDED_)
#define AFX_GPSSUBCRIBLE_H__A239C645_A7B0_48C3_B905_79BC1F6DBCE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "netsdk.h"
#include "../dvr/kernel/afkinc.h"

class CManager;


class CGPSSubcrible  
{
public:
	CGPSSubcrible(CManager *pManager);
	virtual ~CGPSSubcrible();
    
    bool SendGpsSubcrible(LONG lLoginID,BOOL bStart,LONG KeepTime,LONG InterTime);

	void SetGpsRevCallBack(fGPSRev OnGpsRevMessage, DWORD dwUser);
	void OnRevGpsMsg(long ILongID,char strgps[],long lenth);

public:
	static	int __stdcall		ReceiveData(
										afk_handle_t object,	//数据提供者
										unsigned char *data,	//数据体
										unsigned int datalen,	//数据长度
										void *param,			//回调参数
										void *udata);
private:
	fGPSRev m_RecievCallBack;
	DWORD m_dwuserdata;

	CManager*		m_pManager;

};


#endif // !defined(AFX_GPSSUBCRIBLE_H__A239C645_A7B0_48C3_B905_79BC1F6DBCE0__INCLUDED_)


