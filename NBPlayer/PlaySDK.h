#ifndef __PLAYSDK_INC__
#define __PLAYSDK_INC__

#include "INBPlayer.h"
#include "PlayM4.h"
//#include "plaympeg4.h"
#include "dhplay.h"
#include "playdef.h"
#include <time.h>
#include "BSRPlayer.h"
#include "NetSDK.h"

#pragma pack(1)
typedef struct   _DL_MP4FILE_HEAD
{
	BYTE                flag[7];
	char                DVR_name[64];
	char                chan_name[64];
	unsigned char       chan_no; 
	unsigned char       video_encoder;
	unsigned char       audio_encoder;
	unsigned char       audio_sampling;
	unsigned char       video_quality;
	unsigned char       frame_rate;
	unsigned char       video_format;
	unsigned char       resolution;
	unsigned short      video_rate;
	char                card_no[21];
	BYTE                Reserve[90];
	_DL_MP4FILE_HEAD()
	{
		memset(this, 0, sizeof(_DL_MP4FILE_HEAD));
		flag[0] = 'D';
		flag[1] = 'A';
		flag[2] = 'L';
		flag[3] = 'I';
		flag[4] = '2';
		flag[5] = '6';
		flag[6] = '4';
	};
}DL_MP4FILE_HEAD;
#pragma pack()

typedef void (CALLBACK *DecodeBackFuncType)(long nPort, char* pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
typedef void (CALLBACK *DLDecodeBackyuvData)(long lOpenStream, DWORD dwDataType, char* pBuf, DWORD nWidth, DWORD nHeight, int err, DWORD dwUser);

inline bool PlaySDK_OpenStream(int nDevType, int nPort
							  , const char* pHead, uint32 nHeadLen
							  , uint32 nBufSize, DecodeBackFuncType funCB)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		if (PLAY_OpenStream(nPort, (PBYTE)pHead, nHeadLen, nBufSize) == FALSE)
		{
			return false;
		}
		PLAY_SetDecCallBack(nPort, funCB);
		PLAY_Play(nPort, NULL);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		if (PlayM4_OpenStream(nPort, (PBYTE)pHead, nHeadLen, nBufSize) == FALSE)
		{
			return false;
		}
		PlayM4_SetDecCallBack(nPort, funCB);
		PlayM4_Play(nPort, NULL);
		BOOL b = PlayM4_PlaySoundShare(nPort);
		if (!b)
		{
			DWORD dw = PlayM4_GetLastError(nPort);
			int i = 0;
			i++;
		}
	}
	return true;
}

inline void PlaySDK_CloseStream(int nDevType, int nPort)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		PLAY_SetDecCallBack(nPort, NULL);
		PLAY_Stop(nPort);
		PLAY_CloseStream(nPort);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		PlayM4_SetDecCallBack(nPort, NULL);
		PlayM4_StopSoundShare(nPort);
		PlayM4_Stop(nPort);
		PlayM4_CloseStream(nPort);
	}
}

inline bool PlaySDK_InputData(int nDevType, int nPort, const char* pData, uint32 nLen)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		return PLAY_InputData(nPort, (PBYTE)pData, nLen) ? true : false;
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		if (nLen < 40)
		{
			return true;
		}
		return PlayM4_InputData(nPort, (PBYTE)(pData + 40), nLen - 40) ? true : false;
	}
	else
	{
		return false;
	}
}

inline uint32 PlaySDK_GetBufDataLen(int nDevType, int nPort)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		return PLAY_GetSourceBufferRemain(nPort);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		return PlayM4_GetSourceBufferRemain(nPort);
	}
	else
	{
		return 0;
	}
}

inline void PlaySDK_ClearBufData(int nDevType, int nPort)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		PLAY_ResetSourceBuffer(nPort);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		PlayM4_ResetSourceBuffer(nPort);
	}
}

