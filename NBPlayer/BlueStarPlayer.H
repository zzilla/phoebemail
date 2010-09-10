#ifndef __BLUESTARPLAYER_INC__
#define __BLUESTARPLAYER_INC__

#include "INBPlayer.h"
#include "MY_IAddRefAble.h"
#include "AX_Mutex.h"
#include "videorender.h"
#include <vector>
#include <list>
#include <map>
using namespace std;

#define MAX_BUFF_SIZE 1024*300
#define HEAD_BUFF_SIZE 32
#define HEAD_PACKET_SIZE 256
#define DOUBLE_MUL 16

class CBlueStarPlayer : public INBPlayer, public MY_IAddRefAble
{
public:
	CBlueStarPlayer(void);
	virtual ~CBlueStarPlayer(void);

	virtual bool Create(int nStmSrcType, uint32 nBufSize, bool bRealTime);
	virtual void Close(bool bSaveRes);
	virtual bool IsCreated();
	virtual int  GetPort();
	virtual int  GetStmSrcType();

	virtual bool InputData(const char* pData, uint32 nLen);

	virtual void AddWnd(HWND hWnd);
	virtual void DelWnd(HWND hWnd);

	virtual void PausePlay(bool bPause);
	virtual bool IsPausePlay();
	virtual void SetRateScale(double fRateScale);
	virtual double GetRateScale();
	virtual void PlayOneFrame();
	virtual time_t GetPlayingTime();
	virtual bool RePaintFrame();
	virtual bool CapturePicture(char* szFilePath);

	virtual void OpenSound();

	virtual void SetColorSetup(int nBrightness, int nContrast, int nSaturation, int nHue);
	virtual void GetColorSetup(int* nBrightness, int* nContrast, int* nSaturation, int* nHue);

	virtual int	 GetBufDataLen();
	virtual void ClearBufData();

	//组操作方法
	virtual bool AddToGroup(int nGroupId);					//nGroupId非0值
	virtual void DelFromGroup();
	virtual void GroupPausePlay(bool bPause);
	virtual void GroupSetRateScale(double fRateScale);
protected:
	AX_Mutex m_mtxData;
	bool	 m_bCreate;
	int      m_nStmSrcType;
	int      m_nPort;
	bool     m_bChangeWnd;
	bool     m_bOpenStream;
	bool     m_bPausePlay;
	int      m_fRateScale;
	BOOL     m_bPlay;

	vector<HWND>	m_vecWnd;
	HWND m_PlayWnd;//当前播放窗口句柄

	//char m_szBuf[MAX_BUFF_SIZE];
	char *m_pBuf;
	int  m_nBufCurLen;//当前缓冲区位置

	void SetRateScale();
	HANDLE m_thrHandle;

public:
	static void CALLBACK MyDrawTitleFun(LONG nPort, HDC hDC, long time, DWORD dwWidth, DWORD dwHeight, LPVOID lpVoid);

protected:
	static DWORD WINAPI StaticInputDataThread(LPVOID pVoid);
	void InputDataThread();
	
};

#endif	//__BLUESTARPLAYER_INC__