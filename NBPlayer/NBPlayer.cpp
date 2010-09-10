#include "stdafx.h"
#include "NBPlayer.h"
#include "ResManage.h"
#include "TimerManage.h"
#include "PlayerCBManage.h"
#include "SingleZenoLock.h"
#include "ddoffscreenrender.h"
#include "GroupManage.h"
#include "GroupPlayer.h"
#include <assert.h>
#include "AudioThread.h"
#include "BlueStarPlayer.h"
#include "HikDecoderInterface.h"


void myInvalidParameterHandler(const wchar_t* expression,
							   const wchar_t* function, 
							   const wchar_t* file, 
							   unsigned int line, 
							   uintptr_t pReserved)
{
} 


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}

bool g_bDllInit = false;
NBPLAYER_API bool InitNBPlayerDll()
{
	if (g_bDllInit)
	{
		return true;
	}
	DDOffscreenRender::UpdateMonitorEnum();
	CAudioThreadSingleton::instance();
	CResManage::Instance()->InitManage();
	CTimerManage::Instance()->InitManage();
	CPlayerCBManage::Instance()->InitManage();
	CGroupManage::Instance()->InitManage();

	g_bDllInit = true;

	_set_invalid_parameter_handler(myInvalidParameterHandler);

	return TRUE;
}

NBPLAYER_API void UninitNBPlayerDll()
{
	if (g_bDllInit)
	{
		g_bDllInit = false;

		CTimerManage::Instance()->UnInitManage();
		CGroupManage::Instance()->UnInitManage();
		CPlayerCBManage::Instance()->UnInitManage();
		CResManage::Instance()->UnInitManage();
		CAudioThreadSingleton::unInstance();
	}
}

bool IsInitDll()
{
	return g_bDllInit;
}

NBPLAYER_API INBPlayer* NewNBPlayer()
{
	if (g_bDllInit)
	{
		CNBPlayer* pPlayer = new CNBPlayer;
		pPlayer->addRef();
		return pPlayer;
	}
	return NULL;
}

NBPLAYER_API void DeleteNBPlayer(INBPlayer* pPlayer)
{
	if (g_bDllInit)
	{
		if (pPlayer)
		{
			pPlayer->Close(true);
			((CNBPlayer*)pPlayer)->release();
		}
	}
}

void StaticNBPlayer::CloseSound()
{
	if (g_bDllInit)
	{
		CAudioThreadSingleton::instance()->closeSound();
	}
}

int StaticNBPlayer::GetVolume()
{
	if (g_bDllInit)
	{
		return CAudioThreadSingleton::instance()->getVolume();
	}
	return 0;
}

void StaticNBPlayer::SetVolume(int nVolume)
{
	if (g_bDllInit)
	{
		CAudioThreadSingleton::instance()->setVolume(nVolume);
	}
};

