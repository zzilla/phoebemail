// TPBroadcast.h: interface for the TPBroadcast class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TPBROADCAST_H__2543F871_771C_4286_A2D5_F03047DA80B6__INCLUDED_)
#define AFX_TPBROADCAST_H__2543F871_771C_4286_A2D5_F03047DA80B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TPUDPClient.h"

class TPBroadcast : public TPUDPClient
{
public:
	TPBroadcast(ITPListener *callback, int engineId = 0);
	virtual ~TPBroadcast();
	

public:/*½Ó¿Ú*/
	virtual int Connect(const char* szIp, int nPort);
	
	virtual int Connect(const char* szLocalIp, int nLocalPort, const char* szRemoteIp, int nRemotePort);
};

#endif // !defined(AFX_TPBROADCAST_H__2543F871_771C_4286_A2D5_F03047DA80B6__INCLUDED_)
