#include "playgraph.h"
#include <stdio.h>
#include "../utils.h"
#include "../playmanage.h"
#include "../depend/LumaStretch.h"
//#define FORSIFA

#ifdef _DEBUG
#define __Debug_String(_t)  {char str[120]; sprintf(str, (_t)); OutputDebugString(str);}
#else
#define __Debug_String(_t)
#endif
////////////////////////////////////////////////////////////////////////////////////////////////



CritSec g_playgraph_critsec;

MP4_Xvid_Init DhPlayGraph::m_Fun_MP4_Init = NULL;
MP4_Xvid_Encore DhPlayGraph::m_Fun_Mp4_Encore = NULL;
#define  DEFAULT_ADJUST_BUFNUM 3
#define  DEFAULT_ADJUST_RANGE 8

extern int Mpeg4_Analyse(unsigned char* pBuf,int nSize,int* nWidth,int* nHeight,int*framerate);

extern void YResizeCubic(unsigned char* ptr_in, unsigned char* ptr_rz, int old_rows, int old_cols, int rsz_rows, int rsz_cols);
extern int img_conv(unsigned char* psrc, unsigned int srcwidth, unsigned int srcheight, 
			 unsigned char* pdst, unsigned int dstwidth, unsigned int dstheight);

/*!
* 构造函数
*/
DhPlayGraph::DhPlayGraph()
: m_h264videoDecoder(NULL)
, m_audioDecoder(NULL)
, m_hMp4Decoder(NULL)
, m_streamParser(NULL)
, m_IfFindIFrame(FALSE)
, m_IfFindAFrame(FALSE)
, m_IfFindFirstIFrame(FALSE)
{
	m_item = new DhAVFrame ;
	memset(m_item, 0, sizeof(DhAVFrame));

	m_item->ivsobjnum  = 0;
	m_item->ivsObjList = NULL;
	
	//m_hisih264videoDecoder = new HisiH264VideoDecoder ;//海思h264解码器
	m_h264videoDecoder  = new H264VideoDecoder ;//解码器
	m_audioDecoder		    = new DhAudioDecoder ;
	m_hMp4Decoder          = new Mpeg4Decoder ;		
	m_Decoder  = m_h264videoDecoder ;	
	
	m_streamParser = new DhStreamParser ;
	
	m_preBuffer = NULL;
	m_yuvBuf = NULL;
	m_audioBuf		= NULL;
	m_lastYUVBuf	= NULL;
	
	m_adjust_bufnum = DEFAULT_ADJUST_BUFNUM;
	m_adjust_range = DEFAULT_ADJUST_RANGE;
	
	m_hDeinterlace = NULL;
	
	memset(&m_Watermark_info, 0, sizeof(m_Watermark_info));
	m_Watermark_info.buf = new char[128];
	m_wmCallback = NULL;

	m_wmPandoraCallback = NULL;
	m_pandorawatermark_user = 0;
	
	m_height = 0 ;
	m_width = 0 ;
	
	m_aviinfo.m_hAVIFile = NULL;
	m_iHandledDataLen = 0;	
	m_IfDecOnly = TRUE;

	m_bYUV2AVIEnable = FALSE;

	m_dwFrameNum = 0;

	m_IvsObjList = new DH_IVS_OBJ[MAX_IVSOBJ_NUM];
	m_IVSCallback = NULL;
	m_IVS_user = 0;

	m_pPresetPosInfo = NULL;
	m_lPresetPosInfoSize = 0;
	
	memset(m_aviFileName , 0  , MAX_PATH);
	m_fileChangeCount = 0;
	m_fileEncChangeCount = 0;
	m_fileRateChangeCount = 0;
}

/*!
* 析构函数
*/
DhPlayGraph::~DhPlayGraph()
{
	/*	m_video_render.clear() ;*/
	
	SAFE_DELETEBUFF(m_Watermark_info.buf);
	
	if (NULL != m_hDeinterlace)
    {
        HI_ReleaseDeinterlace(m_hDeinterlace);//释放deinterlace资源
        m_hDeinterlace = NULL;                //建议每次释放后把句柄置空.
    }
	
	SAFE_DELETE(m_hMp4Decoder);
	SAFE_DELETE(m_audioDecoder);
	SAFE_DELETE(m_h264videoDecoder)
	
	SAFE_DELETE(m_item);

	SAFE_DELETE(m_streamParser);
	SAFE_DELETEBUFF(m_preBuffer);
	SAFE_DELETEBUFF(m_yuvBuf);
	SAFE_DELETEBUFF(m_lastYUVBuf);
	SAFE_DELETEBUFF(m_audioBuf);
	SAFE_DELETEBUFF(m_IvsObjList);

	if (NULL != m_IVSCallback)
	{
		m_IVSCallback = NULL;
		m_IVS_user = 0;
	}
	SAFE_DELETEBUFF(m_pPresetPosInfo);
	
	StopAVIConvReal();
}

int DhPlayGraph::init(DecodeOption *info)
{
	m_filerate = 0 ;
	m_userrate = 25 ;
	m_fixrate = 0;
	
	m_deinterlace = 0 ;
	m_h264flag = 0 ;
	
	m_FirstIFrameTime = 0 ;
	m_BeginTime = 0;
	m_curtime = 0;
	m_preIFrameTime = 0;
	m_PframeNum = 0;
	m_flagcount = 0;
	m_dwFrameNum = 0;
	m_iHandledDataLen = 0;
	
	m_IfUsed = TRUE ;
	m_IfFindIFrame = FALSE ;
	m_IfFindAFrame = FALSE;
	m_IfFindFirstIFrame = FALSE ;
	m_IfDecVideo = TRUE ;
	m_IfDecAudio = FALSE ;
	m_IfPauseData = TRUE ;
	m_IfDecPause  = FALSE ;
	m_IfStepGo     = FALSE ;
	m_IfFirstFrame = FALSE;
	m_bYUV2AVIEnable = FALSE;
	m_bWaterMarkExist = FALSE;
	m_callback = info->cb ;
	m_port = info->decId ;
	m_IfDecOnly = info->ifDecOnly ;
	m_item->context = NULL;
	m_item->width = 0 ;
	m_item->height = 0 ;

	m_wmCallback = NULL;
	m_watermark_user = 0;

	m_wmPandoraCallback = NULL;
	m_pandorawatermark_user = 0;

	if ((NULL == m_streamParser) || (NULL == m_h264videoDecoder)
		|| (NULL == m_hMp4Decoder) || (NULL ==  m_IvsObjList))
	{
		__Debug_String("Decoder create fail!!!!!\n");
		return 6;//DH_PLAY_ALLOC_MEMORY_ERROR
	}
	
	if (m_audioBuf == NULL)
	{
		m_audioBuf = new BYTE[AUDIOBUFLEN];
		if (m_audioBuf == NULL)
		{
			__Debug_String("Decoder create fail!!!!!\n");
			return 6;//DH_PLAY_ALLOC_MEMORY_ERROR
		}
	}
	memset(m_audioBuf, 0, AUDIOBUFLEN);

	if (m_IfDecOnly == FALSE)
	{
		m_video_render.SetACTimerObj(info->pACTimerObj);
		int ret = m_video_render.init(info->decId,info->imgBufCount, m_width*m_height*3/2, info->cb);
		if (ret >0)
		{
			return ret;
		}
	}

	if (m_IVSCallback != NULL)
	{
		m_video_render.SetIVSCallback(m_IVSCallback, m_IVS_user);
	}
	
	m_transMode = info->transMode ;
	
	if (m_h264videoDecoder->init() < 0)
	{
		return 19; //DH_PLAY_INIT_DECODER_ERROR
	}

	if (m_hMp4Decoder->init(m_port, info->hwnd) < 0)
	{
		return 19; //DH_PLAY_INIT_DECODER_ERROR
	}
	
	m_encode_type = DH_ENCODE_VIDEO_HI_H264 ;
	m_Decoder  = m_h264videoDecoder ;	////////////////////
	
	//码流分析器初始化
	// 	m_streamParser->Reset(STREAMPARSER_RESET_REFIND);
	
	m_streamParser->Reset(STREAMPARSER_RESET_REFIND, 
		(m_port == 100) ? DH_STREAM_NEW:0);
	
	memset(&m_demuxinfo, 0, sizeof(m_demuxinfo));
	memset(&m_osdInfo, 0, sizeof(m_osdInfo));
	memset(&m_chnInfo, 0, sizeof(m_chnInfo));
	memset(&m_shelterinfo, 0, sizeof(m_shelterinfo));
	
	m_aviinfo.m_hAVIFile = 0;
	m_aviinfo.m_hVideoStream = 0;
	m_aviinfo.m_hAudioStream = 0;
	m_aviinfo.m_aviVideoWritePos = 0;
	m_aviinfo.m_aviAudioWritePos = 0;
	m_aviinfo.m_foundIFrame = false;
	memset(&m_aviinfo.strhdr, 0, sizeof(m_aviinfo.strhdr));
	m_aviinfo.enc_param.handle = NULL;
	m_aviinfo.m_encBuffer = 0 ;
	
	m_status = AVI_IDL;
	
	if (NULL != m_preBuffer)
	{
		delete[] m_preBuffer;
		m_preBuffer = NULL;
	}

	m_ivsObjNum = 0;
	m_ivsSeq = 0;

	if (m_pPresetPosInfo != NULL)
	{
		delete[] m_pPresetPosInfo;
		m_pPresetPosInfo = NULL;
	}
	m_lPresetPosInfoSize = 0;

	return 0;
}

int DhPlayGraph::start()
{
	if (m_IfDecOnly == FALSE)
	{
		m_video_render.SetPause(0) ;
		m_video_render.m_YuvDataList.setlock(true);

		int ret = m_video_render.startTimer();
		if (ret > 0)
		{
			return ret;
		}
	}
	
	m_IfDecPause = FALSE ;
	m_IfPauseData = TRUE ;
	
	return 0;
}

void DhPlayGraph::RealStop()
{
	StopAVIConvReal();
	m_video_render.m_YuvDataList.clear();

	//20090929 bug: 播放一条HB的录像，在播放过程中拖放另一段HB的录像进行播放时，会出现上一段录像的残留画面
	m_hMp4Decoder->clean();
	m_h264videoDecoder->clean();

	if (m_hDeinterlace != NULL)
    {
        HI_ReleaseDeinterlace(m_hDeinterlace);//释放deinterlace资源
        m_hDeinterlace = NULL;
    }

	// 重设流畅度参数
	g_cDHPlayManage.m_nFluency[m_port] = 4;
}

int DhPlayGraph::stop()
{
	reset(Reset_level_FASTRETURN) ;
	
	if (m_IfDecOnly == FALSE)
	{
		m_video_render.SetPause(1) ;
		m_video_render.StopTimerSafely();
		m_video_render.StopSoundThread();
	}
	m_IfDecPause = FALSE ;
	m_IfStepGo = FALSE ;

	m_IfFindIFrame = FALSE ;
	m_IfFindAFrame = FALSE;
	m_IfFindFirstIFrame = FALSE ;
	m_IfFirstFrame = FALSE;
	
	m_iHandledDataLen = 0;

	m_dwFrameNum = 0;
	m_bFrameNumScan = FALSE;

	m_wmCallback = NULL;
	m_watermark_user = 0;

	m_wmPandoraCallback = NULL;
	m_pandorawatermark_user = 0;
	
	StopAVIConv();
	
	return 0;
}

int DhPlayGraph::pause()
{
	if (m_IfDecOnly == FALSE)
	{
		m_video_render.SetPause(1) ;
	}
	
	m_IfDecPause = TRUE ;
	
	return 0;
}


int DhPlayGraph::GetImgCount(int* imgcount) 
{
	if (m_IfDecOnly)//内部处理，不判断imgcount的有效性
	{
		*imgcount = 0;
		return -1;
	}
	
	*imgcount = m_video_render.m_YuvDataList.chunkCount() ;
	
	return 0 ;
}

BYTE* DhPlayGraph::GetLocateFrameData(int* width, int* height) 
{
	*width = m_width ;
	*height = m_height ;	
	
	if (m_yuvBuf == NULL)
	{
		m_yuvBuf = new unsigned char[m_width*m_height*3/2];
		memset(m_yuvBuf, 0, m_width*m_height*3/2);
	}
	
	m_video_render.m_YuvDataList.GetLastFrameBuf(m_yuvBuf);
	
	return Deinterlace(m_yuvBuf) ;
}

time_t DhPlayGraph::GetCurTimeEx() 
{
	time_t tm_value = 0;

	if ((m_streamParser->GetStreamType() == DH_STREAM_DHSTD) && m_curtime != 0 && m_curtime-m_FirstIFrameTime !=0) 
	{
		//bug，P帧时间戳不够精确
		if( m_filerate != 0)
		{
			tm_value = (m_curtime-m_FirstIFrameTime)*1000 + 1000 / m_filerate * m_PframeNum;
		}
	}
	else
	{
		if ( m_filerate != 0 )
		{
			tm_value = (m_curtime-m_FirstIFrameTime)*1000 + 1000 / m_filerate *  m_PframeNum ;
		}
	}
// 	else if (m_curtime-m_FirstIFrameTime == 0)
// 	{
// 		tm_value = m_curtime;
// 	}

	return tm_value;
}

int searchOneNode(char* pBuf, int len, char* nodeName, int* beginPos, int* endPos)
{
	int i = 0, j = 0;
	int nodeLen = strlen(nodeName);
	
	for (; i < len-nodeLen; i++)
	{
		if (memcmp(pBuf+i, nodeName, nodeLen) == 0)
		{
			*beginPos = i+nodeLen+1;
			for (j = *beginPos; j < len-nodeLen; j++)
			{
				if (memcmp(pBuf+j, nodeName, nodeLen) == 0)
				{
					*endPos = j-2;
					return 0;
				}
			}
		}
	}
	
	return -1;
}

