#ifndef PLAYGRAPH_H
#define PLAYGRAPH_H

#include "decode.h"
#include "interface.h"
#include "videorender.h"
#include "FifoBuffer.h"
#include "h264videodecode.h"
#include "../depend/DllDeinterlace.h"
//#include "hisih264videodecode.h"
#include "mp4decoder.h"
#include "DhStreamParser/DhStreamParser.h"
#include "audiodecode.h"
//#include "analyse.h"
#include <time.h>
#include <vfw.h>

typedef struct _SHELTERINFO
{
	int SX;
	int SY;
	int W;
	int H;
	BYTE Y;
	BYTE U;
	BYTE V;
	BOOL bEn;
}SHELTERINFO;


typedef struct
{
	int m_decID ;
	PAVIFILE m_hAVIFile;
	PAVISTREAM m_hVideoStream;
	PAVISTREAM m_hAudioStream;
	unsigned int m_aviVideoWritePos;
	unsigned int m_aviAudioWritePos;

	bool m_foundIFrame;
	
	AVISTREAMINFO strhdr;

	XVID_ENC_PARAM enc_param;
	XVID_ENC_FRAME enc_frame;
	XVID_ENC_STATS enc_stats;

	unsigned char * m_encBuffer;
}AVICONVERTINFO;

//智能分析帧
typedef struct 
{
	DWORD x;
	DWORD y;
}POSITION_;

typedef struct
{
	char alerttime[100];
	char entrytime[100];
	DWORD event;
	POSITION_ position;
	DWORD width;
	DWORD height;
	DWORD direction;
	DWORD speed;
}IVALERT;

typedef struct 
{
	POSITION_ p0;
	POSITION_ p1;
	DWORD direction;
	BOOL bEnable;
}TRIP;

typedef enum
{
	ENUM_FILE_CHANGE = 0,  //分辨率发生改变
	ENUM_FILE_RATE,
	ENUM_FILE_ENC,
	ENUM_FILE_RATE_AND_FIX,
}FILE_CHANGE_ENUM;

#define MAX_TRIP_NUM 20
#define MAX_IVALERT_NUM 10

typedef	int (*MP4_Xvid_Encore)(void *handle, int opt, void *param1, void *param2);
//! 解码管理器
class DhPlayGraph
{
public:
	//! 构造函数
    DhPlayGraph();

	// 析构函数
    virtual ~DhPlayGraph();

	//! 初始化工作流图
    int init(DecodeOption *info); 
	
	//! 启动解码
    int start();

	//! 暂停解码回调
    int pause();

	//! 停止解码
	int stop();

	//! 填充解码缓冲区
    int parseData(unsigned char *data, unsigned int len);

	//! 单步执行
    int stepGo();

	//! 更改显示帧率
    int changeRate(int rate);
	
	//获得帧率
	int getRate();

	int getFileRate();
	//得到指定帧数据
	BYTE* GetLocateFrameData(int* width, int* height) ;

	//! 重置
	void reset(int level);
	
	//得到已解码帧数
	int GetImgCount(int* imgcount) ;

	//设置定位标记
	void SetIfFrameUsed(BOOL IfUsed) ;

	//是否解码视频
	void SetDecVideo(BOOL IfDecVideo) ;
	
	//是否解码音频
	void SetDecAudio(BOOL IfDecAudio) ;

	time_t GetCurTimeEx() ;

	void AdjustFluency(int bufnum, int adjustrange);

	unsigned char* GetLastFrame();

	bool FixRate(int rate);
	
	void GetTimeStr(char* pBuf);

	void SetWaterMarkCallback(void *pvCallback, long userData);

	void SetPandoraWaterMarkCallback(void *pvCallback, long userData);

	void ConvToAVI(unsigned char* pAVIFileName);

	void SetAVIConvSize(long lWidth, long lHeight);
	
	void StopAVIConv();

	void RealStop();

	void SetBeginTime(DWORD timeBegin){m_BeginTime = timeBegin;}

	void NotifyRendering();

	DWORD GetHandledDataLen();
	void  SetHandledDataLen(DWORD dCurDataPos);

	inline void SetStepGoStatus(BOOL bStatus) { m_IfStepGo = bStatus ;}

	BOOL WaterMarkVerify(DH_FRAME_INFO* fFrameInfo);

	BOOL GetMediaInfo(char* pBuf, int len);

	void SetIVSCallback(void *pIVSFunc, long nUserData);

	double GetFrameBitRate(void) { return m_video_render.GetRealFrameBitRate(); }

private:
	unsigned char*  m_yuvBuf ;//暂存解码数据
	unsigned char*	m_preBuffer;
	unsigned char*  m_audioBuf;//音频数据AUDIOBUFLEN
	unsigned char*	m_lastYUVBuf;

	int m_width ;//图像宽
	int m_height ;//图像高

	DhAVFrame* m_item ;
	unsigned int m_encode_type ;//解码类型,h264、MP4、PCM
	DH_FRAME_INFO* m_frameinfo ;

