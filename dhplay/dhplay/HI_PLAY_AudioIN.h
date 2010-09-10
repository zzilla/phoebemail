/************************************************************


  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.


*************************************************************


  File Name     : HI_PLAY_AudioIN.h

  Version       : V1.0 

  Author        : Hisilicon multimedia software group 

  Created       : 53171 

  Last Modified : 2006-4-29

  Description   : VSC - Audio In

  Function List : 

  History       :

************************************************************/


#if !defined(AFX_HI_PLAY_AUDIOIN_H__7C96B411_7A43_4235_A5D0_7BFCCA58D92B__INCLUDED_)
#define AFX_HI_PLAY_AUDIOIN_H__7C96B411_7A43_4235_A5D0_7BFCCA58D92B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <windows.h>
#include <mmsystem.h>
#define PLAY_AUDIO_BUF_NUM		10
#define BUFFERSIZE 4096
/*#include "dhplay.h"*/

typedef void (WINAPI *audioCallFunction)(LPBYTE pDataBuffer, DWORD DataLength, long nUser);

class CHI_PLAY_AudioIn
{
	friend void CALLBACK AudioInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	CHI_PLAY_AudioIn(void);
	virtual ~CHI_PLAY_AudioIn(void);

	BOOL Start(audioCallFunction audiocbFun, int BitsPerSample, int SamplesPerSec, int nLength, long nReserved, LONG nUser); 
	void Stop(void);
	BOOL IsOpen(void);

	BOOL IsExistDevice(void);
	WAVEINCAPS * GetDeviceCap(void);
	
public:
	audioCallFunction m_audiocbFun;
	LONG m_nUser;

	int m_nBitsPerSample;

protected:
	
private:
	BOOL OpenDevice(int BitsPerSample, int SamplesPerSec);
	void CloseDevice(void);
    BOOL PerpareBuffer(int BufferLen);
    void UnperpareBuffer(void);
	BOOL StartData(void);
	void CloseData(void);
	void FreeBuffer(void);
	
	BOOL		m_bDevOpen;
	BOOL		m_bStartData;
	BOOL		m_bBuffer;

	WAVEINCAPS 	m_waveCaps;
	HWAVEIN		m_hWaveIn;
	WAVEHDR		*m_pWaveHead;
	MMRESULT	m_mmr;
	
	CRITICAL_SECTION m_waveLock;
	BOOL        m_bWaveResetting;


	//ADD 自定义的一个缓冲
	//////////////////////////////////////////////////////////////////////////
	char m_audiobuffer[PLAY_AUDIO_BUF_NUM][BUFFERSIZE];
	//////////////////////////////////////////////////////////////////////////
};

#endif // !defined(AFX_HI_PLAY_AUDIOIN_H__7C96B411_7A43_4235_A5D0_7BFCCA58D92B__INCLUDED_)
