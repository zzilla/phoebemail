//------------------------------------------------------------------------------
// File: AccurateTimer.h
//
// Desc: Playsdk code - header file for timer used by video render
//
// Copyright (c) ZheJiang Dahua Technology Stock Co.Ltd.  All rights reserved.
//------------------------------------------------------------------------------
#ifndef __ACCURATE_TIMER_H__
#define __ACCURATE_TIMER_H__

#pragma warning ( disable : 4786 )

#include <windows.h>
#include <conio.h>
#include <time.h>
#include <map>
#include <vector>
#include <process.h>
#include <mmsystem.h>

////////////////////////////////////////////////////////////////////////////////

typedef enum __tACTimerType
{
	ACTimerType_Auto = 0,
	ACTimerType_Multi,
	ACTimerType_Thread
} tACTimerType;

typedef struct
{
	int key;
	DWORD dwUser;
	int interval;
	LARGE_INTEGER  timeout;
	LPTIMECALLBACK cbACTimerLambFunc;
}ACTIMER_LAMB, *PACTIMER_LAMB;

typedef	std::map<int, ACTIMER_LAMB> ACTIMER_LAMB_MAP;

////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------
class CAccurateTimer
{
public:
	static CAccurateTimer* Instance()
	{
		return &m_instance;
	};

	CAccurateTimer(tACTimerType eACTimerType=ACTimerType_Auto);
	virtual ~CAccurateTimer();
	
	int  SetACTimer(int interval, LPTIMECALLBACK timerProc, DWORD dwUser);
	BOOL ResetACTimer(int iTimerID, int interval);
	BOOL KillACTimer(int iTimerID);

	void ACTimerTick(void);
	void ACTimerTickThread(void);
	void ShepherdACTimerLamb(void);

	void ChangePriority(void);

	int  get_Ref(void) { return m_TimerRef; }
	BOOL get_ReclaimFlag(void) { return m_bReclaimFlag; }
	void set_ReclaimFlag(BOOL bVal) { m_bReclaimFlag = bVal; }

	int  getACTimerID(void) { return ++m_ACTimerID; }

	tACTimerType getACTimerType(void) { return m_eACTimerType;}
	
	inline ULONG AddRef(void) { ++m_TimerRef; return m_TimerRef; }
	inline ULONG Release(void) { m_TimerRef =  (--m_TimerRef) < 0 ? 0 : m_TimerRef; return m_TimerRef; }
	
	inline double        Absolute(double val) { return val > 0 ? val : -val; }
	inline LARGE_INTEGER GetFrequency()       { return m_freq; }
	
private:
	static CAccurateTimer   m_instance;

	LARGE_INTEGER		s[2];
	LARGE_INTEGER		m_freq;
	LARGE_INTEGER		m_halfMsec;
	CRITICAL_SECTION	m_crit;
	ACTIMER_LAMB_MAP	m_waitPool;
	
	BOOL				m_bReclaimFlag;
	BOOL				m_bPriorityInit;
	int					m_ACTimerID;
	long				m_TimerRef;
	MMRESULT			m_timer;
	tACTimerType		m_eACTimerType;

	// timer thread
	HANDLE				m_hTimerThread;
	DWORD				m_dwThreadId;
	BOOL				m_bTimerThreadOn;
};

////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------
typedef	std::vector<CAccurateTimer*> ACTIMER_OBJ_LIST;
//-------------------------------------------------------------------------
class CACTimerManager
{
public:
	CACTimerManager();
	~CACTimerManager();

	BOOL GetACTimer(CAccurateTimer** pACTimerObj, tACTimerType eACTimerType);
	void TimerReclaim(void);
	void SetVideoPerTimerVal(int val);

private:
	ACTIMER_OBJ_LIST	m_ACTimerObjList;
	CRITICAL_SECTION	m_crit;
	int					m_VideoPerTimer;
};



#endif  // __ACCURATE_TIMER_H__
