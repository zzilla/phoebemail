#include "ddoverlayrender.h"
#include <multimon.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>
#include "../utils.h"
#include <vector>
extern std::vector<HWNDHMONITOR> g_GUID_List;

typedef int (CALLBACK *FARPROC)();
HMONITOR (WINAPI* g_pfnMonitorFromWindow)(HWND, DWORD) = NULL;
HMONITOR (WINAPI* g_pfnMonitorFromPoint)(POINT, DWORD) = NULL;
int      (WINAPI* g_pfnGetSystemMetrics)(int) = NULL;

CritSec g_Overlaycrs;

DDPIXELFORMAT ddpfOverlayPixelFormats[] = 
{
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0}, // UYVY
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},  // YUY2
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','V','1','2'),0,0,0,0,0},  // YV12	
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','V','U','9'),0,0,0,0,0},  // YVU9
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('I','F','0','9'),0,0,0,0,0},  // IF09
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0x00FF0000,0x0000FF00,0x000000FF, 0} //RGB32
};

color_convert_func overlayccfunc[] = 
{
	yuv2uyvy16_mmx,
	yuv2yuyv16_mmx,	
	yuv2yv12,	
	0
};

DDOverlayRender::DDOverlayRender(DWORD ck)
: m_pDD(0), m_pDDSPrimary(0), m_pDDSVideoSurface(0)
, m_pDDSVideoSurfaceBack(0)
{
	m_index		= -1;
	m_hWnd		= 0;
	m_width		= 352;
	m_height	= 288;
	m_colorKey	= ck;
	m_callback	= 0;
	
	m_hasOverlaySupport	= FALSE;
	m_hasFourCCSupport	= FALSE;

	m_colorConvert = NULL;
	m_updataOverlay = false;
	m_MonitorBeginPosX = 0;
	m_MonitorBeginPosY = 0;
}

DDOverlayRender::~DDOverlayRender()
{
	clean();
}

void DDOverlayRender::GetRenderRect(HWND hWnd, RECT &rcRect)
{
	GetClientRect(hWnd, &rcRect);
// 	if (rcRect.left<0 && rcRect.top<0 && rcRect.right<0 && rcRect.bottom<0)
// 	{
// 		return ; 
// 	}

	LONG lWidth = rcRect.right - rcRect.left;
	LONG lHeight = rcRect.bottom - rcRect.top;
	
	POINT lPoint;
	lPoint.x = rcRect.left;
	lPoint.y = rcRect.top;
	ClientToScreen(m_hWnd, &lPoint);
	
	rcRect.left	= lPoint.x;
	rcRect.top	= lPoint.y;
	rcRect.right	= lPoint.x + lWidth;
	rcRect.bottom	= lPoint.y + lHeight;
}

int DDOverlayRender::init(int index, HWND hWnd, int width, int height, draw_callback cb)
{
	m_index		= index;
	m_hWnd		= hWnd;
	m_width		= width;
	m_height	= height;
	m_callback	= cb;

	int ret = 0;
	__try
	{
		ret = initDirectDraw();
	}
	__except(0,1)
	{
		ret = 9;//DH_PLAY_CREATE_DDRAW_ERROR
	}
	
	resize();

	return ret;
}

void DDOverlayRender::resize()
{
	// 获取窗口大小
//	RECT rect;
//	GetWindowRect(m_hWnd, &rect);

	RECT rect;
	GetRenderRect(m_hWnd, rect);

	rect.left -= m_MonitorBeginPosX;
	rect.right -= m_MonitorBeginPosX;
	rect.top -= m_MonitorBeginPosY;
	rect.bottom -= m_MonitorBeginPosY;

	if (rect.right != m_destRect.right 
		|| rect.bottom != m_destRect.bottom 
		|| rect.top != m_destRect.top 
		|| rect.left != m_destRect.left)
	{
		m_destRect.left		= rect.left;
		m_destRect.top		= rect.top;
		m_destRect.right	= rect.right;
		m_destRect.bottom	= rect.bottom;
	
		m_updataOverlay = false;
	}
}

int DDOverlayRender::clean()
{
	int ret = 0;
	__try
	{
		ret = destroyDDObjects();
	}
	__except(0,1)
	{
		ret = -1;
	}

	m_updataOverlay = false;

	return ret;
}


