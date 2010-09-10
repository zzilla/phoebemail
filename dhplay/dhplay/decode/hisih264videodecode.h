#ifndef HISIH264VIDEODECODE_H
#define HISIH264VIDEODECODE_H

#include "interface.h"
#include "../depend/DllDeinterlace.h"
#include "../depend/hi_config.h"
#include "../depend/hi_h264api.h"

#define FILED_WIDTH    720   //场图像宽
#define FIELD_HEIGHT   240   //场图像高

typedef HI_HDL (/*_stdcall*/* HI264_DecCreate)( H264_DEC_ATTR_S *pDecAttr );

typedef void (/*_stdcall*/* HI264_DecDestroy)( HI_HDL hDec );

typedef HI_S32 (/*_stdcall*/* HI264_DecFrame)(
    HI_HDL  hDec,
    HI_U8*  pStream,
    HI_U32  iStreamLen,
    HI_U64  ullPTS,
    H264_DEC_FRAME_S *pDecFrame,
    HI_U32  uFlags );

typedef HI_S32 (*HI264_DecAU)(
	HI_HDL hDec,
	HI_U8 *pStream,
	HI_U32 iStreamLen,
	HI_U64 ullPTS,
	H264_DEC_FRAME_S *pDecFrame,
	HI_U32 uFlags );

class HisiH264VideoDecoder:public IDecode
{
public:
    HisiH264VideoDecoder();
    ~HisiH264VideoDecoder();
    
	/************************************************************************/
	/* 视频解码接口(初始化，清除，重置，图像大小发生改变，解码，etc			*/
	/************************************************************************/
    int init();		// 初始化
    int clean();	// 清除
	int reset();	// 重置
    int resize(int w, int h); // 改变图像大小，可以不用
    int decode(unsigned char *src_buf,unsigned int buflen,unsigned char* dest_buf, int param1, int param2); // 解码	
	
	int getWidth(){return m_width;}
	int getHeight(){return m_height;}

private:
	void *m_decHandle;	
	void *m_hDeinterlace;
	H264_DEC_ATTR_S		m_attr;
	int m_width;
	int m_height;
	DEINTERLACE_FRAME_S m_struDstFrame;                     //Deinterlace处理后YUV420输出
    DEINTERLACE_PARA_S  m_struPara;                         //初始化Deinterlace输入参数
	
	static HI264_DecCreate m_Fun_HIDecCreate;
    static HI264_DecDestroy m_Fun_HIDecDestroy;
	static HI264_DecAU m_Fun_HIDecAU;
};

#endif /* HISIH264VIDEODECODE_H */