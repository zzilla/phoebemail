// newstream.cpp: implementation of the newstream class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>
/*#include <time.h>*/
#include "psstream.h"
#include "StdMp4Parse.h"

//#define HISI264_PS 2
#define ADI264_PS 1
#define MPEG_PS 0 

PSStream::PSStream(unsigned char* rawBuf):StreamParser(rawBuf)
{
	m_iFrameTempLength = 0;
	m_width = 0;
	m_height = 0;
	m_rate = 0;
	m_bIFrameFirstIndex = false;
	m_bIFrameSecondIndex = false;

	m_ifAnalysed = false;
	m_EncodeType = MPEG_PS;
}

PSStream::~PSStream()
{
}


int PSStream::Reset(int level)
{
	StreamParser::Reset(level);
	m_bIFrameFirstIndex = false;
	m_bIFrameSecondIndex = false;

	return 0;
}

inline
bool PSStream::CheckSign(const unsigned int& Code)
{
	if (Code == 0x01BA || Code == 0x01E0)
	{
		return true ;
	}

	return false ;
}

inline 
int PSStream::AnalyseStreamType(unsigned char *pBuf, unsigned long len)
{
	int i = 0;
	int ret ;	
	
	if (len <= 0 || !pBuf)
	{
		return -1;
	}
	
	while (i < (len-4))
	{
		//mpeg4
		if((*(pBuf+i) == 0x00)&&(*(pBuf+i+1) == 0x00)&&(*(pBuf+i+2) == 0x01)&&(*(pBuf+i+3) == 0xB6))
		{
//			ret = Mpeg4_Analyse(pBuf+i, nSize-i, nWidth,nHeight,framerate);
//			if(ret >= 0)
//			{
				ret = MPEG_PS;
				return ret;
//			}
		}
		
		//ADI h264
		if((*(pBuf+i) == 0x00)&&(*(pBuf+i+1) == 0x00)&&(*(pBuf+i+2) == 0x01)&&(((*(pBuf+i+3))&0x1F) == 0x07)
			|| (*(pBuf+i) == 0x00)&&(*(pBuf+i+1) == 0x00)&&(*(pBuf+i+2) == 0x01)&&(((*(pBuf+i+3))&0x1F) == 0x01))
		{
//			ret = H264_Analyse(pBuf+i, nSize-i, nWidth,nHeight,framerate);
//			if(ret >= 0)
//			{
				ret = ADI264_PS;
				return ret;
//			}
		}
		
		i++;
	}
	
	return -1;
}