inline time_t PlaySDK_GetPlayingTime(int nDevType, int nPort)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		time_t tResult = 0;
		TimeInfo ti;
		int retlen;
		if (PLAY_QueryInfo(nPort, PLAY_CMD_GetTime, (char*)&ti, sizeof(TimeInfo), &retlen))
		{
			tm temp;
			memset(&temp, 0, sizeof(temp));
			temp.tm_year = ti.year - 1900;
			temp.tm_mon = ti.month - 1;
			temp.tm_mday = ti.day;
			temp.tm_hour = ti.hour;
			temp.tm_min = ti.minute;
			temp.tm_sec = ti.second;
			temp.tm_isdst = -1;

			tResult = mktime(&temp);
			if (tResult == -1)
			{
				tResult = 0;
			}
		}
		return tResult;
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		DWORD dwTime = PlayM4_GetSpecialData(nPort);

		time_t tResult = 0;
		tm temp;
		memset(&temp, 0, sizeof(temp));
		temp.tm_year = (((dwTime)>>26) + 2000) - 1900;
		temp.tm_mon = (((dwTime)>>22) & 15) - 1;
		temp.tm_mday = (((dwTime)>>17) & 31);
		temp.tm_hour = (((dwTime)>>12) & 31);
		temp.tm_min = (((dwTime)>>6) & 63);
		temp.tm_sec = (((dwTime)>>0) & 63);
		temp.tm_isdst = -1;

		tResult = mktime(&temp);
		if (tResult == -1)
		{
			tResult = 0;
		}

		return tResult;
	}
	else
	{
		return 0;
	}
}

inline time_t PlaySDK_GetPlayingTimeEx(int nDevType, int nPort)		//模拟成同日的
{
	if (nDevType == StreamSourceType_Dahua)
	{
		time_t tResult = 0;
		TimeInfo ti;
		int retlen;
		if (PLAY_QueryInfo(nPort, PLAY_CMD_GetTime, (char*)&ti, sizeof(TimeInfo), &retlen))
		{
			tm temp;
			memset(&temp, 0, sizeof(temp));
			temp.tm_year = 2000 - 1900;
			temp.tm_mon = 1 - 1;
			temp.tm_mday = 1;
			temp.tm_hour = ti.hour;
			temp.tm_min = ti.minute;
			temp.tm_sec = ti.second;
			temp.tm_isdst = -1;

			tResult = mktime(&temp);
			if (tResult == -1)
			{
				tResult = 0;
			}
		}
		return tResult;
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		DWORD dwTime = PlayM4_GetSpecialData(nPort);

		time_t tResult = 0;
		tm temp;
		memset(&temp, 0, sizeof(temp));
		temp.tm_year = 2000 - 1900;
		temp.tm_mon = 1 - 1;
		temp.tm_mday = 1;
		temp.tm_hour = (((dwTime)>>12) & 31);
		temp.tm_min = (((dwTime)>>6) & 63);
		temp.tm_sec = (((dwTime)>>0) & 63);
		temp.tm_isdst = -1;

		tResult = mktime(&temp);
		if (tResult == -1)
		{
			tResult = 0;
		}

		return tResult;
	}
	else
	{
		return 0;
	}
}

inline bool PlaySDK_CapturePicture(int nDevType, const char* pBuf, int nSize, int nWidth, int nHeight, int nType, const char* szFileName)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		return (bool)PLAY_ConvertToBmpFile((char*)pBuf, nSize, nWidth, nHeight, nType, (char*)szFileName);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		return (bool)PlayM4_ConvertToBmpFile((char*)pBuf, nSize, nWidth, nHeight, nType, (char*)szFileName);
	}
	return false;
}

inline void PlaySDK_SetColor(int nDevType, int nPort, int nBrightness, int nContrast, int nSaturation, int nHue)
{
	if (nDevType == StreamSourceType_Dahua)
	{
		PLAY_SetColor(nPort, 0, nBrightness, nContrast, nSaturation, nHue);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		if (!PlayM4_SetColor(nPort, 0, nBrightness, nContrast, nSaturation, nHue))
		{
			DWORD dw = PlayM4_GetLastError(nPort);

			int i = 0;
			i++;
		}	
	}
}

inline DWORD PlaySDK_GetLastError(int nDevType, int nPort)
{
	DWORD dw = 0;
	if (nDevType == StreamSourceType_Dahua)
	{
		dw = PLAY_GetLastError(nPort);
	}
	else if (nDevType == StreamSourceType_Hik)
	{
		dw = PlayM4_GetLastError(nPort);
	}
	return dw;
}

inline bool PlaySDK_DL_Init()
{
	return NET_DLDVR_Init();
}

inline bool PlaySDK_DL_Cleanup()
{
	return NET_DLDVR_Cleanup();
}

