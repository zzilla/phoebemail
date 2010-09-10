#ifndef AUDIORENDER_H
#define AUDIORENDER_H

/*#include "manager.h"*/

struct AudioRender
{
	virtual int init(int channels, int samplePerSec, int bitPerSample, HWND hWnd)=0;
	virtual int clean() = 0;
	// 写数据
	virtual int write(unsigned char *pcm, int len) = 0;
	// 跟工作线程相关的方法
	virtual void terminating() = 0; // 中止工作
	virtual bool SetVolume(DWORD wNewVolume) = 0;
	virtual WORD GetVolume() = 0;

	int m_bitsPerSample ;
	int m_samplesPerSecond ;
};

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

inline void dbg_print_ex(int level, const char *msg, ...)
{
#ifdef _DEBUG
	char buf[256];
	
	va_list ap;
	va_start(ap, msg); // use variable arg list
	vsprintf(buf, msg, ap);
	va_end( ap );
	
	OutputDebugString(buf);
#endif
}

#endif // AUDIORNDER_H