void StaticNBPlayer::ClearNBPlayerResource()
{
	if (g_bDllInit)
	{
		CResManage::Instance()->ClearAllResource();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

CNBPlayer::CNBPlayer()
{
	m_bCreate = false;
	m_nStmSrcType = 0;
	m_nSDKBufSize = 0;
	m_bRealtimePlay = true;
	m_bOpenPort = false;
	m_nPort = 0;
	m_nPaintTimerId = 0;

	m_bPausePlay = false;
	m_fRateScale = DOUBLE_DIV;
	m_nTimerPeriod = 0;

	m_bSetDecodeThreadPriority = false;
	m_nLastAjustPeriodTime = 0;

	m_pFrames = NULL;
	m_nFrameNum = 0;
	m_nFrmWrite = 0;
	m_nFrmRead = 0;

	m_nGroupId = 0;
	InitExtendData();
	m_lOpenStream = 0;
	m_nAudioSample = 0;
	m_nFrameRate = 0;
}

INBPlayer::~INBPlayer()
{

}

CNBPlayer::~CNBPlayer()
{
	//
}

bool CNBPlayer::Create(int nStmSrcType, uint32 nBufSize, bool bRealTime)
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

	if (nStmSrcType == StreamSourceType_BlueStar)
	{
		lock.release();
		this->~CNBPlayer();
		((CBlueStarPlayer*)this)->CBlueStarPlayer::CBlueStarPlayer();
		return Create(nStmSrcType, nBufSize, bRealTime);
	}

	InitExtendData();

	char* pFrameBuf = CResManage::Instance()->LoadFrameBuffer();
	if (pFrameBuf == NULL)
	{
		pFrameBuf = new char[sizeof(FrameInfo) * MAX_BUFFER_FRAMES];
		if (pFrameBuf == NULL)
		{
			return false;
		}
	}
	memset(pFrameBuf, 0, sizeof(FrameInfo) * MAX_BUFFER_FRAMES);

	bool bOpenPort = false;
	int nPort = CResManage::Instance()->LoadPort(nStmSrcType, nBufSize);
	if (nPort != -1)
	{
		bOpenPort = true;
	}
	else
	{
		nPort = UsePort(nStmSrcType);
		if (nPort == -1)
		{
			delete pFrameBuf;
			return false;
		}
	}

	m_bCreate = true;
	m_nStmSrcType = nStmSrcType;
	m_nSDKBufSize = nBufSize;
	m_bRealtimePlay = bRealTime;
	m_bOpenPort = bOpenPort;
	m_nPort = nPort;
	m_nPaintTimerId = CTimerManage::Instance()->SetTimer(40
														, (m_nStmSrcType << 16) | m_nPort
														, CPlayerCBManage::Instance()->GetOnTimerCB(m_nStmSrcType));

	if (m_nStmSrcType == StreamSourceType_Dali)
	{
		PlaySDK_DL_Init();
	}
	m_bPausePlay = false;
	m_fRateScale = DOUBLE_DIV;

	m_bSetDecodeThreadPriority = false;
	m_nLastAjustPeriodTime = 0;

	m_pFrames = (FrameInfo*)pFrameBuf;
	m_nFrameNum = 0;
	m_nFrmWrite = 0;
	m_nFrmRead = 0;

	m_nGroupId = 0;

	if (m_bOpenPort)
	{
		EfectExtendData(0);
	}

	CPlayerCBManage::Instance()->RegisterCallBack(this);

	return true;
}
	
void CNBPlayer::Close(bool bSaveRes)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		CPlayerCBManage::Instance()->UnRegisterCallBack(this);

		if (GetSoundId() == CAudioThreadSingleton::instance()->getId())
		{
			CAudioThreadSingleton::instance()->closeSound();
		}

		//保存数据，以便后面释放
		bool bOpenPort = m_bOpenPort;
		int nStmSrcType = m_nStmSrcType;
		int nPort = m_nPort;
		uint32 nBufSize = m_nSDKBufSize;
		long lOpenStream = m_lOpenStream;

		CTimerManage::Instance()->KillTimer(m_nPaintTimerId);

		for (int i = 0;i < m_vecWnd.size();i++)
		{
			WndInfo& infoWnd = m_vecWnd[i];
			if (bSaveRes)
			{
				CResManage::Instance()->SaveRander(infoWnd.hWnd, infoWnd.pRander);
			}
			else
			{
				delete infoWnd.pRander;
			}
		}
		for (list<HWND>::iterator iter = m_listUnuseWnd.begin();iter != m_listUnuseWnd.end();++iter)
		{
			HWND hWnd = *iter;
			if (bSaveRes)
			{
				//不需要做什么
			}
			else
			{
				VideoRender* pRander = CResManage::Instance()->LoadRander(hWnd);
				if (pRander)
				{
					delete pRander;
				}
			}
		}

		if (bSaveRes)
		{
			CResManage::Instance()->SaveFrameBuffer((char*)m_pFrames);
		}
		else
		{
			delete (char*)m_pFrames;
		}

		m_bCreate = false;
		m_nStmSrcType = 0;
		m_nSDKBufSize = 0;
		m_bRealtimePlay = true;
		m_bOpenPort = false;
		m_nPort = 0;
		m_nPaintTimerId = 0;

		m_bPausePlay = false;
		m_fRateScale = DOUBLE_DIV;
		m_nTimerPeriod = 0;

		m_bSetDecodeThreadPriority = false;
		m_nLastAjustPeriodTime = 0;

		m_vecWnd.clear();
		m_listUnuseWnd.clear();

		m_pFrames = NULL;
		m_nFrameNum = 0;
		m_nFrmWrite = 0;
		m_nFrmRead = 0;

		int nGroupId = m_nGroupId;
		m_nGroupId = 0;

		InitExtendData();

		m_lOpenStream = 0;

		lock.release();

		//锁后释放
		if (bOpenPort)
		{
			if (nStmSrcType == StreamSourceType_Dali)
			{
				PlaySDK_DL_CloseStream(lOpenStream);
				PlaySDK_DL_Cleanup();
				UnusePort(nStmSrcType, nPort);
			}
			else
			{
				if (bSaveRes)
				{
					CResManage::Instance()->SavePort(nStmSrcType, nPort, nBufSize);
				}
				else
				{
					PlaySDK_CloseStream(nStmSrcType, nPort);
					UnusePort(nStmSrcType, nPort);
				}
			}
		}
		else
		{
			UnusePort(nStmSrcType, nPort);
		}

		if (nGroupId != 0)
		{
			CGroupManage::Instance()->DelPlayerFromGroup(this, nGroupId);
		}
	}
}

