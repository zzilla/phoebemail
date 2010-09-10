//------------------------------------------------------------------------------------------
// File: MonitorManager.cpp
//
// Desc: Video render module - manages information about the display devices in the system
// 
// Copyright (c) ZheJiang Dahua Technology Stock Co.Ltd. All Rights Reserved.
//------------------------------------------------------------------------------------------
#include "MonitorManager.h"

#pragma  warning(disable:4706)
#define  COMPILE_MULTIMON_STUBS   
#include <multimon.h>
#pragma  warning(default:4706)

//------------------------------------------------------------------------------------------
BOOL WINAPI DDEnumCallbackEx(GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName,
                              VOID* pContext, HMONITOR hm)
{
	if (pGuid != NULL)
	{
		CMonitorManager *pThis = (CMonitorManager*)pContext;
		pThis->AddMonitor2List(pGuid, pszDesc, pszDriverName, hm);
	}

    return TRUE; // Keep enumerating
}
//------------------------------------------------------------------------------------------
CMonitorManager::CMonitorManager()
{
	EnumSysMonitors();
}

CMonitorManager::~CMonitorManager()
{
	m_MonitorList.clear();
}

void CMonitorManager::AddMonitor2List(GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName, HMONITOR hm)
{
	HWNDHMONITOR tMonitorItem;
	memcpy(&(tMonitorItem.guid), pGuid, sizeof(GUID));
	tMonitorItem.hMonitor = hm;
	
	m_MonitorList.push_back(tMonitorItem);
}

void CMonitorManager::GetRenderRect(HWND hWnd, RECT &rcRect)
{
	GetClientRect(hWnd, &rcRect);

	LONG lWidth  = rcRect.right - rcRect.left;
	LONG lHeight = rcRect.bottom - rcRect.top;
	
	POINT lPoint;
	lPoint.x = rcRect.left;
	lPoint.y = rcRect.top;
	ClientToScreen(hWnd, &lPoint);
	
	rcRect.left		= lPoint.x;
	rcRect.top		= lPoint.y;
	rcRect.right	= lPoint.x + lWidth;
	rcRect.bottom	= lPoint.y + lHeight;
}

HRESULT CMonitorManager::EnumSysMonitors(void)
{
	m_MonitorList.clear();

	HRESULT hr;
	hr = DirectDrawEnumerateEx(DDEnumCallbackEx,
	                           this, 
	                           DDENUM_ATTACHEDSECONDARYDEVICES);

	return hr;
}

BOOL CMonitorManager::GetMonitorResolution(HWND hWnd, long *lResoluteW, long *lResoluteH)
{
	HMONITOR hm = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	
	BOOL bFindMonitor = FALSE;

	for(int i = 0; i < m_MonitorList.size(); i++)
	{
		if (hm == m_MonitorList[i].hMonitor)
		{
			bFindMonitor = TRUE;
			break;
		}
	}

	if (!bFindMonitor) return FALSE;
	
	DISPLAY_DEVICE dd;
	ZeroMemory(&dd, sizeof(DISPLAY_DEVICE));
	dd.cb = sizeof(DISPLAY_DEVICE);
	bFindMonitor = EnumDisplayDevices(NULL, i, &dd, 0);
	if (!bFindMonitor) return FALSE;
	
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(DEVMODE));
	dm.dmSize = sizeof(dm);
	bFindMonitor = EnumDisplaySettings((char*)dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm);
	if (!bFindMonitor) return FALSE;
	
	*lResoluteW = dm.dmPelsWidth;
	*lResoluteH = dm.dmPelsHeight;
	
	return TRUE;
}

BOOL CMonitorManager::GetMonitorGUID(HWND hWnd, GUID* pGuid)
{
	BOOL bFindMonitor = FALSE;
	HMONITOR hm = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	memset(pGuid, 0, sizeof(GUID));
	MONITOR_ITEM_LIST::iterator it;
	for (it = m_MonitorList.begin(); it != m_MonitorList.end(); it++)
	{
		if (hm == (*it).hMonitor)
		{
			bFindMonitor = TRUE;
			*pGuid = (*it).guid;
			break;
		}
	}

	return bFindMonitor;
}

BOOL CMonitorManager::GetMonitorBeginPos(HWND hWnd, long *lBeginPosX, long *lBeginPosY)
{
	long lResoluteW = 0;
	long lResoluteH = 0;
	BOOL bRet = GetMonitorResolution(hWnd, &lResoluteW, &lResoluteH);
	if (!bRet) return FALSE;
		
	*lBeginPosX = 0;
	*lBeginPosY = 0;
	
	RECT rc;
	GetRenderRect(hWnd, rc);
	
	HMONITOR hm = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	
	MONITOR_ITEM_LIST::iterator it;
	for (it = m_MonitorList.begin(); it != m_MonitorList.end(); it++)
	{
		if (hm == (*it).hMonitor)
		{
			POINT ptLeftTop;
			ptLeftTop.y = rc.top;
			long lXPos  = rc.left;
			if (lXPos < 0)
			{
				for (; lXPos <= 1280*11; lXPos++)
				{
					ptLeftTop.x = lXPos;
					HMONITOR ptMonitor = MonitorFromPoint(ptLeftTop, MONITOR_DEFAULTTONULL);
					if (ptMonitor != hm)
					{
						*lBeginPosX = lXPos - lResoluteW;
						break;
					}
				}
			}
			else
			{
				for (; lXPos >0; lXPos--)
				{
					ptLeftTop.x = lXPos;
					HMONITOR ptMonitor = MonitorFromPoint(ptLeftTop, MONITOR_DEFAULTTONULL);
					if (ptMonitor != hm)
					{
						*lBeginPosX = lXPos + 1;
						break;
					}
				}
			}

			ptLeftTop.x = rc.left;
			long lYPos  = rc.top;
			if (lYPos < 0)
			{
				for (; lYPos <= 1024*11; lYPos++)
				{
					ptLeftTop.y = lYPos;
					HMONITOR ptMonitor = MonitorFromPoint(ptLeftTop, MONITOR_DEFAULTTONULL);
					if (ptMonitor != hm)
					{
						*lBeginPosY = lYPos - lResoluteH;
						break;
					}
				}		
			}
			else
			{
				for (; lYPos >0; lYPos--)
				{
					ptLeftTop.y = lYPos;
					HMONITOR ptMonitor = MonitorFromPoint(ptLeftTop, MONITOR_DEFAULTTONULL);
					if (ptMonitor != hm)
					{
						*lBeginPosY = lYPos + 1;
						break;
					}
				}
			}
		
			break;
		}
	}
	
	return TRUE;
}

