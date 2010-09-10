#include "stdafx.h"
#include "ShStreamFileOpr.h"
#include <time.h>
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShStreamFileOpr::ShStreamFileOpr()
{

}

ShStreamFileOpr::~ShStreamFileOpr()
{

}

int   ShStreamFileOpr::CreateIndex(char* filename, DWORD beginpos, std::vector<DH_INDEX_INFO*>& m_pFileIndex
									, DWORD& m_nTotalFrameNum, DHTIME& m_begintime, DHTIME& m_endtime,__int64& nDataLen) //建立索引，把索引信息放在m_pFileIndex里
{
	DWORD len = 0;
	BYTE mData[16];   //用于读取桢头数据结构
	BYTE hFrame[3] = {00,00,01};  //动检帧桢头标志
	HANDLE f_CreateIndex = NULL;
	DH_INDEX_INFO* tmp_index = NULL ;

	f_CreateIndex = CreateFile(filename, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL) ;

	if (f_CreateIndex == NULL || (DWORD)f_CreateIndex == 0xffffffff)
	{
		return 1 ;
	}

	__int64  m_nDataLen   ;
	__int64 temp_pCurPos ;

	m_nDataLen  = GetFileSize(f_CreateIndex, NULL) ;
	temp_pCurPos = beginpos ;	
	m_nTotalFrameNum = 0 ;

	while (true)
	{
		if (m_bCreateIndex == 0)
		{
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}

		if ((m_nDataLen - temp_pCurPos) < 16 || temp_pCurPos >= m_nDataLen)
		{
			//文件结束
L:
			if (m_pFileIndex.size() <= 0)
			{
				//文件很小，没有I帧
				CloseHandle(f_CreateIndex) ;
				f_CreateIndex = NULL ;
				return 1 ;
			}
			
			*(DWORD*)&m_begintime = *(DWORD*)&(m_pFileIndex[0]->time) ;
			*(DWORD *)&m_endtime 	= *(DWORD *)&(m_pFileIndex[m_pFileIndex.size()-1]->time) ;

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

		if (ReadFile(f_CreateIndex, mData, 16, &len, NULL) == FALSE)
		{
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}

		if (memcmp(mData, hFrame , 3) != 0)   //非桢头
		{
			if (m_nDataLen - temp_pCurPos < 1024)//忽略最后1K个无效数据
			{
				goto L ;
			}
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1;   //重新处理或提示文件有问题
		}

		if (mData[3] == 0xf2)
		{
			tmp_index = new DH_INDEX_INFO ;
			memset(tmp_index, 0, sizeof(DH_INDEX_INFO));

			*(DWORD *)&tmp_index->time = *(DWORD *)&mData[8];   //9~12字节表示时间

// 			long dt = mData[8] | mData[9] << 8 | mData[10] << 16 | mData[11] << 24;
// 			struct tm *newtime = gmtime((const time_t *)&dt);
// 		
// 			if (newtime == 0)
// 			{
// 				CloseHandle(f_CreateIndex) ;
// 				f_CreateIndex = NULL ;
// 				return 1 ;
// 			}
// 
// 			tmp_index->time.second = newtime->tm_sec +1 ;
// 			tmp_index->time.minute		= newtime->tm_min + 1;
// 			tmp_index->time.hour		= newtime->tm_hour + 1;
// 			tmp_index->time.day		= newtime->tm_mday;
// 			tmp_index->time.month		= newtime->tm_mon+1;
// 			tmp_index->time.year		= newtime->tm_year - 100;


			tmp_index->framelen = 1024 ;
			tmp_index->IFrameNum = 0 ;
			tmp_index->IFramePos = temp_pCurPos ;
			tmp_index->IFrameRate = 0 ;
			m_pFileIndex.push_back(tmp_index) ;
			temp_pCurPos += 1040 ;
		}
		else if (mData[3] == 0xf0)
		{
			temp_pCurPos += 16 + *(WORD *)&mData[6];    //7~8字节表示数据大小
		}
		else
		{
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1;                 //错误
		}		
	}

	return 1;
}


__int64 ShStreamFileOpr::GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, __int64 KeyFramePos, 
		DWORD nFrame)  //得到指定帧的结束位置
{
	return -1 ;
}

