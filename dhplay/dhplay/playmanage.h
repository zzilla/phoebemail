   /********************************************************************
	created:	2006/01/10
	created:	10:1:2006   8:51
	filename: 	playmanage.h
	file base:	playmanage
	file ext:	h
	author:		chenmy
	
	purpose:	
*********************************************************************/
#ifndef PLAYMANAGE_H
#define PLAYMANAGE_H

#include "stdafx.h"
#include "play.h"
#include "callback.h"
#include "HI_PLAY_AudioIN.h"
#ifdef DHVECPLAY
	#include "dhvecplay.h"
#else
	#include "dhplay.h"
#endif

/*#include "Multimon.H"*/
#include <vector>

#include "AccurateTimer.h"

#define MAX_CHUNKSIZE 65536

//sdk的版本定义
#ifdef DHVECPLAY
//高16位表示当前的build号。9~16位表示主版本号，1~8位表示次版本号。
#define  VERSION  0x1207  //表示xx.yy
#define  BUILD_VER 0x0200  //编译版本

#else
#define VERSION 28
#define BUILD_VER 3
#define MEND_VER 3

#endif

//端口状态
typedef enum _PORT_STATE
{
	PORT_STATE_FREE,
	PORT_STATE_BUSY
}PORT_STATE;

//系统信息(需要提供给用户的系统信息)
typedef struct _SYS_INFO 
{
	int nstatus;     //系统支持状态(按位表示)
	DWORD dwVersion;  //SDK版本号
	//。。。。         //其他系统信息
}SYS_INFO;

//消息相关信息
typedef struct _MSG_INFO {
	BOOL nMsgFlag;    //1设置有效
	HWND hWnd;        //消息窗口句柄
	UINT nMsg;        //消息
	_MSG_INFO(){
		nMsgFlag = FALSE ;
		hWnd = NULL ;
	}
}MSG_INFO;

struct DeviceInfo
{
	LPSTR lpDeviceDescription ;
	LPSTR lpDeviceName ;
// 	HMONITOR* hMonitor ;
}  ;
//播放管理类定义

class CDHPLAY_MANAGE
{
public:
	int m_nSoundPort;                    //当前音频端口,独占方式
	std::vector<int> m_nShareSoundPortList ;//共享声音播放端口列表
// 	int m_volume;
	int m_nFluency[FUNC_MAX_PORT];
	int m_nQuality[FUNC_MAX_PORT];      //画质
	int m_nStreamMode[FUNC_MAX_PORT];
	int m_nTimer1Cnt;
	SYS_INFO m_pSysInfo;                //系统信息，包括显卡、声卡等一些系统信息
	char m_DLLPath[256+1024];//playsdk.dll所在的路径
	std::vector<DeviceInfo*>DeviceInfoList ;
	int m_supportmultidevice;
	int m_error[FUNC_MAX_PORT] ;    //错误类型
	CRITICAL_SECTION m_interfaceCritSec[FUNC_MAX_PORT] ;   
	CDHPlay   *pDHPlay[FUNC_MAX_PORT];    //播放对象指针表
	CDHAVData *pDHFile[FUNC_MAX_PORT] ; //播放文件指针表
	CDisplay  *pDisplay[FUNC_MAX_PORT] ;//显示类指针表
	CCallback *pCallback[FUNC_MAX_PORT];  //回调指针表
	MSG_INFO  *pMsgFileEnd[FUNC_MAX_PORT];//文件播放结束消息
	MSG_INFO  *pMsgEncChang[FUNC_MAX_PORT];    //编码格式改变消息
	PORT_STATE m_ePortState[FUNC_MAX_PORT];
	DWORD      m_dwTimerType[FUNC_MAX_PORT]; //端口对应的定时器类型

	CACTimerManager  m_ACTimerManager;
	CRITICAL_SECTION m_PortStateCrit;

	CHI_PLAY_AudioIn* pAudioRecored;
	DhRenderManager m_YUVRenderMng;
//主要程序函数有：
	CDHPLAY_MANAGE();
	~CDHPLAY_MANAGE();

	int  GetCaps();           //获取当前系统信息，按位取；
	int  CheckPort(LONG nPort);   //检测端口是否正常可控
	DWORD GetError(LONG nPort);    //获取错误码
	DWORD GetSdkVersion();    //获取版本号
	DWORD GetFileHeadLenth();    //获取文件头的长度
	BOOL  SetPortState(LONG lPort, PORT_STATE eState); //设置端口状态
	BOOL GetPortState(LONG lPort, PORT_STATE *peState); //获取端口状态
	int  OpenYuvRender(int port, HWND hwnd, void (CALLBACK* DrawFun)(int nPort,HDC hDc));
	int  RenderYuv(int port, unsigned char* py, unsigned char* pu, unsigned char* pv, int width, int height);
	int  CloseYuvRender(int port);

protected:
private:
	
	void init_sysinfo();
};

extern CDHPLAY_MANAGE g_cDHPlayManage;

#endif
