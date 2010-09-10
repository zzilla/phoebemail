#include "StdAfx.h"
#include "ResManage.h"
#include <assert.h>
#include "SingleZenoLock.h"
#include "PlaySDK.h"
#include "NBPlayer.h"

CResManage CResManage::m_staticThis;

CResManage::CResManage(void)
{
	m_bManageInit = false;
}

CResManage::~CResManage(void)
{
	UnInitManage();
}
	
CResManage* CResManage::Instance()
{
	return &m_staticThis;
}

bool CResManage::InitManage()
{
	m_bManageInit = true;
	return true;
}

void CResManage::UnInitManage()
{
	m_bManageInit = false;
	ClearAllResource();
}

void CResManage::SaveRander(HWND hWnd, VideoRender* pRander)
{
	if (m_bManageInit)
	{
		//渲染对象将不做存储
		if (false)
		{
			CSingleZenoLock lock(m_mtxRander);
			m_mapRander[hWnd].push_front(pRander);
		}
		else
		{
			delete pRander;
		}
	}
	else
	{
		delete pRander;
	}
}

VideoRender* CResManage::LoadRander(HWND hWnd)
{
	if (m_bManageInit == false)
	{
		return NULL;
	}

	CSingleZenoLock lock(m_mtxRander);
	map<HWND, list<VideoRender*> >::iterator iter = m_mapRander.find(hWnd);
	if (iter == m_mapRander.end())
	{
		return NULL;
	}
	list<VideoRender*>& listRander = iter->second;
	VideoRender* pRet = listRander.front();
	listRander.pop_front();
	if (listRander.empty())
	{
		m_mapRander.erase(iter);
	}

	return pRet;
}

void CResManage::SavePort(int nDevType, int nPort, uint32 nBufSize)
{
	if (m_bManageInit)
	{
		if (nDevType == StreamSourceType_Dahua)
		{
			//海康的通道不做缓冲
			PlaySDK_ClearBufData(nDevType, nPort);

			CSingleZenoLock lock(m_mtxPort);
			m_listPort[nDevType].push_front(nPort);
			m_listPortBufSize[nDevType].push_front(nBufSize);
			return;
		}
	}


	PlaySDK_CloseStream(nDevType, nPort);
	CNBPlayer::UnusePort(nDevType, nPort);
}

int CResManage::LoadPort(int nDevType, uint32 nBufSize)
{
	if (m_bManageInit == false)
	{
		return -1;
	}

	CSingleZenoLock lock(m_mtxPort);
	list<int>::iterator iter1 = m_listPort[nDevType].begin();
	list<uint32>::iterator iter2 = m_listPortBufSize[nDevType].begin();
	while (iter1 != m_listPort[nDevType].end())
	{
		if ((*iter2) == nBufSize)
		{
			int nRet = *iter1;
			m_listPort[nDevType].erase(iter1);
			m_listPortBufSize[nDevType].erase(iter2);
			return nRet;
		}
		++iter1;
		++iter2;
	}
	if (m_listPort[nDevType].empty() == false)
	{
		//防止存储的过长，这里减掉一个
		int nPort = m_listPort[nDevType].front();
		m_listPort[nDevType].pop_front();
		m_listPortBufSize[nDevType].pop_front();
		lock.release();

		PlaySDK_CloseStream(nDevType, nPort);
		CNBPlayer::UnusePort(nDevType, nPort);
	}

	return -1;
}

void CResManage::SaveFrameBuffer(char* pBuf)
{
	if (m_bManageInit)
	{
		CSingleZenoLock lock(m_mtxFrameBuffer);
		m_listFrameBuffer.push_front(pBuf);
	}
	else
	{
		delete pBuf;
	}
}

char* CResManage::LoadFrameBuffer()
{
	if (m_bManageInit == false)
	{
		return NULL;
	}

	CSingleZenoLock lock(m_mtxFrameBuffer);
	if (m_listFrameBuffer.empty())
	{
		return NULL;
	}
	char* pRet = m_listFrameBuffer.front();
	m_listFrameBuffer.pop_front();
	return pRet;
}

void CResManage::ClearAllResource()
{
	map<HWND, list<VideoRender*> > mapRander;
	CSingleZenoLock lock1(m_mtxRander);
	mapRander.swap(m_mapRander);
	lock1.release();
	for (map<HWND, list<VideoRender*> >::iterator iter = mapRander.begin();iter != mapRander.end();++iter)
	{
		list<VideoRender*>& listRander = iter->second;
		for (list<VideoRender*>::iterator iter1 = listRander.begin();iter1 != listRander.end();++iter1)
		{
			delete *iter1;
		}
	}

	list<int> listPort[StreamSourceType_Num];
	CSingleZenoLock lock2(m_mtxPort);
	for (int i = 0;i < StreamSourceType_Num;i++)
	{
		listPort[i].swap(m_listPort[i]);
	}
	lock2.release();
	for (int i = 0;i < StreamSourceType_Num;i++)
	{
		for (list<int>::iterator iter = listPort[i].begin();iter != listPort[i].end();++iter)
		{
			PlaySDK_CloseStream(i, *iter);
			CNBPlayer::UnusePort(i, *iter);
		}
	}

	list<char*> listBuf;
	CSingleZenoLock lock3(m_mtxFrameBuffer);
	listBuf.swap(m_listFrameBuffer);
	lock3.release();
	for (list<char*>::iterator iter = listBuf.begin();iter != listBuf.end();++iter)
	{
		delete *iter;
	}
}