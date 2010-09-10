#include "h264videodecode.h"
#include "../utils.h"
#include <windows.h>
#include <stdio.h>

#define MINQP 25
#define MAXQP 31

CritSec g_h264_critsec;

	
H264VideoDecoder::H264VideoDecoder()
{
	m_decHandle	= 0;
	m_width = 0;
	m_height = 0;
	m_preBuffer = 0;
	m_threadId = NULL;
}

H264VideoDecoder::~H264VideoDecoder()
{
}

static int GetModulePathLen(char* cFileName)
{
	int pos = 0;

	for (;pos < 1024; pos++)
	{
		if (memcmp(cFileName+pos, "playsdk.dll", 10) == 0)
		{
			break;
		}
	}

	if (pos == 1024)
	{
		pos = -1;
	}

	return pos;
}

int H264VideoDecoder::init()
{
	m_threadId = NULL;

	if (m_decHandle == 0)
	{
		__try
		{
			g_h264_critsec.Lock();
			m_decHandle = h264_decode_init();
			g_h264_critsec.UnLock();	
		} 
		__except (EXCEPTION_EXECUTE_HANDLER) 
		{
			switch(GetExceptionCode())
			{
				case EXCEPTION_ACCESS_VIOLATION:
					OutputDebugString("Exception: h264_decode_init access violation!\n");

					return -1;
				default:
					OutputDebugString("Exception: h264_decode_init unknown exception!\n");
					return -1;		
			}		
		}
	
		if (m_decHandle == 0) 
		{
			return -1;
		}
	}
	
	return 0;
}

static inline void swapimg2(unsigned char *src, unsigned char *dst, int width, int height, int flag)
{
	int i = 0;
	for (i = 0; i < height; i++)
	{
		memcpy(dst+(2*i+flag)*width, src+i*width, width);
	}

	for (i = 0; i < height/2; i++)
	{
		memcpy(dst+height*2*width + (2*i+flag)*width/2, src+height*width + i*width/2, width/2);
	}

	for (i = 0; i < height/2; i++)
	{
 		memcpy(dst+height*width*5/2 + (2*i+flag)*width/2, src+height*width*5/4 + i*width/2, width/2);
	}
}

int H264VideoDecoder::clean()
{
	if (m_decHandle == 0) 
	{
		return 0;
	}

	__try
	{

		g_h264_critsec.Lock();
		h264_decode_end(m_decHandle);
		m_decHandle = 0;
		m_threadId = NULL;
		g_h264_critsec.UnLock();
	} 
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		switch(GetExceptionCode())
		{
			case EXCEPTION_ACCESS_VIOLATION:
				OutputDebugString("Exception: h264_decode_end access violation!\n");

				return -1;
			default:
				OutputDebugString("Exception: h264_decode_end unknown exception!\n");
				return -1;		
		}		
	}
	
    return 0;
}

int H264VideoDecoder::resize(int w, int h)
{
 	if (m_width != w || m_height != h)
	{
		m_width   = w ;
		m_height = h ;
		
		reset() ;
	}

	return 0 ;
}

int H264VideoDecoder::decode(unsigned char *buf, unsigned int buflen,unsigned char* dest_buf, int param1, int param2)
{
	if (m_decHandle == 0)
	{
		return -1;
	}
	
	int error = 0;

 	int w=352,h=576 ;

	int flag = 0;

	FILE* fp = NULL; 
	
L:
	g_h264_critsec.Lock();
	__try 
	{
		if(m_decHandle){
			DWORD threadId = GetCurrentThreadId();
			if(m_threadId == 0) m_threadId = threadId;//m_threadId在解码库重置后清零
			if(m_threadId != threadId){
				OutputDebugString("thread id not equal.##############################\n");
				error = -1;
			}else{
				error = h264_decode_frame(m_decHandle, buf+error, buflen-error, dest_buf, &w, &h, &m_qp,param1);
			}
		}		
	} 
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		switch(GetExceptionCode())
		{		
			case EXCEPTION_ACCESS_VIOLATION:
			
				OutputDebugString("Exception: h264_decode_frame access violation!\n");
				error = -1;

				break;
			default:
				OutputDebugString("Exception: h264_decode_frame unknown exception!\n");
				error = -1;
				break;			
 		}	
	}
	g_h264_critsec.UnLock();
	
	if (error > 0)
	{
		if (flag == 0)
		{
			if (h == (int)m_height/2 && error < (int)buflen)
			{
				if (m_preBuffer == 0)
				{
					m_preBuffer = new unsigned char[720*576*3/2];
				}
				
				swapimg2(dest_buf, m_preBuffer, m_width, h, flag);
				flag = 1;
				goto L;
			}
		}
		else
		{
			swapimg2(dest_buf, m_preBuffer, m_width, h, flag);
			memcpy(dest_buf, m_preBuffer, m_width*m_height*3/2);
		}
		
	}
	else if (error <0) 
	{
		int wlj = 0;
	}

	return error;
}

int H264VideoDecoder::reset()
{
	int ret = clean();
	if (ret < 0) //当清楚解码器操作出现异常的时候,返回
	{
		return ret; 
	}
		
	ret = init();
	if (ret < 0) 
	{
		m_decHandle = 0;
	}

	return ret;
}





















