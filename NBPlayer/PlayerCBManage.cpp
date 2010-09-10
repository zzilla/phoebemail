#include "StdAfx.h"
#include "PlayerCBManage.h"
#include <assert.h>
#include "SingleZenoLock.h"

CPlayerCBManage CPlayerCBManage::m_staticThis;

CPlayerCBManage::CPlayerCBManage(void)
{
	m_bManageInit = false;
}

CPlayerCBManage::~CPlayerCBManage(void)
{
	for (int i = 0;i < StreamSourceType_Num;i++)
	{
		assert(m_mapPlayer[i].size() == 0);
	}
}

CPlayerCBManage* CPlayerCBManage::Instance()
{
	return &m_staticThis;
}

bool CPlayerCBManage::InitManage()
{
	m_bManageInit = true;
	return true;
}

void CPlayerCBManage::UnInitManage()
{
	m_bManageInit = false;
}

void CALLBACK CPlayerCBManage::StaticDecodeBackFunc_DH(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	m_staticThis.DecodeBackFunc_DH(nPort, pBuf, nSize, pFrameInfo, nReserved1, nReserved2);
}

void CALLBACK CPlayerCBManage::StaticDecodeBackFunc_HK(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	m_staticThis.DecodeBackFunc_HK(nPort, pBuf, nSize, pFrameInfo, nReserved1, nReserved2);
}

void CALLBACK CPlayerCBManage::StaticDecodeBackFunc_DL(long lOpenStream, DWORD dwDataType, char* pBuf, DWORD nWidth, DWORD nHeight, int err, DWORD dwUser)
{
	m_staticThis.DecodeBackFunc_DL(lOpenStream, dwDataType, pBuf, nWidth, nHeight, err, dwUser);
}

void CPlayerCBManage::StaticOnTimerFunc(int nTimerId, int nContext)
{
	m_staticThis.OnTimerFunc(nTimerId, nContext);
}

void CPlayerCBManage::DecodeBackFunc_DH(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	CNBPlayer* pPlayer = GetPlayer(StreamSourceType_Dahua, nPort);
	if (pPlayer)
	{
		pPlayer->OnDecode(pBuf, nSize, pFrameInfo);
		pPlayer->release();
	}
}

void CPlayerCBManage::DecodeBackFunc_HK(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	CNBPlayer* pPlayer = GetPlayer(StreamSourceType_Hik, nPort);
	if (pPlayer)
	{
		pPlayer->OnDecode(pBuf, nSize, pFrameInfo);
		pPlayer->release();
	}
}

void CPlayerCBManage::DecodeBackFunc_DL(long lOpenStream, DWORD dwDataType, char* pBuf, DWORD nWidth, DWORD nHeight, int err, DWORD dwUser)
{
	CNBPlayer* pPlayer = GetDlPlayer(lOpenStream);
	if (pPlayer)
	{
		pPlayer->OnDlDecode(pBuf, nWidth*nHeight*3/2, nWidth, nHeight, dwDataType);
		pPlayer->release();
	}
}

void CPlayerCBManage::OnTimerFunc(int nTimerId, int nContext)
{
	int nPort = nContext & 0xFFFF;
	int nDevType = (nContext >> 16) & 0xFFFF;

	CNBPlayer* pPlayer = GetPlayer(nDevType, nPort);
	if (pPlayer)
	{
		pPlayer->OnTime(nTimerId);
		pPlayer->release();
	}	
}

DecodeBackFuncType CPlayerCBManage::GetDecodeCB(int nDevType)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		return StaticDecodeBackFunc_DH;
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		return StaticDecodeBackFunc_HK;
	}
	return NULL;
}

DLDecodeBackyuvData CPlayerCBManage::GetDlDecodeCB()
{
	return StaticDecodeBackFunc_DL;
}

OnTimerFuncType CPlayerCBManage::GetOnTimerCB(int nDevType)
{
	return StaticOnTimerFunc;
}

CNBPlayer* CPlayerCBManage::GetPlayer(int nDevType, int nPort)
{
	if (m_bManageInit == false)
	{
		return NULL;
	}

	if ((nDevType >= 0) && (nDevType < StreamSourceType_Num))
	{
		CSingleZenoLock lock(m_mtxPlayer);
		map<int, CNBPlayer*>::iterator iter = m_mapPlayer[nDevType].find(nPort);
		if (iter != m_mapPlayer[nDevType].end())
		{
			CNBPlayer* pPlayer = iter->second;
			pPlayer->addRef();
			return pPlayer;
		}
	}
	return NULL;
}

CNBPlayer* CPlayerCBManage::GetDlPlayer(long lOpenStream)
{
	if (m_bManageInit == false)
	{
		return NULL;
	}

	CSingleZenoLock lock(m_mtxPlayer);

	map<long, int>::iterator it = m_mapDlPlayer.find(lOpenStream);
	if (it != m_mapDlPlayer.end())
	{
		lock.release();
		return GetPlayer(StreamSourceType_Dali, it->second);
	}

	return NULL;
}

void CPlayerCBManage::RegisterCallBack(CNBPlayer* pPlayer)
{
	if (m_bManageInit == false)
	{
		return;
	}

	CSingleZenoLock lock(m_mtxPlayer);
	m_mapPlayer[pPlayer->GetStmSrcType()][pPlayer->GetPort()] = pPlayer;
	pPlayer->addRef();
}
	
void CPlayerCBManage::UnRegisterCallBack(CNBPlayer* pPlayer)
{
	CSingleZenoLock lock(m_mtxPlayer);
	m_mapPlayer[pPlayer->GetStmSrcType()].erase(pPlayer->GetPort());
	lock.release();
	pPlayer->release();
}


void CPlayerCBManage::SetDlPlayer(CNBPlayer* pPlayer)
{
	CSingleZenoLock lock(m_mtxPlayer);
	m_mapDlPlayer[pPlayer->GetlOpenStream()] = pPlayer->GetPort();
	lock.release();
}