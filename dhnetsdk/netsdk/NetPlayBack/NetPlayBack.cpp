#include "../StdAfx.h"
#include "NetPlayBack.h"

#define BUFFERSIZE   512*1024*20
#define MAXWRITESIZE 512*1024*16
#define MINWRITESIZE 512*1024*1

bool __stdcall NetPlayBackBufferPlayFunction(bool bPlay, void *userdata)
{
    NetPlayBack_CallBack *pcbInfo = (NetPlayBack_CallBack*)userdata;
    if (pcbInfo->ReadDataPauseFunc)
    {
        pcbInfo->ReadDataPauseFunc(bPlay, pcbInfo->pUserData);
    }
    return true;
}

CNetPlayBack::CNetPlayBack(NetPlayBack_CallBack  cbCallBack)
{
    m_cbCallBack = cbCallBack;

//    m_Buffer.init(BUFFERSIZE, MAXWRITESIZE, MINWRITESIZE, 
//       NetPlayBackBufferPlayFunction, &m_cbCallBack);
}

CNetPlayBack::~CNetPlayBack()
{
}

bool  CNetPlayBack::AddData(unsigned char *data, int datalen)
{
	if (!m_Buffer.IsInited())
	{
		m_Buffer.init(BUFFERSIZE, MAXWRITESIZE, MINWRITESIZE, 
			NetPlayBackBufferPlayFunction, &m_cbCallBack);
	}
    return m_Buffer.AddData(data, datalen);
}

int   CNetPlayBack::GetData(unsigned char *data, int datalen)
{
    return m_Buffer.GetData(data, datalen);
}

void  CNetPlayBack::DecDataLength(int datalen)
{
    m_Buffer.DecDataLength(datalen);
}

void  CNetPlayBack::Reset()
{
    m_Buffer.Reset();
}

int  CNetPlayBack::GetSize()
{
    return m_Buffer.GetSize();
}

void CNetPlayBack::Pause(enum PauseResumeID id)
{
	m_Buffer.Pause(id);
}

void CNetPlayBack::Resume(enum PauseResumeID id)
{
	m_Buffer.Resume(id);
}

void CNetPlayBack::IsRePause()
{
	m_Buffer.IsRePause();
}


