    /*
**  ************************************************************************
**                                     DHPlay  通用播放SDK
**         (c) Copyright 1992-2004, ZheJiang Dahua Technology Stock Co.Ltd.
**                            All Rights Reserved
**
**	File  Name	: data.cpp
**	Description	: 播放sdk播放数据控制程序
**	Modification	: 2005/12/09		chenmy		Create the file
**  ************************************************************************
*/
#include "StdAfx.h"
#include "stdio.h"
#include "data.h"
#include "playmanage.h"
#include "NewStreamFileOpr.h"
#include "oldStreamfileOpr.h"
#include "ShStreamFileOpr.h"
#include "StandardStreamFirOpr.h"
#include "DHSTDStreamFileOpr.h"
//#define INDEXCREATEBACK
/*	$Function		:	CDHAVData
==  ===============================================================
==	Description		:	构造函数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
static const BYTE I_Frame[4] = {0x00,0x00,0x01,0xFD} ;
static const BYTE Audio_Frame[4] = {0x00,0x00,0x01,0xF0} ;
static const BYTE B_Frame[4] = {0x00,0x00,0x01,0xFB} ;
static const BYTE P_Frame[4] = {0x00,0x00,0x01,0xFC} ;
static const BYTE A_Frame[4] = {0x00,0x00,0x01,0xFA} ;
static const BYTE LB_Frame[4] = {0x44,0x48,0x50,0x54} ;//DHPT
static const BYTE SH_Frame[4] = {0x00,0x00,0x01,0xF2};
static const BYTE RW_I_Frame[4] = {0x00,0x00,0x01,0x00};

CDHAVData::CDHAVData()
{
	m_nOpenMode = FM_NOOPEN;  //文件打开模式
	m_pCurPos = 0;    //当前指针位置
	memset((BYTE *)&m_sfile_hdr, 0, sizeof(DH_VIDEOFILE_HDR)) ;   //文件信息结构（具体内容待定）
	m_nTotalFrameNum = 0 ;//总桢数
	m_nFileTime = 0 ;
	m_pFile    = NULL ;
	m_nDataLen = 0;
	m_pStreamBuf = NULL;//文件流缓冲指针
	m_nBufLen = 0;    //数据流缓冲大小
	m_pWrite = NULL;     //数据流写指针
	m_pRead = NULL;      //数据流读指针
	m_EndPos = 0 ;
	memset(filename,0,_MAX_PATH+_MAX_FNAME) ;

	m_streamtype = Stream_No ;
	m_newstreamfileopr = new NewStreamFileOpr ;
	m_oldstreamfileopr = new OldStreamOperation ;
	m_shstreamfileopr = new ShStreamFileOpr ;
	m_standardfileopr = new StandardStreamFirOpr ;
	m_dhstdstreamfileopr = new DHSTDStreamFileOpr;
	m_fileOpr = m_newstreamfileopr ;

	m_hCreateIndex = NULL ;
	m_port = 0;
	m_bIsIndexCreated = false;
}

/*	$Function		:	～CDHAVData
==  ===============================================================
==	Description		:	析构函数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
CDHAVData::~CDHAVData()
{	
	if (m_pStreamBuf != NULL)
	{
		delete[] m_pStreamBuf ;
		m_pStreamBuf = NULL;
	}

	if (m_newstreamfileopr)
	{
		delete m_newstreamfileopr  ;
		m_newstreamfileopr = NULL ;
	}

	if (m_oldstreamfileopr)
	{
		delete m_oldstreamfileopr ;
		m_oldstreamfileopr = NULL ;
	}
	
	if (m_shstreamfileopr)
	{
		delete m_shstreamfileopr ;
		m_shstreamfileopr = NULL ;
	}

	if (m_standardfileopr)
	{
		delete m_standardfileopr ;
		m_standardfileopr = NULL ;
	}

	if (m_dhstdstreamfileopr)
	{
		delete m_dhstdstreamfileopr;
		m_dhstdstreamfileopr = NULL;
	}

	if (m_hCreateIndex)
	{
		CloseHandle(m_hCreateIndex);
		m_hCreateIndex = NULL;
	}

	ResetIndex() ;//复位索引
}

DWORD WINAPI ThreadProcCreateIndex(LPVOID pParam)
{
	DWORD dwRet = 1;

	CDHAVData* dhdata = (CDHAVData*)pParam ;

	if (g_cDHPlayManage.pDHFile[dhdata->m_port]->CreateIndex() != 0)
	{
		//建立索引失败
		g_cDHPlayManage.pDHFile[dhdata->m_port]->ResetIndex() ;
		dhdata->m_bIsIndexCreated = FALSE;
	}
	else
	{
		dwRet = 0;
		dhdata->m_bIsIndexCreated = TRUE;

		if (g_cDHPlayManage.pCallback[dhdata->m_port]
			&&g_cDHPlayManage.pCallback[dhdata->m_port]->GetCBStatus(CBT_FILEREF) )
		{
#ifdef _DEBUG
			char str[100];
			sprintf(str, "Index Created %d", dhdata->m_port);
			OutputDebugString(str);
#endif
			
			g_cDHPlayManage.pCallback[dhdata->m_port]->ProcessFileRef() ;
		}
	}

	if (g_cDHPlayManage.pCallback[dhdata->m_port]
		&&g_cDHPlayManage.pCallback[dhdata->m_port]->GetCBStatus(CBT_FILEREFEX) )
	{
		g_cDHPlayManage.pCallback[dhdata->m_port]->ProcessFileRefEx(dhdata->m_bIsIndexCreated) ;
	}

	return dwRet;
}

/*	$Function		:	Open
==  ===============================================================
==	Description		:	文件打开函数，打开一个文件
==	Argument		:	输入参数：
==				:			strName   文件名
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
FILE_OPEN_ERROR CDHAVData::Open(LONG nPort, LPSTR strName)    
{
	//判断传入的文件名是否为空
	if (strName == NULL)
	{
		return FILE_CANNOT_OPEN ;
	}
	
	if (strlen(strName) >= _MAX_FNAME+_MAX_PATH)
	{
		return FILE_CANNOT_OPEN ;
	}

	strcpy(filename,strName) ;

	//打开文件
	m_pFile = CreateFile(strName, GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL) ;

	if (m_pFile == NULL)
	{
		return FILE_CANNOT_OPEN ;
	}	
	
	//得到文件长度	
	m_nDataLen = GetFileSize(m_pFile, NULL) ;

	if (m_nDataLen <= 1024 || m_nDataLen>=MAX_FILE_LENGTH)
	{
		m_nDataLen = 0 ;
		CloseHandle(m_pFile) ;
		m_pFile = NULL ;
		return FILE_CANNOT_OPEN ;
	}

	m_nOpenMode = FM_FILE;
	m_port = nPort;
	//如果文件索引已建立，调用了SetIndexInfo
	if (IsIndexCreated())
	{
		if (m_streamtype == Stream_NewStream)
		{
			m_fileOpr = m_newstreamfileopr ;
		}
		else if (m_streamtype == Stream_OldStream)
		{
			m_fileOpr = m_oldstreamfileopr ;
		}
		else if (m_streamtype == Stream_ShStream)
		{
			m_fileOpr = m_shstreamfileopr ;
		}
		else if (m_streamtype == Stream_Standard)
		{
			m_fileOpr = m_standardfileopr ;
		}
		else if (m_streamtype == Stream_DHSTD)
		{
			m_fileOpr = m_dhstdstreamfileopr;
		}

		// 计算两次，在澳大利亚时区下第一次计算mktime会出错
		m_nFileTime = GetTimeInterVal(m_sfile_hdr.DirEndTime , m_sfile_hdr.DirStartTime);
		//m_nFileTime = GetTimeInterVal(m_sfile_hdr.DirEndTime , m_sfile_hdr.DirStartTime);

		m_pCurPos = m_sfile_hdr.StreamPos;
		myFileSeek(m_pFile,m_pCurPos,FILE_BEGIN);

		if (g_cDHPlayManage.pCallback[nPort]
	    	&&g_cDHPlayManage.pCallback[nPort]->GetCBStatus(CBT_FILEREF) )
		{
			g_cDHPlayManage.pCallback[nPort]->ProcessFileRef() ;
		}

		if (g_cDHPlayManage.pCallback[nPort]
			&&g_cDHPlayManage.pCallback[nPort]->GetCBStatus(CBT_FILEREFEX) )
		{
			g_cDHPlayManage.pCallback[nPort]->ProcessFileRefEx(TRUE) ;
		}

		return FILE_NO_ERROR ;
	}

	memset(&m_sfile_hdr,0,sizeof(m_sfile_hdr)) ;	
	
	BYTE buf[133] ;
	memset(buf,0,133) ;
	DWORD len = 0 ;
	
	myFileSeek (m_pFile, 0, FILE_BEGIN) ;
	ReadFile(m_pFile, buf, 132, &len, NULL) ;

	//如果有文件头
	if (memcmp(buf+sizeof(m_sfile_hdr), I_Frame, 4) == 0
		|| memcmp(buf+sizeof(m_sfile_hdr), Audio_Frame, 4) == 0
		|| memcmp(buf+sizeof(m_sfile_hdr), B_Frame,4) == 0 
		|| memcmp(buf+sizeof(m_sfile_hdr),A_Frame,4) == 0
		|| memcmp(buf+sizeof(m_sfile_hdr), P_Frame,4) == 0)
	{
		m_sfile_hdr.StreamPos = sizeof(m_sfile_hdr) ;
		m_streamtype = Stream_NewStream ;
	}
	else//寻找帧头标志	
	{		
		DWORD rest = 0 ;
		len = 1048576*2  ;//2M
		BYTE* pBuf = new BYTE[len+1] ;

		while (pBuf == NULL)
		{
			if (len < 4096)
			{
				return FILE_CANNOT_OPEN ;
			}

			len = len / 2 ;
			pBuf = new BYTE[len+1] ;			
		}

		myFileSeek (m_pFile, 0, FILE_BEGIN) ;
		ReadFile(m_pFile, pBuf, len, &rest, NULL) ;

		int audio_frame_pos = -1 ;
		int avifile_pos = -1;

		unsigned int code = 0xFFFFFFFF;
		unsigned char *pos = pBuf;	// 拿到数据区首指针
		int standardstrpos = -1 ;

		while(rest-- )
		{
			code = (code << 8) | *pos++;

			if (code == 0x44485054) //找到"DHPT"(44485054)，视频帧
			{
// 				m_streamtype = Stream_OldStream ;
// 				m_sfile_hdr.StreamPos = pos - pBuf - 4;
// 				break ;
			}
			else if (code == 0x3026B275)
			{
				m_streamtype = Stream_Standard;
				m_sfile_hdr.StreamPos = pos - pBuf - 4;
				break ;
			}
			else if (code == 0x44484156)
			{
				m_streamtype = Stream_DHSTD;
				m_sfile_hdr.StreamPos = pos - pBuf - 4;
				break;
			}
			else if (code == 0x01FD || code == 0x01FB || code == 0x01FC || code == 0x01FA)
			{
				m_streamtype = Stream_NewStream ;
				m_sfile_hdr.StreamPos = pos - pBuf  - 4;
				break ;
			}
			else if (code == 0x01F2)
			{
				m_streamtype = Stream_ShStream ;
				m_sfile_hdr.StreamPos = pos - pBuf  - 4;
				break ;
			}
			else if (code == 0x01F0)
			{
				if ( audio_frame_pos == -1)
				{
					audio_frame_pos = pos - pBuf - 4 ;
				}
			}
			else if (code == 0x52494646)//RIFF
			{
				if (avifile_pos == -1)
				{
					avifile_pos = pos - pBuf - 4;
				}
			}
			else if (code == 0x01B6)
			{
				if (standardstrpos == -1)
				{
					standardstrpos = pos - pBuf - 4 ;
					if (standardstrpos > 45)
					{
						standardstrpos -= 45 ;
					}
					else 
					{
						standardstrpos = 0 ;
					}
				}
			}
		}//end of while(rest-- )

		if (audio_frame_pos != -1)
		{
			m_sfile_hdr.StreamPos = audio_frame_pos ;
		}
	
		if (m_streamtype == Stream_No && standardstrpos > -1)
		{
			m_streamtype = Stream_Standard ;

			if (audio_frame_pos == -1)
			{
				m_sfile_hdr.StreamPos = standardstrpos ;
			}
			else
			{
				m_sfile_hdr.StreamPos = (audio_frame_pos < standardstrpos) ? audio_frame_pos : standardstrpos;
			}

			if (avifile_pos != -1 )
			{
				m_sfile_hdr.StreamPos = avifile_pos;
			}

			m_sfile_hdr.StreamPos = 0;
		}

		delete[] pBuf ;
		pBuf = NULL ;
	}

	//默认文件opr是新码流
	if (m_streamtype == Stream_NewStream)
	{
		m_fileOpr = m_newstreamfileopr ;
	}
	else if (m_streamtype == Stream_OldStream)
	{
		m_fileOpr = m_oldstreamfileopr ;
	}
	else if (m_streamtype == Stream_ShStream)
	{
		m_fileOpr = m_shstreamfileopr ;
	}
	else if (m_streamtype == Stream_Standard)
	{
		m_fileOpr = m_standardfileopr ;
	}
	else if (m_streamtype == Stream_DHSTD)
	{
		m_fileOpr = m_dhstdstreamfileopr;
	}
	
	m_fileOpr->GetFileInfo(m_pFile,m_nTotalFrameNum, m_sfile_hdr);
	m_nFileTime = GetTimeInterVal(m_sfile_hdr.DirEndTime , m_sfile_hdr.DirStartTime);

	DWORD dwThreadId ;
	m_hCreateIndex = CreateThread( 
					NULL,                        // no security attributes 
					0,                           // use default stack size  
					ThreadProcCreateIndex,                  // thread function 
					this,                // argument to thread function 
					0,                           // use default creation flags 
					&dwThreadId); 	
	

	m_pCurPos = m_sfile_hdr.StreamPos ;

	myFileSeek(m_pFile,m_pCurPos,FILE_BEGIN) ;
	
	return FILE_NO_ERROR;
}
/*	$Function		:	ReOpen
==  ===============================================================
==	Description		:	重新播放文件，暂时只作重置文件开始位置的处理
==	Argument		:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::ReOpen()
{	
	//重置文件开始位置
	m_pCurPos = m_sfile_hdr.StreamPos;		
	return true ;	
}
/*	$Function		:	IsIndexCreated
==  ===============================================================
==	Description		:	判断文索引是否建立
==	Argument		:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDHAVData::IsIndexCreated()
{
	return m_bIsIndexCreated;
}

/*	$Function		:	ResetIndex
==  ===============================================================
==	Description		:	清空文件索引信息
==	Argument		:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDHAVData::ResetIndex()
{
	for (int i = 0 ; i < m_pFileIndex.size() ; i++)
	{
		DH_INDEX_INFO* m_index = m_pFileIndex[i] ;
		delete m_index ;
		m_index = NULL ;
	}

	m_pFileIndex.clear() ;

// 	m_pCurPos = m_sfile_hdr.StreamPos ;
	m_nTotalFrameNum = 0 ;
	m_bIsIndexCreated = false;

	return TRUE ;
}

/*	$Function		:	CreateIndex_newstream
==  ===============================================================
==	Description		:	建立文件索引函数，建立文件索引，在播放线程中调用，
							不影响程序执行时间，当索引建立后直接返回
==	Argument		:	
==				:			
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
int CDHAVData::CreateIndex()   
{
	int iRet = m_fileOpr->CreateIndex(filename, m_sfile_hdr.StreamPos, m_pFileIndex, m_nTotalFrameNum,
		m_sfile_hdr.DirStartTime, m_sfile_hdr.DirEndTime, m_nDataLen) ;

	if (iRet != 0)
	{
		ResetIndex() ;
	}
	else
	{	
		m_nFileTime = GetTimeInterVal(m_sfile_hdr.DirEndTime , m_sfile_hdr.DirStartTime);
	}

	return iRet ;
}

/*	$Function		:	GetPos
==  ===============================================================
==	Description		:	获取文件播放进度函数，由当前文件指针位置和文件总长度，得到播放进度
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
float CDHAVData::GetPos()
{
	if (m_nDataLen == 0)
	{
		return 0 ;
	}

	return (m_pCurPos-m_sfile_hdr.StreamPos) / (m_nDataLen - m_sfile_hdr.StreamPos + 0.0) ;
}

BOOL CDHAVData::SetPos(float fRelativePos)
{
	m_pCurPos = (m_nDataLen - m_sfile_hdr.StreamPos)* fRelativePos + m_sfile_hdr.StreamPos ;
	return TRUE ;
}

BOOL CDHAVData::GetDataPos(float fRelativePos, DWORD *pCurDataPos)
{
	if (NULL == pCurDataPos) return FALSE;

	*pCurDataPos = (m_nDataLen - m_sfile_hdr.StreamPos) * fRelativePos + m_sfile_hdr.StreamPos ;
	return TRUE ;
}

BOOL CDHAVData::GetRelativePos(DWORD dCurDataPos, float *pfRelativePos)
{
	if (m_nDataLen == 0 || (dCurDataPos <= 0))
	{
		return FALSE;
	}

	float fRelatVal = (dCurDataPos / (m_nDataLen - m_sfile_hdr.StreamPos + 0.0));

	*pfRelativePos = (fRelatVal > 0.999)? 1.0 : fRelatVal;
	
	return TRUE;
}

/*	$Function		:	Close
==  ===============================================================
==	Description		:	文件关闭函数，关闭一个打开的文件
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
int CDHAVData::Close()   
{
	if (m_nOpenMode != FM_FILE)
	{
		return 0 ;
	}

	if (m_pFile != NULL)
	{
		CloseHandle(m_pFile) ;
		m_pFile = NULL ;
	}

	m_fileOpr->EndCreateIndex() ;

	DWORD dwRes = WaitForSingleObject(m_hCreateIndex, INFINITE);
	if (WAIT_OBJECT_0 == dwRes)
	{
		CloseHandle(m_hCreateIndex);
		m_hCreateIndex = NULL;
	}

	ResetIndex() ;
	
	return 1;
}

/*	$Function		:	GetTotalFrames
==  ===============================================================
==	Description		:	获得文件桢数函数，得到文件总桢数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
DWORD CDHAVData::GetTotalFrames()
{
	return m_nTotalFrameNum ;
}


/*	$Function		:	GetKeyFramePos
==  ===============================================================
==	Description		:	获得关键桢函数，查找指定位置之前的关键桢位置
==	Argument		:	输入参数：
==				:			nValue   指定的位置，可以是时间或桢号，类型由nType指定
							nType 指定nValue的类型。如果nType 是BY_FRAMENUM则nValue表示帧号，如果nType 是BY_FRAMTIME，则nValue表示时间，单位ms。 
						输出参数：
						    pFramePos 一结构体指针，内含关键桢（I桢）的位置，桢序号，时间信息
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/        
BOOL CDHAVData::GetKeyFramePos(DWORD nValue, DWORD nType, PFRAME_POS pFramePos, int* KeyFrameRate)
{
	if (m_nOpenMode != FM_FILE || !IsIndexCreated())//非文件模式
	{
		return FALSE ;
	}

	int IFrameIdx = 0;

	DWORD nFrame = -1;
	
	if (nType == BY_FRAMENUM)
	{
		nFrame = nValue ;
		if (nFrame > m_nTotalFrameNum)
		{
			return false ;
		}

		for (IFrameIdx = 0 ; IFrameIdx < m_pFileIndex.size()-1 ; IFrameIdx++)
		{
			if (nFrame >= m_pFileIndex[IFrameIdx]->IFrameNum && nFrame < m_pFileIndex[IFrameIdx + 1]->IFrameNum)
			{
				break ;
			}
		}

		if (IFrameIdx < 0 || IFrameIdx > m_pFileIndex.size()-1)
		{
			return false ;
		}
		
		if (KeyFrameRate != NULL)
		{
			*KeyFrameRate = m_pFileIndex[IFrameIdx]->IFrameRate ;
		}
		pFramePos->nFilePos = m_pFileIndex[IFrameIdx]->IFramePos;
		pFramePos->nFrameLen = m_pFileIndex[IFrameIdx]->framelen;
		pFramePos->nFrameNum = m_pFileIndex[IFrameIdx]->IFrameNum;
		pFramePos->nFrameTime = GetTimeInterVal(m_pFileIndex[IFrameIdx]->time, m_sfile_hdr.DirStartTime) * 1000;

	}
	else if (nType == BY_FRAMETIME)
	{
		for (IFrameIdx = 0 ; IFrameIdx < m_pFileIndex.size()-1 ; IFrameIdx++)
		{
			if ((nValue >= 1000*GetTimeInterVal(m_pFileIndex[IFrameIdx]->time, m_sfile_hdr.DirStartTime))
				&& (nValue < 1000*GetTimeInterVal(m_pFileIndex[IFrameIdx + 1]->time, m_sfile_hdr.DirStartTime)))
			{
				break ;
			}
		}
		
		if (IFrameIdx < 0 || IFrameIdx > m_pFileIndex.size()-1)
		{
			return false ;
		}

		if (KeyFrameRate != NULL)
		{
			*KeyFrameRate = m_pFileIndex[IFrameIdx]->IFrameRate ;
		}

		pFramePos->nFilePos = m_pFileIndex[IFrameIdx]->IFramePos;
		pFramePos->nFrameLen = m_pFileIndex[IFrameIdx]->framelen;
		pFramePos->nFrameNum = m_pFileIndex[IFrameIdx]->IFrameNum;
		pFramePos->nFrameTime = GetTimeInterVal(m_pFileIndex[IFrameIdx]->time, m_sfile_hdr.DirStartTime) * 1000;

	}

	return TRUE;
}

    
/*	$Function		:	GetNextKeyFramePos
==  ===============================================================
==	Description		:	获得关键桢函数，查找指定位置之后的关键桢位置
==	Argument		:	输入参数：
==				:			nValue   指定的位置，可以是时间或桢号，类型由nType指定
							nType 指定nValue的类型。如果nType 是BY_FRAMENUM则nValue表示帧号，如果nType 是BY_FRAMTIME，则nValue表示时间，单位ms。 
						输出参数：
						    pFramePos 一结构体指针，内含关键桢（I桢）的位置，桢序号，时间信息
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/         
BOOL CDHAVData::GetNextKeyFramePos(DWORD nValue, DWORD nType, PFRAME_POS pFramePos)
{
	if (m_nOpenMode != FM_FILE || !IsIndexCreated())//非文件模式
	{
		return FALSE ;
	}

	int IFrameIdx = 0;

	DWORD nFrame = -1;
	
	if (nType == BY_FRAMENUM)
	{
		nFrame = nValue ;
		if (nFrame > m_nTotalFrameNum)
		{
			return false ;
		}

		for (IFrameIdx = 0 ; IFrameIdx < m_pFileIndex.size()-1 ; IFrameIdx++)
		{
			if (nFrame >= m_pFileIndex[IFrameIdx]->IFrameNum && nFrame < m_pFileIndex[IFrameIdx + 1]->IFrameNum)
			{	
				break ;
			}
		}

		IFrameIdx++ ;

		if (IFrameIdx > m_pFileIndex.size()-1)
		{
			IFrameIdx =  m_pFileIndex.size()-1;
		}

		pFramePos->nFilePos = m_pFileIndex[IFrameIdx]->IFramePos;
		pFramePos->nFrameLen = m_pFileIndex[IFrameIdx]->framelen;
		pFramePos->nFrameNum = m_pFileIndex[IFrameIdx]->IFrameNum;
		pFramePos->nFrameTime = GetTimeInterVal(m_pFileIndex[IFrameIdx]->time, m_sfile_hdr.DirStartTime) * 1000;

	}
	else if (nType == BY_FRAMETIME)
	{
		for (IFrameIdx = 0 ; IFrameIdx < m_pFileIndex.size()-1 ; IFrameIdx++)
		{
			if ((nValue >= 1000*GetTimeInterVal(m_pFileIndex[IFrameIdx]->time, m_sfile_hdr.DirStartTime)) 
				&& (nValue < 1000*GetTimeInterVal(m_pFileIndex[IFrameIdx+1]->time, m_sfile_hdr.DirStartTime)))
			{
				break ;
			}
		}
		
		IFrameIdx++ ;
		
		if (IFrameIdx > m_pFileIndex.size()-1)
		{
			IFrameIdx =  m_pFileIndex.size()-1;
		}

		if (IFrameIdx < 0 || IFrameIdx > m_pFileIndex.size()-1)
		{
			return false ;
		}

		pFramePos->nFilePos = m_pFileIndex[IFrameIdx]->IFramePos;
		pFramePos->nFrameLen = m_pFileIndex[IFrameIdx]->framelen;
		pFramePos->nFrameNum = m_pFileIndex[IFrameIdx]->IFrameNum;
		pFramePos->nFrameTime = GetTimeInterVal(m_pFileIndex[IFrameIdx]->time, m_sfile_hdr.DirStartTime) * 1000;
	}

	return TRUE;
}

/*	$Function		:	GetPlayedTime
==  ===============================================================
==	Description		:	得到当前播放的时间
==	Argument		:	输入参数：
							frameNum 桢序号
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/ 
int CDHAVData::GetPlayedTime(DWORD frameNum,bool ByFrameNum)
{
	int i = 0;

	if (ByFrameNum == false)
	{
		DWORD tmp_pos = m_pCurPos ;
		for (i = 0 ; i < m_pFileIndex.size()-1 ; ++i)
		{
			if ((m_pFileIndex[i]->IFramePos <= tmp_pos) && (m_pFileIndex[i+1]->IFramePos > tmp_pos))
			{
				break ;
			}
		}
			
		if (i == m_pFileIndex.size()-1)	
		{
			return GetTimeInterVal(m_sfile_hdr.DirEndTime,m_sfile_hdr.DirStartTime)*1000;
		}
		else
		{
			return GetTimeInterVal(m_pFileIndex[i]->time,m_sfile_hdr.DirStartTime)*1000;
		}
	}
	//得到指定帧前一I帧
	for (i = 0 ; i < m_pFileIndex.size()-1 ; i++)
	{
		if ((m_pFileIndex[i + 1]->IFrameNum == frameNum))
		{
			if ((m_pFileIndex[i + 1]->IFrameNum - m_pFileIndex[i]->IFrameNum) == 1)
			{
				i++;
			}
			break;
		}
		if ((m_pFileIndex[i]->IFrameNum <= frameNum) && (m_pFileIndex[i + 1]->IFrameNum > frameNum))
		{
			break ;
		}
	}

	//最后的帧正好为I帧
	if ((i == m_pFileIndex.size()-1) && (m_pFileIndex[m_pFileIndex.size()-1]->IFrameNum == frameNum))
	{
		return GetTimeInterVal(m_sfile_hdr.DirEndTime,m_sfile_hdr.DirStartTime)*1000;
	}

	//码流最后为P帧的情况下，使用以下按比例计算时间的方法定位最后的I帧之后的P帧
	if ((frameNum > m_pFileIndex[m_pFileIndex.size()-1]->IFrameNum) && (frameNum <= m_nTotalFrameNum-1))
	{
		float fTimeRate = (float)(frameNum - m_pFileIndex[m_pFileIndex.size()-1]->IFrameNum)/
			(float)(m_nTotalFrameNum - 1 - m_pFileIndex[m_pFileIndex.size()-1]->IFrameNum);

		return (1000*((float)GetTimeInterVal(m_sfile_hdr.DirEndTime, m_pFileIndex[m_pFileIndex.size()-1]->time)*fTimeRate)
			+ 1000*GetTimeInterVal(m_pFileIndex[m_pFileIndex.size()-1]->time,m_sfile_hdr.DirStartTime));
	}
	
	//指定帧的时间为前一I帧的时间加上一估计值
	//修改为浮点运算，并将单位简单处理为ms，即s*1000
	return (GetTimeInterVal(m_pFileIndex[i]->time,m_sfile_hdr.DirStartTime) +
		    (float)GetTimeInterVal(m_pFileIndex[i + 1]->time,m_pFileIndex[i]->time)
		    * (frameNum - m_pFileIndex[i]->IFrameNum) 
 		    / (float)(m_pFileIndex[i + 1]->IFrameNum - m_pFileIndex[i]->IFrameNum))*1000;
}



/*	$Function		:	SetCurrentFrameNum
==  ===============================================================
==	Description		:	当前播放桢数设置函数，设置当前播放桢数，
						设置当前播放位置到某I桢位置，返回设置误差偏移
==	Argument		:	输入参数：
==				:			nFrameNum  指定的播放桢数
== 	Return		:		设置的桢数和文件指针所在的I桢桢数误差偏移
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
DWORD CDHAVData::SetCurrentFrameNum(DWORD nFrameNum)
{
	int index;

	if (m_nOpenMode != FM_FILE || !IsIndexCreated())//非文件模式
	{
		return - 1 ;
	}

	if (nFrameNum >= m_nTotalFrameNum)
	{
		return -1;
	}
	
	for(index = 0; index < m_pFileIndex.size()-1; index ++)
	{
		int a = m_pFileIndex[index]->IFrameNum;
		int b = m_pFileIndex[index + 1]->IFrameNum;
		if(nFrameNum >= m_pFileIndex[index]->IFrameNum && nFrameNum < m_pFileIndex[index + 1]->IFrameNum)
		{			
			//把文件指针置为该帧前一I帧的位置
			m_pCurPos = m_pFileIndex[index]->IFramePos;
			//返回该I帧的帧序号
			return m_pFileIndex[index]->IFrameNum;
		}
	}

	if (nFrameNum >= m_pFileIndex[index]->IFrameNum)
	{			
		m_pCurPos = m_pFileIndex[index]->IFramePos;
		return m_pFileIndex[index]->IFrameNum;
	}
	
	return -1 ;
}



/*	$Function		:	SetEndPos
==  ===============================================================
==	Description		:	文件定位时设置读取的最后位置
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
BOOL CDHAVData::SetEndPos(DWORD endPos)
{
	m_EndPos = endPos ;
	return true ;
}

/*	$Function		:	ReadOneFrame
==  ===============================================================
==	Description		:	文件定位时，读取指定帧前一I帧到该帧结束的数据
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
int CDHAVData::ReadOneFrame(BYTE *pBuf)
{
	if (m_pCurPos >= m_EndPos)
	{	
		return 0 ;
	}

	if (myFileSeek (m_pFile, m_pCurPos, FILE_BEGIN)	== -1)
	{
		return 0 ;
	}

	DWORD Len = ((m_EndPos - m_pCurPos) <= MAX_CHUNKSIZE )?  (m_EndPos - m_pCurPos) : MAX_CHUNKSIZE ;
	if (Len == 0)
	{		
		return 0 ;
	}

	if (ReadFile(m_pFile, pBuf, Len, &Len, NULL) == FALSE)
	{
		return 0;
	}
	
	m_pCurPos += Len ;	

	return Len ;
}
/*	$Function		:	GetFrameEndPos
==  ===============================================================
==	Description		:	得到指定帧的结束位置
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2006/4/28		zhougf   Create
==  ===============================================================
*/
__int64 CDHAVData::GetFrameEndPos(DWORD nFrame)
{
	FRAME_POS  framePos  ;

	if (GetKeyFramePos(nFrame, BY_FRAMENUM, &framePos) == false)
	{
		return 0;
	}

	return m_fileOpr->GetFrameEndPos(m_pFile, framePos.nFrameNum ,(DWORD)framePos.nFilePos/*&0xffffffff*/, nFrame) ;
}
/////////////////////流接口////////////////////////////////////////////////////////////////////////

