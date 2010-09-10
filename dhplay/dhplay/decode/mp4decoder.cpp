#include "mp4decoder.h"
#include <string.h>
#include <stdlib.h>
#include "../utils.h"
#include "../playmanage.h"
CritSec g_mp4_critsec;

MP4_Xvid_Init Mpeg4Decoder::m_XvidInit = NULL;
MP4_Xvid_Decore Mpeg4Decoder::m_XvidDecore = NULL;

Mpeg4Decoder::Mpeg4Decoder()
{
	m_width = 352;
	m_height = 288;
	
	m_dataPtr1 = m_yuvData1 + 16;

	dec_param.handle = NULL;
	m_bWaterMark = true;

	m_qp = 0;
	m_qptotal = 0;
	m_count = 0;
	m_hwnd = NULL;
	m_port = 0;
}

Mpeg4Decoder::~Mpeg4Decoder()
{
	clean();
}

int Mpeg4Decoder::init(int port, HWND hwnd)
{
	g_mp4_critsec.Lock();

	m_port = port;
	if (hwnd != NULL)
	{
		m_hwnd = hwnd;
	}

	int err = 0;

	if (!dec_param.handle) 
	{
		XVID_INIT_PARAM enc_init;
		enc_init.cpu_flags = 0;
		xvid_init(NULL, 0, &enc_init, NULL);


		dec_param.width = m_width;
		dec_param.height = m_height;

		__try 
		{
			int qp= 0;
			
			err = xvid_decore(dec_param.handle, XVID_DEC_CREATE, &dec_param, &qp);

		}
		__except(0,1) 
		{
			dec_param.handle = NULL;
			err = -1;
		}

		if (err!=0)
		{
			dec_param.handle = NULL;
			g_mp4_critsec.UnLock();
			return -1;
		}
	}

	__try
	{
		if (err == 0)
		{
			SetCheckWaterMark(0,NULL);
			SetCheckWaterMark(1,NULL);
		}
	}
	__except(0,1)
	{
		int wlj = 0;
	}

	g_mp4_critsec.UnLock();

	return 0;
}

int Mpeg4Decoder::clean()
{
	int err = 0;

	if (dec_param.handle) 
	{
		__try 
		{
			err = xvid_decore(dec_param.handle, XVID_DEC_DESTROY, NULL, NULL);
		}
		__except(1) 
		{
			dec_param.handle = NULL;
			err = -1;
		}

		dec_param.handle = NULL;
		if (err != 0)
		{
			return -1;
		}
	}
	
	return 0;
}

int Mpeg4Decoder::reset()
{
	int ret = clean();
	
	if (ret<0)
	{
		return ret;
	}
		
	return init(m_port, m_hwnd);
}

int Mpeg4Decoder::resize(int w, int h) 
{
	if (m_width != w || m_height != h)
	{
		m_width   = w ;
		m_height = h ;
		
		reset() ;
	}

	return 0 ;
}

int Mpeg4Decoder::decode(unsigned char *src_buf,unsigned int buflen,unsigned char* dest_buf, int param1, int param2)// 解码	
{	
	int ret = 0;

	if (dec_param.handle) 
	{
		dec_frame.general		=	0;
		dec_frame.length		=	buflen;
		dec_frame.bitstream		=	src_buf;
		dec_frame.colorspace	=	XVID_CSP_I420;//XVID_CSP_YV12;
//		dec_frame.width = 0;
//		dec_frame.height = 0;
 		dec_frame.stride		=	m_width;

#if 1
		BOOL bPicQuality = g_cDHPlayManage.m_nQuality[m_port];
		dec_frame.postproc_level=bPicQuality ? 6 : 0;////////////////////////

		if (NULL != m_hwnd)
		{
			RECT rc;
			
			GetWindowRect(m_hwnd, &rc);
			if (((rc.right-rc.left)>m_width) && ((rc.bottom-rc.top) >m_height))
			{
				dec_frame.postproc_level = 6;
			}
		}

		//屏蔽高分辩率做dering deblook导致的问题
		if (m_width > 704 && m_height > 576)
		{
			dec_frame.postproc_level = 0;
		}

#else
		if (m_height==576||m_height==480||m_width==704||m_width==640) 
		{
			dec_frame.postproc_level=2;
		} 
		else 
		{ 
			// CIF可以增加滤波
			dec_frame.postproc_level=2;
		} 
#endif

		dec_frame.bDeInterlace	=	0;
		dec_frame.image			=	dest_buf;

		__try
		{	
			ret = xvid_decore(dec_param.handle, XVID_DEC_DECODE, &dec_frame, NULL);
		}
		__except (0, 1) 
		{
			OutputDebugString(" xvid_decore exception error!\n");
			ret = -1;
		}

		if (ret != 0) 
		{
			ret = -1;
		}
		else
		{
			ret = 1 ;
		}

	} 
	else 
	{
		ret = -1;
	}

	return ret;
}


int Mpeg4Decoder::SetCheckWaterMark(int nStep, int* pInfo)
{
	if (nStep < 0 || nStep > 3)
	{
		return -1;
	}

//	if (m_XvidDecore == 0)
	{
		return -1;
	}

	int ret = 0;

	__try
	{
		//shut down the water mark function
		if (0 == nStep)	{
			m_bWaterMark = false;
			pInfo = NULL;
	
	//		char tmp[100];
	//		sprintf(tmp,"XVID_DEC_WATERMARK dec_param.handle = %d,nStep = %d\n",dec_param.handle,nStep);
	//		fprintf(fp_test,tmp);
		//	fclose(fp_test);

			ret = xvid_decore(dec_param.handle, XVID_DEC_WATERMARK, &nStep, pInfo);
		} 

		//start the water mark function
		else if (1 == nStep) {
			m_bWaterMark = true;
			pInfo = NULL;
	
	//		char tmp[100];
	//		sprintf(tmp,"XVID_DEC_WATERMARK dec_param.handle = %d,nStep = %d\n",dec_param.handle,nStep);
	//		fprintf(fp_test,tmp);
	//		fclose(fp_test);

			ret = m_XvidDecore(dec_param.handle, XVID_DEC_WATERMARK, &nStep, pInfo);
		}

		//Reset the watermark information
		else if (3 == nStep && m_bWaterMark) {
		
	//		char tmp[100];
	//		sprintf(tmp,"XVID_DEC_WATERMARK dec_param.handle = %d,nStep = %d\n",dec_param.handle,nStep);
	//		fprintf(fp_test,tmp);
	//		fclose(fp_test);

			pInfo = NULL;
			ret = xvid_decore(dec_param.handle, XVID_DEC_WATERMARK, &nStep, pInfo);
		}

		//Get the watermark information
		else if (2 == nStep && m_bWaterMark) {

		//	char tmp[100];
		//	sprintf(tmp,"XVID_DEC_WATERMARK dec_param.handle = %d,nStep = %d\n",dec_param.handle,nStep);
		//	fprintf(fp_test,tmp);
		//	fclose(fp_test);

			ret = xvid_decore(dec_param.handle, XVID_DEC_WATERMARK, &nStep, pInfo);
		}
	}
	__except(0,1)
	{
		int wlj = 0;
	}

	return ret;
}


