bool CNBPlayer::IsCreated()
{
	return m_bCreate;
}

int CNBPlayer::GetPort()
{
	return m_nPort;
}

int CNBPlayer::GetStmSrcType()
{
	return m_nStmSrcType;
}

long CNBPlayer::GetlOpenStream()
{
	return m_lOpenStream;
}

bool CNBPlayer::InputData(const char* pData, uint32 nLen)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		if (m_bOpenPort == false)
		{
			if (m_nStmSrcType == StreamSourceType_Dali)
			{
				if (nLen >= 256)
				{
					if (PlaySDK_DL_OpenStream(m_lOpenStream, pData, 256
						, CPlayerCBManage::Instance()->GetDlDecodeCB(), m_nAudioSample, m_nFrameRate))
					{
						EfectExtendData(0);
						m_bOpenPort = true;

						CPlayerCBManage::Instance()->SetDlPlayer(this);
					}
				}
			}
			else
			{
				int nHeadLen = 0;
				if (m_nStmSrcType == StreamSourceType_Hik)
				{
					nHeadLen = 40;
				}
				if (nLen >= nHeadLen)
				{
					if (PlaySDK_OpenStream(m_nStmSrcType, m_nPort, pData, nHeadLen, m_nSDKBufSize
						, CPlayerCBManage::Instance()->GetDecodeCB(m_nStmSrcType)))
					{
						EfectExtendData(0);
						m_bOpenPort = true;
					}
				}
			}
		}
		if (m_bOpenPort)
		{
			int nStmSrcType = m_nStmSrcType;
			int nPort = m_nPort;
			lock.release();

			if (m_nStmSrcType == StreamSourceType_Dali)
			{
				return PlaySDK_DL_InputData(m_lOpenStream, pData, nLen);
			}
			else
			{
				return PlaySDK_InputData(nStmSrcType, nPort, pData, nLen);
			}
		}
	}
	return false;
}

void CNBPlayer::AddWnd(HWND hWnd)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		for (int i = 0;i < m_vecWnd.size();i++)
		{
			WndInfo& infoWnd = m_vecWnd[i];
			if (infoWnd.hWnd == hWnd)
			{
				return;
			}
		}
		for (list<HWND>::iterator iter = m_listUnuseWnd.begin();iter != m_listUnuseWnd.end();++iter)
		{
			if ((*iter) == hWnd)
			{
				m_listUnuseWnd.erase(iter);
				break;
			}
		}
		WndInfo infoWnd;
		infoWnd.hWnd = hWnd;
		infoWnd.pRander = CResManage::Instance()->LoadRander(hWnd);
		if (infoWnd.pRander == NULL)
		{
			infoWnd.pRander = new DDOffscreenRender;
			infoWnd.pRander->init(0, hWnd, 0, 0, NULL);
		}
		m_vecWnd.push_back(infoWnd);
	}
}
	
