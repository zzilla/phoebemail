#ifndef _DH_FILEOPR_INTERFACE
#define _DH_FILEOPR_INTERFACE

#include <vector>
#include <time.h>
#define interface struct 

typedef struct _DHTIME								
{
	DWORD second		:6;					//	秒	1-60		
	DWORD minute		:6;					//	分	1-60		
	DWORD hour			:5;					//	时	1-24		
	DWORD day			:5;					//	日	1-31		
	DWORD month			:4;					//	月	1-12		
	DWORD year			:6;					//	年	2000-2063	
}DHTIME,*pDHTIME;

//视频头结构
typedef struct _DH_VIDEOFILE_HDR
{
	BYTE	  CorpInfo[8];           //公司信息(DAHUA)
	WORD      VerMajor;              //主版本
	WORD      VerMinor;              //副版本
	DWORD     ProductType;			 //产品类型
	struct{
		BYTE   ProductMajor;         //产品主版本
		BYTE   ProductMinor;         //产品副版本
		WORD   ProductPatch;         //补丁版本
	}ProductVersion;                //产品版本信息
	DWORD     AudioType;            //音频类型(PCM /ADPCM)
	WORD      AudioChannel;         //音频声道数
	WORD      AudioBitPerSample;    //音频采样位数
	DWORD     AudioSamplesPerSec;   //音频采样率
	DWORD     VideoType;            //视频类型(MPEG1/MPEG4/H.264)
	WORD      VideoWidth;           //视频宽度
	WORD      VideoHigh;            //视频高度
	WORD      VideoMode;            //制式(NTSC / PAL)
	WORD      Framerate;            //帧率
	DHTIME    DirStartTime;		 	 //开始时间
	DHTIME    DirEndTime;			 //结束时间
	DWORD     StreamPos;            //指示流数据（音视频）开始位置
	BYTE      Reserved[64];         //保留空间。
	DWORD     ExtendID;              //表示扩展块类型的四字符码(如：CARD，IDX，LIST)
	DWORD     ExtendSize;           //扩展数据块的长度
}DH_VIDEOFILE_HDR;

//索引信息结构，用于以不同的方式定位
typedef struct _INDEX_INFO{
	DWORD  IFramePos;    //I桢位置偏移
	DWORD  framelen ;    //I帧的数据长度，包括帧头，在GetNextKeyFramePos这个函数里有用
	DWORD  IFrameNum;    //I桢桢序号
	DWORD  IFrameRate ; //I帧帧率，用以估算P帧时间
	DHTIME time;        //I桢数据的时间
	
}DH_INDEX_INFO;


static __int64 myFileSeek (HANDLE hf, __int64 distance, DWORD MoveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = distance;

   li.LowPart = SetFilePointer (hf, li.LowPart, &li.HighPart, MoveMethod);

//   if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
//   {
//      li.QuadPart = -1;
//   }

   return li.QuadPart;
}

static DWORD GetTimeInterVal(DHTIME dhETime, DHTIME dhSTime)
{
	tm stime, etime ;
	//由于大华所定义的时间与标准时间结构有区别，所以要做下转换
	stime.tm_mday    = dhSTime.day ;
	stime.tm_mon     = dhSTime.month - 1;
	stime.tm_year    = dhSTime.year + 100 ;
	stime.tm_sec     = dhSTime.second - 1;
	stime.tm_min     = dhSTime.minute - 1;
	stime.tm_hour    = dhSTime.hour - 1  ;
	stime.tm_isdst   = -1;////自动计算是否为夏时制, (bug: 时区选为美国，自动使用夏时制时，时间计算有误)

	etime.tm_mday    = dhETime.day ;
	etime.tm_mon     = dhETime.month - 1;
	etime.tm_year    = dhETime.year + 100;
	etime.tm_sec     = dhETime.second - 1;
	etime.tm_min     = dhETime.minute - 1;
	etime.tm_hour    = dhETime.hour - 1  ;
	etime.tm_isdst   = -1;//自动计算是否为夏时制, (bug: 时区选为美国，自动使用夏时制时，时间计算有误)

	stime.tm_year = (stime.tm_year >= 138)? 137 : stime.tm_year;
	etime.tm_year = (etime.tm_year >= 138)? 137 : etime.tm_year;

	time_t start_time, end_time ;
	start_time = mktime(&stime) ;
	end_time   = mktime(&etime) ;

	return ((end_time - start_time) > 0) ? (end_time - start_time) : 0  ;
}

interface IFileOperator
{
	virtual  int   CreateIndex(char* fileName, DWORD beginpos,std::vector<DH_INDEX_INFO*>& m_pFileIndex, 
		DWORD& m_totalFrameNum, DHTIME& begintime, DHTIME& endtime, __int64& nDataLen) = 0 ;//建立索引，把索引信息放在m_pFileIndex里

	virtual  __int64 GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, 
		__int64 KeyFramePos, DWORD nFrame) = 0;              //得到指定帧的结束位置	

	virtual	 void   EndCreateIndex(){m_bCreateIndex = 0 ;}

	virtual void GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info) = 0;

	IFileOperator(){ m_bCreateIndex = 1 ;}

	int m_bCreateIndex ;
};


#endif