#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

/************************************************************************
 * 初始化DirectDraw的一般步骤
 * 1. 创建DirectDraw对象，COM接口为IID_IDirectDraw7。
 * 2. 设置协作级别，协作级别如果为DDSCL_FULLSCREEN则还要调用SetDisplayMode()
 * 3. 创建主表面
 * 4. 创建后台主绘图表面（OFFSCREEN或者OVERLAY表面）
 * 5. 获取后台主绘图表面的附加翻转表面（可以多个）
 * 6. 如果是窗口模式，那么这里要设置裁剪区域
************************************************************************/
int DDOverlayRender::initDirectDraw()
{
	int err = 0;

	destroyDDObjects();

/*	if (FAILED(DirectDrawCreateEx(NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL)))
	{
		err = -1;
		goto err_return;
	}
*/
	HMODULE hUser32;
    hUser32 = GetModuleHandle(TEXT("USER32"));
    if (hUser32)
	{
		*(FARPROC*)&g_pfnMonitorFromWindow   = GetProcAddress(hUser32,"MonitorFromWindow");
		*(FARPROC*)&g_pfnMonitorFromPoint    = GetProcAddress(hUser32,"MonitorFromPoint");
		*(FARPROC*)&g_pfnGetSystemMetrics    = GetProcAddress(hUser32,"GetSystemMetrics");
	}	

	HMONITOR hMonitor = g_pfnMonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

//	RECT rc;
//	GetWindowRect(m_hWnd, &rc);
	RECT rc;
	GetRenderRect(m_hWnd, rc);
	int ScreenWidth = g_pfnGetSystemMetrics(SM_CXSCREEN);
	int ScreenHeight = g_pfnGetSystemMetrics(SM_CYSCREEN);

	m_MonitorBeginPosX = 0;
	m_MonitorBeginPosY = 0;
	HWNDHMONITOR* hwndmonitor;

	int i=0;
	for(i = 0; i < g_GUID_List.size(); i++)
	{
		hwndmonitor = &g_GUID_List[i];
		if (hMonitor == hwndmonitor->hMonitor)
		{
			POINT pt;
			pt.y = rc.top;
			int j = rc.left;
			if (j  < 0)
			{
				for (; j <= 1280*11; j++)
				{
					pt.x = j;
					HMONITOR ptMonitor = g_pfnMonitorFromPoint(pt,MONITOR_DEFAULTTONULL);
					if (ptMonitor != hMonitor)
					{
						m_MonitorBeginPosX = j-ScreenWidth;
						break;
					}
				}
			}
			else
			{
				for (; j >0; j--)
				{
					pt.x = j;
					HMONITOR ptMonitor = g_pfnMonitorFromPoint(pt,MONITOR_DEFAULTTONULL);
					if (ptMonitor != hMonitor)
					{
						m_MonitorBeginPosX = j+1;
						break;
					}
				}
			}

			pt.x = rc.left;
			int k= rc.top;
			if (k < 0)
			{
				for (; k <=1024*11; k++)
				{
					pt.y = k;
					HMONITOR ptMonitor = g_pfnMonitorFromPoint(pt,MONITOR_DEFAULTTONULL);
					if (ptMonitor != hMonitor)
					{
						m_MonitorBeginPosY = k-ScreenHeight;
						break;
					}
				}		
			}
			else
			{
				for (; k >0; k--)
				{
					pt.y = k;
					HMONITOR ptMonitor = g_pfnMonitorFromPoint(pt,MONITOR_DEFAULTTONULL);
					if (ptMonitor != hMonitor)
					{
						m_MonitorBeginPosY = k+1;
						break;
					}
				}		
			}
		
			break;
		}
	}

	if (FAILED(DirectDrawCreateEx(i == 0 ? NULL : &(g_GUID_List[i].guid), (VOID**)&m_pDD, IID_IDirectDraw7, NULL)))
	{
		err = 9; //DH_PLAY_CREATE_DDRAW_ERROR
		goto err_return;
	}
		
// 	if (FAILED(m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_SETFOCUSWINDOW))) 
// 	{
// 		err = -2;
// 		goto err_return;
// 	}

	if (FAILED(m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL))) 
	{
		err = 9; //DH_PLAY_CREATE_DDRAW_ERROR
		goto err_return;
	}


	// 如果实现全屏显示，那么此处要设置显示模式，如：
	// hr = m_pDD->SetDisplayMode(640,480,8,0,0);

	// 创建主表面，填充表面描述结构体
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));

	ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	//创建主表面    
	if ( FAILED(m_pDD->CreateSurface(&ddsd, &m_pDDSPrimary, NULL))) 
	{
		err = 9; //DH_PLAY_CREATE_DDRAW_ERROR
		goto err_return;
	}

	m_pDD->GetDisplayMode(&ddsd);

	m_screenWidth = ddsd.dwWidth;
	m_screenHeight = ddsd.dwHeight;
	
	m_hasOverlaySupport = hasOverLaySupport();

    // 创建绘图表面
	if (FAILED(createDrawSurface())) 
	{
		err = 9; //DH_PLAY_CREATE_DDRAW_ERROR
		goto err_return;
	}

