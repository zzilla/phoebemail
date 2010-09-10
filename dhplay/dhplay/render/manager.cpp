#include "manager.h"
#include "videorender.h"
#include "ddoffscreenrender.h"
#include "ddoverlayrender.h"
#include "gdirender.h"
#include "pcmrender.h"

#include <vector>

DWORD g_dwCurVolume = 0;	//保存当前系统音量

CritSec g_critsec;

std::vector<HWNDHMONITOR> g_GUID_List;

//转换YUV图像
int f_Rotateangle( unsigned char* py,
				  unsigned char* pu,
				  unsigned char* pv,
				  unsigned char* pyout,
				  unsigned char* puout,
				  unsigned char* pvout,
				  int   width,
				  int   height,
				  int   rotateType);

DhRenderManager::DhRenderManager()
{
	m_hWnd = NULL; 
	m_videoRender = 0;
	m_audioRender = 0;

	m_eVideoRenderMode = ByDDOffscreen;
}

DhRenderManager::~DhRenderManager()
{
	close();
}

struct EnumInfo
{
    BOOL    bMultimonSupported;
    HRESULT hr;
};

BOOL WINAPI DDEnumCallbackEx2( GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName,
                              VOID* pContext, HMONITOR hmon )
{
	if (pGuid != NULL)
	{
		HWNDHMONITOR* hwndmonitor = new HWNDHMONITOR;
		hwndmonitor->hMonitor = hmon;
		hwndmonitor->guid = *pGuid;
		g_GUID_List.push_back(*hwndmonitor);
	}
	
    return TRUE; // Keep enumerating
}


HRESULT EnumerateScreens()
{
    HRESULT  hr;
    EnumInfo enumInfo;

    ZeroMemory( &enumInfo, sizeof(enumInfo) );
    enumInfo.bMultimonSupported = TRUE;

    if( FAILED( hr = DirectDrawEnumerateEx( DDEnumCallbackEx2, 
                                            &enumInfo, 
                                            DDENUM_ATTACHEDSECONDARYDEVICES ) ) )
        return hr;

    // If something failed inside the enumeration, be sure to return that HRESULT
    if( FAILED(enumInfo.hr) )
        return enumInfo.hr;

    return S_OK;
}

int DhRenderManager::openSound(int channels, int samplePerSec, int bitsPerSample)
{
	AutoLock lock(&g_critsec);

	if (m_audioRender != 0)
	{
		return 0;
	}
	
	CAudioRender * ar = new CPcmRender;

	if(ar && ar->init(channels, samplePerSec, bitsPerSample, NULL) < 0)
	{
		delete ar;
		ar = NULL;
		return -1 ;
	} 
	else 
	{
		m_audioRender = ar;
	}
	return 0 ;
}

BOOL DhRenderManager::VideoVerticalSyncEnable(BOOL bEnable)
{
	BOOL ret = FALSE;

	if (ByDDOffscreen == m_eVideoRenderMode)
	{
		if (NULL != m_videoRender)
		{
			m_videoRender->VerticalSyncEnable(bEnable);
		}

		ret = TRUE;
	}

	return ret;
}

int DhRenderManager::openVideo(int index, HWND hWnd, int width, int height, draw_callback cb, VideoRenderMethod vrm, DWORD ck)
{
	AutoLock lock(&g_critsec);
	
	static int zgf_i = 0;
	if (zgf_i == 0)
	{
		EnumerateScreens();
		zgf_i = 1;
	}	

	VideoRender *vr = 0;
 
 	if(vrm == ByDDOverlay) //overlay,没有字符叠加功能
	{	
		vr = new DDOverlayRender(ck);		
	} 
	else if(vrm == ByDDOffscreen) 
	{
		vr = new DDOffscreenRender;
	} 
	else 
	{
		vr = new GDIRender;
	}
	
	if (vr == 0)
	{
		return 6; //DH_PLAY_ALLOC_MEMORY_ERROR
	}

	m_eVideoRenderMode = vrm;

	int ret = vr->init(index, hWnd, width, height, cb);
	if (ret > 0)
	{
		delete vr;
		vr = NULL;
		return ret;
	}

	m_videoRender = vr;

	return 0;
}

int DhRenderManager::ResetVideoRender(int index, HWND hWnd, int width, int height, draw_callback cb, VideoRenderMethod vrm, DWORD ck)
{
	AutoLock lock(&g_critsec);
	
	static int zgf_i = 0;
	if (zgf_i == 0)
	{
		EnumerateScreens();
		zgf_i = 1;
	}
	
	if (m_videoRender != 0)
	{
		int ret = m_videoRender->init(index, hWnd, width, height, cb);
		if (ret > 0)
		{
			delete m_videoRender;
			m_videoRender = NULL;
		}
		else
		{
			return 0;
		}
	}
	
	VideoRender *vr = 0;
	
	if(vrm == ByDDOverlay) //overlay,没有字符叠加功能
	{	
		vr = new DDOverlayRender(ck);		
	} 
	else if(vrm == ByDDOffscreen) 
	{
		vr = new DDOffscreenRender;
	} 
	else 
	{
		vr = new GDIRender;
	}
		
	if (vr == 0)
	{
		return 6; //DH_PLAY_ALLOC_MEMORY_ERROR
	}
	
	m_eVideoRenderMode = vrm;
	
	int ret = vr->init(index, hWnd, width, height, cb);
	if (ret > 0)
	{
		delete vr;
		vr = NULL;
		return ret;
	}
	
	m_videoRender = vr;

	return 0;
}

