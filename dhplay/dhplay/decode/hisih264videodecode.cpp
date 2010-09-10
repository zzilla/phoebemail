#include "hisih264videodecode.h"
#include "../utils.h"
#include <windows.h>
#include <stdio.h>

CritSec g_hi264_critsec;
HI264_DecCreate HisiH264VideoDecoder::m_Fun_HIDecCreate = NULL;
HI264_DecDestroy HisiH264VideoDecoder::m_Fun_HIDecDestroy = NULL;
HI264_DecAU HisiH264VideoDecoder::m_Fun_HIDecAU = NULL;

static void CopyYUV(unsigned char *pdst, H264_DEC_FRAME_S *pframe)
{
	unsigned int i;
	
	for (i = 0; i < pframe->uHeight; i++) 
	{
		memcpy(pdst, pframe->pY + i * pframe->uYStride, pframe->uWidth);
		pdst += pframe->uWidth;
	}
	
	for (i = 0; i < pframe->uHeight / 2; i++) 
	{
		memcpy(pdst, pframe->pU + i * pframe->uUVStride, (pframe->uWidth >> 1));
		pdst += (pframe->uWidth >> 1);
	}
	
	for (i = 0; i < pframe->uHeight / 2; i++) 
	{
		memcpy(pdst, pframe->pV + i * pframe->uUVStride, (pframe->uWidth >> 1));
		pdst += (pframe->uWidth >> 1);
	}
}

HisiH264VideoDecoder::HisiH264VideoDecoder():m_hDeinterlace(NULL)
{
	m_decHandle	= 0;
	m_hDeinterlace = 0;
	memset(&m_attr,0,sizeof(H264_DEC_ATTR_S));
	m_width = 352;
	m_height = 288;
}

