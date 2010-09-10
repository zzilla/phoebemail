// DhStreamParser.cpp: implementation of the DhStreamParser class.
//
//////////////////////////////////////////////////////////////////////
#include "DhStreamParser.h"
#include "StreamParser.h"
#include "newstream.h"
#include "rwstream.h"
#include "oldstream.h"
#include "shstream.h"
#include "psstream.h"
#include "dhstdstream.h"
#include "asfstream.h"
//#include "..\..\stdafx.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4305)
#pragma warning(disable:4309)

DhStreamParser::DhStreamParser(int nStreamType, int nFlag)
{
	m_nStreamType = nStreamType ;
	m_nTryTimes = 0 ;

	m_buffer = new unsigned char[MAX_BUFFER_SIZE];

	if (m_nStreamType == DH_STREAM_MPEG4)
	{
		m_streamParser = new RwStream(m_buffer) ;
	}
	else if (m_nStreamType == DH_STREAM_DHPT)
	{
		m_streamParser = new OldStream(m_buffer) ;
	}
	else if (m_nStreamType == DH_STREAM_NEW)
	{
		m_streamParser = new NewStream(m_buffer) ;
	}
	else if (m_nStreamType == DH_STREAM_HB)
	{
		m_streamParser = new ShStream(m_buffer) ;
	}
	else
	{
		m_nStreamType = DH_FRAME_TYPE_UNKNOWN ;
		m_streamParser = NULL ;
	}
}

DhStreamParser::~DhStreamParser()
{
	if (m_streamParser)
	{
		delete m_streamParser ;
		m_streamParser = NULL ;
	}

	if (m_buffer != NULL)
	{
		delete[] m_buffer;
	}
}

int DhStreamParser::InputData(unsigned char *pData, unsigned long nDataLength)
{
	if (m_nStreamType == DH_FRAME_TYPE_UNKNOWN 
		|| m_nStreamType == DH_STREAM_AUDIO)
	{
		int iRet = AutoScanStream(pData, nDataLength);
		if ( iRet< 0)
		{
			if (m_nStreamType == DH_STREAM_AUDIO && m_streamParser)//纯音频数据
			{
				return m_streamParser->ParseData(pData, nDataLength) ;
			}
			else//没找到
			{
				return -1 ;
			}
		}
	}

	return m_streamParser->ParseData(pData, nDataLength) ;
}

DH_FRAME_INFO *DhStreamParser::GetNextFrame()
{	
	if (m_nStreamType == DH_FRAME_TYPE_UNKNOWN)
	{
		return NULL ;
	}

	return m_streamParser->GetNextFrame() ; 
}

DH_FRAME_INFO *DhStreamParser::GetNextKeyFrame()
{
	if (m_nStreamType == DH_FRAME_TYPE_UNKNOWN)
	{
		return NULL ;
	}

	return m_streamParser->GetNextKeyFrame() ;
}

int DhStreamParser::Reset(int nLevel, int streamtype)
{
	if (m_nStreamType == DH_FRAME_TYPE_UNKNOWN)
	{
		if (streamtype == DH_STREAM_NEW && nLevel == STREAMPARSER_RESET_REFIND)
		{
			m_nStreamType = streamtype;
			m_streamParser = new NewStream(m_buffer) ;
		}

		return -1 ;
	}

	if (nLevel == STREAMPARSER_RESET_REFIND)
	{
		if (streamtype == DH_STREAM_NEW)
		{
			if (m_streamParser)
			{
				delete m_streamParser;
				m_streamParser = NULL;
			}

			m_nStreamType = DH_STREAM_NEW;
			m_streamParser = new NewStream(m_buffer) ;

			return 0;
		}

		m_nStreamType = DH_FRAME_TYPE_UNKNOWN ;

		return 0 ;
	}

	return m_streamParser->Reset(nLevel) ;
}

int DhStreamParser::GetStreamType()
{
	return m_nStreamType ;
}

