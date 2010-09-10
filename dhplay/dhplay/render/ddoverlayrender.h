#ifndef DDOVERLAYRENDER_H
#define DDOVERLAYRENDER_H

#include "manager.h"
#include "videorender.h"
#include "colorspace.h"

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>

class DDOverlayRender : public VideoRender
{
public:
	// 构造函数
    DDOverlayRender(DWORD color);

	// 析构函数
	virtual ~DDOverlayRender();

	// 接口函数
	virtual int init(int index, HWND hWnd, int width, int height, draw_callback cb);
	
    virtual int render(unsigned char *py, unsigned char *pu, unsigned char *pv, 
		int width, int height,RECT*rc);

	virtual void resize();

	virtual int clean();

	virtual void VerticalSyncEnable(BOOL bEnable) {};

private:
	// 初始化DirectDraw
    int initDirectDraw();

	// 创建绘图表面
    HRESULT createDrawSurface();

	// 内部清理
    HRESULT destroyDDObjects();

	// 检查是否支持Overlay
    BOOL hasOverLaySupport();

	// 检查是否支持FourCC (四字符代码)
    BOOL hasFourCCSupport(LPDIRECTDRAWSURFACE7 lpdds);
    
	// 颜色操作函数
    DWORD colorMatch(IDirectDrawSurface7 * pdds, COLORREF rgb);
    DWORD convertGDIColor(COLORREF dwGDIColor);
	
	void adjustRect(RECT &rcSrc, RECT &rcDest);
	void GetRenderRect(HWND hWnd, RECT &rcRect);
private:
	// 给定的参数
	int	m_index;

	DWORD	m_colorKey;

	// 窗口句柄
    HWND	m_hWnd;
	
	int	m_width;	// 要创建的绘图表面的宽度
	int	m_height;	// 要创建的绘图表面的高度
	
	draw_callback m_callback;

private: // DirectDraw 相关
    LPDIRECTDRAW7        m_pDD;						// DirectDraw对象
    LPDIRECTDRAWSURFACE7 m_pDDSPrimary;				// 主表面
    LPDIRECTDRAWSURFACE7 m_pDDSVideoSurface;		// 绘图表面
    LPDIRECTDRAWSURFACE7 m_pDDSVideoSurfaceBack;	// 后台绘图表面

	// Overlay方式需要使用的参数
    DDOVERLAYFX m_OverlayFX;
    DWORD m_dwOverlayFlags;
	DDBLTFX	m_ddbltfx;
 
	// 绘图能力信息
    DDCAPS  m_ddCaps;
    
	// Overlay支持标识
    BOOL m_hasOverlaySupport;

	// FourCC支持标识
    BOOL m_hasFourCCSupport;

	color_convert_func m_colorConvert;

	//目的表面在主表面上的位置
	RECT m_destRect;
	RECT rcSrc;

	//屏幕宽和高
	int	m_screenWidth;
	int	m_screenHeight;

	//是否支持Overlay
	bool m_updataOverlay;

	int m_MonitorBeginPosX;
	int m_MonitorBeginPosY;
};

#endif /* DDOVERLAYRENDER_H */