void CNBPlayer::DelWnd(HWND hWnd)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		for (int i = 0;i < m_vecWnd.size();i++)
		{
			WndInfo& infoWnd = m_vecWnd[i];
			if (infoWnd.hWnd == hWnd)
			{
				CResManage::Instance()->SaveRander(infoWnd.hWnd, infoWnd.pRander);
				m_listUnuseWnd.push_front(infoWnd.hWnd);
				m_vecWnd.erase(m_vecWnd.begin() + i);
				break;
			}
		}
	}
}

void CNBPlayer::PausePlay(bool bPause)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		if (m_bPausePlay == bPause)
		{
			return;
		}

		m_bPausePlay = bPause;
		CTimerManage::Instance()->DelayTimer(m_nPaintTimerId, m_bPausePlay ? DELAY_PAUSE_TIMER : 0);

		if (m_bPausePlay && (GetSoundId() == CAudioThreadSingleton::instance()->getId()))
		{
			CAudioThreadSingleton::instance()->clearBuf();
		}
	}
}

bool CNBPlayer::IsPausePlay()
{
	return m_bPausePlay;
}

void CNBPlayer::SetRateScale(double fRateScale)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		int nRateScale = (int)(fRateScale * DOUBLE_DIV + 0.1);
		nRateScale = max(1, nRateScale);
		if (nRateScale == m_fRateScale)
		{
			return;
		}

		m_fRateScale = nRateScale;

		//调整计时器
		int nPeriod = CalePaintTimerPeriod();
		SetPaintTimerPeriod(nPeriod);
		if (m_bPausePlay)
		{
			SetPaintDelay(DELAY_PAUSE_TIMER);
		}

		if ((m_fRateScale != DOUBLE_DIV) && (GetSoundId() == CAudioThreadSingleton::instance()->getId()))
		{
			CAudioThreadSingleton::instance()->clearBuf();
		}
	}
}

double CNBPlayer::GetRateScale()
{
	return (double)m_fRateScale / DOUBLE_DIV;
}

void CNBPlayer::PlayOneFrame()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		FrameInfo* pFrame = ReadFrame();
		if (pFrame)
		{
			PaintFrame(pFrame);
		}
	}
}

time_t CNBPlayer::GetPlayingTime()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		FrameInfo* pLastRead = GetLastReadFrame();
		if (pLastRead)
		{
			return pLastRead->tTime;
		}
	}
	return 0;
}

bool CNBPlayer::RePaintFrame()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		FrameInfo* pLastRead = GetLastReadFrame();
		if (pLastRead)
		{
			PaintFrame(pLastRead);
			return true;
		}
	}
	return false;
}

bool CNBPlayer::CapturePicture(char* szFilePath)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		FrameInfo* pLastRead = GetLastReadFrame();
		if (pLastRead)
		{
			if (m_nStmSrcType == StreamSourceType_Dali)
			{
				return PlaySDK_DL_CapturePicture(m_lOpenStream, szFilePath);
			}
			else
			{
				return PlaySDK_CapturePicture(m_nStmSrcType, pLastRead->cData, pLastRead->nSize
					, pLastRead->nWidth, pLastRead->nHeight, pLastRead->nType, szFilePath);
			}
		}
	}
	return false;
}

void CNBPlayer::OpenSound()
{
	if (m_bCreate)
	{
		CAudioThreadSingleton::instance()->openSound(GetSoundId());
	}
}

void CNBPlayer::SetColorSetup(int nBrightness, int nContrast, int nSaturation, int nHue)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		m_nBrightness = nBrightness;
		m_nContrast = nContrast;
		m_nSaturation = nSaturation;
		m_nHue = nHue;
		if (m_bOpenPort)
		{
			EfectExtendData(1);
		}
	}
}

void CNBPlayer::GetColorSetup(int* nBrightness, int* nContrast, int* nSaturation, int* nHue)
{
	*nBrightness = m_nBrightness;
	*nContrast = m_nContrast;
	*nSaturation = m_nSaturation;
	*nHue = m_nHue;
}

