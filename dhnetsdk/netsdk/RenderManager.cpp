
#include "RenderManager.h"
#include "VideoRender.h"

CRenderManager::CRenderManager(CManager *pManager)
	: m_pManager(pManager)
{
//	InitializeCriticalSection(&m_csRenders);
}

CRenderManager::~CRenderManager()
{
//	DeleteCriticalSection(&m_csRenders);
}

BOOL CRenderManager::Init()
{
	return TRUE;
}

BOOL CRenderManager::Uninit()
{
//	EnterCriticalSection(&m_csRenders);
	m_csRenders.Lock();

	list<RenderResource *>::iterator it = m_renders.begin();
	
	while (it != m_renders.end()) 
	{
		if ((*it) && (*it)->render)
		{
			delete (*it)->render;
		}
		if (*it)
		{
			delete (*it);
		}
		it++;
	}
	m_renders.clear();

//	LeaveCriticalSection(&m_csRenders);
	m_csRenders.UnLock();

	return TRUE;
}

CVideoRender* CRenderManager::GetRender(HWND hwnd)
{
	if (!hwnd) 
	{
		return 0;
	}
	
	CVideoRender* result = NULL;

//	EnterCriticalSection(&m_csRenders);
	m_csRenders.Lock();
	
	list<RenderResource *>::iterator it = m_renders.begin();

	//查找空闲render资源
	it = m_renders.begin();
	for (; it != m_renders.end(); it++)
	{
		if ((*it) && (*it)->bAvailable) 
		{
			(*it)->bAvailable = FALSE;
			(*it)->hwindow = hwnd;
			int nRet = (*it)->render->ChangeHwnd(hwnd);
			if (nRet >= 0)
			{
				result = (*it)->render;
				break;
			}
			else
			{
				//changeHwnd失败，这个。。。
			}
		}
	}

	if (NULL == result) 
	{
		//否则创建新的render
		RenderResource* newresource = 0;
		CVideoRender* newrender = 0;
		
		newrender = new CVideoRender(hwnd);
		if (!newrender)
		{
			goto e_clearup;
		}

		newresource = new RenderResource;
		if (!newresource)
		{
			goto e_clearup;
		}

		newresource->render = newrender;
		newresource->bAvailable = FALSE;
		newresource->hwindow = hwnd;
		m_renders.push_back(newresource);

		result = newrender;
	}
	
//	LeaveCriticalSection(&m_csRenders);
	m_csRenders.UnLock();
	return result;

e_clearup:

//	LeaveCriticalSection(&m_csRenders);
	m_csRenders.UnLock();
	return (CVideoRender*)-1;
}

void CRenderManager::ReleaseRender(CVideoRender* rls_render)
{
	if (!rls_render)
	{
		return;
	}
	
//	EnterCriticalSection(&m_csRenders);
	m_csRenders.Lock();

	list<RenderResource *>::iterator it = m_renders.begin();
	for (; it != m_renders.end(); it++)
	{
		if(*it)
		{
			if ((*it)->render == rls_render) 
			{
				//正常render，更改空闲标志
				(*it)->bAvailable = TRUE;
				break;
			}
		}
	}
	
//	LeaveCriticalSection(&m_csRenders);
	m_csRenders.UnLock();

	return;
}

