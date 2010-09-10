
#include "StdAfx.h"
#include "AlarmConfig.h"
#include "RecordConfig.h"

BOOL Alarm_Input_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_ALARMIN_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_ALARMIN_INFO stuAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["Alarm"]["Chn"].type() != Json::nullValue)
		{
			stuAlarmInfo.nChannelID = value["Alarm"]["Chn"].asInt();
		}

		if (value["Alarm"]["En"].type() != Json::nullValue)
		{
			stuAlarmInfo.bEnable = value["Alarm"]["En"].asInt();
		}

		if (value["Alarm"]["SensorType"].type() != Json::nullValue)
		{
			stuAlarmInfo.nAlarmType = _stricmp((char *)value["Alarm"]["SensorType"].asString().c_str(), "NC")==0?0:1;
		}

		if (value["Alarm"]["Name"].type() != Json::nullValue)
		{
			int nNameLen = value["Alarm"]["Name"].asString().size();
			nNameLen = nNameLen>MAX_CHANNELNAME_LEN?MAX_CHANNELNAME_LEN:nNameLen;
			strncpy(stuAlarmInfo.szChnName, value["Alarm"]["Name"].asString().c_str(), nNameLen);
		}

		int i = 0;
		int j = 0;
		for(i = 0; i < WEEK_DAY_NUM; i++)
		{
			for( j = 0; j < MAX_REC_TSECT; j++)
			{	
				if (value["Alarm"]["EventHandler"]["TimeSection"][i][j].type() == Json::stringValue)
				{
					getTimeScheduleFromStr(stuAlarmInfo.stuTimeSection[i][j], (char *)value["Alarm"]["EventHandler"]["TimeSection"][i][j].asString().c_str());

				}
			}
		}

		ParseStrtoEventHandle(szInBuffer, "Alarm", &stuAlarmInfo.stuEventHandler);

		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_ALARMIN_INFO);
		}
		memcpy(lpOutBuffer, &stuAlarmInfo, sizeof(CFG_ALARMIN_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_Input_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_ALARMIN_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_ALARMIN_INFO *pInfo = NULL;
	pInfo = (CFG_ALARMIN_INFO *)lpInBuffer;

	bRet = TRUE;

	value["Alarm"]["Chn"] = pInfo->nChannelID;
	value["Alarm"]["En"] = pInfo->bEnable;
	value["Alarm"]["SensorType"] = pInfo->nAlarmType==0?"NC":"NO";
	value["Alarm"]["Name"] = pInfo->szChnName;

	int i = 0;
	int j = 0;
	for(i = 0; i < WEEK_DAY_NUM; i++)
	{
		for( j = 0; j < MAX_REC_TSECT; j++)
		{	
			char szBuf[128] = {0};
			sprintf(szBuf, "%d %02d:%02d:%02d-%02d:%02d:%02d", 
				pInfo->stuTimeSection[i][j].dwRecordMask,
				pInfo->stuTimeSection[i][j].nBeginHour,
				pInfo->stuTimeSection[i][j].nBeginMin,
				pInfo->stuTimeSection[i][j].nBeginSec,
				pInfo->stuTimeSection[i][j].nHourEnd,
				pInfo->stuTimeSection[i][j].nEndMin,
				pInfo->stuTimeSection[i][j].nEndSec);
			value["Alarm"]["EventHandler"]["TimeSection"][i][j] = szBuf;	
		}
	}

	BuildEventHandletoStr(pInfo->stuEventHandler, "Alarm", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_Net_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_NETALARMIN_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_NETALARMIN_INFO stuNetAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["NetAlarm"]["Chn"].type() != Json::nullValue)
		{
			stuNetAlarmInfo.nChannelID = value["NetAlarm"]["Chn"].asInt();
		}

		if (value["NetAlarm"]["En"].type() != Json::nullValue)
		{
			stuNetAlarmInfo.bEnable = value["NetAlarm"]["En"].asInt();
		}

		if (value["NetAlarm"]["SensorType"].type() != Json::nullValue)
		{
			stuNetAlarmInfo.nAlarmType = _stricmp((char *)value["NetAlarm"]["SensorType"].asString().c_str(), "NC")==0?0:1;
		}

		if (value["NetAlarm"]["Name"].type() != Json::nullValue)
		{
			int nNameLen = value["NetAlarm"]["Name"].asString().size();
			nNameLen = nNameLen>MAX_CHANNELNAME_LEN?MAX_CHANNELNAME_LEN:nNameLen;
			strncpy(stuNetAlarmInfo.szChnName, value["NetAlarm"]["Name"].asString().c_str(), nNameLen);
		}

		int i = 0;
		int j = 0;
		for(i = 0; i < WEEK_DAY_NUM; i++)
		{
			for( j = 0; j < MAX_REC_TSECT; j++)
			{
				if (value["NetAlarm"]["EventHandler"]["TimeSection"][i][j].type() == Json::stringValue)
				{
					getTimeScheduleFromStr(stuNetAlarmInfo.stuTimeSection[i][j], (char *)value["NetAlarm"]["EventHandler"]["TimeSection"][i][j].asString().c_str());

				}
			}
		}

		ParseStrtoEventHandle(szInBuffer, "NetAlarm", &stuNetAlarmInfo.stuEventHandler);

		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_NETALARMIN_INFO);
		}
		memcpy(lpOutBuffer, &stuNetAlarmInfo, sizeof(CFG_NETALARMIN_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_Net_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_NETALARMIN_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_NETALARMIN_INFO *pInfo = NULL;
	pInfo = (CFG_NETALARMIN_INFO *)lpInBuffer;

	bRet = TRUE;

	value["NetAlarm"]["Chn"] = pInfo->nChannelID;
	value["NetAlarm"]["En"] = pInfo->bEnable;
	value["NetAlarm"]["SensorType"] = pInfo->nAlarmType==0?"NC":"NO";
	value["NetAlarm"]["Name"] = pInfo->szChnName;

	int i = 0;
	int j = 0;
	for(i = 0; i < WEEK_DAY_NUM; i++)
	{
		for( j = 0; j < MAX_REC_TSECT; j++)
		{	
			char szBuf[128] = {0};
			sprintf(szBuf, "%d %02d:%02d:%02d-%02d:%02d:%02d", 
				pInfo->stuTimeSection[i][j].dwRecordMask,
				pInfo->stuTimeSection[i][j].nBeginHour,
				pInfo->stuTimeSection[i][j].nBeginMin,
				pInfo->stuTimeSection[i][j].nBeginSec,
				pInfo->stuTimeSection[i][j].nHourEnd,
				pInfo->stuTimeSection[i][j].nEndMin,
				pInfo->stuTimeSection[i][j].nEndSec);
			value["NetAlarm"]["EventHandler"]["TimeSection"][i][j] = szBuf;	
		}
	}

	BuildEventHandletoStr(pInfo->stuEventHandler, "NetAlarm", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_Motion_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_MOTION_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_MOTION_INFO stuMotionAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["MotionDetect"]["Chn"].type() != Json::nullValue)
		{
			stuMotionAlarmInfo.nChannelID = value["MotionDetect"]["Chn"].asInt();
		}

		if (value["MotionDetect"]["En"].type() != Json::nullValue)
		{
			stuMotionAlarmInfo.bEnable = value["MotionDetect"]["En"].asInt();
		}

		if (value["MotionDetect"]["Level"].type() != Json::nullValue)
		{
			stuMotionAlarmInfo.nSenseLevel = value["MotionDetect"]["Level"].asInt();
		}

		if (value["MotionDetect"]["Row"].type() != Json::nullValue)
		{
			stuMotionAlarmInfo.nMotionRow = value["MotionDetect"]["Row"].asInt();
		}

		if (value["MotionDetect"]["Col"].type() != Json::nullValue)
		{
			stuMotionAlarmInfo.nMotionCol = value["MotionDetect"]["Col"].asInt();
		}

		
		int i = 0;
		int j = 0;
		for (i = 0; i < stuMotionAlarmInfo.nMotionRow; i++)
		{
			char szValue[64] = {0};
			if (value["MotionDetect"]["Region"][i]["En"].type() != Json::nullValue)
			{
				strcpy(szValue, (char *)value["MotionDetect"]["Region"][i]["En"].asString().c_str());
			}
			
			for (int ncol = 0; ncol < stuMotionAlarmInfo.nMotionCol; ncol++)
			{
				if (szValue[ncol] == 0x31)//=="1"
				{
					stuMotionAlarmInfo.byRegion[i][ncol] = 1;
				}
				else
				{
					stuMotionAlarmInfo.byRegion[i][ncol] = 0;
				}
			}			
		}

		for(i = 0; i < WEEK_DAY_NUM; i++)
		{
			for( j = 0; j < MAX_REC_TSECT; j++)
			{	
				if (value["MotionDetect"]["EventHandler"]["TimeSection"][i][j].type() == Json::stringValue)
				{
					getTimeScheduleFromStr(stuMotionAlarmInfo.stuTimeSection[i][j], (char *)value["MotionDetect"]["EventHandler"]["TimeSection"][i][j].asString().c_str());

				}
			}
		}

		ParseStrtoEventHandle(szInBuffer, "MotionDetect", &stuMotionAlarmInfo.stuEventHandler);

		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_MOTION_INFO);
		}
		memcpy(lpOutBuffer, &stuMotionAlarmInfo, sizeof(CFG_MOTION_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_Motion_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_MOTION_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_MOTION_INFO *pInfo = NULL;
	pInfo = (CFG_MOTION_INFO *)lpInBuffer;

	bRet = TRUE;

	value["MotionDetect"]["Chn"] = pInfo->nChannelID;
	value["MotionDetect"]["En"] = pInfo->bEnable;
	value["MotionDetect"]["Level"] = pInfo->nSenseLevel;
	value["MotionDetect"]["Row"] = pInfo->nMotionRow;
	value["MotionDetect"]["Col"] = pInfo->nMotionCol;
	
	
	for (int k = 0; k < pInfo->nMotionRow; k++)
	{
		char szValue[128] = {0};
		for (int ncol = 0; ncol < pInfo->nMotionCol; ncol++)
		{
			if (pInfo->byRegion[k][ncol] == 1)
			{
				strcat(szValue, "1");
			}
			else
			{
				strcat(szValue, "0");
			}
		}	
		value["MotionDetect"]["Region"][k]["En"] = szValue;	
	}

	int i = 0;
	int j = 0;
	for(i = 0; i < WEEK_DAY_NUM; i++)
	{
		for( j = 0; j < MAX_REC_TSECT; j++)
		{	
			char szBuf[128] = {0};
			sprintf(szBuf, "%d %02d:%02d:%02d-%02d:%02d:%02d", 
				pInfo->stuTimeSection[i][j].dwRecordMask,
				pInfo->stuTimeSection[i][j].nBeginHour,
				pInfo->stuTimeSection[i][j].nBeginMin,
				pInfo->stuTimeSection[i][j].nBeginSec,
				pInfo->stuTimeSection[i][j].nHourEnd,
				pInfo->stuTimeSection[i][j].nEndMin,
				pInfo->stuTimeSection[i][j].nEndSec);
			value["MotionDetect"]["EventHandler"]["TimeSection"][i][j] = szBuf;	
		}
	}

	BuildEventHandletoStr(pInfo->stuEventHandler, "MotionDetect", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_Lost_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_VIDEOLOST_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_VIDEOLOST_INFO stuLostAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["LossDetect"]["Chn"].type() != Json::nullValue)
		{
			stuLostAlarmInfo.nChannelID = value["LossDetect"]["Chn"].asInt();
		}
		
		if (value["LossDetect"]["En"].type() != Json::nullValue)
		{
			stuLostAlarmInfo.bEnable = value["LossDetect"]["En"].asInt();
		}

		int i = 0;
		int j = 0;
		for(i = 0; i < WEEK_DAY_NUM; i++)
		{
			for( j = 0; j < MAX_REC_TSECT; j++)
			{	
				if (value["LossDetect"]["EventHandler"]["TimeSection"][i][j].type() == Json::stringValue)
				{
					getTimeScheduleFromStr(stuLostAlarmInfo.stuTimeSection[i][j], (char *)value["LossDetect"]["EventHandler"]["TimeSection"][i][j].asString().c_str());

				}
			}
		}
		
		ParseStrtoEventHandle(szInBuffer, "LossDetect", &stuLostAlarmInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_VIDEOLOST_INFO);
		}
		memcpy(lpOutBuffer, &stuLostAlarmInfo, sizeof(CFG_VIDEOLOST_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_Lost_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_VIDEOLOST_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_VIDEOLOST_INFO *pInfo = NULL;
	pInfo = (CFG_VIDEOLOST_INFO *)lpInBuffer;

	bRet = TRUE;

	value["LossDetect"]["Chn"] = pInfo->nChannelID;
	value["LossDetect"]["En"] = pInfo->bEnable;

	int i = 0;
	int j = 0;
	for(i = 0; i < WEEK_DAY_NUM; i++)
	{
		for( j = 0; j < MAX_REC_TSECT; j++)
		{	
			char szBuf[128] = {0};
			sprintf(szBuf, "%d %02d:%02d:%02d-%02d:%02d:%02d", 
				pInfo->stuTimeSection[i][j].dwRecordMask,
				pInfo->stuTimeSection[i][j].nBeginHour,
				pInfo->stuTimeSection[i][j].nBeginMin,
				pInfo->stuTimeSection[i][j].nBeginSec,
				pInfo->stuTimeSection[i][j].nHourEnd,
				pInfo->stuTimeSection[i][j].nEndMin,
				pInfo->stuTimeSection[i][j].nEndSec);
			value["LossDetect"]["EventHandler"]["TimeSection"][i][j] = szBuf;	
		}
	}

	BuildEventHandletoStr(pInfo->stuEventHandler, "LossDetect", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_Blind_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_SHELTER_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_SHELTER_INFO stuBlindAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["BlindDetect"]["Chn"].type() != Json::nullValue)
		{
			stuBlindAlarmInfo.nChannelID = value["BlindDetect"]["Chn"].asInt();
		}
		
		if (value["BlindDetect"]["En"].type() != Json::nullValue)
		{
			stuBlindAlarmInfo.bEnable = value["BlindDetect"]["En"].asInt();
		}

		if (value["BlindDetect"]["Level"].type() != Json::nullValue)
		{
			stuBlindAlarmInfo.nSenseLevel = value["BlindDetect"]["Level"].asInt();
		}
		
		int i = 0;
		int j = 0;
		for(i = 0; i < WEEK_DAY_NUM; i++)
		{
			for( j = 0; j < MAX_REC_TSECT; j++)
			{	
				if (value["BlindDetect"]["EventHandler"]["TimeSection"][i][j].type() == Json::stringValue)
				{
					getTimeScheduleFromStr(stuBlindAlarmInfo.stuTimeSection[i][j], (char *)value["BlindDetect"]["EventHandler"]["TimeSection"][i][j].asString().c_str());

				}
			}
		}

		ParseStrtoEventHandle(szInBuffer, "BlindDetect", &stuBlindAlarmInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_SHELTER_INFO);
		}
		memcpy(lpOutBuffer, &stuBlindAlarmInfo, sizeof(CFG_SHELTER_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_Blind_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_SHELTER_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_SHELTER_INFO *pInfo = NULL;
	pInfo = (CFG_SHELTER_INFO *)lpInBuffer;

	bRet = TRUE;

	value["BlindDetect"]["Chn"] = pInfo->nChannelID;
	value["BlindDetect"]["En"] = pInfo->bEnable;
	value["BlindDetect"]["Level"] = pInfo->nSenseLevel;

	int i = 0;
	int j = 0;
	for(i = 0; i < WEEK_DAY_NUM; i++)
	{
		for( j = 0; j < MAX_REC_TSECT; j++)
		{
			char szBuf[128] = {0};
			sprintf(szBuf, "%d %02d:%02d:%02d-%02d:%02d:%02d", 
				pInfo->stuTimeSection[i][j].dwRecordMask,
				pInfo->stuTimeSection[i][j].nBeginHour,
				pInfo->stuTimeSection[i][j].nBeginMin,
				pInfo->stuTimeSection[i][j].nBeginSec,
				pInfo->stuTimeSection[i][j].nHourEnd,
				pInfo->stuTimeSection[i][j].nEndMin,
				pInfo->stuTimeSection[i][j].nEndSec);
			value["BlindDetect"]["EventHandler"]["TimeSection"][i][j] = szBuf;	
		}
	}

	BuildEventHandletoStr(pInfo->stuEventHandler, "BlindDetect", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_IPConflict_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_IPCONFLICT_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_IPCONFLICT_INFO stuIPAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{	
		if (value["IPConflict"]["En"].type() != Json::nullValue)
		{
			stuIPAlarmInfo.bEnable = value["IPConflict"]["En"].asInt();
		}
		
		ParseStrtoEventHandle(szInBuffer, "IPConflict", &stuIPAlarmInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_IPCONFLICT_INFO);
		}
		memcpy(lpOutBuffer, &stuIPAlarmInfo, sizeof(CFG_IPCONFLICT_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_IPConflict_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_IPCONFLICT_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_IPCONFLICT_INFO *pInfo = NULL;
	pInfo = (CFG_IPCONFLICT_INFO *)lpInBuffer;

	bRet = TRUE;

	value["IPConflict"]["En"] = pInfo->bEnable;

	BuildEventHandletoStr(pInfo->stuEventHandler, "IPConflict", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_NetAbort_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_NETABORT_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_NETABORT_INFO stuNetAbortAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{	
		if (value["NetAbort"]["En"].type() != Json::nullValue)
		{
			stuNetAbortAlarmInfo.bEnable = value["NetAbort"]["En"].asInt();
		}
		
		ParseStrtoEventHandle(szInBuffer, "NetAbort", &stuNetAbortAlarmInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_NETABORT_INFO);
		}
		memcpy(lpOutBuffer, &stuNetAbortAlarmInfo, sizeof(CFG_NETABORT_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_NetAbort_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_NETABORT_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_NETABORT_INFO *pInfo = NULL;
	pInfo = (CFG_NETABORT_INFO *)lpInBuffer;

	bRet = TRUE;

	value["NetAbort"]["En"] = pInfo->bEnable;

	BuildEventHandletoStr(pInfo->stuEventHandler, "NetAbort", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_NoDisk_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_STORAGENOEXIST_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_STORAGENOEXIST_INFO stuNoDiskAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{	
		if (value["StorageNotExist"]["En"].type() != Json::nullValue)
		{
			stuNoDiskAlarmInfo.bEnable = value["StorageNotExist"]["En"].asInt();
		}
		
		ParseStrtoEventHandle(szInBuffer, "StorageNotExist", &stuNoDiskAlarmInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_STORAGENOEXIST_INFO);
		}
		memcpy(lpOutBuffer, &stuNoDiskAlarmInfo, sizeof(CFG_STORAGENOEXIST_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_NoDisk_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_STORAGENOEXIST_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_STORAGENOEXIST_INFO *pInfo = NULL;
	pInfo = (CFG_STORAGENOEXIST_INFO *)lpInBuffer;

	bRet = TRUE;

	value["StorageNotExist"]["En"] = pInfo->bEnable;

	BuildEventHandletoStr(pInfo->stuEventHandler, "StorageNotExist", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_StorageError_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_STORAGEFAILURE_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_STORAGEFAILURE_INFO stuDiskErrorAlarmInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{	
		if (value["StorageFailure"]["En"].type() != Json::nullValue)
		{
			stuDiskErrorAlarmInfo.bEnable = value["StorageFailure"]["En"].asInt();
		}
		
		ParseStrtoEventHandle(szInBuffer, "StorageFailure", &stuDiskErrorAlarmInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_STORAGEFAILURE_INFO);
		}
		memcpy(lpOutBuffer, &stuDiskErrorAlarmInfo, sizeof(CFG_STORAGEFAILURE_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_StorageError_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_STORAGEFAILURE_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_STORAGEFAILURE_INFO *pInfo = NULL;
	pInfo = (CFG_STORAGEFAILURE_INFO *)lpInBuffer;

	bRet = TRUE;

	value["StorageFailure"]["En"] = pInfo->bEnable;

	BuildEventHandletoStr(pInfo->stuEventHandler, "StorageFailure", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}


BOOL Alarm_NoDiskSpace_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_STORAGELOWSAPCE_INFO))
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_STORAGELOWSAPCE_INFO stuDiskNoSpaceInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{	
		if (value["StorageLowSpace"]["En"].type() != Json::nullValue)
		{
			stuDiskNoSpaceInfo.bEnable = value["StorageLowSpace"]["En"].asInt();
		}

		if (value["StorageLowSpace"]["LowerLimit"].type() != Json::nullValue)
		{
			stuDiskNoSpaceInfo.nLowerLimit = value["StorageLowSpace"]["LowerLimit"].asInt();
		}
		
		ParseStrtoEventHandle(szInBuffer, "StorageLowSpace", &stuDiskNoSpaceInfo.stuEventHandler);
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_STORAGELOWSAPCE_INFO);
		}
		memcpy(lpOutBuffer, &stuDiskNoSpaceInfo, sizeof(CFG_STORAGELOWSAPCE_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Alarm_NoDiskSpace_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_STORAGELOWSAPCE_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_STORAGELOWSAPCE_INFO *pInfo = NULL;
	pInfo = (CFG_STORAGELOWSAPCE_INFO *)lpInBuffer;

	bRet = TRUE;

	value["StorageLowSpace"]["En"] = pInfo->bEnable;
	value["StorageLowSpace"]["LowerLimit"] = pInfo->nLowerLimit;

	BuildEventHandletoStr(pInfo->stuEventHandler, "StorageLowSpace", value);

	std::string str;
	Json::FastWriter writer(str);
	writer.write(value);

	if (dwOutBufferSize < str.size())
	{
		bRet = FALSE;
	}
	else
	{
		strcpy(szOutBuffer, str.c_str());
	}
		
	return bRet;
}

BOOL ParseStrtoEventHandle(char *szInBuffer, char *szName, CFG_ALARM_MSG_HANDLE *pstruHandle)
{
	if (pstruHandle == NULL || szInBuffer == NULL || szName == NULL)
	{
		return -1;
	}

	int i = 0;
	int j = 0;
	
	Json::Reader reader;
	static Json::Value value;
	
	memset(pstruHandle, 0, sizeof(CFG_ALARM_MSG_HANDLE));
	
	if(reader.parse(szInBuffer, value, false))		
	{
		for (i = 0; i < MAX_CHANNEL_NUM; i++)
		{
			for (j = 0; j < 32; j++)
			{
				if (value[szName]["EventHandler"]["Record"][32*i+j]["En"].type() != Json::nullValue)
				{
					pstruHandle->abRecordMask = true;
					if (value[szName]["EventHandler"]["Record"][32*i+j]["En"].asInt() == 1)
					{
						pstruHandle->dwRecordMask[i] |= (0x01<<j);	
					}
					pstruHandle->nChannelCount++;
				}

				if (value[szName]["EventHandler"]["AlarmOut"][32*i+j]["En"].type() != Json::nullValue)
				{
					pstruHandle->abAlarmOutMask = true;
					if (value[szName]["EventHandler"]["AlarmOut"][32*i+j]["En"].asInt() == 1)
					{
						pstruHandle->dwAlarmOutMask[i] |= (0x01<<j);
					}
					pstruHandle->nAlarmOutCount++;
				}

				if (value[szName]["EventHandler"]["Tour"][32*i+j]["En"].type() != Json::nullValue)
				{
					pstruHandle->abTourMask = true;
					if (value[szName]["EventHandler"]["Tour"][32*i+j]["En"].asInt() == 1)
					{
						pstruHandle->dwTourMask[i] |= (0x01<<j);	
					}
				}

				if (value[szName]["EventHandler"]["Snapshot"][32*i+j]["En"].type() != Json::nullValue)
				{
					pstruHandle->abSnapshot = true;
					if (value[szName]["EventHandler"]["Snapshot"][32*i+j]["En"].asInt() == 1)
					{
						pstruHandle->dwSnapshot[i] |= (0x01<<j);	
					}
				}

				if (value[szName]["EventHandler"]["Matrix"][32*i+j]["En"].type() != Json::nullValue)
				{
					pstruHandle->abMatrixMask = true;
					if (value[szName]["EventHandler"]["Matrix"][32*i+j]["En"].asInt() == 1)
					{
						pstruHandle->dwMatrixMask[i] |= (0x01<<j);	
					}
				}
			}
		}

		for (i = 0; i < MAX_VIDEO_CHANNEL_NUM; i++)
		{
			if (value[szName]["EventHandler"]["PtzLink"][i]["Type"].type() != Json::nullValue)
			{
				char szValue[32] = {0};
				strcpy(szValue, (char *)value[szName]["EventHandler"]["PtzLink"][i]["Type"].asString().c_str());
				if (_stricmp(szValue, "None") == 0)
				{
					pstruHandle->stuPtzLink[i].emType = LINK_TYPE_NONE;
				}
				else if (_stricmp(szValue, "Preset") == 0)
				{
					pstruHandle->stuPtzLink[i].emType = LINK_TYPE_PRESET;
				}
				else if (_stricmp(szValue, "Tour") == 0)
				{
					pstruHandle->stuPtzLink[i].emType = LINK_TYPE_TOUR;
				}
				else if (_stricmp(szValue, "Pattern") == 0)
				{
					pstruHandle->stuPtzLink[i].emType = LINK_TYPE_PATTERN;
				}	
			}
			
			if (value[szName]["EventHandler"]["PtzLink"][i]["Value"].type() != Json::nullValue)
			{
				pstruHandle->stuPtzLink[i].nValue = value[szName]["EventHandler"]["PtzLink"][i]["Value"].asInt();
			}	
		}
	
		if (value[szName]["EventHandler"]["RecordEn"].type() != Json::nullValue)
		{
			pstruHandle->abRecordEnable = true;
			pstruHandle->bRecordEnable = value[szName]["EventHandler"]["RecordEn"].asInt();
		}

		if (value[szName]["EventHandler"]["RecordLatch"].type() != Json::nullValue)
		{
			pstruHandle->abRecordLatch = true;
			pstruHandle->nRecordLatch = value[szName]["EventHandler"]["RecordLatch"].asInt();
		}

		if (value[szName]["EventHandler"]["AlarmOutEn"].type() != Json::nullValue)
		{
			pstruHandle->abAlarmOutEn = true;
			pstruHandle->bAlarmOutEn = value[szName]["EventHandler"]["AlarmOutEn"].asInt();
		}

		if (value[szName]["EventHandler"]["AlarmOutLatch"].type() != Json::nullValue)
		{
			pstruHandle->abAlarmOutLatch = true;
			pstruHandle->nAlarmOutLatch = value[szName]["EventHandler"]["AlarmOutLatch"].asInt();
		}
		
		if (value[szName]["EventHandler"]["PtzLinkEn"].type() != Json::nullValue)
		{
			pstruHandle->abPtzLinkEn = true;
			pstruHandle->bPtzLinkEn = value[szName]["EventHandler"]["PtzLinkEn"].asInt();
		}
		
		if (value[szName]["EventHandler"]["TourEn"].type() != Json::nullValue)
		{
			pstruHandle->abTourEnable = true;
			pstruHandle->bTourEnable = value[szName]["EventHandler"]["TourEn"].asInt();
		}

		if (value[szName]["EventHandler"]["SnapshotEn"].type() != Json::nullValue)
		{
			pstruHandle->abSnapshotEn = true;
			pstruHandle->bSnapshotEn = value[szName]["EventHandler"]["SnapshotEn"].asInt();
		}

		if (value[szName]["EventHandler"]["SnapshotTimes"].type() != Json::nullValue)
		{
			pstruHandle->abSnapshotTimes = true;
			pstruHandle->nSnapshotTimes = value[szName]["EventHandler"]["SnapshotTimes"].asInt();
		}

		if (value[szName]["EventHandler"]["TipEn"].type() != Json::nullValue)
		{
			pstruHandle->abTipEnable = true;
			pstruHandle->bTipEnable = value[szName]["EventHandler"]["TipEn"].asInt();
		}
		
		if (value[szName]["EventHandler"]["MailEn"].type() != Json::nullValue)
		{
			pstruHandle->abMailEnable = true;
			pstruHandle->bMailEnable = value[szName]["EventHandler"]["MailEn"].asInt();
		}

		if (value[szName]["EventHandler"]["MessageEn"].type() != Json::nullValue)
		{
			pstruHandle->abMessageEnable = true;
			pstruHandle->bMessageEnable = value[szName]["EventHandler"]["MessageEn"].asInt();
		}

		if (value[szName]["EventHandler"]["BeepEn"].type() != Json::nullValue)
		{
			pstruHandle->abBeepEnable = true;
			pstruHandle->bBeepEnable = value[szName]["EventHandler"]["BeepEn"].asInt();
		}

		if (value[szName]["EventHandler"]["VoiceEn"].type() != Json::nullValue)
		{
			pstruHandle->abVoiceEnable = true;
			pstruHandle->bVoiceEnable = value[szName]["EventHandler"]["VoiceEn"].asInt();
		}

		if (value[szName]["EventHandler"]["MatrixEn"].type() != Json::nullValue)
		{
			pstruHandle->abMatrixEnable = true;
			pstruHandle->bMatrixEnable = value[szName]["EventHandler"]["MatrixEn"].asInt();
		}

		if (value[szName]["EventHandler"]["EventLatch"].type() != Json::nullValue)
		{
			pstruHandle->abEventLatch = true;
			pstruHandle->nEventLatch = value[szName]["EventHandler"]["EventLatch"].asInt();
		}

		if (value[szName]["EventHandler"]["LogEn"].type() != Json::nullValue)
		{
			pstruHandle->abLogEnable = true;
			pstruHandle->bLogEnable = value[szName]["EventHandler"]["LogEn"].asInt();
		}

		if (value[szName]["EventHandler"]["Delay"].type() != Json::nullValue)
		{
			pstruHandle->abDelay = true;
			pstruHandle->nDelay = value[szName]["EventHandler"]["Delay"].asInt();
		}

		if (value[szName]["EventHandler"]["OnVideoMessageEn"].type() != Json::nullValue)
		{
			pstruHandle->abVideoMessageEn = true;
			pstruHandle->bVideoMessageEn = value[szName]["EventHandler"]["OnVideoMessageEn"].asInt();
		}

		if (value[szName]["EventHandler"]["MMSEn"].type() != Json::nullValue)
		{
			pstruHandle->abMMSEnable = true;
			pstruHandle->bMMSEnable = value[szName]["EventHandler"]["MMSEn"].asInt();
		}

		if (value[szName]["EventHandler"]["MessageToNetEn"].type() != Json::nullValue)
		{
			pstruHandle->abMessageToNetEn = true;
			pstruHandle->bMessageToNetEn = value[szName]["EventHandler"]["MessageToNetEn"].asInt();
		}
	}
	else
	{
		return -1;
	}
	
	return 0;
}

BOOL BuildEventHandletoStr(CFG_ALARM_MSG_HANDLE struHandle, char *szName, Json::Value &value)
{
	if (szName == NULL)
	{
		return -1;
	}
	
	int i = 0;
	int j = 0;

	int nChannelCount = 1;
	int nAlarmOutCount = 1;
	if (struHandle.nChannelCount != 0)
	{
		if (struHandle.nChannelCount%32==0)
		{
			nChannelCount = struHandle.nChannelCount/32;
		}
		else
		{
			nChannelCount = struHandle.nChannelCount/32+1;
		}
	}

	if (struHandle.nAlarmOutCount != 0)
	{
		if (struHandle.nAlarmOutCount%32==0)
		{
			nAlarmOutCount = struHandle.nAlarmOutCount/32;
		}
		else
		{
			nAlarmOutCount = struHandle.nAlarmOutCount/32+1;
		}
	}
	
	for (i = 0; i < nChannelCount; i++)
	{
		for (j = 0; j < 32; j++)
		{
			if (32*i+j >= struHandle.nChannelCount)
			{
				break;
			}
			if (struHandle.abRecordMask)
			{
				if ((struHandle.dwRecordMask[i]>>j)&0x01)
				{
					value[szName]["EventHandler"]["Record"][32*i+j]["En"] = 1;
				}
				else
				{
					value[szName]["EventHandler"]["Record"][32*i+j]["En"] = 0;
				}
			}
			
			if (struHandle.abTourMask)
			{
				if ((struHandle.dwTourMask[i]>>j)&0x01)
				{
					value[szName]["EventHandler"]["Tour"][32*i+j]["En"] = 1;
				}
				else
				{
					value[szName]["EventHandler"]["Tour"][32*i+j]["En"] = 0;
				}
			}
			
			if (struHandle.abSnapshot)
			{
				if ((struHandle.dwSnapshot[i]>>j)&0x01)
				{
					value[szName]["EventHandler"]["Snapshot"][32*i+j]["En"] = 1;
				}
				else
				{
					value[szName]["EventHandler"]["Snapshot"][32*i+j]["En"] = 0;
				}
			}

			if (struHandle.abMatrixMask)
			{
				if ((struHandle.dwMatrixMask[i]>>j)&0x01)
				{
					value[szName]["EventHandler"]["Matrix"] = 1;
				}
				else
				{
					value[szName]["EventHandler"]["Matrix"] = 0;
				}
			}	
		}
	}

	for (i = 0; i < nAlarmOutCount; i++)
	{
		for (j = 0; j < 32; j++)
		{
			if (32*i+j >= struHandle.nAlarmOutCount)
			{
				break;
			}
			if (struHandle.abAlarmOutMask)
			{
				if ((struHandle.dwAlarmOutMask[i]>>j)&0x01)
				{
					value[szName]["EventHandler"]["AlarmOut"][32*i+j]["En"] = 1;
				}
				else
				{
					value[szName]["EventHandler"]["AlarmOut"][32*i+j]["En"] = 0;
				}
			}
		}
	}
	
	for (i = 0; i < struHandle.nChannelCount; i++)
	{
		switch(struHandle.stuPtzLink[i].emType)
		{
		case LINK_TYPE_NONE:
			value[szName]["EventHandler"]["PtzLink"][i]["Type"] = "None";
			break;
		case LINK_TYPE_PRESET:
			value[szName]["EventHandler"]["PtzLink"][i]["Type"] = "Preset";
			break;
		case LINK_TYPE_TOUR:
			value[szName]["EventHandler"]["PtzLink"][i]["Type"] = "Tour";
			break;
		case LINK_TYPE_PATTERN:
			value[szName]["EventHandler"]["PtzLink"][i]["Type"] = "Pattern";
			break;
		default:
			value[szName]["EventHandler"]["PtzLink"][i]["Type"] = "None";
			break;
		}
		
		value[szName]["EventHandler"]["PtzLink"][i]["Value"] = struHandle.stuPtzLink[i].nValue;	
	}
	
	if (struHandle.abRecordEnable)
	{
		value[szName]["EventHandler"]["RecordEn"] = struHandle.bRecordEnable;
	}

	if (struHandle.abRecordLatch)
	{
		value[szName]["EventHandler"]["RecordLatch"] = struHandle.nRecordLatch;
	}

	if (struHandle.abAlarmOutEn)
	{
		value[szName]["EventHandler"]["AlarmOutEn"] = struHandle.bAlarmOutEn;
	}
	
	if (struHandle.abPtzLinkEn)
	{
		value[szName]["EventHandler"]["PtzLinkEn"] = struHandle.bPtzLinkEn;
	}

	if (struHandle.abAlarmOutLatch)
	{
		value[szName]["EventHandler"]["AlarmOutLatch"] = struHandle.nAlarmOutLatch;
	}

	if (struHandle.abTourEnable)
	{
		value[szName]["EventHandler"]["TourEn"] = struHandle.bTourEnable;
	}

	if (struHandle.abSnapshotEn)
	{
		value[szName]["EventHandler"]["SnapshotEn"] = struHandle.bSnapshotEn;
	}

	if (struHandle.abSnapshotTimes)
	{
		value[szName]["EventHandler"]["SnapshotTimes"] = struHandle.nSnapshotTimes;
	}

	if (struHandle.abTipEnable)
	{
		value[szName]["EventHandler"]["TipEn"] = struHandle.bTipEnable;
	}

	if (struHandle.abMailEnable)
	{
		value[szName]["EventHandler"]["MailEn"] = struHandle.bMailEnable;
	}

	if (struHandle.abMessageEnable)
	{
		value[szName]["EventHandler"]["MessageEn"] = struHandle.bMessageEnable;
	}

	if (struHandle.abBeepEnable)
	{
		value[szName]["EventHandler"]["BeepEn"] = struHandle.bBeepEnable;
	}

	if (struHandle.abVoiceEnable)
	{
		value[szName]["EventHandler"]["VoiceEn"] = struHandle.bVoiceEnable;
	}

	if (struHandle.abMatrixEnable)
	{
		value[szName]["EventHandler"]["MatrixEn"] = struHandle.bMatrixEnable;
	}

	if (struHandle.abEventLatch)
	{
		value[szName]["EventHandler"]["EventLatch"] = struHandle.nEventLatch;
	}

	if (struHandle.abLogEnable)
	{
		value[szName]["EventHandler"]["LogEn"] = struHandle.bLogEnable;
	}
	
	if (struHandle.abDelay)
	{
		value[szName]["EventHandler"]["Delay"] = struHandle.nDelay;
	}

	if (struHandle.abVideoMessageEn)
	{
		value[szName]["EventHandler"]["OnVideoMessageEn"] = struHandle.bVideoMessageEn;
	}

	if (struHandle.abMMSEnable)
	{
		value[szName]["EventHandler"]["MMSEn"] = struHandle.bMMSEnable;
	}
	
	if (struHandle.abMessageToNetEn)
	{
		value[szName]["EventHandler"]["MessageToNetEn"] = struHandle.bMessageToNetEn;
	}
		
	return 0;	
}