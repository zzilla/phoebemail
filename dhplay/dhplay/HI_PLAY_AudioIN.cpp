/************************************************************


  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.


*************************************************************


  File Name     : HI_PLAY_AudioIN.cpp

  Version       : V1.0 

  Author        : Hisilicon multimedia software group 

  Created       : 53171 

  Last Modified : 2006-4-29

  Description   : VSC - Audio In 从RB中读取数据

  Function List : 

  History       :

************************************************************/
#include "HI_PLAY_AudioIN.h"
#include <stdio.h>
#define SAFE_DELETEA(p) { if(p != NULL) { delete[] (p);   (p) = NULL; } }   //Delete Arrary
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//DWORD CHI_PLAY_AudioIn::s_dwInstance = 0;
CHI_PLAY_AudioIn::CHI_PLAY_AudioIn(void)
{
	memset(&m_waveCaps,0,sizeof(m_waveCaps));

	m_bDevOpen	 = FALSE;
	m_bStartData = FALSE;
	m_bBuffer	 = FALSE;
	m_hWaveIn	 = 0;

	m_pWaveHead  = NULL;

	m_nUser = 0;
	m_audiocbFun = NULL;
	m_nBitsPerSample = 16;

//	InitializeCriticalSection(m_waveLock);
}

CHI_PLAY_AudioIn::~CHI_PLAY_AudioIn(void)
{
	Stop();//	FreeBuffer();
}

/************************************************ 
* Function Name	     : AudioInProc
* Description	     : Audio Callback Function 
* Return Type        : void CALLBACK 
* Parameters         : HWAVEIN hwi
* Parameters         : UINT uMsg
* Parameters         : DWORD dwInstance
* Parameters         : DWORD dwParam1
* Parameters         : DWORD dwParam2
* Last Modified      : 2006-4-30 9:17:15
************************************************/
void CALLBACK AudioInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch ( uMsg ) 
	{
	case MM_WIM_DATA:
		{		
			WAVEHDR *pWaveHdr = (WAVEHDR *)dwParam1;
			CHI_PLAY_AudioIn *pAudioIn = (CHI_PLAY_AudioIn *)(pWaveHdr->dwUser);

			if (pAudioIn->m_bStartData) 
			{
				if (pWaveHdr && hwi) 
				{
					if (pWaveHdr->dwFlags & WHDR_DONE == WHDR_DONE) 
					{
						if (pWaveHdr->dwBytesRecorded > 0) 
						{
//							for (int i = 0; i < (int)pWaveHdr->dwBytesRecorded; i++)
//							{
//								unsigned char c = (unsigned char)pWaveHdr->lpData[i];
//
//								if (c>=0xfe || c<= 0x01) 
//								{
//							 		pWaveHdr->lpData[i] = (char)0x00;
//								}
//							}

							unsigned char* pAudioBuf = (unsigned char*)pWaveHdr->lpData;

							if (pAudioIn->m_nBitsPerSample == 8)
							{
								unsigned short *p16 = (unsigned short *)pAudioBuf;

								pWaveHdr->dwBytesRecorded = pWaveHdr->dwBytesRecorded/2;
								for (int i = 0; i < (int)pWaveHdr->dwBytesRecorded; i++)
								{
									pAudioBuf[i] = BYTE((*p16)>>8) + 128;
									p16++;
								}
							}

							if (pAudioIn->m_audiocbFun)
							{
								pAudioIn->m_audiocbFun(pAudioBuf, 
									pWaveHdr->dwBytesRecorded, pAudioIn->m_nUser);
							}

							//////////////////////////////////////////////////////////////////////////
							waveInPrepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR));
							waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR));				
						}
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

BOOL CHI_PLAY_AudioIn::OpenDevice(int BitsPerSample, int SamplesPerSec)
{
	if ( m_bDevOpen )
	{
		return FALSE;
	}
	
	//////////////////////////////////////////////////////////////////////////
	WAVEFORMATEX m_Format;
	m_Format.wFormatTag		= WAVE_FORMAT_PCM;
	m_Format.wBitsPerSample = BitsPerSample;
    m_Format.nSamplesPerSec = SamplesPerSec;
	m_Format.nChannels		= 1;
	m_Format.cbSize			= 0;

	m_Format.nBlockAlign = 
		m_Format.nChannels * m_Format.wBitsPerSample / 8;
	m_Format.nAvgBytesPerSec = 
		m_Format.nSamplesPerSec * m_Format.nBlockAlign;
	// m_waveFormat.cbSize = sizeof(m_waveFormat);
	//////////////////////////////////////////////////////////////////////////
	m_mmr = waveInOpen(0, WAVE_MAPPER, &m_Format, 0, 0, WAVE_FORMAT_QUERY);
	if ( MMSYSERR_NOERROR != m_mmr )
	{
		return FALSE;
	}
	
	m_mmr = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_Format, (DWORD)AudioInProc, 0, CALLBACK_FUNCTION);
	if( MMSYSERR_NOERROR != m_mmr )
	{
		return FALSE;
	}
	
	m_bDevOpen = TRUE;

	return TRUE;
}

void CHI_PLAY_AudioIn::CloseDevice(void)
{
	if (!m_bDevOpen)
	{
		return;
	}
	
	if(!m_hWaveIn)
	{
		return;
	}

	m_mmr = waveInClose(m_hWaveIn);
	if( MMSYSERR_NOERROR != m_mmr )
	{
		return;
	}
	
	m_hWaveIn = 0;
	m_bDevOpen = FALSE;
}

