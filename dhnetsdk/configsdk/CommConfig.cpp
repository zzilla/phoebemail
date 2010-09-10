
#include "StdAfx.h"
#include "CommConfig.h"
#include "json/json.h"


BOOL Comm_PTZ_Parse(char* szInBuffer, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)
{
	BOOL bRet = FALSE;
	
	if ( NULL == szInBuffer || NULL == lpOutBuffer || dwOutBufferSize < sizeof(CFG_PTZ_INFO) )
	{
		return bRet;
	}
	
	Json::Reader reader;
	static Json::Value value;
	
	CFG_PTZ_INFO stuPTZInfo = {0};
	memset(lpOutBuffer, 0, dwOutBufferSize);
	
	if(reader.parse(szInBuffer, value, false))		
	{
		if (value["Ptz"]["Chn"].type() != Json::nullValue)
		{
			stuPTZInfo.nChannelID = value["Ptz"]["Chn"].asInt();
		}

		int i = 0;
		int nStringLen = 0;
		for (i = 0; i < MAX_DECPRO_LIST_SIZE; i++)
		{
			if (value["Ptz"]["DecProName"][i]["name"].type() != Json::nullValue)
			{
				nStringLen = value["Ptz"]["DecProName"][i]["name"].asString().size();
				nStringLen = nStringLen>MAX_NAME_LEN?MAX_NAME_LEN:nStringLen;
				strncpy(stuPTZInfo.stuDecProName[i].name, (char *)value["Ptz"]["DecProName"][i]["name"].asString().c_str(), nStringLen);
			}
		}

		if (value["Ptz"]["ProName"].type() != Json::nullValue)
		{
			stuPTZInfo.nProName = value["Ptz"]["ProName"].asInt();
		}

		if (value["Ptz"]["Address"].type() != Json::nullValue)
		{
			stuPTZInfo.nDecoderAddress = value["Ptz"]["Address"].asInt();
		}

		if (value["Ptz"]["BaudBase"].type() != Json::nullValue)
		{
			stuPTZInfo.struComm.byBaudRate = value["Ptz"]["BaudBase"].asInt();
		}

		if (value["Ptz"]["DataBits"].type() != Json::nullValue)
		{
			stuPTZInfo.struComm.byDataBit = value["Ptz"]["DataBits"].asInt();
		}

		if (value["Ptz"]["StopBits"].type() != Json::nullValue)
		{
			stuPTZInfo.struComm.byStopBit = value["Ptz"]["StopBits"].asInt();
		}

		if (value["Ptz"]["Parity"].type() != Json::nullValue)
		{
			stuPTZInfo.struComm.byParity = value["Ptz"]["Parity"].asInt();
		}

		if (value["Ptz"]["MartixID"].type() != Json::nullValue)
		{
			stuPTZInfo.abMartixID = true;
			stuPTZInfo.nMartixID = value["Ptz"]["MartixID"].asInt();
		}

		if (value["Ptz"]["Type"].type() != Json::nullValue)
		{
			stuPTZInfo.abPTZType = true;
			stuPTZInfo.nPTZType = value["Ptz"]["Type"].asInt();
		}

		if (value["Ptz"]["CamID"].type() != Json::nullValue)
		{
			stuPTZInfo.abCamID = true;
			stuPTZInfo.nCamID = value["Ptz"]["CamID"].asInt();
		}
		
		bRet = TRUE;
		if (lpBytesReturned)
		{
			*lpBytesReturned = sizeof(CFG_PTZ_INFO);
		}
		memcpy(lpOutBuffer, &stuPTZInfo, sizeof(CFG_PTZ_INFO));
	}
	else
	{
		bRet = FALSE;
	}
	
	return bRet;
}

BOOL Comm_PTZ_Packet(LPVOID lpInBuffer, DWORD dwInBufferSize, char* szOutBuffer, DWORD dwOutBufferSize)
{
	BOOL bRet = FALSE;
	
	if ( NULL == lpInBuffer || NULL == szOutBuffer || dwInBufferSize < sizeof(CFG_PTZ_INFO) )
	{
		return bRet;
	}

	memset(szOutBuffer, 0, dwOutBufferSize);
	
	Json::Value value;
	CFG_PTZ_INFO *pPTZInfo = NULL;
	pPTZInfo = (CFG_PTZ_INFO *)lpInBuffer;

	bRet = TRUE;
	
	value["Ptz"]["Chn"] = pPTZInfo->nChannelID;
	value["Ptz"]["ProName"] = pPTZInfo->nProName;
	value["Ptz"]["Address"] = pPTZInfo->nDecoderAddress;
	value["Ptz"]["BaudBase"] = pPTZInfo->struComm.byBaudRate;
	value["Ptz"]["DataBits"] = pPTZInfo->struComm.byDataBit;
	value["Ptz"]["StopBits"] = pPTZInfo->struComm.byStopBit;
	value["Ptz"]["Parity"] = pPTZInfo->struComm.byParity;

	if (pPTZInfo->abMartixID)
	{
		value["Ptz"]["MartixID"] = pPTZInfo->nMartixID;
	}
	
	if (pPTZInfo->abPTZType)
	{
		value["Ptz"]["Type"] = pPTZInfo->nPTZType;
	}
	
	if (pPTZInfo->abCamID)
	{
		value["Ptz"]["CamID"] = pPTZInfo->nCamID;
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


