// Tread.h: interface for the CTread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREAD_H__948D05D2_192C_407A_BBDD_E0BB0886FCD4__INCLUDED_)
#define AFX_TREAD_H__948D05D2_192C_407A_BBDD_E0BB0886FCD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include <map>

class CRef
{
public:
	CRef():m_lLifeCount(1)
	{
	}
	virtual ~CRef()
	{
	}
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
private:
	LONG			m_lLifeCount;
};

class CTask : public CRef
{
public:
	CTask(){}
	virtual ~CTask(){}
	
public:
	//任务
	virtual int	Task() = 0;
	//任务是否准备好
	virtual int Prepare() = 0;
	virtual int IsPrepared() = 0;
	//任务是否完成
	virtual int Done() = 0;
	virtual int IsDone() = 0;

};

/*
 * 任务中心.	
 */
class CTaskCenter
{
public:
	// 顺序访问接口
	CTask*	GetFirstTask(void** pvPairData = NULL);
	CTask*	GetNextTask(void** pvPairData = NULL, CTask* pTask = NULL);//获取任务使用权
	BOOL	BackTask(CTask* pTask);			 //返回任务，使用后都需要返回任务使用权
	BOOL	IsEnd(void);

	// 创建添加删除接口
	void*	GetPairData(CTask* pTask);
	BOOL	AddTask(CTask *pTask, void* pvPairData = NULL);
	BOOL	DecTask(CTask *pTask);
	DWORD	TaskNum(void);//任务数目
	
private:
	SPTools::CReadWriteMutex	m_csListTask;
	std::map<CTask *, void*>	m_mapTask;
	//没有定义多个iterator,是因为没有多个线程共享数据.
	std::map<CTask *, void*>::iterator m_itTask;
};

class CDynamicThread //: public IThread
{
public:
	CDynamicThread();
	virtual ~CDynamicThread();
	friend void WINAPI WorkThread(void* pvData);
public:
	int Run();
	int Stop();
public:
	//动态创建的任务，任务对象的生命周期要受到控制，不然可能内存膨胀。
	enum eLifeCountMode{eFromTaskPrepared, eFromTaskDone};
	enum eTaskType{eBurnFileTrans};
	CTask*	CreateTask(eTaskType eType, eLifeCountMode eMode, DWORD m_dwLife, DWORD dwUserData);
	int		DeleteTask(CTask* pTask);
	int		GetUserData(CTask* pTask, DWORD& dwUserData);

private:
	int	ExitThread(void);
	int WorkFunc(DWORD dwTimePass);
	struct TaskLifecycle
	{
		eLifeCountMode	m_eMode;
		DWORD			m_dwLife;
		DWORD			m_dwUserData;
	};	
private:
	//如果希望数据共享，可放在类外定义.
	CTaskCenter	m_cTaskCenter;

	OS_THREAD	m_osThread;
	OS_EVENT	m_osEvent;

	BOOL		m_bRun;						//是否在工作.
};

/************************************************************************/
/* 刻录文件上传                                                         */
/************************************************************************/
typedef void (CALLBACK *fTransFileCallBack)(LONG lHandle, int nTransType, int nState, int nSendSize, int nTotalSize, DWORD dwUser);
class CManager;
struct afk_device_prob_s;

class CTaskBurnFileTrans : public CTask
{
public:
	CTaskBurnFileTrans();
	virtual ~CTaskBurnFileTrans();
	int	Task();
	int Prepare();
	int IsPrepared();
	int Done();
	int IsDone();
public:
	int SetParam(CManager *mgr, char *szFileName, int nFileSize, char* szIP, int nPort, fTransFileCallBack cbCallBack, DWORD dwUser, int nWaittime = 500);
private:
	//发送文件
	FILE*				m_pFileSend;

	char				m_szIP[48];
	int					m_nPort;
	int					m_nWaittime;

	fTransFileCallBack	m_cbCallBack;
	DWORD				m_dwUser;
	int					m_nSendSize;//已发送的大小
	int					m_nFileSize;

	CManager*			m_mgr;
	afk_device_prob_s*	m_prob;
	LONG				m_lTcpHandle;

	BOOL				m_bPrepared;
	BOOL				m_bDone;
};


#endif // !defined(AFX_TREAD_H__948D05D2_192C_407A_BBDD_E0BB0886FCD4__INCLUDED_)
