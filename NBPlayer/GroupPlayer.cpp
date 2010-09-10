#include "StdAfx.h"
#include "GroupPlayer.h"
#include "NBPlayer.h"
#include "SingleZenoLock.h"
#include "TimerManage.h"
#include "GroupManage.h"

CGroupPlayer::CGroupPlayer(int nGroupId)
{
	m_nGroupId = nGroupId;
	m_nWatchTimerId = CTimerManage::Instance()->SetTimer(40, m_nGroupId, CGroupManage::Instance()->GetOnTimerCB());
	m_bPausePlay = false;
	m_fRateScale = 1;

	m_nLastOnTimeTime = 0;
	m_nLastOnTimeVideoTime = 0;
}

CGroupPlayer::~CGroupPlayer(void)
{
	CTimerManage::Instance()->KillTimer(m_nWatchTimerId);
}

void CGroupPlayer::InitPlayerInfoData(PlayerInfo* pInfo)
{
	pInfo->nSetDelay = 0;
	pInfo->nLastSynchTime = 0;
	pInfo->nLastSynchNextFramePaintTime = 0;
	pInfo->bSynchLimit = false;
	pInfo->nNextFrameTime = 0;
	pInfo->nNextPaintTime = 0;
}

int CGroupPlayer::AddPlayer(CNBPlayer* pPlayer)
{
	CSingleZenoLock lock(m_mtxData);
	for (int i = 0;i < m_vecPlayer.size();i++)
	{
		if (m_vecPlayer[i].pPlayer == pPlayer)
		{
			return m_vecPlayer.size();
		}
	}
	PlayerInfo info;
	info.pPlayer = pPlayer;
	InitPlayerInfoData(&info);
	m_vecPlayer.push_back(info);
	return m_vecPlayer.size();
}
	
int CGroupPlayer::DelPlayer(CNBPlayer* pPlayer)
{
	CSingleZenoLock lock(m_mtxData);
	for (int i = 0;i < m_vecPlayer.size();i++)
	{
		PlayerInfo& info = m_vecPlayer[i];
		if (info.pPlayer == pPlayer)
		{
			bool bSetDelay = info.nSetDelay != 0;
			m_vecPlayer.erase(m_vecPlayer.begin() + i);
			int nRet = m_vecPlayer.size();
			lock.release();

			if (bSetDelay)
			{
				pPlayer->SetPaintDelay(0);
			}

			return nRet;
		}
	}
	return 0;
}

void CGroupPlayer::PausePlay(bool bPause)
{
	CSingleZenoLock lock(m_mtxData);
	if (m_bPausePlay == bPause)
	{
		return;
	}

	m_bPausePlay = bPause;
	for (int i = 0;i < m_vecPlayer.size();i++)
	{
		PlayerInfo& info = m_vecPlayer[i];
		info.pPlayer->PausePlay(bPause);
		InitPlayerInfoData(&info);
	}
}
	
void CGroupPlayer::SetRateScale(double fRateScale)
{
	CSingleZenoLock lock(m_mtxData);

	fRateScale = max(0.01, min(100, fRateScale));
	double fDiffScale = m_fRateScale - fRateScale;
	if ((fDiffScale < 0.0001) && (fDiffScale > -0.0001))
	{
		return;
	}

	m_fRateScale = fRateScale;
	for (int i = 0;i < m_vecPlayer.size();i++)
	{
		PlayerInfo& info = m_vecPlayer[i];
		info.pPlayer->SetRateScale(fRateScale);
		InitPlayerInfoData(&info);
	}
}

int CGroupPlayer::GetGroupId()
{
	return m_nGroupId;
}

