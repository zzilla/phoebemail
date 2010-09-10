// newstream.cpp: implementation of the newstream class.
//
//////////////////////////////////////////////////////////////////////
#include <time.h>
#include "newstream.h"
/*#include <windows.h>*/

#define PCM_TYPE_G711U	22

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

NewStream::NewStream(unsigned char* rawBuf):StreamParser(rawBuf)
{
}

NewStream::~NewStream()
{
}

inline
bool NewStream::CheckSign(const unsigned int& Code)
{
	if (Code == 0x01FD || Code == 0x01FC || Code == 0x01FB || Code == 0x01FA
		|| Code == 0x01F0 || Code == 0x01F1 || Code == 0x01FE)
	{
		return true ;
	}

	return false ;
}

inline
bool NewStream::ParseOneFrame()
{
	if ((m_code == 0x01FD || m_code == 0x01FB || m_code == 0x01FE) && (rest >= 12))//I帧B帧
	{
		m_FrameInfo = m_FrameInfoList.GetFreeNote() ;
		m_FrameInfo->nParam2 = m_bufptr[0]/*2*/ ;
		m_FrameInfo->nFrameRate  = m_bufptr[1] &0x1F;
		m_FrameInfo->nParam1 = m_bufptr[1]>>5;
		m_FrameInfo->nWidth = m_bufptr[2]*8 ;
		m_FrameInfo->nHeight = m_bufptr[3]*8 ;

		unsigned long dt = m_bufptr[4] | m_bufptr[5] <<8 | m_bufptr[6] << 16 | m_bufptr[7] << 24;
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
		tmp_time.tm_mday = m_FrameInfo->nDay  /*+ 1*/;
		tmp_time.tm_mon  = m_FrameInfo->nMonth - 1 ;
		tmp_time.tm_year = m_FrameInfo->nYear - 1900 ;
		tmp_time.tm_isdst= -1; //自动计算是否为夏时制, (bug: 时区选为美国，自动使用夏时制时，时间计算有误)
		m_FrameInfo->nTimeStamp = mktime(&tmp_time) ;

		m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO ;
		m_FrameInfo->nSubType = DH_FRAME_TYPE_VIDEO_I_FRAME ;

		if (m_code == 0x01FD)
		{
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_H264 ;
		}
		else if (m_code == 0x01FB)
		{
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_MPEG4 ;
		}
		else if (m_code == 0x01FE)
		{
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_JPEG;
		}

		m_FrameInfo->nFrameLength = m_frameLen = 
			/*m_bufptr[11]<<24|*/m_bufptr[10]<<16|m_bufptr[9]<<8|m_bufptr[8] ;		
			
		m_preShSeq = m_bufptr[11] ;	

		m_FrameInfo->pHeader = m_bufptr - 4 ;			
		m_FrameInfo->pContent = m_bufptr + 12 ;
		m_FrameInfo->nLength  = m_FrameInfo->nFrameLength + 16 ;
		rest -= 12 ;
		m_bufptr += 12 ;

		return true ;
	}
	else if ((m_code == 0x01FC || m_code == 0x01FA) && (rest >= 4))//P帧和A帧
	{
		m_FrameInfo = m_FrameInfoList.GetFreeNote() ;
		m_FrameInfo->nType = DH_FRAME_TYPE_VIDEO ;
		m_FrameInfo->nSubType = DH_FRAME_TYPE_VIDEO_P_FRAME ;

		if (m_code == 0x01FC)
		{
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_H264 ;
		}
		else
		{
			m_FrameInfo->nEncodeType = DH_ENCODE_VIDEO_MPEG4 ;
		}

		m_FrameInfo->nFrameLength = m_frameLen = 
			/*m_bufptr[3]<<24|*/m_bufptr[2]<<16|m_bufptr[1]<<8|m_bufptr[0] ;			

		if (m_preShSeq == 0)
		{
			m_preShSeq = m_bufptr[3];
		}
				
		int tmpframelen = m_FrameInfo->nFrameLength;

		if (m_bufptr[3] > m_preShSeq+1  || m_bufptr[3] < m_preShSeq)
		{
			if (m_bufptr[3] == 0 && m_preShSeq == 255)
			{
			}
			else
			{
 	 	 		m_FrameInfo->nFrameLength = m_frameLen = (m_frameLen-1);
			}
		}
	
		m_preShSeq = m_bufptr[3] ;	

		m_FrameInfo->pHeader = m_bufptr - 4 ;
		m_FrameInfo->pContent = m_bufptr + 4 ;
		m_FrameInfo->nLength  = m_FrameInfo->nFrameLength + 8 ;
		rest -= 4 ;
		m_bufptr += 4 ;
		
		return true ;
	}
	else if ((m_code == 0x01F0) && (rest >= 4))//音频帧
	{
		m_FrameInfo = m_FrameInfoList.GetFreeNote() ;
		m_FrameInfo->nType = DH_FRAME_TYPE_AUDIO ;				
		m_FrameInfo->nEncodeType = m_bufptr[0] ;
		if (m_FrameInfo->nEncodeType == 10)
		{
			m_FrameInfo->nEncodeType = PCM_TYPE_G711U;
		}
		m_FrameInfo->nChannels = 1;	
		AudioInfoOpr(m_FrameInfo, m_bufptr[1]) ;

		m_FrameInfo->nFrameLength = m_frameLen = m_bufptr[3] <<8 | m_bufptr[2] ;		
		m_FrameInfo->pHeader = m_bufptr - 4 ;
		m_FrameInfo->pContent = m_bufptr + 4 ;
		m_FrameInfo->nLength  = m_frameLen + 8 ;
		rest -= 4 ;
		m_bufptr += 4 ;	

		return true ;
	}
	else if ((m_code == 0x01F1) && (rest >= 12))//动检帧，压缩卡
	{	
		m_FrameInfo = m_FrameInfoList.GetFreeNote() ;		
		if (m_bufptr[0] == 8)
		{
			m_FrameInfo->nFrameLength = m_frameLen = 
				m_bufptr[11]<<24|m_bufptr[10]<<16|m_bufptr[9]<<8|m_bufptr[8] ;	
			rest -= 12 ;
			m_bufptr += 12 ;	
		}
		else
		{
			m_FrameInfo->nFrameLength = m_frameLen = 4 ;
			rest -= 4 ;
			m_bufptr += 4 ;	
			m_FrameInfo->nLength  = 0 ;//无效帧
		}

		m_FrameInfo->nType = DH_FRAME_TYPE_DATA;
		
		return true ;
	}

	return false ;
}

inline
bool NewStream::CheckIfFrameValid()
{
	static const unsigned int IFRAME_D_CODE = 0x000001FD ;
	static const unsigned int IFRAME_B_CODE = 0x000001FB ;
	static const unsigned int PFRAME_C_CODE = 0x000001FC ;
	static const unsigned int PFRAME_A_CODE = 0x000001FA ;
	static const unsigned int MDFRAME_CODE = 0x000001F1 ;
	static const unsigned int AUDIOCODE = 0x000001F0 ;
	static const unsigned int PICCODE = 0x000001FE ;
	int i = 4 ;
	m_code = 0 ;

	while (rest >0 && i--)
	{
		m_code = m_code << 8 | *m_bufptr++ ;
		rest-- ;
		if (m_code != (IFRAME_D_CODE >> (i*8)) && m_code != (IFRAME_B_CODE >> (i*8))
			&& m_code != (PFRAME_C_CODE >> (i*8)) && m_code != (PFRAME_A_CODE >> (i*8))
			&& m_code != (MDFRAME_CODE >> (i*8)) && m_code != (AUDIOCODE >> (i*8))
			&& m_code != (PICCODE >> (i*8)))
		{

			return false ;
		}
	}

	return true ;
}
