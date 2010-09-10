  
/********************************************************************
	created:	2005/12/09
	created:	9:12:2005   15:06
	filename: 	data.h
	file base:	data
	file ext:	h
	author:		
	
	purpose:	
*********************************************************************/

#ifndef DATA_H
#define DATA_H

#include "dhplay.h"
#include "fileoprinterface.h"
#include "decode/playgraph.h"
//文件打开模式
typedef enum
{
	FM_NOOPEN = -1,//文件未打开
	FM_FILE = 0,
	FM_STREAM,	
}DATA_MODE;

//视频类型，暂时只支持h264数据格式播放
enum 
{
	DH_MPEG1,
	DH_MPEG4,
	DH_H264,
	DH_MPEG2,
	DH_H263,
	DH_MJPG,
};

#define MAX_FILE_LENGTH  0xffffffff	//4*1024*1024*1024

typedef struct _INDEX_INFO_EXT
{
	DHTIME StartTime ;
	DHTIME EndTime ;
	DWORD TotalFrameNum ;
	DWORD StreamPos ;
	DWORD StreamdataType ;
}INDEX_INFO_EXT ;
/*/视频头结构
typedef struct _DH_VIDEOFILE_HDR
{
	BYTE	   CorpInfo[8];           //公司信息(DAHUA)
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
}DH_VIDEOFILE_HDR;*/

//打开文件的可能情况
typedef enum _FILE_OPEN_ERROR
{
	FILE_CANNOT_OPEN, //文件打不开
	FILE_HEADER_ERROR,//文件头不存在
	FILE_INDEX_ERROR, //不能建立索引
	FILE_NO_ERROR     //文件打开正确，可以建立索引
}FILE_OPEN_ERROR ;

class NewStreamFileOpr ;
class OldStreamOperation ;
class ShStreamFileOpr ;
class StandardStreamFirOpr ;
class DHSTDStreamFileOpr;
////////////////////////////////////////////////////////////////////
class CDHAVData
{
	//文件模式数据相关的成员变量
	HANDLE m_pFile ; //文件指针 仅对文件有效
	char filename[_MAX_PATH+_MAX_FNAME] ;//文件名
	DWORD m_nTotalFrameNum; //总桢数，包括I桢和P桢
	DWORD m_nFileTime ;//文件总时间
	__int64  m_pCurPos;        //当前读指针位置
	__int64  m_nDataLen ;      //数据长度
	__int64  m_EndPos ;		   //帧定位时，指定帧的结束位置	

	std::vector<DH_INDEX_INFO*> m_pFileIndex ;//文件索引信息
	DH_VIDEOFILE_HDR m_sfile_hdr ;   //虚拟文件头，用以记录开始时间、结束时间、文件开始位置

	IFileOperator* m_fileOpr ;
	NewStreamFileOpr* m_newstreamfileopr ;
	OldStreamOperation* m_oldstreamfileopr ;
	ShStreamFileOpr* m_shstreamfileopr ;
	StandardStreamFirOpr* m_standardfileopr ;
	DHSTDStreamFileOpr* m_dhstdstreamfileopr;
	RECT      m_MDRect ;
	DWORD m_ThreShold ;
	DWORD m_MDFrameBegin ;
	DWORD m_MDFrameEnd ;
	//流模式数据相关的成员变量
	BYTE* m_pStreamBuf ;    //文件流缓冲指针
	DWORD m_nBufLen;        //数据流缓冲大小
	DWORD m_pWrite;         //数据流写偏移最大2*m_nBufLen
	DWORD m_pRead;          //数据流读偏移最大m_nBufLen

	//公共属性成员
	DATA_MODE m_nOpenMode;  //视频打开模式
	STREAMDATA_TYPE m_streamtype ;//码流类型，流码流和旧码流（LB之类）

	HANDLE m_hCreateIndex ;

public:
	int m_port;
	BOOL m_bIsIndexCreated;
//主要成员函数有：	
public:
	CDHAVData();
	~CDHAVData();

