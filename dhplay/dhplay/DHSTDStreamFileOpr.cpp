// DHSTDStreamFileOpr.cpp: implementation of the DHSTDStreamFileOpr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DHSTDStreamFileOpr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define HEADER_DATA_LEN		36


DHSTDStreamFileOpr::DHSTDStreamFileOpr()
{

}

DHSTDStreamFileOpr::~DHSTDStreamFileOpr()
{

}

int  DHSTDStreamFileOpr:: CreateIndex(char* filename, DWORD beginpos, std::vector<DH_INDEX_INFO*>& m_pFileIndex, 
		DWORD& m_totalFrameNum, DHTIME& begintime, DHTIME& endtime,__int64& m_nDataLen)//建立索引，把索引信息放在m_pFileIndex里
{
	DWORD len;
	BYTE mData[HEADER_DATA_LEN];
	const static BYTE hFrame[4] = {'D','H','A','V'};
	DWORD tmp_FirstIFrameNum = 0;
	HANDLE f_CreateIndex = NULL;

	f_CreateIndex = CreateFile(filename, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL) ;
	
	if (f_CreateIndex == NULL || (DWORD)f_CreateIndex == 0xffffffff)
	{
		return 1 ;
	}

	DWORD dwTotalFrameNum = 0;

	__int64 temp_pCurPos ;

	temp_pCurPos = beginpos ;

	while(true)
	{
		if (m_bCreateIndex == 0)
		{
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}

		if ((m_nDataLen - temp_pCurPos) < HEADER_DATA_LEN || temp_pCurPos >= m_nDataLen)
		{
			//文件结束
			if (((m_nDataLen - temp_pCurPos) > 0) && (m_nDataLen - temp_pCurPos < HEADER_DATA_LEN))
			{
				m_nDataLen = temp_pCurPos ;
			}
L:
			if (m_pFileIndex.size() <= 0)
			{
				//文件很小，没有I帧
				CloseHandle(f_CreateIndex) ;
				f_CreateIndex = NULL ;
				return 1 ;
			}
			
			m_totalFrameNum = dwTotalFrameNum;
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;

			return 0 ;
		}

		if (myFileSeek (f_CreateIndex, temp_pCurPos, FILE_BEGIN)	== -1)
		{	
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}

		if (ReadFile(f_CreateIndex, mData, HEADER_DATA_LEN, &len, NULL) == FALSE)
		{
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}

		if (memcmp(mData, hFrame , 4))   //非桢头
		{
			if (m_nDataLen - temp_pCurPos < 1024)//忽略最后1K个无效数据
			{
		 		m_nDataLen = temp_pCurPos ;
				goto L ;
			}
			temp_pCurPos++;
			continue;
		}

		unsigned char crc = 0 ;
		for (int i = 0; i < 23; i++)
		{
			crc += mData[i];
		}

		int len = *(DWORD *)&mData[12] ;    //5~8字节表示数据大小
		if (len > 500*1024 || len < 5 || crc != mData[23])
		{
			temp_pCurPos += 1;
			continue;
		}
	
		if (mData[4] == 0xFD)
		{
			if (tmp_FirstIFrameNum == 0)
			{
				tmp_FirstIFrameNum = *(DWORD *)&mData[8];
			}
			DH_INDEX_INFO* m_index = new DH_INDEX_INFO ;
			memset(m_index, 0, sizeof(DH_INDEX_INFO));

			*(DWORD *)&m_index->time = *(DWORD *)&mData[16];   //9~12字节表示时间
			m_index->IFramePos = temp_pCurPos;
			m_index->IFrameNum = *(DWORD *)&mData[8] - tmp_FirstIFrameNum;	
			m_index->framelen  = mData[12]|mData[13]<<8|mData[14]<<16|mData[15]<<24;

			if (mData[24] == 0x81 && mData[28] == 0x80)
			{
				m_index->IFrameRate = mData[27];
			}
			else if (mData[24] == 0x80 && mData[28] == 0x81)
			{
				m_index->IFrameRate = mData[31];
			}

			if (m_nDataLen - temp_pCurPos >= m_index->framelen)
			{
				m_pFileIndex.push_back(m_index);
				dwTotalFrameNum++;
				temp_pCurPos += m_index->framelen;
				continue;
			}
			else
			{
				if (m_index != NULL)
				{
					delete m_index;
					m_index = NULL;
				}
			}
		}
		else if ((mData[4] == 0xFC) || (mData[4] == 0xFE) || (mData[4] == 0xFB))
		{
			DWORD dwFramelen = mData[12]|mData[13]<<8|mData[14]<<16|mData[15]<<24;

			if (tmp_FirstIFrameNum == 0)
			{
				temp_pCurPos += dwFramelen;
				continue;
			}

			if (m_nDataLen - temp_pCurPos >= dwFramelen)
			{
				dwTotalFrameNum++;
				temp_pCurPos += dwFramelen;
				continue;
			}
		}

		temp_pCurPos += len;
	}
	
	return 1;
}