/*	$Function		:	OpenStream
==  ===============================================================
==	Description		:	流打开函数
==	Argument		:	输入参数：
==				:			pBuf  文件头数据
							dwSize   pBuf所指向的数据长度
							dwPoolSize  指定的数据流缓冲区大小
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::OpenStream(BYTE *pBuf, DWORD dwSize, DWORD dwPoolSize)   
{
	if (dwPoolSize < SOURCE_BUF_MIN || dwPoolSize > SOURCE_BUF_MAX)
	{
		return FALSE;
	}

	m_nOpenMode = FM_STREAM ;

	m_nBufLen = dwPoolSize;
	m_pStreamBuf = new BYTE[m_nBufLen] ;

	if (m_pStreamBuf == NULL)
	{
		m_nBufLen = 0 ;
		return FALSE ;
	}
	
	memset(m_pStreamBuf,0,m_nBufLen) ;
	m_pRead = 0;
	m_pWrite = 0;
	
	return TRUE;
}

//输入数据流
/*	$Function		:	InputData
==  ===============================================================
==	Description		:	数据流输入函数
==	Argument		:	输入数据
==				:			pBuf    所输入的数据
							nSize   输入的数据长度
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::InputData(PBYTE pBuf,DWORD nSize)  
{
	DWORD mLen;

	if (nSize > m_nBufLen || m_pStreamBuf == NULL)
	{		
#ifdef _DEBUG
	char str[200] ;
	sprintf(str,"nSize=%d, m_nBufLen=%d, m_pStreamBuf=%d, pBuf=%d",nSize,m_nBufLen,m_pStreamBuf,pBuf) ;
	OutputDebugString(str) ;
#endif
		return FALSE ;
	}

	if ((m_pWrite - m_pRead) > (m_nBufLen - nSize) )   //写数据堵塞
	{
#ifdef _DEBUG
	char str[200] ;
	sprintf(str,"写数据堵塞 m_pWrite=%d, m_pRead=%d,m_nBufLen=%d,nSize=%d\n",m_pWrite, m_pRead ,m_nBufLen, nSize) ;
	OutputDebugString(str) ;
 #endif	
		return FALSE;
	}
	
	if (m_pWrite < m_nBufLen && (m_pWrite + nSize) > m_nBufLen)
	{
		mLen = m_nBufLen - m_pWrite;
		memcpy(m_pStreamBuf + m_pWrite, pBuf, mLen);
		memcpy(m_pStreamBuf, pBuf + mLen, nSize - mLen);
	}
	else
	{
		memcpy(m_pStreamBuf + m_pWrite%m_nBufLen, pBuf, nSize);
	}
	m_pWrite += nSize;
 //	InterlockedExchangeAdd((long*)&m_pWrite,nSize) ;

	return TRUE;
}


/*	$Function		:	CloseStream
==  ===============================================================
==	Description		:	流关闭函数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::CloseStream()
{
	if (m_nOpenMode != FM_STREAM )
	{
		return FALSE;
	}	

	if (m_pStreamBuf != NULL)
	{		
		delete[] m_pStreamBuf ;
		m_pStreamBuf = NULL ;
	}

	return TRUE;
}

/*	$Function		:	GetBuffSize
==  ===============================================================
==	Description		:	获得缓冲大小函数
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
DWORD CDHAVData::GetBuffSize()
{
	if (m_nOpenMode != FM_STREAM )
	{
		return 0;
	}

	return m_nBufLen;
}

/*	$Function		:	GetBufferRemain
==  ===============================================================
==	Description		:	获得缓冲剩余缓冲的大小
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
DWORD CDHAVData::GetBufferRemain()
{
	if (m_nOpenMode != FM_STREAM )
	{
		return 0;
	}

#ifdef _DEBUG
	if (m_pWrite == m_pRead) 
	{
		char str[100];
		sprintf(str, "\nm_pWrite= %d,m_pRead = %d\n", m_pWrite,m_pRead);
		OutputDebugString(str) ;	
	}
#endif

	return (m_pWrite - m_pRead);
}
/*	$Function		:	GetBufferRemain
==  ===============================================================
==	Description		:	清除剩余数据
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2006/01/16		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::ResetRemainData()
{
	if (m_nOpenMode != FM_STREAM )
	{
		return FALSE;
	}
	 
	m_pWrite = m_pRead = 0;
	m_streamtype = Stream_No ;

	return TRUE;
}    
////////////////////////////////////公共接口////////////////////////////////////////
/*	$Function		:	Read
==  ===============================================================
==	Description		:	数据读取函数
==	Argument		:	输入参数：
==				:			pBuf  所读取数据的存储缓冲区
							dwSize  读取数据的长度
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
int CDHAVData::Read(BYTE *pBuf, DWORD dwSize) 
{
	DWORD len, mRemain;
	
	if (m_nOpenMode == FM_FILE)
	{	
		if (m_pCurPos >= m_nDataLen)
		{
			__int64 tmp_filelen = GetFileSize(m_pFile, NULL) ;

			if (m_nDataLen + 2048 < tmp_filelen)
			{
				m_nDataLen = tmp_filelen ;
			}

			if (m_pCurPos >= m_nDataLen)
			{
				return 0 ;
			}
		}

		if (myFileSeek (m_pFile, m_pCurPos, FILE_BEGIN)	== -1)
		{
			return -1 ;
		}

		if (m_nDataLen - m_pCurPos < dwSize)
		{
			dwSize = m_nDataLen - m_pCurPos ;
		}

		ReadFile(m_pFile, pBuf, dwSize, &len, NULL);
		if (len <= 0)
		{
			return 0;
		}
	
		m_pCurPos += len;		
	}//end of if (m_nOpenMode == FM_FILE)

	else if (m_nOpenMode == FM_STREAM)
	{
		if (m_pWrite <= m_pRead)
		{
			return 0 ;
		}

		DWORD nSize = ((m_pWrite - m_pRead) < dwSize) ? (m_pWrite - m_pRead) : dwSize ;

		if ((m_nBufLen - m_pRead) <= nSize)    //当读取数据在缓冲尾时
		{
			memcpy(pBuf, m_pStreamBuf + m_pRead, (m_nBufLen - m_pRead));
			mRemain = nSize - (m_nBufLen - m_pRead);
			if (mRemain > 0)
			{
				memcpy(pBuf + (m_nBufLen - m_pRead), m_pStreamBuf, mRemain);
			}
			m_pRead = mRemain;
			m_pWrite %= m_nBufLen;         //虚拟写偏移复位			
		}
		else                               //正常读
		{
			memcpy(pBuf, m_pStreamBuf + m_pRead, nSize);
			m_pRead += nSize;
		}
		len = nSize;
	}
	else
	{
		return -3 ;//未知格式
	}
	
	return len;
}

/*	$Function		:	GetIndexInfo
==  ===============================================================
==	Description		:	获得索引信息函数，还有文件开始时间结束时间和总帧数
==	Argument		:	输出参数：
==				:			pBuffer  索引信息数据的存储区
						    pSize    索引信息数据大小
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::GetIndexInfo(BYTE *pBuffer, DWORD *pSize) 
{
	if (m_nOpenMode != FM_FILE)
	{
		return FALSE;
	}

	INDEX_INFO_EXT index_info_ext ;

	if (pSize ==NULL)
	{
		return FALSE ;
	}
	
	*pSize = sizeof(DH_INDEX_INFO) * m_pFileIndex.size() + sizeof(INDEX_INFO_EXT);

	if (pBuffer == NULL)
	{
		return FALSE ;
	}

	memcpy(&index_info_ext.StartTime, &m_sfile_hdr.DirStartTime, sizeof(DHTIME)) ; 
	memcpy(&index_info_ext.EndTime, &m_sfile_hdr.DirEndTime, sizeof(DHTIME)) ;
	index_info_ext.TotalFrameNum = m_nTotalFrameNum ;
	index_info_ext.StreamPos = m_sfile_hdr.StreamPos ;
	index_info_ext.StreamdataType = m_streamtype ;
	
	memcpy(pBuffer, &index_info_ext, sizeof(INDEX_INFO_EXT)) ;
	
	int i ;	
	for (i = 0 ; i < m_pFileIndex.size() ; i++)
	{
		DH_INDEX_INFO * m_index = m_pFileIndex[i] ;
		memcpy(pBuffer + i * sizeof(DH_INDEX_INFO) + sizeof(INDEX_INFO_EXT)
			, m_index, sizeof(DH_INDEX_INFO)) ;
	}

	return TRUE;
}


/*	$Function		:	SetIndexInfo
==  ===============================================================
==	Description		:	索引信息开始结束时间总帧数设置函数，
==	Argument		:	输入参数：
==				:			pBuffer   设置的文件索引数据
							nSize     数据长度
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
BOOL CDHAVData::SetIndexInfo(BYTE *pBuffer, DWORD nSize)  
{
	if (!m_pFileIndex.empty())
	{
		return FALSE ;
	}

	if (pBuffer == NULL)
	{
		return FALSE ;
	}

	int size = (nSize - sizeof(INDEX_INFO_EXT))  / sizeof(DH_INDEX_INFO);

	if (size <= 0)
	{
		return FALSE;
	}

	m_bIsIndexCreated = TRUE;

	memset((BYTE *)&m_sfile_hdr, 0, sizeof(DH_VIDEOFILE_HDR)) ;   //文件信息结构（具体内容待定）

	INDEX_INFO_EXT index_info_ext ;
	memcpy(&index_info_ext, pBuffer, sizeof(INDEX_INFO_EXT)) ;

	//设置开始时间
	memcpy(&m_sfile_hdr.DirStartTime,&index_info_ext.StartTime,sizeof(DHTIME) ) ;
	//设置结束时间
	memcpy(&m_sfile_hdr.DirEndTime,&index_info_ext.EndTime,sizeof(DHTIME)) ;
	//设置总帧数
	memcpy(&m_nTotalFrameNum,&index_info_ext.TotalFrameNum,sizeof(DWORD)) ;
	//设置文件第一个I帧B帧或音频帧开始位置
	memcpy(&m_sfile_hdr.StreamPos, &index_info_ext.StreamPos,sizeof(DWORD)) ;
	//设置文件码流类型
	memcpy(&m_streamtype, &index_info_ext.StreamdataType, sizeof(DWORD)) ;

	for (int i = 0 ; i < size ; i++)
	{
		DH_INDEX_INFO* m_index = new DH_INDEX_INFO ;
		memcpy(m_index, pBuffer + sizeof(INDEX_INFO_EXT) + i * sizeof(DH_INDEX_INFO)
			, sizeof(DH_INDEX_INFO)) ;
		m_pFileIndex.push_back(m_index) ;
	}

	return TRUE;
} 

BOOL CDHAVData::GetMediaInfo(char* pBuf, int len)
{
	if (pBuf == NULL || len < 24)
	{
		return FALSE;
	}

	memset(pBuf, '\0', len);

	memcpy(pBuf, &m_sfile_hdr.VideoWidth, sizeof(WORD));
	memcpy(pBuf+sizeof(long), &m_sfile_hdr.VideoHigh, sizeof(WORD));
	memcpy(pBuf+sizeof(long)*2, &m_sfile_hdr.Framerate, sizeof(WORD));
	memcpy(pBuf+sizeof(long)*3, &m_sfile_hdr.AudioChannel, sizeof(WORD));
	memcpy(pBuf+sizeof(long)*4, &m_sfile_hdr.AudioBitPerSample, sizeof(WORD));
	memcpy(pBuf+sizeof(long)*5, &m_sfile_hdr.AudioSamplesPerSec, sizeof(DWORD));

	return TRUE;
}

/*	$Function		:	GetFileTotalTime
==  ===============================================================
==	Description		:	时间获取函数，取得文件播放总时间长度，暂时不考虑跨天的视频数据
==	Argument		:	
==				:	
== 	Return		:	
==	Modification	:	2005/12/09		chenmy   Create
==  ===============================================================
*/
DWORD CDHAVData::GetFileTotalTime() 
{
	if (m_nOpenMode != FM_FILE)//非文件模式
	{
		return 0 ;
	}

	return 	m_nFileTime ;
}  

