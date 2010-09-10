// StandardStreamFirOpr.cpp: implementation of the StandardStreamFirOpr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StandardStreamFirOpr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StandardStreamFirOpr::StandardStreamFirOpr()
{

}

StandardStreamFirOpr::~StandardStreamFirOpr()
{

}

int   StandardStreamFirOpr::CreateIndex(char* filename, DWORD beginpos, 
									std::vector<DH_INDEX_INFO*>& m_pFileIndex, 
									DWORD& m_nTotalFrameNum, DHTIME& m_begintime, 
									DHTIME& m_endtime,
									__int64& nDataLen) //建立索引，把索引信息放在m_pFileIndex里
{
	return 1;
}


__int64 StandardStreamFirOpr::GetFrameEndPos(HANDLE m_pFile, DWORD KeyFrameNum, 
											 __int64 KeyFramePos, DWORD nFrame)  //得到指定帧的结束位置
{
	return -1 ;
}

void StandardStreamFirOpr::GetFileInfo(HANDLE m_pFile, DWORD& dwFrame, DH_VIDEOFILE_HDR& _video_info)
{

}