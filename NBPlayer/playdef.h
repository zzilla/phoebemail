#ifndef PLAYDEF_H
#define PLAYDEF_H


/************************************************************************
 * 帧类型
 ************************************************************************/
#define DEC_DEMUX_TYPE_VIDEO_FRAME	1 // 视频帧
#define DEC_DEMUX_TYPE_AUDIO_FRAME	2 // 音频帧
#define DEC_DEMUX_TYPE_DATA_FRAME	3 // 自定义数据帧

// 视频帧子类型
#define DEC_VIDEO_I_FRAME		1 // 关键帧
#define DEC_VIDEO_P_FRAME		2
#define DEC_VIDEO_B_FRAME		3

// 编码类型
#define DEC_ENCODE_MPEG4	1		// MPEG4编码
#define DEC_ENCODE_H264		2		// 大华H264编码
#define DEC_ENCODE_H264_ADI	3		// ADI H264编码
#define DEC_ENCODE_H264_STD	4		// 标准H264编码

/************************************************************************
 * 数据拆分结果返回的结构体, 各种帧均使用该结构,其
 * 中帧率信息和时间信息仅仅对I帧有效
 * 图像大小仅仅对视频有效
 * 序列号目前没有使用 
 ************************************************************************/
struct tagDemuxInfo {
	int type;			// 见帧类型宏定义
	int subtype;		// 见视频帧子类型宏定义
	int encode;			// 见编码类型宏定义
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
	int second;
	unsigned long timestamp;
	
	int bitspersample;
	int samplespersecond;
};

#define PLAY_CMD_GetTime 1

struct TimeInfo
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

#endif /*DHDEC_H*/

