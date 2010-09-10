//--------------------------------------------------------------------------------
// File: AccurateTimer.cpp
//
// Desc:
// 
// Copyright (c) ZheJiang Dahua Technology Stock Co.Ltd. All Rights Reserved.
//--------------------------------------------------------------------------------
#include "AccurateTimer.h"

#ifdef THREAD_RENDER
#include "decode/videorender.h"
#endif

#define VIDEORENDER_PER_ACTIMER		12


//--------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////
// Function name   : CALLBACK TimerFunc
// Description     : Media callback timer method
//                 : 
// Return type     : static void  : 
// Argument        : UINT uID : 
// Argument        : UINT uMsg : 
// Argument        : DWORD dwUser : 
// Argument        : DWORD dw1 : 
// Argument        : DWORD dw2 : 
///////////////////////////////////////////////////////////////
static void CALLBACK TimerFunc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{	
	CAccurateTimer* pThis = (CAccurateTimer*)dwUser;

	pThis->ACTimerTick();
}

// thread timer mode
DWORD WINAPI ACTimer_TimerProc(LPVOID pParam)
{
	CAccurateTimer* pThis = (CAccurateTimer*)pParam;
	
	pThis->ACTimerTickThread();
	
	return 0;
}

/////////////////////////////////////////////////////////////
CAccurateTimer::CAccurateTimer(tACTimerType eACTimerType)
{
	m_TimerRef = 0;
	m_ACTimerID = 0;
	m_bPriorityInit = FALSE;

	m_bTimerThreadOn = FALSE;
	m_hTimerThread = NULL;
	m_dwThreadId = 0;

	m_bReclaimFlag = FALSE;

	m_waitPool.clear();
	QueryPerformanceFrequency(&m_freq);
	m_halfMsec.QuadPart = m_freq.QuadPart / 2000;
	m_freq.QuadPart /= 1000;   // convert to msecs
	InitializeCriticalSection(&m_crit);

	TIMECAPS caps;
	if (timeGetDevCaps(&caps, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
    {
        //error       
    } 
	
	switch (eACTimerType)
	{
	case ACTimerType_Auto:
	case ACTimerType_Multi:
		m_timer = timeSetEvent(3, 3, TimerFunc, (DWORD)this, TIME_PERIODIC|TIME_CALLBACK_FUNCTION);
		// if set multi timer fail, use thread mode
		if (m_timer == 0)
		{
			m_bTimerThreadOn = TRUE;
			m_hTimerThread = CreateThread(NULL, 0, ACTimer_TimerProc, this, 0, &m_dwThreadId);
			SetThreadPriority(m_hTimerThread, THREAD_PRIORITY_TIME_CRITICAL/*THREAD_PRIORITY_HIGHEST*/);
		}
		m_eACTimerType = ACTimerType_Multi;
		break;
	case ACTimerType_Thread:
		m_bTimerThreadOn = TRUE;
		m_hTimerThread = CreateThread(NULL, 0, ACTimer_TimerProc, this, 0, &m_dwThreadId);
		SetThreadPriority(m_hTimerThread, THREAD_PRIORITY_TIME_CRITICAL/*THREAD_PRIORITY_HIGHEST*/);
		m_eACTimerType = ACTimerType_Thread;
		break;
	default:
		break;
	}	
}

CAccurateTimer::~CAccurateTimer()
{
	if (!m_bTimerThreadOn && (m_timer != 0))
	{
		timeKillEvent(m_timer), m_timer = 0;
	}
	else if (m_bTimerThreadOn && (m_hTimerThread != NULL))
	{
		m_bTimerThreadOn = FALSE;

		DWORD dwRes = 0;
		dwRes = WaitForSingleObject(m_hTimerThread, INFINITE);
		if(WAIT_OBJECT_0 == dwRes)
		{
			CloseHandle(m_hTimerThread);
			m_hTimerThread = NULL;
			m_dwThreadId = 0;
		}
	}	

	m_waitPool.clear();
	DeleteCriticalSection(&m_crit);
}

void CAccurateTimer::ShepherdACTimerLamb(void)
{
	QueryPerformanceCounter(&s[1]);
	//double diff = (((double)s[1].QuadPart - (double)s[0].QuadPart)/(double)m_freq.QuadPart);
	__int64 now = s[1].QuadPart+m_halfMsec.QuadPart;
	
	ACTIMER_LAMB_MAP::iterator it = m_waitPool.begin();
	while ( it != m_waitPool.end() )
	{
		if ( now >= it->second.timeout.QuadPart )
		{
			// render video
#ifdef THREAD_RENDER
			DhVideoRender *pVideo = (DhVideoRender*)it->second.dwUser;
			SetEvent(pVideo->m_hVideoRender);
#else
			it->second.cbACTimerLambFunc(0, 0, it->second.dwUser, 0, 0);
#endif
			// reset timeout value
			//LARGE_INTEGER now; 
			//QueryPerformanceCounter(&now);
			LARGE_INTEGER t;
			QueryPerformanceCounter(&t);
			__int64 nowPass = t.QuadPart+m_halfMsec.QuadPart;
			it->second.timeout.QuadPart = it->second.interval * m_freq.QuadPart + s[1].QuadPart;
			if(it->second.timeout.QuadPart < nowPass)
			{
				it->second.cbACTimerLambFunc(0, 0, it->second.dwUser, 0, 0);
				QueryPerformanceCounter(&t);
				__int64 nowPass = t.QuadPart+m_halfMsec.QuadPart;
				it->second.timeout.QuadPart = nowPass + 10 * m_freq.QuadPart;
			}
			
			continue;
		}
		++it;
	}
	
	//s[0] = s[1];
}

void CAccurateTimer::ACTimerTick(void)
{
	EnterCriticalSection(&m_crit);
	
	ChangePriority();

	ShepherdACTimerLamb();	
	
	LeaveCriticalSection(&m_crit);
}

void CAccurateTimer::ACTimerTickThread(void)
{
	while (m_bTimerThreadOn)
	{
		EnterCriticalSection(&m_crit);

		ShepherdACTimerLamb();

		LeaveCriticalSection(&m_crit);

		Sleep(2);
	}
}

///////////////////////////////////////////////////////////////
// Function name   : SetACTimer
// Description     : Set Timer method
//                 : 
// Return type     : int  : timer id 
// Argument        : int timeout : 
//                 : LPTIMECALLBACK timerProc
//                 : DWORD dwUser
///////////////////////////////////////////////////////////////
int CAccurateTimer::SetACTimer(int interval, LPTIMECALLBACK timerProc, DWORD dwUser)
{	
	if ( interval > 0 )   // anything to wait on ?
	{
		ACTIMER_LAMB tACTimerLamb;
		ZeroMemory(&tACTimerLamb, sizeof(ACTIMER_LAMB));

		tACTimerLamb.key = getACTimerID();
		tACTimerLamb.dwUser = dwUser;
		tACTimerLamb.interval = interval;
		tACTimerLamb.cbACTimerLambFunc = timerProc;

		LARGE_INTEGER now; 
		QueryPerformanceCounter(&now);
		tACTimerLamb.timeout.QuadPart = interval * m_freq.QuadPart + now.QuadPart;

		// wrap pool data access
		EnterCriticalSection(&m_crit);

		m_waitPool.insert(ACTIMER_LAMB_MAP::value_type(tACTimerLamb.key, tACTimerLamb));
		AddRef();
		m_bReclaimFlag = FALSE;

		LeaveCriticalSection(&m_crit);

		return tACTimerLamb.key;
	}

	return -1;
}

BOOL CAccurateTimer::ResetACTimer(int iTimerID, int interval)
{
	if (iTimerID <= 0) return FALSE;

	EnterCriticalSection(&m_crit);
	ACTIMER_LAMB_MAP::iterator it = m_waitPool.find(iTimerID);
	if (it == m_waitPool.end())
	{
		LeaveCriticalSection(&m_crit);
		return FALSE;
	}

	it->second.interval = interval;
	
	LeaveCriticalSection(&m_crit);

	return TRUE;
}

BOOL CAccurateTimer::KillACTimer(int iTimerID)
{
	if (iTimerID <= 0) return FALSE;

	EnterCriticalSection(&m_crit);
	ACTIMER_LAMB_MAP::iterator it = m_waitPool.find(iTimerID);
	if (it == m_waitPool.end())
	{
		LeaveCriticalSection(&m_crit);
		return FALSE;
	}

	m_waitPool.erase(iTimerID);
	Release();

	if (m_TimerRef == 0) m_bReclaimFlag = TRUE;

	LeaveCriticalSection(&m_crit);

	return TRUE;
}

void CAccurateTimer::ChangePriority(void)
{
	if (!m_bPriorityInit)
	{
		if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL/*THREAD_PRIORITY_HIGHEST*/))
		{
			DWORD err = GetLastError();
		}

		m_bPriorityInit = TRUE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------
CACTimerManager::CACTimerManager()
{
	m_ACTimerObjList.clear();
	
	timeBeginPeriod(1);
	InitializeCriticalSection(&m_crit);

	m_VideoPerTimer = VIDEORENDER_PER_ACTIMER;
}

CACTimerManager::~CACTimerManager()
{
	ACTIMER_OBJ_LIST::iterator it = m_ACTimerObjList.begin();
	while ( it != m_ACTimerObjList.end())
	{
		if ((*it)->get_Ref() <= 0)
		{
			delete *it;
			it = m_ACTimerObjList.erase(it);
		}
		else
			it++;
	}

	m_ACTimerObjList.clear();

	timeEndPeriod(1);
	DeleteCriticalSection(&m_crit);
}

BOOL CACTimerManager::GetACTimer(CAccurateTimer** pACTimerObj, tACTimerType eACTimerType)
{
	int iTimerRefSum = 0;
	int iActiveCount = 0;

	TimerReclaim();

	EnterCriticalSection(&m_crit);

	iActiveCount = m_ACTimerObjList.size();

// 	ACTIMER_OBJ_LIST::iterator it;
// 	for (it = m_ACTimerObjList.begin(); it != m_ACTimerObjList.end(); it++)
// 	{
// 		iTimerRefSum += (*it)->get_Ref();
// 	}

// 	if (iTimerRefSum >= (m_VideoPerTimer<<4))
// 	{
// 		LeaveCriticalSection(&m_crit);
// 		return FALSE;
// 	}

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	if ((iActiveCount < sysInfo.dwNumberOfProcessors))
	{
		CAccurateTimer* pACTimer = new CAccurateTimer(eACTimerType);
		m_ACTimerObjList.push_back(pACTimer);

		*pACTimerObj = pACTimer;
	}
	else
	{
		ACTIMER_OBJ_LIST::iterator itMinRef;
		itMinRef = m_ACTimerObjList.begin();

		ACTIMER_OBJ_LIST::iterator it;
		for (it = m_ACTimerObjList.begin()+1; it != m_ACTimerObjList.end(); it++)
		{
			if ((eACTimerType == (*it)->getACTimerType()) || (ACTimerType_Auto == eACTimerType))
			{
				if ((*itMinRef)->get_Ref() > (*it)->get_Ref())
				{
					itMinRef = it;
				}
			}
		}

		if ((eACTimerType == (*itMinRef)->getACTimerType()) || (ACTimerType_Auto == eACTimerType))
		{
			if ((*itMinRef)->get_Ref() < m_VideoPerTimer)
			{
				*pACTimerObj = *itMinRef;
			}
			else
			{
				CAccurateTimer* pACTimer = new CAccurateTimer(eACTimerType);
				m_ACTimerObjList.push_back(pACTimer);
				*pACTimerObj = pACTimer;
			}
		}
		else
		{
			CAccurateTimer* pACTimer = new CAccurateTimer(eACTimerType);
			m_ACTimerObjList.push_back(pACTimer);
			*pACTimerObj = pACTimer;
		}
	}

	(*pACTimerObj)->set_ReclaimFlag(FALSE);

	LeaveCriticalSection(&m_crit);

	return TRUE;
}

void CACTimerManager::TimerReclaim(void)
{	
	EnterCriticalSection(&m_crit);

// 	SYSTEM_INFO sysInfo;
// 	GetSystemInfo(&sysInfo);

	ACTIMER_OBJ_LIST::iterator it;
	for (it = m_ACTimerObjList.begin(); it != m_ACTimerObjList.end();)
	{
		if (((*it)->get_ReclaimFlag()) && ((*it)->get_Ref() <= 0))
		{
			delete *it;
			it = m_ACTimerObjList.erase(it);
		}
		else
			it++;
	}

	LeaveCriticalSection(&m_crit);
}

void CACTimerManager::SetVideoPerTimerVal(int val)
{
	if (val <= 0)
	{
		return;
	}

	m_VideoPerTimer = val;
}




