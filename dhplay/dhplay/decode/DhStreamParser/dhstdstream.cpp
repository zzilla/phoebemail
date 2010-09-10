// newstream.cpp: implementation of the newstream class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <time.h>
#include "dhstdstream.h"

#define PCM_TYPE_G711U	22
				

DhStdStream::DhStdStream(unsigned char* rawBuf):StreamParser(rawBuf)
{
	m_frameSeq = 0 ;
	m_error = false ;
	m_encType = DH_ENCODE_VIDEO_MPEG4;
}

DhStdStream::~DhStdStream()
{
}

bool DhStdStream::CheckSign(const unsigned int& Code)
{	
	if (Code == 0x44484156) //0x44484156--"DHAV"
	{
		return true ;
	}

	return false ;
}

bool DhStdStream::ParseOneFrame()
{
	unsigned long temp = 0xffff ;
	unsigned char crc = 0 ;	

	if (rest < 20)
	{
		return false;
	}

// 	//加强对子帧序号的判断，现阶段都必须为0。防止不标准码流的出现
// 	if ( 0 != m_bufptr[3] )
// 	{
// 		return false;
// 	}

	for (int i = 0; i < 19; i++)
	{
		crc += m_bufptr[i] ;
	}
	crc += 0x44 ;
	crc += 0x48 ;
	crc += 0x41 ;
	crc += 0x56 ;

	m_FrameInfo = m_FrameInfoList.GetFreeNote() ;

	if (crc != m_bufptr[19])
	{
		m_FrameInfo->nFrameLength = 4 ;
		m_frameLen = 4;
		rest -= 4 ;
		m_bufptr += 4 ;
		return true;
	}

	m_extendLen = m_bufptr[18] ;
	if (rest < 20 + m_extendLen)
	{
		return false;
	}

	unsigned long dt = m_bufptr[12] | m_bufptr[13] <<8 | m_bufptr[14] << 16 | m_bufptr[15] << 24;
	m_FrameInfo->nSecond = dt & 0x3f;
	m_FrameInfo->nMinute = (dt >> 6) & 0x3f;
	m_FrameInfo->nHour = (dt >> 12) & 0x1f;
	m_FrameInfo->nDay = (dt >> 17) & 0x1f;
	m_FrameInfo->nMonth = (dt >> 22) & 0xf;
	m_FrameInfo->nYear = 2000 + (dt >> 26);

	tm tmp_time ;
	tmp_time.tm_hour = m_FrameInfo->nHour /*- 1*/ ;
	tmp_time.tm_min  = m_FrameInfo->nMinute /*- 1*/ ;
	tmp_time.tm_sec  = m_FrameInfo->nSecond /*- 1*/ ;
	tmp_time.tm_mday = m_FrameInfo->nDay  + 1;
	tmp_time.tm_mon  = m_FrameInfo->nMonth /*- 1*/ ;
	tmp_time.tm_year = m_FrameInfo->nYear - 1900 ;
	tmp_time.tm_isdst= -1; //自动计算是否为夏时制, (bug: 时区选为美国，自动使用夏时制时，时间计算有误)
	m_FrameInfo->nTimeStamp = mktime(&tmp_time) ;
	
	m_FrameInfo->nFrameNum = m_bufptr[4] | m_bufptr[5] <<8 | m_bufptr[6] << 16 | m_bufptr[7] << 24 ;
	m_FrameInfo->nLength = m_bufptr[8]|m_bufptr[9]<<8|m_bufptr[10]<<16|m_bufptr[11]<<24 ; //包括帧头和帧尾的总长度
	m_frameLen = m_FrameInfo->nLength - 24 ; //包括帧尾和扩展的数据长度，不包括帧头24个字节
	m_FrameInfo->nFrameLength = m_frameLen - 8 - m_extendLen  ; //不包括帧头和帧尾和扩展的纯数据长度
	m_FrameInfo->pHeader = m_bufptr - 4 ;			
	m_FrameInfo->pContent = m_FrameInfo->pHeader + 24 + m_extendLen ;

	if (m_bufptr[0] == 0xFD)
	{
		m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO ;

		m_frameSeq = m_bufptr[4] | m_bufptr[5] <<8 | m_bufptr[6] << 16 | m_bufptr[7] << 24 ;

		unsigned char deinterlace ;
		unsigned char encodetype ;
		
		if (m_bufptr[20] == 0x80 && m_bufptr[24] == 0x81)
		{
			m_FrameInfo->nWidth = m_bufptr[22]*8 ;
			m_FrameInfo->nHeight = m_bufptr[23]*8 ;
			m_FrameInfo->nFrameRate  = m_bufptr[27] ;
			deinterlace = m_bufptr[21] ;
			encodetype = m_bufptr[26] ;
		}
		else if (m_bufptr[20] == 0x81 && m_bufptr[24] == 0x80)
		{	
			m_FrameInfo->nWidth = m_bufptr[26]*8 ;
			m_FrameInfo->nHeight = m_bufptr[27]*8 ;
			m_FrameInfo->nFrameRate  = m_bufptr[23] ;
			deinterlace = m_bufptr[25] ;
			encodetype = m_bufptr[22] ;
		}
		else
		{
			rest -= 5;
			m_bufptr += 5;
			m_FrameInfo->nFrameLength = 4 ;
			return false ;
		}
		
		switch(deinterlace)
		{
		case 0:
			m_FrameInfo->nParam1 = 2 ;
			break;
		case 1:
			m_FrameInfo->nParam1 = 1 ;
			break;
		case 2:
			m_FrameInfo->nParam1 = 0 ;
			break;
		}

		m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO ;
		m_FrameInfo->nSubType = DH_FRAME_TYPE_VIDEO_I_FRAME ;

		switch(encodetype)
		{
		case 2: //海思264
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_HI_H264 ;
			m_FrameInfo->nParam2 = 2;
			break ;
		case 1:
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_MPEG4 ;
			break ;
		case 8:  //ADI和大华264
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_H264;
			m_FrameInfo->nParam2 = 1;
			break;
		default:
			return false;
		}
			
		m_encType = m_FrameInfo->nEncodeType;
	}
	else if (m_bufptr[0] == 0xFC || m_bufptr[0] == 0xFE)
	{
		temp = m_bufptr[4] | m_bufptr[5] <<8 | m_bufptr[6] << 16 | m_bufptr[7] << 24 ;

		if (m_frameSeq == 0)
		{
			m_frameSeq =  temp ;
		}
			
		if(temp > m_frameSeq+1  || temp < m_frameSeq)
		{
			m_FrameInfo->nFrameLength = m_FrameInfo->nFrameLength-1;
			m_frameLen -= 1;
		}
	
		m_frameSeq = temp;

		m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO;
		if (m_bufptr[0] == 0xFC)
		{
			m_FrameInfo->nSubType = DH_FRAME_TYPE_VIDEO_P_FRAME ;
		}
		else 
		{
			m_FrameInfo->nSubType = DH_FRAME_TYPE_VIDEO_B_FRAME ;
		}

		m_FrameInfo->nEncodeType = m_encType;
	}
	else if (m_bufptr[0] == 0xF0)
	{
		m_FrameInfo->nType = DH_FRAME_TYPE_AUDIO ;				
		m_FrameInfo->nEncodeType = m_bufptr[22] ;
		if (m_FrameInfo->nEncodeType == 10)
		{
			m_FrameInfo->nEncodeType = PCM_TYPE_G711U;
		}
		m_FrameInfo->nChannels = m_bufptr[21];
		AudioInfoOpr(m_FrameInfo, m_bufptr[23]) ;
	}
	else if (m_bufptr[0] == 0xF1)
	{
		m_FrameInfo->nType = DH_FRAME_TYPE_DATA ;

		if (m_bufptr[1] == 0x06)//水印信息
		{
			if (rest < (20 + m_extendLen + 12 + m_FrameInfo->nFrameLength + 8))
			{
				return false;
			}

			unsigned char* watermarkBuf = m_FrameInfo->pContent;
			if (watermarkBuf[0] == 0x88)
			{
				watermarkBuf = m_FrameInfo->pContent + 8;
			}

			if (watermarkBuf[0] == 'T' && watermarkBuf[1] == 'E' 
				&& watermarkBuf[2] == 'X' && watermarkBuf[3] == 'T')
			{
				m_FrameInfo->nSubType = DH_FRAME_TYPE_WATERMARK_TEXT;
			}
			else if (watermarkBuf[0] == 'J' && watermarkBuf[1] == 'P' 
				&& watermarkBuf[2] == 'E' && watermarkBuf[3] == 'G')
			{
				m_FrameInfo->nSubType = DH_FRAME_TYPE_WATERMARK_JPEG;
			}
			else if (watermarkBuf[0] == 'B' && watermarkBuf[1] == 'M' 
				&& watermarkBuf[2] == 'P' && watermarkBuf[3] == ' ')
			{
				m_FrameInfo->nSubType = DH_FRAME_TYPE_WATERMARK_BMP;
			}

			m_FrameInfo->pContent = watermarkBuf + 12;
			m_FrameInfo->nFrameLength = watermarkBuf[8] | watermarkBuf[9] <<8 | 
				watermarkBuf[10] << 16 | watermarkBuf[11] << 24;
			

			if ( m_FrameInfo->nFrameLength != m_FrameInfo->nLength - 24 - m_extendLen - 8 - 12)
			{
				rest -= 5;
				m_bufptr += 5;
				m_FrameInfo->nFrameLength = 0;
				return false;
			}
			*(m_FrameInfo->pContent + m_FrameInfo->nFrameLength) = '\0';
		}
		else if (m_bufptr[1] == 0x05)//智能分析帧
		{
			m_FrameInfo->nType = DH_FRAME_TYPE_DATA ;
			m_FrameInfo->nSubType = DH_FRAME_TYPE_DATA_INTL;
			m_FrameInfo->nIvsSeq = m_bufptr[4] | m_bufptr[5] <<8 | m_bufptr[6] << 16 | m_bufptr[7] << 24 ;
		}
	}
	else
	{
		m_FrameInfo->nType = DH_FRAME_TYPE_DATA ;
	}

	rest -= 20;
	m_bufptr += 20;

	return true ;
}