BOOL CHI_PLAY_AudioIn::PerpareBuffer(int BufferLen)
{
	if (m_bBuffer)
	{
		return FALSE;
	}
	
	m_mmr = waveInReset(m_hWaveIn);
	if( MMSYSERR_NOERROR != m_mmr )
	{
	/*	TRACE("waveInReset Error\n");*/
		return FALSE;
	}
	
	if( m_pWaveHead == NULL )
	{
		m_pWaveHead = new WAVEHDR[PLAY_AUDIO_BUF_NUM];
		
		for(int i=0; i < PLAY_AUDIO_BUF_NUM; i++ )
		{
			ZeroMemory(&m_pWaveHead[i],sizeof(WAVEHDR));
			
// 			CHI_PLAY_AudioBuffer buff(PLAY_AUDIO_FRAME_SIZE, FALSE);//m_Format.nBlockAlign * m_BufferSize
 			m_pWaveHead[i].lpData = m_audiobuffer[i];
		// 	m_pWaveHead[i].dwBufferLength = BUFFERSIZE;
			m_pWaveHead[i].dwBufferLength = BufferLen;
			m_pWaveHead[i].dwFlags = 0;
			m_pWaveHead[i].dwUser = (DWORD)(void *)this;
			waveInPrepareHeader(m_hWaveIn, &m_pWaveHead[i], sizeof(WAVEHDR));
			waveInAddBuffer(m_hWaveIn, &m_pWaveHead[i], sizeof(WAVEHDR));
		}
	}
	else
	{
		for(int i=0 ; i < PLAY_AUDIO_BUF_NUM ; i++ )
		{
			waveInPrepareHeader(m_hWaveIn, &m_pWaveHead[i], sizeof(WAVEHDR));
			waveInAddBuffer(m_hWaveIn, &m_pWaveHead[i], sizeof(WAVEHDR));
		}
	}
	m_bBuffer = TRUE;
	return TRUE;
}

void CHI_PLAY_AudioIn::UnperpareBuffer(void)
{
	if (!m_bBuffer)
	{
		return;
	}
	
	if( m_pWaveHead == NULL )
	{
		return;
	}
	
	for(int i = 0 ; i < PLAY_AUDIO_BUF_NUM ; i++ )
	{
		m_mmr = waveInUnprepareHeader(m_hWaveIn, &m_pWaveHead[i], sizeof(WAVEHDR));
		if( MMSYSERR_NOERROR != m_mmr )
		{
		/*	TRACE("waveInUnprepareHeader Error\n");*/
			continue;
		}		
	}

	m_bBuffer = FALSE;
}

void CHI_PLAY_AudioIn::FreeBuffer(void)
{
	if( m_pWaveHead == NULL )
	{
		return;
	}
	
	for(int i = 0 ; i < PLAY_AUDIO_BUF_NUM ; i++ )
	{
// 		TRACE("%d\n",i);
	// 	SAFE_DELETEA(m_pWaveHead[i].lpData);
	}
	SAFE_DELETEA(m_pWaveHead);
}

BOOL CHI_PLAY_AudioIn::Start(audioCallFunction audiocbFun, int BitsPerSample, int SamplesPerSec, int nLength, long nReserved, LONG nUser)
{
	BOOL bRet=FALSE;
	m_nBitsPerSample = BitsPerSample;
	if (m_nBitsPerSample == 8)
	{
		m_nBitsPerSample = 16;
	}

	if( !OpenDevice(m_nBitsPerSample, SamplesPerSec) )
	{
		goto Exit;
	}

	m_nBitsPerSample = BitsPerSample;

	if (nLength > BUFFERSIZE || nLength < 320)
	{
		nLength = BUFFERSIZE;
	}

	if ( !PerpareBuffer(nLength) )
	{
		goto Exit1;
	}

	if ( !StartData() ) 
	{
		goto Exit2;
	}
	
	bRet = TRUE;
	m_nUser = nUser;
	m_audiocbFun = audiocbFun;
	goto Exit;

Exit2:
	UnperpareBuffer();
Exit1:
	CloseDevice ();
Exit:

	return bRet;
}

void CHI_PLAY_AudioIn::Stop(void)
{
	CloseData();
	UnperpareBuffer();
	CloseDevice();
	FreeBuffer();
}

BOOL CHI_PLAY_AudioIn::StartData(void)
{
	if (m_bStartData)
	{
		return FALSE;
	}
	
	if(!m_hWaveIn)
	{
		return FALSE;
	}
	
	m_mmr = waveInStart(m_hWaveIn);
	if( MMSYSERR_NOERROR != m_mmr )
	{
// 		TRACE("waveInStart Error\n");
		return FALSE;
	}
	
	m_bStartData = TRUE;
	return TRUE;
}

void CHI_PLAY_AudioIn::CloseData(void)
{
	if ( !m_bStartData )
	{
		return;
	}
	
	if( !m_hWaveIn )
	{
		return;
	}
	
	m_bStartData = FALSE;			 //important	
	m_mmr = waveInReset(m_hWaveIn);  //m_mmr = waveInStop(m_hWaveIn);
	if( MMSYSERR_NOERROR != m_mmr )
	{
// 		TRACE("waveInReset Error\n");
	}
}

BOOL CHI_PLAY_AudioIn::IsExistDevice(void)
{
	if( waveInGetNumDevs() <= 0 )
	{
		return FALSE;
	}
	return TRUE;
}

WAVEINCAPS* CHI_PLAY_AudioIn::GetDeviceCap(void)
{
	m_mmr = waveInGetDevCaps(WAVE_MAPPER,&m_waveCaps,sizeof(m_waveCaps));
	if( MMSYSERR_NOERROR != m_mmr )
	{
		return &m_waveCaps;
	}
	return NULL;
}


BOOL CHI_PLAY_AudioIn::IsOpen(void)
{
	return m_bDevOpen;
}


