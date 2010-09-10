#include "StdAfx.h"
#include "GroupManage.h"
#include "SingleZenoLock.h"
#include "NBPlayer.h"
#include "GroupPlayer.h"
#include <assert.h>

CGroupManage CGroupManage::m_staticThis;

CGroupManage::CGroupManage(void)
{
	m_bManageInit = false;
}

CGroupManage::~CGroupManage(void)
{
	UnInitManage();
}

CGroupManage* CGroupManage::Instance()
{
	return &m_staticThis;
}

bool CGroupManage::InitManage()
{
	m_bManageInit = true;
	return true;
}

void CGroupManage::UnInitManage()
{
	m_bManageInit = false;

	CSingleZenoLock lock(m_mtxData);
	for (map<int, CGroupPlayer*>::iterator iter = m_mapGroup.begin();iter != m_mapGroup.end();iter++)
	{
		CGroupPlayer* pGroup = iter->second;
		pGroup->release();
	}
	m_mapGroup.clear();
}

bool CGroupManage::AddPlayerToGroup(CNBPlayer* pPlayer, int nGroupId)
{
	if (nGroupId == 0)
	{
		return false;
	}

	if (m_bManageInit == false)
	{
		return false;
	}

	CSingleZenoLock lock(m_mtxData);
	map<int, CGroupPlayer*>::iterator iter = m_mapGroup.find(nGroupId);
	if (iter == m_mapGroup.end())
	{
		CGroupPlayer* pGroup = new CGroupPlayer(nGroupId);
		pGroup->addRef();
		m_mapGroup[nGroupId] = pGroup;
	}
	CGroupPlayer* pGroup = m_mapGroup[nGroupId];
	pGroup->AddPlayer(pPlayer);

	return true;
}
	
void CGroupManage::DelPlayerFromGroup(CNBPlayer* pPlayer, int nGroupId)
{
	CSingleZenoLock lock(m_mtxData);
	map<int, CGroupPlayer*>::iterator iter = m_mapGroup.find(nGroupId);
	if (iter != m_mapGroup.end())
	{
		CGroupPlayer* pGroup = iter->second;
		if (pGroup->DelPlayer(pPlayer) == 0)
		{
			m_mapGroup.erase(iter);
			pGroup->release();
		}
	}
}
	
CGroupPlayer* CGroupManage::GetGroupPlayer(int nGroupId)
{
	if (m_bManageInit == false)
	{
		return NULL;
	}

	CSingleZenoLock lock(m_mtxData);
	map<int, CGroupPlayer*>::iterator iter = m_mapGroup.find(nGroupId);
	if (iter != m_mapGroup.end())
	{
		CGroupPlayer* pGroup = iter->second;
		pGroup->addRef();
		return pGroup;
	}
	return NULL;
}

void CGroupManage::StaticOnTimerFunc(int nTimerId, int nContext)
{
	m_staticThis.OnTimerFunc(nTimerId, nContext);
}

void CGroupManage::OnTimerFunc(int nTimerId, int nContext)
{
	CGroupPlayer* pGroup = GetGroupPlayer(nContext);
	if (pGroup)
	{
		pGroup->OnTime(nTimerId);
		pGroup->release();
	}
}

OnTimerFuncType CGroupManage::GetOnTimerCB()
{
	return StaticOnTimerFunc;
}
