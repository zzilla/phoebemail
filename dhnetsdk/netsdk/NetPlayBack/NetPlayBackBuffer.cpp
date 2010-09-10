#include "NetPlayBackBuffer.h"

CNetPlayBackBuffer::CNetPlayBackBuffer()
{
    m_pData = 0;
    m_nTotalSize = m_nMaxWriteSize = m_nMinWriteSize = 0;
    m_nReadPos = m_nWritePos = 0;
    m_pPauseFunc = 0;
    m_userdata = 0;
    m_bWriteFlag = true;
	m_iPauseFlag = 0;
//	InitializeCriticalSection(&m_csBuf);
}

CNetPlayBackBuffer::~CNetPlayBackBuffer()
{
    if (m_pData)
    {
        delete[] m_pData;
    }
//	DeleteCriticalSection(&m_csBuf);
}

bool  CNetPlayBackBuffer::init(int nTotalSize, int nMaxWriteSize, int nMinWriteSize,
        NetPlayBackBufferPause pPauseFunc, void *userdata)
{
	bool bRet = false;
//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();
	
	m_iPauseFlag = 0;
	
    if (nTotalSize > 0 && pPauseFunc != 0)
    {
        m_nTotalSize = nTotalSize;
        m_nMaxWriteSize = nMaxWriteSize;
        m_nMinWriteSize = nMinWriteSize;
        if (m_pData)
        {
            delete[] m_pData;
        }
        m_pData = new unsigned char[nTotalSize];
        m_nReadPos = m_nWritePos = 0;
        m_pPauseFunc = pPauseFunc;
        m_userdata = userdata;
        m_bWriteFlag = true;
        bRet = true;
    }
	
//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
    return bRet;
}

bool  CNetPlayBackBuffer::IsInited()
{
	//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();
	
	bool bRet = m_pData!=NULL ? true : false;
	
	//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
    return bRet;
}

bool  CNetPlayBackBuffer::AddData(unsigned char *data, int datalen)
{
	if (!data || datalen <= 0)
	{
		return false;
	}

//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();

    /***********************缓冲数据***********************/
    //现在当包长大于存储空间时采取丢包的原则
	//如果不是中断过长时间的话不应有包过长的情况发生
    int totalSize = datalen + m_nWritePos;
    //如果缓冲区足够缓冲数据
    if (totalSize <= m_nTotalSize)
    {   
        memcpy(m_pData + m_nWritePos, data, datalen);
        m_nWritePos += datalen;
    }
    //如果缓冲区不足以缓冲数据,从头再来
    else
    {   
        if ((totalSize >= m_nTotalSize) 
            && m_nReadPos == 0)
        {
            OutputState();
            goto e_exit;
        }

        memmove(m_pData, m_pData + m_nReadPos, m_nWritePos - m_nReadPos);
        
        m_nWritePos -= m_nReadPos;
        m_nReadPos = 0;

        if ((datalen + m_nWritePos >= m_nTotalSize))
        {   
            OutputState();
            goto e_exit;
        }

        memcpy(m_pData + m_nWritePos, data, datalen);
        m_nWritePos += datalen;
    }

	if (m_nWritePos >= m_nMaxWriteSize && m_nReadPos != 0)
	{
        memmove(m_pData, m_pData + m_nReadPos, m_nWritePos - m_nReadPos);
        
        m_nWritePos -= m_nReadPos;
        m_nReadPos = 0;
	}

    OutputState();

//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
	return true;
e_exit:
//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
	return false;
}

int   CNetPlayBackBuffer::GetData(unsigned char *data, int datalen)
{
	if (!data || datalen <= 0)
	{
		return 0;
	}

//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();
	
	int nRet = 0;
    int size = m_nWritePos - m_nReadPos;
    if (size > 0)
    {
        int readlen = 0;
        if (size >= datalen)
        {
            readlen = datalen;
        }
        else
        {
            readlen = size;
        }
        memcpy(data, m_pData + m_nReadPos, datalen);
       // OutputState();
        nRet = readlen;
    }
    OutputState();
	
//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
    return nRet;
}

void  CNetPlayBackBuffer::DecDataLength(int datalen)
{
//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();

    m_nReadPos += datalen;
	OutputState();
	
//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
}
 
void  CNetPlayBackBuffer::Reset()
{
//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();

    m_nWritePos = m_nReadPos = 0;
	if (IsPaused(BUFFER_SHORT))
	{
		Resume(BUFFER_SHORT);
	}
    OutputState();

//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
}

int   CNetPlayBackBuffer::OutputState()
{  
	if (!IsPaused(BUFFER_SHORT))
	{
		if (m_nWritePos >= m_nMaxWriteSize)
		{
			
			this->Pause(BUFFER_SHORT);
		}
	}
    else
    {
		if (m_nWritePos - m_nReadPos <= m_nMinWriteSize/* && m_nWritePos >= m_nMaxWriteSize*/)
        {
			memmove(m_pData, m_pData + m_nReadPos, m_nWritePos - m_nReadPos);
        
			m_nWritePos -= m_nReadPos;
			m_nReadPos = 0;
			if (IsPaused(BUFFER_SHORT))
			{
				Resume(BUFFER_SHORT);
			}

            return 1;
        }
    }
	
    return 0;
}

void CNetPlayBackBuffer::Pause(enum PauseResumeID id)
{
//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();

	if (0 == m_iPauseFlag)
	{
		if (m_pPauseFunc)
		{
			m_pPauseFunc(true, m_userdata);
		}
	}
	m_iPauseFlag |= (1<<id);

//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
}

void CNetPlayBackBuffer::Resume(enum PauseResumeID id)
{
//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();

	m_iPauseFlag &= ~(1<<id);
	if (0 == m_iPauseFlag)
	{
		if (m_pPauseFunc)
		{
			m_pPauseFunc(false, m_userdata);
		}
	}
	
//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
}

BOOL CNetPlayBackBuffer::IsPaused(enum PauseResumeID id)
{
//	EnterCriticalSection(&m_csBuf);
	m_csBuf.Lock();

	BOOL bRet = FALSE;
	if (m_iPauseFlag & (1<<id))
	{
		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}

//	LeaveCriticalSection(&m_csBuf);
	m_csBuf.UnLock();
	return bRet;
}

void CNetPlayBackBuffer::IsRePause()
{
	m_csBuf.Lock();

	if (m_nWritePos - m_nReadPos >= m_nMaxWriteSize)
	{
		this->Pause(BUFFER_SHORT);
	}

	m_csBuf.UnLock();
}


