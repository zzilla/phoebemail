// DHSTDStreamFileOpr.h: interface for the DHSTDStreamFileOpr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DHSTDSTREAMFILEOPR_H__11F1F571_08FA_4A59_8239_332DC430A6F8__INCLUDED_)
#define AFX_DHSTDSTREAMFILEOPR_H__11F1F571_08FA_4A59_8239_332DC430A6F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "fileoprinterface.h"

class DHSTDStreamFileOpr  : public IFileOperator
{
public:
	DHSTDStreamFileOpr();
	virtual ~DHSTDStreamFileOpr();

	int   CreateIndex(char* fileName, DWORD beginpos, std::vector<DH_INDEX_INFO*>& m_pFileIndex, 
		DWORD& m_totalFrameNum, DHTIME& begintime, DHTIME& endtime,__int64& nDataLen)  ;//建立索引，把索引信息放在m_pFileIndex里

	__int64 GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, __int64 KeyFramePos, 
		DWORD nFrame); //得到指定帧的结束位置

	void GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info);
};

#endif // !defined(AFX_DHSTDSTREAMFILEOPR_H__11F1F571_08FA_4A59_8239_332DC430A6F8__INCLUDED_)
