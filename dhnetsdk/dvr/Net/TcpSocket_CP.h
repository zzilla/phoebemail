/*
- Completion Port TCP socket class 
- TcpSocket_CP.h
- by lin_jianyan 
- 2007_09_12
*/

#ifndef LJY_COMPLETIONPORTTCP_H
#define LJY_COMPLETIONPORTTCP_H

#include <Winsock2.h>
#pragma comment( lib, "Ws2_32.lib")

//////////////////////////////////////////////////////////////////////////

#define DEFAULT_BUFFER_SIZE  (100*1024)
#define MAX_IPADDR_LEN	 16

//////////////////////////////////////////////////////////////////////////
class CTcpSocket_CP;
//server callback
typedef int (__stdcall *fListenCallBack)(CTcpSocket_CP* caller, char *ip, int port, CTcpSocket_CP *newSocket, DWORD dwUserData);
//disconnect call back
typedef int (__stdcall *fDisconnectCallBack)(DWORD dwUserData);
//receive data call back
typedef int (__stdcall *fTcpDataCallBack)(char *pbuf, int nlen, DWORD dwUserData);

class CPerIoData;
class CPerHandleData;
//////////////////////////////////////////////////////////////////////////

class CTcpSocket_CP
{
//constructor & destructor
public:
	CTcpSocket_CP();
	virtual ~CTcpSocket_CP();

//interfaces
public:
	//create socket , type 0:server socket，type 1:client socket
    int Create(int type = 1);

	//*server interfaces
	//if ip==NULL then listen on all local ip addresses
	int StartListen(const char *ip, int port, fListenCallBack listenFunc, DWORD dwUserData);
	int StopListen();
	//*end of server interfaces

	//*client interfaces
	int SetCallBack(fDisconnectCallBack disCallBack, fTcpDataCallBack dataCallBack, DWORD dwUserData);
	int ConnectHost(const char *szIp, int iPort, int iTimeOut = 1000); 
	int Disconnect();
	int SendData(char *pbuf, int nlen);

	//option functions
	//must be called before Create();
	int GetRecvBufferSize() {return m_iBufSize; }
	int SetRecvBufferSize(int iSize);
	//if enable TIME_WAIT status
	int EnableTimeWaitState(BOOL bEnableTimeWait=TRUE);
	//*end of client interfaces

//internal functions
public:
	int						InitializeNetwork();
	int						ClearNetwork();
	SOCKET					GetClienSocket() {return m_cltSocket; }
	HANDLE					GetSendEvent() {return m_hSendEvent; }
	fDisconnectCallBack		GetDisconectCBFunc() {return m_fDisconnectCallBack; }
	fTcpDataCallBack		GetDataCBFunc() { return m_fTcpDataCallBack; }
	DWORD					GetClientUserData() {return m_dwCltUserData; }
	int						SetSocket(SOCKET s, SOCKADDR_IN *addr, int addrLen);
	int						ListenSocket();
	int						CreateClientEnvironment();
	int						ClearClientEnvironment();
protected:

private:
	SOCKET		m_svrSocket;		//server socket
	SOCKADDR_IN	m_siSvrAddr;		//server address
	HANDLE		m_hListenThread;	//listening thread
	fListenCallBack		m_fListenCallBack;
	DWORD		m_dwSvrUserData;
	HANDLE		m_hListenExitEvent;	//exit event for the listening thread 

    SOCKET		m_cltSocket;		//client socket
	SOCKADDR_IN	m_siCltAddr;		//client address
	BOOL		m_bEnableTimeWait;

	char*		m_pBuf;
	int			m_iBufSize;		//default 100*1024
	CPerIoData*		m_pPerIoRecv;
//	CPerIoData*		m_pPerIoSend;
	CPerHandleData*	m_pPerHandleData;

	HANDLE		m_hSendEvent;
	CRITICAL_SECTION	m_csSend;	//限制发送接口不可多线程调用
//	int			m_iSendTimeout;
	
    fDisconnectCallBack	m_fDisconnectCallBack;	
    fTcpDataCallBack	m_fTcpDataCallBack;
	DWORD		m_dwCltUserData;	
};

#endif // LJTCPSOCKET_H
