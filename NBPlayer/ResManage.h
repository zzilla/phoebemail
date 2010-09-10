#ifndef __RESMANAGE_INC__
#define __RESMANAGE_INC__

#include <map>
#include <list>
#include "AX_Mutex.h"
#include "videorender.h"
#include "INBPlayer.h"
using namespace std;

class CResManage
{
public:
	CResManage(void);
	~CResManage(void);

protected:
	static CResManage m_staticThis;
public:
	static CResManage* Instance();

//初始化
protected:
	bool m_bManageInit;
public:
	bool InitManage();
	void UnInitManage();

//绘图资源的保存
protected:
	AX_Mutex		m_mtxRander;
	map<HWND, list<VideoRender*> >	m_mapRander;
public:
	void			SaveRander(HWND hWnd, VideoRender* pRander);
	VideoRender*	LoadRander(HWND hWnd);

//端口资源保存
protected:
	AX_Mutex		m_mtxPort;
	list<int>		m_listPort[StreamSourceType_Num];
	list<uint32>	m_listPortBufSize[StreamSourceType_Num];
public:
	void			SavePort(int nDevType, int nPort, uint32 nBufSize);		//nDevType小于2
	int				LoadPort(int nDevType, uint32 nBufSize);				//－1为失败

//缓冲区管理
protected:
	AX_Mutex		m_mtxFrameBuffer;
	list<char*>		m_listFrameBuffer;
public:
	void			SaveFrameBuffer(char* pBuf);
	char*			LoadFrameBuffer();

//其他函数
public:
	void ClearAllResource();
};

#endif //__RESMANAGE_INC__