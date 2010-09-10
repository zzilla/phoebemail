#ifndef VIDEORENDER_H
#define VIDEORENDER_H

#include "winsock2i.h"

typedef void (__stdcall *draw_callback)(int index, HDC hDc);

struct VideoRender
{
	/*!
	 * hWnd: 关联的窗口句柄
	 * width, height: 一般为视频图像的宽与高
	 * cb: 绘图回调, 主要是统一管理输出.
	 */
	virtual int init(int index, HWND hWnd, int width, int height, draw_callback cb) = 0;
	virtual int clean() = 0;
   	virtual int render(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height,RECT*rc) = 0;
	virtual void resize() = 0;
	virtual ~VideoRender(){} ;
};

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#pragma warning(disable : 4996)

inline void dbg_print(const char *msg, ...)
{
#ifdef _DEBUG
	//char buf[256];
	//
	//va_list ap;
	//va_start(ap, msg); // use variable arg list
	//vsprintf(buf, msg, ap);
	//va_end( ap );
	//
	//OutputDebugString(buf);
#endif
}

#endif // VIDEORENDER_H