	///////////文件操作接口/////////////
	FILE_OPEN_ERROR Open(LONG nPort, LPSTR strName);              //打开文件
	BOOL     ReOpen() ;                                 //重新打开文件，暂时只作重置文件起始位置的处理
	int		     Close();                                   //关闭文件
	int          CreateIndex();                             //建立文件索引,因码流而不同
	BOOL     GetIndexInfo(BYTE *pBuffer, DWORD *pSize); //获取索引信息
	BOOL     SetIndexInfo(BYTE *pBuffer, DWORD nSize);  //设置索引信息	
	float       GetPos();                                   //获得当前位置
	BOOL     SetPos(float fRelativePos) ;                //设置文件位置
	BOOL	GetDataPos(float fRelativePos, DWORD *pCurDataPos); //根据播放百分位, 获取当前数据指针位置
	BOOL	GetRelativePos(DWORD dCurDataPos, float *pfRelativePos); //根据文件数据指针位置，获取播放百分位
	DWORD GetTotalFrames();                           //获得总桢数	
	BOOL     GetKeyFramePos(DWORD nValue, DWORD nType, PFRAME_POS pFramePos, int* KeyFrameRate = NULL);//得到前一个关键帧（I帧）的相关信息
	BOOL     GetNextKeyFramePos(DWORD nValue, DWORD nType, PFRAME_POS pFramePos);//得到下一个关键帧（I帧）的相关信息
	int          GetPlayedTime(DWORD frameNum, bool ByFrameNum = true) ;          //根据桢序号，得到播放的时间	
	DWORD GetFileTotalTime();                      //获取文件总的时间长度
	BOOL     IsIndexCreated() ;                       //文件是否建立索引           
	BOOL     ResetIndex() ;                           //清空索引信息
	BOOL     SetMDRange(DWORD nValueBegin, DWORD nValueEnd, DWORD nType, RECT* MDRect) ;
	BOOL     SetMDThreShold(DWORD ThreShold) ;
	DWORD GetMDPosition(DWORD direction, DWORD nFrame, DWORD* MDValue) ;
	//用于定位处理,从给定的帧序号开始播放。是这样实现的：先用SetCurrentFrameNum把文件
	//指针m_pCurPos置为该帧前一个I帧的的位置，用GetFrameEndPos得到该帧的结束位置，
	//用SetEndPos把endPos置为该帧的结束位置，再用ReadOneFrame读取m_pCurPos到endPos之间的数据
	DWORD SetCurrentFrameNum(DWORD nFrameNum);       //根据桢序号设置位置	
	BOOL     SetEndPos(DWORD endPos) ;                  //设置ReadOneFrame的结束位置
	int          ReadOneFrame(BYTE *pBuf) ;				 //读取要定位的帧前一I帧到该帧的数据
	__int64 GetFrameEndPos(DWORD nFrame);              //得到指定帧的结束位置，因码流而不同
	BOOL     ChangePos(DWORD newpos){m_pCurPos = newpos ;return TRUE ;} ;
	//////////流操作接口///////
	BOOL     OpenStream(BYTE *pBuf, DWORD dwSize, DWORD dwPoolSize);   //打开流接口
	BOOL      InputData(PBYTE pBuf,DWORD nSize);   //输入数据流
	BOOL     CloseStream();         //关闭流
	DWORD GetBuffSize();         //得到缓冲大小
	DWORD GetBufferRemain();     //获取剩余缓冲大小
	BOOL     ResetRemainData();     //清除剩余数据

	/////////通用接口//////////
	int          Read(BYTE *pBuf, DWORD dwSize);           //读取数据
	DATA_MODE GetDataOpenMode(){return m_nOpenMode;}
	STREAMDATA_TYPE GetStreamDataType(){return m_streamtype ;}

	BOOL GetMediaInfo(char* pBuf, int len);
	/////////
protected:

};
#endif