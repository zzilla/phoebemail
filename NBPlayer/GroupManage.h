#ifndef __GROUPMANAGE_INC__
#define __GROUPMANAGE_INC__

#include "AX_Mutex.h"
#include "TimerManage.h"
#include "AX_IAddRefAble.h"
#include <map>
using namespace std;

class CNBPlayer;
class CGroupPlayer;
class CGroupManage
{
public:
	CGroupManage(void);
	~CGroupManage(void);

//全局
protected:
	static CGroupManage m_staticThis;
public:
	static CGroupManage* Instance();

//初始化
protected:
	bool m_bManageInit;
public:
	bool InitManage();
	void UnInitManage();

//
public:
	bool AddPlayerToGroup(CNBPlayer* pPlayer, int nGroupId);			//nGroupId不等于0
	void DelPlayerFromGroup(CNBPlayer* pPlayer, int nGroupId);
	CGroupPlayer* GetGroupPlayer(int nGroupId);

//数据
protected:
	AX_Mutex m_mtxData;
	map<int, CGroupPlayer*> m_mapGroup;

//计时器
protected:
	static void StaticOnTimerFunc(int nTimerId, int nContext);
	void OnTimerFunc(int nTimerId, int nContext);
public:
	OnTimerFuncType		GetOnTimerCB();
};

#endif //__GROUPMANAGE_INC__