int DhRenderManager::close()
{
	AutoLock lock(&m_renderCritsec);

	if (m_videoRender != 0)
	{
	// 	m_videoRender->clean();
		delete m_videoRender;
		m_videoRender = 0;
	}

	if (m_audioRender!= 0)
	{
		m_audioRender->terminating();
		m_audioRender->clean();
		delete m_audioRender;
		m_audioRender = 0;
	}

	return 0;
}

int DhRenderManager::closeSound() 
{
	if (m_audioRender != 0)
	{
		m_audioRender->terminating();
		m_audioRender->clean();
		delete m_audioRender;
		m_audioRender = 0;
	}

	return 0;
}

int DhRenderManager::renderAudio(unsigned char *pcm, int len,int bitsPerSample, int samplesPerSecond)
{
	if (m_audioRender != 0) 
	{
		if (m_audioRender->m_bitsPerSample != bitsPerSample || m_audioRender->m_samplesPerSecond != samplesPerSecond) 
		{
			m_audioRender->clean();
			if (m_audioRender->init(1, samplesPerSecond, bitsPerSample, NULL) < 0)
			{
				return -1;
			}			
		}
		return m_audioRender->write(pcm, len);
	}

	return -1;
}

int DhRenderManager::renderVideo(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height,int rotateType,RECT*rect)
{
	AutoLock lock(&m_renderCritsec);
	
 	if ( rotateType != 0)
	{	
		
		int temp_width = width;
		int temp_height = height;

		unsigned char* p_y = new unsigned char[height * width];
		unsigned char* p_u = new unsigned char[height * width / 4];
		unsigned char* p_v = new unsigned char[height * width / 4];

		memset(p_y , 0 , height * width);
		memset(p_u , 0 , height *width / 4);
		memset(p_v , 0 , height * width /4);
		f_Rotateangle( py ,pu , pv , p_y , p_u , p_v, width , height , rotateType);
		
		if ( rotateType == 1 || rotateType == 3)
		{
			temp_height = width;
			temp_width = height; 
		}
		
		if (m_videoRender != 0) 
		{		
			int ret = m_videoRender->render(p_y , p_u , p_v, temp_width, temp_height,rect);
			if ( p_y != NULL )
			{
				delete[] p_y;
				p_y = NULL;
			}
			if ( p_u != NULL )
			{
				delete[] p_u;
				p_u = NULL;
			}
			if ( p_v != NULL )
			{
				delete[] p_v;
				p_v = NULL;
			}

			if (ret > 0) 
				return ret;
			else
				return 0;
		}
		
		if ( p_y != NULL )
		{
			delete[] p_y;
			p_y = NULL;
		}
		if ( p_u != NULL )
		{
			delete[] p_u;
			p_u = NULL;
		}
		if ( p_v != NULL )
		{
			delete[] p_v;
			p_v = NULL;
		}

		return 9;
	}

	if (m_videoRender != 0) 
	{
		int ret = m_videoRender->render(py, pu, pv, width, height,rect);
		if (ret > 0)
		{
			return ret;
		}
		else
			return 0;
	}

	return 9;
}


bool SetVolume(HWAVEOUT * phWave, DWORD wNewVolume)
{
	DWORD wOldVolume, wOldLeft, wOldRight;
	DWORD dwLeftVolume, dwRightVolume;
	FLOAT dbTemp;

	if(!phWave)
	{
		return false;
	}
	if(wNewVolume < 0xff)
	{
		wNewVolume = 0xff;
	}

	dwLeftVolume = wNewVolume;
	dwRightVolume = wNewVolume;

	if(::waveOutGetVolume(*phWave, &wOldVolume) == MMSYSERR_NOERROR)
	{
		wOldLeft = wOldVolume>>16;
		wOldRight = wOldVolume&0xffff;
		if(wOldLeft && wOldRight)
		{
			if(wOldLeft >= wOldRight)
			{
				dwLeftVolume = wNewVolume;
				dbTemp = wNewVolume*((FLOAT)wOldRight/(FLOAT)wOldLeft);
				dwRightVolume = (DWORD)dbTemp+0.5 > dbTemp ? (DWORD)dbTemp : (DWORD)dbTemp+1;
			}
			else
			{
				dwRightVolume = wNewVolume;
				dbTemp = wNewVolume*((FLOAT)wOldLeft/(FLOAT)wOldRight);
				dwLeftVolume = (DWORD)dbTemp+0.5 > dbTemp ? (DWORD)dbTemp : (DWORD)dbTemp+1;
			}
		}
		else if(wOldLeft)
		{
			dwRightVolume = 0;
			dwLeftVolume = wNewVolume;
		}
		else if(wOldRight)
		{
			dwRightVolume = wNewVolume;
			dwLeftVolume = 0;
		}
		else
		{
			//bug: 音量设置为0后，无法继续设置
			dwRightVolume = wNewVolume;
			dwLeftVolume = wNewVolume;
		}
	}

	wNewVolume = dwRightVolume|dwLeftVolume<<16;
	if(::waveOutSetVolume( *phWave, wNewVolume) == MMSYSERR_NOERROR )
	{
		return true;
	}

	return false;
}


