
#if !defined NETPLAYBACK_H_
#define NETPLAYBACK_H_

#include "NetPlayBackBuffer.h"

typedef bool (__stdcall *NetPlayBack_ReadDataPause)
        (bool bPause, void *userdata);

typedef struct
{
    NetPlayBack_ReadDataPause     ReadDataPauseFunc;
    void                          *pUserData;
}NetPlayBack_CallBack;

class CNetPlayBack
{
public:
    CNetPlayBack(NetPlayBack_CallBack  cbCallBack);
    ~CNetPlayBack();
protected:
    NetPlayBack_CallBack m_cbCallBack;
    CNetPlayBackBuffer   m_Buffer;   
public:
    bool  AddData(unsigned char *data, int datalen);
    int   GetData(unsigned char *data, int datalen);
    void  DecDataLength(int datalen);
    void  Reset();
    int   GetSize();
	void  IsRePause();

	void  Pause(enum PauseResumeID id);
	void  Resume(enum PauseResumeID id);
};

#endif

