//////////////////////////////////////////////////////////////////////
#if !defined _SHSTREAMFILEOPERATION_H
#define _SHSTREAMFILEOPERATION_H

#include "fileoprinterface.h"

class ShStreamFileOpr  : public IFileOperator
{
public:
	ShStreamFileOpr();
	virtual ~ShStreamFileOpr();

	int   CreateIndex(char* fileName, DWORD beginpos, std::vector<DH_INDEX_INFO*>& m_pFileIndex, 
		DWORD& m_totalFrameNum, DHTIME& begintime, DHTIME& endtime,__int64& nDataLen)  ;//建立索引，把索引信息放在m_pFileIndex里

	__int64 GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, __int64 KeyFramePos, 
		DWORD nFrame); //得到指定帧的结束位置

	void GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info);
};

#endif 