bool DhRenderManager::setvolume(DWORD lVolume )
{
	bool bRet;
	WAVEFORMATEX waveFormat;
	HWAVEOUT hWaveOut;

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;		// init
	waveFormat.nChannels = 1;
	waveFormat.wBitsPerSample = 8;
	waveFormat.nSamplesPerSec = 8000;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = sizeof(waveFormat);
	
	MMRESULT ret = waveOutOpen(NULL, WAVE_MAPPER, &waveFormat, NULL, NULL, WAVE_FORMAT_QUERY);

	if (MMSYSERR_NOERROR != ret) {
		return false;
	}
	
	if (MMSYSERR_NOERROR != 
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD)0,0,CALLBACK_NULL))
	{
		return false;
	}
	
	bRet = SetVolume(&hWaveOut, lVolume);

	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);

	g_dwCurVolume = lVolume;

	return bRet;
}

DWORD GetVolume(HWAVEOUT * phWave)
{
	DWORD wOldVolume;
	if(!phWave)
	{
		return -1;
	}

	if(::waveOutGetVolume(*phWave, &wOldVolume) == MMSYSERR_NOERROR)
	{
		return wOldVolume;
	}

	return 0;
}

DWORD DhRenderManager::getvolume()
{
	DWORD dwVolumeRet = 0;
	WAVEFORMATEX waveFormat;
	HWAVEOUT hWaveOut;

	if(g_dwCurVolume != 0)
	{
		return g_dwCurVolume;
	}
	else
	{
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;		// init
		waveFormat.nChannels = 1;
		waveFormat.wBitsPerSample = 8;
		waveFormat.nSamplesPerSec = 8000;
		waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		waveFormat.cbSize = sizeof(waveFormat);
		
		MMRESULT ret = waveOutOpen(NULL, WAVE_MAPPER, &waveFormat, NULL, NULL, WAVE_FORMAT_QUERY);
		if (MMSYSERR_NOERROR != ret) {
			return 0;
		}
    
		ret = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, (DWORD)0,0,CALLBACK_NULL);
		
		if (MMSYSERR_NOERROR != ret) {
			return 0;
		}

		dwVolumeRet = (0xffff & GetVolume(&hWaveOut));
		if(dwVolumeRet == 0)
		{
			dwVolumeRet = (0xffff0000 & GetVolume(&hWaveOut)) >> 16;
		}

		waveOutReset(hWaveOut);
		waveOutClose(hWaveOut);

		return dwVolumeRet;
	}
}

int f_RotateBuf( unsigned char* pBuffer,
				unsigned char* pBufferOut,
				int width,
				int height,
				int rotateType)
{	
	if ( pBuffer == NULL || pBufferOut == NULL)
	{
		return -1;
	}
	long index = 0;
	int temp_width = 0;
	int temp_height = 0;
	 
	if ( rotateType == 1) //旋转90
	{			
		for( temp_width = 0 ; temp_width != width ; temp_width++)
		{
			for ( temp_height = height - 1 ; temp_height >= 0 ; temp_height--)		
			{
				*(pBufferOut ++ ) = *(pBuffer + (temp_height * width) + temp_width);
			}
		}
	}
	
	if ( rotateType == 2) //旋转180
	{
		long nBuffLen = 0;
		for ( nBuffLen = width*height - 1 ; nBuffLen >= 0 ; nBuffLen--)
		{
			*(pBufferOut ++) = *( pBuffer + nBuffLen);
		}
		
	}
	
	if ( rotateType == 3) //旋转270
	{
		for ( temp_width = width - 1; temp_width >= 0; temp_width-- )
		{
			for ( temp_height = 0 ; temp_height != height ; temp_height++)
			{
				*(pBufferOut ++) = *( pBuffer + ( temp_height * width ) + temp_width ); 
			}
		}		 
	}
	return 0;
}

int f_Rotateangle( unsigned char* py,
				  unsigned char* pu,
				  unsigned char* pv,
				  unsigned char* pyout,
				  unsigned char* puout,
				  unsigned char* pvout,
				  int   width,
				  int   height,
				  int   rotateType)
{
	if ( py ==NULL || pu == NULL || pv ==NULL || pyout ==NULL || puout == NULL || pvout == NULL)
	{
		return -1;
	}
	
	f_RotateBuf(py , pyout , width , height , rotateType);
	f_RotateBuf(pu , puout , width/2 , height/2 , rotateType);
	f_RotateBuf(pv , pvout , width/2 , height/2 , rotateType);
	
	return 0;	
}





















