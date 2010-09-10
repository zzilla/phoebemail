
#ifndef _ENCODECONFIG_H_
#define _ENCODECONFIG_H_
#include "configsdk.h"

BOOL Encode_Common_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);

BOOL Encode_Common_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);


BOOL Encode_WaterMark_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);

BOOL Encode_WaterMark_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);


BOOL Encode_SnapCap_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);

BOOL Encode_SnapCap_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize);


CFG_VIDEO_COMPRESSION CompressionStrtoInt(char *szCompression);

BOOL CompressionInttoStr(CFG_VIDEO_COMPRESSION emCompression, char *szValue);

CFG_BITRATE_CONTROL BitRateControlStrtoInt(char *szBitRateControl);

BOOL BitRateControlInttoStr(CFG_BITRATE_CONTROL emBitRateControl, char *szValue);

int FrameTypeStrtoInt(char *szFrameType);

BOOL FrameTypeInttoStr(int nFrameType, char *szValue);



#endif // _ENCODECONFIG_H_


