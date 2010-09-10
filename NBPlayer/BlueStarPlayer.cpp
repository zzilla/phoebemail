#include "StdAfx.h"
#include "BlueStarPlayer.h"
#include "NBPlayer.h"
#include "SingleZenoLock.h"
#include "ResManage.h"

CBlueStarPlayer::CBlueStarPlayer(void)
{
	m_bCreate = false;
	m_nStmSrcType = 0;
	m_nPort = 0;
	m_bChangeWnd = true;
	m_bOpenStream = false;
	m_bPausePlay = false;
	m_fRateScale = 1;
	m_pBuf = NULL;
	m_nBufCurLen = 0;
	m_PlayWnd = NULL;
	m_bPlay = FALSE;
	m_thrHandle = NULL;
}

CBlueStarPlayer::~CBlueStarPlayer(void)
{
	
}

bool CBlueStarPlayer::Create(int nStmSrcType, uint32 nBufSize, bool bRealTime)
{
	if ((nStmSrcType < 0) || (nStmSrcType >= StreamSourceType_Num)
		|| (nBufSize == 0) || (nBufSize > 0x1000000))
	{
		return false;
	}

	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		return false;
	}

	int nPort = CNBPlayer::UsePort(nStmSrcType);
	if (nPort == -1)
	{
		return false;
	}

	m_bCreate = true;
	m_nStmSrcType = nStmSrcType;
	m_nPort = nPort;
	m_fRateScale = 1;

	if (m_pBuf == NULL)
	{
		m_pBuf = new char[MAX_BUFF_SIZE];
	}
	memset(m_pBuf,0,MAX_BUFF_SIZE);

	if (m_thrHandle == NULL)
	{
		m_thrHandle = CreateThread(NULL,0,StaticInputDataThread, (LPVOID)this,0,0);
	}
	return true;
}

void CBlueStarPlayer::Close(bool bSaveRes)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		int nPort = m_nPort;
		int nStmSrcType = m_nStmSrcType;

		m_bCreate = false;

		lock.release();

		BSR_PLAYER_StopInput(nPort);
		if(m_thrHandle)
		{
			WaitForSingleObject(m_thrHandle,INFINITE);
			CloseHandle(m_thrHandle);
			m_thrHandle = NULL;
		}
		BSR_PLAYER_Stop(nPort);

		lock.acquire();
		m_nStmSrcType = 0;
		m_nPort = 0;
		m_bChangeWnd = true;
		m_bOpenStream = false;
		m_bPausePlay = false;
		m_fRateScale = 1;
		m_bPlay = FALSE;
		m_vecWnd.clear();
		m_PlayWnd = NULL;
		if (m_pBuf != NULL)
		{
			delete m_pBuf;
			m_pBuf = NULL;
		}
		m_nBufCurLen = 0;
		CNBPlayer::UnusePort(nStmSrcType, nPort);
	}
}

bool CBlueStarPlayer::IsCreated()
{
	return m_bCreate;
}

int CBlueStarPlayer::GetPort()
{
	return m_nPort;
}

int CBlueStarPlayer::GetStmSrcType()
{
	return m_nStmSrcType;
}

bool CBlueStarPlayer::InputData(const char* pData, uint32 nLen)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate && m_PlayWnd)
	{
		if (nLen > HEAD_PACKET_SIZE)
		{
			if (!m_bOpenStream)
			{
				lock.release();
				if (BSR_PLAYER_OpenStream(m_nPort,(BYTE *)pData,HEAD_PACKET_SIZE,PLAYER_DEFDISBUF) == FALSE)
				{
					return false;
				}
				lock.acquire();
				m_bOpenStream = true;
			}

			if (m_bChangeWnd)
			{
				if (m_bPlay)
				{
					lock.release();
					BSR_PLAYER_StopInput(m_nPort);
					BSR_PLAYER_Stop(m_nPort);
					lock.acquire();
					m_bPlay = false;
					lock.release();
					if (BSR_PLAYER_OpenStream(m_nPort,(BYTE *)pData,HEAD_PACKET_SIZE,PLAYER_DEFDISBUF) == FALSE)
					{
						m_bOpenStream = false;
						return false;
					}
					lock.acquire();
				}

				lock.release();
				BSR_PLAYER_RigisterDrawFun(m_nPort, (FUNCPLAYERTITLEDRAW)&CBlueStarPlayer::MyDrawTitleFun, this);
				m_bPlay = BSR_PLAYER_Play(m_nPort,m_PlayWnd);
				lock.acquire();
				m_bChangeWnd = false;
			}

			int nBufCurLen = m_nBufCurLen + nLen - HEAD_PACKET_SIZE;
			if (nBufCurLen > MAX_BUFF_SIZE)
			{
				return false;
			}
			memcpy(m_pBuf + m_nBufCurLen,pData + HEAD_PACKET_SIZE,nBufCurLen - m_nBufCurLen);
			m_nBufCurLen = nBufCurLen;
		}
		return true;
	}
	return false;
}

