#ifndef __GROUPPLAYER_INC__
#define __GROUPPLAYER_INC__

#include "AX_Mutex.h"
#include "MY_IAddRefAble.h"
#include <vector>
using namespace std;

class CNBPlayer;
class CGroupPlayer : public MY_IAddRefAble
{
public:
	CGroupPlayer(int nGroupId);
	virtual ~CGroupPlayer(void);

	friend class CGroupManage;

//接口
public:
	void	PausePlay(bool bPause);
	void	SetRateScale(double fRateScale);

	int		GetGroupId();

//辅助函数
protected:
	int		AddPlayer(CNBPlayer* pPlayer);
	int		DelPlayer(CNBPlayer* pPlayer);

//回调
public:
	void	OnTime(int nTimerId);

//数据
protected:
	AX_Mutex	m_mtxData;

	struct PlayerInfo
	{
		CNBPlayer*	pPlayer;

		int			nSetDelay;				//最后一次同步的Delay的值
		uint32		nLastSynchTime;			//最后一次发送同步消息的时间
		uint32		nLastSynchNextFramePaintTime;
		bool		bSynchLimit;

		uint64		nNextFrameTime;			//临时保存下一帧的视频时间
		uint32		nNextPaintTime;			//临时保存下一帧的绘制时间(无延时情况下)
	};
	vector<PlayerInfo> m_vecPlayer;
	uint32		m_nLastOnTimeTime;			//最后一次ONTIME的时间
	uint64		m_nLastOnTimeVideoTime;		//

	int			m_nGroupId;
	int			m_nWatchTimerId;
	bool		m_bPausePlay;
	double		m_fRateScale;
protected:
	void		InitPlayerInfoData(PlayerInfo* pInfo);
};

#endif //__GROUPPLAYER_INC__