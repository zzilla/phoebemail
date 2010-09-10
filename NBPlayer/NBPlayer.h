#ifndef __NBPLAYER_INC__
#define __NBPLAYER_INC__

//#include "platform.h"
#include "AX_Mutex.h"
#include "INBPlayer.h"
#include "MY_IAddRefAble.h"
#include "PlaySDK.h"
#include "videorender.h"
#include <vector>
#include <list>
#include <map>
using namespace std;

#define  MAX_FRAME_SIZE			(800*600*3/2)
#define  MAX_BUFFER_FRAMES		6
#define  MAX_PORT_NUM			101

#define  DOUBLE_DIV				1000			//这里用INT型表示浮点型，INT处以1000才是真正的浮点值

struct OSDItemDataEx
{
	HBITMAP hBitmap;				//

	int fLeft;						//位置的百分比，0-1之间
	int fTop;
	int fRight;
	int fBottom;

	COLORREF clTrans;				//透明色
	int fTrans;						//除了透明色外其它颜色的透明度

	RECT	rcOSD;
	HBITMAP hOSDBitmap;
	BYTE*	pOSDBitData;
	BITMAP	bitOSD;
};

class CNBPlayer : public INBPlayer, public MY_IAddRefAble
{
public:
	CNBPlayer();
	virtual ~CNBPlayer();

	friend class CPlayerCBManage;
	friend class CGroupPlayer;

//INBPlayer重载
public:
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
	virtual bool AddToGroup(int nGroupId);		//避免死锁，不可以在锁里面操作CGroupManage
	virtual void DelFromGroup();
	virtual void GroupPausePlay(bool bPause);
	virtual void GroupSetRateScale(double fRateScale);

//回调
protected:
	void	OnTime(int nTimerId);
	void	OnDecode(char* pBuf, long nSize, FRAME_INFO* pFrameInfo);
	void    OnDlDecode(char* pBuf, long nSize, int nWidth, int nHeight, DWORD dwDataType);

//数据
protected:
	AX_Mutex m_mtxData;

	bool	m_bCreate;
	int		m_nStmSrcType;
	uint32	m_nSDKBufSize;
	bool	m_bRealtimePlay;
	bool	m_bOpenPort;
	int		m_nPort;
	int		m_nPaintTimerId;

	bool	m_bPausePlay;
	int		m_fRateScale;
	int		m_nTimerPeriod;			//当前事件的周期

	bool	m_bSetDecodeThreadPriority;
	uint32  m_nLastAjustPeriodTime;	

	struct WndInfo
	{
		HWND hWnd;
		VideoRender* pRander;
	};
	vector<WndInfo>	m_vecWnd;
	list<HWND> m_listUnuseWnd;

	struct FrameInfo
	{
		int		nWidth;
		int		nHeight;
		int		nRate;
		int		nType;

		time_t	tTime;				//视频时间
		time_t	tSynchTime;			//同步用的视频时间

		uint32	nMilSec;			//视频时间的毫秒估计
		uint32	nPaintTime;			//绘制时的时间

		int		nSize;
		char	cData[MAX_FRAME_SIZE];
	};
	FrameInfo* m_pFrames;
	int		m_nFrameNum;			//m_nFrameNum必须比缓冲小1帧，这1帧放最后一次播放的数据
	int		m_nFrmWrite;
	int		m_nFrmRead;

	int		m_nGroupId;				//


//其他数据
protected:
	int m_nBrightness;
	int m_nContrast;
	int m_nSaturation;
	int m_nHue;

	void InitExtendData();
	void EfectExtendData(int nDataType);		//0所有，1颜色设置

//辅助函数
protected:
	AX_Mutex&	GetDataLock();

	uint32		CalePaintTimerPeriod();
	uint32		CalePaintTimerPeriod(int fRateScale, int nRate);
	void		SetPaintTimerPeriod(int nPeriod);
	void		SetPaintDelay(int nDelay);
	void		TinyAjustTimerPeriod(int nBufSize);

	void		PaintFrame(FrameInfo* pFrameInfo);

	FrameInfo*	ReadFrame();
	FrameInfo*	WriteFrame();
	FrameInfo*	GetLastReadFrame();
	FrameInfo*	GetNextReadFrame();
	FrameInfo*	GetLastWriteFrame();

	int			GetSoundId();

//端口管理
protected:
	static AX_Mutex		m_mtxUnusePort;
	static bool			m_bInitUnusePortList;
	static list<int>	m_listUnusePort[StreamSourceType_Num];

	static void		InitUnusePort();
public:
	static int		UsePort(int nDevType);				//错误返回－1
	static void		UnusePort(int nDevType, int nPort);

//大立数据
private:
	long m_lOpenStream;
	int m_nAudioSample;
	int m_nFrameRate;
public:
	long GetlOpenStream();
};

//全局函数
bool IsInitDll();

#endif	//__NBPLAYER_INC__