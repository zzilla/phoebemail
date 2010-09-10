//
// winsock2i.h - Include winsock2.h safely.
//
// Copyleft  02/24/2005  by freefalcon
//
//
// When WIN32_LEAN_AND_MEAN is not defined and _WIN32_WINNT is LESS THAN 0x400,
// if we include winsock2.h AFTER windows.h or winsock.h, we get some compiling
// errors as following:
//   winsock2.h(99) : error C2011: 'fd_set' : 'struct' type redefinition
// 
// When WIN32_LEAN_AND_MEAN is not defined and _WIN32_WINNT is NOT LESS THAN 0x400,
// if we include winsock2.h BEFORE windows.h, we get some other compiling errors:
//   mswsock.h(69) : error C2065: 'SOCKET' : undeclared identifier
// 
// So, this file is used to help us to include winsock2.h safely, it should be 
// placed before any other header files.
//
#ifndef DH_AX_API_WINSOCK2I_H_INCLUDED
#define DH_AX_API_WINSOCK2I_H_INCLUDED

#ifndef _WINSOCK2API_

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define   _WIN32_WINNT   0x0500		//to enable TryEnterCriticalSection()

#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif

#ifndef FD_SETSIZE
#define   FD_SETSIZE     4096
#endif

// Prevent inclusion of winsock.h
#ifdef _WINSOCKAPI_
#error Header winsock.h is included unexpectedly.
#endif

// NOTE: If you use Windows Platform SDK, you should enable following definition:
#define USING_WIN_PSDK

#if !defined(WIN32_LEAN_AND_MEAN) && (_WIN32_WINNT >= 0x0400) && !defined(USING_WIN_PSDK)
#include <windows.h>
#else
#include <winsock2.h>
#endif

#else//_WINSOCK2API_
#if defined(WIN32)||defined(WINCE)
#error Please Make Sure Only Include winsock2i.h
//解决方案，在文件头添加#include "winsock2i.h"
#endif

#endif//_WINSOCK2API_

#endif//DH_AX_API_WINSOCK2I_H_INCLUDED
