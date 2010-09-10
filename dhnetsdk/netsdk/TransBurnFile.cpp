// Tread.cpp: implementation of the CTread class.
//
//////////////////////////////////////////////////////////////////////

#include "TransBurnFile.h"
#include "manager.h"
#include "devconfigex.h"
#include "../dvr/dvr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void WINAPI WorkThread(void* pvData)
{
	CDynamicThread* pThread = (CDynamicThread*)pvData;
	if(NULL == pThread)
	{
		return;
	}
	DWORD dwRelexTime = 0;
	DWORD dwTimeEnd = GetTickCount();
	DWORD dwTimeStart = GetTickCount();
	while (TRUE)
	{
		//开始计算时间
		dwTimeStart = dwTimeEnd;
		dwTimeEnd = GetTickCount();

		DWORD dwRet = WaitForSingleObjectEx(pThread->m_osEvent, 10);
		if (WAIT_OBJECT_0 == dwRet)
		{
			break;
		}
		//工作
		int nRet = pThread->WorkFunc(dwTimeEnd - dwTimeStart);

		if(nRet <= 0)//如果线程无事可做
		{
			//累加线程休息的时间
			dwRelexTime+= (dwTimeEnd - dwTimeStart);
			if(dwRelexTime >= 1000*60)//1000*60 = 一分钟
			{
				pThread->ExitThread();
				break;
			}
		}
		else
		{	
			//一旦有事可做将休息时间重置.
			dwRelexTime = 0;
		}
	}
	
	return;
}



int CDynamicThread::Run()
{
	if(m_bRun)//如果线程已经开启，并不创建新的线程.
	{
		return 1;
	}

	CreateEventEx(m_osEvent, TRUE, FALSE);
	DWORD dwThreadID = 0;
	int nRet = CreateThreadEx(m_osThread, 0, (LPTHREAD_START_ROUTINE)WorkThread, (void*)this, 0, &dwThreadID);
	if (nRet < 0)
	{
		m_bRun = FALSE;
		return -1;
	}
	else
	{
		m_bRun = TRUE;
		return 1;
	}
}

int CDynamicThread::ExitThread()
{
	m_bRun = FALSE;
	SetEventEx(m_osEvent);
	CloseThreadEx(m_osThread);
	CloseEventEx(m_osEvent);

	return 1;
}

int CDynamicThread::Stop()
{
	SetEventEx(m_osEvent);
	DWORD dwRet = WaitForSingleObjectEx(m_osThread, 500);
	if (dwRet != WAIT_OBJECT_0)
	{
		m_bRun = FALSE;
		TerminateThreadEx(m_osThread, 0xFFFFFFFF);
	}

	CloseThreadEx(m_osThread);
	CloseEventEx(m_osEvent);
	m_bRun = FALSE;

	return 1;
}


CDynamicThread::CDynamicThread()
{
	m_bRun = FALSE;
}

CDynamicThread::~CDynamicThread()
{

}

int CDynamicThread::WorkFunc(DWORD dwTimePass)
{
	CTask *pTask = NULL;
	TaskLifecycle *pLife = NULL;

	pTask = m_cTaskCenter.GetFirstTask((void**)&pLife);
	while(!m_cTaskCenter.IsEnd())
	{
		if(NULL != pTask)
		{
			if( pTask->IsPrepared() > 0 )
			{		
				pTask->Task();
				if(pLife->m_eMode == eFromTaskPrepared)
				{
					pLife->m_dwLife -= dwTimePass;
				}
			}
			else
			{						
				pTask->Prepare();
			}
			
			if(pTask->IsDone() > 0)
			{		
				pTask->Done();
				if(pLife->m_eMode == eFromTaskDone)
				{
					pLife->m_dwLife -= dwTimePass;
				}
			}
		}

		m_cTaskCenter.BackTask(pTask);
		if(pLife->m_dwLife > 0x80000000)
		{
			m_cTaskCenter.DecTask(pTask);
			pTask->DecRef();
			if(NULL != pLife)
			{
				delete pLife;
			}
		}

		pTask = m_cTaskCenter.GetNextTask((void**)&pLife);
	}	
	
	return m_cTaskCenter.TaskNum();
}

/*
 *  根据枚举类型创建任务。
 */
CTask* CDynamicThread::CreateTask(eTaskType eType, eLifeCountMode eMode, DWORD m_dwLife, DWORD dwUserData)
{
	TaskLifecycle *pTaskLifecycle = NULL;
	pTaskLifecycle = new TaskLifecycle;
	if(NULL == pTaskLifecycle)
	{
		return NULL;
	}
	
	memset(pTaskLifecycle, 0 , sizeof(TaskLifecycle));
	pTaskLifecycle->m_eMode = eMode;
	pTaskLifecycle->m_dwLife = m_dwLife;
	pTaskLifecycle->m_dwUserData = dwUserData;

	//创建任务
	CTask* pTask = NULL;
	if(eType == eBurnFileTrans)
	{
		pTask = new CTaskBurnFileTrans;
	}
	

	if(NULL == pTask)
	{
		delete pTaskLifecycle;
		return NULL;
	}

	m_cTaskCenter.AddTask(pTask, (void*)pTaskLifecycle);
	return pTask;
}

