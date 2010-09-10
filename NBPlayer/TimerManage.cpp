#include "StdAfx.h"
#include "TimerManage.h"
#include "SingleZenoLock.h"
#include <assert.h>
#include <MMSystem.h>

#define	MAX_HIGHPRECISION_PERIOD		20

CTimerManage CTimerManage::m_staticThis;

CTimerManage::CTimerManage(void)
{
	m_bManageInit = false;

	m_nPreciseTimerCount = 0;
	m_nTimerIdSeek = 0;
}

CTimerManage::~CTimerManage(void)
{
	if (m_bManageInit)
	{
		UnInitManage();
	}
}

CTimerManage* CTimerManage::Instance()
{
	return &m_staticThis;
}

bool CTimerManage::InitManage()
{
	timeBeginPeriod(1);

	m_nPreciseTimerCount = 0;

	//创建系统计时器
	for (int i = 0;i < 1/*sysInfo.dwNumberOfProcessors*/;i++)
	{
		int nTimerId = timeSetEvent(1, 1, StaticThreadProcTimer, i, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
		m_vecTimerId.push_back(nTimerId);
	}

	//创建自己的计时器
	SYSTEM_INFO sysInfo;
	::GetSystemInfo(&sysInfo);
	m_bCloseThread = false;
	for (int i = 0;i < sysInfo.dwNumberOfProcessors;i++)
	{
		AX_thread_t threadId;
		AX_hthread_t hThread;
		if (AX_OS::thr_create(StaticThreadProcMy, (void*)i, HIGH_PRIORITY_CLASS|CREATE_SUSPENDED, &threadId, &hThread, THREAD_PRIORITY_TIME_CRITICAL) == 0)
		{
			HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
			LARGE_INTEGER liDueTime;
			liDueTime.QuadPart = -10000;
			SetWaitableTimer(hTimer, &liDueTime, 1, NULL, NULL, 0);

			m_vecSysTimer.push_back(hTimer);
			m_vecThread.push_back(hThread);
		}
		else
		{
			break;
		}
	}
	for (int i = 0;i < m_vecThread.size();i++)
	{
		AX_OS::thr_continue(m_vecThread[i]);
	}

	m_bManageInit = true;
	return true;
}

void CTimerManage::UnInitManage()
{
	m_bManageInit = false;

	for (int i = 0;i < m_vecTimerId.size();i++)
	{
		timeKillEvent(m_vecTimerId[i]);
	}
	m_vecTimerId.clear();

	timeEndPeriod(1);

	m_bCloseThread = true;
	for (int i = 0;i < m_vecThread.size();i++)
	{
		AX_thr_func_return ret;
		AX_OS::thr_join(m_vecThread[i], &ret);
		CloseHandle(m_vecSysTimer[i]);
	}
	m_vecSysTimer.clear();
	m_vecThread.clear();

	CSingleZenoLock lock(m_mtxTimer);
	m_vecTimer.clear();
}

int CTimerManage::SetTimer(uint32 nPeriod, int nContext, OnTimerFuncType funcCB)
{
	if (m_bManageInit == false)
	{
		return 0;
	}

	CSingleZenoLock lock(m_mtxTimer);

	while (true)
	{
		m_nTimerIdSeek++;
		if (m_nTimerIdSeek == 0)
		{
			m_nTimerIdSeek++;
		}

		bool bExist = false;
		for (int i = 0;i < m_vecTimer.size();i++)
		{
			if (m_vecTimer[i].nTimerId == m_nTimerIdSeek)
			{
				bExist = true;
				break;
			}
		}
		if (bExist == false)
		{
			break;
		}
	}

	TimerInfo infoTimer;
	infoTimer.nTimerId = m_nTimerIdSeek;
	infoTimer.nContext = nContext;
	infoTimer.nPeriod = max(1, nPeriod);
	infoTimer.funcCB = funcCB;
	infoTimer.nNextPeriod = infoTimer.nPeriod;
	infoTimer.tLastActiveTime = (uint32)AX_OS::get_time();

	//从低到高排序
	if (infoTimer.nPeriod < MAX_HIGHPRECISION_PERIOD)
	{
		m_vecTimer.insert(m_vecTimer.begin(), infoTimer);
	}
	else
	{
		m_vecTimer.push_back(infoTimer);
	}

	//
	assert(m_vecTimer.size() < 1000);

	return m_nTimerIdSeek;
}

void CTimerManage::DelayTimer(int nTimerId, int nDelay)
{
	CSingleZenoLock lock(m_mtxTimer);
	for (int i = 0;i < m_vecTimer.size();i++)
	{
		TimerInfo& infoTimer = m_vecTimer[i];
		if (infoTimer.nTimerId == nTimerId)
		{
			if (nDelay == DELAY_PAUSE_TIMER)
			{
				infoTimer.nNextPeriod = DELAY_PAUSE_TIMER;
			}
			else
			{
				infoTimer.nNextPeriod = infoTimer.nPeriod + (uint32)nDelay;
			}
			break;
		}
	}
}

void CTimerManage::ModifyTimerPeriod(int nTimerId, uint32 nPeriod)
{
	CSingleZenoLock lock(m_mtxTimer);
	for (int i = 0;i < m_vecTimer.size();i++)
	{
		TimerInfo& infoTimer = m_vecTimer[i];
		if (infoTimer.nTimerId == nTimerId)
		{
			bool bModify = ((infoTimer.nPeriod < MAX_HIGHPRECISION_PERIOD) && (nPeriod >= MAX_HIGHPRECISION_PERIOD))
							|| ((infoTimer.nPeriod >= MAX_HIGHPRECISION_PERIOD) && (nPeriod < MAX_HIGHPRECISION_PERIOD));

			infoTimer.nPeriod = max(1, nPeriod);
			infoTimer.nNextPeriod = infoTimer.nPeriod;

			if (bModify)
			{
				TimerInfo tmp = infoTimer;
				m_vecTimer.erase(m_vecTimer.begin() + i);
				if (tmp.nPeriod < MAX_HIGHPRECISION_PERIOD)
				{
					m_vecTimer.insert(m_vecTimer.begin(), tmp);
				}
				else
				{
					m_vecTimer.push_back(tmp);
				}
			}

			break;
		}
	}
}

void CTimerManage::KillTimer(int nTimerId)
{
	CSingleZenoLock lock(m_mtxTimer);
	for (int i = 0;i < m_vecTimer.size();i++)
	{
		TimerInfo& infoTimer = m_vecTimer[i];
		if (infoTimer.nTimerId == nTimerId)
		{
			m_vecTimer.erase(m_vecTimer.begin() + i);
			break;
		}
	}
}

void CALLBACK CTimerManage::StaticThreadProcTimer(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	int nTimerIndex = dwUser;
	m_staticThis.ThreadProcTimer(nTimerIndex);
}

void CTimerManage::ThreadProcTimer(int nTimerIndex)
{
	if (m_bManageInit == false)
	{
		return;
	}

	uint32 nCurTime = (uint32)AX_OS::get_time();

	list<TimerInfo> listActive;

	CSingleZenoLock lock(m_mtxTimer);
	m_nPreciseTimerCount = 0;
	for (int i = 0;i < m_vecTimer.size();i++)
	{
		TimerInfo& infoTimer = m_vecTimer[i];
		if ((infoTimer.nPeriod < MAX_HIGHPRECISION_PERIOD)
			|| (m_vecThread.size() == 0))
		{
			m_nPreciseTimerCount++;
			if ((infoTimer.nNextPeriod != DELAY_PAUSE_TIMER)
				&& ((nCurTime - infoTimer.tLastActiveTime) > infoTimer.nNextPeriod))
			{
				listActive.push_back(infoTimer);

				infoTimer.tLastActiveTime = nCurTime - (nCurTime - infoTimer.tLastActiveTime) % infoTimer.nPeriod;
				infoTimer.nNextPeriod = infoTimer.nPeriod;
			}
		}
		else
		{
			break;
		}
	}
	lock.release();

	for (list<TimerInfo>::iterator iter = listActive.begin();iter != listActive.end();++iter)
	{
		TimerInfo& infoTimer = *iter;
		if (infoTimer.funcCB)
		{
			infoTimer.funcCB(infoTimer.nTimerId, infoTimer.nContext);
		}
	}
}

void* CTimerManage::StaticThreadProcMy(void* pParam)
{
	int nThreadIndex = (int)pParam;
	while (m_staticThis.m_bManageInit == false)
	{
		AX_OS::sleep(10);
	}
	m_staticThis.ThreadProcMy(nThreadIndex);
	return NULL;
}

void CTimerManage::ThreadProcMy(int nThreadIndex)
{
	HANDLE hTimer = m_vecSysTimer[nThreadIndex];

	while (m_bCloseThread == false)
	{
		WaitForSingleObject(hTimer, 1);

		uint32 nCurTime = (uint32)AX_OS::get_time();

		list<TimerInfo> listActive;

		CSingleZenoLock lock(m_mtxTimer);
		for (int i = m_nPreciseTimerCount + nThreadIndex;i < m_vecTimer.size();i += m_vecThread.size())
		{
			TimerInfo& infoTimer = m_vecTimer[i];
			if ((infoTimer.nNextPeriod != DELAY_PAUSE_TIMER)
				&& ((nCurTime - infoTimer.tLastActiveTime) > infoTimer.nNextPeriod))
			{
				listActive.push_back(infoTimer);

				infoTimer.tLastActiveTime = nCurTime - (nCurTime - infoTimer.tLastActiveTime) % infoTimer.nPeriod;
				infoTimer.nNextPeriod = infoTimer.nPeriod;
			}
		}
		lock.release();

		for (list<TimerInfo>::iterator iter = listActive.begin();iter != listActive.end();++iter)
		{
			TimerInfo& infoTimer = *iter;
			if (infoTimer.funcCB)
			{
				infoTimer.funcCB(infoTimer.nTimerId, infoTimer.nContext);
			}
		}
	}
}