__int64 DHSTDStreamFileOpr::GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, __int64 KeyFramePos, 
		DWORD nFrame) //得到指定帧的结束位置
{
	const static BYTE hFrame[4] = {'D','H','A','V'};
	const static int HEADERLEN = 24;
	BYTE mData[HEADERLEN];
	DWORD len = 0 ;

	DWORD FrameNum		= KeyFrameNum ;
	__int64 temp_pos		= KeyFramePos ;
	__int64 m_nDataLen	  = GetFileSize(m_pFile,NULL) ;
	
	while(true)
	{
		if (temp_pos >= m_nDataLen)
		{
			return 0 ;
		}

		if (myFileSeek (m_pFile, temp_pos, FILE_BEGIN)	== -1)
		{
			return -1 ;
		}

		if (ReadFile(m_pFile, mData, HEADERLEN, &len, NULL) == FALSE)
		{
			return -1 ;
		}

		if (len < HEADERLEN)
		{
			return -1 ;
		}
		
		if(memcmp(mData, hFrame , 4))   //非桢头
		{
			temp_pos += 1;
			continue;
		}

		unsigned char crc = 0 ;
		for (int i = 0; i < 23; i++)
		{
			crc += mData[i];
		}

		int len = *(DWORD *)&mData[12] ; 
		if (len > 500*1024 || len < 5 || crc != mData[23])
		{
			temp_pos += 1;
			continue;
		}

		temp_pos += len;  

		if(mData[4] == 0xFD || mData[4] == 0xFC)//视频帧
		{
			if (FrameNum == nFrame)
			{
				return temp_pos ;
			}
			else
			{
				FrameNum ++;
			}
		}
	}

	return -1 ;
}