HisiH264VideoDecoder::~HisiH264VideoDecoder()
{
//    clean();
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

static char cFileName[256+1024];

int HisiH264VideoDecoder::init()
{
	if (1)
	{
		g_hi264_critsec.Lock();
		static int g_initall = 0;
		if (g_initall == 0)
		{
			HMODULE  hModule = GetModuleHandle("playsdk.dll");

			memset(cFileName, 0, 256+1024);
			GetModuleFileName(hModule, cFileName, 256+1024);
			_strlwr( cFileName );        //把所有的大写字符变成小写,防止读写错误
			int pos = GetModulePathLen(cFileName);
			if (pos != -1)
			{
				memset(cFileName+pos, 0, 256+1024-pos-1);
				memcpy(cFileName+pos, "hi_h264dec_w.dll", 16);
			}
			HINSTANCE hInst;
			hInst = LoadLibrary(cFileName);

			m_Fun_HIDecCreate = (HI264_DecCreate)GetProcAddress(hInst, "Hi264DecCreate");
            m_Fun_HIDecDestroy = (HI264_DecDestroy)GetProcAddress(hInst, "Hi264DecDestroy");
			m_Fun_HIDecAU =  (HI264_DecAU)GetProcAddress(hInst, "Hi264DecAU");
			g_initall = 1;
		}
		g_hi264_critsec.UnLock();

		if (m_Fun_HIDecCreate == NULL || m_Fun_HIDecDestroy == NULL || m_Fun_HIDecAU == NULL)
		{
			return -1;
		}
	}
	
	return 0;
}

int HisiH264VideoDecoder::clean()
{
	if (NULL != m_hDeinterlace)
    {
        HI_ReleaseDeinterlace(m_hDeinterlace);//释放deinterlace资源
        m_hDeinterlace = NULL;                //建议每次释放后把句柄置空.
    }

	if ( 0 == m_decHandle ) 
	{
		return 0;
	}
	
 	__try
 	{
 		if (m_Fun_HIDecDestroy)
 		{
 			m_Fun_HIDecDestroy(m_decHandle);
 		}
 	}
 	__except(0,1)
 	{
 	}
  
	//销毁堆栈的时候加锁
	__try
	{
		g_hi264_critsec.Lock();
		if (m_Fun_HIDecDestroy)
		{
			m_Fun_HIDecDestroy(m_decHandle);
			g_hi264_critsec.UnLock();
		}
	}
	__except(0,1)
	{
		g_hi264_critsec.UnLock();
	}

	m_decHandle = 0;
	
    return 0;
}

int HisiH264VideoDecoder::resize(int w, int h)
{
	if (m_width != w || m_height != h)
	{
		m_width   = w ;
		m_height = h ;
		
		reset() ;
	}

	return 0 ;
}

typedef struct ParseContext
{
	HI_U32 FrameStartFound; /* 表示是否从码流缓冲区中找到AU起始地址。0：找到AU起始地址; 1:未找到AU起始地址。*/
	HI_U32 iFrameLength; /* 一幅完整图像对应的码流长度。 */
	HI_U32 PrevFirstMBAddr; /* 记录上一个Slice的起始宏块地址 */
} ParseContext;

#define MOST_BIT_MASK 0x80000000
#define MAX_AU_SIZE 0x80000

static HI_U32 CountPrefixZeros(HI_U32 CodeNum)
{
	HI_U32 ZeroCount,i;
	ZeroCount = 0;

	for(i = 0; i < 32;i++)
	{
		if((CodeNum & MOST_BIT_MASK))
		{
			break;
		}
		else
		{
			ZeroCount++;
			CodeNum = CodeNum<<1;
		}
	}
	return ZeroCount;
}

HI_U32 CheckFirstMbAddr(HI_U8* p)
{
	HI_U32 code, zeros;
	code = (*p++)<<24;
	code |= (*p++)<<16;
	code |= (*p++)<<8;
	code |= *p++;
	zeros = CountPrefixZeros(code);
	code = code << zeros;
	return ((code >> (31 - zeros)) - 1);
}

HI_S32 Hi264DecLoadAU(HI_U8* pStream, HI_U32 iStreamLen, ParseContext *pc)
{
	HI_U32 i;
	HI_U32 FirstMbAddr;
	HI_U8* p;
	HI_U32 state = 0xffffffff;
	if( NULL == pStream || iStreamLen <= 4)
	{
		return -1;
	}
	pc->FrameStartFound = 0;
	pc->PrevFirstMBAddr = 0;
	for( i = 0; i < iStreamLen; i++)
	{
		/*find a I-slice or a P-slice*/
		if( (state & 0xFFFFFF1F) == 0x101 ||
			(state & 0xFFFFFF1F) == 0x105 )
		{
			p = &pStream[i];
			FirstMbAddr = CheckFirstMbAddr(p);
			if( 1 == pc->FrameStartFound )
			{
				if( FirstMbAddr <= pc->PrevFirstMBAddr)
				{
					pc->iFrameLength = i - 4;
					pc->PrevFirstMBAddr = FirstMbAddr;
					state = 0xffffffff;
					return 0;
				}
				else
				{
					pc->PrevFirstMBAddr = FirstMbAddr;
				}
			}
			else
			{
				pc->PrevFirstMBAddr = FirstMbAddr;
				pc->FrameStartFound = 1;
			}
		}
		/*find a sps, pps or au_delimiter*/
		if( (state&0xFFFFFF1F) == 0x107 ||
			(state&0xFFFFFF1F) == 0x108 ||
			(state&0xFFFFFF1F) == 0x109 )
		{
			if(1 == pc->FrameStartFound)
			{
				pc->iFrameLength = i - 4;
				pc->PrevFirstMBAddr = 0;
				state = 0xffffffff;
				return 0;
			}
			else
			{
				pc->FrameStartFound = 1;
				pc->PrevFirstMBAddr = 0;
			}
		}
		state = (state << 8) | pStream[i];
		if( MAX_AU_SIZE - 1 <= i )
		{
			pc->iFrameLength = i - 3;
			return 0;
		}
	}

	pc->iFrameLength = i;

	return -1;
}

int HisiH264VideoDecoder::decode(unsigned char *buf, unsigned int buflen,unsigned char* dest_buf, int param1, int param2)
{
	if (m_Fun_HIDecAU == 0)
	{
		return -1;
	}

	if (m_decHandle == 0)
	{		
		m_attr.uBufNum        = 4;     // reference frames number: 16    
		m_attr.uPicHeightInMB = m_height/16;     // D1(720x576)
		m_attr.uPicWidthInMB  = m_width / 16;
		m_attr.pUserData      = NULL;   // no user data
		m_attr.uStreamInType  = 0x00;   // bitstream begin with "00 00 01" or "00 00 00 01"
		/* bit0 = 1: H.264 normal output mode; bit0 = 0: direct output mode */
		m_attr.uWorkMode      = 0x00;
		
		if (m_Fun_HIDecCreate)
		{
			m_decHandle = m_Fun_HIDecCreate(&m_attr);		
		}
		else
		{
			m_decHandle = 0;
		}

		if (m_decHandle == 0) 
		{
			return -1;
		}
	}

	H264_DEC_FRAME_S	outframe;
	int iRet;
	int nError = 0;
	DEINTERLACE_FRAME_S struDstFrame;
	
	if (m_height >= 480 && (m_width >= 704 || m_width == 352))
	{
		unsigned char* pBuf = buf;
		int LoadAULen = buflen;
		ParseContext PC;
		int ReturnValue = 0;
		do
		{
			ReturnValue = Hi264DecLoadAU( pBuf, LoadAULen, &PC );
			if ( ReturnValue == 0)
			{
				iRet = m_Fun_HIDecAU(m_decHandle, pBuf, PC.iFrameLength, 0, &outframe, 0);
				pBuf += PC.iFrameLength;
				LoadAULen -= PC.iFrameLength;
			}
			else
			{
				iRet = m_Fun_HIDecAU(m_decHandle, pBuf, LoadAULen, 0, &outframe, 0);
			}

			if ( HI_H264DEC_OK == iRet)
			{
				if ( NULL == m_hDeinterlace )
				{
					//初始化deinterlace.把解码之后的场信息赋入,把Deinterlace之后的图像跨度信息赋入.
					m_struPara.iFieldWidth  = outframe.uWidth ;
					m_struPara.iFieldHeight = outframe.uHeight ;
					m_struPara.iSrcYPitch   = outframe.uWidth ;
					m_struPara.iSrcUVPitch  = outframe.uWidth/2;
					m_struPara.iDstYPitch   = outframe.uWidth;//outframe.iYStride;     //此例中输出Y分量跨度设为Y分量宽度.
					m_struPara.iDstUVPitch  = outframe.uWidth/2;//outframe.iUVStride; //此例中输出UV分量跨度设为UV分量宽度.
					
					HI_InitDeinterlace(&m_hDeinterlace, m_struPara);  //调用Deinterlace初始化函数.
					
					if (m_hDeinterlace == 0) 
					{
						return -1;
					}
				}
				struDstFrame.pszY = dest_buf;
				struDstFrame.pszU = struDstFrame.pszY + outframe.uWidth * (outframe.uHeight*2);
				struDstFrame.pszV = struDstFrame.pszU + outframe.uWidth * ( outframe.uHeight*2) / 4 ;					
				//调用deinterlace主函数.它完成两场输出一帧图像的功能.
				HI_Deinterlace(
					m_hDeinterlace,                                     //句柄
					struDstFrame,                             //输出帧图像结构体
					outframe.pY,                                      //输入场YUV420的Y分量
					outframe.pU,						//输入场YUV420的U分量
					outframe.pV,					//输入场YUV420的V分量
					(PIC_TYPE_E)outframe.uPicFlag );// 

				nError = 1;
			}//end of if ( HI_H264DEC_OK == iRet)
		} while(ReturnValue == 0);
	}//end of if (m_height >= 480)
	else
	{
		__try
		{
			iRet = m_Fun_HIDecAU(m_decHandle, buf, buflen, 0, &outframe, 0);
			if (iRet == 0)
			{
				CopyYUV(dest_buf,&outframe);
				nError = 1;
			}
			else
			{
				nError = -1;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			nError = -1;
		}
	}

	return nError;
}

int HisiH264VideoDecoder::reset()
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
		m_hDeinterlace = 0;
	}
	return ret;
}





















