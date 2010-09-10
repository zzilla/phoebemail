
#ifdef  WIN32
#pragma warning(disable:4786)
#endif


#include "AutoBuffer.h"

CAutoBuffer::CAutoBuffer()
{
	m_buff = NULL;

	m_datalen = 0;
}

CAutoBuffer::~CAutoBuffer(void)
{
	if (m_buff != NULL)
	{
		delete m_buff;
		m_buff = NULL;
	}
	
	m_datalen = 0;
}

CAutoBuffer * CAutoBuffer::CreateBuffer( int nSize, char * aBuffer, bool isbackup )
{
	CAutoBuffer * _instance = NULL;

	if (nSize > 0)
	{
		_instance = new CAutoBuffer;
		if (_instance == NULL)
		{
			return NULL;
		}

		BOOL bSuccess = TRUE;

		if ( isbackup )
		{
			bSuccess = _instance->CopyBuf(nSize, (const char *)aBuffer);
		}
		else
		{
			bSuccess = _instance->SetBuf(nSize, aBuffer);
		}

		if (!bSuccess)
		{
			delete _instance;
			return NULL;
		}
	}

	return _instance;
}

CAutoBuffer * CAutoBuffer::CreateBuffer( int nSize, const char * aBuffer )
{
	CAutoBuffer * _instance = NULL;

	if (nSize > 0)
	{
		_instance = new CAutoBuffer;
		if (_instance == NULL)
		{
			return NULL;
		}

		BOOL bSuccess = _instance->CopyBuf(nSize, aBuffer);
		if (!bSuccess)
		{
			delete _instance;
			return NULL;
		}
	}
	
	return _instance;
}

//以下二方法将接管外部缓存区
BOOL CAutoBuffer::SetBuf( int nSize, char * aBuffer )
{
	if (m_buff != NULL)
	{
		delete m_buff;
		m_buff = NULL;
	}

	m_datalen = nSize;

	if (aBuffer != NULL)
	{
		m_buff = aBuffer;
	}
	else
	{
		m_buff = new char[nSize];
		if (m_buff == NULL)
		{
			m_datalen = 0;
			return FALSE;
		}

		memset(m_buff, 0, nSize);
	}

	return TRUE;
}

//以下方法将自创缓存内容
BOOL CAutoBuffer::CopyBuf( int nSize, const char * aBuffer )
{
	if (m_buff != NULL)
	{
		delete m_buff;
		m_buff = NULL;
	}

	m_datalen = nSize;

	if (aBuffer != NULL)
	{
		m_buff = new char [m_datalen + 1];
		if (m_buff == NULL)
		{
			m_datalen = 0;
			return FALSE;
		}

		memcpy(m_buff, aBuffer, m_datalen);
		m_buff[m_datalen] = 0;
	}

	return TRUE;
}

char * CAutoBuffer::GetBuf()
{
	return m_buff;
}

unsigned   CAutoBuffer::BufferSize()
{
	return m_datalen;
}

