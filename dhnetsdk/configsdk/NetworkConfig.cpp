
#include "StdAfx.h"
#include "NetworkConfig.h"
#include "json/json.h"
#include "RecordConfig.h"


BOOL Net_NAS_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_NAS_INFO) )
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_NAS_INFO stuNASInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);

	int nStringLen = 0;
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["NAS"]["En"].type() != Json::nullValue)
		{
			stuNASInfo.bEnable = value["NAS"]["En"].asInt();
		}

		if (value["NAS"]["Version"].type() != Json::nullValue)
		{
			stuNASInfo.nVersion = value["NAS"]["Version"].asInt();
		}

		if (value["NAS"]["Protocol"].type() != Json::nullValue)
		{
			stuNASInfo.nProtocol = value["NAS"]["Protocol"].asInt();
		}

		if (value["NAS"]["Address"].type() != Json::nullValue)
		{
			nStringLen = value["NAS"]["Address"].asString().size();
			nStringLen = nStringLen>MAX_ADDRESS_LEN?MAX_ADDRESS_LEN:nStringLen;
			strncpy(stuNASInfo.szAddress, value["NAS"]["Address"].asString().c_str(), nStringLen);
		}

		if (value["NAS"]["Port"].type() != Json::nullValue)
		{
			stuNASInfo.nPort = value["NAS"]["Port"].asInt();
		}

		if (value["NAS"]["UserName"].type() != Json::nullValue)
		{
			nStringLen = value["NAS"]["UserName"].asString().size();
			nStringLen = nStringLen>MAX_USERNAME_LEN?MAX_USERNAME_LEN:nStringLen;
			strncpy(stuNASInfo.szUserName, value["NAS"]["UserName"].asString().c_str(), nStringLen);
		}

		if (value["NAS"]["Password"].type() != Json::nullValue)
		{
			nStringLen = value["NAS"]["Password"].asString().size();
			nStringLen = nStringLen>MAX_PASSWORD_LEN?MAX_PASSWORD_LEN:nStringLen;
			strncpy(stuNASInfo.szPassword, value["NAS"]["Password"].asString().c_str(), nStringLen);
		}

		if (value["NAS"]["Directory"].type() != Json::nullValue)
		{
			nStringLen = value["NAS"]["Directory"].asString().size();
			nStringLen = nStringLen>MAX_DIRECTORY_LEN?MAX_DIRECTORY_LEN:nStringLen;
			strncpy(stuNASInfo.szDirectory, value["NAS"]["Directory"].asString().c_str(), nStringLen);
		}

		if (value["NAS"]["FileLen"].type() != Json::nullValue)
		{
			stuNASInfo.nFileLen = value["NAS"]["FileLen"].asInt();
		}

		if (value["NAS"]["Interval"].type() != Json::nullValue)
		{
			stuNASInfo.nInterval = value["NAS"]["Interval"].asInt();
		}

		char szName[64] = {0};
		for(int i = 0; i < MAX_VIDEO_CHANNEL_NUM; i++)
		{
			for(int j = 0; j < WEEK_DAY_NUM; j++)
			{
				for (int k = 0; k < MAX_NAS_TIME_SECTION; k++)
				{
					sprintf(szName, "En%d", k);
					stuNASInfo.stuChnTime[i].stuTimeSection[j][k].dwRecordMask = value["NAS"]["ChnTime"][i][j][szName].asInt();
					
					sprintf(szName, "Time%d", k);
					sscanf((char *)value["NAS"]["ChnTime"][i][j][szName].asString().c_str(),
						"%02d:%02d:%02d-%02d:%02d:%02d",
						&stuNASInfo.stuChnTime[i].stuTimeSection[j][k].nBeginHour,
						&stuNASInfo.stuChnTime[i].stuTimeSection[j][k].nBeginMin,
						&stuNASInfo.stuChnTime[i].stuTimeSection[j][k].nBeginSec,
						&stuNASInfo.stuChnTime[i].stuTimeSection[j][k].nHourEnd,
						&stuNASInfo.stuChnTime[i].stuTimeSection[j][k].nEndMin,
						&stuNASInfo.stuChnTime[i].stuTimeSection[j][k].nEndSec);
				}	
			}
		}
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_NAS_INFO);
		}
		memcpy(lpOutBuffer, &stuNASInfo, sizeof(CFG_NAS_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Net_NAS_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_NAS_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_NAS_INFO *pNASInfo = NULL;
	pNASInfo = (CFG_NAS_INFO *)lpInBuffer;

	bRet = TRUE;

	value["NAS"]["En"] = pNASInfo->bEnable;
	value["NAS"]["Version"] = pNASInfo->nVersion;
	value["NAS"]["Protocol"] = pNASInfo->nProtocol;
	value["NAS"]["Address"] = pNASInfo->szAddress;
	value["NAS"]["Port"] = pNASInfo->nPort;
	value["NAS"]["UserName"] = pNASInfo->szUserName;
	value["NAS"]["Password"] = pNASInfo->szPassword;
	value["NAS"]["Directory"] = pNASInfo->szDirectory;
	value["NAS"]["FileLen"] = pNASInfo->nFileLen;
	value["NAS"]["Interval"] = pNASInfo->nInterval;
	
	char szName[64] = {0};
	for(int i = 0; i < MAX_VIDEO_CHANNEL_NUM; i++)
	{
		for(int j = 0; j < WEEK_DAY_NUM; j++)
		{
			for (int k = 0; k < MAX_NAS_TIME_SECTION; k++)
			{
				sprintf(szName, "En%d", k);
				value["NAS"]["ChnTime"][i][j][szName] = (int)pNASInfo->stuChnTime[i].stuTimeSection[j][k].dwRecordMask;
				
				sprintf(szName, "Time%d", k);
				char szBuf[128] = {0};
				sprintf(szBuf, "%02d:%02d:%02d-%02d:%02d:%02d", 
				pNASInfo->stuChnTime[i].stuTimeSection[j][k].nBeginHour,
				pNASInfo->stuChnTime[i].stuTimeSection[j][k].nBeginMin,
				pNASInfo->stuChnTime[i].stuTimeSection[j][k].nBeginSec,
				pNASInfo->stuChnTime[i].stuTimeSection[j][k].nHourEnd,
				pNASInfo->stuChnTime[i].stuTimeSection[j][k].nEndMin,
				pNASInfo->stuChnTime[i].stuTimeSection[j][k].nEndSec);
				value["NAS"]["ChnTime"][i][j][szName] = szBuf;
			}		
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




