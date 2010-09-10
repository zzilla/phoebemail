#ifndef DHMPEG4_DECODER_H
#define DHMPEG4_DECODER_H

#include "interface.h"
#include "../depend/xvid.h"
#include <stdio.h>
#include <windows.h>

typedef int (/*_stdcall*/* MP4_Xvid_Init)(void *handle, int opt, void *param1, void *param2);
typedef int (/*_stdcall*/* MP4_Xvid_Decore)(void *handle, int opt, void *param1, void *param2);

class Mpeg4Decoder : public IDecode
{
public:
	Mpeg4Decoder();
	~Mpeg4Decoder();

	/************************************************************************/
	/* 视频解码接口(初始化，清除，重置，图像大小发生改变，解码，etc			*/
	/************************************************************************/
    int init(int port, HWND hwnd);		// 初始化
    int clean();	// 清除
	int reset();	// 重置
    int decode(unsigned char *src_buf,unsigned int buflen,unsigned char* dest_buf, int param1, int param2) ; // 解码	
	int resize(int w, int h) ; 

	int SetCheckWaterMark(int nStep, int* pInfo);

	int getWidth(){return m_width;}
	int getHeight(){return m_height;}

protected:

	int m_width;
	int m_height;
	HWND m_hwnd;
	
	unsigned char m_yuvData1[16+704*576*3/2];

	unsigned char *m_dataPtr1;

	XVID_DEC_FRAME dec_frame;
	XVID_DEC_PARAM dec_param;

	unsigned int m_qp;
	int m_qptotal;
	unsigned int m_count;

private:
	bool m_bWaterMark;
	int m_port;
	static MP4_Xvid_Init m_XvidInit;
	static MP4_Xvid_Decore m_XvidDecore;
};

#endif // DHMPEG4_DECODER_H
