
#ifndef _AutoBuffer_H_
#define _AutoBuffer_H_

#include "Referable.h"

using namespace SPTools;

class CAutoBuffer : public IReferable
{
	friend class IReferable;
public:
	virtual ~CAutoBuffer(void);

	//创建以缓冲区aBuffer为内容的自维护缓存，isbackup表示内部是否复制一份
	//如果部分管理，必须复制，如果全部管理，可不复制,并接管外部缓存区；复制后不再管理外部缓冲区
	static CAutoBuffer * CreateBuffer( int nSize, char * aBuffer = NULL, bool isbackup = false );
	static CAutoBuffer * CreateBuffer( int nSize, const char * aBuffer );

	char * GetBuf();
	unsigned int BufferSize();

private:
	CAutoBuffer(void);
	BOOL SetBuf( int nSize, char * aBuffer = NULL );
	BOOL CopyBuf( int nSize, const char * aBuffer );

	char * m_buff;
	unsigned int m_datalen;
};

typedef CReferableObj<CAutoBuffer> IBufferRef;

#endif // _AutoBuffer_H_

