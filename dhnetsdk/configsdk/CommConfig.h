
#ifndef _COMMCONFIG_H_
#define _COMMCONFIG_H_

#include "configsdk.h"
BOOL Comm_PTZ_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);

BOOL Comm_PTZ_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);



#endif // _COMMCONFIG_H_


