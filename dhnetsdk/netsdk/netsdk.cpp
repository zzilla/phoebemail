// netsdk.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#include "Manager.h"
#include "RealPlay.h"
#include "SearchRecordAndPlayBack.h"
#include "AlarmDeal.h"
#include "DevConfig.h"
#include "DevConfigEx.h"
#include "DevControl.h"
#include "RenderManager.h"
#include "Talk.h"
#include "ServerSet.h"
#include "SnapPicture.h"
#include "GPSSubcrible.h"
#include "DecoderDevice.h"
#include "AutoRegister.h"
#include "DevNewConfig.h"

/************************************************************************
 ** 网络SDK实现管理者
 ***********************************************************************/
CManager g_Manager;

extern DEVMutex g_csFlag;

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

CLIENT_API BOOL CALL_METHOD CLIENT_Init(fDisConnect cbDisConnect, DWORD dwUser)
{
	BOOL bRet = FALSE;

	int r = g_Manager.Init(cbDisConnect,dwUser);
	if (r >= 0)
	{
		bRet = TRUE;
	}
	else
	{
		g_Manager.SetLastError(r);
	}
	
    return bRet;
}

CLIENT_API void  CALL_METHOD CLIENT_Cleanup()
{
	int r = g_Manager.Uninit();
	if (r < 0)
	{
#ifdef _DEBUG
		OutputDebugString("Error:程序清理环境出错-->CLIENT_Cleanup()\n");
#endif
		g_Manager.SetLastError(r);
	}
	else
	{
		g_Manager.SetLastError(NET_NOERROR);
	}
}

CLIENT_API void CALL_METHOD CLIENT_SetAutoReconnect(fHaveReConnect cbAutoConnect, DWORD dwUser)
{
	g_Manager.SetAutoReconnCallBack(cbAutoConnect, dwUser);
}

CLIENT_API void CALL_METHOD CLIENT_SetSubconnCallBack(fSubDisConnect cbSubDisConnect, DWORD dwUser)
{
	g_Manager.SetSubDisconnCallBack(cbSubDisConnect, dwUser);
}

CLIENT_API void CALL_METHOD CLIENT_SetConnectTime(int nWaitTime, int nTryTimes)
{
	g_Manager.SetConnectTryNum(nTryTimes);
	g_Manager.SetConnectTime(nWaitTime);
}

CLIENT_API void CALL_METHOD CLIENT_SetNetworkParam(NET_PARAM *pNetParam)
{
	g_Manager.SetNetParameter(pNetParam);
}

CLIENT_API void CALL_METHOD CLIENT_SetDVRMessCallBack(fMessCallBack cbMessage,DWORD dwUser)
{
	g_Manager.SetMessCallBack(cbMessage,dwUser);
}

CLIENT_API void CALL_METHOD CLIENT_RigisterDrawFun(fDrawCallBack cbDraw, DWORD dwUser)
{
	g_Manager.SetDrawCallBack(cbDraw,dwUser);
}

CLIENT_API DWORD CALL_METHOD CLIENT_GetSDKVersion()
{
    return 0x00000999; // 0x03030600//3.3.6.0
}


CLIENT_API LONG CALL_METHOD CLIENT_Login(char *pchDVRIP, WORD wDVRPort, 
                       char *pchUserName, char *pchPassword, LPNET_DEVICEINFO lpDeviceInfo, int *error)
{
	LONG lRet = g_Manager.Login_Dev(pchDVRIP,wDVRPort,pchUserName,pchPassword,lpDeviceInfo,error);
// #ifdef _DEBUG
// 	CLIENT_StartListen(lRet);
// #endif
	return lRet;
}


CLIENT_API LONG CALL_METHOD CLIENT_LoginEx(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPassword, int nSpecCap, void* pCapParam, LPNET_DEVICEINFO lpDeviceInfo, int *error)
{
	LONG lRet = g_Manager.Login_DevEx(pchDVRIP,wDVRPort,pchUserName,pchPassword,nSpecCap,pCapParam,lpDeviceInfo,error);
	return lRet;
}


