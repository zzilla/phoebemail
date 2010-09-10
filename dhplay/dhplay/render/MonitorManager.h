//------------------------------------------------------------------------------------------
// File: MonitorManager.h
//
// Desc: Video render module - manages information about the display devices in the system
//
// Copyright (c) ZheJiang Dahua Technology Stock Co.Ltd.  All rights reserved.
//------------------------------------------------------------------------------------------
#ifndef __MONITORMANAGER_H__
#define __MONITORMANAGER_H__

#include <windows.h>
#include <vector>
#include <ddraw.h>
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _HWND_MONITOR 
{
	GUID guid;
	HMONITOR hMonitor;
}HWNDHMONITOR;

////////////////////////////////////////////////////////////////////////////////////////////
typedef	std::vector<HWNDHMONITOR> MONITOR_ITEM_LIST;
//------------------------------------------------------------------------------------------
class CMonitorManager
{
public:
	CMonitorManager();
	~CMonitorManager();
	
	BOOL GetMonitorResolution(HWND hWnd, long *lResoluteW, long *lResoluteH);
	BOOL GetMonitorBeginPos(HWND hWnd, long *lBeginPosX, long *lBeginPosY);

	BOOL GetMonitorGUID(HWND hWnd, GUID* pGuid);
	void GetRenderRect(HWND hWnd, RECT &rcRect);

	void AddMonitor2List(GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName, HMONITOR hm);
	
private:
	HRESULT EnumSysMonitors(void);

	MONITOR_ITEM_LIST m_MonitorList;
	
};

#endif //__MONITORMANAGER_H__