	DhVideoRender   m_video_render ;//视频数据定时回调

	//解码器
	IDecode* m_Decoder ;
    //HisiH264VideoDecoder* m_hisih264videoDecoder ;//海思h264解码器	
	H264VideoDecoder* m_h264videoDecoder ;//H264解码
	Mpeg4Decoder * m_hMp4Decoder ;//MPEG解码
	DhAudioDecoder* m_audioDecoder;//音频解码

	//流分析器
	DhStreamParser* m_streamParser ;

	dec_callback m_callback;//回调指针

	int m_port ;//通道号
	unsigned int m_filerate ;//文件中的帧率
//	unsigned int m_tmpfilerate;
	int m_userrate ;//用户设定的速度级别，快放一、快放二。。。慢放一、慢放二。。。
	int m_transMode ;
	int m_deinterlace ;
	int m_h264flag ;//0 大华264   1 标准264

	//控制变量
	BOOL m_IfDecOnly ;//只解码，不通过定时器
	BOOL m_IfDecPause ;
	BOOL m_IfStepGo ;
	BOOL m_IfUsed ;//用于帧定位时，跳过指定帧前面的帧
	BOOL m_IfPauseData ;
	BOOL m_IfFindIFrame ;
	BOOL m_IfFindFirstIFrame ;
	BOOL m_IfFindAFrame;
	BOOL m_IfDecVideo ;
	BOOL m_IfDecAudio ;
	BOOL m_IfFirstFrame;
	time_t m_FirstIFrameTime ;
	time_t m_curtime ;
	time_t m_preIFrameTime;
	time_t m_BeginTime;
	int m_PframeNum ;
	DWORD m_dwFrameNum;
	BOOL  m_bFrameNumScan;

	int m_adjust_range;
	int m_adjust_bufnum;

	int m_fixrate;
	DH_FRAME_INFO m_demuxinfo;

	__int64 m_iHandledDataLen;

	// for 视频数据叠加
	struct OSDData {
		unsigned char data[64*32];
		int en;
		int num;
		int x,y;
	} m_osdInfo,m_chnInfo;

	SHELTERINFO m_shelterinfo;
	
	int m_flagcount;

	BOOL m_bWaterMarkExist;

	watermark_callback m_wmPandoraCallback; //帧编码水印信息回调
	long m_pandorawatermark_user;

	watermark_callback m_wmCallback; //水印信息回调
	WATERMARK_INFO m_Watermark_info;	//需要填充水印信息的数据结构
	long m_watermark_user;
	void *m_hDeinterlace;
	DEINTERLACE_PARA_S  m_struPara;  

	AVICONVERTINFO m_aviinfo;
	static MP4_Xvid_Init m_Fun_MP4_Init;
	static MP4_Xvid_Encore m_Fun_Mp4_Encore;

	IVALERT m_Ivalert[MAX_IVALERT_NUM];
	TRIP m_Trip[MAX_TRIP_NUM];

	enum{AVI_IDL, AVI_RUN, AVI_STOP};
	int  m_status;

	int m_tempLen;
	int m_bufLen;

	BOOL m_bYUV2AVIEnable;
	long m_lAVIDesWidth;
	long m_lAVIDesHeight;
	BYTE* m_pYUVResizeBuf;

	int m_ivsObjNum;
	unsigned long m_ivsSeq;
	DH_IVS_OBJ* m_IvsObjList;

	BYTE* m_pPresetPosInfo;
	long  m_lPresetPosInfoSize;

	IVS_callback m_IVSCallback; //智能IVS回调
	long m_IVS_user;

//avi发生改变文件名称保存
	char m_aviFileName[MAX_PATH];  //the first file name;
	unsigned int  m_fileEncChangeCount;     //编码格式变化次数
	unsigned int  m_fileRateChangeCount;    //帧率变化情况
	unsigned int  m_fileChangeCount;        //分辨率发生改变

private:
	int decode(DH_FRAME_INFO* frameinfo);
	unsigned char* Deinterlace(unsigned char* data);
	int AdjustRateInter();
	int DemuxCallback(DH_FRAME_INFO* frameinfo);
	inline void swapimg(unsigned char *src, unsigned char *dst, int width, int height);
	void ConvertToAVI(unsigned char * pBuf, long nSize, DH_FRAME_INFO* frameinfo);
	void ConvertToAVIEx(unsigned char * pBuf, long nSize, DH_FRAME_INFO* pFrameInfo);
	void StopAVIConvReal();
	void StopAVIConvRealImp();           //内部调用stop avi的接口，意味着文件格式发生改变，文件名称需要后面加后缀
	int ParseIvsInfo(char* pBuf, int len);
	int ParseIVSPresetPosInfo(BYTE* pDataBuf, long lDataLen);
	int GetAviFileName( char* pAviFileName,FILE_CHANGE_ENUM nType);
	void ChangeNewAviFile(FILE_CHANGE_ENUM nType);
};

#endif /* PLAYGRAPH_H */





