CLIENT_API BOOL CALL_METHOD CLIENT_Logout(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.Logout_Dev(lLoginID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL	CALL_METHOD CLIENT_AdjustFluency(LONG lRealHandle, int nLevel)
{
	int r = g_Manager.GetRealPlay().AdjustFluency(lRealHandle, nLevel);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_RealPlay(LONG lLoginID, int nChannelID, HWND hWnd)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	LONG ret = g_Manager.GetRealPlay().StartRealPlay(lLoginID,nChannelID,hWnd);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret;
}

CLIENT_API LONG CALL_METHOD CLIENT_RealPlayEx(LONG lLoginID, int nChannelID, HWND hWnd, RealPlayType rType)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	LONG lret = 0;
	switch(rType) 
	{
	case RType_Realplay:	    //实时监视-主码流
		lret = g_Manager.GetRealPlay().StartRealPlay(lLoginID,nChannelID, hWnd);
		break;
	case RType_Multiplay:	//多画面预览
		lret = g_Manager.GetRealPlay().StartMultiPlay(lLoginID, 0, hWnd, rType);
		break;
	case RType_Realplay_0:	//实时监视-主码流
		lret = g_Manager.GetRealPlay().StartRealPlay(lLoginID,nChannelID, hWnd, 0);
		break;
	case RType_Realplay_1:	//实时监视-从码流1
		lret = g_Manager.GetRealPlay().StartRealPlay(lLoginID,nChannelID, hWnd, 1);
		break;
	case RType_Realplay_2:	//实时监视-从码流2
		lret = g_Manager.GetRealPlay().StartRealPlay(lLoginID,nChannelID, hWnd, 2);
		break;
	case RType_Realplay_3:	//实时监视-从码流3
		lret = g_Manager.GetRealPlay().StartRealPlay(lLoginID,nChannelID, hWnd, 3);
		break;
	case RType_Multiplay_1:
	case RType_Multiplay_4:
	case RType_Multiplay_6:
	case RType_Multiplay_8:
	case RType_Multiplay_9:
	case RType_Multiplay_12:
	case RType_Multiplay_16:
		lret = g_Manager.GetRealPlay().StartMultiPlay(lLoginID, nChannelID, hWnd, rType);
		break;
	default:
		break;
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lret;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopRealPlay(LONG lRealHandle)
{
	int r = g_Manager.GetRealPlay().StopRealPlay(lRealHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopRealPlayEx(LONG lRealHandle)
{
	int r = g_Manager.GetRealPlay().StopRealPlay(lRealHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;	
}

CLIENT_API BOOL CALL_METHOD CLIENT_ClientSetVideoEffect(LONG lPlayHandle, 
        unsigned char brightness, unsigned char contrast, 
        unsigned char hue, unsigned char saturation)
{
	int r = g_Manager.GetRealPlay().SetDecoderVideoEffect(lPlayHandle,brightness,contrast,hue,saturation);
	if (r < 0)
	{
		r = g_Manager.GetPlayBack().SetDecoderVideoEffect(lPlayHandle,brightness,contrast,hue,saturation);
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_ClientGetVideoEffect(LONG lPlayHandle, unsigned char *brightness, unsigned char *contrast, 
                                      unsigned char *hue, unsigned char *saturation)
{
	int r = g_Manager.GetRealPlay().GetDecoderVideoEffect(lPlayHandle,brightness,contrast,hue,saturation);
	if (r < 0)
	{
		r = g_Manager.GetPlayBack().GetDecoderVideoEffect(lPlayHandle,brightness,contrast,hue,saturation);
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_OpenSound(LONG hPlayHandle)
{
	int ret = g_Manager.GetRealPlay().Decoder_OpenSound(hPlayHandle);
	if (ret < 0)
	{
		ret = g_Manager.GetPlayBack().Decoder_OpenSound(hPlayHandle);
	}

	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}

	return ret<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_CloseSound()
{
	int r = g_Manager.GetRealPlay().Decoder_CloseSound();
	if (r < 0)
	{
		r = g_Manager.GetPlayBack().Decoder_CloseSound();
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetVolume(LONG lPlayHandle, int nVolume)
{
	int ret = g_Manager.GetRealPlay().SetVolume(lPlayHandle,nVolume);
	if (ret < 0)
	{
		ret = g_Manager.GetPlayBack().SetVolume(lPlayHandle,nVolume);
	}

	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}

	return ret<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_SetDeviceMode(LONG lLoginID, EM_USEDEV_MODE emType, void* pValue)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	BOOL bRet = g_Manager.SetDeviceMode(lLoginID, emType, pValue);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return bRet;
}

CLIENT_API LONG CALL_METHOD CLIENT_StartTalkEx(LONG lLoginID, pfAudioDataCallBack pfcb, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetTalk().StartTalk(lLoginID, pfcb, dwUser);
	
	if (lRet <= 0)
	{
		g_Manager.SetLastError(lRet);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet<=0?0:lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetAudioClientVolume(LONG lTalkHandle, WORD wVolume)
{
	BOOL bRet = FALSE;
	bRet = g_Manager.GetTalk().SetVolume(lTalkHandle, wVolume);
	if (FALSE == bRet)
	{
		g_Manager.SetLastError(-1);
	}

	return bRet;
}

CLIENT_API LONG CALL_METHOD CLIENT_TalkSendData(LONG lTalkHandle, char *pSendBuf, DWORD dwBufSize)
{
	LONG lRet = -1;
	lRet = g_Manager.GetTalk().TalkSendData(lTalkHandle, pSendBuf, dwBufSize);
	if (lRet <= 0)
	{
		g_Manager.SetLastError(lRet);
	}

	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_RecordStart()
{
	BOOL bRet = FALSE;
	bRet = g_Manager.GetTalk().RecordStart();
	if (FALSE == bRet)
	{
		g_Manager.SetLastError(-1);
	}

	return bRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_RecordStop()
{
	BOOL bRet = FALSE;
	bRet = g_Manager.GetTalk().RecordStop();
	if (FALSE == bRet)
	{
		g_Manager.SetLastError(-1);
	}

	return bRet;
}

CLIENT_API void CALL_METHOD CLIENT_AudioDec(char *pAudioDataBuf, DWORD dwBufSize)
{
	g_Manager.GetTalk().AudioDec(pAudioDataBuf, dwBufSize);
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopTalkEx(LONG lTalkHandle)
{
	BOOL bRet = FALSE;
	bRet = g_Manager.GetTalk().StopTalk(lTalkHandle);
	if (FALSE == bRet)
	{
		g_Manager.SetLastError(-1);
	}

	return bRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_AudioBroadcastAddDev(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	BOOL bRet = FALSE;
	bRet = g_Manager.GetTalk().BroadcastAddDev(lLoginID);
	if (FALSE == bRet)
	{
		g_Manager.SetLastError(-1);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return bRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_AudioBroadcastDelDev(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	BOOL bRet = FALSE;
	bRet = g_Manager.GetTalk().BroadcastDelDev(lLoginID);
	if (FALSE == bRet)
	{
		g_Manager.SetLastError(-1);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return bRet;
}

CLIENT_API int CALL_METHOD CLIENT_InitAudioEncode(AUDIO_FORMAT aft)
{
	int iRet = 0;
	iRet = g_Manager.GetTalk().InitAudioEncode(aft);

	return iRet;
}


CLIENT_API int	CALL_METHOD CLIENT_AudioEncode(LONG lTalkHandle, BYTE *lpInBuf, DWORD *lpInLen, BYTE *lpOutBuf, DWORD *lpOutLen)
{
	int iRet = 0;
	iRet = g_Manager.GetTalk().AudioEncode(lTalkHandle, lpInBuf, lpInLen, lpOutBuf, lpOutLen);

	return iRet;
}

CLIENT_API int	CALL_METHOD CLIENT_ReleaseAudioEncode()
{
	int iRet = 0;
	iRet = g_Manager.GetTalk().ReleaseAudioEncode();

	return iRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SaveRealData(LONG lRealHandle, const char *pchFileName)
{
	int ret = g_Manager.GetRealPlay().StartSaveRealData(lRealHandle,pchFileName);
	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}
	
	return ret<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopSaveRealData(LONG lRealHandle)
{
	int r = g_Manager.GetRealPlay().StopSaveRealData(lRealHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetRealDataCallBack(LONG lRealHandle, fRealDataCallBack cbRealData, DWORD dwUser)
{
	int r = g_Manager.GetRealPlay().SetRealDataCallBack(lRealHandle,cbRealData,dwUser);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetRealDataCallBackEx(LONG lRealHandle, fRealDataCallBackEx cbRealData, DWORD dwUser, DWORD dwFlag)
{
	int r = g_Manager.GetRealPlay().SetRealDataCallBackEx(lRealHandle,cbRealData,dwUser,dwFlag);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_CapturePicture(LONG hPlayHandle, const char *pchPicFileName)
{
	int r = g_Manager.GetRealPlay().CapturePicture(hPlayHandle,pchPicFileName);
	if (r < 0)
	{
		r = g_Manager.GetPlayBack().CapturePicture(hPlayHandle,pchPicFileName);
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryRecordTime(LONG lLoginID, int nChannelId, int nRecordFileType, LPNET_TIME tmStart, LPNET_TIME tmEnd, char* pchCardid, BOOL *bResult, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	BOOL b = g_Manager.GetPlayBack().QueryRecordTime(lLoginID,
		nChannelId,
		nRecordFileType,
		tmStart,
		tmEnd,
		pchCardid,
		bResult,
		waittime);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return b;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryRecordFile(LONG lLoginID, 
								int nChannelId, 
								int nRecordFileType, 
								LPNET_TIME time_start,
								LPNET_TIME time_end,
								char* cardid, 
								LPNET_RECORDFILE_INFO fileinfo,
                                int maxlen,
                                int *filecount,
								int waittime,
                                BOOL bTime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	BOOL b = g_Manager.GetPlayBack().QueryRecordFile(lLoginID,
												nChannelId,
												nRecordFileType,
												time_start,
												time_end,
												cardid,
												fileinfo,
												maxlen,
												filecount,
												waittime,
												bTime);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return b;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryFurthestRecordTime(LONG lLoginID, int nRecordFileType, char *pchCardid,  NET_FURTHEST_RECORD_TIME* pFurthrestTime, int nWaitTime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	BOOL b = g_Manager.GetPlayBack().QueryFurthestRecordTime(lLoginID, nRecordFileType, pchCardid, pFurthrestTime, nWaitTime);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return b;
}


CLIENT_API LONG CALL_METHOD CLIENT_PlayBackByRecordFile(LONG lLoginID, 
									  LPNET_RECORDFILE_INFO lpRecordFile, 
									  HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	LONG ret = g_Manager.GetPlayBack().PlayBackByRecordFile(lLoginID,lpRecordFile,hWnd,
											cbDownLoadPos,dwUserData);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret;
}

CLIENT_API LONG CALL_METHOD CLIENT_PlayBackByRecordFileEx(LONG lLoginID, LPNET_RECORDFILE_INFO lpRecordFile, 
				HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser, 
				fDataCallBack fDownLoadDataCallBack, DWORD dwDataUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	LONG ret = g_Manager.GetPlayBack().PlayBackByRecordFileEx(lLoginID,lpRecordFile,hWnd,
		cbDownLoadPos,dwPosUser, fDownLoadDataCallBack, dwDataUser);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret;
}

CLIENT_API BOOL CALL_METHOD CLIENT_PausePlayBack(LONG lPlayHandle, BOOL bPause)
{
	int r = g_Manager.GetPlayBack().PausePlayBack(lPlayHandle,bPause);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SeekPlayBack(LONG lPlayHandle, unsigned int offsettime, unsigned int offsetbyte)
{
	int r = g_Manager.GetPlayBack().SeekPlayBack(lPlayHandle,offsettime,offsetbyte);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopPlayBack(LONG lPlayHandle)
{
	int r = g_Manager.GetPlayBack().StopPlayBack(lPlayHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StepPlayBack(LONG lPlayHandle, BOOL bStop)
{
	int r = g_Manager.GetPlayBack().StepPlayBack(lPlayHandle,bStop);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_FastPlayBack(LONG lPlayHandle)
{
	int r = g_Manager.GetPlayBack().FastPlayBack(lPlayHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SlowPlayBack(LONG lPlayHandle)
{
	int r = g_Manager.GetPlayBack().SlowPlayBack(lPlayHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_NormalPlayBack(LONG lPlayHandle)
{
	int r = g_Manager.GetPlayBack().NormalPlayBack(lPlayHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetFramePlayBack(LONG lPlayHandle, int framerate)
{
	int r = g_Manager.GetPlayBack().SetFramePlayBack(lPlayHandle,framerate);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_GetFramePlayBack(LONG lPlayHandle, int *fileframerate, int *playframerate)
{
	int r = g_Manager.GetPlayBack().GetFramePlayBack(lPlayHandle,fileframerate,playframerate);
	if (r < 0)
	{
		r = g_Manager.GetRealPlay().GetFrameRealPlay(lPlayHandle,fileframerate,playframerate);
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_GetPlayBackOsdTime(LONG lPlayHandle, LPNET_TIME lpOsdTime, LPNET_TIME lpStartTime, LPNET_TIME lpEndTime)
{
	int r = g_Manager.GetPlayBack().GetPlayBackOsdTime(lPlayHandle,lpOsdTime,lpStartTime,lpEndTime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_DownloadByRecordFile(LONG lLoginID,LPNET_RECORDFILE_INFO lpRecordFile, char *sSavedFileName, 
                                      fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetPlayBack().DownloadByRecordFile(lLoginID,lpRecordFile,sSavedFileName,
												cbDownLoadPos,dwUserData);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API LONG CALL_METHOD CLIENT_DownloadByTime(LONG lLoginID, int nChannelId, int nRecordFileType, LPNET_TIME tmStart, LPNET_TIME tmEnd, char *sSavedFileName, fTimeDownLoadPosCallBack cbTimeDownLoadPos, DWORD dwUserData)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

    LONG lRet = g_Manager.GetPlayBack().DownloadByTime(lLoginID,nChannelId, nRecordFileType, tmStart, tmEnd, sSavedFileName,
		cbTimeDownLoadPos,dwUserData);;

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopDownload(LONG lFileHandle)
{
	int r = g_Manager.GetPlayBack().StopDownload(lFileHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_GetDownloadPos(LONG lFileHandle, int *nTotalSize, int *nDownLoadSize)
{
	int r = g_Manager.GetPlayBack().GetDownloadPos(lFileHandle,nTotalSize,nDownLoadSize);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_StartListen(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetAlarmDeal().StartListen(lLoginID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StartListenEx(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetAlarmDeal().StartListenEx(lLoginID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_StopListen(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetAlarmDeal().StopListen(lLoginID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_StartService(WORD wPort, char *pIp /* = NULL */, fServiceCallBack pfscb /* = NULL */, 
												DWORD dwTimeOut /* = 0xffffffff */, DWORD dwUserData /* = 0 */)
{
	g_Manager.GetCS().Lock();

	LONG lRet = g_Manager.GetAlarmServer().StartServer(wPort, pIp, pfscb, dwTimeOut, dwUserData);

	g_Manager.GetCS().UnLock();
	
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopService(LONG lHandle)
{
	g_Manager.GetCS().Lock();

	BOOL bRet = g_Manager.GetAlarmServer().StopServer(lHandle);

	g_Manager.GetCS().UnLock();

	return bRet;
}
CLIENT_API LONG CALL_METHOD CLIENT_MultiPlay(LONG lLoginID,  HWND hWnd)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetRealPlay().StartMultiPlay(lLoginID, 0, hWnd);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopMultiPlay(LONG lMultiHandle)
{
	int ret = g_Manager.GetRealPlay().StopMultiPlay(lMultiHandle);
	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}

	return ret<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryConfig(LONG lLoginID, int nConfigType, 
                                   char *pConfigbuf, int maxlen, int *nConfigbuflen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryConfig(lLoginID,nConfigType, 0,pConfigbuf,maxlen,nConfigbuflen,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetupConfig(LONG lLoginID, int nConfigType, 
                                   char *pConfigbuf, int nConfigbuflen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().SetupConfig(lLoginID,nConfigType, 0, pConfigbuf,nConfigbuflen,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_QueryChannelName(LONG lLoginID, 
                                        char *pChannelName, int maxlen, int *nChannelCount, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryChannelName(lLoginID,pChannelName,maxlen,nChannelCount,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_PTZControl(LONG lLoginID, int nChannelID, DWORD dwPTZCommand,DWORD dwStep, BOOL dwStop)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int ret = g_Manager.GetRealPlay().PTZControl(lLoginID, nChannelID, dwPTZCommand, dwStep, dwStop);
	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_YWPTZControl(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, 
                                    unsigned char param1, unsigned char param2, unsigned char param3, 
                                    BOOL dwStop)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int ret = g_Manager.GetRealPlay().YWPTZControl(lLoginID, nChannelID, dwPTZCommand, param1, param2, param3, dwStop);
	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_YWPTZControlEx(LONG lLoginID, int nChannelID, DWORD dwPTZCommand, 
                                    LONG lParam1, LONG lParam2, LONG lParam3, BOOL dwStop)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int ret = g_Manager.GetRealPlay().YWPTZControlEx(lLoginID,nChannelID,dwPTZCommand,lParam1,lParam2,lParam3,dwStop);
	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryLogCallback(LONG lLoginID, fLogDataCallBack cbLogData, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().QueryLogCallback(lLoginID,cbLogData,dwUser);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
	
}


CLIENT_API BOOL CALL_METHOD CLIENT_QueryLog(LONG lLoginID, char *pLogBuffer, int maxlen, int *nLogBufferlen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryLog(lLoginID,pLogBuffer,maxlen,nLogBufferlen,NULL,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

//扩展接口，可选择日志类型
CLIENT_API BOOL CALL_METHOD CLIENT_QueryLogEx(LONG lLoginID, LOG_QUERY_TYPE logType, char *pLogBuffer, int maxlen, int *nLogBufferlen, void* reserved, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().QueryLog(lLoginID, pLogBuffer, maxlen, nLogBufferlen, reserved, waittime, (int)logType);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);

	return r<0?FALSE:TRUE;
}


// 查询设备日志，查询大量日志，可以用分页方式查询：
CLIENT_API BOOL CALL_METHOD CLIENT_QueryDeviceLog(LONG lLoginID,QUERY_DEVICE_LOG_PARAM *pQueryParam, char *pLogBuffer, int nLogBufferLen, int *pRecLogNum, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfigEx().QueryDeviceLog(lLoginID, pQueryParam, pLogBuffer, nLogBufferLen, pRecLogNum, waittime);

	if(r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_CreateTransComChannel(LONG lLoginID, int TransComType, 
                                      unsigned int baudrate, unsigned int databits,
                                      unsigned int stopbits, unsigned int parity,
                                      fTransComCallBack cbTransCom, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDevControl().CreateTransComChannel(
										lLoginID,TransComType,baudrate,databits,stopbits,
										parity,cbTransCom,dwUser);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SendTransComData(LONG lTransComChannel, char *pBuffer, DWORD dwBufSize)
{
	int r = g_Manager.GetDevControl().SendTransComData(lTransComChannel,pBuffer,dwBufSize);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_DestroyTransComChannel(LONG lTransComChannel)
{
	int r = g_Manager.GetDevControl().DestroyTransComChannel(lTransComChannel);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_Reset(LONG lLoginID, BOOL bReset)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevControl().Reset(lLoginID,bReset);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_StartUpgrade(LONG lLoginID, char *pchFileName, fUpgradeCallBack cbUpgrade, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDevControl().StartUpgrade(lLoginID,pchFileName,cbUpgrade,dwUser);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API LONG CALL_METHOD CLIENT_StartUpgradeEx(LONG lLoginID, EM_UPGRADE_TYPE emType, char *pchFileName, fUpgradeCallBack cbUpgrade, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDevControl().StartUpgradeEx(lLoginID, emType, pchFileName, cbUpgrade, dwUser);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SendUpgrade(LONG lUpgradeID)
{
	int r = g_Manager.GetDevControl().SendUpgrade(lUpgradeID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopUpgrade(LONG lUpgradeID)
{
	int r = g_Manager.GetDevControl().StopUpgrade(lUpgradeID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryRecordState(LONG lLoginID, char *pRSBuffer, int maxlen, 
                                        int *nRSBufferlen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryRecordState(lLoginID,pRSBuffer,maxlen,nRSBufferlen,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetupRecordState(LONG lLoginID, char *pRSBuffer, int nRSBufferlen)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().SetupRecordState(lLoginID,pRSBuffer,nRSBufferlen);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryDeviceTime(LONG lLoginID, LPNET_TIME pDeviceTime, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().QueryDeviceTime(lLoginID,pDeviceTime,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetupDeviceTime(LONG lLoginID, LPNET_TIME pDeviceTime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().SetupDeviceTime(lLoginID,pDeviceTime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryComProtocol(LONG lLoginID, int nProtocolType, char *pProtocolBuffer, 
                                        int maxlen, int *nProtocollen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().QueryComProtocol(lLoginID,nProtocolType,pProtocolBuffer,maxlen,nProtocollen,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QuerySystemInfo(LONG lLoginID, int nSystemType, 
												   char *pSysInfoBuffer, int maxlen, int *nSysInfolen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().GetDevFunctionInfo(lLoginID,nSystemType,pSysInfoBuffer,maxlen,nSysInfolen,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryUserInfo(LONG lLoginID, USER_MANAGE_INFO *info, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryUserInfo(lLoginID, info, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryUserInfoEx(LONG lLoginID, USER_MANAGE_INFO_EX *info, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	int r = g_Manager.GetDevConfig().QueryUserInfoEx(lLoginID, info, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
	
}

/*
 *	2.602.0000.0版本以后接口：
 *	功能：对16位用户名的操作。
 */
CLIENT_API BOOL CALL_METHOD CLIENT_OperateUserInfoEx(LONG lLoginID, int nOperateType, void *opParam, void *subParam, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	int r = g_Manager.GetDevConfig().OperateUserInfoEx(lLoginID, nOperateType, opParam, subParam, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;

}

CLIENT_API BOOL CALL_METHOD CLIENT_OperateUserInfo(LONG lLoginID, int nOperateType, void *opParam, void *subParam, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().OperateUserInfo(lLoginID, nOperateType, opParam, subParam, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StartTalk(LONG lRealHandle, BOOL bCustomSend)
{
	//Begin: Modify by li_deming(11517) 2008-1-14
	/*
	int r = g_Manager.GetRealPlay().StartTalk(lRealHandle,bCustomSend);
		if (r < 0)
		{
			g_Manager.SetLastError(r);
		}*/
	int r =-1;
	g_Manager.SetLastError(NET_ERROR);//该接口己不使用,请使用CLIENT_StartTalkEx接口
	//End:li_deming(11517)
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopTalk(LONG lRealHandle)
{
	//Begin: Modify by li_deming(11517) 2008-1-14
	/*
	int r = g_Manager.GetRealPlay().StopTalk(lRealHandle);
		if (r < 0)
		{
			g_Manager.SetLastError(r);
		}
	
	return r<0?FALSE:TRUE;
	*/
	return CLIENT_StopTalkEx(lRealHandle);
	//End:li_deming(11517)
}

CLIENT_API BOOL CALL_METHOD CLIENT_SendTalkData_Custom(LONG lRealHandle, char *pBuffer, DWORD dwBufSize)
{
	return FALSE;
}

CLIENT_API LONG CALL_METHOD CLIENT_GetStatiscFlux(LONG lLoginID, LONG lPlayHandle)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetStatiscFlux(lLoginID,lPlayHandle);
	if (lRet < 0)
	{
		g_Manager.SetLastError(lRet);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}


CLIENT_API BOOL CALL_METHOD CLIENT_QueryIOControlState(LONG lLoginID, IO_CTRL_TYPE emType, 
                                           void *pState, int maxlen, int *nIOCount, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryIOControlState(lLoginID,emType,pState,maxlen,nIOCount,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_IOControl(LONG lLoginID, IO_CTRL_TYPE emType, void *pState, int maxlen)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().IOControl(lLoginID,emType,pState,maxlen);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_GetDEVWorkState(LONG lLoginID, LPNET_DEV_WORKSTATE lpWorkState, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().GetDEVWorkState(lLoginID,lpWorkState,waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_MakeKeyFrame(LONG lLoginID, int nChannelID, int nSubChannel)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevControl().MakeKeyFrame(lLoginID,nChannelID,nSubChannel);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_SetMaxFlux(LONG lLoginID, WORD wFlux)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevControl().SetMaxFlux(lLoginID,wFlux);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


/************************************************************************
 ** 开始查找录像文件：返回值 0 失败， >0 查找句柄
 ***********************************************************************/
CLIENT_API LONG	CALL_METHOD CLIENT_FindFile(LONG lLoginID,
								int nChannelId,
								int nRecordFileType, 
								char* cardid, 
								LPNET_TIME time_start,
								LPNET_TIME time_end,
								BOOL bTime, 
								int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetPlayBack().FindFile(lLoginID,nChannelId,nRecordFileType,
									cardid,time_start,time_end,bTime,waittime);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
 }


/************************************************************************
 ** 查找录像文件：返回值 -1 参数出错，0 录像文件信息数据取完，1 取回一条录像文件信息
 ***********************************************************************/
CLIENT_API int	CALL_METHOD CLIENT_FindNextFile(LONG lFindHandle,LPNET_RECORDFILE_INFO lpFindData)
{
	int nRet = g_Manager.GetPlayBack().FindNextFile(lFindHandle,lpFindData);

	return nRet;
}


/************************************************************************
 ** 结束录像文件查找：返回值 TRUE 成功， FALSE 失败
 ***********************************************************************/
CLIENT_API BOOL CALL_METHOD CLIENT_FindClose(LONG lFindHandle)
{
	int r = g_Manager.GetPlayBack().FindClose(lFindHandle);

	return r<0?FALSE:TRUE;
}

/************************************************************************
 ** 重启设备：返回值 TRUE 成功， FALSE 失败
 ***********************************************************************/
CLIENT_API BOOL CALL_METHOD CLIENT_RebootDev(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevControl().RebootDev(lLoginID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


/************************************************************************
 ** 关闭设备：返回值 TRUE 成功， FALSE 失败
 ***********************************************************************/
CLIENT_API BOOL CALL_METHOD CLIENT_ShutDownDev(LONG lLoginID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevControl().ShutDownDev(lLoginID);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_GetDVRIPByResolveSvr(char *pchDVRIP, WORD wDVRPort, 
                                            BYTE *sDVRName, WORD wDVRNameLen, 
                                            BYTE *sDVRSerialNumber, WORD wDVRSerialLen, 
                                            char* sGetIP)
{
	g_Manager.GetCS().Lock();

	BOOL b = g_Manager.GetDVRIPByResolveSvr(pchDVRIP, wDVRPort, sDVRName, wDVRNameLen,
							sDVRSerialNumber, wDVRSerialLen, sGetIP);

	g_Manager.GetCS().UnLock();
	return b;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetPlayerBufNumber(LONG lRealHandle, DWORD dwBufNum)
{
    return FALSE;
}

CLIENT_API LONG CALL_METHOD CLIENT_PlayBackByTime(LONG lLoginID, int nChannelID, 
                                      LPNET_TIME lpStartTime, LPNET_TIME lpStopTime, HWND hWnd , fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	LONG lRet =  g_Manager.GetPlayBack().PlayBackByTime(lLoginID, nChannelID, lpStartTime, lpStopTime, cbDownLoadPos, dwPosUser, hWnd);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API LONG CALL_METHOD CLIENT_PlayBackByTimeEx(LONG lLoginID, int nChannelID, 
												  LPNET_TIME lpStartTime, LPNET_TIME lpStopTime, HWND hWnd, fDownLoadPosCallBack cbDownLoadPos, DWORD dwPosUser,
												  fDataCallBack fDownLoadDataCallBack, DWORD dwDataUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	LONG ret =  g_Manager.GetPlayBack().PlayBackByTimeEx(lLoginID, nChannelID, lpStartTime, lpStopTime, cbDownLoadPos, dwPosUser, hWnd, fDownLoadDataCallBack, dwDataUser);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return ret;
}

CLIENT_API BOOL CALL_METHOD CLIENT_GetFileByTime(LONG lLoginID, int nChannelID, 
                                     LPNET_TIME lpStartTime, LPNET_TIME lpStopTime, char *sSavedFileName)
{
    return FALSE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_PlayBackControl(LONG lPlayHandle, 
                                       DWORD dwControlCode, DWORD dwInValue, DWORD *lpOutValue)
{
	return FALSE;
}

CLIENT_API BOOL  CALL_METHOD CLIENT_GetDevConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned,int waittime/*=500*/)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = -1;

	if (dwCommand >= DEV_IPFILTER_CFG)
	{
		if ( DEV_POS_CFG == dwCommand )
		{
			r = g_Manager.GetDevConfig().GetDevConfig(lLoginID,dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned, waittime);
		}
		else
		{
			r = g_Manager.GetDevConfigEx().GetDevNewConfig(lLoginID,dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned, waittime);
		}
	}
	else
	{
		r = g_Manager.GetDevConfig().GetDevConfig(lLoginID,dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned, waittime);
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;

}

CLIENT_API BOOL  CALL_METHOD CLIENT_SetDevConfig(LONG lLoginID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer,DWORD dwInBufferSize,int waittime/*=500*/)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = -1;

	if (dwCommand >= DEV_IPFILTER_CFG)
	{
		if ( DEV_POS_CFG == dwCommand )
		{
			r = g_Manager.GetDevConfig().SetDevConfig(lLoginID,dwCommand,lChannel,lpInBuffer,dwInBufferSize, waittime);
		}
		else
		{
			r = g_Manager.GetDevConfigEx().SetDevNewConfig(lLoginID,dwCommand,lChannel,lpInBuffer,dwInBufferSize, waittime);
		}
	}
	else
	{
		 r = g_Manager.GetDevConfig().SetDevConfig(lLoginID,dwCommand,lChannel,lpInBuffer,dwInBufferSize, waittime);
	}

	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL  CALL_METHOD CLIENT_SetupChannelName(LONG lLoginID,char *pbuf, int nbuflen)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r =  g_Manager.GetDevConfig().SetupChannelName(lLoginID, pbuf, nbuflen);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API DWORD CALL_METHOD CLIENT_GetLastError(void)
{
	g_Manager.GetCS().Lock();
	
	DWORD dwRet = g_Manager.GetLastError();
	
	g_Manager.GetCS().UnLock();
	return dwRet;
}


CLIENT_API BOOL CALL_METHOD CLIENT_ControlDevice(LONG lLoginID, CtrlType type, void *param, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevControl().ControlDevice(lLoginID, type, param, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL CALL_METHOD CLIENT_QueryDevState(LONG lLoginID, int nType, char *pBuf, int nBufLen, int *pRetLen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().QueryDevState(lLoginID, nType, pBuf, nBufLen, pRetLen, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_ListenServer(char* ip, WORD port, int nTimeout, fServiceCallBack cbListen, DWORD dwUserData)
{
	g_Manager.GetCS().Lock();
	
	LONG lRet = g_Manager.ListenServer(ip, port, cbListen, dwUserData);
	
	g_Manager.GetCS().UnLock();
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_StopListenServer(LONG lServerHandle)
{
	g_Manager.GetCS().Lock();
	
	BOOL bRet = g_Manager.StopListenServer(lServerHandle);
	
	g_Manager.GetCS().UnLock();
	return bRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_ResponseDevReg(char *devSerial, char* ip, WORD port, BOOL bAccept)
{
	g_Manager.GetCS().Lock();
	
	BOOL bRet = g_Manager.ResponseDevReg(devSerial, ip, port, bAccept);
	
	g_Manager.GetCS().UnLock();
	return bRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetupChannelOsdString(LONG lLoginID, int nChannelNo, CHANNEL_OSDSTRING* struOsdString, int nbuflen)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().SetupChannelOsdString(lLoginID, nChannelNo, struOsdString, nbuflen);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

/*************************************************************************
**设置抓图回调
**************************************************************************/
CLIENT_API void CALL_METHOD CLIENT_SetSnapRevCallBack(fSnapRev OnSnapRevMessage, DWORD dwUser)
{
	g_Manager.GetCS().Lock();
   
	g_Manager.GetSnapPicture().SetSnapRevCallBack(OnSnapRevMessage,dwUser);

	g_Manager.GetCS().UnLock();
}

/*****************抓图请求******************/
CLIENT_API BOOL CALL_METHOD CLIENT_SnapPicture(LONG lLoginID, SNAP_PARAMS par)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	bool bret = g_Manager.GetSnapPicture().SnapPictureQuery(lLoginID,par);
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return bret;
}

/*****************设置GPS订阅回调函数**********/
CLIENT_API void CALL_METHOD CLIENT_SetSubcribeGPSCallBack(fGPSRev OnGPSMessage,
                                              DWORD dwUser)
{
	g_Manager.GetCS().Lock();
   
	g_Manager.GetGPSSubcrible().SetGpsRevCallBack(OnGPSMessage,dwUser);

	g_Manager.GetCS().UnLock();
} 

/******************GPS信息订阅*****************/
CLIENT_API BOOL CALL_METHOD CLIENT_SubcribeGPS (LONG lLoginID,        //设备句柄
									BOOL bStart,		  //是否启动
                                    LONG KeepTime,        //订阅持续时间（单位秒）
                                    LONG InterTime)      //订阅时间内GPS发送频率（单位秒）
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
    
	bool bret = g_Manager.GetGPSSubcrible().SendGpsSubcrible(lLoginID, bStart, KeepTime,InterTime);
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return bret;
}


CLIENT_API BOOL  CALL_METHOD CLIENT_GetPlatFormInfo(LONG lLoginID, DWORD dwCommand, int nSubCommand, int nParam, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned,int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().GetPlatFormInfo(lLoginID, dwCommand, nSubCommand, nParam, lpOutBuffer, dwOutBufferSize, lpBytesReturned, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL  CALL_METHOD CLIENT_SetPlatFormInfo(LONG lLoginID, DWORD dwCommand, int nSubCommand, int nParam, LPVOID lpInBuffer, DWORD dwInBufferSize, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDevConfig().SetPlatFormInfo(lLoginID, dwCommand, nSubCommand, nParam, lpInBuffer, dwInBufferSize, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API BOOL   CALL_METHOD CLIENT_GetPtzOptAttr(LONG lLoginID,DWORD dwProtocolIndex,LPVOID lpOutBuffer,DWORD dwBufLen,DWORD *lpBytesReturned,int waittime)
{
	if(g_Manager.IsDeviceValid((afk_device_s *)lLoginID,1) <0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		*lpBytesReturned = 0;
		return FALSE;
	}
	
	if (dwBufLen < sizeof(PTZ_OPT_ATTR))
	{
		return NET_ILLEGAL_PARAM;
	}
	int r = g_Manager.GetDevConfig().GetDevConfig_PtzCfg(lLoginID,(PTZ_OPT_ATTR *)lpOutBuffer,dwProtocolIndex,waittime);
	if(r < 0)
	{	
		*lpBytesReturned =0;
		g_Manager.SetLastError(r);
	}
	else
	{
		*lpBytesReturned = sizeof(PTZ_OPT_ATTR);
	}
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API LONG CALL_METHOD CLIENT_ExportConfigFile(LONG lLoginID, CONFIG_FILE_TYPE emConfigFileType, char *szSavedFilePath, 
                                      fDownLoadPosCallBack cbDownLoadPos, DWORD dwUserData)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDevConfigEx().ExportConfigFile(lLoginID, emConfigFileType, szSavedFilePath, cbDownLoadPos, dwUserData);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}


CLIENT_API BOOL CALL_METHOD CLIENT_StopExportCfgFile(LONG lExportHandle)
{
	int ret = g_Manager.GetDevConfigEx().StopExportCfgFile(lExportHandle);
	if (ret < 0)
	{
		g_Manager.SetLastError(ret);
	}

	return ret<0?FALSE:TRUE;
}


CLIENT_API LONG CALL_METHOD CLIENT_ImportConfigFile(LONG lLoginID, char *szFileName, fDownLoadPosCallBack cbUploadPos, DWORD dwUserData, DWORD param)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDevConfigEx().ImportConfigFile(lLoginID, szFileName, cbUploadPos, dwUserData);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}


CLIENT_API BOOL CALL_METHOD CLIENT_StopImportCfgFile(LONG lImportHandle)
{
	int r = g_Manager.GetDevConfigEx().StopImportCfgFile(lImportHandle);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SearchDevices(char* szBuf, int nBufLen, int* pRetLen, DWORD dwSearchTime)
{
	int r = g_Manager.GetDevConfigEx().SearchDevice(szBuf, nBufLen, pRetLen, dwSearchTime);
	if(r < 0)
	{
		g_Manager.SetLastError(r);
	}

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryDecoderInfo(LONG lLoginID, LPDEV_DECODER_INFO lpDecInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().QueryDecoderInfo(lLoginID, lpDecInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryDecoderTVInfo(LONG lLoginID, int nMonitorID, LPDEV_DECODER_TV lpMonitorInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().QueryDecoderTVInfo(lLoginID, nMonitorID, lpMonitorInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryDecEncoderInfo(LONG lLoginID, int nEncoderID, LPDEV_ENCODER_INFO lpEncoderInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().QueryDecEncoderInfo(lLoginID, nEncoderID, lpEncoderInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetDecTVOutEnable(LONG lLoginID, BYTE *pDecTVOutEnable, int nBufLen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().SetDecoderTVEnable(lLoginID, pDecTVOutEnable, nBufLen, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetOperateCallBack(LONG lLoginID, fMessDataCallBack cbMessData, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().SetOperateCallBack(lLoginID, cbMessData, dwUser);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_CtrlDecTVScreen(LONG lLoginID, int nMonitorID, BOOL bEnable, int nSplitType, BYTE *pEncoderChannel, int nBufLen, void* userdata)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDecoderDevice().CtrlDecTVScreen(lLoginID, nMonitorID, bEnable, nSplitType, pEncoderChannel, nBufLen, userdata);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API LONG CALL_METHOD CLIENT_SwitchDecTVEncoder(LONG lLoginID, int nEncoderID, LPDEV_ENCODER_INFO lpEncoderInfo, void* userdata)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDecoderDevice().SwitchDecTVEncoder(lLoginID, nEncoderID, lpEncoderInfo, userdata);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API int CALL_METHOD CLIENT_AddTourCombin(LONG lLoginID, int nMonitorID, int nSplitType, BYTE *pEncoderChannnel, int nBufLen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int nRet = g_Manager.GetDecoderDevice().AddTourCombin(lLoginID, nMonitorID, nSplitType, pEncoderChannnel, nBufLen, waittime);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);

	return nRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_DelTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().DelTourCombin(lLoginID, nMonitorID, nCombinID, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, int nSplitType, BYTE *pEncoderChannel, int nBufLen, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().SetTourCombin(lLoginID, nMonitorID, nCombinID, nSplitType, pEncoderChannel, nBufLen, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, LPDEC_COMBIN_INFO lpDecCombinInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().QueryTourCombin(lLoginID, nMonitorID, nCombinID, lpDecCombinInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetDecoderTour(LONG lLoginID, int nMonitorID, LPDEC_TOUR_COMBIN lpDecTourInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().SetDecoderTour(lLoginID, nMonitorID, lpDecTourInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryDecoderTour(LONG lLoginID, int nMonitorID, LPDEC_TOUR_COMBIN lpDecTourInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().QueryDecoderTour(lLoginID, nMonitorID, lpDecTourInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryDecChannelFlux(LONG lLoginID, int nEncoderID, LPDEV_DECCHANNEL_STATE lpChannelStateInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetDecoderDevice().QuerChannelFlux(lLoginID, nEncoderID, lpChannelStateInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
	
}

CLIENT_API BOOL CALL_METHOD CLIENT_SetDecPlaybackPos(LONG lLoginID, fDecPlayBackPosCallBack cbPlaybackPos, DWORD dwUser)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().SetDecPlaybackPos(lLoginID, cbPlaybackPos, dwUser);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API LONG CALL_METHOD CLIENT_DecTVPlayback(LONG lLoginID, int nEncoderID, DEC_PLAYBACK_MODE emPlaybackMode, LPVOID lpInBuffer, DWORD dwInBufferSize, void* userdata)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetDecoderDevice().DecTVPlayback(lLoginID, nEncoderID, emPlaybackMode, lpInBuffer, dwInBufferSize, userdata);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;
}

CLIENT_API BOOL CALL_METHOD CLIENT_CtrlDecPlayback(LONG lLoginID, int nEncoderID, DEC_CTRL_PLAYBACK_TYPE emCtrlType, int nValue, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDecoderDevice().CtrlDecPlayback(lLoginID, nEncoderID, emCtrlType, nValue, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}


CLIENT_API LONG CALL_METHOD CLIENT_ControlConnectServer(LONG lLoginID, char* RegServerIP, WORD RegServerPort, int TimeOut)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG lRet = g_Manager.GetAutoRegister().ConnectRegServer(lLoginID, RegServerIP, RegServerPort, TimeOut);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return lRet;

}

CLIENT_API BOOL CALL_METHOD CLIENT_ControlRegisterServer(LONG lLoginID, LONG ConnectionID, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetAutoRegister().ControlRegister(lLoginID, ConnectionID, waittime);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);

	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL CALL_METHOD CLIENT_ControlDisconnectRegServer(LONG lLoginID, LONG ConnectionID)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetAutoRegister().DisConnectRegServer(lLoginID, ConnectionID);

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	
	return r<0?FALSE:TRUE;
	
}

CLIENT_API BOOL CALL_METHOD CLIENT_QueryControlRegServerInfo(LONG lLoginID, LPDEV_SERVER_AUTOREGISTER lpRegServerInfo, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetAutoRegister().QueryRegServerInfo(lLoginID, lpRegServerInfo, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	
	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;	
}

CLIENT_API LONG CALL_METHOD CLIENT_FileTransmit(LONG lLoginID, int nTransType, char* szInBuf, int nInBufLen, fTransFileCallBack cbTransFile, DWORD dwUserData, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	LONG nRet = g_Manager.GetDevConfigEx().SetDevNewConfig_FileTrans(lLoginID, nTransType, szInBuf, nInBufLen, cbTransFile, dwUserData, waittime);
	if (nRet < 0)
	{
		g_Manager.SetLastError(nRet);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return nRet;
}

CLIENT_API BOOL   CALL_METHOD CLIENT_QueryTransComParams(LONG lLoginID, int TransComType, COMM_STATE* pCommState, int nWaitTime)
{
	if(g_Manager.IsDeviceValid((afk_device_s *)lLoginID,1) <0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetDevConfig().QueryTransComParams(lLoginID, TransComType, pCommState, nWaitTime);
	if(r < 0)
	{	
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL   CALL_METHOD CLIENT_CtrlDecoderTour(LONG lLoginID, int nMonitorID, DEC_CTRL_TOUR_TYPE emActionParam, int waittime)
{
	if (g_Manager.IsDeviceValid((afk_device_s*)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	int r = g_Manager.GetDecoderDevice().CtrlDecTour(lLoginID, nMonitorID, emActionParam, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}
	g_Manager.EndDeviceUse((afk_device_s*)lLoginID);
	return r<0?FALSE:TRUE;
}

CLIENT_API BOOL  CALL_METHOD CLIENT_GetNewDevConfig(LONG lLoginID, char* szCommand, int nChannelID, char* szOutBuffer, DWORD dwOutBufferSize, int *error, int waittime/*=500*/)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}

	int r = g_Manager.GetNewDevConfig().GetDevConfig(lLoginID, szCommand, nChannelID, szOutBuffer, dwOutBufferSize, error, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;

}

CLIENT_API BOOL  CALL_METHOD CLIENT_SetNewDevConfig(LONG lLoginID, char* szCommand, int nChannelID, char* szInBuffer, DWORD dwInBufferSize, int *error, int *restart, int waittime/*=500*/)
{
	if (g_Manager.IsDeviceValid((afk_device_s *)lLoginID, 1) < 0)
	{
		g_Manager.SetLastError(NET_INVALID_HANDLE);
		return FALSE;
	}
	
	int r = g_Manager.GetNewDevConfig().SetDevConfig(lLoginID, szCommand, nChannelID, szInBuffer, dwInBufferSize, error, restart, waittime);
	if (r < 0)
	{
		g_Manager.SetLastError(r);
	}

	g_Manager.EndDeviceUse((afk_device_s *)lLoginID);
	return r<0?FALSE:TRUE;
}

