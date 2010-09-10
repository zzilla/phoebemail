#include "stdafx.h"
#include "oldStreamfileOpr.h"
#include <time.h>

OldStreamOperation::OldStreamOperation()
{
}

OldStreamOperation::~OldStreamOperation()
{
}

int OldStreamOperation::CreateIndex(char* fileName, DWORD beginpos, 
		std::vector<DH_INDEX_INFO*>& m_pFileIndex, DWORD& m_totalFrameNum, 
		DHTIME& begintime, DHTIME& endtime,__int64& nDataLen)
{
	HANDLE f_CreateIndex = NULL;
	unsigned int FOURCC = 0xFFFFFFFF;
	unsigned char head[50];
	unsigned char* pBuf = 0;
	int rest = 0;
	DWORD FileReadLen = 0;
	BOOL FileReadResult = TRUE;
	DWORD FrameLen = 0;
	DWORD FileSize = 0;
	unsigned int FrameOffset = beginpos;
	unsigned int VideoFrameIndex = 0;
	
	DH_INDEX_INFO* pIndexInfo = 0;
	
	//打开文件
	f_CreateIndex = CreateFile(fileName, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (f_CreateIndex == NULL)
	{
		return 1 ;
	}	
	
	//得到文件长度
	FileSize = GetFileSize(f_CreateIndex, NULL);
	if(FileSize == -1)
	{
		CloseHandle(f_CreateIndex) ;
		f_CreateIndex = NULL ;
		return 1 ;	
	}

	//索引到第一个帧头
	if(	myFileSeek(f_CreateIndex, FrameOffset, FILE_BEGIN) == -1)	
	{
		CloseHandle(f_CreateIndex) ;
		f_CreateIndex = NULL ;
		return 1 ;
	}
	
	//建立索引
	while(true)
	{
		if (m_bCreateIndex == 0)
		{
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}
		//读去8字节的头（包括帧标识和帧长度）
		FileReadResult = ReadFile(f_CreateIndex, head, 50, &FileReadLen, NULL);

		if (FileReadLen != 50 || FileReadResult == false)
		{
			//读帧头不符合我们的要求，退出
			m_totalFrameNum = VideoFrameIndex ;
			if (m_totalFrameNum != 0)
			{
				goto IndexCreateOK ;
			}
			else
			{
				return 1;
			}
		}
		
		//取标识
		FOURCC = head[0] << 24 | head[1] << 16 | head[2] << 8 | head[3];
		
		if (FOURCC == 0x44485054)// "DHPT"如果是视频帧
		{	
			//得到视频帧长度
			FrameLen = head[7] << 24 | head[6] << 16 | head[5] << 8 | head[4];
			//如果此帧不完整，退出
			if(FrameOffset + FrameLen + 8 > FileSize || FrameLen > FileSize)
			{
				m_totalFrameNum = VideoFrameIndex ;
				if (m_totalFrameNum != 0)
				{
					goto IndexCreateOK ;
				}
				else
				{
					return 1;
				}
			}
			
			//此帧完整的话，判断其是关键帧（I帧）还是非关键帧
			pBuf = head + 8;//跳到实际的视频数据开头
			FOURCC = pBuf[0] << 24 | pBuf[1] << 16 | pBuf[2] << 8 | pBuf[3];
			
			if(FOURCC == 0x00000100)//如果是关键帧就建立索引
			{
				unsigned int ID = 0xFFFFFFFF;
				
				pIndexInfo = new DH_INDEX_INFO;
				memset(pIndexInfo, 0, sizeof(DH_INDEX_INFO));

				pIndexInfo->IFramePos = FrameOffset;
				pIndexInfo->framelen = FrameLen + 8;
				pIndexInfo->IFrameNum = VideoFrameIndex++;
				memset((void*)&(pIndexInfo->time),0,sizeof(DHTIME)) ;
				m_pFileIndex.push_back(pIndexInfo);//添加到索引队列

				while (ID != 0x000001B6 && pBuf <= head + 50) 
				{
					ID = ID << 8 | *pBuf++;
				
					if (ID == 0x01B3)
					{
						pIndexInfo->time.hour	= pBuf[0]>>3 + 1;
						pIndexInfo->time.minute	= (pBuf[0]&0x07)<<3 | pBuf[1]>>5 + 1;
						pIndexInfo->time.second	= (pBuf[1]&0x0f)<<2 | pBuf[2]>>6 + 1;
					}
					else if (ID == 0x01B2)
					{
						pIndexInfo->IFrameRate = pBuf[6] ;
						
						if (pBuf[6] > 150)
						{
							if(pBuf[6] == 0xFF)
							{
								pIndexInfo->IFrameRate = 1;
							}
							else
							{
								pIndexInfo->IFrameRate = 25;
							}
						}		
					}
				}
			
				FrameOffset += FrameLen + 8;//指向下一帧开始的偏移
			}
			else if(FOURCC == 0x000001B6)//如果不是关键帧，只是做相应的偏移
			{
				if(VideoFrameIndex != 0)
				{
					VideoFrameIndex++;			
				}
				FrameOffset += FrameLen + 8;	
			}
		} 
		else if (FOURCC == 0x000001f0)//如果是音频帧，只是做相应的偏移
		{
			//得到视频帧长度
			FrameLen = head[5] << 8 | head[4];
			//如果此帧不完整，退出
			if(FrameOffset + FrameLen + 8 > FileSize)
			{
				m_totalFrameNum = VideoFrameIndex ;
				if (m_totalFrameNum != 0)
				{
					goto IndexCreateOK ;
				}
				else
				{
					return 1;
				}
			}
			
			FrameOffset += FrameLen + 8;
		} 
		else 
		{
			//文件中只要码流有错，就放弃建立索引
			return 1 ;
		}
	
		//索引到下一个帧头
		if (myFileSeek(f_CreateIndex, FrameOffset, FILE_BEGIN) == -1)
		{	
			CloseHandle(f_CreateIndex) ;
			f_CreateIndex = NULL ;
			return 1 ;
		}	
	}	

IndexCreateOK:
	*(DWORD*)&begintime = *(DWORD*)&(m_pFileIndex[0]->time) ;
 	*(DWORD *)&endtime 	 = *(DWORD *)&(m_pFileIndex[m_pFileIndex.size()-1]->time) ;

	tm* tmp_time = new tm ;
		
	tmp_time->tm_hour = endtime.hour - 1 ;
	tmp_time->tm_min  = endtime.minute - 1 ;
	tmp_time->tm_sec  = endtime.second - 1 ;
	tmp_time->tm_mday = 0;
	tmp_time->tm_mon  = 0 ;
	tmp_time->tm_year  = 100 ;
	tmp_time->tm_isdst= -1; //自动计算是否为夏时制， (bug: 时区选为美国，自动使用夏时制时，时间计算有误)
	
	time_t _time_t = mktime(tmp_time) ;

	delete tmp_time ;

	int tmp_rate = m_pFileIndex[m_pFileIndex.size()-1]->IFrameRate ;

	_time_t += (m_totalFrameNum - m_pFileIndex[m_pFileIndex.size()-1]->IFrameNum - 1) / tmp_rate   + 1 ;

	tmp_time = localtime(&_time_t) ;
	
	if (tmp_time != NULL)
	{
		endtime.hour = tmp_time->tm_hour + 1 ;
		endtime.minute = tmp_time->tm_min + 1 ;
		endtime.second  = tmp_time->tm_sec + 1 ;
	}

	pIndexInfo = new DH_INDEX_INFO ;
	*(DWORD *)&pIndexInfo->time = *(DWORD *)&endtime;  				
	pIndexInfo->IFramePos = FileSize ;
	pIndexInfo->IFrameNum = m_totalFrameNum  ;
	pIndexInfo->framelen  = 0 ;
	pIndexInfo->IFrameRate = 25 ;
	m_pFileIndex.push_back(pIndexInfo) ;
			 
	CloseHandle(f_CreateIndex) ;
	f_CreateIndex = NULL ;

	return 0 ;
}

__int64 OldStreamOperation::GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, __int64 KeyFramePos, DWORD nFrame) //得到指定帧的结束位置
{	
	BYTE mData[50];
	DWORD len = 0 ;
	DWORD FrameNum		= KeyFrameNum ;
	__int64 temp_pos		= KeyFramePos ;
	__int64 FileSize	= GetFileSize(m_pFile,NULL) ;
	int FrameLen = 0;
	unsigned int FOURCC = 0xFFFFFFFF;

	while(TRUE)
	{
		if (temp_pos >= FileSize)
		{
			return -1 ;
		}

		myFileSeek(m_pFile, temp_pos, FILE_BEGIN) ;

		ReadFile(m_pFile, mData, 50, &len, NULL)  ;

		if (len < 50)
		{
			return -1 ;
		}
		
       FOURCC = mData[0] << 24 | mData[1] << 16 | mData[2] << 8 | mData[3];
	   if(FOURCC == 0x44485054)
	   {		   	
			//得到视频帧长度
			FrameLen = mData[7] << 24 | mData[6] << 16 | mData[5] << 8 | mData[4];
			//如果此帧不完整，退出
			temp_pos += FrameLen + 8 ;
			if(temp_pos  > FileSize)
			{
				return -1;
			}
			
			if(FrameNum == nFrame)
			{
				return temp_pos ;
			}	

			FrameNum++ ;
	   }
	   else if(FOURCC == 0x000001F0)
	   {
			//得到音频帧长度
			FrameLen =  mData[0] << 8 | mData[1];
			//如果此帧不完整，退出
			temp_pos += FrameLen + 8 ;
			if(temp_pos + FrameLen + 8 > FileSize)
			{
				return -1;
			}
	   }
	}
}

void OldStreamOperation::GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info)
{

}