int CNBPlayer::GetBufDataLen()
{
	if (m_bCreate)
	{
		int nLen = PlaySDK_GetBufDataLen(m_nStmSrcType, m_nPort);
		if (nLen == 0)
		{
			nLen = m_nFrameNum;
		}
		return nLen;
	}
	return 0;
}

void CNBPlayer::ClearBufData()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		int nStmSrcType = m_nStmSrcType;
		int nPort = m_nPort;
		long lOpenStream = m_lOpenStream;
		memset(m_pFrames, 0, MAX_BUFFER_FRAMES * sizeof(FrameInfo));
		m_nFrameNum = m_nFrmRead = m_nFrmWrite = 0;
		lock.release();

		if (nStmSrcType == StreamSourceType_Dali)
		{
			PlaySDK_DL_ClearBufData(lOpenStream);
		}
		else
		{
			PlaySDK_ClearBufData(nStmSrcType, nPort);
		}
		
		if (GetSoundId() == CAudioThreadSingleton::instance()->getId())
		{
			CAudioThreadSingleton::instance()->clearBuf();
		}
	}
}

bool CNBPlayer::AddToGroup(int nGroupId)
{
	if (nGroupId == 0)
	{
		return false;
	}

	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		if (m_nGroupId == nGroupId)
		{
			return true;
		}

		int nDelFromGroup = m_nGroupId;
		m_nGroupId = nGroupId;
		int nAddToGroup = m_nGroupId;

		lock.release();

		if (nDelFromGroup != 0)
		{
			CGroupManage::Instance()->DelPlayerFromGroup(this, nDelFromGroup);
		}
		CGroupManage::Instance()->AddPlayerToGroup(this, nAddToGroup);

		return true;
	}

	return false;
}

void CNBPlayer::DelFromGroup()
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate)
	{
		int nDelFromGroup = m_nGroupId;
		m_nGroupId = 0;

		lock.release();

		if (nDelFromGroup != 0)
		{
			CGroupManage::Instance()->DelPlayerFromGroup(this, nDelFromGroup);
		}
	}
}

void CNBPlayer::GroupPausePlay(bool bPause)
{
	CGroupPlayer* pGroup = CGroupManage::Instance()->GetGroupPlayer(m_nGroupId);
	if (pGroup)
	{
		pGroup->PausePlay(bPause);
		pGroup->release();
	}
}

void CNBPlayer::GroupSetRateScale(double fRateScale)
{
	CGroupPlayer* pGroup = CGroupManage::Instance()->GetGroupPlayer(m_nGroupId);
	if (pGroup)
	{
		fRateScale = max(0.01, min(100, fRateScale));
		pGroup->SetRateScale(fRateScale);
		pGroup->release();
	}
}


AX_Mutex& CNBPlayer::GetDataLock()
{
	return m_mtxData;
}

uint32 CNBPlayer::CalePaintTimerPeriod()
{
	FrameInfo* pLastRead = GetLastReadFrame();
	return CalePaintTimerPeriod(m_fRateScale, (pLastRead == NULL) ? 0 : pLastRead->nRate);
}

uint32 CNBPlayer::CalePaintTimerPeriod(int fRateScale, int nRate)
{
	if (nRate <= 0)
	{
		return 40;
	}
	else
	{
		uint32 nPeriod = (uint32)((1000 * DOUBLE_DIV) / (nRate * fRateScale));
		return max(1, min(nPeriod, 1000));
	}
}

void CNBPlayer::SetPaintTimerPeriod(int nPeriod)
{
	if (nPeriod != m_nTimerPeriod)
	{
		m_nTimerPeriod = nPeriod;
		CTimerManage::Instance()->ModifyTimerPeriod(m_nPaintTimerId, m_nTimerPeriod);
	}
}

void CNBPlayer::SetPaintDelay(int nDelay)
{
	CTimerManage::Instance()->DelayTimer(m_nPaintTimerId, nDelay);
}

