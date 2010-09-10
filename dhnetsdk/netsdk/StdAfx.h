// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__31AC7DFE_0537_45E0_BEA1_7DB6EEA627A7__INCLUDED_)
#define AFX_STDAFX_H__31AC7DFE_0537_45E0_BEA1_7DB6EEA627A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4786)

#else	//linux

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#endif

#include "../dvr/osIndependent.h"
#include "../dvr/mutex.h"
#include "../dvr/ReadWriteMutex.h"
#include "../dvr/kernel/afkdef.h"
#include "../dvr/kernel/afkinc.h"
#include "../dvr/dvrdevice/dvr2cfg.h"
#include "../dvr/versionctl.h"

#include <list>
#include <algorithm>
using namespace std;
// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__31AC7DFE_0537_45E0_BEA1_7DB6EEA627A7__INCLUDED_)