inline
bool PSStream::ParseOneFrame()
{
	int irest = rest;
	bool ifIFrame = false;

	unsigned int code = 0xffffffff;
	unsigned char *pBuf = m_bufptr;

	if (m_bIFrameSecondIndex && m_code == 0x01E0)
	{
		m_FrameInfo = m_FrameInfoList.GetFreeNote() ;
		m_frameLen = m_FrameInfo->nFrameLength = (pBuf[0]<<8|pBuf[1]) - (3+pBuf[4]);
		m_FrameInfo->pContent = pBuf+5+pBuf[4];
		m_FrameInfo->pHeader = pBuf;
		m_FrameInfo->nLength = m_FrameInfo->nFrameLength  + 5+pBuf[4];
		m_bufptr = m_FrameInfo->pContent;
		rest -= (5+pBuf[4]);
		m_FrameInfo->nEncodeType = DH_STREAM_MPEG4;
		m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO ;
		m_FrameInfo->nSubType = DH_FRAME_TYPE_VIDEO_I_FRAME;

		return true;
	}

	while (irest--)
	{
		code = code << 8 | *pBuf++;
		if (code == 0x01BB || code == 0x01BC)
		{
			ifIFrame = true;
		}
		else if (code == 0x01E0)
		{
			if (irest >= (5+pBuf[4]))
			{
				m_FrameInfo = m_FrameInfoList.GetFreeNote() ;
				m_FrameInfo->pHeader = m_bufptr - 4 ;
				m_FrameInfo->nFrameLength = m_frameLen = (pBuf[0]<<8|pBuf[1]) - (3+pBuf[4]);
			
				if (m_FrameInfo->nFrameLength == 0xfffc-3-pBuf[4] /*&& (0x0f&pBuf[22]) == 0 && pBuf[22]>=0x10 && pBuf[22]<=0x40*/)
				{
					m_bIFrameFirstIndex = true;
				}
				else
				{
					m_bIFrameFirstIndex = false;
				}

				m_FrameInfo->nLength = m_frameLen + rest - irest + pBuf[4] + 9;
				m_FrameInfo->pContent = m_FrameInfo->pHeader + m_FrameInfo->nLength - m_frameLen;

				m_bufptr = m_FrameInfo->pContent;
				rest -= (m_FrameInfo->pContent - m_FrameInfo->pHeader - 4);
				m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO ;
				m_FrameInfo->nSubType = ifIFrame ? DH_FRAME_TYPE_VIDEO_I_FRAME
					: DH_FRAME_TYPE_VIDEO_P_FRAME;

				if (!m_ifAnalysed) 
				{
					m_EncodeType = AnalyseStreamType(m_FrameInfo->pContent,m_FrameInfo->nFrameLength);
				
					if (m_EncodeType != ADI264_PS && m_EncodeType != MPEG_PS) 
					{
						m_EncodeType = MPEG_PS;
					}

					m_ifAnalysed = true;
				}

#ifdef HISI264_PS
				m_FrameInfo->nParam2 = 2;
				m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_H264;

				if (ifIFrame)
				{
					switch(*(m_FrameInfo->pHeader+80))
					{
					case 3:
						m_FrameInfo->nWidth = 352;
						m_FrameInfo->nHeight = 288;
						break;
					case 4:
						m_FrameInfo->nWidth = 176;
						m_FrameInfo->nHeight = 144;
						break;
					case 0:
						m_FrameInfo->nWidth = 704;
						m_FrameInfo->nHeight = 576;
						break;
					case 1:
						m_FrameInfo->nWidth = 352;
						m_FrameInfo->nHeight = 576;
						break;
					case 2:
						m_FrameInfo->nWidth = 704;
						m_FrameInfo->nHeight = 288;
						break;
					}
				}
			
#endif

//#ifdef ADI264_PS
				if (m_EncodeType == ADI264_PS) 
				{
					m_FrameInfo->nParam2 = 1;
					m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_H264;
					
					if (ifIFrame)
					{
						video_standard_t tVideoStd = (video_standard_t)*(m_FrameInfo->pHeader+83);

						switch(*(m_FrameInfo->pHeader+80))
						{
						case DH_CAPTURE_SIZE_CIF:
							if (VIDEO_STANDARD_PAL == tVideoStd)
							{
								m_FrameInfo->nWidth = 352;
								m_FrameInfo->nHeight = 288;
							}
							else if (VIDEO_STANDARD_NTSC == tVideoStd)
							{
								m_FrameInfo->nWidth = 352;
								m_FrameInfo->nHeight = 240;
							}							
							break;
						case DH_CAPTURE_SIZE_QCIF:
							if (VIDEO_STANDARD_PAL == tVideoStd)
							{
								m_FrameInfo->nWidth = 176;
								m_FrameInfo->nHeight = 144;
							}
							else if (VIDEO_STANDARD_NTSC == tVideoStd)
							{
								m_FrameInfo->nWidth = 176;
								m_FrameInfo->nHeight = 128;
							}
							break;
						case DH_CAPTURE_SIZE_D1:
							if (VIDEO_STANDARD_PAL == tVideoStd)
							{
								m_FrameInfo->nWidth = 704;
								m_FrameInfo->nHeight = 576;
							}
							else if (VIDEO_STANDARD_NTSC == tVideoStd)
							{
								m_FrameInfo->nWidth = 704;
								m_FrameInfo->nHeight = 480;
							}
							break;
						case DH_CAPTURE_SIZE_HD1:
							if (VIDEO_STANDARD_PAL == tVideoStd)
							{
								m_FrameInfo->nWidth = 352;
								m_FrameInfo->nHeight = 576;
							}
							else if (VIDEO_STANDARD_NTSC == tVideoStd)
							{
								m_FrameInfo->nWidth = 352;
								m_FrameInfo->nHeight = 480;
							}
							break;
						case DH_CAPTURE_SIZE_DCIF:
							if (VIDEO_STANDARD_PAL == tVideoStd)
							{
								m_FrameInfo->nWidth = 704;
								m_FrameInfo->nHeight = 288;
							}
							else if (VIDEO_STANDARD_NTSC == tVideoStd)
							{
								m_FrameInfo->nWidth = 704;
								m_FrameInfo->nHeight = 240;
							}
							break;
						case DH_CAPTURE_SIZE_VGA :
							m_FrameInfo->nWidth = 640;
							m_FrameInfo->nHeight = 480;
							break;
						case DH_CAPTURE_SIZE_QVGA :
							m_FrameInfo->nWidth = 320;
							m_FrameInfo->nHeight = 240;
							break;
						case DH_CAPTURE_SIZE_SVCD:
							m_FrameInfo->nWidth = 480;
							m_FrameInfo->nHeight = 480;
							break;
						case DH_CAPTURE_SIZE_SVGA:
							m_FrameInfo->nWidth = 800;
							m_FrameInfo->nHeight = 592;
							break;
						case DH_CAPTURE_SIZE_XVGA:
							m_FrameInfo->nWidth = 1024;
							m_FrameInfo->nHeight = 768;
							break;
						case DH_CAPTURE_SIZE_WXGA:
							m_FrameInfo->nWidth = 1280;
							m_FrameInfo->nHeight = 800;
							break;
						case DH_CAPTURE_SIZE_SXGA:
							m_FrameInfo->nWidth = 1280;
							m_FrameInfo->nHeight = 1024;
							break;
						case DH_CAPTURE_SIZE_WSXGA:
							m_FrameInfo->nWidth = 1600;
							m_FrameInfo->nHeight = 1024;
							break;
						case DH_CAPTURE_SIZE_UXGA:
							m_FrameInfo->nWidth = 1600;
							m_FrameInfo->nHeight = 1200;
							break;
						}
					}
				}
				else if (m_EncodeType == MPEG_PS) /*#ifdef MPEG_PS */	
				{
					m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_MPEG4;
				}
//#endif

				if (ifIFrame)
				{
					m_FrameInfo->nFrameRate = *(m_FrameInfo->pHeader+81);
				}	
				if (m_FrameInfo->nFrameRate <= 0)
				{
					m_FrameInfo->nFrameRate = 25;
				}
				
				m_FrameInfo->nParam1 = 1;
				
				return true;
			}
			
		}
		else if (code == 0x01C0)
		{
			if (irest >= (5+pBuf[4]))
			{
				m_FrameInfo = m_FrameInfoList.GetFreeNote() ;
				m_FrameInfo->pHeader = m_bufptr - 4 ;
				m_FrameInfo->nFrameLength = m_frameLen = pBuf[0]<<8|pBuf[1] - (3+pBuf[4]);
			
				m_FrameInfo->nLength = m_frameLen + rest - irest + pBuf[4] + 9;
				m_FrameInfo->pContent = m_FrameInfo->pHeader + m_FrameInfo->nLength - m_frameLen;
				m_bufptr = m_FrameInfo->pContent;
				rest -= (m_FrameInfo->pContent - m_FrameInfo->pHeader - 4);
				m_FrameInfo->nType = DH_FRAME_TYPE_AUDIO ;

//#ifdef MPEG_PS
				if (m_EncodeType == MPEG_PS) 
				{
					m_FrameInfo->nEncodeType = 21;//MPEG_AUDIO;
				}		
//#endif

//#ifdef ADI264_PS
				else if (m_EncodeType == ADI264_PS) 
				{
					m_FrameInfo->nEncodeType = 14;
					m_FrameInfo->nBitsPerSample = 16;
					m_FrameInfo->nSamplesPerSecond = 8000;
				}
//#endif

#ifdef HISI264_PS
				m_FrameInfo->nEncodeType = 22;
				m_FrameInfo->nBitsPerSample = 16;
				m_FrameInfo->nSamplesPerSecond = 8000;
#else

#endif
				return true;
			}
		}
	}

	return false ;
}