inline bool PlaySDK_DL_OpenStream(long& lOpenStream, const char* pHead, uint32 nHeadLen, DLDecodeBackyuvData funCB, int& nAudioSample, int& nFrameRate)
{
	//int nLen = sizeof(_DL_MP4FILE_HEAD);
	DL_MP4FILE_HEAD *pDl = (DL_MP4FILE_HEAD *)pHead;

	//int nAudioType = (int)pDl->audio_encoder;
	int nSDKAudioSample = (int)pDl->audio_sampling;
	int nSDKFrameRate = (int)pDl->frame_rate;
	lOpenStream = NET_DLDVR_OpenStream(NULL, (BYTE*)pHead, nHeadLen, 1, (int)pDl->video_encoder);
	if (lOpenStream < 0)
	{
		lOpenStream = 0;
		return false;
	}

	nAudioSample = nSDKAudioSample;
	nFrameRate = nSDKFrameRate;
	bool bSound = NET_DLDVR_OpenSoundShare(lOpenStream);
	NET_DLDVR_SetYUVDataCallBack(lOpenStream, (fYUVDataCallBack)funCB, 0);
	return true;
}

inline bool PlaySDK_DL_InputData(long lOpenStream, const char* pData, uint32 nLen)
{
	if (nLen < 256)
	{
		return true;
	}

	long nSize = NET_DLDVR_SetStreamBuf(lOpenStream, (BYTE *)(pData + 256), nLen - 256);
	//if (nSize == nLen - 256)
	if (nSize > 0)
	{
		return true;
	}
	return false;
}

inline void PlaySDK_DL_CloseStream(long lOpenStream)
{
	NET_DLDVR_EmptyStreamBuf(lOpenStream);
	NET_DLDVR_CloseSoundShare(lOpenStream);
	NET_DLDVR_CloseStream(lOpenStream);
}

inline void PlaySDK_DL_ClearBufData(long lOpenStream)
{
	NET_DLDVR_EmptyStreamBuf(lOpenStream);
}

inline time_t PlaySDK_DL_GetPlayingTime(long lOpenStream)
{
	time_t tResult = 0;
	NET_DLDVR_TIME OsdTime = {0};
	if (NET_DLDVR_GetPlayBackOsdTime(lOpenStream, &OsdTime))
	{
		tm temp;
		memset(&temp, 0, sizeof(temp));
		temp.tm_year = OsdTime.dwYear - 1900;
		temp.tm_mon = OsdTime.dwMonth - 1;
		temp.tm_mday = OsdTime.dwDay;
		temp.tm_hour = OsdTime.dwHour;
		temp.tm_min = OsdTime.dwMinute;
		temp.tm_sec = OsdTime.dwSecond;
		temp.tm_isdst = -1;

		tResult = mktime(&temp);
		if (tResult == -1)
		{
			tResult = 0;
		}
	}
	return tResult;
}

inline time_t PlaySDK_DL_GetPlayingTimeEx(long lOpenStream)		//模拟成同日的
{
	time_t tResult = 0;
	NET_DLDVR_TIME OsdTime = {0};
	if (NET_DLDVR_GetPlayBackOsdTime(lOpenStream, &OsdTime))
	{
		tm temp;
		memset(&temp, 0, sizeof(temp));
		temp.tm_year = 2000 - 1900;
		temp.tm_mon = 1 - 1;
		temp.tm_mday = 1;
		temp.tm_hour = OsdTime.dwHour;
		temp.tm_min = OsdTime.dwMinute;
		temp.tm_sec = OsdTime.dwSecond;
		temp.tm_isdst = -1;

		tResult = mktime(&temp);
		if (tResult == -1)
		{
			tResult = 0;
		}
	}
	return tResult;
}

inline bool PlaySDK_DL_CapturePicture(long lOpenStream, char* szFileName)
{
	//if (!NET_DLDVR_CapturePicture(lOpenStream, szFileName))
	//{
	//	DWORD dw = NET_DLDVR_GetLastError();
	//	int i = 0;
	//	i++;
	//	return false;
	//}
	//
	//return true;
	return false;
}

inline void PlaySDK_DL_SetColor(long lOpenStream, int nBrightness, int nContrast, int nSaturation, int nHue)
{
	//if (!NET_DLDVR_SetColor(lOpenStream, 0, nBrightness, nContrast, nSaturation, nHue))
	//{
	//	DWORD dw = NET_DLDVR_GetLastError();
	//	int i = 0;
	//	i++;
	//}
}

#endif //__PLAYSDK_INC__