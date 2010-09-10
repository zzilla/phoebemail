
#ifndef _RECORDCONFIG_H_
#define _RECORDCONFIG_H_

#include "configsdk.h"
BOOL Record_Common_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);

BOOL Record_Common_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);

BOOL getTimeScheduleFromStr(CFG_TIME_SECTION & schedule, char *szTime);


#endif // _RECORDCONFIG_H_