void CBlueStarPlayer::AddWnd(HWND hWnd)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		if (m_PlayWnd == hWnd)
		{
			return;
		}

		for (vector<HWND>::iterator iter = m_vecWnd.begin();iter != m_vecWnd.end();iter++)
		{
			if ((*iter) == hWnd)
			{
				m_vecWnd.erase(iter);
				break;
			}
		}
		m_vecWnd.push_back(hWnd);
		m_PlayWnd = hWnd;
		m_bChangeWnd = true;
	}
}

void CBlueStarPlayer::DelWnd(HWND hWnd)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		for (vector<HWND>::iterator iter = m_vecWnd.begin();iter != m_vecWnd.end();iter++)
		{
			if ((*iter) == hWnd)
			{
				m_vecWnd.erase(iter);
				break;
			}
		}

		if (m_vecWnd.empty())
		{
			m_PlayWnd = NULL;
			return;
		}

		if (m_PlayWnd == hWnd)
		{
			m_bChangeWnd = true;
			m_PlayWnd = m_vecWnd[m_vecWnd.size() - 1];
		}
	}
}

void CBlueStarPlayer::PausePlay(bool bPause)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bPausePlay == bPause)
	{
		return;
	}
	m_bPausePlay = bPause;
	if (m_bPausePlay == false)
	{
		lock.release();
		SetRateScale();
	}
	else
	{
		lock.release();
		BSR_PLAYER_PlayControl(m_nPort, PLAYER_SPEED_PAUSE);
	}
}

bool CBlueStarPlayer::IsPausePlay()
{
	return m_bPausePlay;
}

void CBlueStarPlayer::SetRateScale(double fRateScale)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		int nRateScale = (int)(fRateScale*DOUBLE_MUL);
		nRateScale = max(1, nRateScale);
		nRateScale = min(nRateScale,256);
		if (nRateScale == m_fRateScale)
		{
			return;
		}
		lock.release();
		m_fRateScale = nRateScale;
		SetRateScale();
	}
}

void CBlueStarPlayer::SetRateScale()
{
	bool b = false;
	if (m_fRateScale >= 1 && m_fRateScale < 2)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_1OF16TIMES);
	}
	else if (m_fRateScale >= 2 && m_fRateScale < 4)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_1OF8TIMES);
	}
	else if (m_fRateScale >= 4 && m_fRateScale < 8)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_1OF4TIMES);
	}	
	else if (m_fRateScale >= 8 && m_fRateScale < 16)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_1OF2TIMES);
	}	
	else if (m_fRateScale >= 16 && m_fRateScale < 32)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_NORMAL);
	}
	else if (m_fRateScale >= 32 && m_fRateScale < 64)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_2TIMES);
	}
	else if (m_fRateScale >= 64 && m_fRateScale < 128)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_4TIMES);
	}
	else if (m_fRateScale >= 128 && m_fRateScale < 256)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_8TIMES);
	}
	else if (m_fRateScale == 256)
	{
		b = BSR_PLAYER_PlayControl(m_nPort,PLAYER_SPEED_16TIMES);
	}

	if (b == false)
	{
		UINT ub = BSR_PLAYER_GetPlayStat(m_nPort);
		int nb = BSR_PLAYER_GetLastError();
		b = true;
	}
}

double CBlueStarPlayer::GetRateScale()
{
	return (double)m_fRateScale/(double)DOUBLE_MUL;
}

void CBlueStarPlayer::PlayOneFrame()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		lock.release();
		BSR_PLAYER_PlayControl(m_nPort, PLAYER_SPEED_ONEBYONE);
	}
}