void ShStreamFileOpr::GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info)
{
	static const int BUFLEN = 4116;
	static const int HEADERLEN = 20;
	static const BYTE V_Frame[4] = {0x00,0x00,0x01,0xF2} ;
	static const BYTE A_Frame[4] = {0x00,0x00,0x01,0xF0} ;

	unsigned int Audio_Samples[] = {4000, 8000, 11025, 16000, 20000, 22050, 32000, 44100, 48000};

	BYTE buf[BUFLEN] ;
	int rate = 25;
	BYTE *pos ;	
	DWORD len;

	__int64 m_pCurPos = 0;
	__int64 m_nDataLen = GetFileSize(m_pFile, NULL) ;

	bool bVideoFlag = true;
	bool bAudioFlag = true;
	
	while (bVideoFlag || bAudioFlag)
	{
		if (m_pCurPos > m_nDataLen - BUFLEN || (m_pCurPos >= 4194304))
		{
			if (!bVideoFlag)
			{
				dwFrame = 0;
				break;
			}
			
			break;
		}

		myFileSeek (m_pFile, m_pCurPos, FILE_BEGIN) ;
		ReadFile(m_pFile, buf, BUFLEN, &len, NULL) ;

		pos = buf;
			
		while ( (pos = (BYTE *)memchr(pos, 0, BUFLEN-HEADERLEN - (pos - buf))) != NULL)   //先找到第一个字符
		{
			//新码流
			if (bVideoFlag && (memcmp(pos, V_Frame, 4) == 0))
			{
				bVideoFlag = false;
				*(DWORD*)(&(_video_info.DirStartTime)) = *(DWORD *)(pos+8);   //9~12字节表示时间

				_video_info.VideoType	= 0; //HB, do not know encode type
				_video_info.Framerate   = *(pos+5) & 0x1F;

				int size = (*(pos+6)) | (*(pos+7) << 8) ;
				_video_info.VideoWidth	= ((size>>3)&0x3f)*16;
				_video_info.VideoHigh	= ((size>>9)&0x7f)*16;
				
				pos++;
			}
			else if (bAudioFlag && (memcmp(pos, A_Frame, 4) == 0))
			{	
				bAudioFlag = false;
				_video_info.AudioChannel = 1;
				
				_video_info.AudioType = *(pos+4);
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

		m_pCurPos += (BUFLEN - HEADERLEN);
	}

	bVideoFlag = true;
	m_pCurPos = m_nDataLen-BUFLEN-1;

	while(bVideoFlag)
	{
		if (m_pCurPos < BUFLEN || (m_nDataLen-m_pCurPos) >= 2097152)
		{
			memset(&(_video_info.DirStartTime), 0, sizeof(DHTIME));
			return;
		}

		myFileSeek (m_pFile, m_pCurPos, FILE_BEGIN) ;
		ReadFile(m_pFile, buf, BUFLEN, &len, NULL) ;

		pos = buf ;
			
		while ( (pos = (BYTE *)memchr(pos, 0, BUFLEN-HEADERLEN - (pos - buf))) != NULL)   //先找到第一个字符
		{
			if (memcmp(pos, V_Frame, 4) == 0)
			{
				bVideoFlag = false;
				*(DWORD*)(&(_video_info.DirEndTime)) = *(DWORD *)(pos+8);   //9~12字节表示时间
				rate = pos[5];

				int second = _video_info.DirEndTime.second;
				_video_info.DirEndTime.second = (second > 60) ? 60 : second;
				//dwFrame = GetTimeInterVal(_video_info.DirEndTime, _video_info.DirStartTime) * rate;
				break ;
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

		m_pCurPos -= (BUFLEN - HEADERLEN);
	}
}