// StandardStreamFirOpr.h: interface for the StandardStreamFirOpr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STANDARDSTREAMFIROPR_H__1B82AC47_3963_44CD_BC5D_0D60AC1ACBBF__INCLUDED_)
#define AFX_STANDARDSTREAMFIROPR_H__1B82AC47_3963_44CD_BC5D_0D60AC1ACBBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fileoprinterface.h"

class StandardStreamFirOpr : public IFileOperator
{
public:
	StandardStreamFirOpr();
	virtual ~StandardStreamFirOpr();

	int   CreateIndex(char* fileName, DWORD beginpos, std::vector<DH_INDEX_INFO*>& m_pFileIndex, 
		DWORD& m_totalFrameNum, DHTIME& begintime, DHTIME& endtime,__int64& nDataLen)  ;//建立索引，把索引信息放在m_pFileIndex里

	__int64 GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, __int64 KeyFramePos, 
		DWORD nFrame); //得到指定帧的结束位置

	void GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info);
};

#endif // !defined(AFX_STANDARDSTREAMFIROPR_H__1B82AC47_3963_44CD_BC5D_0D60AC1ACBBF__INCLUDED_)
