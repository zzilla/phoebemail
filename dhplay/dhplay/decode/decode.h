#ifndef _DECODE_H
#define _DECODE_H

#include <windows.h>
#include "..\AccurateTimer.h"


#define  AUDIOBUFLEN (8192*2)

#define DEC_YUVDATA     1//解码视频数据
#define DEC_PCMDATA    2//解码音频数据

#define Reset_level_RAWYUV  0  //清空原始缓冲和解码缓冲
#define Reset_level_YUV           1  //清空解码缓冲
#define Reset_level_FASTRETURN 2 //使解码快速返回
#define Reset_level_STREAMPASER 3 //重置缓冲和码流解析器，重新进行码流识别

#define SM_File 0
#define SM_Stream_File 1
#define SM_Stream_Stream 2

#define SAFE_DELETE(x)				{if ((x)!=NULL) {delete (x); (x)=NULL;}} 
#define SAFE_DELETEBUFF(x)			{if((x)!=NULL) {delete [] (x); (x) = NULL;}}
#define SAFE_CLOSEHANDLE(x)			{if((x)!=NULL) {CloseHandle(x); (x) = NULL;}}


typedef enum
{
	Stream_No,
	Stream_NewStream,
	Stream_OldStream,
	Stream_ShStream,
	Stream_Standard,
	Stream_DHSTD
}STREAMDATA_TYPE ;

typedef int (__stdcall *dec_callback)(
        int index,				// 索引号
		unsigned char *data,	// 数据体
		unsigned int datalen,	// 数据长度
		int cbtype,				// 回调类型
		int param1,			// 回调参数，如果是视频数据为图像宽，如果是音频数据为数据宽度
		int param2//回调参数，如果视音频数据为图像高，如果是音频数据为采样率
);

struct DecodeOption
{
	int decId;			// 给定的ID号, 用于一个应用程序与多个解码对象通信
	dec_callback cb;	// 注册回调函数
	int imgBufCount;	// 图像缓冲块数
	int transMode;		// 传递模式. 流媒体模式或者文件模式. 流媒体模式 = 1,文件模式 = 0
	int streamType;		// 流类型，暂时没用到
	bool ifDecOnly;    //是否仅仅解码，不通过定时器回调数据
	HWND hwnd;
	CAccurateTimer *pACTimerObj;
};

struct tagDemuxInfo {
	int type;			// VIDEO, AUDIO, DATA
	int subtype;		// I Frame, BP Frame, PCM8, MS-ADPCM etc.
	int encode;			// MPEG4, H264, STDH264
	int sequence;		// I帧间隔里面的序号
	
	// 图像大小信息
	int width;
	int height;
	
	// 帧率信息
	int rate;
	
	// time information
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int secode;
	unsigned long timestamp;

	int channels;
	int bitspersample;
	int samplespersecond;
};

typedef struct 
{
	unsigned long 		key;			// 水印key值 可根据不同的key值来判断水印是文字，
								// 图片，或错误水印.
	unsigned short		len;			// 水印长度
	unsigned short		reserved;		// 保留字段
	unsigned short		reallen;		// 缓冲中实际水印长度
	char*				buf;			// 水印数据缓冲
}WATERMARK_INFO;			// 水印信息结构


typedef int (__stdcall*watermark_callback)(char* buf, long key, long len, long reallen, long reserved, long nUser);	//水印信息获取函数


//智能分析器begin-----------------------
#define  MAX_IVSOBJ_NUM 200
#define  MAX_TRACKPOINT_NUM 10

typedef struct _DH_IVS_POINT
{
	short 		x; 
	short 		y; 
	short		xSize;
	short		ySize;
	//轨迹点是物体外接矩形的中心，根据X，Y及XSize，YSize计算出的物体外接矩形坐标（left，top，right，bottom）：
	//RECT=(X-XSize, Y-YSize, X+XSize, Y+YSize)
	
}DH_IVS_POINT; 

typedef struct _DH_IVS_OBJ
{
	int				decode_id;
	int				obj_id;
	int				enable;//0表删除
	DH_IVS_POINT 	track_point[MAX_TRACKPOINT_NUM]; 
	int				trackpt_num;
}DH_IVS_OBJ;

typedef struct _DH_IVS_PREPOS
{
	BYTE* pIVSPreposInfo;
	long  lInfoSize;
} DH_IVS_PREPOS;

//
// IVS information callback function. This function returns preset position or motion tracks informations.
// parameters: buf			information data
//             type			information type. IVSINFOTYPE_PRESETPOS or IVSINFOTYPE_MOTINTRKS
//             len			IVS information single data object size
//             reallen		IVS information data object total size
//             reserved		only for motion tracks, 1 -- normal track    2 -- lost track
//             nUser		user data
//
typedef int (__stdcall*IVS_callback)(char* buf, long type, long len, long reallen, long reserved, long nUser);

//智能分析器end-----------------------


#endif