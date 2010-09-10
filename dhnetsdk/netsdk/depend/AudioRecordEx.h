#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if !defined(__AUDIORECORDEX_H__)
#define __AUDIORECORDEX_H__

#ifdef AUDIORECORDEX_EXPORTS
#define AUDIORECORDEX_API __declspec(dllexport)
#else
#define AUDIORECORDEX_API __declspec(dllimport)
#endif

//录音动态库_回调函数(Mpeg数据缓冲区,数据长度)
typedef void (WINAPI *pCallFunction)(LPBYTE pDataBuffer, DWORD DataLength, int user);

typedef struct AUDIOFORMAT
{    
	DWORD		dwFrameLen;			//每帧长度
    WORD        wChannels;          // 声道数  --2
    DWORD       dwSamplesPerSec;     // 采样率 --44100
    WORD        wBitsPerSample;     // 每采样比特数 --16
	UINT		nAudioBitrate;		//	码率 --vcd:224K
	UINT		nAudioLayer;			//	层1:1, 层2:2
} AUDIOFORMAT;

/*
AudioBitrate:
          {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448} //层1
          {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384}    //层2
*/
//录音动态库_初始化
AUDIORECORDEX_API int WINAPI InitAudioRecordEx(AUDIOFORMAT tempFormat);

//录音动态库_注册使用
/*
 *	这里的返回值是一个句柄，这个句柄只是为退出使用时调用CloseAudioRecordEx(HANDLE huser)提供参数的。
 */
AUDIORECORDEX_API HANDLE WINAPI OpenAudioRecordEx(pCallFunction pProc, int user);
//录音动态库_退出使用
/*
 *	这里关于huser的值要提醒注意的是如果huser是非NULL的就把huser指定的注册信息消掉，但如果是NULL将把所有以前注册的信息全
 *	消掉。
 */
AUDIORECORDEX_API int WINAPI CloseAudioRecordEx(HANDLE huser);

//语音动态库_关闭
AUDIORECORDEX_API int WINAPI UnInitAudioRecordEx();

#endif //__AUDIORECORDEX_H__

