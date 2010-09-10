#ifndef __PLAYERCBMANAGE_INC__
#define __PLAYERCBMANAGE_INC__

#include "platform.h"
#include "AX_Mutex.h"
#include "AX_IAddRefAble.h"
#include "NBPlayer.h"
#include "TimerManage.h"
#include "PlaySDK.h"
#include <map>
using namespace std;

class CPlayerCBManage
{
public:
	CPlayerCBManage(void);
	~CPlayerCBManage(void);

protected:
	static CPlayerCBManage m_staticThis;
public:
	static CPlayerCBManage* Instance();

//初始化
protected:
	bool m_bManageInit;
public:
	bool InitManage();
	void UnInitManage();

//解码回调函数
protected:
	static void CALLBACK StaticDecodeBackFunc_DH(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK StaticDecodeBackFunc_HK(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	static void CALLBACK StaticDecodeBackFunc_DL(long lOpenStream, DWORD dwDataType, char* pBuf, DWORD nWidth, DWORD nHeight, int err, DWORD dwUser);
	static void StaticOnTimerFunc(int nTimerId, int nContext);
	void DecodeBackFunc_DH(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	void DecodeBackFunc_HK(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
	void DecodeBackFunc_DL(long lOpenStream, DWORD dwDataType, char* pBuf, DWORD nWidth, DWORD nHeight, int err, DWORD dwUser);
	void OnTimerFunc(int nTimerId, int nContext);
public:
	DecodeBackFuncType	GetDecodeCB(int nDevType);
	DLDecodeBackyuvData GetDlDecodeCB();
	OnTimerFuncType		GetOnTimerCB(int nDevType);

//回调对象管理
protected:
	AX_Mutex		m_mtxPlayer;
	map<int, CNBPlayer*> m_mapPlayer[StreamSourceType_Num];
	map<long, int> m_mapDlPlayer;

protected:
	CNBPlayer* GetPlayer(int nDevType, int nPort);
	CNBPlayer* GetDlPlayer(long lOpenStream);

public:
	void RegisterCallBack(CNBPlayer* pPlayer);
	void UnRegisterCallBack(CNBPlayer* pPlayer);

	void SetDlPlayer(CNBPlayer* pPlayer);
};

#endif //__PLAYERCBMANAGE_INC__