bool DhStdStream::CheckIfFrameValid()
{
	static const unsigned int IFRAME_CODE = 0x44484156 ;

	int i = 4 ;
	m_code = 0 ;

	unsigned char* pFrameData = m_bufptr;

	// check Next Frame Header
	while (rest >0 && i--)
	{
		m_code = m_code << 8 | *m_bufptr++ ;
		rest-- ;
		if (m_code != (IFRAME_CODE >> (i*8)))
		{
			return false ;
		}
	}

	if (m_bCheckFrameEndFlag)
	{
		// check frame end, move pointer to frame end item
		pFrameData -= 8;
		unsigned int iSTDFrameEndFlag = 0x64686176;
		unsigned int iEndFrameFlag = (*pFrameData)<<24 | (*(pFrameData+1))<<16 | 
			(*(pFrameData+2))<<8 | (*(pFrameData+3));
		
		// 水印帧dhav帧尾标志中的“d”被替换成了结束符
		if ((m_FrameInfo->nSubType == DH_FRAME_TYPE_WATERMARK_TEXT ||
			m_FrameInfo->nSubType == DH_FRAME_TYPE_WATERMARK_JPEG ||
			m_FrameInfo->nSubType == DH_FRAME_TYPE_WATERMARK_BMP)
			&& m_FrameInfo->nType == DH_FRAME_TYPE_DATA)
		{
			iSTDFrameEndFlag = 0x00686176;
		}
		
		if (iEndFrameFlag != iSTDFrameEndFlag)
		{
			return false;
		}	
	}

	return true ;
}