void CNBPlayer::TinyAjustTimerPeriod(int nBufSize)
{
	if (m_bRealtimePlay 
		&& (m_nGroupId == 0)
		&& (m_fRateScale == DOUBLE_DIV))
	{
		//只有这种情况才支持微调
		int nPeriod = CalePaintTimerPeriod();
		int nOldPeriod = nPeriod;

		if (((m_nFrameNum * nOldPeriod) >= 2100) && (m_nFrameNum > 2))
		{
			nPeriod -= nPeriod / 5;
		}
		else
		{
			if (nBufSize > 100 * 1024)
			{
				nPeriod -= nPeriod / 5;
			}
			else if (nBufSize > 50 * 1024)
			{
				nPeriod -= nPeriod / 10;
			}
			else if (nBufSize > 30 * 1024)
			{
				nPeriod -= nPeriod / 15;
			}
			else if (nBufSize > 10 * 1024)
			{
				nPeriod -= nPeriod / 20;
			}
		}

		if (nOldPeriod != nPeriod)
		{
			uint32 nTime = ::GetTickCount();
			if ((nTime - m_nLastAjustPeriodTime) < 1000)
			{
				//防止过度循环，1秒限制
				return;
			}
			m_nLastAjustPeriodTime = nTime;
		}

		SetPaintTimerPeriod(nPeriod);
	}
}

void CNBPlayer::PaintFrame(FrameInfo* pFrameInfo)
{
	char* py,* pu,* pv;
	py = pFrameInfo->cData;
	if (m_nStmSrcType == StreamSourceType_Dahua)
	{
		pu = pFrameInfo->cData + pFrameInfo->nWidth * pFrameInfo->nHeight;
		pv = pFrameInfo->cData + pFrameInfo->nWidth * pFrameInfo->nHeight * 5/4;
	}
	else
	{
		pv = pFrameInfo->cData + pFrameInfo->nWidth * pFrameInfo->nHeight;
		pu = pFrameInfo->cData + pFrameInfo->nWidth * pFrameInfo->nHeight * 5/4;
	}

	for (int i = 0;i < m_vecWnd.size();i++)
	{
		WndInfo& infoWnd = m_vecWnd[i];
		infoWnd.pRander->render((unsigned char*)py, (unsigned char*)pu, (unsigned char*)pv, 
				pFrameInfo->nWidth, pFrameInfo->nHeight, NULL);
	}
}

CNBPlayer::FrameInfo* CNBPlayer::ReadFrame()
{
	if (m_nFrameNum > 0)
	{
		FrameInfo* pRet = &m_pFrames[m_nFrmRead];

		m_nFrameNum--;
		m_nFrmRead++;
		if (m_nFrmRead == MAX_BUFFER_FRAMES)
		{
			m_nFrmRead = 0;
		}

		return pRet;
	}
	return NULL;
}

CNBPlayer::FrameInfo* CNBPlayer::WriteFrame()
{
	if (m_nFrameNum < (MAX_BUFFER_FRAMES - 1))
	{
		FrameInfo* pRet = &m_pFrames[m_nFrmWrite];

		m_nFrmWrite++;
		m_nFrameNum++;
		if (m_nFrmWrite == MAX_BUFFER_FRAMES)
		{
			m_nFrmWrite = 0;
		}

		return pRet;
	}
	return NULL;
}

CNBPlayer::FrameInfo* CNBPlayer::GetLastReadFrame()
{
	int nLastRead = m_nFrmRead;
	nLastRead--;
	if (nLastRead < 0)
	{
		nLastRead = MAX_BUFFER_FRAMES - 1;
	}
	return &m_pFrames[nLastRead];
}

CNBPlayer::FrameInfo* CNBPlayer::GetNextReadFrame()
{
	if (m_nFrameNum > 0)
	{
		return &m_pFrames[m_nFrmRead];
	}
	return NULL;
}

CNBPlayer::FrameInfo* CNBPlayer::GetLastWriteFrame()
{
	int nLastWrite = m_nFrmWrite;
	nLastWrite--;
	if (nLastWrite < 0)
	{
		nLastWrite = MAX_BUFFER_FRAMES - 1;
	}
	return &m_pFrames[nLastWrite];
}

int CNBPlayer::GetSoundId()
{
	return (m_nStmSrcType << 16) + m_nPort;
}