err_return:
	return err;
}

HRESULT DDOverlayRender::createDrawSurface()
{
	DDSURFACEDESC2 ddsd;
	DDSCAPS2       ddscaps;
	HRESULT		   hr;

	SAFE_RELEASE(m_pDDSVideoSurface); 

	// 创建主绘图表面,可以是离屏表面或者是Overlay表面
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	// 根据执行的类型创建表面
	// 创建Overlay表面
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps    = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = 1;
	ddsd.dwWidth           = m_width;
	ddsd.dwHeight          = m_height;

	int i = 0;
	while (i<3)
	{
		ddsd.ddpfPixelFormat   = ddpfOverlayPixelFormats[i];
		hr = m_pDD->CreateSurface(&ddsd, &m_pDDSVideoSurface, NULL);
		if (FAILED(hr)) 
		{
			i++;
		}
		else 
		{
			break;
		}
	}

	if (i<3) 
	{
		m_colorConvert = overlayccfunc[i];
	} 
	else 
	{
		return hr;
	}

	ZeroMemory(&ddscaps, sizeof(ddscaps));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	
	if (FAILED(hr = m_pDDSVideoSurface->GetAttachedSurface(&ddscaps, &m_pDDSVideoSurfaceBack)))
	{
		return hr;
	}	

	// 如果是Overlay表面则设置Overlay显示属性
	ZeroMemory(&m_OverlayFX, sizeof(m_OverlayFX) );
	m_OverlayFX.dwSize = sizeof(m_OverlayFX);
		
	m_dwOverlayFlags = DDOVER_SHOW;

	if (m_ddCaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY) 
	{
		// 指定覆盖关键色
		DWORD dwDDSColor = convertGDIColor(m_colorKey);

		m_OverlayFX.dckDestColorkey.dwColorSpaceLowValue  = dwDDSColor;
		m_OverlayFX.dckDestColorkey.dwColorSpaceHighValue = dwDDSColor;
			
		m_dwOverlayFlags |= DDOVER_DDFX  | DDOVER_KEYDESTOVERRIDE ;
			
		ZeroMemory(&m_ddbltfx, sizeof(m_ddbltfx));
		m_ddbltfx.dwSize = sizeof(m_ddbltfx);
		m_ddbltfx.dwFillColor = dwDDSColor;
	}

	LPDIRECTDRAWCLIPPER pClipper = NULL;

	if(FAILED(hr = m_pDD->CreateClipper(0, &pClipper, NULL))) 
	{
		return hr;
	}
	if(FAILED(hr = pClipper->SetHWnd(0, m_hWnd)))
	{
		return hr;
	}

	if(FAILED( hr = m_pDDSPrimary->SetClipper(pClipper))) 
	{
		return hr;
	}

	SAFE_RELEASE(pClipper);

	dbg_print("Initialize DirectDraw Ok.\n");

	return S_OK;
}