int searchINTL(char* pBuf, int len, IVALERT ivalert[], TRIP trip[], int* tripNum, int* ivalertNum)
{
	int beginPos = 0, endPos = 0;
	int iRet = 0;
	char tmpStr[100];
	int tmpPos = 0;
	int i = 0;
	*tripNum = 0;
	*ivalertNum = 0;
	
	iRet = searchOneNode(pBuf, len, "ivalert", &beginPos, &endPos);
	if ( iRet == -1)
	{
		iRet = searchOneNode(pBuf, len, "trip", &beginPos, &endPos);
		if (iRet == 0)
		{
			int tmp = 0;
			endPos = 0;
			
			for (i = 0; i < MAX_TRIP_NUM; i++)
			{
				if (tmp >= len)
				{
					return (*tripNum == 0) ? -1 : 0;
				}
				
				int j = 0;
				//p0
				iRet = searchOneNode(pBuf+tmp, len-tmp, "p0", &beginPos, &endPos);
				if (iRet == -1)
				{
					return (*tripNum == 0) ? -1 : 0;
				}
				memset(tmpStr, 0, 100);	
				for (j = beginPos+tmp; j < beginPos+tmp+100; j++)
				{
					if (pBuf[j] == ',')
					{
						tmpPos = j;
						memcpy(tmpStr, pBuf+tmp+beginPos, tmpPos-beginPos-tmp);
						break;
					}
				}
				trip[i].p0.x = atoi(tmpStr);
				memset(tmpStr, 0, 100);	
				memcpy(tmpStr, pBuf+tmpPos+1, endPos+tmp-tmpPos-1);
				trip[i].p0.y = atoi(tmpStr);
				
				//p1
				iRet = searchOneNode(pBuf+tmp, len-tmp, "p1", &beginPos, &endPos);
				if (iRet == -1)
				{
					return (*tripNum == 0) ? -1 : 0;
				}
				memset(tmpStr, 0, 100);	
				for (j = beginPos+tmp; j < beginPos+tmp+100; j++)
				{
					if (pBuf[j] == ',')
					{
						tmpPos = j;
						memcpy(tmpStr, pBuf+tmp+beginPos, tmpPos-beginPos-tmp);
						break;
					}
				}
				trip[i].p1.x = atoi(tmpStr);
				memset(tmpStr, 0, 100);	
				memcpy(tmpStr, pBuf+tmpPos+1, endPos+tmp-tmpPos-1);
				trip[i].p1.y = atoi(tmpStr);
				
				//direction
				iRet = searchOneNode(pBuf+tmp, len-tmp, "direction", &beginPos, &endPos);
				if (iRet == -1)
				{
					return (*tripNum == 0) ? -1 : 0;
				}
				memset(tmpStr, 0, 100);	
				memcpy(tmpStr, pBuf+tmp+beginPos, endPos-beginPos);
				trip[i].direction= atoi(tmpStr);
				
				//bEnable
				iRet = searchOneNode(pBuf+tmp, len-tmp, "enable", &beginPos, &endPos);
				if (iRet == -1)
				{
					return (*tripNum == 0) ? -1 : 0;
				}
				if (memcmp(pBuf+tmp+beginPos, "true", 4) == 0)
				{
					trip[i].bEnable = true;
				}
				else
				{
					trip[i].bEnable = false;
				}
				
				(*tripNum)++;
				
				tmp += (endPos+strlen("enable"));
			}
			
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else//ivalert
	{
		int tmp = 0;
		endPos = 0;
		
		for (i = 0; i < MAX_IVALERT_NUM; i++)
		{
			if (tmp >= len)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			
			int j = 0;
			
			//elerttime
			iRet = searchOneNode(pBuf+tmp, len-tmp, "alerttime", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memcpy(ivalert[i].alerttime, pBuf+tmp+beginPos, endPos-beginPos);
			
			//entrytime
			iRet = searchOneNode(pBuf+tmp, len-tmp, "entrytime", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memcpy(ivalert[i].entrytime, pBuf+tmp+beginPos, endPos-beginPos);
			
			//event
			iRet = 	searchOneNode(pBuf+tmp, len-tmp, "event", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memset(tmpStr, 0, 100);
			memcpy(tmpStr, pBuf+tmp+beginPos, endPos-beginPos);
			ivalert[i].event = atoi(tmpStr);
			
			//position
			iRet = 	searchOneNode(pBuf+tmp, len-tmp, "position", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memset(tmpStr, 0, 100);
			
			tmpPos = 0;
			for (j = beginPos+tmp; j < beginPos+tmp+100;j++)
			{
				if (pBuf[j] == ',')
				{
					tmpPos = j;
					memcpy(tmpStr, pBuf+tmp+beginPos, tmpPos-beginPos-tmp);
					break;
				}
			}
			ivalert[i].position.x = atoi(tmpStr);
			memcpy(tmpStr, pBuf+tmpPos+1, endPos+tmp-tmpPos-1); 
			ivalert[i].position.y = atoi(tmpStr);
			
			//witdh
			iRet = 	searchOneNode(pBuf+tmp, len-tmp, "width", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memset(tmpStr, 0, 100);
			memcpy(tmpStr, pBuf+tmp+beginPos, endPos-beginPos);
			ivalert[i].width = atoi(tmpStr);
			
			//height
			iRet = 	searchOneNode(pBuf+tmp, len-tmp, "height", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memset(tmpStr, 0, 100);
			memcpy(tmpStr, pBuf+tmp+beginPos, endPos-beginPos);
			ivalert[i].height = atoi(tmpStr);
			
			//direction
			iRet = 	searchOneNode(pBuf+tmp, len-tmp, "direction", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memset(tmpStr, 0, 100);
			memcpy(tmpStr, pBuf+tmp+beginPos, endPos-beginPos);
			ivalert[i].direction = atoi(tmpStr);
			
			//speed
			iRet = 	searchOneNode(pBuf+tmp, len-tmp, "speed", &beginPos, &endPos);
			if (iRet == -1)
			{
				return (*ivalertNum == 0) ? -1 : 0;
			}
			memset(tmpStr, 0, 100);
			memcpy(tmpStr, pBuf+tmp+beginPos, endPos-beginPos);
			ivalert[i].speed = atoi(tmpStr);
			
			(*ivalertNum)++;
			tmp += (endPos+strlen("speed"));
		}
	}
	
	return 0;
}

unsigned int sum_32_verify(unsigned char* buf, int len)
{
	int i;
	unsigned int ret = 0;
	unsigned int num = len>>2;
	unsigned int res = len%4;
	unsigned int res_val = 0;
	static unsigned int mask[4] = {0x0, 0x000000ff, 0x0000ffff, 0x00ffffff};
	unsigned int *p = (unsigned int *)buf;
	
	if (res == 0)
	{
		res_val = 0;
	}
	else
	{
		res_val = p[num] & mask[res] ;
	}
	
	for ( i = 0; i < num; i ++)
	{
		ret += *(p + i);
	}	
	
	ret += res_val;
	
	return ret;
}


unsigned int crc32(unsigned char* p, unsigned int len);

BOOL DhPlayGraph::WaterMarkVerify(DH_FRAME_INFO* fFrameInfo)
{
	BOOL bRetVal = TRUE;
	unsigned char* VeryfyBuf = m_frameinfo->pHeader+24;

	unsigned int crcValue = 0;
	unsigned int crcRealValue = 0;
	
	DWORD extendLen = m_frameinfo->pHeader[22];
	if (extendLen == 0)
	{
		bRetVal = FALSE;
		goto __error;
	}

	if (VeryfyBuf[0] == 0x88)
	{
		int check_type = VeryfyBuf[7];

		unsigned int checkValue = VeryfyBuf[1]|VeryfyBuf[2]<<8|VeryfyBuf[3]<<16|VeryfyBuf[4]<<24;

		VeryfyBuf += 8;		
		int iWaterMarkLen = VeryfyBuf[8]|VeryfyBuf[9]<<8|VeryfyBuf[10]<<16|VeryfyBuf[11]<<24;

		if (check_type == 2)
		{
			unsigned int crcRealValue = crc32(VeryfyBuf, 12+iWaterMarkLen);
			
			if (checkValue != crcRealValue)
			{
				bRetVal = FALSE;
				goto __error;
			}
		}
		else if (check_type == 0)
		{
			unsigned int sumRealValue = 0;

			sumRealValue = sum_32_verify(VeryfyBuf, 12+iWaterMarkLen);
			
			if (checkValue != sumRealValue)
			{
				bRetVal = FALSE;
				goto __error;
			}
		}		
	}

	//水印帧纯数据校验
	crcValue = VeryfyBuf[4]|VeryfyBuf[5]<<8|VeryfyBuf[6]<<16|VeryfyBuf[7]<<24;
	crcRealValue = crc32(m_frameinfo->pContent, m_frameinfo->nFrameLength);
	
	if (crcValue != crcRealValue)
	{
		bRetVal = FALSE;
		goto __error;
	}

__error:
	return bRetVal;
}

int DhPlayGraph::ParseIVSPresetPosInfo(BYTE* pDataBuf, long lDataLen)
{
	SAFE_DELETEBUFF(m_pPresetPosInfo);
	m_lPresetPosInfoSize = 0;

	if (lDataLen == 0) return 3;

	BYTE* pInfoOffset = pDataBuf + 24;

	if (*pInfoOffset != 0x84)
		return 1; // 不是预置点信息帧段

	BYTE cInfoType = *(pInfoOffset + 1);
	if (cInfoType == 0x00)
	{
		return 2; // 未定义预置点信息
	}
	else if (cInfoType == 0x01)
	{
		long lInfoDataLen = (*(pInfoOffset + 3)<<8) | (*(pInfoOffset + 2));
		if (lInfoDataLen <= 0)
		{
			return 3; // 信息有误
		}
		m_lPresetPosInfoSize = lInfoDataLen<<2; //扩展帧长度以4字节为单位
		
		m_pPresetPosInfo = new BYTE[m_lPresetPosInfoSize];
		if (m_pPresetPosInfo == NULL)
		{
			return 6; //DH_PLAY_ALLOC_MEMORY_ERROR
		}
		
		memcpy(m_pPresetPosInfo, (pInfoOffset+4), m_lPresetPosInfoSize);
		
		return 0;
	}
	
	return 2;
}

int DhPlayGraph::ParseIvsInfo(char* pIvsBuf, int len)
{
	m_ivsObjNum = 0;

	if (len == 0) return 0;
	
	char* bufptr = pIvsBuf;
	WORD version = *(WORD*)bufptr;
	
	bufptr += 2;
	if (version != 1)
	{
		m_ivsObjNum = -1;
		return -2;//版本号不正确
	}
	
	m_ivsObjNum = *bufptr;
	bufptr += 2;
	
	if (m_ivsObjNum <= 0 || m_ivsObjNum > MAX_IVSOBJ_NUM)
	{
		m_ivsObjNum = -1;
		return -3;//目标个数不合理
	}
	
	for (int i = 0; i < m_ivsObjNum; i++)
	{
		m_IvsObjList[i].decode_id		= m_port;
		m_IvsObjList[i].obj_id		= *(int*)bufptr;
		m_IvsObjList[i].enable		= (bufptr[4] == 3) ? 0 : 1;
		m_IvsObjList[i].trackpt_num	= bufptr[5];
		
		int fatherIdNum = bufptr[7];//父ID数目，暂时不用
		bufptr += 12;
		bufptr += (fatherIdNum*4);//跳过父ID
		for (int j = 0; j < m_IvsObjList[i].trackpt_num; j++)
		{
			m_IvsObjList[i].track_point[j].x	   = *(short*)bufptr;
			m_IvsObjList[i].track_point[j].y	   = *(short*)(bufptr + 2);
			m_IvsObjList[i].track_point[j].xSize = *(short*)(bufptr + 4);
			m_IvsObjList[i].track_point[j].ySize = *(short*)(bufptr + 6);
			
			bufptr += 8;
		}
		
	}
	
	return m_ivsObjNum;
}

int DhPlayGraph::parseData(unsigned char *data, unsigned int len)
{	
	m_streamParser->InputData(data, len) ;//流分析
	
	while (m_IfPauseData && (m_frameinfo = m_streamParser->GetNextFrame()) != NULL )//加一个变量m_IfPauseData，是为了在定位和停止时能快速返回
	{
		BOOL bchangeRate = FALSE;
		BOOL bchange = FALSE;
		if (m_frameinfo->nLength == 0 && m_frameinfo->nType == DH_FRAME_TYPE_DATA) 
		{
			continue ;
		} 

		if ((m_streamParser->GetStreamType() == DH_STREAM_DHSTD) && (NULL != m_wmCallback) && 
			m_IfFindFirstIFrame && (m_frameinfo->nType == DH_FRAME_TYPE_VIDEO))
		{
			BOOL bWaterMarkErr = FALSE;

			// 定位操作，单帧播放时引发帧号改变时，跳过检测
			if (m_bFrameNumScan)
			{
				if (m_dwFrameNum == 0xFFFFFFFF)
				{
					if (m_frameinfo->nFrameNum != 0)
					{
						bWaterMarkErr = TRUE;
					}
				}
				else if ((m_frameinfo->nFrameNum - m_dwFrameNum) != 1)
				{
					bWaterMarkErr = TRUE;
				}
			}

			if (m_bWaterMarkExist && bWaterMarkErr)
			{
				m_wmCallback((char*)m_frameinfo->pContent, 0, m_frameinfo->nLength, 
					m_frameinfo->nLength, 2, m_watermark_user);
			}

			m_dwFrameNum = m_frameinfo->nFrameNum;
		}
		
		if ((m_frameinfo->nSubType == DH_FRAME_TYPE_WATERMARK_TEXT ||
			m_frameinfo->nSubType == DH_FRAME_TYPE_WATERMARK_JPEG ||
			m_frameinfo->nSubType == DH_FRAME_TYPE_WATERMARK_BMP)
			&& m_frameinfo->nType == DH_FRAME_TYPE_DATA) 
		{
			m_bWaterMarkExist = TRUE;

			long key = 0;
			long reserve = 0;
			BOOL bRet = WaterMarkVerify(m_frameinfo);
			if (bRet)
			{
				reserve = 1;
			}
			else
			{
				reserve = 2;
			}

			switch(m_frameinfo->nSubType)
			{
			case DH_FRAME_TYPE_WATERMARK_TEXT:
				key = 0;
				break;
			case DH_FRAME_TYPE_WATERMARK_JPEG:
			case DH_FRAME_TYPE_WATERMARK_BMP:
				key = 1;
				break;
			default:
				break;
			}

			if (NULL != m_wmCallback)
			{
				m_wmCallback((char*)m_frameinfo->pContent, key, m_frameinfo->nFrameLength, 
					m_frameinfo->nFrameLength, reserve, m_watermark_user);
			}
			continue ;
		} 
		
		if (m_frameinfo->nFrameLength == 0)
		{
			if (m_frameinfo->nType == DH_FRAME_TYPE_VIDEO)
			{
				m_IfFindIFrame = FALSE ;
				
				if (NULL != m_IVSCallback)
				{
					m_ivsObjNum = -1;
				}
				
				continue;
			}
			else if (m_frameinfo->nType == DH_FRAME_TYPE_AUDIO)
			{
				continue;
			}
		}
		
		if (m_frameinfo->nSubType == DH_FRAME_TYPE_DATA_INTL && m_frameinfo->nType == DH_FRAME_TYPE_DATA) 
		{
			if (NULL != m_wmCallback) 
			{
				memset(m_Ivalert, 0, sizeof(IVALERT)*MAX_IVALERT_NUM);
				memset(m_Trip, 0, sizeof(TRIP)*MAX_TRIP_NUM);
				int trip_Num = 0;
				int ivalert_Num = 0;
				int iRet = searchINTL((char*)m_frameinfo->pContent, m_frameinfo->nFrameLength, m_Ivalert, m_Trip, &trip_Num, &ivalert_Num);
				if (iRet != -1)
				{
					if (trip_Num == 0)
					{
						m_wmCallback((char*)m_Ivalert, 0, sizeof(IVALERT), sizeof(IVALERT)*ivalert_Num, 3, m_watermark_user);
					}
					else
					{
						m_wmCallback((char*)m_Trip, 0, sizeof(TRIP), sizeof(TRIP)*trip_Num, 4, m_watermark_user);
					}

					continue;
				}
			}

			// IVS parse
			if (NULL != m_IVSCallback)
			{
				ParseIVSPresetPosInfo(m_frameinfo->pHeader, m_frameinfo->nLength);

				if (m_ivsSeq != 0 && m_frameinfo->nIvsSeq != (m_ivsSeq + 1))
				{
					m_ivsObjNum = -1;
					m_ivsSeq = m_frameinfo->nIvsSeq;
					continue;
				}
				m_ivsSeq = m_frameinfo->nIvsSeq;
				ParseIvsInfo((char*)m_frameinfo->pContent, m_frameinfo->nFrameLength);
			}

			continue;
		}
		
		if (m_streamParser->GetStreamType() == DH_STREAM_DHSTD)
		{
			//数据校验
			unsigned char* VeryfyBuf = m_frameinfo->pHeader+24;
			DWORD extendLen = m_frameinfo->pHeader[22];
			while (extendLen > 0)
			{
				if (VeryfyBuf[0] == 0x80 || VeryfyBuf[0] == 0x81 || VeryfyBuf[0] == 0x83)
				{
					extendLen -= 4;
					VeryfyBuf += 4;
					continue;
				}
				else if (VeryfyBuf[0] == 0x88)
				{	
					if (VeryfyBuf[7] == 2)
					{
						unsigned int crcValue = VeryfyBuf[1]|VeryfyBuf[2]<<8|VeryfyBuf[3]<<16|VeryfyBuf[4]<<24;
						unsigned int crcRealValue = crc32(m_frameinfo->pContent, m_frameinfo->nFrameLength);
						
						if (crcValue != crcRealValue)
						{
							if (NULL != m_wmCallback) 
							{
								unsigned int time = GetCurTimeEx();
								memcpy(m_Watermark_info.buf, &time, 4);
								m_wmCallback(m_Watermark_info.buf, 0, 4, 
									4, 2, m_watermark_user);
							}
						}
					}
					else if (VeryfyBuf[7] == 0)
					{
						unsigned int crcValue = VeryfyBuf[1]|VeryfyBuf[2]<<8|VeryfyBuf[3]<<16|VeryfyBuf[4]<<24;
						unsigned int crcRealValue = 0;
					
						crcRealValue = sum_32_verify(m_frameinfo->pContent, m_frameinfo->nFrameLength);
						
						if (crcValue != crcRealValue)
						{
							if (NULL != m_wmCallback && m_IfFindFirstIFrame) 
							{
								unsigned int time = GetCurTimeEx();
								memcpy(m_Watermark_info.buf, &time, 4);
								m_wmCallback(m_Watermark_info.buf, 0, 4, 
									4, 2, m_watermark_user);
							}
						}
					}

					extendLen -= 8;
					break;
				}
				else
				{
					break;
				}
			}//end of while (extendLen > 0)
		}//end of if (m_streamParser->GetStreamType() == DH_STREAM_DHSTD)
		
		if (m_frameinfo->nEncodeType != m_encode_type)
		{		
			m_encode_type =m_frameinfo->nEncodeType ;
			
			if (m_encode_type == DH_ENCODE_VIDEO_H264 && m_frameinfo->nParam2 != 2)
			{
				m_Decoder = m_h264videoDecoder ;
			}
			else if((m_encode_type == DH_ENCODE_VIDEO_H264 && m_frameinfo->nParam2 == 2) ||//新码流中的hi264没
				m_encode_type == DH_ENCODE_VIDEO_HI_H264)//DHAV的中的hi264
			{	
				m_Decoder = m_h264videoDecoder;
// 				if(!m_hisih264videoDecoder)
// 				{
//					m_Decoder = m_h264videoDecoder;
// 				}
// 				else
// 				{
// 					m_Decoder = m_hisih264videoDecoder;
// 				}
			}
			else if (m_encode_type == DH_ENCODE_VIDEO_MPEG4)
			{
				m_Decoder = m_hMp4Decoder ;
			}
			else if (m_encode_type == DH_ENCODE_VIDEO_JPEG)
			{
				m_Decoder = NULL;
			}
			else if (m_encode_type >= DH_ENCODE_AUDIO_PCM8)
			{
				m_Decoder = m_audioDecoder;
			}
		}
		
		if (m_frameinfo->nType == DH_FRAME_TYPE_VIDEO &&
			m_frameinfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)
		{
			if (m_FirstIFrameTime == 0)
			{
				m_FirstIFrameTime = m_frameinfo->nTimeStamp ;
			}
			
			if ( m_frameinfo->nTimeStamp != 0)
			{
				m_curtime = m_frameinfo->nTimeStamp ;
			}
			else
			{
				int zgf = 0;
			}
			
			m_PframeNum = 0 ;
			
			m_preIFrameTime = m_curtime;//add by zgf 0109
			m_IfFindIFrame = TRUE ;
			m_deinterlace = m_frameinfo->nParam1 ;
			
			m_h264flag = m_frameinfo->nParam2  ;

			m_bFrameNumScan = TRUE;
			
//			if (m_encode_type == DH_ENCODE_VIDEO_HI_H264)
//			{
//				m_deinterlace = 2;
//			}
//			else if (m_encode_type == DH_ENCODE_VIDEO_H264)
//			{
//			//	m_deinterlace = 1;//新码流传入的m_deinterlace为1
// 			}
			if((m_encode_type == DH_ENCODE_VIDEO_H264 && m_frameinfo->nParam2 == 2))//新码流中的hi264没
			{
 				m_deinterlace = 1;
			}
 			else if (m_encode_type == DH_ENCODE_VIDEO_HI_H264)	//DHAV的中的hi264
 			{
				m_deinterlace = 2;
 			}
			
			if (m_streamParser->GetStreamType() == DH_STREAM_HB 
				||m_streamParser->GetStreamType() == DH_STREAM_PS
				||m_streamParser->GetStreamType() == DH_STREAM_MPEG4)
			{
				m_deinterlace = 2 ;
			}		
			
			if (m_frameinfo->nFrameRate != m_filerate)
			{
				if( m_status == AVI_RUN )
				{
					bchangeRate = TRUE;
				}
				m_filerate = m_frameinfo->nFrameRate ;
				if (m_filerate <= 0 || m_filerate > 100)//当帧率过大或小于等于0时，自动置为25
				{
					m_filerate = 25 ;
				}
				
				if (m_fixrate == 0)
				{
					if (m_IfDecOnly == FALSE)
					{
						int nRate = (m_userrate*m_filerate -1)/25 + 1;
						//防止帧率为0
						if (nRate == 0)
						{
							nRate = 1;
						}
						
						//帧率发生改变时，确保之前帧率的图像播放完毕
						if ((SM_File == m_transMode) || (SM_Stream_File == m_transMode))
						{
							while (m_video_render.m_YuvDataList.chunkCount() > 0)
							{
								Sleep(5);
							}
						}

						//实时流模式下，帧率发生改变时，清空YUV缓冲
						if (SM_Stream_Stream == m_transMode)
						{
							if (m_video_render.m_YuvDataList.chunkCount() >0)
							{
								m_video_render.m_YuvDataList.reset();
							}

							if (m_video_render.m_PcmDataList.chunkCount() >0)
							{
								m_video_render.m_PcmDataList.reset();
							}
						}

						m_video_render.changeRate(nRate);
					}
				}
			}
			
			if (m_width != m_frameinfo->nWidth || m_height != m_frameinfo->nHeight)
			{
				if( m_status == AVI_RUN )
				{
					bchange = TRUE;
				}
				if (m_frameinfo->nWidth > 0 && m_frameinfo->nWidth <= 2000
					&& m_frameinfo->nHeight > 0 && m_frameinfo->nHeight <= 2000)
				{
					if (m_yuvBuf != NULL)
					{
						SAFE_DELETEBUFF(m_lastYUVBuf);
						m_lastYUVBuf = new BYTE[m_width*m_height*3/2];
						memset(m_lastYUVBuf, 0, m_width*m_height*3/2);
						memcpy(m_lastYUVBuf, m_yuvBuf, m_width*m_height*3/2);
					}

					m_width = m_frameinfo->nWidth ;
					m_height= m_frameinfo->nHeight ;
					
					//文件方式下为了方便统计
					if (SM_File == m_transMode)
					{
						while (m_video_render.m_YuvDataList.chunkCount() > 0)
						{
							Sleep(5);
						}
					}
					
//					m_video_render.stopTimer();
					int ret = m_video_render.m_YuvDataList.resize(m_width*m_height*3/2);//为支持大分辨率码流，缓冲区改用动态分配内存 080715 zgf
					if (ret >0)
					{
						return 6;//DH_PLAY_ALLOC_MEMORY_ERROR
					}
//					m_video_render.startTimer();

					if (NULL != m_hDeinterlace)
					{
					HI_ReleaseDeinterlace(m_hDeinterlace);//释放deinterlace资源
						m_hDeinterlace = NULL;                //建议每次释放后把句柄置空.
					}

					if (m_lastYUVBuf != NULL)
						m_video_render.m_YuvDataList.GetLastFrameBuf(m_lastYUVBuf);
				}			
				
				if (m_yuvBuf)
				{
					delete m_yuvBuf;
					m_yuvBuf = NULL;
					m_yuvBuf = new unsigned char[m_width*m_height*3/2];
					memset(m_yuvBuf, 0, m_width*m_height*3/2);
				}
			}
			
			if (m_encode_type != DH_ENCODE_VIDEO_JPEG)
			{
				m_Decoder->resize(m_width,m_height) ;
			}//08-6-11
			
			if (m_IfFindFirstIFrame == FALSE)
			{
				m_IfFindFirstIFrame = TRUE ;
				m_dwFrameNum = m_frameinfo->nFrameNum;
			}
			
			m_demuxinfo.nWidth = m_frameinfo->nWidth;
			m_demuxinfo.nHeight = m_frameinfo->nHeight;
			m_demuxinfo.nYear = m_frameinfo->nYear;
			m_demuxinfo.nMonth = m_frameinfo->nMonth;
			
			m_demuxinfo.nDay = m_frameinfo->nDay;
			m_demuxinfo.nHour = m_frameinfo->nHour;
			m_demuxinfo.nMinute = m_frameinfo->nMinute;
			m_demuxinfo.nSecond = m_frameinfo->nSecond;
			m_demuxinfo.nTimeStamp = m_frameinfo->nTimeStamp;
			m_demuxinfo.nFrameRate = m_frameinfo->nFrameRate;
			
			// 与新水印混淆
			if (m_IfFirstFrame && (NULL != m_wmPandoraCallback) && (m_encode_type == DH_ENCODE_VIDEO_MPEG4))
			{
				int ret ;
				memset(m_Watermark_info.buf, 0, 128);
				ret = m_hMp4Decoder->SetCheckWaterMark(2, (int*)&m_Watermark_info);
				if (0 == ret )
				{
					m_wmPandoraCallback(m_Watermark_info.buf, m_Watermark_info.key, 
						m_Watermark_info.len, m_Watermark_info.reallen, 
						m_Watermark_info.reserved, m_pandorawatermark_user);
					m_hMp4Decoder->SetCheckWaterMark(3, NULL);
				}
			}	
		}
		else if (m_frameinfo->nType == DH_FRAME_TYPE_VIDEO &&
			m_frameinfo->nSubType == DH_FRAME_TYPE_VIDEO_P_FRAME)
		{
			if (m_IfFindFirstIFrame)
			{
				m_PframeNum ++;
			}
			if ( m_frameinfo->nTimeStamp != 0)
			{
				m_curtime = m_frameinfo->nTimeStamp ;
			}
		}
		
		if( m_status == AVI_RUN)
		{
			if( bchange == TRUE && bchangeRate == FALSE)
			{
				m_fileChangeCount++;
				ChangeNewAviFile(ENUM_FILE_CHANGE);
			}

			else if ( bchange == FALSE && bchangeRate == TRUE )
			{
				m_fileRateChangeCount++;
				ChangeNewAviFile(ENUM_FILE_RATE);
			}
			
			else if( bchange == TRUE && bchangeRate == TRUE)
			{
				m_fileChangeCount++;
				m_fileRateChangeCount++;
				ChangeNewAviFile(ENUM_FILE_RATE_AND_FIX);
			}

		}
		//avi转换的时候视频不解码音频要解码
		if (m_status == AVI_RUN && m_frameinfo->nType == DH_FRAME_TYPE_VIDEO)
		{
			ConvertToAVI(0 , 0, m_frameinfo);
		}
		else
		{
			decode(m_frameinfo) ;
		}	

		if (SM_File == m_transMode)
		{
			m_iHandledDataLen += m_frameinfo->nLength;
		}
	}	
	
	return 0;
}

DWORD DhPlayGraph::GetHandledDataLen()
{
	return m_iHandledDataLen;
}

void DhPlayGraph::SetHandledDataLen(DWORD dCurDataPos)
{
	m_iHandledDataLen = dCurDataPos;
}

int DhPlayGraph::stepGo()
{
	int iRet = 0;
	
	if (m_IfDecOnly)
	{
		iRet = -1;
	}
	else
	{
		pause() ;
		m_video_render.render() ;
		m_IfStepGo = TRUE ;
	}
	
	return iRet;
}

int DhPlayGraph::changeRate(int rate)
{
	m_userrate = rate ;
	if (m_IfDecOnly == FALSE)
	{
		m_video_render.changeRate((rate*m_filerate - 1)/25 + 1) ;//取上值
	}
	
	return 0;
}

int DhPlayGraph::getRate()
{
	int rate = 25;
	
	if (m_IfDecOnly)
	{
		rate = m_filerate;
	}
	else
	{
		rate = m_video_render.getRate();
	}
	
	return rate;
}

int DhPlayGraph::getFileRate()
{
	return m_filerate;
}

void DhPlayGraph::reset(int level)
{
	if (level == Reset_level_RAWYUV)//清空临时缓冲和解码缓冲
	{
		if (m_IfDecOnly == FALSE)
		{
			m_video_render.m_YuvDataList.reset() ;
			m_video_render.m_PcmDataList.reset() ; 
			m_video_render.m_YuvDataList.setlock(true) ;
		}
		m_bFrameNumScan = FALSE;
		m_IfFindIFrame = FALSE ;	
		m_IfPauseData = TRUE ;
		m_streamParser->Reset(STREAMPARSER_RESET_CONTINUE) ;
	}
	else if (level == Reset_level_YUV)//清空已解码YUV数据缓冲
	{
		if (m_IfDecOnly == FALSE)
		{
			m_video_render.m_YuvDataList.reset() ; 
		}
	}
	else if (level == Reset_level_FASTRETURN)//使解码模块快速返回
	{	
		if (m_IfDecOnly == FALSE)
		{
			m_video_render.m_YuvDataList.setlock(false) ;
			m_video_render.m_YuvDataList.reset() ;
		}
		// 		m_video_render.m_PcmDataList.reset();
		m_IfPauseData = FALSE ;
	}
	else if (level == Reset_level_STREAMPASER)//使解码模块快速返回,且作标记，使重新分析码流类型
	{
		if (m_IfDecOnly == FALSE)
		{
			m_video_render.m_YuvDataList.reset() ;
			m_video_render.m_PcmDataList.reset() ; 
			m_video_render.m_YuvDataList.setlock(true) ;
		}
		m_bFrameNumScan = FALSE;
		m_IfFindIFrame = FALSE ;	
		m_IfPauseData = TRUE ;
		m_streamParser->Reset(STREAMPARSER_RESET_REFIND) ;
	}
}

void DhPlayGraph::SetIfFrameUsed(BOOL IfUsed) 
{
	if (IfUsed && 	m_IfDecOnly == FALSE)
	{
		m_video_render.m_YuvDataList.setlock(true) ;
	}
	
	m_IfUsed = IfUsed ;
}

//是否解码视频
void DhPlayGraph::SetDecVideo(BOOL IfDecVideo)
{
	m_IfDecVideo = IfDecVideo ;
}

//是否解码音频
void DhPlayGraph::SetDecAudio(BOOL IfDecAudio)
{
	m_IfDecAudio = IfDecAudio ;
}

static int MaxZeroLen(unsigned char* pBuf, int iLen)
{
	int maxLen = 0;
	int tmpLen = 0;
	
	for (int i = 0; i < iLen; i++)
	{
		if (pBuf[i] == 0)
		{
			tmpLen++;
		}
		else
		{
			if (tmpLen > maxLen)
			{
				maxLen = tmpLen;
			}
			
			tmpLen = 0;
		}
	}
	
	if (tmpLen > maxLen)
	{
		maxLen = tmpLen;
	}
	
	return maxLen;
}

int DhPlayGraph::decode(DH_FRAME_INFO* frameinfo)
{
	if (m_IfUsed)//如果该数据是要被使用的，送到解码数据缓冲。
	{		
		if (frameinfo->nType == DH_FRAME_TYPE_VIDEO && (m_IfDecOnly ||m_IfDecVideo))//视频解码
		{				
			if (m_IfFindIFrame == FALSE)//如果没找到I帧，不解码
			{
				if (m_IfFindFirstIFrame == TRUE)//为与播放器统计的帧数保持一致，播放器从文件第一个I帧开始统计帧数
				{
					m_callback(m_port, NULL, 0, DEC_YUVDATA, 0, 0) ;
				}
				
				return 0;
			}
			
			BYTE* pWritepos = 0;
			
			if (m_IfDecOnly)
			{
				if (m_yuvBuf == NULL)
				{
					m_yuvBuf = new unsigned char[m_width*m_height*3/2];
					memset(m_yuvBuf, 0, m_width*m_height*3/2);
				}
				pWritepos = m_yuvBuf;
			}
			else
			{
				pWritepos = m_video_render.m_YuvDataList.GetNextWritePos();
				while (!pWritepos)
				{
					if(m_video_render.m_YuvDataList.getlock() == false)
					{
						return -1; //暂停时顺利关闭解码线程 wtg 5.8
					}

					Sleep(10);
					pWritepos = m_video_render.m_YuvDataList.GetNextWritePos();
				}
			}
			
			DemuxCallback(frameinfo);
			
			if (m_Decoder == NULL || m_encode_type == DH_ENCODE_VIDEO_JPEG)
			{
				return 0;
			}
			
			if (m_h264flag == 2 && m_encode_type == DH_ENCODE_VIDEO_H264)
			{
//				m_Decoder = m_hisih264videoDecoder;
//				if (m_hisih264videoDecoder == NULL)
//				{
					m_Decoder = m_h264videoDecoder;
//				}
			}
			
			int nWidth,nHeight,nRate;
			
			if (frameinfo->nEncodeType == DH_ENCODE_VIDEO_MPEG4 && frameinfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)
			{
				if(Mpeg4_Analyse(frameinfo->pContent,frameinfo->nFrameLength,&nWidth,&nHeight,&nRate)<0)
				{
					int wlj = 0;
				}
				else
				{
					if ((nWidth != frameinfo->nWidth || nHeight != frameinfo->nHeight))
					{
					}	
				}
			}
			
			int error = m_Decoder->decode(frameinfo->pContent,frameinfo->nFrameLength,
				pWritepos, m_h264flag, frameinfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME) ;

			BOOL bPicQuality = g_cDHPlayManage.m_nQuality[m_port];
			
			if (bPicQuality)
			{
				LumaStretch(pWritepos, NULL, m_width, m_height);
			}
			
			if (error > 0)
			{
				unsigned char* data = Deinterlace(pWritepos);
				
				//之前AVI转换是在解码之后编码再转，太慢了，现在把重编码这块屏蔽掉，在码流之后就进行avi转换。
				//				ConvertToAVI(data, m_width*m_height*3/2, frameinfo);
				if (m_bYUV2AVIEnable)
				{
					ConvertToAVIEx(data, m_width*m_height*3/2, frameinfo);
				}
				
				if (m_IfDecOnly || (m_IfFirstFrame == FALSE))
				{
					__try
					{
						m_callback(m_port, data, m_width*m_height*3/2, DEC_YUVDATA, 
							m_width, m_height) ;

						m_video_render.m_YuvDataList.GetLastFrameBuf(pWritepos);
					}
					__except(0,1)
					{
						int wlj = 0;
					}
					
					m_IfFirstFrame = TRUE;
					
					return 0;
				}

				// if set visible decode callback
				if ((g_cDHPlayManage.pCallback[m_port] != NULL) && g_cDHPlayManage.pCallback[m_port]->GetCBStatus(CBT_VisibleDec))
				{
					g_cDHPlayManage.pCallback[m_port]->ProcessVisibleDec(DEC_VIDEO, data, m_width*m_height*3/2,
						m_width, m_height);
				}
				
				if (m_transMode != SM_Stream_Stream && m_fixrate != 0)
				{
					m_video_render.changeRate(m_fixrate);
				}
				else if (m_transMode == SM_Stream_Stream)//流模式
				{
					if (frameinfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)
					{
						// 设置初始帧率，期望在下下个I帧消除延时
						int frameCount = m_video_render.m_YuvDataList.chunkCount();
						
						// whf 2008-09-04
						// 为了适应设备8K不定长的打包方式，以及m_filerate不准
						// （比如标称25fps，实际帧率只有24fps）
						// 流畅度参数用来控制解码后缓冲的长度threshold，默认为5
						// 小于此长度减速播放，大于此长度加速播放
						// 将调节过程时间IFrameNum固定为2秒
						// 由此引起的延时平均值可以用threshold来估算
						int threshold = 8;
						int IFrameNum = 2;
						switch(g_cDHPlayManage.m_nFluency[m_port])
						{
						case 1:
							threshold = 0;
							break;
						case 2:
							threshold = 3;
							break;
						case 3:
							threshold = 5;
							break;
						case 4:
							threshold = 8;
							break;
						case 5:
							threshold = 10;
							break;
						case 6:
							threshold = 12;
							break;
						case 7:
							threshold = 14;
							break;
						default:
							threshold = 8;
							break;
						}
						
						if (m_filerate <= 15)
						{
							threshold = 0;
						}
						
						// frames是预计在下个I帧到达时应该放掉的帧数，以维持解码后缓冲趋向于threshold
						int frames = IFrameNum * m_filerate + frameCount - threshold;
						
						if(frames < IFrameNum) // 保证frames正确性
						{
							frames = IFrameNum;
						}				
// #ifdef _DEBUG
// 						char buf[100];
// 						sprintf(buf, "port = %d, threshold = %d, frameCount = %d\n", m_port, threshold, frameCount);
// 						OutputDebugString(buf);		
// #endif
						
						m_userrate = frames / IFrameNum;
						int interval = IFrameNum*1000 / frames;
						m_video_render.changeInterval(interval);
						//m_video_render.m_YuvDataList.SetbReadNull(false);
					}//if (frameinfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)
					
					if (m_video_render.m_YuvDataList.GetIfReadNull() == true)
					{
						//重启定时器
						int timerType = m_video_render.GetTimerType();
						
// 						if (_TIMER_1 == timerType)
// 						{
// 							m_video_render.stopTimer();
// 							m_video_render.startTimer();
// 						}

						// whf 2008-08-30
						// 使用统一的接口来显示解码后的帧，便于帧间隔统计
						m_item->width		= m_width ;
						m_item->height		= m_height ;
						m_item->context		= pWritepos;
						m_item->frameLen	= frameinfo->nLength;
						m_item->frameType	= frameinfo->nSubType;
					
						m_item->ivsobjnum = m_ivsObjNum;
						if (m_ivsObjNum > 0)
						{
							m_item->ivsObjList = m_IvsObjList;
						}
						else
						{
							//m_item->ivsObjList[0].decode_id = m_port;
							m_item->ivsObjList = NULL;
						}

						m_item->tIvsPrePos.pIVSPreposInfo = NULL;
						m_item->tIvsPrePos.lInfoSize = 0;

						if ((m_lPresetPosInfoSize > 0) && (m_pPresetPosInfo != NULL))
						{
							m_item->tIvsPrePos.pIVSPreposInfo = m_pPresetPosInfo;
							m_item->tIvsPrePos.lInfoSize = m_lPresetPosInfoSize;
							m_lPresetPosInfoSize = 0;
						}

						m_ivsObjNum = 0;

						if (m_video_render.m_YuvDataList.write(m_item) == FALSE)
						{
							return -1 ;
						}

						__try
						{
							m_video_render.render();
						}
						__except(0,1)
						{
							int wlj = 0;
						}
					
						m_video_render.m_YuvDataList.SetbReadNull(false);

						return 0;
					}//end if (m_video_render.m_YuvDataList.GetIfReadNull() == true)
				}//end of else if (m_transMode == SM_Stream_Stream)//流模式

				m_item->width		= m_width ;
				m_item->height		= m_height ;
				m_item->context		= pWritepos;
				m_item->frameLen	= frameinfo->nLength;
				m_item->frameType	= frameinfo->nSubType;
				m_item->ivsobjnum	= m_ivsObjNum;
			
				if (m_ivsObjNum > 0)
				{
					m_item->ivsObjList = m_IvsObjList;
				}
				else
				{
					//m_item->ivsObjList[0].decode_id = m_port;
					m_item->ivsObjList = NULL;
				}
				
				m_item->tIvsPrePos.pIVSPreposInfo = NULL;
				m_item->tIvsPrePos.lInfoSize = 0;

				if ((m_lPresetPosInfoSize > 0) && (m_pPresetPosInfo != NULL))
				{
					m_item->tIvsPrePos.pIVSPreposInfo = m_pPresetPosInfo;
					m_item->tIvsPrePos.lInfoSize = m_lPresetPosInfoSize;
					m_lPresetPosInfoSize = 0;
				}
				
				m_ivsObjNum = 0;
			
				if (m_video_render.m_YuvDataList.write(m_item) == FALSE)
				{
					return -1 ;
				}	
			}
			else if (error == 0)
			{
				m_callback(m_port, NULL, 0, DEC_YUVDATA, 0, 0) ;
			}
			else
			{
				m_callback(m_port, NULL, 0, DEC_YUVDATA, 0, 0) ;
			 	m_width = m_height = 0 ;
			 	m_Decoder->reset() ;
				m_IfFindIFrame = FALSE ;
			}

			if (getRate() <= 30 )//正常播放，每解码一帧Sleep，以减小CPU
			{
		 		Sleep(1) ;
			}
		}//if (m_dec_type != DEC_Audio_FRAME && m_IfDecVideo == TRUE )//视频解码
		else if (frameinfo->nType == DH_FRAME_TYPE_AUDIO)//音频解码
		{	
#ifdef FORSIFA
			if (m_IfFindFirstIFrame == FALSE)//测试 //添加此处代码，纯音频文件不能播放-
			{
				return 0;
			}
#endif

			if (m_fixrate != 0)//改变帧时去掉声音
			{
				return 0;
			}

			while (m_IfDecPause && m_IfPauseData)
			{
				if (m_IfStepGo)
				{
					return 0 ;
				}
				
				Sleep(15) ;
			}

			if (frameinfo->nFrameLength <=0 || frameinfo->nFrameLength > 4096)
			{
				return 0 ;
			}

			//bug: 在文件流模式下不能回调音频解码数据
			if (m_transMode == SM_Stream_Stream)//(m_transMode != SM_File)
			{
				if ((m_transMode == SM_Stream_Stream)&&( m_port == 100 || m_port == 99))//语音对讲,适时丢数据
				{
					int count = m_video_render.m_PcmDataList.chunkCount();

					if (count > 2)
					{
						m_flagcount++;
 
						if (m_flagcount > 30)
						{
							m_flagcount = 0;
				 			m_video_render.m_PcmDataList.reset();
						}
					}
					else
					{
						m_flagcount = 0;
					}
				}
				else
				{
					if (m_IfFindFirstIFrame == FALSE)//实时监视，忽略第一个I帧前的音频帧，以使图像快速显示
					{
						return 0;
					}
				}
			}

			m_IfFindAFrame = TRUE;
			m_demuxinfo.nChannels = frameinfo->nChannels;
			m_demuxinfo.nBitsPerSample = frameinfo->nBitsPerSample;
			m_demuxinfo.nSamplesPerSecond = frameinfo->nSamplesPerSecond;

			DemuxCallback(frameinfo);

			int len = m_Decoder->decode(frameinfo->pContent, frameinfo->nFrameLength, 
				m_audioBuf, frameinfo->nEncodeType, frameinfo->nBitsPerSample) ;

			if (!m_bYUV2AVIEnable)
			{
				ConvertToAVI(m_audioBuf, len, frameinfo);
			}
			else
			{
				ConvertToAVIEx(m_audioBuf, len, frameinfo);
			}

			if (m_IfDecAudio == FALSE && m_IfDecOnly == FALSE)//防止纯音频静音快速播完
			{
				if (m_transMode != SM_Stream_Stream && m_IfFindFirstIFrame == FALSE)
				{
					memset(m_audioBuf, 0, len);
				}
				else
				{
					return 0;
				}
			}

			if (len <= 0 || len > AUDIOBUFLEN)
			{
				return 0;
			}

			// if set visible decode callback
			if ((g_cDHPlayManage.pCallback[m_port] != NULL) && g_cDHPlayManage.pCallback[m_port]->GetCBStatus(CBT_VisibleDec))
			{
				g_cDHPlayManage.pCallback[m_port]->ProcessVisibleDec(DEC_AUDIO, m_audioBuf, len,
					frameinfo->nBitsPerSample,frameinfo->nSamplesPerSecond);
			}
	
			if (m_IfDecOnly == TRUE || m_transMode == SM_File)
			{
				m_callback(m_port, m_audioBuf, len, DEC_PCMDATA, 
					frameinfo->nBitsPerSample, frameinfo->nSamplesPerSecond) ;	
			}
			else if (m_transMode == SM_Stream_Stream || m_transMode == SM_Stream_File)
			{
				m_video_render.StartSoundThread();
				DhAVFrame dhavframe;
				memset(&dhavframe, 0, sizeof(DhAVFrame));
				dhavframe.context = m_audioBuf;
				dhavframe.width = frameinfo->nBitsPerSample;
				dhavframe.height = frameinfo->nSamplesPerSecond;
				dhavframe.size = len;

				bool bRet = m_video_render.m_PcmDataList.write(&dhavframe);
				
				if (bRet == FALSE && m_transMode == SM_Stream_File)
				{
					Sleep(45);
					static const int MAXNUM = 3;
					for (int num = 0; num < MAXNUM; num++)
					{
						bRet = m_video_render.m_PcmDataList.write(&dhavframe);
						if (bRet == TRUE)
						{
							break;
						}
						else
						{
							Sleep(15);
						}
					}
				}//end of if (bRet == FALSE && m_transMode == SM_Stream_File)
			}//end of else if (m_transMode == SM_Stream_Stream || m_transMode == SM_Stream_File)
		}
		else if (frameinfo->nType == DH_FRAME_TYPE_DATA)
		{
			if (frameinfo->nSubType == 1)//OSD叠加
			{
				if (frameinfo->nFrameLength > 64*32 
				|| frameinfo->nFrameLength == 0 || frameinfo->nParam2 == 0)
				{
					m_osdInfo.en = 0;
				}
				else
				{
					memcpy(m_osdInfo.data, frameinfo->pContent, frameinfo->nFrameLength);
					m_osdInfo.num = frameinfo->nParam1;
					m_osdInfo.en = frameinfo->nParam2;
					m_osdInfo.x = frameinfo->nWidth;
					m_osdInfo.y = frameinfo->nHeight;
				}
			}
			else if (frameinfo->nSubType == 2)//通道号叠加
			{
				if (frameinfo->nFrameLength > 64*32 
				|| frameinfo->nFrameLength == 0 || frameinfo->nParam2 == 0)
				{
					m_chnInfo.en = 0;
				}
				else
				{
					memcpy(m_chnInfo.data, frameinfo->pContent, frameinfo->nFrameLength);
					m_chnInfo.num = frameinfo->nParam1;
					m_chnInfo.en = frameinfo->nParam2;
					m_chnInfo.x = frameinfo->nWidth;
					m_chnInfo.y = frameinfo->nHeight;
				}
			}
			else if (frameinfo->nSubType == 6)//区域遮挡
			{
				m_shelterinfo.bEn = frameinfo->nParam2;
				m_shelterinfo.SX = frameinfo->nYear;
				m_shelterinfo.SY = frameinfo->nMonth;
				m_shelterinfo.W = frameinfo->nWidth;
				m_shelterinfo.H = frameinfo->nHeight;
				m_shelterinfo.Y = frameinfo->nHour;
				m_shelterinfo.U = frameinfo->nMinute;
				m_shelterinfo.V = frameinfo->nSecond;
			}			
		}
	}
	else//把该帧数据丢弃
	{
		if (frameinfo->nType == DH_FRAME_TYPE_VIDEO && m_IfFindIFrame == TRUE  
			&& m_IfDecVideo == TRUE)//视频解码
		{
			if (m_h264flag == 2)
			{
// 				m_Decoder = m_hisih264videoDecoder;
// 				if (m_hisih264videoDecoder == NULL)
// 				{
					m_Decoder = m_h264videoDecoder;
// 				}
			}

			if (m_yuvBuf == NULL)
			{
				m_yuvBuf = new unsigned char[m_width*m_height*3/2];
				memset(m_yuvBuf, 0, m_width*m_height*3/2);
			}

			int error = m_Decoder->decode(frameinfo->pContent,frameinfo->nFrameLength,m_yuvBuf
				,m_h264flag,m_height) ;

			if (error <= 0)
			{
				m_Decoder->reset() ;
				m_IfFindIFrame = FALSE ;
				m_width = m_height = 0 ;
			}
		}
	}	

	return 0 ;
}
	
inline
int DhPlayGraph::AdjustRateInter()
{
	int iRet = 0;

	int framecount = m_video_render.m_YuvDataList.chunkCount();

	if (framecount >= m_adjust_bufnum && framecount <= m_adjust_bufnum + 2)
	{
		if (m_userrate >= 25 && m_userrate < 50)
		{
			m_userrate = m_adjust_range/2 + 25 ;
			changeRate(m_userrate) ;
		}
	}
	else if (framecount > m_adjust_bufnum + 2)
	{
		if (m_userrate >= 25 && m_userrate < 50)
		{
			m_userrate = m_adjust_range + 25 ;
			changeRate(m_userrate) ;
		}
	}
	else if (framecount > m_adjust_bufnum + 4)
	{
		if (m_userrate >= 25 && m_userrate < 50)
		{
			m_userrate = m_adjust_range + 2 + 25 ;
			changeRate(m_userrate) ;
		}
	}
	else if (framecount < m_adjust_bufnum )
	{
		if (m_userrate >= 25 && m_userrate < 50)
		{
			m_userrate = 25 ;
			changeRate(m_userrate) ;
		}
	}

	return iRet;
}

void DhPlayGraph::AdjustFluency(int bufnum, int adjustrange)
{
	if (bufnum >= 0 && bufnum <= 15)
	{
		m_adjust_bufnum = bufnum;
	}

	if (adjustrange >=0 && adjustrange < 25)
	{
		m_adjust_range = adjustrange; 
	}
}

bool DhPlayGraph::FixRate(int rate)
{
	bool bRet = true;

	if (rate < 0 || rate > 250)
	{
		bRet = false;
	}
	else if (rate == 0)
	{
		changeRate(m_userrate);
		m_fixrate = rate;
	}
	else
	{
		m_fixrate = rate;
	}

	return bRet;
}

extern  void De_interlace(unsigned char *img,int LINE_PXL,int LINE_NUM) ;
static inline void swapimg(unsigned char *src, unsigned char *dst, int width, int height);

/*********************************************************
 * invert: 颜色翻转(背景色)
***********************************************************/
inline void draw_a_character_16x16(
	unsigned long x,unsigned long y,
	unsigned long stride ,
	unsigned char *src,
	unsigned char *dst_base,
	unsigned char val,
	unsigned char zoom ,
	bool invert)
{
	// 字符必须是16X16
	unsigned char i,j;
	unsigned long Value_Y = 0 ;
	unsigned char color;
	unsigned short data;

	if (invert) {
		for (i=0;i<16;i++) {
			data = src[2*i]<<8|src[2*i+1];
			for (j=0;j<16;j++) {
				if (data&(0x0001<<j)) // 测位
					continue; // 如果注释，则反色
				{
					color = *(unsigned char *)(dst_base+stride*(i+y)+x+16-j);
					*(unsigned char *)(dst_base+stride*(i+y)+x+16-j)= 255- color;
				}
			}
		}
	}
}

static inline void shelter(unsigned char* img, int width, int height, SHELTERINFO shelterinfo)
{
	int i = 0;

	if ( (shelterinfo.bEn == FALSE) || (shelterinfo.SX + shelterinfo.W > width) 
		|| (shelterinfo.SY + shelterinfo.H > height) )
	{
		return;
	}

	for (i = shelterinfo.SY; i < shelterinfo.SY + shelterinfo.H; i++)
	{
		memset(img+i*width+shelterinfo.SX, shelterinfo.Y, shelterinfo.W);
	}

	for (i = shelterinfo.SY/2; i < shelterinfo.SY/2 + shelterinfo.H/2; i++)
	{
		memset(img+width*height+i*width/2+shelterinfo.SX/2, shelterinfo.U, shelterinfo.W/2);
	}

	for (i = shelterinfo.SY/2; i < shelterinfo.SY/2 + shelterinfo.H/2; i++)
	{
		memset(img+width*height*5/4+i*width/2+shelterinfo.SX/2, shelterinfo.V, shelterinfo.W/2);
	}
}


unsigned char* DhPlayGraph::Deinterlace(unsigned char* data)
{
	unsigned char* cRet = data;

	switch (m_deinterlace) 
	{
	case 0:
		if (m_width==704&&m_height==576
			|| m_width==704&&m_height==480
			|| m_width==352&&m_height==576
			|| m_width==352&&m_height==480) 
		{
			if (m_preBuffer == 0)
			{
				m_preBuffer = new unsigned char[m_width*m_height*3/2];
			}

			if (m_hDeinterlace == NULL)
			{
				m_struPara.iFieldWidth  = m_width ;
				m_struPara.iFieldHeight = m_height/2 ;
				m_struPara.iSrcYPitch   = m_width ;
				m_struPara.iSrcUVPitch  = m_width/2;
				m_struPara.iDstYPitch   = m_width;//outframe.iYStride;     //此例中输出Y分量跨度设为Y分量宽度.
				m_struPara.iDstUVPitch  = m_width/2;//outframe.iUVStride; //此例中输出UV分量跨度设为UV分量宽度.
				
				HI_InitDeinterlace(&m_hDeinterlace, m_struPara);  //调用Deinterlace初始化函数.
			}
		
			swapimg(data, m_preBuffer, m_width, m_height);

			cRet = m_preBuffer;
			memcpy(data, m_preBuffer, m_width*m_height*3/2);

//			m_tempLen = m_width*m_height*3/2;
// 			char temp[100];
// 			sprintf("m_width = %d,m_height = %d,m_tempLen = %d\n",m_width,m_height,m_tempLen);
// 			OutputDebugString(temp);
		
		} 
		break;
	case 1:
	case 3:
		if (m_width==704&&m_height==576
			|| m_width==704&&m_height==480
			|| m_width==352&&m_height==576
			|| m_width==352&&m_height==480) 
		{
	 		De_interlace(data,m_width,m_height) ;
			cRet = data;
		}
		break;
	
	case 2:
		break ;

	case 4:
		if (m_width==640&&m_height==480) 
		{
			if (m_preBuffer == 0)
			{
				m_preBuffer = new unsigned char[m_width*m_height*3/2];
			}

			if (m_hDeinterlace == NULL)
			{
				m_struPara.iFieldWidth  = m_width ;
				m_struPara.iFieldHeight = m_height/2 ;
				m_struPara.iSrcYPitch   = m_width ;
				m_struPara.iSrcUVPitch  = m_width/2;
				m_struPara.iDstYPitch   = m_width;//outframe.iYStride;     //此例中输出Y分量跨度设为Y分量宽度.
				m_struPara.iDstUVPitch  = m_width/2;//outframe.iUVStride; //此例中输出UV分量跨度设为UV分量宽度.
				
				HI_InitDeinterlace(&m_hDeinterlace, m_struPara);  //调用Deinterlace初始化函数.
			}
		
			swapimg(data, m_preBuffer, m_width, m_height);

			cRet = m_preBuffer;

			memcpy(data, m_preBuffer, m_width*m_height*3/2);
		} 
		break;

	case 5:
		if (m_width==640 && m_height==480) 
		{
	 		De_interlace(data,m_width,m_height) ;
			cRet = data;
		}
		break;
	
	default:
		break;
	}

	if (m_osdInfo.en) 
	{
		for (int i=0;i<m_osdInfo.num;++i)
		{
			draw_a_character_16x16(
				m_osdInfo.x+16*i,m_osdInfo.y,
				m_width,m_osdInfo.data+i*32,data,0,0,1);
		}
	}

	if (m_chnInfo.en) 
	{
		for (int i=0;i<m_chnInfo.num;++i)
		{
			draw_a_character_16x16(
				m_chnInfo.x+16*i,m_chnInfo.y,
				m_width,m_chnInfo.data+i*32,data,0,0,1);
		}
	}

  	shelter(cRet, m_width, m_height, m_shelterinfo);

	return cRet;
}

inline void DhPlayGraph::swapimg(unsigned char *src, unsigned char *dst, int width, int height)
{
	DEINTERLACE_FRAME_S struDstFrame;
	struDstFrame.pszY = dst;
	struDstFrame.pszU = struDstFrame.pszY + width * height;
	struDstFrame.pszV = struDstFrame.pszU + width * height / 4 ;
	memset(dst, 0, width*height*3/2);

	//调用deinterlace主函数.它完成两场输出一帧图像的功能.
	int nError = HI_Deinterlace(
		m_hDeinterlace,                                     //句柄
		struDstFrame,                             //输出帧图像结构体
		src,                                      //输入场YUV420的Y分量
		src+width*height,						//输入场YUV420的U分量
		src+width*height*5/4,					//输入场YUV420的V分量
		PIC_INTERLACED_ODD);


	nError = HI_Deinterlace(
		m_hDeinterlace,                                     //句柄
		struDstFrame,                             //输出帧图像结构体
		src+width*height/2,                                      //输入场YUV420的Y分量
		src+width*height*9/8,						//输入场YUV420的U分量
		src+width*height*11/8,					//输入场YUV420的V分量
		PIC_INTERLACED_EVEN);
}

BYTE* DhPlayGraph::GetLastFrame()
{
	if (m_IfDecOnly)
	{
		return NULL;
	}

	return m_video_render.m_YuvDataList.GetLastFrame();
}

inline
int DhPlayGraph::DemuxCallback(DH_FRAME_INFO* frameinfo)
{
	if (g_cDHPlayManage.pCallback[m_port] == NULL 
		|| !g_cDHPlayManage.pCallback[m_port]->GetCBStatus(CBT_DemuxCBFun)
		|| m_IfFindIFrame == FALSE)
	{
		return -1;
	}

	tagDemuxInfo demuxinfo;
	demuxinfo.type = frameinfo->nType;
	demuxinfo.subtype = frameinfo->nSubType + 1;
	demuxinfo.encode = frameinfo->nEncodeType;			// MPEG4, H264, STDH264
	demuxinfo.sequence = m_PframeNum;		// I帧间隔里面的序号
	
	// 图像大小信息
	demuxinfo.width = m_demuxinfo.nWidth;
	demuxinfo.height = m_demuxinfo.nHeight;
	
	// 帧率信息
	demuxinfo.rate = m_demuxinfo.nFrameRate;
	
	GetTimeStr((char*)&demuxinfo.year);
	demuxinfo.timestamp = GetCurTimeEx() ;
	demuxinfo.channels = frameinfo->nChannels;
	demuxinfo.bitspersample = frameinfo->nBitsPerSample;
	demuxinfo.samplespersecond = frameinfo->nSamplesPerSecond;

 	g_cDHPlayManage.pCallback[m_port]->ProcessDemux((char*)frameinfo->pHeader, frameinfo->nLength, &demuxinfo);

	return 0;
}

BOOL DhPlayGraph::GetMediaInfo(char* pBuf, int len)
{
	if (pBuf == NULL || len < 24)
	{
		return FALSE;
	}

	if (!m_IfFindFirstIFrame)
	{
		return FALSE;
	}

	memset(pBuf, 0, len);
	
	memcpy(pBuf, &m_demuxinfo.nWidth, sizeof(long));
	memcpy(pBuf+sizeof(long), &m_demuxinfo.nHeight, sizeof(long));
	memcpy(pBuf+sizeof(long)*2, &m_demuxinfo.nFrameRate, sizeof(long));
	memcpy(pBuf+sizeof(long)*3, &m_demuxinfo.nChannels, sizeof(long));
	memcpy(pBuf+sizeof(long)*4, &m_demuxinfo.nBitsPerSample, sizeof(long));
	memcpy(pBuf+sizeof(long)*5, &m_demuxinfo.nSamplesPerSecond, sizeof(long));
	
	return TRUE;
}

void DhPlayGraph::GetTimeStr(char* pBuf)
{
	if (m_streamParser->GetStreamType() == DH_STREAM_NEW)
	{
		if ( m_filerate <= 0 )
		{
			return ;
		}
		time_t timestamp = m_demuxinfo.nTimeStamp + m_PframeNum / m_filerate;
		tm *newtime = localtime(&timestamp ); 
		if (newtime == NULL)
		{
			return;
		}
		int year = newtime->tm_year+1900;
// 		int day = newtime->tm_mday - 1;
		int mon = newtime->tm_mon + 1;
		memcpy(pBuf, &year, sizeof(int));
		memcpy(pBuf+sizeof(int), &mon, sizeof(int));
		memcpy(pBuf+sizeof(int)*2, &newtime->tm_mday, sizeof(int));
		memcpy(pBuf+sizeof(int)*3, &newtime->tm_hour, sizeof(int));
		memcpy(pBuf+sizeof(int)*4, &newtime->tm_min, sizeof(int));
		memcpy(pBuf+sizeof(int)*5, &newtime->tm_sec, sizeof(int));
	}
	else
	{
		memcpy(pBuf, &m_demuxinfo.nYear, sizeof(int));
		memcpy(pBuf+sizeof(int), &m_demuxinfo.nMonth, sizeof(int));
		memcpy(pBuf+sizeof(int)*2, &m_demuxinfo.nDay, sizeof(int));
		memcpy(pBuf+sizeof(int)*3, &m_demuxinfo.nHour, sizeof(int));
		memcpy(pBuf+sizeof(int)*4, &m_demuxinfo.nMinute, sizeof(int));
		memcpy(pBuf+sizeof(int)*5, &m_demuxinfo.nSecond, sizeof(int));
	}
}

void DhPlayGraph::SetWaterMarkCallback(void *pvCallback, long userData)
{
	m_wmCallback = (watermark_callback)pvCallback;
	m_watermark_user = userData;
}

void DhPlayGraph::SetPandoraWaterMarkCallback(void *pvCallback, long userData)
{
	m_wmPandoraCallback = (watermark_callback)pvCallback;
	m_pandorawatermark_user = userData;
}

void DhPlayGraph::SetIVSCallback(void *pIVSFunc, long nUserData)
{
	m_IVSCallback = (IVS_callback)pIVSFunc;
	m_IVS_user = nUserData;
}

void DhPlayGraph::StopAVIConv()
{
	m_status = AVI_STOP;
}

void DhPlayGraph::SetAVIConvSize(long lWidth, long lHeight)
{
	m_bYUV2AVIEnable = TRUE;
	m_lAVIDesWidth = lWidth;
	m_lAVIDesHeight = lHeight;
	
	m_pYUVResizeBuf = new BYTE[m_lAVIDesWidth*m_lAVIDesHeight*3/2];
}

void DhPlayGraph::ConvToAVI(unsigned char* pAVIFileName)
{
	if (1)
	{
	// 	AutoLock lock(&g_h264_critsec);
		g_playgraph_critsec.Lock();
		static int g_initall = 0;
		if (g_initall == 0)
		{
			HINSTANCE hInst;
			hInst = LoadLibrary("dllmpeg4.dll");
			m_Fun_MP4_Init = (MP4_Xvid_Init)GetProcAddress(hInst, "xvid_init_dll");
			m_Fun_Mp4_Encore = (MP4_Xvid_Decore)GetProcAddress(hInst, "xvid_encore_dll");
			AVIFileInit();	
			XVID_INIT_PARAM enc_init;
			enc_init.cpu_flags = 0;
			if (m_Fun_MP4_Init)
			{
				m_Fun_MP4_Init(NULL, 0, &enc_init, NULL);
			}
			g_initall = 1;
		}
		g_playgraph_critsec.UnLock();
	}

	if (m_Fun_MP4_Init == NULL ||m_Fun_Mp4_Encore == NULL)
	{
		return ;
	}

	if (m_bYUV2AVIEnable)
	{
		m_aviinfo.m_encBuffer = new unsigned char [256*1024];
	}

	::CoInitialize(NULL);
	
	//保存第一次打开时后的文件名
	if ( m_fileChangeCount == 0 && m_fileEncChangeCount == 0 && m_fileRateChangeCount == 0)
	{
		memset(m_aviFileName , 0 , MAX_PATH);
		memcpy(m_aviFileName, pAVIFileName , strlen((const char*)pAVIFileName));
	}

	HRESULT hr = AVIFileOpen(&m_aviinfo.m_hAVIFile, (const char*)pAVIFileName, OF_WRITE | OF_CREATE, NULL);
	
	if(hr!=AVIERR_OK) 
	{
		return;
	}

	m_status = AVI_RUN;
}

void DhPlayGraph::ConvertToAVIEx(unsigned char * pBuf, long nSize, DH_FRAME_INFO* pFrameInfo)
{
	HRESULT hr = 0;
	int err = 0;

	if (m_status == AVI_STOP)
	{
		StopAVIConvReal();

		if (m_pYUVResizeBuf != NULL)
		{
			delete[] m_pYUVResizeBuf;
			m_pYUVResizeBuf = NULL;
		}
		m_bYUV2AVIEnable = FALSE;
	}
	else if (m_status == AVI_IDL)
	{
		return ;
	}
	
	if (m_aviinfo.m_hAVIFile == NULL || m_Fun_MP4_Init == NULL ||m_Fun_Mp4_Encore == NULL)
	{
		return ;
	}
	
	if (DH_FRAME_TYPE_VIDEO == pFrameInfo->nType)
	{
		m_aviinfo.m_foundIFrame = TRUE;

		if(m_aviinfo.enc_param.handle == NULL) 
		{	
			int width = m_width;
			int height = m_height;
			
			if ((m_width>=640 && m_width<=720) && (m_height<=288 && m_height>=240))
			{
				height = height * 2;
			}
			else if ((m_width<=352 && m_width>=320) && (m_height<=576 && m_height>=480)) 
			{
				width = width * 2;
			}

			if (m_bYUV2AVIEnable)
			{
				m_aviinfo.enc_param.width	= m_lAVIDesWidth;
				m_aviinfo.enc_param.height	= m_lAVIDesHeight;
			}
			else
			{
				m_aviinfo.enc_param.width	= width;
				m_aviinfo.enc_param.height	= height;
			}
			
			m_aviinfo.enc_param.fincr	= 1;
			m_aviinfo.enc_param.fbase	= 25;
			m_aviinfo.enc_param.rc_reaction_delay_factor = 16;
			m_aviinfo.enc_param.rc_averaging_period = 100;
			m_aviinfo.enc_param.rc_buffer	= 100;
			m_aviinfo.enc_param.rc_bitrate	= 900*1000; 
			m_aviinfo.enc_param.min_quantizer	= 2;
			m_aviinfo.enc_param.max_quantizer	= 32;
			m_aviinfo.enc_param.max_key_interval	= m_filerate;
		
			err = m_Fun_Mp4_Encore(NULL, XVID_ENC_CREATE, &m_aviinfo.enc_param, NULL);
			if(err!=XVID_ERR_OK)
			{
				m_aviinfo.enc_param.handle=NULL;
			}

			if (m_aviinfo.m_hVideoStream == NULL)
			{
				m_aviinfo.strhdr.fccType			= streamtypeVIDEO;
				m_aviinfo.strhdr.fccHandler		= mmioFOURCC('X','V','I','D') ;
				m_aviinfo.strhdr.dwQuality		= -1;
				m_aviinfo.strhdr.dwScale			= 1;
				m_aviinfo.strhdr.dwRate			= m_filerate;

				hr = AVIFileCreateStream(m_aviinfo.m_hAVIFile,
					&m_aviinfo.m_hVideoStream,&m_aviinfo.strhdr);


				if (hr==AVIERR_OK)
				{
					BITMAPINFOHEADER bitmaphdr;		
					bitmaphdr.biSize			= sizeof(BITMAPINFOHEADER);
					
					if (m_bYUV2AVIEnable)
					{
						bitmaphdr.biWidth		= m_lAVIDesWidth;
						bitmaphdr.biHeight		= m_lAVIDesHeight;
					}
					else
					{
						bitmaphdr.biWidth		= width;
						bitmaphdr.biHeight		= height;
					}
							
					bitmaphdr.biPlanes			= 1;
					bitmaphdr.biBitCount		= 24;
					bitmaphdr.biCompression		= mmioFOURCC('X','V','I','D') ;
					bitmaphdr.biSizeImage		= bitmaphdr.biWidth*bitmaphdr.biHeight*3;
					bitmaphdr.biXPelsPerMeter	= 0;
					bitmaphdr.biYPelsPerMeter	= 0;
					bitmaphdr.biClrUsed			= 0;
					bitmaphdr.biClrImportant	= 0;
					
					AVIStreamSetFormat(m_aviinfo.m_hVideoStream, 0
						, &bitmaphdr, bitmaphdr.biSize);
				}
			}
		}//end of if(m_aviinfo.enc_param.handle == NULL) 

		if (m_aviinfo.enc_param.handle)
		{
			unsigned char* tmp_zgf = NULL;
			if ((m_width>=640 && m_width<=720) && (m_height<=288 && m_height>=240))
			{
				tmp_zgf = new unsigned char[ m_width*m_height*3] ;

				int i ;
				for (i = 0 ; i < m_height ; i++)
				{
					memcpy(tmp_zgf + 2*i * m_width, pBuf + i*m_width, m_width);
					memcpy(tmp_zgf + (2*i + 1) * m_width, pBuf + i*m_width, m_width) ;
				}

				for (i = 0 ; i < m_height/2 ;i++)
				{
					memcpy(tmp_zgf+m_width*2*m_height + 2*i * m_width/2, pBuf + m_width*m_height + i*m_width/2, m_width/2);
					memcpy(tmp_zgf+m_width*2*m_height + (2*i + 1) * m_width/2, pBuf + m_width*m_height + i*m_width/2, m_width/2) ;
				}

				for (i = 0 ; i < m_height/2 ;i++)
				{
					memcpy(tmp_zgf+m_width*5*m_height/2 + 2*i * m_width/2, pBuf + m_width*m_height*5/4 + i*m_width/2, m_width/2);
					memcpy(tmp_zgf+m_width*5*m_height/2 + (2*i + 1) * m_width/2, pBuf + m_width*m_height*5/4 + i*m_width/2, m_width/2) ;
				}
			}
			else if ((m_width<=352 && m_width>=320) && (m_height<=576 && m_height>=480)) 
			{
				tmp_zgf = new unsigned char[ m_width*m_height*3] ;

				for (int i = 0 ; i < m_width*m_height*3/2 ; i++)
				{
					tmp_zgf[i*2] = tmp_zgf[i*2 + 1] = pBuf[i] ;
				}

			}

			if (m_lAVIDesWidth != m_width ||
				m_lAVIDesHeight != m_height)
			{
				img_conv((tmp_zgf ? tmp_zgf : pBuf), m_width, m_height, m_pYUVResizeBuf, m_lAVIDesWidth, m_lAVIDesHeight);
				m_aviinfo.enc_frame.image = m_pYUVResizeBuf;
			}
			else
			{
				m_aviinfo.enc_frame.image = tmp_zgf ? tmp_zgf : pBuf;
			}

			m_aviinfo.enc_frame.bitstream = m_aviinfo.m_encBuffer;
			m_aviinfo.enc_frame.length = -1; 
			
			m_aviinfo.enc_frame.colorspace =XVID_CSP_I420;
			m_aviinfo.enc_frame.intra = -1; 
			m_aviinfo.enc_frame.quant = 3;
			m_aviinfo.enc_frame.motion = 0;
			m_aviinfo.enc_frame.general = XVID_H263QUANT;


			err = m_Fun_Mp4_Encore(
				m_aviinfo.enc_param.handle, XVID_ENC_ENCODE, 
				&m_aviinfo.enc_frame, &m_aviinfo.enc_stats);
			
			HRESULT hr = AVIStreamWrite(m_aviinfo.m_hVideoStream, 
				m_aviinfo.m_aviVideoWritePos, 1, m_aviinfo.m_encBuffer,
				m_aviinfo.enc_frame.length,
				m_aviinfo.enc_frame.intra==1?AVIIF_KEYFRAME:0,
				NULL,NULL);

			if (tmp_zgf)
			{
				delete tmp_zgf;
			}

//			HRESULT hr = AVIStreamWrite(m_aviinfo.m_hVideoStream, 
//				m_aviinfo.m_aviVideoWritePos, 1, pFrameInfo->pContent,
//				pFrameInfo->nFrameLength,
//				pFrameInfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME,
//				NULL,NULL);


			m_aviinfo.m_aviVideoWritePos++;
		}
	}//end of if (DH_FRAME_TYPE_VIDEO == dataType)
	else if (DH_FRAME_TYPE_AUDIO == pFrameInfo->nType)
	{
		if (m_aviinfo.m_foundIFrame == FALSE)
		{
			return;
		}

		if (m_aviinfo.m_hAudioStream == NULL)
		{
			WAVEFORMATEX WaveFormat;
			ZeroMemory (&WaveFormat, sizeof (WAVEFORMATEX));
		
			if (hr == AVIERR_OK)
			{
				WaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
				WaveFormat.nChannels		= 1;
				WaveFormat.nSamplesPerSec	= pFrameInfo->nSamplesPerSecond;
				WaveFormat.wBitsPerSample	= pFrameInfo->nBitsPerSample;
				WaveFormat.nAvgBytesPerSec	= (WaveFormat.nSamplesPerSec *WaveFormat.nChannels*WaveFormat.wBitsPerSample  / 8);
				WaveFormat.nBlockAlign		= WaveFormat.nChannels * WaveFormat.wBitsPerSample  / 8;
			
				m_aviinfo.strhdr.fccType			= streamtypeAUDIO;
				m_aviinfo.strhdr.fccHandler		= WAVE_FORMAT_PCM;
				m_aviinfo.strhdr.dwQuality		= -1;
				m_aviinfo.strhdr.dwScale			= WaveFormat.nBlockAlign;
				m_aviinfo.strhdr.dwRate			= WaveFormat.nAvgBytesPerSec;
				m_aviinfo.strhdr.dwSampleSize	= WaveFormat.nBlockAlign;
			
				hr = AVIFileCreateStream(m_aviinfo.m_hAVIFile,
					&m_aviinfo.m_hAudioStream,&m_aviinfo.strhdr);
				if (hr==AVIERR_OK)
				{
					hr = AVIStreamSetFormat (m_aviinfo.m_hAudioStream, 0, &WaveFormat, sizeof (WAVEFORMATEX));
					if(hr!=AVIERR_OK)
					{
						AVIStreamClose(m_aviinfo.m_hAudioStream);
						m_aviinfo.m_hAudioStream = 0;
					}
				}
			}
		}//end of 	if (m_aviinfo.m_hAudioStream == NULL)

		hr = AVIStreamWrite(m_aviinfo.m_hAudioStream, m_aviinfo.m_aviAudioWritePos, 
			1, pBuf, nSize,0,NULL,NULL);
		m_aviinfo.m_aviAudioWritePos++;
	}//end of else if (DH_FRAME_TYPE_AUDIO == dataType)
}

void DhPlayGraph::StopAVIConvReal()
{
	if (m_aviinfo.m_hAVIFile)
	{
		if (m_aviinfo.m_hVideoStream)
			AVIStreamClose(m_aviinfo.m_hVideoStream);
		if (m_aviinfo.m_hAudioStream)
			AVIStreamClose(m_aviinfo.m_hAudioStream);
		if (m_aviinfo.m_hAVIFile)
			AVIFileClose(m_aviinfo.m_hAVIFile);
		
		m_aviinfo.m_hVideoStream=NULL;
		m_aviinfo.m_hAudioStream=NULL;
		m_aviinfo.m_hAVIFile=NULL;

		if (m_Fun_Mp4_Encore)
		{
			m_Fun_Mp4_Encore(m_aviinfo.enc_param.handle,XVID_ENC_DESTROY,NULL,NULL);	
		}
		m_aviinfo.enc_param.handle = 0;
		m_aviinfo.m_aviVideoWritePos = 0;
		m_aviinfo.m_aviAudioWritePos = 0;
		m_fileChangeCount = 0;
		m_fileEncChangeCount = 0;
		m_fileRateChangeCount = 0;
	}
}

void DhPlayGraph::StopAVIConvRealImp()
{
	if (m_aviinfo.m_hAVIFile)
	{
		if (m_aviinfo.m_hVideoStream)
			AVIStreamClose(m_aviinfo.m_hVideoStream);
		if (m_aviinfo.m_hAudioStream)
			AVIStreamClose(m_aviinfo.m_hAudioStream);
		if (m_aviinfo.m_hAVIFile)
			AVIFileClose(m_aviinfo.m_hAVIFile);
		
		m_aviinfo.m_hVideoStream=NULL;
		m_aviinfo.m_hAudioStream=NULL;
		m_aviinfo.m_hAVIFile=NULL;
		
		if (m_Fun_Mp4_Encore)
		{
			m_Fun_Mp4_Encore(m_aviinfo.enc_param.handle,XVID_ENC_DESTROY,NULL,NULL);	
		}
		m_aviinfo.enc_param.handle = 0;
		m_aviinfo.m_aviVideoWritePos = 0;
		m_aviinfo.m_aviAudioWritePos = 0;
	}
}

void DhPlayGraph::ConvertToAVI(unsigned char * pBuf, long nSize, DH_FRAME_INFO* pFrameInfo)
{
	HRESULT hr = 0;
	int err = 0;

	if (m_status == AVI_STOP)
	{
		StopAVIConvReal();
	}
	else if (m_status == AVI_IDL)
	{
		return ;
	}
	
	if (m_aviinfo.m_hAVIFile == NULL /*|| m_Fun_MP4_Init == NULL ||m_Fun_Mp4_Encore == NULL*/)
	{
		return ;
	}
	
	if (DH_FRAME_TYPE_VIDEO == pFrameInfo->nType)
	{
		if (m_IfFindIFrame == FALSE)//如果没找到I帧，不转
		{
			return ;
		}
		
		m_aviinfo.m_foundIFrame = TRUE;
		/*
		if(m_aviinfo.enc_param.handle == NULL) 
		{	
			int width = m_width;
			int height = m_height;
			
			if ((m_width>=640 && m_width<=720) && (m_height<=288 && m_height>=240))
			{
				height = height * 2;
			}
			else if ((m_width<=352 && m_width>=320) && (m_height<=576 && m_height>=480)) 
			{
				width = width * 2;
			}

			m_aviinfo.enc_param.width	= width;
			m_aviinfo.enc_param.height	= height;
			m_aviinfo.enc_param.fincr	= 1;
			m_aviinfo.enc_param.fbase	= 25;
			m_aviinfo.enc_param.rc_reaction_delay_factor = 16;
			m_aviinfo.enc_param.rc_averaging_period = 100;
			m_aviinfo.enc_param.rc_buffer	= 100;
			m_aviinfo.enc_param.rc_bitrate	= 900*1000; 
			m_aviinfo.enc_param.min_quantizer	= 2;
			m_aviinfo.enc_param.max_quantizer	= 32;
			m_aviinfo.enc_param.max_key_interval	= m_filerate;
		
			err = m_Fun_Mp4_Encore(NULL, XVID_ENC_CREATE, &m_aviinfo.enc_param, NULL);
			if(err!=XVID_ERR_OK)
			{
				m_aviinfo.enc_param.handle=NULL;
			}

			if (m_aviinfo.m_hVideoStream == NULL)
			{
				m_aviinfo.strhdr.fccType			= streamtypeVIDEO;
				m_aviinfo.strhdr.fccHandler		= mmioFOURCC('X','V','I','D') ;
				m_aviinfo.strhdr.dwQuality		= -1;
				m_aviinfo.strhdr.dwScale			= 1;
				m_aviinfo.strhdr.dwRate			= m_filerate;

				hr = AVIFileCreateStream(m_aviinfo.m_hAVIFile,
					&m_aviinfo.m_hVideoStream,&m_aviinfo.strhdr);


				if (hr==AVIERR_OK)
				{
					BITMAPINFOHEADER bitmaphdr;		
					bitmaphdr.biSize			= sizeof(BITMAPINFOHEADER);
					bitmaphdr.biWidth			= width;
					bitmaphdr.biHeight		= height;		
					bitmaphdr.biPlanes			= 1;
					bitmaphdr.biBitCount		= 24;
					bitmaphdr.biCompression		= mmioFOURCC('X','V','I','D') ;
					bitmaphdr.biSizeImage		= bitmaphdr.biWidth*bitmaphdr.biHeight*3;
					bitmaphdr.biXPelsPerMeter	= 0;
					bitmaphdr.biYPelsPerMeter	= 0;
					bitmaphdr.biClrUsed			= 0;
					bitmaphdr.biClrImportant	= 0;
					
					AVIStreamSetFormat(m_aviinfo.m_hVideoStream, 0
						, &bitmaphdr, bitmaphdr.biSize);
				}
			}
		}//end of if(m_aviinfo.enc_param.handle == NULL) 

		if (m_aviinfo.enc_param.handle)
		{
			unsigned char* tmp_zgf = NULL;
			if ((m_width>=640 && m_width<=720) && (m_height<=288 && m_height>=240))
			{
				tmp_zgf = new unsigned char[ m_width*m_height*3] ;

				int i ;
				for (i = 0 ; i < m_height ; i++)
				{
					memcpy(tmp_zgf + 2*i * m_width, pBuf + i*m_width, m_width);
					memcpy(tmp_zgf + (2*i + 1) * m_width, pBuf + i*m_width, m_width) ;
				}

				for (i = 0 ; i < m_height/2 ;i++)
				{
					memcpy(tmp_zgf+m_width*2*m_height + 2*i * m_width/2, pBuf + m_width*m_height + i*m_width/2, m_width/2);
					memcpy(tmp_zgf+m_width*2*m_height + (2*i + 1) * m_width/2, pBuf + m_width*m_height + i*m_width/2, m_width/2) ;
				}

				for (i = 0 ; i < m_height/2 ;i++)
				{
					memcpy(tmp_zgf+m_width*5*m_height/2 + 2*i * m_width/2, pBuf + m_width*m_height*5/4 + i*m_width/2, m_width/2);
					memcpy(tmp_zgf+m_width*5*m_height/2 + (2*i + 1) * m_width/2, pBuf + m_width*m_height*5/4 + i*m_width/2, m_width/2) ;
				}
			}
			else if ((m_width<=352 && m_width>=320) && (m_height<=576 && m_height>=480)) 
			{
				tmp_zgf = new unsigned char[ m_width*m_height*3] ;

				for (int i = 0 ; i < m_width*m_height*3/2 ; i++)
				{
					tmp_zgf[i*2] = tmp_zgf[i*2 + 1] = pBuf[i] ;
				}

			}

			m_aviinfo.enc_frame.bitstream = m_aviinfo.m_encBuffer;
			m_aviinfo.enc_frame.length = -1; 
			m_aviinfo.enc_frame.image = tmp_zgf ? tmp_zgf : pBuf;
			m_aviinfo.enc_frame.colorspace =XVID_CSP_I420;
			m_aviinfo.enc_frame.intra = -1; 
			m_aviinfo.enc_frame.quant = 3;
			m_aviinfo.enc_frame.motion = 0;
			m_aviinfo.enc_frame.general = XVID_H263QUANT;


			err = m_Fun_Mp4_Encore(
				m_aviinfo.enc_param.handle, XVID_ENC_ENCODE, 
				&m_aviinfo.enc_frame, &m_aviinfo.enc_stats);
			
			HRESULT hr = AVIStreamWrite(m_aviinfo.m_hVideoStream, 
				m_aviinfo.m_aviVideoWritePos, 1, m_aviinfo.m_encBuffer,
				m_aviinfo.enc_frame.length,
				m_aviinfo.enc_frame.intra==1?AVIIF_KEYFRAME:0,
				NULL,NULL);

			if (tmp_zgf)
			{
				delete tmp_zgf;
			}*/	
			if (m_aviinfo.m_hVideoStream == NULL)
			{
				if(pFrameInfo->nEncodeType == DH_ENCODE_VIDEO_MPEG4)
				{
					m_aviinfo.strhdr.fccType		= streamtypeVIDEO;
					m_aviinfo.strhdr.fccHandler		= mmioFOURCC('X','V','I','D') ;
					m_aviinfo.strhdr.dwQuality		= -1;
					m_aviinfo.strhdr.dwScale		= 1;
					m_aviinfo.strhdr.dwRate			= m_filerate;
					
					hr = AVIFileCreateStream(m_aviinfo.m_hAVIFile,
						&m_aviinfo.m_hVideoStream,&m_aviinfo.strhdr);
					
					if (hr==AVIERR_OK)
					{
						BITMAPINFOHEADER bitmaphdr;		
						bitmaphdr.biSize			= sizeof(BITMAPINFOHEADER);
						bitmaphdr.biWidth			= m_width;
						bitmaphdr.biHeight			= m_height;		
						bitmaphdr.biPlanes			= 1;
						bitmaphdr.biBitCount		= 24;
						bitmaphdr.biCompression		= mmioFOURCC('X','V','I','D') ;
						bitmaphdr.biSizeImage		= bitmaphdr.biWidth*bitmaphdr.biHeight*3;
						bitmaphdr.biXPelsPerMeter	= 0;
						bitmaphdr.biYPelsPerMeter	= 0;
						bitmaphdr.biClrUsed			= 0;
						bitmaphdr.biClrImportant	= 0;
						
						AVIStreamSetFormat(m_aviinfo.m_hVideoStream, 0
							, &bitmaphdr, bitmaphdr.biSize);
					}
				}
				else
				{
					m_aviinfo.strhdr.fccType		= streamtypeVIDEO;
					m_aviinfo.strhdr.fccHandler		= mmioFOURCC('h','2','6','4');
					m_aviinfo.strhdr.dwQuality		= -1;
					m_aviinfo.strhdr.dwScale		= 1;
					m_aviinfo.strhdr.dwRate			= m_filerate;
					
					hr = AVIFileCreateStream(m_aviinfo.m_hAVIFile,
						&m_aviinfo.m_hVideoStream,&m_aviinfo.strhdr);
					
					if (hr==AVIERR_OK)
					{
						BITMAPINFOHEADER bitmaphdr;		
						bitmaphdr.biSize			= sizeof(BITMAPINFOHEADER);
						bitmaphdr.biWidth			= m_width;
						bitmaphdr.biHeight			= m_height;		
						bitmaphdr.biPlanes			= 1;
						bitmaphdr.biBitCount		= 24;
						bitmaphdr.biCompression		= mmioFOURCC('h','2','6','4');
						bitmaphdr.biSizeImage		= bitmaphdr.biWidth*bitmaphdr.biHeight*3;
						bitmaphdr.biXPelsPerMeter	= 0;
						bitmaphdr.biYPelsPerMeter	= 0;
						bitmaphdr.biClrUsed			= 0;
						bitmaphdr.biClrImportant	= 0;
						
						AVIStreamSetFormat(m_aviinfo.m_hVideoStream, 0
							, &bitmaphdr, bitmaphdr.biSize);
					}
					
				}	
			}

// 			hr = AVIStreamWrite(m_aviinfo.m_hVideoStream, 
// 				m_aviinfo.m_aviVideoWritePos, 1, pFrameInfo->pContent,
// 				pFrameInfo->nFrameLength,
// 				pFrameInfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME,
//  			NULL,NULL);		

			hr = AVIStreamWrite(m_aviinfo.m_hVideoStream, 
				m_aviinfo.m_aviVideoWritePos, 1, pFrameInfo->pContent,
				pFrameInfo->nFrameLength,
				(pFrameInfo->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME)? AVIIF_KEYFRAME:0,
 				NULL,NULL);
			
			m_aviinfo.m_aviVideoWritePos++;
			
	}//end of if (DH_FRAME_TYPE_VIDEO == dataType)
	
	
	else if (DH_FRAME_TYPE_AUDIO == pFrameInfo->nType)
	{
		if (m_aviinfo.m_foundIFrame == FALSE)
		{
			return;
		}
		
		if (m_aviinfo.m_hAudioStream == NULL)
		{
			WAVEFORMATEX WaveFormat;
			ZeroMemory (&WaveFormat, sizeof (WAVEFORMATEX));
		
			if (hr == AVIERR_OK)
			{
				WaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
				WaveFormat.nChannels		= 1;
				WaveFormat.nSamplesPerSec	= pFrameInfo->nSamplesPerSecond;
				WaveFormat.wBitsPerSample	= pFrameInfo->nBitsPerSample;
				WaveFormat.nAvgBytesPerSec	= (WaveFormat.nSamplesPerSec *WaveFormat.nChannels*WaveFormat.wBitsPerSample  / 8);
				WaveFormat.nBlockAlign		= WaveFormat.nChannels * WaveFormat.wBitsPerSample  / 8;
			
				m_aviinfo.strhdr.fccType		= streamtypeAUDIO;
				m_aviinfo.strhdr.fccHandler		= WAVE_FORMAT_PCM;
				m_aviinfo.strhdr.dwQuality		= -1;
				m_aviinfo.strhdr.dwScale		= WaveFormat.nBlockAlign;
				m_aviinfo.strhdr.dwRate			= WaveFormat.nAvgBytesPerSec;
				m_aviinfo.strhdr.dwSampleSize	= WaveFormat.nBlockAlign;
			
				hr = AVIFileCreateStream(m_aviinfo.m_hAVIFile,
					&m_aviinfo.m_hAudioStream,&m_aviinfo.strhdr);
				if (hr==AVIERR_OK)
				{
					hr = AVIStreamSetFormat (m_aviinfo.m_hAudioStream, 0, &WaveFormat, sizeof (WAVEFORMATEX));
					if(hr!=AVIERR_OK)
					{
						AVIStreamClose(m_aviinfo.m_hAudioStream);
						m_aviinfo.m_hAudioStream = 0;
					}
				}
			}
		}//end of 	if (m_aviinfo.m_hAudioStream == NULL)

		hr = AVIStreamWrite(m_aviinfo.m_hAudioStream, m_aviinfo.m_aviAudioWritePos, 
			1, pBuf, nSize,0,NULL,NULL);
		
		m_aviinfo.m_aviAudioWritePos++;
	}//end of else if (DH_FRAME_TYPE_AUDIO == dataType)
}

void DhPlayGraph::ChangeNewAviFile(FILE_CHANGE_ENUM nType)
{
	if ( nType == ENUM_FILE_ENC && m_fileEncChangeCount <= 1)
	{
		return;
	}

	if ( nType == ENUM_FILE_RATE && m_fileRateChangeCount <= 1)
	{
		return;
	}

	if ( nType == ENUM_FILE_CHANGE && m_fileChangeCount <= 1 )
	{
		return;
	}
	
	if ( nType == ENUM_FILE_RATE_AND_FIX && m_fileChangeCount <= 1 && m_fileRateChangeCount <= 1)
	{
		return;
	}

	StopAVIConvRealImp();	
	char FileName[MAX_PATH] = {0};
	int nRet = GetAviFileName(FileName , nType);
	if ( nRet ==  0)
	{
		ConvToAVI((unsigned char*)FileName);
	}
}

int DhPlayGraph::GetAviFileName(char* pAviFileName , FILE_CHANGE_ENUM nType)
{

	std::string aviFileName;
	aviFileName = m_aviFileName;
	
//	std::string::size_type pos = 0;
	std::string::size_type idx = 0;

	idx = aviFileName.rfind("." , MAX_PATH);
	if( idx > MAX_PATH)
	{
		return -1;
	}
	memcpy(pAviFileName , m_aviFileName , idx);

	char temp[64] = {0};
	if ( nType == ENUM_FILE_CHANGE)
	{
		sprintf( temp , "_%d.avi" , m_fileChangeCount);	
	}
	else if ( nType == ENUM_FILE_RATE)
	{
		sprintf( temp , "Rate_%d.avi", m_fileRateChangeCount);
	}
	else if ( nType == ENUM_FILE_ENC)
	{
		sprintf( temp , "Enc_%d.avi" , m_fileEncChangeCount);
	}
	else if ( nType == ENUM_FILE_RATE_AND_FIX)
	{
		sprintf(temp , "Rate_%d_Fix_%d.avi", m_fileRateChangeCount , m_fileChangeCount);
	}
	strncat( pAviFileName , temp , strlen( temp ));
	return 0;
}


/// 未做互斥
void DhPlayGraph::NotifyRendering()
{
}


static unsigned int crc32_table[256] = 
{
	0x0,0x77073096,0xee0e612c,0x990951ba,0x76dc419,0x706af48f,0xe963a535,0x9e6495a3,
	0xedb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,0x9b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
	0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
	0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
	0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
	0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
	0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
	0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
	0x76dc4190,0x1db7106,0x98d220bc,0xefd5102a,0x71b18589,0x6b6b51f,0x9fbfe4a5,0xe8b8d433,
	0x7807c9a2,0xf00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x86d3d2d,0x91646c97,0xe6635c01,
	0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
	0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
	0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
	0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
	0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
	0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
	0xedb88320,0x9abfb3b6,0x3b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x4db2615,0x73dc1683,
	0xe3630b12,0x94643b84,0xd6d6a3e,0x7a6a5aa8,0xe40ecf0b,0x9309ff9d,0xa00ae27,0x7d079eb1,
	0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
	0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
	0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
	0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
	0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
	0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
	0x9b64c2b0,0xec63f226,0x756aa39c,0x26d930a,0x9c0906a9,0xeb0e363f,0x72076785,0x5005713,
	0x95bf4a82,0xe2b87a14,0x7bb12bae,0xcb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0xbdbdf21,
	0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
	0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
	0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
	0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
	0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
	0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d
};

unsigned int crc32(unsigned char* p, unsigned int len)
{
	unsigned int i;
	unsigned int crc = 0xFFFFFFFF;

	for(i=0;i<len;i++)
		crc = crc32_table[(crc^p[i])&0xff] ^ (crc>>8);

	return ~crc;
}