void CNBPlayer::OnTime(int nTimerId)
{

	CSingleZenoLock lock(m_mtxData);
	if (m_bCreate && (m_bPausePlay == false))
	{
		uint32 nCurTime = (uint32)AX_OS::get_time();

		int nOldPeriod = CalePaintTimerPeriod();
		FrameInfo* pPaint = ReadFrame();
		if (pPaint)
		{
			pPaint->nPaintTime = nCurTime;
			PaintFrame(pPaint);

			int nPeriod = CalePaintTimerPeriod();
			if (nPeriod != nOldPeriod)
			{
				//调整计时器
				SetPaintTimerPeriod(nPeriod);
				if (m_bPausePlay)
				{
					SetPaintDelay(DELAY_PAUSE_TIMER);
				}
			}
		}
		else
		{
			//没帧数据
			int nBreak = 0;
		}
	}
}

void CNBPlayer::OnDecode(char* pBuf, long nSize, FRAME_INFO* pFrameInfo)
{
	if (((pFrameInfo->nHeight == 0) && (pFrameInfo->nWidth == 0)) || (pFrameInfo->nType == T_AUDIO8 || pFrameInfo->nType == T_AUDIO16))
	{
		//音频
		if (m_bCreate && (m_fRateScale == DOUBLE_DIV) && (m_bPausePlay == false))
		{
			CAudioThreadSingleton::instance()->inputData(GetSoundId()
														, pBuf, nSize
														, (pFrameInfo->nFrameRate == 0) ? 16000 : pFrameInfo->nFrameRate
														, (pFrameInfo->nType == T_AUDIO8) ? 8 : 16);
		}
	}
	else
	{
		if (nSize <= MAX_FRAME_SIZE)
		{
			time_t tPlayingTimeEx = PlaySDK_GetPlayingTimeEx(m_nStmSrcType, m_nPort);
			time_t tPlayingTime = PlaySDK_GetPlayingTime(m_nStmSrcType, m_nPort);

			while (true)
			{
				uint32 nNowBufDataSize = PlaySDK_GetBufDataLen(m_nStmSrcType, m_nPort);

				CSingleZenoLock lock(m_mtxData);
				if (m_bCreate == false)
				{
					break;
				}

				if ((m_bSetDecodeThreadPriority == false) && (m_nStmSrcType == StreamSourceType_Hik))
				{
					m_bSetDecodeThreadPriority = true;
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
				}

				FrameInfo* pLastWrite = GetLastWriteFrame();
				FrameInfo* pWrite = WriteFrame();
				if (pWrite)
				{
					memcpy(pWrite->cData, pBuf, nSize);
					pWrite->nWidth = pFrameInfo->nWidth;
					pWrite->nHeight = pFrameInfo->nHeight;
					pWrite->nRate = max(1, min(pFrameInfo->nFrameRate, 1000));
					pWrite->tSynchTime = tPlayingTimeEx;
					pWrite->tTime = tPlayingTime;
					pWrite->nSize = nSize;
					pWrite->nType = pFrameInfo->nType;
					pWrite->nPaintTime = 0;

					//做毫秒级估计
					if (pLastWrite && (pLastWrite->tTime == pWrite->tTime))
					{
						pWrite->nMilSec = pLastWrite->nMilSec + CalePaintTimerPeriod(DOUBLE_DIV, pWrite->nRate);
						pWrite->nMilSec = min(1000, pWrite->nMilSec);
					}
					else
					{
						pWrite->nMilSec = 0;
					}

					TinyAjustTimerPeriod(nNowBufDataSize);

					break;
				}
				else
				{
					int nPeriod = CalePaintTimerPeriod();
					TinyAjustTimerPeriod(nNowBufDataSize);
					lock.release();

					nPeriod = max(1, min(nPeriod, 40));
					AX_OS::sleep(nPeriod);
				}
			}
		}
	}
}

