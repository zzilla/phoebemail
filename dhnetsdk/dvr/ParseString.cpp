

#include "StdAfx.h"
#include "ParseString.h"

// 提取协议里的各个信息
char* GetProtocolValue(char *szBuf, const char *szKey1, const char *szKey2, char *szOutBuf, int nOutLen)
{
	if (szBuf == NULL || szKey1 == NULL || szKey2 == NULL || szOutBuf == NULL)
	{
		return 0;
	}

	char *p1 = strstr(szBuf, szKey1);
	if (p1 == NULL)
	{
		return 0;
	}

	char *p2 = strstr((p1+strlen(szKey1)), szKey2);
	if (p2 == NULL)
	{
		return 0;
	}

	int nValueLen = (int)(p2 - (p1+strlen(szKey1)));
	memset(szOutBuf, 0, nOutLen);
	if (nOutLen <= nValueLen)
	{
		return 0;
	}

	memcpy(szOutBuf, (p1+strlen(szKey1)), nValueLen);
	
	return (p1+strlen(szKey1));
}