DWORD DDOverlayRender::convertGDIColor( COLORREF dwGDIColor )
{
	if (m_pDDSPrimary == NULL) 
	{
		return 0x00000000;
	}

	COLORREF       rgbT;
	HDC            hdc;
	DWORD          dw = CLR_INVALID;
	DDSURFACEDESC2 ddsd;
	HRESULT        hr;

	if (dwGDIColor != CLR_INVALID && m_pDDSPrimary->GetDC(&hdc) == DD_OK)
	{
		rgbT = GetPixel(hdc, 0, 0);
		SetPixel(hdc, 0, 0, dwGDIColor);
		m_pDDSPrimary->ReleaseDC(hdc);
	}

	ddsd.dwSize = sizeof(ddsd);
	hr = m_pDDSPrimary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if (hr == DD_OK) 
	{
		dw = *(DWORD *) ddsd.lpSurface; 
		if( ddsd.ddpfPixelFormat.dwRGBBitCount < 32 )
		dw &= ( 1 << ddsd.ddpfPixelFormat.dwRGBBitCount ) - 1;  
		m_pDDSPrimary->Unlock(NULL);
	}

	if (dwGDIColor != CLR_INVALID && m_pDDSPrimary->GetDC(&hdc) == DD_OK)
	{
		SetPixel( hdc, 0, 0, rgbT );
		m_pDDSPrimary->ReleaseDC(hdc);
	}

	return dw;    
}

HRESULT DDOverlayRender::destroyDDObjects()
{
	SAFE_RELEASE(m_pDDSVideoSurfaceBack);
	SAFE_RELEASE(m_pDDSVideoSurface);
	SAFE_RELEASE(m_pDDSPrimary);

	if (m_pDD) 
	{
		m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
	}

	SAFE_RELEASE( m_pDD );

	return S_OK;
}

BOOL DDOverlayRender::hasOverLaySupport()
{
	ZeroMemory(&m_ddCaps, sizeof(m_ddCaps));
	m_ddCaps.dwSize = sizeof(m_ddCaps);
	m_pDD->GetCaps(&m_ddCaps, NULL);

	if( m_ddCaps.dwCaps & DDCAPS_OVERLAY ) 
	{
		if (m_ddCaps.dwCaps & DDCAPS_OVERLAYSTRETCH) 
		{
			return TRUE;
		} 
		else
		{
			return FALSE;
		}
	} 
	else 
	{
		return FALSE;    
	}
}

BOOL DDOverlayRender::hasFourCCSupport(LPDIRECTDRAWSURFACE7 lpdds)
{
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	lpdds->GetSurfaceDesc(&ddsd);

	if (ddsd.ddpfPixelFormat.dwFlags == DDPF_FOURCC)
		return TRUE;

	return FALSE;
}

DWORD DDOverlayRender::colorMatch(IDirectDrawSurface7 * pdds, COLORREF rgb)
{
    COLORREF                rgbTemp;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    DDSURFACEDESC2          ddsd;
    HRESULT                 hr;
    
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK) {
        rgbTemp = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        pdds->ReleaseDC(hdc);
    }
    
    ddsd.dwSize = sizeof(ddsd);
    while ((hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING) {
		
    }
	    
    if (hr == DD_OK) {
        dw = *(DWORD *) ddsd.lpSurface;
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32) {
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;
		}

        pdds->Unlock(NULL);
    }
    
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbTemp);
        pdds->ReleaseDC(hdc);
    }
    
    return dw;
}

