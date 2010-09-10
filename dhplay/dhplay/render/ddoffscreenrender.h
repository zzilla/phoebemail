#ifndef DDRENDER_H
#define DDRENDER_H

#include "manager.h"
#include "videorender.h"
#include "colorspace.h"

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>

// #define USE_OSD_SURFACE
#define LPMYDIRECTDRAWSURFACE	LPDIRECTDRAWSURFACE7

class DDOffscreenRender : public VideoRender
{
public:
	// 构造函数
	DDOffscreenRender();
	// 析构函数
	virtual ~DDOffscreenRender();
	// 接口函数
	virtual int init(int index, HWND hWnd, int width, int height, 
		draw_callback cb);	

	virtual int render(unsigned char *py, unsigned char *pu, unsigned char *pv, 
		int width, int height,RECT*rc);	

	virtual void resize();

	virtual int clean();

	virtual void VerticalSyncEnable(BOOL bEnable) {m_bVerticalSyncEnable = bEnable;};

private:
	// 初始化DirectDraw
	int initDirectDraw();

	// 创建绘图表面
	HRESULT createDrawSurface();

	// 内部清理
	HRESULT destroyDDObjects();

	// 检查是否支持FourCC (四字符代码)
	BOOL hasFourCCSupport(LPDIRECTDRAWSURFACE7 lpdds);

	void adjustRect(RECT &rcSrc, RECT &rcDest);

	void GetRenderRect(HWND hWnd, RECT &rcRect);

private:
	// 给定的参数
	int	m_index;
	int	m_width;	// 要创建的绘图表面的宽度
	int	m_height;	// 要创建的绘图表面的高度
	// 窗口句柄
	HWND	m_hWnd;
	draw_callback m_callback;

	BOOL m_bVerticalSyncEnable;
	CritSec m_csRgbSurfaceCritsec;//对RGB页面要进行保护

private: // DirectDraw 相关
	// DirectDraw对象
	LPDIRECTDRAW7        m_pDD;						// DirectDraw对象
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;				// 主表面
	LPDIRECTDRAWSURFACE7 m_pDDSVideoSurface;		// 绘图表面

	LPMYDIRECTDRAWSURFACE m_pDDRGBSurface;           //用于OSD叠加

	// 绘图能力信息
	DDCAPS  m_ddCaps;

	// FourCC支持标识
	BOOL m_hasFourCCSupport;

	color_convert_func m_colorConvert;

	RECT m_destRect;
	//屏幕宽和高
	int	m_screenWidth;
	int	m_screenHeight;

	int m_MonitorBeginPosX;
	int m_MonitorBeginPosY;

private:
	bool GetMonitorBeginPos(int* width, int*height);
};

#endif /* DDRENDER_H */