void DHSTDStreamFileOpr::GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info)
{
	static const int BUFLEN = 4116;
	static const int HEADERLEN = 30;
	static const BYTE DHSTD_IFrame[5] = {'D','H','A','V',0xFD};
	static const BYTE DHSTD_PFrame[5] = {'D','H','A','V',0xFC};
	static const BYTE DHSTD_BFrame[5] = {'D','H','A','V',0xFE};
	static const BYTE DHSTD_AFrame[5] = {'D','H','A','V',0xF0};

	unsigned int Audio_Samples[] = {4000, 8000, 11025, 16000, 20000, 22050, 32000, 44100, 48000};

	BYTE buf[BUFLEN] ;
	BYTE *pos = 0 ;	
	DWORD len = 0;
	int firstframeseq = 0;
	int lastframeseq = 0;

	__int64 tmp_pCurPos = 0;
	__int64 tmp_nDataLen = GetFileSize(m_pFile, NULL) ;

	bool bVideoFlag = true;
	bool bAudioFlag = true;
	
	while (bVideoFlag || bAudioFlag)
	{
		if (tmp_pCurPos > tmp_nDataLen - BUFLEN || (tmp_pCurPos >= 4194304))
		{
			if (!bVideoFlag)
			{
				dwFrame = 0;
				break;
			}

			break;
		}

		myFileSeek (m_pFile, tmp_pCurPos, FILE_BEGIN) ;
		ReadFile(m_pFile, buf, BUFLEN, &len, NULL) ;

		pos = buf ;
			
		while ( (pos = (BYTE *)memchr(pos, 'D', BUFLEN-HEADERLEN - (pos - buf))) != NULL)   //先找到第一个字符
		{
			//新码流
			if (bVideoFlag && (memcmp(pos, DHSTD_IFrame, 5) == 0))
			{
				bVideoFlag = false;
				firstframeseq = pos[8] | pos[9] <<8 | pos[10] << 16 | pos[11] << 24 ;
				*(DWORD*)(&(_video_info.DirStartTime)) = *(DWORD *)(pos+16);   //9~12字节表示时间

				if ((*(pos+24) == 0x81) && (*(pos+28) == 0x80))
				{
					_video_info.VideoHigh	= *(pos+31)*8;
					_video_info.VideoWidth	= *(pos+30)*8;
					_video_info.Framerate	= *(pos+27);
					_video_info.VideoType	= *(pos+26);
				}
				else if ((*(pos+24) == 0x80) && (*(pos+28) == 0x81))
				{
					_video_info.VideoHigh	= *(pos+27)*8;
					_video_info.VideoWidth	= *(pos+26)*8;
					_video_info.Framerate	= *(pos+31);
					_video_info.VideoType	= *(pos+30);
				}

				switch(_video_info.VideoType)
				{
				case 2: 
					_video_info.VideoType = 4 ;
					break ;
				case 1:
					_video_info.VideoType = 1 ;
					break ;
				case 8:  //ADI和大华264
					_video_info.VideoType = 4;
					break;
				default:
					break;
				}

				pos++;
			}
			else if (bAudioFlag && (memcmp(pos, DHSTD_AFrame, 5) == 0))
			{
				if (*(pos+24) == 0x83)
				{
					bAudioFlag = false;
					_video_info.AudioChannel = *(pos+25);
					
					_video_info.AudioType = *(pos+26);
					if (_video_info.AudioType == 10)
					{
						_video_info.AudioType = 22; //G711
					}
					
					_video_info.AudioSamplesPerSec = Audio_Samples[1];
					if ((*(pos+27) >= 1) && (*(pos+27) <= 9))
					{
						_video_info.AudioSamplesPerSec = Audio_Samples[*(pos+27) - 1];
					}
					
					if (_video_info.AudioType == 7 /*PCM8*/
						|| _video_info.AudioType == 30 /*DH_ENCODE_AUDIO_TALK*/)
					{
						_video_info.AudioBitPerSample = 8;
					}
					else
					{
						_video_info.AudioBitPerSample = 16;
					}
				}
				
				pos++;
			}
			else
			{
				pos++;
				if (BUFLEN-HEADERLEN <= (pos - buf) )
				{
					break;
				}
			}
		}

		tmp_pCurPos += (BUFLEN - HEADERLEN);
	}

	bVideoFlag = true;
	tmp_pCurPos = tmp_nDataLen-BUFLEN-1;

	while(bVideoFlag)
	{
		if (tmp_pCurPos < BUFLEN || (tmp_nDataLen-tmp_pCurPos) >= 2097152)
		{
			memset(&(_video_info.DirStartTime), 0, sizeof(DHTIME));
			return;
		}

		myFileSeek (m_pFile, tmp_pCurPos, FILE_BEGIN) ;
		ReadFile(m_pFile, buf, BUFLEN, &len, NULL) ;

		pos = buf ;
			
		while ( (pos = (BYTE *)memchr(pos, 'D', BUFLEN-HEADERLEN - (pos - buf))) != NULL)   //先找到第一个字符
		{
			if (memcmp(pos, DHSTD_IFrame, 5) == 0 || memcmp(pos, DHSTD_PFrame,5) == 0 
				|| memcmp(pos, DHSTD_BFrame,5) == 0)
			{
				bVideoFlag = false;
				lastframeseq = pos[8] | pos[9] <<8 | pos[10] << 16 | pos[11] << 24 ;
				*(DWORD*)(&(_video_info.DirEndTime)) = *(DWORD *)(pos+16);   //9~12字节表示时间
				dwFrame = lastframeseq - firstframeseq + 1;
			}

			pos++;

			if (BUFLEN-HEADERLEN <= (pos - buf) )
			{
				break;
			}
		}

		tmp_pCurPos -= (BUFLEN - HEADERLEN);
	}
}