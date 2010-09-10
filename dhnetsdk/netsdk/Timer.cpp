// Timer.cpp: implementation of the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Timer.h"

#define ELAPSE_MIN 10 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTimer::CTimer()
{
	m_lpTimerProcess = NULL;
	m_lpParam = NULL;
	m_dwElapse = 0;
	m_dwLastTime = 0;
}

CTimer::~CTimer()
{

}

DWORD WINAPI Timer_ThreadProc(LPVOID lpThreadParameter)
{
	CTimer *pThis = (CTimer*)lpThreadParameter;
	if (pThis != NULL)
	{
		pThis->TimerProc();
	}

	return 0;
}

void CTimer::TimerProc()
{
	while (WaitForSingleObjectEx(m_hExitThread, 50) != WAIT_OBJECT_0)
	{
		DWORD dwCurTime = GetTickCountEx();
		
		if (dwCurTime - m_dwLastTime > m_dwElapse)
		{
			if (m_lpTimerProcess != NULL)
			{
				m_lpTimerProcess(m_lpParam);
			}
			
			m_dwLastTime = GetTickCountEx();
		}
	}
}

BOOL CTimer::SetTimer(DWORD dwElapse, OnTimerProcess lpTimerProcess, void* lpParam)
{
	if (lpTimerProcess == NULL)
	{
		return FALSE;
	}

	if (dwElapse < ELAPSE_MIN)
	{
		dwElapse = ELAPSE_MIN;
	}

	m_dwLastTime = GetTickCountEx();
	m_lpParam = lpParam;
	m_dwElapse = dwElapse;
	m_lpTimerProcess = lpTimerProcess;

	int ret = CreateEventEx(m_hExitThread, TRUE, FALSE);
	if (ret < 0)
	{
		return FALSE;
	}

	DWORD dwThreadID = 0;
	ret = CreateThreadEx(m_hThread, 0, Timer_ThreadProc, this, 0, &dwThreadID);
	if (ret < 0)
	{
		CloseEventEx(m_hExitThread);
		return FALSE;
	}

	return TRUE;
}

BOOL CTimer::KillTimer()
{
	SetEventEx(m_hExitThread);
	DWORD dwRet = WaitForSingleObjectEx(m_hThread,1000*10);
	if (dwRet != WAIT_OBJECT_0)
	{
		TerminateThreadEx(m_hThread, 0xFFFFFFFF);
	}

	CloseThreadEx(m_hThread);
	CloseEventEx(m_hExitThread);

	return TRUE;
}