/*
 *	删除任务 
 */
int	CDynamicThread::DeleteTask(CTask* pTask)
{
	TaskLifecycle *pTaskLifecycle = (TaskLifecycle *)m_cTaskCenter.GetPairData(pTask);
	if(NULL != pTaskLifecycle)
	{
		pTaskLifecycle->m_dwLife = 0xffffffff;//将生命周期结束，并不真正删除.
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
 *	获取用户暂时存储的数据,一般用于保存对象的id,也可以保存其它东西.
 */
int CDynamicThread::GetUserData(CTask* pTask, DWORD& dwUserData)
{
	TaskLifecycle *pTaskLifecycle = (TaskLifecycle *)m_cTaskCenter.GetPairData(pTask);
	if(NULL == pTaskLifecycle)
	{
		return -1;
	}
	
	dwUserData = pTaskLifecycle->m_dwUserData;
	return 1;
}
/*****************************************************************************/



/************************************************************************/
/* CTaskCenter                                                          */
/************************************************************************/
//顺序访问接口
CTask* CTaskCenter::GetFirstTask(void** pvPairData /* = NULL */)
{
	SPTools::CReadWriteMutexLock Lock(m_csListTask, false, true, false);
	if(m_mapTask.size() > 0)
	{
		m_itTask = m_mapTask.begin();
		(*(m_itTask)).first->AddRef();
		if(pvPairData != NULL)
		{
			*pvPairData = (*(m_itTask)).second;
		}
		return (*(m_itTask)).first;
	}
	
	return NULL;
}
//可以指定从那个开始，也可以默认
CTask* CTaskCenter::GetNextTask(void** pvPairData /* = NULL */, CTask* pTask /* = NULL */)
{
	SPTools::CReadWriteMutexLock Lock(m_csListTask, false, true, false);
	if(m_mapTask.size() <= 0)//如果map为空就进行++操作，会导致++无法进行.
	{
		return NULL;
	}
	
	if(NULL != pTask)
	{
		std::map<CTask *, void *>::iterator it = m_mapTask.find(pTask);
		if(it != m_mapTask.end())
		{
			m_itTask = it;
		}
	}

	m_itTask++;
	if(m_itTask != m_mapTask.end())
	{
		if(pvPairData != NULL)
		{
			*pvPairData = (*(m_itTask)).second;
		}
		(*(m_itTask)).first->AddRef();
		return (*(m_itTask)).first;
	}

	return NULL;
}

BOOL CTaskCenter::BackTask(CTask* pTask)
{
	if(NULL == pTask)
	{
		return FALSE;
	}

	SPTools::CReadWriteMutexLock Lock(m_csListTask, false, true, false);
	std::map<CTask *, void *>::iterator it = m_mapTask.find(pTask);
	if(it != m_mapTask.end())
	{
		pTask->DecRef();
		return TRUE;
	}
	
	return FALSE;
}

BOOL CTaskCenter::IsEnd(void)
{
	SPTools::CReadWriteMutexLock Lock(m_csListTask, false, true, false);
	if(m_itTask == m_mapTask.end())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
// 与CTask*对应的数据，可自己定义.
void*  CTaskCenter::GetPairData(CTask *pTask)
{
	SPTools::CReadWriteMutexLock Lock(m_csListTask, false, true, false);
	if(NULL != pTask)
	{
		std::map<CTask *, void *>::iterator it = m_mapTask.find(pTask);
		if(it != m_mapTask.end())
		{
			return (*(it)).second;
		}
	}
	else
	{
		return (*(m_itTask)).second;
	}

	return NULL;
}
// 添加删除接口
BOOL CTaskCenter::AddTask(CTask *pTask, void* pvPairData)
{
	if(NULL == pTask)
	{
		return FALSE;
	}

	std::map<CTask *, void *>::iterator it = m_mapTask.find(pTask);
	if(it != m_mapTask.end())
	{
		return FALSE;
	}

	SPTools::CReadWriteMutexLock Lock(m_csListTask);
	m_mapTask[pTask] = pvPairData;

	return TRUE;
}

BOOL CTaskCenter::DecTask(CTask *pTask)
{
	if(pTask == NULL)
	{
		return FALSE;
	}
	
	SPTools::CReadWriteMutexLock Lock(m_csListTask);
	std::map<CTask *, void *>::iterator it = m_mapTask.find(pTask);
	if(it != m_mapTask.end())
	{
		if(it == m_itTask)
		{
			m_mapTask.erase(m_itTask++);
			return TRUE;
		}
		m_mapTask.erase(it);
		return TRUE;
	}

	return FALSE;
}

DWORD CTaskCenter::TaskNum(void)
{
	return m_mapTask.size();
}


/************************************************************************/
/* class CTaskBurnFileTrans  && class   CTaskTransFileCallBack          */
/************************************************************************/

CTaskBurnFileTrans::CTaskBurnFileTrans()
{
	m_bPrepared = FALSE;
	m_bDone = FALSE;
	
	m_pFileSend = NULL;
	memset(m_szIP, 0, 48);
	m_nPort = 0;
	m_nWaittime = 500;
	m_nSendSize = 0;
	m_cbCallBack = NULL;
	m_dwUser = 0;
	m_lTcpHandle = NULL;
//	m_prob = mgr.m_pdeviceprob;
#ifdef _DEBUG
	char szAddr[40] = {0};
	sprintf(szAddr, "CTaskBurnFileTrans() Addr : 0x%x\n", (LONG)this);
	OutputDebugString(szAddr);
#endif
}

CTaskBurnFileTrans::~CTaskBurnFileTrans()
{
	if(NULL != m_prob)
	{
		m_prob->delete_tcp_socket(m_lTcpHandle);
	}

	if(NULL != m_pFileSend)
	{
		fclose(m_pFileSend);
	}

#ifdef _DEBUG
	char szAddr[40] = {0};
	sprintf(szAddr, "~CTaskBurnFileTrans() Addr : 0x%x\n", (LONG)this);
	OutputDebugString(szAddr);
#endif
}

int	CTaskBurnFileTrans::Task()
{
	char szBuf[1024] = {0};

	if(m_pFileSend == NULL)
	{
		m_bDone = TRUE;
		return -1;
	}
	int nRead = fread(szBuf, sizeof(char), 1024, m_pFileSend); 
	if(nRead < 1024)
	{
		m_bDone = TRUE;//发送完成
	}

	if(NULL != m_prob)
	{
		int nRet = m_prob->write_data(m_lTcpHandle, szBuf, nRead);
		if(nRet < 0)
		{
			m_bDone = TRUE;
			//fseek(m_pFileSend, nRead, SEEK_CUR);
			if(NULL != m_cbCallBack)
			{
				this->AddRef();
				m_cbCallBack((LONG)this, 0x0029, -3, m_nSendSize, m_nFileSize, m_dwUser);
				this->DecRef();
			}
		}
		else
		{
			m_nSendSize += nRead;//发送长度递加
			if(NULL != m_cbCallBack)
			{
				this->AddRef();
				m_cbCallBack((LONG)this, 0x0029, 1, m_nSendSize, m_nFileSize, m_dwUser);
				this->DecRef();
			}
		}
	}
	return 1;
}


int CTaskBurnFileTrans::Prepare()
{
	if(NULL == m_pFileSend)//打开文件失败
	{
		m_bDone = TRUE;
		if(NULL != m_cbCallBack)
		{
			this->AddRef();
			m_cbCallBack((LONG)this, 0x0029, -2, m_nSendSize, m_nFileSize, m_dwUser);
			this->DecRef();
		}
		return -1;
	}

	if(NULL != m_prob)
	{
		m_lTcpHandle = m_prob->new_tcp_socket(m_szIP, m_nPort, m_nWaittime);
	}
	if(m_lTcpHandle <= 0)
	{
		m_bPrepared = FALSE;
		m_bDone = TRUE;
	}
	else
	{
		m_bPrepared = TRUE;
		m_bDone = FALSE;
	}

	return m_bPrepared;
}

int CTaskBurnFileTrans::IsPrepared()
{
	return m_bPrepared;
}

int CTaskBurnFileTrans::Done()
{
	if(m_cbCallBack != NULL)
	{	
		this->AddRef();
		m_cbCallBack((LONG)this, 0x0029, 0, m_nSendSize, m_nFileSize, m_dwUser);
		this->DecRef();
		LONG lHandle = (LONG)this;
		//发送完成，调用sdk接口将对象从线程和任务管理器中删除
		m_mgr->GetDevConfigEx().SetDevNewConfig_FileTrans(NULL, 0x0028, (char *)&lHandle, sizeof(LONG), NULL, 0, m_nWaittime);
	}
	return 1;
}

int CTaskBurnFileTrans::IsDone()
{
	return m_bDone;
}

int CTaskBurnFileTrans::SetParam(CManager *mgr, char *szFileName, int nFileSize, char* szIP, int nPort, fTransFileCallBack cbCallBack, DWORD dwUser, int nWaittime)
{
	if(NULL == mgr || NULL == szFileName || NULL == szIP || NULL == cbCallBack)
	{
		return -1;
	}

	m_nPort = nPort;
	m_nWaittime = nWaittime;
	m_nFileSize = nFileSize;
	m_dwUser = dwUser;
	m_cbCallBack = cbCallBack;
	m_mgr = mgr;
	m_prob = mgr->m_pDeviceProb;

	int nLen = strlen(szIP);
	if( nLen > 48)
	{
		return -1;
	}
	memcpy(m_szIP, szIP, nLen);
	
	m_pFileSend = fopen(szFileName, "rb");
	return 1;
}

/********************************************************************************/