BOOL  CDHAVData::SetMDRange(DWORD nValueBegin, DWORD nValueEnd, DWORD nType, RECT* MDRect)
{
	FRAME_POS frame_pos ;

	GetKeyFramePos(nValueBegin, nType, &frame_pos) ;
	m_MDFrameBegin = frame_pos.nFrameNum;

	GetNextKeyFramePos(nValueEnd, nType, &frame_pos) ;
	m_MDFrameEnd    = frame_pos.nFrameNum ;

	memcpy(&m_MDRect, MDRect, sizeof(RECT)) ;
	
	return TRUE ;
}

BOOL CDHAVData::SetMDThreShold(DWORD ThreShold) 
{
	if (ThreShold > 255)
	{
		return FALSE ;
	}

	m_ThreShold = ThreShold ;

	return TRUE ;
}

DWORD CDHAVData::GetMDPosition(DWORD Direction, DWORD nFrame, DWORD* MDValue)
{
	if (!IsIndexCreated() || m_streamtype == Stream_ShStream)
	{
		return 0 ;
	}

	HANDLE f_MD = NULL ;

	f_MD = CreateFile(filename, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL) ;

	if (f_MD == NULL)
	{
		return 0 ;
	}

	static const BYTE MDFrameHeader[4] = {0x00,0x00,0x01,0xF1} ; //动检帧头
	int temp_MDvalue ;
	BYTE MDbuf[2000] ;  //保存动检帧信息
	BYTE mData[16];   //用于读取桢头数据结构
	DWORD Len ;
	int i, j , k ;

	int X_Begin   = m_MDRect.left >> 4 ;
	int X_End     = ((m_MDRect.right - 1) >> 4) + 1 ;
	int Y_Begin   = m_MDRect.top >> 4 ;
	int Y_End     = ((m_MDRect.bottom - 1) >> 4) + 1 ;	

	if (Direction == 0)
	{
		if (nFrame > m_MDFrameEnd)
		{
			nFrame = m_MDFrameEnd ;
		}
		
		for (i = m_pFileIndex.size() - 1 ; i > 0 && 
			m_pFileIndex[i]->IFrameNum >= m_MDFrameBegin; --i )
		{
			if (m_pFileIndex[i]->IFrameNum >= nFrame)
			{
				continue ;
			}
			//跳到I帧偏移
			myFileSeek(f_MD, m_pFileIndex[i]->IFramePos, FILE_BEGIN) ;
			ReadFile(f_MD, mData, 16, &Len, NULL) ;
			if (Len < 16)
			{
				CloseHandle(f_MD) ;
				return 0 ;
			}
				
			int t_width   = mData[6] >> 1 ;
			int t_height  = mData[7] >> 1 ;

			//跳到动检帧
			myFileSeek(f_MD, m_pFileIndex[i]->IFramePos +16 + *(DWORD *)&mData[12], FILE_BEGIN) ;
			ReadFile(f_MD, mData, 16, &Len, NULL) ;
			if (Len < 16)
			{
				CloseHandle(f_MD) ;
				return 0 ;
			}
				
			if (memcmp(mData, MDFrameHeader, 4) != 0)
			{
				CloseHandle(f_MD) ;
				return 0 ;
			}
			
			ReadFile(f_MD, MDbuf, *(DWORD *)&mData[12], &Len, NULL) ;

			for (j = Y_Begin ; j < Y_End ; j++)
			{
				for (k = X_Begin ; k < X_End ; k++)
				{
					temp_MDvalue = *(BYTE*)(MDbuf + j * t_width + k) ;
						
					if (temp_MDvalue >= m_ThreShold)
					{
						*MDValue = temp_MDvalue ;
						CloseHandle(f_MD) ;
						return m_pFileIndex[i]->IFrameNum ;
					}
				}
			}//end of for (int j = Y_Begin ; j < Y_End ; j++)
		}//end of for (int i = m_pFileIndex.size() - 1 ; i > 0 ; --i )
	}//end of if (Direction == 0)
	else if (Direction > 0)
	{
		if (nFrame < m_MDFrameBegin)
		{
			nFrame = m_MDFrameBegin;
		}
		
		for (i = 0 ; i < m_pFileIndex.size() &&  m_pFileIndex[i]->IFrameNum <= m_MDFrameEnd ; ++i)
		{
			if (m_pFileIndex[i]->IFrameNum <= nFrame)
			{
				continue ;
			}
			//跳到I帧偏移
			myFileSeek(f_MD, m_pFileIndex[i]->IFramePos, FILE_BEGIN) ;
			ReadFile(f_MD, mData, 16, &Len, NULL) ;
			if (Len < 16)
			{
				CloseHandle(f_MD) ;
				return 0 ;
			}

			int t_width   = mData[6] >> 1 ;
			int t_height  = mData[7] >> 1 ;

			//跳到动检帧
			myFileSeek(f_MD, m_pFileIndex[i]->IFramePos +16 + *(DWORD *)&mData[12], FILE_BEGIN) ;
			ReadFile(f_MD, mData, 16, &Len, NULL) ;
			if (Len < 16)
			{
				CloseHandle(f_MD) ;
				return 0 ;
			}
				
			if (memcmp(mData, MDFrameHeader, 4) != 0)
			{
				CloseHandle(f_MD) ;
				return 0 ;
			}
			
			ReadFile(f_MD, MDbuf, *(DWORD *)&mData[12], &Len, NULL) ;

			for (j = Y_Begin ; j < Y_End ; j++)
			{
				for (k = X_Begin ; k < X_End ; k++)
				{
					temp_MDvalue = *(BYTE*)(MDbuf + j * t_width + k) ;
						
					if (temp_MDvalue >= m_ThreShold)
					{
						*MDValue = temp_MDvalue ;
						CloseHandle(f_MD) ;
						return m_pFileIndex[i]->IFrameNum ;
					}
				}
			}//end of for (j = Y_Begin ; j < Y_End ; j++)
		}//end of for (i = 0 ; m_pFileIndex[i]->IFrameNum <= m_MDFrameEnd &&  

	}//end of else if (Direction > 0)

	return 0 ;
}
