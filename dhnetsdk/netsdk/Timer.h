// Timer.h: interface for the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__8291CC06_301F_4C73_823E_C9118B0D9C22__INCLUDED_)
#define AFX_TIMER_H__8291CC06_301F_4C73_823E_C9118B0D9C22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 定时器处理回调函数定义
typedef void (__stdcall *OnTimerProcess)(void* lpParam);


class CTimer  
{
public:
	CTimer();
	virtual ~CTimer();

public:
	BOOL				SetTimer(DWORD dwElapse, OnTimerProcess lpTimerProcess, void* lpParam);

	BOOL				KillTimer();

	void				TimerProc();


private:
	OnTimerProcess		m_lpTimerProcess;
	void*				m_lpParam;
	DWORD				m_dwElapse;
	DWORD				m_dwLastTime;

	OS_THREAD			m_hThread;
	OS_EVENT			m_hExitThread;
};

#endif // !defined(AFX_TIMER_H__8291CC06_301F_4C73_823E_C9118B0D9C22__INCLUDED_)