int DDOverlayRender::render(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height,RECT*rc)
{
	AutoLock lock(&g_Overlaycrs);
	
	HRESULT hr = 0;
	
	if (py == 0 || pu == 0 || pv == 0) 
	{
		goto user_draw;
	}
	
	if ((width != m_width)||(height != m_height)) 
	{
		dbg_print("Render Video (Resize).");

		clean();
		
		m_width = width;
		m_height = height;

		int ret = init(m_index, m_hWnd, m_width, m_height, m_callback);
		if ( ret > 0) 
		{
			dbg_print("init error.\n");
			return ret;
		}
	}

//	RECT rect;
//	GetWindowRect(m_hWnd, &rect);
	RECT rect;
	GetRenderRect(m_hWnd, rect);
	// 多显卡模式下，坐标可能为负值
// 	if (rect.left < 0 && rect.right < 0 && rect.top < 0 && rect.bottom < 0)
// 	{
// 		return 0;
// 	}

	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	// 复制YUV数据到缓冲区
	hr = m_pDDSVideoSurfaceBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT, NULL);

	if(hr == DDERR_SURFACELOST)
	{
		hr = m_pDDSPrimary->Restore();
		m_pDDSVideoSurface->Restore();
		m_pDDSVideoSurfaceBack->Restore();
		m_updataOverlay = false ;

		if (hr == DDERR_WRONGMODE)
		{
			clean();

			int ret = init(m_index, m_hWnd, m_width, m_height, m_callback);
			if ( ret > 0) 
			{
				return ret;
			}
			return 0;
		}
	}

	if (ddsd.lpSurface == 0)
	{
#ifdef _DEBUG
		char str[120];
		sprintf(str, "m_pDDSVideoSurface->Lock error 0x%x!!!!!!!!!\n", hr);
		OutputDebugString(str);
#endif
		return 24; //DH_PLAY_VIDEOSURFACE_LOCK_ERROR
	}

	m_colorConvert(py, pu, pv, (unsigned char *)ddsd.lpSurface, ddsd.lPitch, ddsd.dwWidth, ddsd.dwHeight);
	
	hr = m_pDDSVideoSurfaceBack->Unlock(NULL) ;	
	// 绘图表面翻转
	hr = m_pDDSVideoSurface->Flip(NULL, DDFLIP_WAIT) ;
	
	rcSrc.left = 0;
	rcSrc.top = 0;
	rcSrc.right = m_width;
	rcSrc.bottom = m_height;
	
	resize();

	if ((m_destRect.right-m_destRect.left <= 1) || (m_destRect.bottom-m_destRect.top <= 1) ||
		(m_destRect.left<=0 && m_destRect.top<=0 && m_destRect.right<=0 && m_destRect.bottom<=0))
	{
		return 0;
	}
	
	 adjustRect(rcSrc, m_destRect);

	//hr = m_pDDSPrimary->Blt(NULL,NULL,NULL, DDBLT_WAIT | DDBLT_COLORFILL,&m_ddbltfx);
	if(!m_updataOverlay)
	{
		hr = m_pDDSVideoSurface->UpdateOverlay(&rcSrc, m_pDDSPrimary, &m_destRect, m_dwOverlayFlags, &m_OverlayFX);
		if (hr != DD_OK)
		{
			return 23; //DH_PLAY_UPDATE_ERROR
		}
		m_updataOverlay = true;
	}

	if (hr == DDERR_SURFACELOST)
	{
		m_pDDSPrimary->Restore();
		m_pDDSVideoSurface->Restore();
		m_pDDSVideoSurfaceBack->Restore();
		m_updataOverlay = false ;
	}
	
user_draw: 
	
	if (m_callback) 
	{
		HDC hDC = GetDC(m_hWnd);
		
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		
		m_callback(m_index, hDC);
		
		ReleaseDC(m_hWnd, hDC);
	}

	return hr;
}

void DDOverlayRender::adjustRect(RECT &rcSrc, RECT &rcDest)
{
	// 窗口在屏幕左边超出.
	// 源RECT的Left要发生变化

	int sdx = rcSrc.right - rcSrc.left; // 源的宽.
	int sdy = rcSrc.bottom - rcSrc.top; // 源的高.

	int ddx = rcDest.right - rcDest.left; // 目标的宽
	int ddy = rcDest.bottom - rcDest.top; // 目标的高

	if(rcDest.left < 0) {
		rcSrc.left = (-rcDest.left) * sdx / ddx  ;	
		rcDest.left = 0;
	}
    
	if(rcDest.top < 0) {
		rcSrc.top = -rcDest.top ;
		rcDest.top =0;
	}
	
	if(rcDest.right >m_screenWidth) {
		rcSrc.right = rcSrc.right - ((rcDest.right - m_screenWidth) * sdy / ddy);
		if(rcSrc.right < 30)
			rcSrc.right = 30;
		rcDest.right = m_screenWidth;
	}
	
	if(rcDest.bottom > m_screenHeight) {
		rcSrc.bottom = rcSrc.bottom - ((rcDest.bottom - m_screenHeight) * sdx / ddx);
		rcDest.bottom = m_screenHeight;
	}
}



