time_t CBlueStarPlayer::GetPlayingTime()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		lock.release();

		time_t t = BSR_PLAYER_GetOSDTime(m_nPort);
		if (t == 0 || t == -1)
		{
			t = 0;
		}
		else
		{
			t -= 8*3600*1000;
		}
		return t;
	}
	return 0;
}

bool CBlueStarPlayer::RePaintFrame()
{
	return 0;
}

bool CBlueStarPlayer::CapturePicture(char* szFilePath)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		lock.release();
		return BSR_PLAYER_CaptureBMPPicture(m_nPort, szFilePath);
	}
	return false;
}

void CBlueStarPlayer::OpenSound()
{

}

void CBlueStarPlayer::SetColorSetup(int nBrightness, int nContrast, int nSaturation, int nHue)
{

}

void CBlueStarPlayer::GetColorSetup(int* nBrightness, int* nContrast, int* nSaturation, int* nHue)
{

}

int	CBlueStarPlayer::GetBufDataLen()
{
	return 0;
}

void CBlueStarPlayer::ClearBufData()
{

}

bool CBlueStarPlayer::AddToGroup(int nGroupId)
{

	return 0;
}

void CBlueStarPlayer::DelFromGroup()
{

}

void CBlueStarPlayer::GroupPausePlay(bool bPause)
{

}

void CBlueStarPlayer::GroupSetRateScale(double fRateScale)
{

}

void CALLBACK CBlueStarPlayer::MyDrawTitleFun(LONG nPort, HDC hDC, long time, DWORD dwWidth, DWORD dwHeight, LPVOID lpVoid)
{
	time_t t = time;
	tm *frameTime = gmtime(&t);
	if (NULL == frameTime)
	{
		return;
	}

	char szText[256] = {0};
	sprintf(szText,"%04d-%02d-%02d %02d:%02d:%02d", frameTime->tm_year+1900, frameTime->tm_mon+1, frameTime->tm_mday, frameTime->tm_hour, frameTime->tm_min, frameTime->tm_sec);
	int nLen = strlen(szText);

	TEXTMETRIC Metrics;
	ZeroMemory(&Metrics, sizeof(Metrics));
	::GetTextMetrics(hDC, &Metrics);

	::SetBkMode(hDC, TRANSPARENT);

	::SetTextColor(hDC, RGB(255, 255, 255));
	::TextOut(hDC, dwWidth - 20 - nLen * Metrics.tmAveCharWidth, 15, szText, nLen);
}

DWORD WINAPI CBlueStarPlayer::StaticInputDataThread(LPVOID pVoid)
{
	((CBlueStarPlayer*)pVoid)->InputDataThread();
	return 0;
}

void CBlueStarPlayer::InputDataThread()
{
	while (m_bCreate)
	{
		BOOL bSleep = TRUE;

		CSingleZenoLock lock(m_mtxData);
		if (m_bPausePlay == false)
		{
			if (m_nBufCurLen > HEAD_BUFF_SIZE)
			{
				char szBufHead[9] = {0};

				int nItem = 0;
				bool bFind = false;
				while (nItem + 8 <= m_nBufCurLen)
				{
					if (memcmp(m_pBuf + nItem, "bluestar", 8) == 0)
					{
						bFind = true;
						break;
					}
					nItem++;
				}

				if (nItem != 0)
				{
					memmove(m_pBuf, m_pBuf + nItem, m_nBufCurLen - nItem);
					m_nBufCurLen -= nItem;
				}

				if (bFind)
				{
					int nBufLen = *((int *)(m_pBuf + 8));

					int nBufAddHeadLen = HEAD_BUFF_SIZE + nBufLen;
					if (nBufAddHeadLen > MAX_BUFF_SIZE)
					{
						m_nBufCurLen = 0;
					}
					else
					{
						if (nBufAddHeadLen <= m_nBufCurLen)//已经存在完整的一帧数据
						{
							lock.release();

							BSR_PLAYER_InputData(m_nPort,(BYTE *)m_pBuf + HEAD_BUFF_SIZE,nBufAddHeadLen - HEAD_BUFF_SIZE,0);
							bSleep = FALSE;

							lock.acquire();
							memmove(m_pBuf,m_pBuf + nBufAddHeadLen, m_nBufCurLen - nBufAddHeadLen);

							m_nBufCurLen = m_nBufCurLen - nBufAddHeadLen;
						}
					}
				}
			}
		}
		lock.release();

		if (bSleep)
		{
			Sleep(5);
		}
	}
}