void CNBPlayer::OnDlDecode(char* pBuf, long nSize, int nWidth, int nHeight, DWORD dwDataType)
{
	if (dwDataType == 0x2)
	{
		//音频
		if (m_bCreate && (m_fRateScale == DOUBLE_DIV) && (m_bPausePlay == false))
		{
			CAudioThreadSingleton::instance()->inputData(GetSoundId()
				, pBuf, nSize
				, (m_nAudioSample == 0) ? 8000 : 16000
				, 16);
		}
	}
	else
	{
		if (nSize <= MAX_FRAME_SIZE)
		{
			time_t tPlayingTimeEx = PlaySDK_DL_GetPlayingTimeEx(m_lOpenStream);
			time_t tPlayingTime = PlaySDK_DL_GetPlayingTime(m_lOpenStream);

			while (true)
			{
				uint32 nNowBufDataSize = PlaySDK_GetBufDataLen(m_nStmSrcType, m_nPort);

				CSingleZenoLock lock(m_mtxData);
				if (m_bCreate == false)
				{
					break;
				}

				FrameInfo* pLastWrite = GetLastWriteFrame();
				FrameInfo* pWrite = WriteFrame();
				if (pWrite)
				{
					memcpy(pWrite->cData, pBuf, nSize);
					pWrite->nWidth = nWidth;
					pWrite->nHeight = nHeight;
					pWrite->nRate = 25;//max(1, min(m_nFrameRate, 1000));
					pWrite->tSynchTime = tPlayingTimeEx;
					pWrite->tTime = tPlayingTime;
					pWrite->nSize = nSize;
					pWrite->nType = 0;
					pWrite->nPaintTime = 0;

					//做毫秒级估计
					if (pLastWrite && (pLastWrite->tTime == pWrite->tTime))
					{
						pWrite->nMilSec = pLastWrite->nMilSec + CalePaintTimerPeriod(DOUBLE_DIV, pWrite->nRate);
						pWrite->nMilSec = min(1000, pWrite->nMilSec);
					}
					else
					{
						pWrite->nMilSec = 0;
					}

					TinyAjustTimerPeriod(nNowBufDataSize);

					break;
				}
				else
				{
					int nPeriod = CalePaintTimerPeriod();
					TinyAjustTimerPeriod(nNowBufDataSize);
					lock.release();

					nPeriod = max(1, min(nPeriod, 40));
					AX_OS::sleep(nPeriod);
				}
			}
		}
	}
}

void CNBPlayer::InitExtendData()
{
	m_nBrightness = 64;
	m_nContrast = 64;
	m_nSaturation = 64;
	m_nHue = 64;
}

void CNBPlayer::EfectExtendData(int nDataType)
{
	if ((nDataType == 0) || (nDataType == 1))
	{
		if (m_nStmSrcType == StreamSourceType_Dali)
		{
			PlaySDK_DL_SetColor(m_lOpenStream, m_nBrightness, m_nContrast, m_nSaturation, m_nHue);
		}
		else
		{
			PlaySDK_SetColor(m_nStmSrcType, m_nPort, m_nBrightness, m_nContrast, m_nSaturation, m_nHue);
		}
	}
}

AX_Mutex CNBPlayer::m_mtxUnusePort;
bool CNBPlayer::m_bInitUnusePortList = false;
list<int> CNBPlayer::m_listUnusePort[StreamSourceType_Num];

void CNBPlayer::InitUnusePort()
{
	CSingleZenoLock lock(m_mtxUnusePort);
	if (m_bInitUnusePortList == false)
	{
		m_bInitUnusePortList = true;

		for (int i = 0;i < StreamSourceType_Num;i++)
		{
			for (int j = 1;j < MAX_PORT_NUM;j++)
			{
				m_listUnusePort[i].push_back(j);
			}
		}
	}
}

int CNBPlayer::UsePort(int nDevType)
{
	InitUnusePort();

	CSingleZenoLock lock(m_mtxUnusePort);
	if (m_listUnusePort[nDevType].empty())
	{
		return -1;
	}
	int nRet = m_listUnusePort[nDevType].front();
	m_listUnusePort[nDevType].pop_front();
	return nRet;
}
	
void CNBPlayer::UnusePort(int nDevType, int nPort)
{
	CSingleZenoLock lock(m_mtxUnusePort);
	m_listUnusePort[nDevType].push_front(nPort);
}