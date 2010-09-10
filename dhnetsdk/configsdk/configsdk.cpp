// configsdk.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#include "configsdk.h"
#include "AlarmConfig.h"
#include "CommConfig.h"
#include "EncodeConfig.h"
#include "NetworkConfig.h"
#include "RecordConfig.h"


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


CLIENT_API BOOL  CALL_METHOD CLIENT_ParseData(char* szCommand, char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, void* pReserved)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szCommand || NULL == szInBuffer || NULL == lpOutBuffer)
	{
		return bRet;
	}

	if (_stricmp(szCommand, CFG_CMD_ENCODE) == 0)
	{
		bRet = Encode_Common_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_RECORD) == 0)
	{
		bRet = Record_Common_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_ALARMINPUT) == 0)
	{
		bRet = Alarm_Input_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_NETALARMINPUT) == 0)
	{
		bRet = Alarm_Net_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_MOTIONDETECT) == 0)
	{
		bRet = Alarm_Motion_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_VIDEOLOST) == 0)
	{
		bRet = Alarm_Lost_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_VIDEOBLIND) == 0)
	{
		bRet = Alarm_Blind_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_STORAGENOEXIST) == 0)
	{
		bRet = Alarm_NoDisk_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_STORAGEFAILURE) == 0)
	{
		bRet = Alarm_StorageError_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_STORAGELOWSAPCE) == 0)
	{
		bRet = Alarm_NoDiskSpace_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_NETABORT) == 0)
	{
		bRet = Alarm_NetAbort_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_IPCONFLICT) == 0)
	{
		bRet = Alarm_IPConflict_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_SNAPCAPINFO) == 0)
	{
		bRet = Encode_SnapCap_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_NAS) == 0)
	{
		bRet = Net_NAS_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if (_stricmp(szCommand, CFG_CMD_PTZ) == 0)
	{
		bRet = Comm_PTZ_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else if(_stricmp(szCommand, CFG_CMD_WATERMARK) == 0)
	{
		bRet = Encode_WaterMark_Parse(szInBuffer, lpOutBuffer, dwOutBufferSize, (LPDWORD)pReserved);
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}


CLIENT_API BOOL  CALL_METHOD CLIENT_PacketData(char* szCommand, LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szCommand || NULL == lpInBuffer || NULL == szOutBuffer )
	{
		return bRet;
	}

	if (_stricmp(szCommand, CFG_CMD_ENCODE) == 0)
	{
		bRet = Encode_Common_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_RECORD) == 0)
	{
		bRet = Record_Common_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_ALARMINPUT) == 0)
	{
		bRet = Alarm_Input_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_NETALARMINPUT) == 0)
	{
		bRet = Alarm_Net_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_MOTIONDETECT) == 0)
	{
		bRet = Alarm_Motion_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_VIDEOLOST) == 0)
	{
		bRet = Alarm_Lost_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_VIDEOBLIND) == 0)
	{
		bRet = Alarm_Blind_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_STORAGENOEXIST) == 0)
	{
		bRet = Alarm_NoDisk_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_STORAGEFAILURE) == 0)
	{
		bRet = Alarm_StorageError_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_STORAGELOWSAPCE) == 0)
	{
		bRet = Alarm_NoDiskSpace_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_NETABORT) == 0)
	{
		bRet = Alarm_NetAbort_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_IPCONFLICT) == 0)
	{
		bRet = Alarm_IPConflict_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_SNAPCAPINFO) == 0)
	{
		bRet = Encode_SnapCap_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_NAS) == 0)
	{
		bRet = Net_NAS_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if (_stricmp(szCommand, CFG_CMD_PTZ) == 0)
	{
		bRet = Comm_PTZ_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else if(_stricmp(szCommand, CFG_CMD_WATERMARK) == 0)
	{
		bRet = Encode_WaterMark_Packet(lpInBuffer, dwInBufferSize, szOutBuffer, dwOutBufferSize);
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}





