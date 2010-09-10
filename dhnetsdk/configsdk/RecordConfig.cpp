
#include "StdAfx.h"
#include "RecordConfig.h"
#include "json/json.h"


BOOL Record_Common_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_RECORD_INFO) )
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_RECORD_INFO stuRecordInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["Record"]["Chn"].type() != Json::nullValue)
		{
			stuRecordInfo.nChannelID = value["Record"]["Chn"].asInt();
		}

		if (value["Record"]["PreRec"].type() != Json::nullValue)
		{
			stuRecordInfo.nPreRecTime = value["Record"]["PreRec"].asInt();
		}

		if (value["Record"]["RedundancyEn"].type() != Json::nullValue)
		{
			stuRecordInfo.bRedundancyEn = value["Record"]["RedundancyEn"].asInt();
		}

		if (value["Record"]["Stream"].type() != Json::nullValue)
		{
			stuRecordInfo.nStreamType = value["Record"]["Stream"].asInt();
		}
		
		for(int i = 0; i < WEEK_DAY_NUM; i++)
		{
			for(int j = 0; j < MAX_REC_TSECT; j++)
			{
				if (value["Record"]["TimeSection"][i][j].type() == Json::stringValue)
				{
					getTimeScheduleFromStr(stuRecordInfo.stuTimeSection[i][j], (char *)value["Record"]["TimeSection"][i][j].asString().c_str());
				}	
			}
		}
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_RECORD_INFO);
		}
		memcpy(lpOutBuffer, &stuRecordInfo, sizeof(CFG_RECORD_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Record_Common_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_RECORD_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_RECORD_INFO *pRecordInfo = NULL;
	pRecordInfo = (CFG_RECORD_INFO *)lpInBuffer;

	bRet = TRUE;
	
//	if (pRecordInfo->abChannelID)
	{
		value["Record"]["Chn"] = pRecordInfo->nChannelID;
	}

	value["Record"]["PreRec"] = pRecordInfo->nPreRecTime;

	value["Record"]["RedundancyEn"] = pRecordInfo->bRedundancyEn;

	value["Record"]["Stream"] = pRecordInfo->nStreamType;
	
	for(int i = 0; i < WEEK_DAY_NUM; i++)
	{
		for(int j = 0; j < MAX_REC_TSECT; j++)
		{
			char szBuf[128] = {0};
			sprintf(szBuf, "%d %02d:%02d:%02d-%02d:%02d:%02d",
				pRecordInfo->stuTimeSection[i][j].dwRecordMask,
				pRecordInfo->stuTimeSection[i][j].nBeginHour,
				pRecordInfo->stuTimeSection[i][j].nBeginMin,
				pRecordInfo->stuTimeSection[i][j].nBeginSec,
				pRecordInfo->stuTimeSection[i][j].nHourEnd,
				pRecordInfo->stuTimeSection[i][j].nEndMin,
				pRecordInfo->stuTimeSection[i][j].nEndSec);
			value["Record"]["TimeSection"][i][j] = szBuf;	
		}
	}
	
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


// 根据"00:00:00-24:00:00"得到结构体时间表
BOOL getTimeScheduleFromStr(CFG_TIME_SECTION & schedule, char *szTime)
{	
	if(sscanf(szTime,
		"%d %02d:%02d:%02d-%02d:%02d:%02d",
		&schedule.dwRecordMask,
		&schedule.nBeginHour,
		&schedule.nBeginMin,
		&schedule.nBeginSec,
		&schedule.nHourEnd,
		&schedule.nEndMin,
		&schedule.nEndSec) != 7)
	{
		return FALSE;
	}
		
	return TRUE;
}
