
#ifndef _NETWORKCONFIG_H_
#define _NETWORKCONFIG_H_

#include "configsdk.h"
BOOL Net_NAS_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);

BOOL Net_NAS_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);



#endif // _NETWORKCONFIG_H_