inline int log2bin(unsigned int value) 
{
	int n = 0;
	while (value) {
		value>>=1;
		n++;
	}
	return n;
}

#define MMAX(a,b) ((a)>(b)?(a):(b))

inline
bool PSStream::CheckIfFrameValid()
{
	static const unsigned int PSCODE = 0x000001BA ;
	static const unsigned int PS64K_Code = 0x01E0;
	int i = 4 ;
	m_code = 0 ;

	while (rest >0 && i--)
	{
		m_code = m_code << 8 | *m_bufptr++ ;
		rest-- ;
		if (m_code != (PSCODE >> (i*8)) && m_code != (PS64K_Code >> (i*8)))
		{
			return false ;
		}
	}

	if (m_FrameInfo->nType == DH_FRAME_TYPE_VIDEO && m_bIFrameSecondIndex)
	{
		m_FrameInfo->nWidth = m_width;
		m_FrameInfo->nHeight = m_height;
		m_FrameInfo->nFrameRate = m_rate;
	
		memcpy(m_iFrameTemp+m_iFrameTempLength, m_FrameInfo->pContent, m_FrameInfo->nFrameLength);
	
		//数据校验 目前最多支持不超过6个包	
		if (*(m_FrameInfo->pHeader + 10) > 0x55)
		{
			m_bIFrameSecondIndex = false;
			m_iFrameTempLength = 0;
			return false;
		}
		
		//0x13表自定义头长度
		if (0xfffc - (3+0x13) == m_FrameInfo->nFrameLength || 0xfffc - (3+0x11) == m_FrameInfo->nFrameLength
		|| 0xfffc - (3+0x07) == m_FrameInfo->nFrameLength)
		{
			m_iFrameTempLength += m_FrameInfo->nFrameLength;
			m_FrameInfo->nLength = 0;
			m_FrameInfo->nType = DH_FRAME_TYPE_DATA;
		}
		else
		{
			m_bIFrameSecondIndex = false;
			m_FrameInfo->pContent = m_iFrameTemp;
			m_FrameInfo->pHeader = m_iFrameTemp;
			m_FrameInfo->nLength = m_iFrameTempLength + m_FrameInfo->nFrameLength;
	
			m_FrameInfo->nFrameLength = m_FrameInfo->nLength;
			m_iFrameTempLength = 0;
			return true;
		}
	}
	
//#ifdef MPEG_PS
	if (m_EncodeType == MPEG_PS) 
	{
		if (m_FrameInfo->nType == DH_FRAME_TYPE_VIDEO)
		{
			if (m_FrameInfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)
			{
				unsigned char *pBuf = m_FrameInfo->pContent ;
				
				unsigned int code = 0xffffffff;
				
				int hbe = 0;
				while (code != 0x01B6)
				{
					if (pBuf > m_FrameInfo->pContent+m_FrameInfo->nFrameLength)
					{
						m_FrameInfo->nLength = 0;
						m_FrameInfo->nType = DH_FRAME_TYPE_DATA;
						return true;
					}
					
					code = code << 8 | *pBuf++;
					
					if (code == 0x0120 || code == 0x0121)
					{
						int iret = 0;
						__try
						{
							int nRate;
							//	iret = ParseStdMp4(pBuf, 100, &m_FrameInfo->nWidth, &m_FrameInfo->nHeight);
							iret = Mpeg4_Analyse(m_FrameInfo->pContent,m_FrameInfo->nFrameLength,&m_FrameInfo->nWidth,&m_FrameInfo->nHeight,&nRate);
						}
						__except(0,1)
						{
							iret = -1;
						}
						
						if (iret < 0)
						{
							m_FrameInfo->nWidth = 352;
							m_FrameInfo->nHeight = 288;
						}
						
						if (pBuf[0]==0x00&&pBuf[1]==0xCA)//HBE mpeg
						{
							if (m_FrameInfo->nFrameRate == 0)
							{
								m_FrameInfo->nFrameRate = (pBuf[5]>>3)&0x1F;
							}
						}
					}
					else if (code == 0x01B3)
					{
						m_FrameInfo->nHour	= pBuf[0]>>3;
						m_FrameInfo->nMinute	= (pBuf[0]&0x07)<<3 | pBuf[1]>>5;
						m_FrameInfo->nSecond	= (pBuf[1]&0x0f)<<2 | pBuf[2]>>6;
						m_FrameInfo->nTimeStamp = (m_FrameInfo->nHour*3600+
							m_FrameInfo->nMinute*60+m_FrameInfo->nSecond)/**1000*/;
					}//end of else if (tmp_code == 0x01B3)
				}
			}//end of if (m_FrameInfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)			
			
			if (m_bIFrameFirstIndex)
			{
				m_width = m_FrameInfo->nWidth;
				m_height = m_FrameInfo->nHeight;
				m_rate = m_FrameInfo->nFrameRate;
				
				memcpy(m_iFrameTemp, m_FrameInfo->pHeader, m_FrameInfo->nLength);
				m_iFrameTempLength = m_FrameInfo->nLength;
				
				m_FrameInfo->nType = DH_FRAME_TYPE_DATA;
				m_FrameInfo->nLength = 0;
				m_bIFrameSecondIndex = true;
				m_bIFrameFirstIndex = false;
			}
			
		}//end of if (m_FrameInfo->nType == DH_FRAME_TYPE_VIDEO)
		else if (m_FrameInfo->nType == DH_FRAME_TYPE_AUDIO)
		{
			m_FrameInfo->nBitsPerSample = 16;
			m_FrameInfo->nChannels = 1;
			
			switch (m_FrameInfo->nFrameLength)
			{
			case 288:
				m_FrameInfo->nSamplesPerSecond = 32000;
				break;
			case 216:
				m_FrameInfo->nSamplesPerSecond = 16000;
				break;
			default:
				m_FrameInfo->nSamplesPerSecond = 16000;
				break;
			}
		}
	}
//#endif

	return true ;
}
