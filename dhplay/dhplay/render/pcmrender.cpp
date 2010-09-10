#include "pcmrender.h"

#include <process.h> 
#include <string.h>
#include <stdio.h>

//#define DUMP_PCM

#ifdef DUMP_PCM
FILE *pcmdata = 0;
#endif

/***********************************************************************
 *  构造函数
************************************************************************/
CPcmRender::CPcmRender()
{
	initParam();
#ifdef DUMP_PCM
	pcmdata = fopen("audio_r.pcm", "w+b");
#endif
}

/************************************************************************
 * 析构函数
************************************************************************/
CPcmRender::~CPcmRender()
{
	clean();
	CloseHandle(m_hEventDone);
	CloseHandle(m_mutex);
#ifdef DUMP_PCM
	fclose(pcmdata);
#endif
}

/**********************************************************************
 * 内部使用，初始化参数                                                                     
***********************************************************************/
void CPcmRender::initParam()
{
	m_hWaveOut = NULL;
	m_hEventDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_mutex = CreateMutex(NULL,FALSE,NULL);

	m_bufferIndex = 0;
}

void CPcmRender::setFormat(int channels, int samplePerSec, int bitsPerSample)
{
	// 第一步: 获取waveformat信息
	m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_waveFormat.nChannels = channels;
	m_waveFormat.wBitsPerSample = bitsPerSample;
	m_waveFormat.nSamplesPerSec = samplePerSec;
	m_waveFormat.nBlockAlign = 
		m_waveFormat.nChannels * m_waveFormat.wBitsPerSample / 8;
	m_waveFormat.nAvgBytesPerSec = 
		m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
	m_waveFormat.cbSize = sizeof(m_waveFormat);

	m_bitsPerSample = bitsPerSample ;
	m_samplesPerSecond = samplePerSec ;
}

/***********************************************************************
 * 初始化CPcmRender对象
 *
************************************************************************/
int CPcmRender::init(int channels, int samplePerSec, int bitsPerSample, HWND hWnd)
{
	if (m_hWaveOut != NULL) {
		return 0;// 已经进行了初始化
	}

	setFormat(channels,samplePerSec,bitsPerSample);
	
	MMRESULT ret = waveOutOpen(NULL, WAVE_MAPPER, &m_waveFormat, 
		NULL, NULL, WAVE_FORMAT_QUERY);

	if (MMSYSERR_NOERROR != ret) {
		return -1;
	}
    
	// 第二步: 获取WAVEOUT句柄
	ret = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_waveFormat, 
		(DWORD)m_hEventDone,0,CALLBACK_EVENT);
	
	if (MMSYSERR_NOERROR != ret) {
		return -1;
	}
	
	reset();
	start();
    
    return 0;
}

/************************************************************************
 * 重置缓冲块相关属性，包括“满”、“空”指示以及读写号
************************************************************************/
void CPcmRender::reset()
{
	m_bufferIndex = 0;
}

/***********************************************************************
 *  清理CPcmRender对象：	
 *
 *  1. 首先处理线程相关属性												
 *  2. 再处理WaveOut相关对象
 *                                                                   
 ************************************************************************/
int CPcmRender::clean()
{
	if (m_hWaveOut==NULL) {
		return -2;
	}
	
	Abort();
	
	if (m_hWaveOut != NULL) {
		while (waveOutClose(m_hWaveOut)==WAVERR_STILLPLAYING) {
			waveOutReset(m_hWaveOut);
		}
		m_hWaveOut = NULL;
	}
	
	Abort();
	
	
	
	return 0;
}

BOOL CPcmRender::Abort()
{
	DWORD osError = MMSYSERR_NOERROR;
	
	if (m_hWaveOut != NULL) {
		osError = waveOutReset(m_hWaveOut);
	}
	
	{
	WaitForSingleObject(m_mutex,INFINITE);
	
	if (m_hWaveOut!=NULL) {
		for (unsigned i = 0; i<PCM_BUF_COUNT; i++) {
			while (m_buffer[i].Release()==WAVERR_STILLPLAYING) {
				if (m_hWaveOut!=NULL) {
					waveOutReset(m_hWaveOut);
				}	
			}
		}
	}

	ReleaseMutex(m_mutex);

	m_bufferIndex = 0;
	
	// Signal any threads waiting on this event, they should then check
	// the bufferByteOffset variable for an abort.
	SetEvent(m_hEventDone);
	}
	
	if (osError != MMSYSERR_NOERROR)
		return FALSE;
	
	return TRUE;
}

/***********************************************************************
 * 中止工作线程：
 * 首先发送信号，让线程自己中止。
************************************************************************/
void CPcmRender::terminating()
{
}

/************************************************************************
 * 写数据
************************************************************************/
int CPcmRender::write(unsigned char *pcm, int len)
{
	if (m_hWaveOut==NULL) {
		return -1;
	}

	unsigned char * ptr = pcm;

	WaitForSingleObject(m_mutex,INFINITE);

	DWORD osError = MMSYSERR_NOERROR;

	while (len>0) {
		DWORD flags = m_buffer[m_bufferIndex].header.dwFlags;
		unsigned int uiDataWrittenLen = 0;

		while ((flags&WHDR_DONE)==0) {
			ReleaseMutex(m_mutex);

			if (WaitForSingleObject(m_hEventDone, INFINITE) != WAIT_OBJECT_0) {
				return -1;
			}

			WaitForSingleObject(m_mutex,INFINITE);

			flags = m_buffer[m_bufferIndex].header.dwFlags;
		}
		
		osError = m_buffer[m_bufferIndex].Prepare(m_hWaveOut, len, &uiDataWrittenLen);
		if (osError!=MMSYSERR_NOERROR) {
			break;
		}
		
		memcpy(m_buffer[m_bufferIndex].data, ptr, uiDataWrittenLen);
		len -= uiDataWrittenLen;
		ptr += uiDataWrittenLen;
		
		osError = waveOutWrite(m_hWaveOut, &m_buffer[m_bufferIndex].header, sizeof(WAVEHDR));
		if (osError!= MMSYSERR_NOERROR) {
			break;
		}
		
		m_bufferIndex = (m_bufferIndex+1)%PCM_BUF_COUNT;
	}

	ReleaseMutex(m_mutex);

	return 0;
}

void CPcmRender::loop()
{
}

/************************************************************************
 * 启动线程
************************************************************************/
int CPcmRender::start()
{
	return 0;
}