int DhStreamParser::AutoScanStream(unsigned char *pData, unsigned long nDataLength)
{
	static const unsigned long MAXSCANLEN = 51200 ;
	int DHPTStreamCounter = 0;
	int NEWStreamCounter = 0;
	int MPEG4StreamCounter = 0;
	int AUDIOStreamCounter = 0;
	int AVIStreamCounter = 0;
	int PSStreamCounter = 0;
	int SH2StreamCounter = 0;
	int DHSTDStreamCounter = 0;
	int ASFStreamCounter = 0;
	unsigned int Code = 0xFFFFFFFF;
	unsigned char *pScanBuf = pData;
	//m_nTryTimes = 0;
	long DataRest = nDataLength ;
	unsigned long asfpacketlen = 0;
	unsigned int videoStreamID = 1;

	while (DataRest--)
	{
		Code = (Code << 8) | *pScanBuf++;
		
		if (Code == 0x000001F2)
		{
			if (DataRest < 12)
			{
				continue;
			}

			int rate  = pScanBuf[1] >> 3;
			int size = pScanBuf[2] | pScanBuf[3] << 8 ;
			int width = ((size>>3)&0x3f)*16;
			int height = ((size>>9)&0x7f)*16;

			if (width != 704 && width != 352 && width != 176 && width != 600 && width != 720)
			{
				continue;
			}
			if (height != 576  && height != 480 && height != 288 && height != 240 
				&& height != 144 && height != 120)
			{
				continue;
			}
			if (rate > 100 || rate < 1)
			{
				continue;
			}

			SH2StreamCounter = 1;
	
			if (nDataLength - DataRest > MAXSCANLEN)
			{
				break ;
			}
		}
		else if (Code == 0x44485054)
		{
// 			DHPTStreamCounter = 1 ;
// 			
// 			if (nDataLength - DataRest > MAXSCANLEN)
// 			{
// 				break ;
// 			}
		}
		else if (Code == 0x01FD || Code == 0x01FC || Code == 0x01FB || Code == 0x01FA || Code == 0x01FE)
		{
			if (DataRest < 12)
			{
				continue;
			}

			int width = pScanBuf[2] * 8;
			int height = pScanBuf[3] * 8;
			int rate = pScanBuf[1] & 0x1F;

			if (width < 160 || width > 1920)
			{
				if (Code == 0x01FD || Code == 0x01FB)
				{
					continue;
				}
			}
			if (height < 120 || height > 1216)
			{
				if (Code == 0x01FD || Code == 0x01FB)
				{
					continue;
				}
			}

			if (rate > 100 || rate < 1)
			{
				if (Code == 0x01FD || Code == 0x01FB)
				{
					continue;
				}
			}

		 	NEWStreamCounter = 1 ;
			
			if (nDataLength - DataRest > MAXSCANLEN)
			{
				break ;
			}
		}
		else if (Code == 0x01B6)
		{
			MPEG4StreamCounter = 1 ;
			m_nTryTimes++;

			if (nDataLength - DataRest > MAXSCANLEN)
			{
				break ;
			}
		}
		else if (Code == 0x01F0)
		{
			AUDIOStreamCounter = 1 ;

			if (nDataLength - DataRest > MAXSCANLEN)
			{
				break ;
			}
		}
		else if (Code == 0x52494646)//RIFF
		{
			if (DataRest > 8)
			{
				unsigned int rifftype = pScanBuf[4]<<24|pScanBuf[5]<<16|pScanBuf[6]<<8|pScanBuf[7];
				if (rifftype==0x41564920) {
					AVIStreamCounter = 1;
					break;
				}
			}
		}
		else if (Code == 0x01BA)
		{
			PSStreamCounter = 1;
			if (nDataLength - DataRest > MAXSCANLEN)
			{
				break ;
			}
		}
		else if (Code == 0x44484156)
		{
			if (DataRest < 20)
			{
				continue;
			}

			unsigned char crc = 0 ;	
			for (int i = 0; i < 19; i++)
			{
				crc += pScanBuf[i];
			}
			crc += 0x44 ;
			crc += 0x48 ;
			crc += 0x41 ;
			crc += 0x56 ;

			if (crc != pScanBuf[19])
			{
				continue;
			}

			DHSTDStreamCounter = 1;
			if (nDataLength - DataRest > MAXSCANLEN)
			{
				break ;
			}
		}
		else if (Code == 0x3026B275)//Asf
		{
			static const char AsfHeader[16] = {0x30,0x26,0xB2,0x75,0x8E,0x66,0xCF,0x11,0xA6, 0xD9,0x00, 0xAA,0x00,0x62,0xCE,0x6C};
			if (DataRest > 130)
			{
				if (memcmp(pScanBuf, AsfHeader+4, 12) == 0)
				{
					ASFStreamCounter = 1;
					asfpacketlen = *(unsigned long*)(pScanBuf+118);
				// 	break;
				}
			}
			
		}
		else if (Code == 0xC0EF19BC && ASFStreamCounter == 1)//ASF视频
		{
			static const char AsfVideoGUID[16] = {0xC0,0xEF,0x19,0xBC,0x4D,0x5B,0xCF,0x11,0xA8,0xFD,0x00,0x80,0x5F,0x5C,0x44,0x2B};
			if (DataRest > 80)
			{
				if (memcmp(pScanBuf, AsfVideoGUID+4, 12) == 0)
				{
					videoStreamID = pScanBuf[44] & 0x7F;
					break;
				}
			}
		}
		else if (Code == 0x409E69F8 && ASFStreamCounter == 1)//ASF音频
		{
			static const char AsfAudioGUID[16] = {0x40,0x9E,0x69,0xF8,0x4D,0x5B,0xCF,0x11,0xA8,0xFD,0x00,0x80,0x5F,0x5C,0x44,0x2B};
			if (DataRest > 80)
			{
				if (memcmp(pScanBuf, AsfAudioGUID+4, 12) == 0)
				{
					videoStreamID = ((pScanBuf[44] & 0x7F) == 1) ? 2 : 1;
					break;
				}
			}
		}
	}

	if (SH2StreamCounter || AVIStreamCounter || NEWStreamCounter || DHPTStreamCounter
		|| PSStreamCounter || MPEG4StreamCounter || DHSTDStreamCounter || ASFStreamCounter)
	{
		if (m_streamParser)
		{
			delete m_streamParser;
			m_streamParser = NULL;
		}
	}

	if (SH2StreamCounter > 0)
	{
		m_nStreamType = DH_STREAM_HB ;
		m_streamParser = new ShStream(m_buffer) ;
		return 0 ;
	}
	else if (ASFStreamCounter > 0)
	{
		m_nStreamType = DH_STREAM_ASF;
		m_streamParser = new ASFStream(m_buffer,asfpacketlen,videoStreamID);
		return 0;
	}
	else if (AVIStreamCounter > 0)
	{
		DH_FRAME_INFO tmpAviInfo;
		memset(&tmpAviInfo, 0, sizeof(tmpAviInfo));

		int auds = 0;

		while (DataRest--)
		{
			Code = (Code << 8) | *pScanBuf++;

			if (Code == 0x61766968)
			{
				pScanBuf += 4;
				int resultion =*(int*)pScanBuf;
				tmpAviInfo.nFrameRate = 1000000/resultion;
				pScanBuf+=32;
				tmpAviInfo.nWidth=*(int*)pScanBuf;
				tmpAviInfo.nHeight=*(int*)(pScanBuf+4);
			}
			else if (Code == 0x61756473)
			{
				auds = 1;
			}
			else if (Code == 0x73747266)
			{
				if (auds == 1 && DataRest > 20)
				{
					tmpAviInfo.nSamplesPerSecond = *((int*)(pScanBuf+8));
					tmpAviInfo.nBitsPerSample = (pScanBuf[16] | (pScanBuf[17] << 8))  * 8;
					break;
				}
			}
		}

		if (tmpAviInfo.nBitsPerSample != 8 && tmpAviInfo.nBitsPerSample != 16)
		{
			tmpAviInfo.nBitsPerSample = 8;
		}

		if (tmpAviInfo.nSamplesPerSecond < 4000 || tmpAviInfo.nSamplesPerSecond > 48000)
		{
			tmpAviInfo.nSamplesPerSecond = 8000;
		}
		m_nStreamType = DH_STREAM_MPEG4;
		m_streamParser = new RwStream(m_buffer, MPEG_AVI, &tmpAviInfo);
		return 0;
	}
	else if (DHPTStreamCounter > 0)
	{
		m_nStreamType = DH_STREAM_DHPT ;
		m_streamParser = new OldStream(m_buffer) ;
		return 0 ;
	}
	else if (DHSTDStreamCounter > 0)
	{
		m_nStreamType = DH_STREAM_DHSTD;
		m_streamParser = new DhStdStream(m_buffer);
		return 0;
	}
	else if (NEWStreamCounter > 0)
	{
		m_nStreamType = DH_STREAM_NEW ;
		m_streamParser = new NewStream(m_buffer) ;
		return 0 ;
	}
	else if (PSStreamCounter > 0)
	{
		m_nStreamType = DH_STREAM_PS ;
		m_streamParser = new PSStream(m_buffer) ;
		return 0 ;
	}
	else if (MPEG4StreamCounter > 0)
	{
		if ( m_nTryTimes >= 3 )
		{
			m_nStreamType = DH_STREAM_MPEG4 ;
			m_streamParser = new RwStream(m_buffer) ;
			m_nTryTimes = 0;
			return 0 ;
		}
		else
		{
			return -1;
		}
	}
	else if (AUDIOStreamCounter > 0)
	{
		if (m_nStreamType != DH_STREAM_AUDIO)
		{
			if (m_streamParser)
			{
				delete m_streamParser;
				m_streamParser = NULL;
			}	
			m_nStreamType = DH_STREAM_AUDIO ;
			m_streamParser = new NewStream(m_buffer) ;//纯音频流用新码流来解
		}
		
		return 0 ;
	}

	return -1 ;
}
