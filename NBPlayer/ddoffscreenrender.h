#ifndef DDRENDER_H
#define DDRENDER_H

#include "videorender.h"
#include "colorspace.h"
#include "AX_Mutex.h"

#include "winsock2i.h"
#include <ddraw.h>
#include <stdio.h>
#include <map>
using namespace std;

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

private:
	// 初始化DirectDraw
	int initDirectDraw();

	// 创建绘图表面
	HRESULT createDrawSurface();

	// 内部清理
	HRESULT destroyDDObjects();

	// GDI绘制部分
	inline void gdi_colorConvert(unsigned char *py, unsigned char *pu, 
								unsigned char *pv, int width, int height);

	inline void gdi_YUV_TO_RGB24(unsigned char *puc_y, int stride_y, 
								unsigned char *puc_u, unsigned char *puc_v, int stride_uv, 
								unsigned char *puc_out, int width_y, int height_y,int stride_out);

	int gdi_render(unsigned char *py, unsigned char *pu, unsigned char *pv, 
								int width, int height,RECT*rc);

private: // DirectDraw 相关
	// DirectDraw对象
	LPDIRECTDRAW7        m_pDD;						// DirectDraw对象
	LPDIRECTDRAWSURFACE7 m_pDDSPrimary;				// 主表面
	LPDIRECTDRAWSURFACE7 m_pDDSVideoSurface;		// 绘图表面

	color_convert_func m_colorConvert;

	HWND m_hWnd;
	HMONITOR m_hMonitor;
	RECT m_rcWnd;
	RECT m_rcScreen;
	SIZE m_siDraw;

	int m_nTestMonitor;

	struct RGBPlane 
	{
		RGBPlane();
		~RGBPlane();

		unsigned char * data;
		int w, h;

		void resize(int width,  int height);
		unsigned char * getLine(int y);
	}  m_rgb;

//静态变量
protected:
	static AX_Mutex m_mtxMonitor;
	static HMONITOR m_hMainMonitor;
	static map<HMONITOR, GUID> m_mapMonitor;

	static bool GetMonitorGUID(HMONITOR hMonitor, GUID& guid, bool& bMainMonitor);
public:
	static void UpdateMonitorEnum();
};

#endif /* DDRENDER_H */
