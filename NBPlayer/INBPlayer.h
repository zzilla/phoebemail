#ifndef __INBPLAYER_INC__
#define __INBPLAYER_INC__

#ifdef NBPLAYER_EXPORTS
#define NBPLAYER_API __declspec(dllexport)
#else
#define NBPLAYER_API __declspec(dllimport)
#endif

#include "platform.h"
enum
{
	StreamSourceType_Dahua = 0,
	StreamSourceType_Hik,
	StreamSourceType_BlueStar,
	StreamSourceType_Dali,
	StreamSourceType_Num,			//流类型个数
};

typedef enum 
{
	ZenoPlayer_Unkown = 0,
	ZenoPlayer_Dahua,
	ZenoPlayer_Hik,
	ZenoPlayer_Bluestar,
	ZenoPlayer_Dali,
}ZenoPlayerProvider;

class INBPlayer
{
public:
	virtual ~INBPlayer() = 0;

	virtual bool Create(int nStmSrcType, uint32 nBufSize, bool bRealTime) = 0;
	virtual void Close(bool bSaveRes) = 0;					//bSaveRes为true表示保留资源，让下一个对象使用，调用ClearNBPlayerResource销毁
	virtual bool IsCreated() = 0;
	virtual int  GetPort() = 0;
	virtual int  GetStmSrcType() = 0;

	virtual bool InputData(const char* pData, uint32 nLen) = 0;

	virtual void AddWnd(HWND hWnd) = 0;
	virtual void DelWnd(HWND hWnd) = 0;

	virtual void PausePlay(bool bPause) = 0;
	virtual bool IsPausePlay() = 0;
	virtual void SetRateScale(double fRateScale) = 0;
	virtual double GetRateScale() = 0;
	virtual void PlayOneFrame() = 0;
	virtual time_t GetPlayingTime() = 0;

	virtual bool RePaintFrame() = 0;
	virtual bool CapturePicture(char* szFilePath) = 0;

	virtual void OpenSound() = 0;

	virtual void SetColorSetup(int nBrightness, int nContrast, int nSaturation, int nHue) = 0;		//0－128
	virtual void GetColorSetup(int* nBrightness, int* nContrast, int* nSaturation, int* nHue) = 0;

	virtual int	 GetBufDataLen() = 0;
	virtual void ClearBufData() = 0;

	//组操作方法
	virtual bool AddToGroup(int nGroupId) = 0;					//nGroupId非0值
	virtual void DelFromGroup() = 0;
	virtual void GroupPausePlay(bool bPause) = 0;
	virtual void GroupSetRateScale(double fRateScale) = 0;

};

class NBPLAYER_API StaticNBPlayer
{
public:
	static void CloseSound();
	static int  GetVolume();
	static void SetVolume(int nVolume);

	static void ClearNBPlayerResource();
};

NBPLAYER_API bool			InitNBPlayerDll();
NBPLAYER_API void			UninitNBPlayerDll();				//在调用UninitNBPlayerDll之前，INBPlayer对象必须全部销毁
NBPLAYER_API INBPlayer*		NewNBPlayer();
NBPLAYER_API void			DeleteNBPlayer(INBPlayer* pPlayer);

#endif //__INBPLAYER_INC__