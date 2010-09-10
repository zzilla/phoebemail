
#if !defined NETPLAYBACKBUFFER_H_
#define NETPLAYBACKBUFFER_H_

#include "../StdAfx.h"

typedef bool (__stdcall *NetPlayBackBufferPause)
        (bool bPause, void *userdata);

/*下面这个枚举描述的是不同的原因在暂停标志中所占的位*/
enum PauseResumeID
{
	BUFFER_SHORT = 0,	//因为缓冲区不够而暂停
	BUFFER_RESET,		//因为缓冲区要清空而暂停
	BUFFER_OTHER,		//其它
};

class CNetPlayBackBuffer
{
public:
    CNetPlayBackBuffer();
    ~CNetPlayBackBuffer();
public:
    bool  init(int nTotalSize, int nMaxWriteSize, int nMinWriteSize,
        NetPlayBackBufferPause pPauseFunc, void *userdata);
	bool  IsInited();

    bool  AddData(unsigned char *data, int datalen);
    int   GetData(unsigned char *data, int datalen);
    void  DecDataLength(int datalen);
    void  Reset();
	void  IsRePause();

	void  Pause(enum PauseResumeID id);
	void  Resume(enum PauseResumeID id);

    int   GetSize() { return m_nWritePos - m_nReadPos;}

protected:
    DEVMutex			 m_csBuf;

    int             m_nTotalSize;
    int             m_nMaxWriteSize;
    int             m_nMinWriteSize;

    unsigned char  *m_pData;
    int             m_nReadPos;
    int             m_nWritePos;

    int             m_bWriteFlag;
	int				m_iPauseFlag;

    NetPlayBackBufferPause m_pPauseFunc;
    void           *m_userdata;

protected:
    int             OutputState();

private:
	BOOL			IsPaused(enum PauseResumeID id);
};

#endif