void CGroupPlayer::OnTime(int nTimerId)
{
	CSingleZenoLock lock(m_mtxData);

	//状态同步
	for (int i = 0;i < m_vecPlayer.size();i++)
	{
		PlayerInfo& info = m_vecPlayer[i];
		if (info.pPlayer->IsPausePlay() != m_bPausePlay)
		{
			info.pPlayer->PausePlay(m_bPausePlay);
			InitPlayerInfoData(&info);
		}
		if (info.pPlayer->GetRateScale() != m_fRateScale)
		{
			info.pPlayer->SetRateScale(m_fRateScale);
			InitPlayerInfoData(&info);
		}
	}

	//判断下一帧播放时间
	if (m_bPausePlay == false)
	{
		uint32 nCurTime = (uint32)AX_OS::get_time();
		int nMainIndex = -1;							//主导视频索引

		for (int i = 0;i < m_vecPlayer.size();i++)
		{
			PlayerInfo& info = m_vecPlayer[i];
			
			CSingleZenoLock lock(info.pPlayer->GetDataLock());
			CNBPlayer::FrameInfo* pNextRead = info.pPlayer->GetNextReadFrame();
			if (pNextRead)
			{
				info.nNextFrameTime = (uint64)pNextRead->tSynchTime * 1000 + pNextRead->nMilSec;
				info.nNextPaintTime = (uint32)(nCurTime + (1000 / pNextRead->nRate / 2) / m_fRateScale);

				CNBPlayer::FrameInfo* pLastRead = info.pPlayer->GetLastReadFrame();
				if (pLastRead && (pLastRead->nPaintTime != 0))
				{
					uint32 nPaintTime = min(nCurTime, pLastRead->nPaintTime);
					uint32 nLimit = max((uint32)(1000 / m_fRateScale), 1000);
					if ((info.nSetDelay != 0)
						|| ((nCurTime - nPaintTime) < nLimit))
					{
						info.nNextPaintTime = (uint32)(nPaintTime + 1000 / pNextRead->nRate / m_fRateScale);
					}
					else
					{
						int nBreak = 0;
					}
				}
	
				if ((info.nSetDelay > 0)
					&& ((info.nSetDelay + info.nLastSynchNextFramePaintTime + 10) < nCurTime))
				{
					//已经到达延时时间了
					info.nSetDelay = 0;

					/*char cBuf[64];
					sprintf(cBuf, "零1  %d %d %d\n", i, info.nSetDelay, info.nNextFrameTime);
					OutputDebugString(cBuf);*/
				}

				/*char cBuf[64];
				sprintf(cBuf, "aaaa  %d %d %d\n", i, (uint32)info.nNextFrameTime, info.nNextPaintTime);
				OutputDebugString(cBuf);*/

				if (nMainIndex == -1)
				{
					nMainIndex = i;
				}
				else
				{
					PlayerInfo& infoM = m_vecPlayer[nMainIndex];
					if (infoM.nNextFrameTime > info.nNextFrameTime)
					{
						nMainIndex = i;
					}
				}
			}
			else
			{
				info.nNextFrameTime = 0;
				info.nNextPaintTime = nCurTime;
			}
		}

		if (nMainIndex != -1)
		{
			PlayerInfo& infoM = m_vecPlayer[nMainIndex];
			bool bJumpTime = (infoM.nNextFrameTime - m_nLastOnTimeVideoTime) > 1000;

			for (int i = 0;i < m_vecPlayer.size();i++)
			{
				PlayerInfo& info = m_vecPlayer[i];

				if (info.nNextFrameTime == 0)
				{
					continue;
				}

				if (bJumpTime 
					|| (info.bSynchLimit == false)
					|| ((nCurTime - info.nLastSynchTime) > 1000))		//1秒内才能同步一次
				{
					uint32 nNeedPaintTime = (uint32)((info.nNextFrameTime - infoM.nNextFrameTime) / m_fRateScale) + infoM.nNextPaintTime;	//需要绘制的时间

					bool bSynch = false;
					uint32 nOldLastSynchTime = info.nLastSynchTime;

					if ((nNeedPaintTime > info.nNextPaintTime) && ((nNeedPaintTime - info.nNextPaintTime) > 1000))
					{
						if (info.nSetDelay > 1000)
						{

						}
						else
						{
							info.nSetDelay = (int)(nNeedPaintTime - info.nNextPaintTime);
							info.nLastSynchTime = nCurTime;
							info.nLastSynchNextFramePaintTime = info.nNextPaintTime;
							info.pPlayer->SetPaintDelay(info.nSetDelay);
							bSynch = true;

							/*char cBuf[64];
							sprintf(cBuf, "大  %d %d %d %d\n", i, info.nSetDelay, (uint32)info.nNextFrameTime, nNeedPaintTime);
							OutputDebugString(cBuf);*/
						}
					}
					else
					{
						if (nNeedPaintTime <= info.nNextPaintTime)
						{
							if (info.nSetDelay > 0)
							{
								if (info.nSetDelay > 50)
								{
									info.nLastSynchTime = nCurTime;
									info.pPlayer->SetPaintDelay(0);
									bSynch = true;
								}
								info.nSetDelay = 0;

								/*char cBuf[64];
								sprintf(cBuf, "零2  %d %d %d %d\n", i, info.nSetDelay, (uint32)info.nNextFrameTime, nNeedPaintTime);
								OutputDebugString(cBuf);*/
							}
						}
						else
						{
							int nDelay = nNeedPaintTime - info.nNextPaintTime;
							int nDiff = nDelay - info.nSetDelay;
							int nLimit = max(200, (int)(200 / m_fRateScale));
							if ((nDiff > nLimit) || (nDiff < -nLimit))
							{
								info.nSetDelay = nDelay;
								info.nLastSynchTime = nCurTime;
								info.nLastSynchNextFramePaintTime = info.nNextPaintTime;
								info.pPlayer->SetPaintDelay(info.nSetDelay);
								bSynch = true;

								/*char cBuf[64];
								sprintf(cBuf, "小  %d %d %d %d\n", i, info.nSetDelay, (uint32)info.nNextFrameTime, nNeedPaintTime);
								OutputDebugString(cBuf);*/
							}
						}
					}

					if (bSynch)
					{
						info.bSynchLimit = (info.nLastSynchTime - nOldLastSynchTime) < 1000;
					}
				}
			}//for (int i = 0;i < m_vecPlayer.size();i++)

			m_nLastOnTimeVideoTime = infoM.nNextFrameTime;
		}

		m_nLastOnTimeTime = nCurTime;
	}
}
