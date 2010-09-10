#include "StdAfx.h"
#include "Utils.h"
#include "Utils_StrParser.h"
#include "../dvr/def.h"

#ifndef WIN32	//linux
#include <iconv.h>
#endif

int Cut_UTF8_Tail(unsigned char *pUTF8, int nLen)
{
	if(NULL == pUTF8 || nLen < 0)
	{
		return -1;
	}
	//nLen == 0取字符串长度。
	int nLength = (0 == nLen) ? strlen((char *)pUTF8) : nLen;
	if(strlen((char *)pUTF8) < nLength && nLen > 0)
	{
		return 0;
	}
	unsigned char *pPos = pUTF8+nLength-1;
	for(int i = 0; i < 8; i++)
	{
		if(*(pPos-i) > 191)//0x10111111
		{
			for (int j = 0 ; j < 8 ; j++)
			{
				if (0 == ((0x80>>j) & *(pPos-i)))
				{
					j--;
					break;
				}
			}
			if(i < j)
			{
				memset(pPos-i, 0, i);
			}
			break;
		}
	}
	return i;
}

void Change_Utf8_Assic(unsigned char * pUTF8, char *destbuf)
{
#ifdef WIN32

	int nSrcStrLen = strlen((char*)pUTF8);
	int nDestStrLen = nSrcStrLen*2;
	char* tmpBuf = new char[nDestStrLen];
	memset(tmpBuf, 0, nDestStrLen);

	int nNum = MultiByteToWideChar(CP_UTF8, 0, (char*)pUTF8, nSrcStrLen, (LPWSTR)tmpBuf, nDestStrLen);
	nNum = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)tmpBuf, nNum, destbuf, nSrcStrLen, NULL, NULL);
	
	delete tmpBuf;
	tmpBuf = NULL;

#else	//linux

	iconv_t cd = iconv_open("gbk","utf-8"); 
	if(cd==(iconv_t)-1)
	{
		printf("iconv_t fail.\n");
	}
	
	size_t inLen = strlen((char*)pUTF8);
	size_t outLen = inLen;
	unsigned int nconv = iconv(cd, (char**)&pUTF8, &inLen, &destbuf, &outLen);
	
	iconv_close(cd);

#endif

	return ;
}

void Change_Assic_UTF8(char *pStrGBKData, int nlen, char *pbuf, int buflen)
{
#ifdef WIN32

	int nDestStrLen = nlen*2;
	char* tmpBuf = new char[nDestStrLen];
	memset(tmpBuf, 0, nDestStrLen);

	int nNum = MultiByteToWideChar(CP_ACP, 0, (char*)pStrGBKData, nlen, (LPWSTR)tmpBuf, nDestStrLen);
	nNum = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)tmpBuf, nNum, pbuf, buflen, NULL, NULL);
	
	delete tmpBuf;
	tmpBuf = NULL;
	
#else	//linux

	iconv_t cd = iconv_open("utf-8","gbk"); 
	if(cd==(iconv_t)-1)
	{
		printf("iconv_t fail.\n");
	}
	
	size_t inLen = nlen;
	size_t outLen = buflen;
	unsigned int nconv = iconv(cd, &pStrGBKData, &inLen, &pbuf, &outLen);
	
	iconv_close(cd);

#endif

	return ;
}

////////////////////////////////////////////////////////////////////
//解析设备返回用户信息

///////////parse operation-right item//////////

int ParseRightItemEx(char *buf, int bufLen, OPR_RIGHT_EX *rItem,DWORD* dwListLength,int nMaxLength)
{
	if (0 == bufLen)
	{
		return 0;
	}
	if(NULL == buf)
	{
		return -1;
	}
	int i=0; //for for;
	int nSize=0;
	BYTE szBuf[32]={0};
	CStrParse bufParse;
	bufParse.setSpliter("&&");
	bool bResult = bufParse.Parse(buf);
	if( false == bResult)
	{
		return -1;
	}
	nSize = bufParse.Size();
	if(nSize > MAX_RIGHT_NUM)
	{
		nSize = MAX_RIGHT_NUM;
		//return -1;
	}
	*dwListLength = nSize;

	CStrParse parse;
	parse.setSpliter(":");
	
	for(;i<nSize;i++)
	{

		bResult=parse.Parse(bufParse.getWord(i).c_str());
		if(false == bResult)
		{
			return -1;
		}
		
		rItem[i].dwID = parse.getValue(0);
		int nstrLen = strlen(parse.getWord(1).c_str());
		if(nstrLen > RIGHT_NAME_LENGTH)
		{
				return -1;//名字长度超过支持的最大长度
		}
		memcpy(rItem[i].name, parse.getWord(1).c_str(),nstrLen);
		rItem[i].name[RIGHT_NAME_LENGTH-1] = '\0' ;
		nstrLen = strlen(parse.getWord(2).c_str());
		if( nstrLen > MEMO_LENGTH)
		{
			return -1;
		}
		
		memset(szBuf, 0, 32);
		//memcpy(rItem[i].memo,parse.getWord(2).c_str(),nstrLen);
		memcpy(szBuf,/*rItem[i].memo,*/parse.getWord(2).c_str(),nstrLen);
		Change_Utf8_Assic(szBuf, rItem[i].memo);
	}
	return 0;
}

/*
int ParseRightItemEx(char *buf, int bufLen, OPR_RIGHT_EX *rItem)
{
	if (0 == bufLen)
	{
		return 0;
	}
	
	int itemPos = 0;
	int itemStPos = 0;
	int itemSubLen = 0;
	int itemIndex = 0;
	char tmp[32] = {0};
	
	if (!rItem || !buf)
	{
		goto e_out;
	}
	while (itemPos < bufLen) 
	{
		if (buf[itemPos] != ':')
		{
			itemPos++;
			itemSubLen++;
		}
		else //get item
		{
			switch(itemIndex)
			{
			case 0: //id
				if (itemSubLen >= 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				rItem->dwID = atoi(tmp);
				break;
			case 1: //name
				if (itemSubLen > USER_NAME_LENGTH_EX)
				{
					goto e_out;
				}
				memcpy(rItem->name, buf+itemStPos, itemSubLen);
				break;
			default:
				goto e_out;
			}
			itemIndex ++;
			itemPos++;
			itemStPos = itemPos;
			itemSubLen = 0;
		}
	}
	
	if (itemPos != 0 && 2 == itemIndex)
	{
		if (itemSubLen > MEMO_LENGTH)
		{
			goto e_out;
		}
		memcpy(rItem->memo, buf+itemStPos, itemSubLen);
	}
	else
	{
		goto e_out;
	}
	
	return 0;
e_out:
	return -1;
}
*/
int ParseRightItem(char *buf, int bufLen, OPR_RIGHT *rItem)
{
	if (0 == bufLen)
	{
		return 0;
	}

	int itemPos = 0;
	int itemStPos = 0;
	int itemSubLen = 0;
	int itemIndex = 0;
	char tmp[32] = {0};
	if (!rItem || !buf)
	{
		goto e_out;
	}
	while (itemPos < bufLen) 
	{
		if (buf[itemPos] != ':')
		{
			itemPos++;
			itemSubLen++;
		}
		else //get item
		{
			switch(itemIndex)
			{
			case 0: //id
				if (itemSubLen >= 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				rItem->dwID = atoi(tmp);
				break;
			case 1: //name
				if (itemSubLen > RIGHT_NAME_LENGTH)
				{
					goto e_out;
				}
				memcpy(rItem->name, buf+itemStPos, itemSubLen);
				break;
			default:
				goto e_out;
			}
			itemIndex ++;
			itemPos++;
			itemStPos = itemPos;
			itemSubLen = 0;
		}
	}

	if (itemPos != 0 && 2 == itemIndex)
	{
		if (itemSubLen > MEMO_LENGTH)
		{
			goto e_out;
		}
		memcpy(rItem->memo, buf+itemStPos, itemSubLen);
	}
	else
	{
		goto e_out;
	}
	
	return 0;
e_out:
	return -1;
}

///////////parse right-code string////////////
int ParseRightCode(char *buf, int bufLen, DWORD *dwList, DWORD *lstLen)
{
	if (0 == bufLen) 
	{
		return 0;
	}
	
	int cdPos = 0;
	int cdStPos = 0;
	int cdSubLen = 0;
	int cdIndex = 0;
	char tmp[8] = {0};

	if (!dwList || !buf)
	{
		goto e_out;
	}
	while (cdPos < bufLen) 
	{
		if (buf[cdPos] != ',')
		{
			cdPos++;
			cdSubLen++;
		}
		else //get code
		{
			if (cdSubLen >= 8)
			{
				goto e_out;
			}
			memset(tmp, 0, 8);
			memcpy(tmp, buf+cdStPos, cdSubLen);
			dwList[cdIndex] = atoi(tmp);

			cdIndex ++;
			if (cdIndex >= MAX_RIGHT_NUM)
			{
				goto e_out;
			}
			cdPos++;
			cdStPos = cdPos;
			cdSubLen = 0;
		}
	}
	
	if (cdPos != 0 && cdIndex < MAX_RIGHT_NUM)
	{
		if (cdSubLen >= 8)
		{
			goto e_out;
		}
		memset(tmp, 0, 8);
		memcpy(tmp, buf+cdStPos, cdSubLen);
		dwList[cdIndex] = atoi(tmp);
		if (lstLen)
		{
			*lstLen = cdIndex + 1;
		}
	}
	else
	{
		goto e_out;
	}
	
	return 0;
e_out:
	return -1;
}

///////////parse group item//////////
int ParseGroupItem(char *buf, int bufLen, USER_GROUP_INFO *gpItem)
{
	if (0 == bufLen) 
	{
		return 0;
	}

	int itemPos = 0;
	int itemStPos = 0;
	int itemSubLen = 0;
	int itemIndex = 0;
	char tmp[32] = {0};
	int ret;

	if (!gpItem || !buf)
	{
		goto e_out;
	}

	while (itemPos < bufLen) 
	{
		if (buf[itemPos] != ':')
		{
			itemPos++;
			itemSubLen++;
		}
		else //get item
		{
			switch(itemIndex)
			{
			case 0: //id
				if (itemSubLen > 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				gpItem->dwID = atoi(tmp);
				break;
			case 1: //name
				if (itemSubLen > USER_NAME_LENGTH) 
				{
					goto e_out;
				}
				BYTE szGpName[USER_NAME_LENGTH+1];
				memset(szGpName, 0, USER_NAME_LENGTH+1);
				memcpy(szGpName, buf+itemStPos, itemSubLen);
				Change_Utf8_Assic(szGpName, gpItem->name);
				//gpItem->name[USER_NAME_LENGTH-1] = '\0';
				//memcpy(gpItem->name, buf+itemStPos, itemSubLen);
				break;
			case 2: //right-code list
				ret = ParseRightCode(buf+itemStPos, itemSubLen, gpItem->rights, &gpItem->dwRightNum);
				if (ret < 0)
				{
					goto e_out;
				}
				break;
			default:
				goto e_out;
			}
			itemIndex ++;
			itemPos++;
			itemStPos = itemPos;
			itemSubLen = 0;
		}
	}
	
	if (itemPos != 0 && 3 == itemIndex)
	{
		if (itemSubLen > MEMO_LENGTH)
		{
			goto e_out;
		}
		BYTE szDemo[MEMO_LENGTH+1];
		memset(szDemo, 0, MEMO_LENGTH+1);
		memcpy(szDemo, buf+itemStPos, itemSubLen);
		Change_Utf8_Assic(szDemo, gpItem->memo);
	//	gpItem->memo[MEMO_LENGTH-1] = '\0';
	//	memcpy(gpItem->memo, buf+itemStPos, itemSubLen);
	}
	else
	{
		goto e_out;
	}
	
	return 0;
e_out:
	return -1;
}

int ParseGroupItemEx(char *buf, int bufLen, USER_GROUP_INFO_EX *gpItem,DWORD *dwListLength,int nMaxLength)
{
	if(0 == bufLen)
	{
		return 0;
	}
	if( NULL == gpItem || NULL == buf)
	{
		return -1;
	}
	int i=0;
	int nSize=0;
	int nstrLen;
	bool bResult;
	BYTE szBuf[32]={0};
	CStrParse bufParse;
	CStrParse parse;
	bufParse.setSpliter("&&");
	bResult = bufParse.Parse(buf);
	if(false == bResult)
	{
		return -1;
	}
	nSize = bufParse.Size();
	if(nSize > MAX_GROUP_NUM)
	{
		nSize = MAX_GROUP_NUM;
		//return -1;
	}
	*dwListLength = nSize;
	for (;i<nSize;i++)
	{
	
		parse.setSpliter(":");
		bResult = parse.Parse(bufParse.getWord(i).c_str());
		if(false == bResult)
		{
			return -1;
		}
		//id
		gpItem[i].dwID = parse.getValue(0);
		//name
		nstrLen=parse.getWord(1).size();
		if(nstrLen > nMaxLength)
		{
			return -1;
		}
		memset(szBuf,0,32);
		memcpy(szBuf,parse.getWord(1).c_str(),nstrLen);
		Change_Utf8_Assic(szBuf, gpItem[i].name);
		//gpItem[i].name[USER_NAME_LENGTH_EX-1]='\0';
		//right-code list
		{
			CStrParse rightParse;
			rightParse.setSpliter(",");
			bResult = rightParse.Parse(parse.getWord(2).c_str());
			gpItem[i].dwRightNum = rightParse.Size();
			if(gpItem[i].dwRightNum >	MAX_RIGHT_NUM )
			{
				return -1;
			}
			for(int j=0 ; j < gpItem[i].dwRightNum ; j++)
			{
				gpItem[i].rights[j]=rightParse.getValue(j);
			}
		}
		//memo
		nstrLen=parse.getWord(3).size();
		if(nstrLen > MEMO_LENGTH)
		{
			return -1;
		}
		memset(szBuf,0,32);
		memcpy(szBuf,/*gpItem[i].memo,*/parse.getWord(3).c_str(),nstrLen);
		Change_Utf8_Assic(szBuf, gpItem[i].memo);

	}
	return 0;
}

/*
int ParseGroupItemEx(char *buf, int bufLen, USER_GROUP_INFO_EX *gpItem)
{
	if (0 == bufLen) 
	{
		return 0;
	}
	
	int itemPos = 0;
	int itemStPos = 0;
	int itemSubLen = 0;
	int itemIndex = 0;
	char tmp[32] = {0};
	int ret;

	
	if (!gpItem || !buf)
	{
		goto e_out;
	}
	
	while (itemPos < bufLen) 
	{
		if (buf[itemPos] != ':')
		{
			itemPos++;
			itemSubLen++;
		}
		else //get item
		{
			switch(itemIndex)
			{
			case 0: //id
				if (itemSubLen > 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				gpItem->dwID = atoi(tmp);
				break;
			case 1: //name
				if (itemSubLen > USER_NAME_LENGTH_EX) 
				{
					goto e_out;
				}
				BYTE szGpName[USER_NAME_LENGTH_EX+1];
				memset(szGpName, 0, USER_NAME_LENGTH_EX+1);
				memcpy(szGpName, buf+itemStPos, itemSubLen);
				Change_Utf8_Assic(szGpName, gpItem->name);
				gpItem->name[USER_NAME_LENGTH_EX-1] = '\0';
				//memcpy(gpItem->name, buf+itemStPos, itemSubLen);
				break;
			case 2: //right-code list
				ret = ParseRightCode(buf+itemStPos, itemSubLen, gpItem->rights, &gpItem->dwRightNum);
				if (ret < 0)
				{
					goto e_out;
				}
				break;
			default:
				goto e_out;
			}
			itemIndex ++;
			itemPos++;
			itemStPos = itemPos;
			itemSubLen = 0;
		}
	}
	
	if (itemPos != 0 && 3 == itemIndex)
	{
		if (itemSubLen > MEMO_LENGTH)
		{
			goto e_out;
		}
		BYTE szDemo[MEMO_LENGTH+1];
		memset(szDemo, 0, MEMO_LENGTH+1);
		memcpy(szDemo, buf+itemStPos, itemSubLen);
		Change_Utf8_Assic(szDemo, gpItem->memo);
		gpItem->memo[MEMO_LENGTH-1] = '\0';
		//	memcpy(gpItem->memo, buf+itemStPos, itemSubLen);
	}
	else
	{
		goto e_out;
	}
	
	return 0;
e_out:
	return -1;
}
*/
int ParseUserItemEx(char *buf, int bufLen, USER_INFO_EX *urItem, DWORD * dwListLength,void* special,int nMaxLength)
{
	if(0 == bufLen)
	{
		return 0;
	}
	if(NULL == buf || NULL == urItem ||NULL == dwListLength || NULL == special )
	{
		return -1;
	}
	int nstrLen;
	int i=0;
	int nSize;
	bool bResult;
	BYTE szBuf[32]={0};
	CStrParse bufParse;
	CStrParse parse;
	bufParse.setSpliter("&&");
	bResult = bufParse.Parse(buf);
	if(false == bResult)
	{
		return -1;
	}
	nSize = bufParse.Size();
	if(nSize > MAX_USER_NUM)
	{
		nSize = MAX_USER_NUM;
		//return -1;
	}
	*dwListLength = nSize;
	for(;i<nSize;i++)
	{
	
		parse.setSpliter(":");
		bool bResult = parse.Parse(bufParse.getWord(i).c_str());
		if( false == bResult)
		{
			return -1;
		}
		//id
		urItem[i].dwID = parse.getValue(0);
		//name
		nstrLen = parse.getWord(1).size();
		if( nstrLen > nMaxLength)
		{
			return -1;
		}
		memset(szBuf,0,32);
		memcpy(szBuf,parse.getWord(1).c_str(),nstrLen);
		Change_Utf8_Assic(szBuf, urItem[i].name);
		//urItem[i].name[nMaxLength-1]='\0';
		//password
		nstrLen = parse.getWord(2).size();
		if( nstrLen > nMaxLength)
		{
			return -1;
		}
		memcpy(urItem[i].passWord,parse.getWord(2).c_str(),nstrLen);
		//group no
		urItem[i].dwGroupID = parse.getValue(3);
		//right code list
		{
			CStrParse rightParse;
			rightParse.setSpliter(",");
			bResult = rightParse.Parse(parse.getWord(4));
			urItem[i].dwRightNum = rightParse.Size();
			if(urItem[i].dwRightNum >	MAX_RIGHT_NUM )
			{
				return -1;
			}
			for(int j=0 ; j <= urItem[i].dwRightNum ; j++)
			{
				urItem[i].rights[j]=rightParse.getValue(j);
			}
		}
		//memo
		nstrLen = parse.getWord(5).size();
		if( nstrLen > MEMO_LENGTH)
		{
			return -1;
		}
	
		memset(szBuf,0,32);
		memcpy(szBuf,/*urItem[i].memo,*/parse.getWord(5).c_str(),nstrLen);
		Change_Utf8_Assic(szBuf, urItem[i].memo);

		//fouctionMask
		if(parse.Size() == 5 )
		{
			urItem[i].dwFouctionMask =0;
			*(int *)special = 0;//m没有复用
			return 0;
		}
		urItem[i].dwFouctionMask= parse.getValue(6);
		*(int *)special = 1;
	}

	return 0;
}

/*
int ParseUserItemEx(char *buf, int bufLen, USER_INFO_EX *urItem, void* special)
{
	if (0 == bufLen) 
	{
		return 0;
	}
	
	int *nsp = (int*)special;
	if (NULL == nsp)
	{
		return -1;
	}
	
	int itemPos = 0;
	int itemStPos = 0;
	int itemSubLen = 0;
	int itemIndex = 0;
	char tmp[32] = {0};
	int ret = -1;
	int iSupportLength=0;
	
	if (!urItem || !buf)
	{
		goto e_out;
	}
	
	while (itemPos < bufLen) 
	{
		if (buf[itemPos] != ':')
		{
			itemPos++;
			itemSubLen++;
		}
		else //get item
		{
			switch(itemIndex)
			{
			case 0: //id
				if (itemSubLen > 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				urItem->dwID = atoi(tmp);
				break;
			case 1: //name
				if (itemSubLen > USER_NAME_LENGTH_EX) 
				{
					goto e_out;
				}
				BYTE szUrName[USER_NAME_LENGTH_EX+1];
				memset(szUrName, 0, USER_NAME_LENGTH_EX+1);
				memcpy(szUrName, buf+itemStPos, itemSubLen);
				Change_Utf8_Assic(szUrName, urItem->name);
				urItem->name[USER_NAME_LENGTH_EX-1] = '\0';
				//memcpy(urItem->name, buf+itemStPos, itemSubLen);
				break;
			case 2: //password
				if (itemSubLen > USER_NAME_LENGTH) 
				{
					goto e_out;
				}
				memcpy(urItem->passWord, buf+itemStPos, itemSubLen);
				break;
			case 3: //group No
				if (itemSubLen > 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				urItem->dwGroupID = atoi(tmp);
				break;
			case 4: //right-code list
				if (itemSubLen > 8*MAX_RIGHT_NUM) 
				{
					goto e_out;
				}
				ret = ParseRightCode(buf+itemStPos, itemSubLen, urItem->rights, &urItem->dwRightNum);
				if (ret < 0)
				{
					goto e_out;
				}
				break;
			case 5:
				if (itemSubLen > MEMO_LENGTH) 
				{
					goto e_out;
				}
				BYTE szDemo[MEMO_LENGTH+1];
				memset(szDemo, 0, MEMO_LENGTH+1);
				memcpy(szDemo, buf+itemStPos, itemSubLen);
				Change_Utf8_Assic(szDemo, urItem->memo);
				urItem->memo[MEMO_LENGTH-1] = '\0';
			//	memcpy(urItem->memo, buf+itemStPos, itemSubLen);
				break;
			default:
				goto e_out;
			}
			itemIndex ++;
			itemPos++;
			itemStPos = itemPos;
			itemSubLen = 0;
		}
	}
	
	if (itemPos != 0 && 5 == itemIndex)
	{
		//没有用户复用选项
		if (itemSubLen > MEMO_LENGTH) 
		{
			goto e_out;
		}
		BYTE szDemo[MEMO_LENGTH+1];
		memset(szDemo, 0, MEMO_LENGTH+1);
		memcpy(szDemo, buf+itemStPos, itemSubLen);
		Change_Utf8_Assic(szDemo, urItem->memo);
		urItem->memo[MEMO_LENGTH-1] = '\0';
	//	memcpy(urItem->memo, buf+itemStPos, itemSubLen);
		urItem->dwFouctionMask = 0;
		*nsp = 0;
	}
	else if(itemPos != 0 && 6 == itemIndex)
	{
		if (itemSubLen > 1) 
		{
			goto e_out;
		}
		urItem->dwFouctionMask = *(buf+itemStPos)=='0' ? 0 : 1;
		*nsp = 1;
	}
	else
	{
		goto e_out;
	}

	return 0;
e_out:
	return -1;
	return 0;
}
*/
///////////parse user item//////////
int ParseUserItem(char *buf, int bufLen, USER_INFO *urItem, void* special)
{
	if (0 == bufLen) 
	{
		return 0;
	}
	
	int *nsp = (int*)special;
	if (NULL == nsp)
	{
		return -1;
	}
	
	int itemPos = 0;
	int itemStPos = 0;
	int itemSubLen = 0;
	int itemIndex = 0;
	char tmp[32] = {0};
	int ret = -1;

	if (!urItem || !buf)
	{
		goto e_out;
	}
	
	while (itemPos < bufLen) 
	{
		if (buf[itemPos] != ':')
		{
			itemPos++;
			itemSubLen++;
		}
		else //get item
		{
			switch(itemIndex)
			{
			case 0: //id
				if (itemSubLen > 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				urItem->dwID = atoi(tmp);
				break;
			case 1: //name
				if (itemSubLen > USER_NAME_LENGTH) 
				{
					goto e_out;
				}
				BYTE szUrName[USER_NAME_LENGTH+1];
				memset(szUrName, 0, USER_NAME_LENGTH+1);
				memcpy(szUrName, buf+itemStPos, itemSubLen);
				Change_Utf8_Assic(szUrName, urItem->name);
				//urItem->name[USER_NAME_LENGTH-1] = '\0';
				//memcpy(urItem->name, buf+itemStPos, itemSubLen);
				break;
			case 2: //password
				if (itemSubLen > USER_PSW_LENGTH) 
				{
					goto e_out;
				}
				memcpy(urItem->passWord, buf+itemStPos, itemSubLen);
				break;
			case 3: //group No
				if (itemSubLen > 32)
				{
					goto e_out;
				}
				memset(tmp, 0, 32);
				memcpy(tmp, buf+itemStPos, itemSubLen);
				urItem->dwGroupID = atoi(tmp);
				break;
			case 4: //right-code list
				if (itemSubLen > 8*MAX_RIGHT_NUM) 
				{
					goto e_out;
				}
				ret = ParseRightCode(buf+itemStPos, itemSubLen, urItem->rights, &urItem->dwRightNum);
				if (ret < 0)
				{
					goto e_out;
				}
				break;
			case 5:
				if (itemSubLen > MEMO_LENGTH) 
				{
					goto e_out;
				}
				BYTE szDemo[MEMO_LENGTH+1];
				memset(szDemo, 0, MEMO_LENGTH+1);
				memcpy(szDemo, buf+itemStPos, itemSubLen);
				Change_Utf8_Assic(szDemo, urItem->memo);
				urItem->memo[MEMO_LENGTH-1] = '\0';
			//	memcpy(urItem->memo, buf+itemStPos, itemSubLen);
				break;
			default:
				goto e_out;
			}
			itemIndex ++;
			itemPos++;
			itemStPos = itemPos;
			itemSubLen = 0;
		}
	}
	
	if (itemPos != 0 && 5 == itemIndex)
	{
		//没有用户复用选项
		if (itemSubLen > MEMO_LENGTH) 
		{
			goto e_out;
		}
		BYTE szDemo[MEMO_LENGTH+1];
		memset(szDemo, 0, MEMO_LENGTH+1);
		memcpy(szDemo, buf+itemStPos, itemSubLen);
		Change_Utf8_Assic(szDemo, urItem->memo);
		urItem->memo[MEMO_LENGTH-1] = '\0';
	//	memcpy(urItem->memo, buf+itemStPos, itemSubLen);
		urItem->dwReusable = 0;
		*nsp = 0;
	}
	else if(itemPos != 0 && 6 == itemIndex)
	{
		if (itemSubLen > 1) 
		{
			goto e_out;
		}
		urItem->dwReusable = *(buf+itemStPos)=='0' ? 0 : 1;
		*nsp = 1;
	}
	else
	{
		goto e_out;
	}

	return 0;
e_out:
	return -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////
/*
int ParseListInfoEx(int listType, char *buf, int bufLen, void *des, DWORD *lstLen,int nMaxLength, void* special)
{
	if(bufLen ==0)
	{
		return 0;
	}
	if(NULL == des || NULL == buf )
	{
		return -1;
	}
	int nsize=0;
	int i;//for for
	int ret;
	OPR_RIGHT_EX *rtList = 0;
	USER_GROUP_INFO_EX *gpList = 0;
	USER_INFO_EX *urList = 0;
	CStrParse parse(buf,"&&");
	parse.setSpliter("&&");
	bool bResult = parse.Parse(buf);
	if(false == bResult)
	{
		return -1;
	}
	nsize =parse.Size();
	*lstLen =nsize;
	char subStr[1024]={0};
	int nstrLen =0;
	for(i=0;i<nsize;i++)
	{
		
		switch(listType)
		{
		case 0:
			{	//parse right list
				if(nsize > MAX_RIGHT_NUM)
				{
					return -1;
				}
				nstrLen = strlen(parse.getWord(i).c_str());
				memset(subStr,0,1024);
				memcpy(subStr,parse.getWord(i).c_str(),nstrLen);
				rtList = (OPR_RIGHT_EX *)des;
				ret = ParseRightItemEx(subStr, nstrLen, &rtList[i],nMaxLength);
				break;
			}
		case 1:
			{	//parse group list
				if(nsize > MAX_GROUP_NUM)
				{
					return -1;
				}
				gpList = (USER_GROUP_INFO_EX *)des;
				nstrLen = strlen(parse.getWord(i).c_str());
				memset(subStr,0,1024);
				memcpy(subStr,parse.getWord(i).c_str(),nstrLen);
				ret = ParseGroupItemEx(subStr, nstrLen, &gpList[i],nMaxLength);
				
				break;
			}
		case 2:
			{	//parse user list
				if(nsize > MAX_USER_NUM)
				{
					return -1;
				}
				urList = (USER_INFO_EX *)des;
				nstrLen = strlen(parse.getWord(i).c_str());
				memset(subStr,0,1024);
				memcpy(subStr,parse.getWord(i).c_str(),nstrLen);
				ret = ParseUserItemEx(subStr, nstrLen, &urList[i], special,nMaxLength);
				
				break;
			}
		default://错误的参数
			return -1;
		}
	}
	return ret;
	
}
*/
/*
int ParseListInfoEx(int listType, char *buf, int bufLen, void *des, DWORD *lstLen, void* special)
{
	if (0 == bufLen) 
	{
		return 0;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	char tmpItem[1024];	//Stores temp item info
	int listIdx = 0;	//Stores output-rightList index
	int ret = -1;
	
	OPR_RIGHT_EX *rtList = 0;
	USER_GROUP_INFO_EX *gpList = 0;
	USER_INFO_EX *urList = 0;

	if (!des || !buf || subLen > 1024)
	{
		goto e_out;
	}

	while (pos < bufLen) 
	{
		if (buf[pos] != '&')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (buf[pos+1] != '&')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				memset(tmpItem, 0, 1024);
				memcpy(tmpItem, buf+startPos, subLen);

				//Parse the item to operation right structure
				switch(listType)
				{
				case 0:	//parse right list
					rtList = (OPR_RIGHT_EX *)des;
					ret = ParseRightItemEx(tmpItem, subLen, &rtList[listIdx]);
					listIdx++; //list index add 1
					if (listIdx >= MAX_RIGHT_NUM)
					{
						goto e_out;
					}
					break;
				case 1:	//parse group list
					gpList = (USER_GROUP_INFO_EX *)des;
					ret = ParseGroupItemEx(tmpItem, subLen, &gpList[listIdx]);
					listIdx++; //list index add 1
					if (listIdx >= MAX_GROUP_NUM)
					{
						goto e_out;
					}
					break;
				case 2:	//parse user list
					urList = (USER_INFO_EX *)des;
					ret = ParseUserItemEx(tmpItem, subLen, &urList[listIdx], special);
					listIdx++; //list index add 1
					if (listIdx >= MAX_USER_NUM)
					{
						goto e_out;
					}
					break;
				default:
					goto e_out;
				}
				if (ret < 0)
				{
					goto e_out;
				}
				pos += 2;
				startPos = pos;
				subLen = 0;
				
			}
		}
	}
	
	if (pos != 0) //the last item
	{
		memset(tmpItem, 0, 1024);
		memcpy(tmpItem, buf+startPos, (subLen)<=1024?subLen:1024);
		
		switch(listType)
		{
		case 0:	//parse right list
			rtList = (OPR_RIGHT_EX *)des;
			ret = ParseRightItemEx(tmpItem, subLen, &rtList[listIdx]);
			break;
		case 1:	//parse group list
			gpList = (USER_GROUP_INFO_EX *)des;
			ret = ParseGroupItemEx(tmpItem, subLen, &gpList[listIdx]);
			break;
		case 2:	//parse user list
			urList = (USER_INFO_EX *)des;
			ret = ParseUserItemEx(tmpItem, subLen, &urList[listIdx], special);
			break;
		default:
			goto e_out;
		}
		if (lstLen)
		{
			*lstLen = listIdx + 1;
		}
	}

	return ret;
e_out:
	return -1;
}
*/
/////////////parse returned userinfo///////////////////
int ParseListInfo(int listType, char *buf, int bufLen, void *des, DWORD *lstLen, void* special)
{
	if (0 == bufLen) 
	{
		return 0;
	}
	

	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	char tmpItem[1024];	//Stores temp item info
	int listIdx = 0;	//Stores output-rightList index
	int ret = -1;
	
	OPR_RIGHT *rtList = 0;
	USER_GROUP_INFO *gpList = 0;
	USER_INFO *urList = 0;

	if (!des || !buf || subLen > 1024)
	{
		goto e_out;
	}

	while (pos < bufLen) 
	{
		if (buf[pos] != '&')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (buf[pos+1] != '&')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				memset(tmpItem, 0, 1024);
				memcpy(tmpItem, buf+startPos, subLen);

				//Parse the item to operation right structure
				switch(listType)
				{
				case 0:	//parse right list
					rtList = (OPR_RIGHT *)des;
					ret = ParseRightItem(tmpItem, subLen, &rtList[listIdx]);
					listIdx++; //list index add 1
					if (listIdx >= MAX_RIGHT_NUM)
					{
						goto e_out;
					}
					break;
				case 1:	//parse group list
					gpList = (USER_GROUP_INFO *)des;
					ret = ParseGroupItem(tmpItem, subLen, &gpList[listIdx]);
					listIdx++; //list index add 1
					if (listIdx >= MAX_GROUP_NUM)
					{
						goto e_out;
					}
					break;
				case 2:	//parse user list
					urList = (USER_INFO *)des;
					ret = ParseUserItem(tmpItem, subLen, &urList[listIdx], special);
					listIdx++; //list index add 1
					if (listIdx >= MAX_USER_NUM)
					{
						goto e_out;
					}
					break;
				default:
					goto e_out;
				}
				if (ret < 0)
				{
					goto e_out;
				}
				pos += 2;
				startPos = pos;
				subLen = 0;
				
			}
		}
	}
	
	if (pos != 0) //the last item
	{
		memset(tmpItem, 0, 1024);
		memcpy(tmpItem, buf+startPos, (subLen)<=1024?subLen:1024);
		
		switch(listType)
		{
		case 0:	//parse right list
			rtList = (OPR_RIGHT *)des;
			ret = ParseRightItem(tmpItem, subLen, &rtList[listIdx]);
			break;
		case 1:	//parse group list
			gpList = (USER_GROUP_INFO *)des;
			ret = ParseGroupItem(tmpItem, subLen, &gpList[listIdx]);
			break;
		case 2:	//parse user list
			urList = (USER_INFO *)des;
			ret = ParseUserItem(tmpItem, subLen, &urList[listIdx], special);
			break;
		default:
			goto e_out;
		}
		if (lstLen)
		{
			*lstLen = listIdx + 1;
		}
	}

	return ret;
e_out:
	return -1;
}

////////////////////////////////////////////////////////////////
//将上层的用户信息结构打成协议包
///////////////Construct right-code string/////////////////
int ConstructRightCode(char *des, DWORD *rights, int rNum, int *len, int maxlen)
{
	if (!des||!rights||!len)
	{
		return -1;
	}

	if (rNum <= 0)
	{
		*len = 0;
		return 0;
	}

	int myPos = 0;
	int mySubLen = 0;
	char temp[8] = {0};
	for (int i = 0; i < rNum - 1; i++)
	{
		itoa(rights[i], temp, 10);
		mySubLen = strlen(temp);
		if (myPos + mySubLen > maxlen)
		{
			return -1;
		}
		memcpy(des + myPos, temp, mySubLen);
		myPos += mySubLen;
		memset(des + myPos, ',', 1);
		myPos++;
	}

	itoa(rights[rNum - 1], temp, 10);
	mySubLen = strlen(temp);
	memcpy(des + myPos, temp, mySubLen);
	myPos += mySubLen;
	
	if (len)
	{
		*len = myPos;
	}

	return 0;
}

////////////Build packet////////////////
int BulidUserInfoBufEx(int type, void *opParam, void *subParam, char *buf, int *buflen, int maxlen, int nMaxLength,void* special)
{
	if (!opParam || !buf || !buflen)
	{
		return -1;
	}

	int pos = 0;
	int subLen = 0;
	char tmp[32] = {0};
	char szBuffer[USER_NAME_LENGTH_EX + MEMO_LENGTH + USER_PSW_LENGTH_EX + 1] = {0};
	char* pUTF8 = NULL;
	switch(type)
	{
	case 0:		//增加用户组
		{
			USER_GROUP_INFO_EX *ugInfo = (USER_GROUP_INFO_EX *)opParam;
			//group Id
			itoa(ugInfo->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//group name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, ugInfo->name, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(ugInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength) 
			{
				delete[] pUTF8;
				return -1;
			}
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//group right list
			int ret = ConstructRightCode(buf + pos, ugInfo->rights, ugInfo->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//group memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, ugInfo->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH) 
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(ugInfo->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 1:		//	3 删除用户组
		{
			//group name
			USER_GROUP_INFO *ugInfo = (USER_GROUP_INFO *)opParam;
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, ugInfo->name, nMaxLength);
			subLen = strlen(szBuffer);

			if (subLen > nMaxLength) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(ugInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength) 
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 2:		//	4 修改用户组
		{
			USER_GROUP_INFO_EX *newGp = (USER_GROUP_INFO_EX *)opParam;
			USER_GROUP_INFO_EX *oldGp = (USER_GROUP_INFO_EX *)subParam;
			if (!subParam)
			{
				return -1;
			}
			
			//old group name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer,oldGp->name,nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(oldGp->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength) 
			{
				delete[] pUTF8;
				return -1;
			}
			
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old group Id
			itoa(oldGp->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new group name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newGp->name, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newGp->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength) 
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new group rights
			int ret = ConstructRightCode(buf + pos, newGp->rights, newGp->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new group memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newGp->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH)
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newGp->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 3:		//	6 增加用户
		{
			USER_INFO_EX *urInfo = (USER_INFO_EX *)opParam;
			//user Id
			itoa(urInfo->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->name, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(urInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength) 
			{
				delete[] pUTF8;
				return -1;
			}
			
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;
			
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user password (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));			
			memcpy(szBuffer, urInfo->passWord, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}
			memcpy(buf + pos, urInfo->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user group Id
			itoa(urInfo->dwGroupID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user right list
			int ret = ConstructRightCode(buf + pos, urInfo->rights, urInfo->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			
			//memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH) 
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(urInfo->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			if ((int)special == 1)
			{
				memset(buf + pos, ':', 1);
				pos++;
				//if reusable
				*(buf + pos) = urInfo->dwFouctionMask ==0 ? '0' : '1';
				pos++;
			}
		}
		break;
	case 4:		//	7 删除用户
		{
			//user name
			USER_INFO_EX *urInfo = (USER_INFO_EX *)opParam;
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->name, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(urInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 5:		//	8 修改用户
		{
			USER_INFO_EX *newInfo = (USER_INFO_EX *)opParam;
			USER_INFO_EX *oldInfo = (USER_INFO_EX *)subParam;
			if (!subParam)
			{
				return -1;
			}

			//old user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldInfo->name, nMaxLength);
			
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(oldInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old user Id 
			itoa(oldInfo->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newInfo->name, nMaxLength);
						
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength)
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old password (encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldInfo->passWord, nMaxLength);
			
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}
			memcpy(buf + pos, oldInfo->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;

			//old group Id
			//itoa(oldInfo->dwGroupID, tmp, 10);
			//new group id modify by cqs (10842) changed user group
			itoa(newInfo->dwGroupID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user right list
			int ret = ConstructRightCode(buf + pos, newInfo->rights, newInfo->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newInfo->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newInfo->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;
			
			pos += subLen;
			if ((int)special == 1)
			{
				memset(buf + pos, ':', 1);
				pos++;
				//if reuseble
				*(buf + pos) = newInfo->dwFouctionMask ==0 ? '0' : '1';
				pos++;
			}
		}
		break;
	case 6:		//	10 修改用户密码
		{
			USER_INFO_EX *newUr = (USER_INFO_EX *)opParam;
			USER_INFO_EX *oldUr = (USER_INFO_EX *)subParam;
			if (!subParam)
			{
				return -1;
			}

			//old user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldUr->name, nMaxLength);
			
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(oldUr->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > nMaxLength)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;
			
			//memcpy(buf + pos, oldUr->name, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old user password (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldUr->passWord, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}
			memcpy(buf + pos, oldUr->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user password (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newUr->passWord, nMaxLength);
					
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}
			memcpy(buf + pos, newUr->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user password again (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newUr->passWord, nMaxLength);
			subLen = strlen(szBuffer);
			if (subLen > nMaxLength) 
			{
				return -1;
			}
			memcpy(buf + pos, newUr->passWord, subLen);
			pos += subLen;
		}
		break;
	default:
		return -1;
	}
	if (buflen)
	{
		*buflen = pos;
	}
	return 0;
}
int BulidUserInfoBuf(int type, void *opParam, void *subParam, char *buf, int *buflen, int maxlen, void* special)
{
	if (!opParam || !buf || !buflen)
	{
		return -1;
	}

	int pos = 0;
	int subLen = 0;
	char tmp[32] = {0};
	char szBuffer[USER_NAME_LENGTH + MEMO_LENGTH + USER_PSW_LENGTH + 1] = {0};
	char* pUTF8 = NULL;
	switch(type)
	{
	case 0:		//增加用户组
		{
			USER_GROUP_INFO *ugInfo = (USER_GROUP_INFO *)opParam;
			//group Id
			itoa(ugInfo->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//group name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, ugInfo->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(ugInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//group right list
			int ret = ConstructRightCode(buf + pos, ugInfo->rights, ugInfo->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//group memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, ugInfo->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH) 
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(ugInfo->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 1:		//	3 删除用户组
		{
			//group name
			USER_GROUP_INFO *ugInfo = (USER_GROUP_INFO *)opParam;
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, ugInfo->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(ugInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 2:		//	4 修改用户组
		{
			USER_GROUP_INFO *newGp = (USER_GROUP_INFO *)opParam;
			USER_GROUP_INFO *oldGp = (USER_GROUP_INFO *)subParam;
			if (!subParam)
			{
				return -1;
			}
			
			//old group name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldGp->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(oldGp->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}
			
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old group Id
			itoa(oldGp->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new group name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newGp->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newGp->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new group rights
			int ret = ConstructRightCode(buf + pos, newGp->rights, newGp->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new group memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newGp->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH)
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newGp->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 3:		//	6 增加用户
		{
			USER_INFO *urInfo = (USER_INFO *)opParam;
			//user Id
			itoa(urInfo->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH) 
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(urInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH) 
			{
				delete[] pUTF8;
				return -1;
			}
			
			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;
			
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user password (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->passWord, USER_PSW_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_PSW_LENGTH) 
			{
				return -1;
			}
			memcpy(buf + pos, urInfo->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user group Id
			itoa(urInfo->dwGroupID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//user right list
			int ret = ConstructRightCode(buf + pos, urInfo->rights, urInfo->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			
			//memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH) 
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(urInfo->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			if ((int)special == 1)
			{
				memset(buf + pos, ':', 1);
				pos++;
				//if reusable
				*(buf + pos) = urInfo->dwReusable==0 ? '0' : '1';
				pos++;
			}
		}
		break;
	case 4:		//	7 删除用户
		{
			//user name
			USER_INFO *urInfo = (USER_INFO *)opParam;
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, urInfo->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(urInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
		}
		break;
	case 5:		//	8 修改用户
		{
			USER_INFO *newInfo = (USER_INFO *)opParam;
			USER_INFO *oldInfo = (USER_INFO *)subParam;
			if (!subParam)
			{
				return -1;
			}

			//old user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldInfo->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(oldInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old user Id 
			itoa(oldInfo->dwID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newInfo->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH)
			{
				return -1;
			}
			
			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newInfo->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;

			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old password (encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldInfo->passWord, USER_PSW_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_PSW_LENGTH) 
			{
				return -1;
			}
			memcpy(buf + pos, oldInfo->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old group Id
			//itoa(oldInfo->dwGroupID, tmp, 10);
			//new group id modify by cqs (10842) changed user group
			itoa(newInfo->dwGroupID, tmp, 10);
			subLen = strlen(tmp);
			memcpy(buf + pos, tmp, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user right list
			int ret = ConstructRightCode(buf + pos, newInfo->rights, newInfo->dwRightNum, &subLen, maxlen - pos);
			if (ret < 0 || pos + subLen > maxlen) 
			{
				return -1;
			}
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user memo
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newInfo->memo, MEMO_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > MEMO_LENGTH)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(newInfo->memo, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > MEMO_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;
			
			pos += subLen;
			if ((int)special == 1)
			{
				memset(buf + pos, ':', 1);
				pos++;
				//if reuseble
				*(buf + pos) = newInfo->dwReusable==0 ? '0' : '1';
				pos++;
			}
		}
		break;
	case 6:		//	10 修改用户密码
		{
			USER_INFO *newUr = (USER_INFO *)opParam;
			USER_INFO *oldUr = (USER_INFO *)subParam;
			if (!subParam)
			{
				return -1;
			}

			//old user name
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldUr->name, USER_NAME_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_NAME_LENGTH)
			{
				return -1;
			}

			pUTF8 = new char[subLen*2+2];
			if (NULL == pUTF8)
			{
				return -1;
			}
			memset(pUTF8, 0 , subLen*2+2);
			Change_Assic_UTF8(oldUr->name, subLen, pUTF8, subLen*2+2);
			subLen = strlen(pUTF8);
			if (subLen > USER_NAME_LENGTH)
			{
				delete[] pUTF8;
				return -1;
			}

			memcpy(buf + pos, pUTF8, subLen);
			delete[] pUTF8;
			
			//memcpy(buf + pos, oldUr->name, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//old user password (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, oldUr->passWord, USER_PSW_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_PSW_LENGTH) 
			{
				return -1;
			}
			memcpy(buf + pos, oldUr->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user password (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newUr->passWord, USER_PSW_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_PSW_LENGTH) 
			{
				return -1;
			}
			memcpy(buf + pos, newUr->passWord, subLen);
			pos += subLen;
			memset(buf + pos, ':', 1);
			pos++;
			//new user password again (not encrypted)
			memset(szBuffer, 0, sizeof(szBuffer));
			memcpy(szBuffer, newUr->passWord, USER_PSW_LENGTH);
			subLen = strlen(szBuffer);
			if (subLen > USER_PSW_LENGTH) 
			{
				return -1;
			}
			memcpy(buf + pos, newUr->passWord, subLen);
			pos += subLen;
		}
		break;
	default:
		return -1;
	}
	if (buflen)
	{
		*buflen = pos;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//NET_TIME结构处理函数
/*
 * override the operator >=
 */
bool operator>= (const NET_TIME& lhs, const NET_TIME& rhs)
{
#ifdef WIN32
	SYSTEMTIME lst, rst;
	memset(&lst, 0x00, sizeof(SYSTEMTIME));
	memset(&rst, 0x00, sizeof(SYSTEMTIME));

	lst.wYear = lhs.dwYear;
	lst.wMonth = lhs.dwMonth;
	lst.wDay = lhs.dwDay;
	lst.wHour = lhs.dwHour;
	lst.wMinute = lhs.dwMinute;
	lst.wSecond = lhs.dwSecond;

	rst.wYear = rhs.dwYear;
	rst.wMonth = rhs.dwMonth;
	rst.wDay = rhs.dwDay;
	rst.wHour = rhs.dwHour;
	rst.wMinute = rhs.dwMinute;
	rst.wSecond = rhs.dwSecond;
	
	FILETIME lft, rft;
	
	BOOL b = SystemTimeToFileTime(&lst, &lft);
#ifdef _DEBUG
	if (!b)
	{
		OutputDebugString("translate systemtime to filetime failed\n");
	}
#endif
	
	b = SystemTimeToFileTime(&rst, &rft);
#ifdef _DEBUG
	if (!b)
	{
		OutputDebugString("translate systemtime to filetime failed\n");
	}
#endif
	
	int nc = CompareFileTime( &lft, &rft );
	if (nc >= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
#else	//linux
	int ldays = lhs.dwYear*10000 + lhs.dwMonth*100 + lhs.dwDay;
	int rdays = rhs.dwYear*10000 + rhs.dwMonth*100 + rhs.dwDay;
	if (ldays < rdays)
	{
		return false;
	}
	else if (ldays > rdays)
	{
		return true;
	}
	else if (ldays == rdays)
	{
		int lsecs = lhs.dwHour*10000 + lhs.dwMinute*100 + lhs.dwSecond;
		int rsecs = rhs.dwHour*10000 + rhs.dwMinute*100 + rhs.dwSecond;
		if (lsecs < rsecs)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
#endif
}

/*
 * override the operator <=
 */
bool operator<= (const NET_TIME& lhs, const NET_TIME& rhs)
{
	return operator>=(rhs, lhs);
}

bool operator>= (const FRAME_TIME& lhs, const FRAME_TIME& rhs)
{
	NET_TIME stuLeftTime = {0};
	NET_TIME stuRightTime = {0};

	stuLeftTime.dwYear = lhs.year + 2000;
	stuLeftTime.dwMonth = lhs.month;
	stuLeftTime.dwDay = lhs.day;
	stuLeftTime.dwHour = lhs.hour;
	stuLeftTime.dwMinute = lhs.minute;
	stuLeftTime.dwSecond = lhs.second;
	stuRightTime.dwYear = rhs.year + 2000;
	stuRightTime.dwMonth = rhs.month;
	stuRightTime.dwDay = rhs.day;
	stuRightTime.dwHour = rhs.hour;
	stuRightTime.dwMinute = rhs.minute;
	stuRightTime.dwSecond = rhs.second;

	if (stuLeftTime.dwHour == 24)
	{
		stuLeftTime.dwHour = 23;
		stuLeftTime.dwMinute = 59;
		stuLeftTime.dwSecond = 59;
	}
	if (stuRightTime.dwHour == 24)
	{
		stuRightTime.dwHour = 23;
		stuRightTime.dwMinute = 59;
		stuRightTime.dwSecond = 59;
	}

	return operator>=(stuLeftTime, stuRightTime);
}

DWORD GetMonthDays(const DWORD& dwYear, const DWORD& dwMonth)
{
	DWORD dwMDays = 0;
	switch(dwMonth)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		dwMDays = 31;
		break;
	case 2:
		{
			if (((dwYear%4==0)&& dwYear%100) || (dwYear%400==0))
			{
				dwMDays = 29; 
			}
			else
			{
				dwMDays = 28;
			}
		}
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		dwMDays = 30;
		break;
	default:
		break;
	} 
	
	return dwMDays;
}

/*
 *	can't surpport year offset time
 */
/*
 *	yehao(10857) 2006-12-25:以前的版本只有在et不小于st的情况下算出来的值才是正确的，否则就不行了。在实现的时候如果et大于等于
 *	st就需要交换两者的值，所以这里不能使常量型引用：DWORD GetOffsetTime(const NET_TIME& st, const NET_TIME& et)。
 */
DWORD GetOffsetTime(NET_TIME st, NET_TIME et)
{
	DWORD dwRet = -1; 
	if (et.dwYear != st.dwYear)
	{
		return dwRet;
	}

	int swaped = 1;
	if (st >= et)
	{
		NET_TIME tmp = st;
		st = et;
		et = tmp;
		swaped = -1;
	}
	DWORD dwDays = 0;

	for(int i=st.dwMonth+1; i < et.dwMonth; ++i)
	{
		dwDays += GetMonthDays(st.dwYear, i);
	}

	if (st.dwMonth == et.dwMonth)
	{
		dwDays = (et.dwDay - st.dwDay);
	}
	else
	{
		dwDays += et.dwDay;
		dwDays += GetMonthDays(st.dwYear, st.dwMonth) - st.dwDay;
	}
	
	dwRet = dwDays*24*60*60 + ((int)et.dwHour - (int)st.dwHour)*60*60 + 
		((int)et.dwMinute - (int)st.dwMinute)*60 + (int)et.dwSecond - (int)st.dwSecond;
	
	return dwRet*swaped;
}

 
/*
 * can't surpport year offset time
 */
NET_TIME GetSeekTimeByOffsetTime(const NET_TIME& bgtime, unsigned int offsettime)
{
	NET_TIME tmseek;
	memset(&tmseek, 0x00, sizeof(NET_TIME));
 
	DWORD dwNext = 0;
	//second
	tmseek.dwSecond = (bgtime.dwSecond+offsettime)%60;
	dwNext = (bgtime.dwSecond+offsettime)/60;
	//minute
	tmseek.dwMinute = (dwNext+bgtime.dwMinute)%60;
	dwNext = (dwNext+bgtime.dwMinute)/60;
	//hour
	tmseek.dwHour = (dwNext+bgtime.dwHour)%24;
	dwNext = (dwNext+bgtime.dwHour)/24;

	DWORD curMDays = GetMonthDays(bgtime.dwYear, bgtime.dwMonth);
	DWORD dwLeftDays = dwNext += bgtime.dwDay;
	tmseek.dwMonth = bgtime.dwMonth;
	tmseek.dwYear = bgtime.dwYear;

	while (dwLeftDays > curMDays)
	{
		tmseek.dwMonth++;
		dwLeftDays -= curMDays;
		curMDays = GetMonthDays(tmseek.dwYear, tmseek.dwMonth);
	}
	tmseek.dwDay = dwLeftDays;
	//跨一年暂不支持
	if (tmseek.dwMonth > 12)
	{
		tmseek.dwYear++;
		tmseek.dwMonth -= 12;
	}
	return tmseek;
}


//////////////////////////////////////////////////////////////////////////
//写视频文件头
void  WriteVideoFileHeader(FILE *file, int nType, int nFrameRate, 
                           int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
	if (!file)
	{
		return;
	}
    typedef struct 
    {
		DWORD second		:6;				/*	秒	1-60		*/
		DWORD minute		:6;				/*	分	1-60		*/
		DWORD hour			:5;				/*	时	1-24		*/
		DWORD day			:5;				/*	日	1-31		*/
		DWORD month			:4;				/*	月	1-12		*/
		DWORD year			:6;				/*	年	2001-2064	*/
    } yw_time;
    //视频头文件信息
	typedef struct
    {
		BYTE	CorpInfo[6];		//公司信息
		BYTE    VideoMode;			//视频类型   MPEG1-MPEG4
		BYTE    Framerate;			//帧率       
		yw_time	DirStartTime;		//开始时间
		yw_time	DirEndTime;			//结束时间
    } yw_videofileheader;
	
    yw_videofileheader videofileheader;
    memset(&videofileheader, 0, sizeof(yw_videofileheader));
	
	memcpy(&videofileheader.CorpInfo, "DAHUA", 6);
	if(nType == PRODUCT_DVR_MEPG4_SH2)
	{
		videofileheader.CorpInfo[5] = 0x01;
	}
	else
    {
        videofileheader.CorpInfo[5] = 0x11;
    }
	videofileheader.Framerate = nFrameRate;
    videofileheader.VideoMode = 4;//(m_nIsCurVideoMode == MPEG1_MODE) ? 1 : 4;
	
    videofileheader.DirStartTime.year = (nYear)%100;
	videofileheader.DirStartTime.month = nMonth;
	videofileheader.DirStartTime.day = nDay;
	videofileheader.DirStartTime.hour = nHour;
	videofileheader.DirStartTime.minute = nMinute;
	videofileheader.DirStartTime.second = nSecond;
	
    fwrite(&videofileheader, 1, sizeof(yw_videofileheader), file);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/**
 * split_str - 
 * @: 
 * @: 
 *
 * find func based strstr, it find from left
 */
int split_str(
    const char * const src,
    char *left,
    char *right,
    const char * const spliter,
    size_t src_len)
{
	char *p;
	size_t real_len;

	/* check input*/
	if (!src || !*src || !left || !right)
	{
		return -1000;
	}

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	strncpy(right, src, src_len);
	right[src_len] = '\0';

	/* get right pos */
	p = strstr(right, spliter);
	/* not found spliter */
	if (!p)
	{
#if 0
		// ret error here
		*left = '\0';
		*right = '\0';
		return -1;
#else
		// we load src to lefp
		// ret ok here
		strcpy(left, src);
		*right = '\0';
		return 0;
#endif

	}
	/* left */
	strncpy(left, right, p-right);
	left[p-right] = '\0';
	/* right */
	strcpy(right, p+strlen(spliter));

	return 0;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

size_t trim_blank(char *src, size_t src_len)
{
	size_t pass_len;
	size_t real_len;
	char *p = src;

	/* check input*/
	if (!src || !*src)
		return 0;

	real_len = strlen(src);
	src_len = src_len==0 ? real_len : src_len;
	src_len = src_len>real_len ? real_len : src_len;

	while ((*p != '\0')
	       && ( (*p == ' ')
	            || (*p == '\t')
	            || (*p == '\r')
	            || (*p == '\n') ) )
	{
		p++;
	}

	pass_len = p-src;
	src_len = src_len-pass_len;

	memmove(src, p, src_len);
	src[src_len] = '\0';

	p = src + src_len - 1;
	while (p >= src)
	{
		if ( (*p == ' ')
		     || (*p == '\t')
		     || (*p == '\r')
		     || (*p == '\n') )
		{
			*p-- = 0;
			pass_len++;
		}
		else
		{
			break;
		}
	}

	return (size_t)(pass_len);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

//////////////////////////////////////////////////////////////////////////
//新的通道名协议

int ParseChannelNameBuf(char* src, int srclen, char* des, int deslen)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int listIdx = 0;	//Stores output-rightList index
	int ret = 0;

	if (!des || !src)
	{
		goto e_out;
	}

	while (pos < srclen) 
	{
		if (src[pos] != '&')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (src[pos+1] != '&')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				int length = subLen > 64 ? 64 : subLen;
				
				if (deslen >= 64*(listIdx+1))
				{
					memcpy(des+64*listIdx, src+startPos, length);
					*(des+64*(listIdx+1)-1) = '\0';
					listIdx++;

					pos += 2;
					startPos = pos;
					subLen = 0;
					ret = listIdx;
				}
				else
				{
					return ret;
				}
			}
		}
	}
	
	if (pos != 0) //the last item
	{
		int length = subLen > 64 ? 64 : subLen;

		if (deslen >= 64*(listIdx+1))
		{
			memcpy(des+64*listIdx, src+startPos, length);
			ret++;
		}
		else
		{
			return ret;
		}
	}

	return ret;
e_out:
	return -1;
}

/*
int BuildChannelNameBuf(char* src, int srclen, char* des, int* deslen)
{
	int ret = 0;

	int pos = 0; //pointer for des writing

	if (NULL == src || NULL == des || NULL == deslen) 
	{
		return -1;
	}
	
	int doTimes = srclen/CHAN_NAME_LEN;
	for (int i=0; i<doTimes; i++)
	{
		int length = strlen(src+CHAN_NAME_LEN*i);
		length = length>CHAN_NAME_LEN ? CHAN_NAME_LEN : length;

		if (pos+length+2 > *deslen)
		{
#ifdef DEBUG
			OutputDebugString("WARNING BY LJY : TOO LONG CHANNEL NAME FOR BLIUD!\n");
#endif
			ret = -1;
			break;
		}

		memcpy(des+pos, src+CHAN_NAME_LEN*i, length);
		pos += length;
		memset(des+pos, '&', 2);
		pos += 2;
	}

	*deslen = pos-2; //最后两个'&'号不要

	return ret;
}
*/
int ParseVersionString(char* src, int srclen, DWORD* des)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int listIdx = 0;	//Stores output-rightList index
	int ret = 0;
	char tmp[32] = {0};
	
	if (!des || !src)
	{
		goto e_out;
	}

	*des = 0;
	
	while (pos < srclen) 
	{
		if (src[pos] != '.')
		{
			pos++;
			subLen++;
		}
		else
		{
			switch(listIdx)
			{
			case 0:
				{
					//main version
					if (listIdx >= 1)
					{
						goto e_out;
					}
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					//int iii = atoi(tmp);
					*des |= atoi(tmp);
					*des = *des << 16;
				}
				break;
			case 1:
				{
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					*des |= atoi(tmp);
					
					//后续内容不处理
					return 0;
				}
				break;
			default:
				return -1;
				break;
			}
			
			pos ++;
			startPos = pos;
			subLen = 0;
			listIdx++;
		}
	}
	
	if (1 == listIdx) //the last item
	{
		if (subLen >= 32)
		{
			goto e_out;
		}
		memset(tmp, 0, 32);
		memcpy(tmp, src+startPos, subLen);
		*des |= atoi(tmp);
	}
	
	return ret;
e_out:
	return -1;
}

//Begin: Modify by li_deming(11517) 2008-2-20
//int ParseMultiDdnsString(char* src, int srclen, DEV_MULTI_DDNS_CFG* des)
int ParseMultiDdnsString(char* src, int srclen, DEV_MULTI_DDNS_CFG* des,DWORD* dwsign)
//End:li_deming(11517)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int ddnsIdx = 0;	//Stores item index
	int ret = 0;
	
	if (!des || !src)
	{
		goto e_out;
	}

	memset(des, 0, sizeof(DEV_MULTI_DDNS_CFG));
	des->dwSize = sizeof(DEV_MULTI_DDNS_CFG);
	
	while (pos < srclen) 
	{
		if (src[pos] != ':')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (src[pos+1] != ':')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				//Begin: Modify by li_deming(11517) 2008-2-30				
				//ret = ParseDdnsItem(src+startPos, subLen, &des->struDdnsServer[ddnsIdx]);
				ret = ParseDdnsItem(src+startPos, subLen, &des->struDdnsServer[ddnsIdx],dwsign);
				//End:li_deming(11517)
				if (ret < 0)
				{
					goto e_out;
				}
				des->dwDdnsServerNum++;

				ddnsIdx++;
				if (ddnsIdx >= 10)
				{
#ifdef DEBUG
					OutputDebugString("WARNING BY LJY : TOO LONG CHANNEL NAMES FOR PARSE!\n")
#endif
						return 0;
				}
				
				pos += 2;
				startPos = pos;
				subLen = 0;
			}
		}
	}
	
	if (pos != 0) //the last item
	{
		//Begin: Modify by li_deming(11517) 2008-1-30
		//ret = ParseDdnsItem(src+startPos, subLen, &des->struDdnsServer[ddnsIdx]);
		ret = ParseDdnsItem(src+startPos, subLen, &des->struDdnsServer[ddnsIdx],dwsign);
		//End:li_deming(11517)
		if (ret < 0)
		{
			goto e_out;
		}
		des->dwDdnsServerNum++;
	}
	
	return ret;
e_out:
	return -1;
}
//Begin: Modify by li_deming(11517) 2008-2-20
//int ParseDdnsItem(char* src, int srclen, DDNS_SERVER_CFG* des)
int ParseDdnsItem(char* src, int srclen, DDNS_SERVER_CFG* des,DWORD* dwsign)
//End:li_deming(11517)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int itemIdx = 0;	//Stores item index
	int ret = 0;
	char tmp[32] = {0};
	//Begin: Add by li_deming(11517) 2008-2-20
	//兼容老的IPC协议处理，此处如果是0则在写配置信息的时候不拼接保活时间的配置信息
	//详细内容请参见保存配置信息
	*dwsign=0x0;
	//End:li_deming(11517)	
	if (!des || !src)
	{
		goto e_out;
	}
	
	while (pos < srclen) 
	{
		if (src[pos] != '&')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (src[pos+1] != '&')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				switch(itemIdx)
				{
				case 0:	//id
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->dwId = atoi(tmp);
					break;
				case 1:	//enable
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->bEnable = (atoi(tmp) == 0) ? FALSE : TRUE;
					break;
				case 2:	//server type
					if (subLen >= MAX_SERVER_TYPE_LEN)
					{
						goto e_out;
					}
					memcpy(des->szServerType, src+startPos, subLen);
					break;
				case 3:	//server ip
					if (subLen >= MAX_DOMAIN_NAME_LEN)
					{
						goto e_out;
					}
					memcpy(des->szServerIp, src+startPos, subLen);
					break;
				case 4:	//server port
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->dwServerPort = atoi(tmp);
					break;
				case 5:	//domain name
					{
						unsigned char *szBufUtf8 = new unsigned char[subLen+1];
						if(NULL == szBufUtf8)
						{
							goto e_out;
						}
						char *szBufUnicode = new char[subLen+1];
						if(NULL == szBufUnicode)
						{
							goto e_out;
						}

						memset(szBufUtf8, 0 , subLen+1);
						memcpy(szBufUtf8, src+startPos, subLen);
						memset(szBufUnicode, 0, subLen+1);

						Change_Utf8_Assic(szBufUtf8, szBufUnicode);
						int nStrLen = strlen(szBufUnicode);
						if( nStrLen >= MAX_DOMAIN_NAME_LEN)
						{
							goto e_out;
						}
						memcpy(des->szDomainName, szBufUnicode, nStrLen);

						delete []szBufUtf8;
						delete []szBufUnicode;
						szBufUtf8 = NULL;
						szBufUnicode = NULL;
						
//						if (subLen >= MAX_DOMAIN_NAME_LEN)
//						{
//							goto e_out;
//						}
//						unsigned char szDomainName[MAX_DOMAIN_NAME_LEN+1];
//						memset(szDomainName, 0, MAX_DOMAIN_NAME_LEN+1);
//						memcpy(szDomainName, src+startPos, subLen);
//						Change_Utf8_Assic(szDomainName, des->szDomainName);
//						des->szDomainName[MAX_DOMAIN_NAME_LEN-1] = '\0';
					}
					break;
				case 6:	//user name
					if (subLen >= MAX_HOSTNAME_LEN)
					{
						goto e_out;
					}

					unsigned char szUserName[MAX_HOSTNAME_LEN+1];
					memset(szUserName, 0, MAX_HOSTNAME_LEN+1);
					memcpy(szUserName, src+startPos, subLen);
					Change_Utf8_Assic(szUserName, des->szUserName);
					des->szUserName[MAX_HOSTNAME_LEN-1] = '\0';
					break;
				case 7:	//password
					if (subLen >= MAX_HOSTPSW_LEN)
					{
						goto e_out;
					}
					memcpy(des->szUserPsw, src+startPos, subLen);
					break;
				//Begin: Add by li_deming(11517) 2008-1-30
				//老的DDNS配置协议中没有第8个&&符号
				//此处对老的DDNS配置协议无效
				case 8://alias
					if (subLen >= MAX_DDNS_ALIAS_LEN )
					{
						goto e_out;
					}
					memcpy(des->szAlias, src+startPos, subLen);					
					break;
				//End:li_deming(11517)					
				default:
					goto e_out;
					break;
				}
				itemIdx++;
				pos += 2;
				startPos = pos;
				subLen = 0;
			}
		}
	}
	//Begin: Add by li_deming(11517) 2008-1-30
	//新的DDNS配置协议
	if(9==itemIdx)
	{		
		memset(tmp, 0, 32);
		memcpy(tmp, src+startPos, subLen);
		des->dwAlivePeriod=atoi(tmp);
		*dwsign=0x01;
	}
	//return ret;
	//End:li_deming(11517)
	
	if (8 == itemIdx) //the last item, alias
	{
		if (subLen >= MAX_DDNS_ALIAS_LEN)
		{
			subLen = MAX_DDNS_ALIAS_LEN - 1;
		}
		memcpy(des->szAlias, src+startPos, subLen);
	}
	
	return ret;
e_out:
	return -1;
}
//Begin: Modify by li_deming(11517) 2008-2-20
//int BuildMultiDdnsString(DEV_MULTI_DDNS_CFG* src, char* des, int* deslen)
int BuildMultiDdnsString(DEV_MULTI_DDNS_CFG* src, char* des, int* deslen,DWORD dwsign)
//End:li_deming(11517)
{
	int ret = 0;
	
	int pos = 0; //pointer for des writing
	
	if (NULL == src || NULL == des || NULL == deslen) 
	{
		return -1;
	}
	
	int tmplen = 400;	//极端情况下，一个ddns字符串也不会达到400
	char* tmpbuf = new char[tmplen];
	if (NULL == tmpbuf)
	{
		return -1;
	}

	for (int i=0; i<src->dwDdnsServerNum; i++)
	{
		memset(tmpbuf, 0, tmplen);
		int tmpret = 400;
		//Begin: Modify by li_deming(11517) 2008-2-20		
		//ret = BuildDdnsItem(&src->struDdnsServer[i], tmpbuf, &tmpret);
		ret = BuildDdnsItem(&src->struDdnsServer[i], tmpbuf, &tmpret,dwsign);			
		//End:li_deming(11517)
		if (ret < 0 || (pos+tmpret+2) > (*deslen))
		{
#ifdef DEBUG
			OutputDebugString("WARNING BY LJY : TOO SHORT BUFFER FOR DDNS STRING!\n");
#endif
			ret = -1;
			break;
		}
		memcpy(des+pos, tmpbuf, tmpret);
		pos += tmpret;
		memset(des+pos, ':', 2);
		pos += 2;
	}
	
	*deslen = pos-2; //最后两个'&'号不要
	
	return ret;
}
//Begin: Modify by li_deming(11517) 2008-1-30
//int BuildDdnsItem(DDNS_SERVER_CFG* src, char* des, int* deslen)
int BuildDdnsItem(DDNS_SERVER_CFG* src, char* des, int* deslen,DWORD dwsign)
//End:li_deming(11517)
{
	if (NULL == src || NULL == des || NULL == deslen) 
	{
		return -1;
	}
	
	int ret = 0;
	
	int pos = 0;
	int subLen = 0;
	char tmp[32] = {0};
	
	// id
	itoa(src->dwId, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//enable
	itoa(src->bEnable ? 1 : 0, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//server type
	subLen = (strlen(src->szServerType)>MAX_SERVER_TYPE_LEN) ? MAX_SERVER_TYPE_LEN : strlen(src->szServerType);
	memcpy(des + pos, src->szServerType, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//server ip
	subLen = (strlen(src->szServerIp)>MAX_DOMAIN_NAME_LEN) ? MAX_DOMAIN_NAME_LEN : strlen(src->szServerIp);
	memcpy(des + pos, src->szServerIp, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//server port
	itoa(src->dwServerPort, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//domain name
	subLen = (strlen(src->szDomainName)>MAX_DOMAIN_NAME_LEN) ? MAX_DOMAIN_NAME_LEN : strlen(src->szDomainName);
	char szDomainNameUTF8[MAX_DOMAIN_NAME_LEN*2+2] = {0};
	Change_Assic_UTF8(src->szDomainName, subLen, szDomainNameUTF8, MAX_DOMAIN_NAME_LEN*2+2);
	subLen = strlen(szDomainNameUTF8);
	memcpy(des + pos, szDomainNameUTF8, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//user name
	subLen = (strlen(src->szUserName)>MAX_HOSTNAME_LEN) ? MAX_HOSTNAME_LEN : strlen(src->szUserName);
	char szUTF8[MAX_HOSTNAME_LEN*2+2] = {0};
	Change_Assic_UTF8(src->szUserName, subLen, szUTF8, MAX_HOSTNAME_LEN*2+2);
	subLen = strlen(szUTF8);
	memcpy(des + pos, szUTF8, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//user password
	subLen = (strlen(src->szUserPsw)>MAX_HOSTPSW_LEN) ? MAX_HOSTPSW_LEN : strlen(src->szUserPsw);
	memcpy(des + pos, src->szUserPsw, subLen);
	pos += subLen;
	memset(des + pos, '&', 2);
	pos += 2;

	//alias
	subLen = (strlen(src->szAlias)>MAX_DDNS_ALIAS_LEN) ? MAX_DDNS_ALIAS_LEN : strlen(src->szAlias);
	memcpy(des + pos, src->szAlias, subLen);
	pos += subLen;

	//Begin: Add by li_deming(11517) 2008-1-30
	//alivePeriod保活时间	
	if(dwsign != 0)//含保活时间的多DDNS配置
	{
		memset(des + pos, '&', 2);
		pos += 2;
		itoa(src->dwAlivePeriod, tmp,10);
		subLen = strlen(tmp);
		memcpy(des + pos, tmp, subLen);
		pos += subLen;	
	}	
	//End:li_deming(11517)
	
	*deslen = pos;
		
	return ret;
}

int	ParseUrlCfgString(char* src, int srclen, DEVWEB_URL_CFG* des)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int itemIdx = 0;	//Stores item index
	int ret = 0;
	char tmp[32] = {0};
	
	if (!des || !src)
	{
		goto e_out;
	}
	
	while (pos < srclen) 
	{
		if (src[pos] != ':')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (src[pos+1] != ':')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				switch(itemIdx)
				{
				case 0:	//enable
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->bSnapEnable = (atoi(tmp) == 0) ? FALSE : TRUE;
					break;
				case 1:	//snap interval
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->iSnapInterval = atoi(tmp);
					break;
				case 2:	//host ip
					if (subLen >= MAX_IP_ADDRESS_LEN)
					{
						goto e_out;
					}
					memcpy(des->szHostIp, src+startPos, subLen);
					break;
				case 3:	//host port
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->wHostPort = atoi(tmp);
					break;
				case 4:	//msg interval
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->iMsgInterval = atoi(tmp);
					break;
				case 5:	//url for state 
					if (subLen >= MAX_URL_LEN)
					{
						goto e_out;
					}
					memcpy(des->szUrlState, src+startPos, subLen);
					break;
				case 6:	//url for image 
					if (subLen >= MAX_URL_LEN)
					{
						goto e_out;
					}
					memcpy(des->szUrlImage, src+startPos, subLen);
					break;
				case 7:	//user name, not use for now
					break;
				case 8:	//user password, not use for now
					break;
				default:
					goto e_out;
					break;
				}
				itemIdx++;
				pos += 2;
				startPos = pos;
				subLen = 0;
			}
		}
	}
	
	if (9 == itemIdx) //the last item, devid
	{
		if (subLen >= MAX_DEV_ID_LEN)
		{
			subLen = MAX_DEV_ID_LEN - 1;
		}
		memcpy(des->szDevId, src+startPos, subLen);
	}
	
	return ret;
e_out:
	return -1;
}

int BuildUrlCfgString(DEVWEB_URL_CFG* src, char* des, int* deslen)
{
	if (NULL == src || NULL == des || NULL == deslen) 
	{
		return -1;
	}
	
	int ret = 0;
	
	int pos = 0;
	int subLen = 0;
	char tmp[32] = {0};
	
	// enable
	itoa(src->bSnapEnable? 1 : 0, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//snap interval
	itoa(src->iSnapInterval, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//host ip
	subLen = (strlen(src->szHostIp)>MAX_IP_ADDRESS_LEN) ? MAX_IP_ADDRESS_LEN : strlen(src->szHostIp);
	memcpy(des + pos, src->szHostIp, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//host port
	itoa(src->wHostPort, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//msg interval
	itoa(src->iMsgInterval, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//url state
	subLen = (strlen(src->szUrlState)>MAX_URL_LEN) ? MAX_URL_LEN : strlen(src->szUrlState);
	memcpy(des + pos, src->szUrlState, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//url image
	subLen = (strlen(src->szUrlImage)>MAX_URL_LEN) ? MAX_URL_LEN : strlen(src->szUrlImage);
	memcpy(des + pos, src->szUrlImage, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//user name, not used
//	subLen = (strlen(src->)>MAX_HOSTPSW_LEN) ? MAX_HOSTPSW_LEN : strlen(src->szUserPsw);
//	memcpy(des + pos, src->szUserPsw, subLen);
//	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;

	//user password, not used
	memset(des + pos, ':', 2);
	pos += 2;

	//device id
	subLen = (strlen(src->szDevId)>MAX_DEV_ID_LEN) ? MAX_DEV_ID_LEN : strlen(src->szDevId);
	memcpy(des + pos, src->szDevId, subLen);
	pos += subLen;
	
	*deslen = pos;
	
	return ret;
}


int	ParseTransStrategyCfgString(char* src, int srclen, DEV_TRANSFER_STRATEGY_CFG* des)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int itemIdx = 0;	//Stores item index
	int ret = 0;
	char tmp[32] = {0};
	
	if (!des || !src)
	{
		goto e_out;
	}
	
	while (pos < srclen) 
	{
		if (src[pos] != ':')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (src[pos+1] != ':')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				switch(itemIdx)
				{
				case 0:	//enable
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->bEnable = (atoi(tmp) == 0) ? FALSE : TRUE;
					break;
				case 1:	//strategy
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					if (_stricmp(tmp, STR_STRATEGY_IMAGE_FIRST) == 0)
					{
						des->iStrategy = 0;
					}
					else if (_stricmp(tmp, STR_STRATEGY_FLUENCY_FIRST) == 0)
					{
						des->iStrategy = 1;
					}
					else if (_stricmp(tmp, STR_STRATEGY_AUTO) == 0)
					{
						des->iStrategy = 2;
					}
					break;
				default:
					goto out;
					break;
				}
				itemIdx++;
				pos += 2;
				startPos = pos;
				subLen = 0;
			}
		}
	}
	
	if (1 == itemIdx) //the last item, strategy
	{
		if (subLen >= 32)
		{
			goto e_out;
		}
		memset(tmp, 0, 32);
		memcpy(tmp, src+startPos, subLen);
		if (_stricmp(tmp, STR_STRATEGY_IMAGE_FIRST) == 0)
		{
			des->iStrategy = 0;
		}
		else if (_stricmp(tmp, STR_STRATEGY_FLUENCY_FIRST) == 0)
		{
			des->iStrategy = 1;
		}
		else if (_stricmp(tmp, STR_STRATEGY_AUTO) == 0)
		{
			des->iStrategy = 2;
		}
	}

out:
	return ret;
e_out:
	return -1;
}

int BuildTransStrategyCfgString(DEV_TRANSFER_STRATEGY_CFG* src, char* des, int* deslen)
{
	if (NULL == src || NULL == des || NULL == deslen) 
	{
		return -1;
	}
	
	int ret = 0;
	
	int pos = 0;
	int subLen = 0;
	char tmp[32] = {0};
	
	// enable
	itoa(src->bEnable? 1 : 0, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
	memset(des + pos, ':', 2);
	pos += 2;
	
	//strategy
	if (1 == src->iStrategy)
	{
		subLen = strlen(STR_STRATEGY_FLUENCY_FIRST);
		memcpy(des + pos, STR_STRATEGY_FLUENCY_FIRST, subLen);
		pos += subLen;
	}
	else if (2 == src->iStrategy)
	{
		subLen = strlen(STR_STRATEGY_AUTO);
		memcpy(des + pos, STR_STRATEGY_AUTO, subLen);
		pos += subLen;
	}
	else
	{
		subLen = strlen(STR_STRATEGY_IMAGE_FIRST);
		memcpy(des + pos, STR_STRATEGY_IMAGE_FIRST, subLen);
		pos += subLen;
	}
		
	*deslen = pos;
	
	return ret;
}

int	ParseDownloadStrategyCfgString(char* src, int srclen, DEV_DOWNLOAD_STRATEGY_CFG* des)
{
	if (0 == srclen) 
	{
		return -1;
	}
	
	int pos = 0;		//Real time pointer
	int startPos = 0;	//Start points
	int subLen = 0;		//Length of the sub item
	int itemIdx = 0;	//Stores item index
	int ret = 0;
	char tmp[32] = {0};
	
	if (!des || !src)
	{
		goto e_out;
	}
	
	while (pos < srclen) 
	{
		if (src[pos] != ':')
		{
			pos++;
			subLen++;
		}
		else
		{
			if (src[pos+1] != ':')
			{
				pos++;
				subLen++;
			}
			else //end of a new item
			{
				switch(itemIdx)
				{
				case 0:	//enable
					if (subLen >= 32)
					{
						goto e_out;
					}
					memset(tmp, 0, 32);
					memcpy(tmp, src+startPos, subLen);
					des->bEnable = (atoi(tmp) == 0) ? FALSE : TRUE;
					break;
				default:
					goto out;
					break;
				}
				itemIdx++;
				pos += 2;
				startPos = pos;
				subLen = 0;
			}
		}
	}
	
	if (0 == itemIdx) //the last item, enable
	{
		if (subLen >= 32)
		{
			goto e_out;
		}
		memset(tmp, 0, 32);
		memcpy(tmp, src+startPos, subLen);
		des->bEnable = (atoi(tmp) == 0) ? FALSE : TRUE;
	}
	
out:
	return ret;
e_out:
	return -1;
}

int BuildDownloadStrategyCfgString(DEV_DOWNLOAD_STRATEGY_CFG* src, char* des, int* deslen)
{
	if (NULL == src || NULL == des || NULL == deslen) 
	{
		return -1;
	}
	
	int ret = 0;
	
	int pos = 0;
	int subLen = 0;
	char tmp[32] = {0};
	
	// enable
	itoa(src->bEnable? 1 : 0, tmp, 10);
	subLen = strlen(tmp);
	memcpy(des + pos, tmp, subLen);
	pos += subLen;
//	memset(des + pos, ':', 2);
//	pos += 2;

	*deslen = pos;
	
	return ret;
}

char * Ip2Str(const unsigned int iIp, char *pStr)
{
	unsigned char i;
	unsigned char cIp[4];

	for (i=0; i<4; i++)
		cIp[i] = (unsigned char)( (iIp>>(i*8)) & (0xff) );

	sprintf(pStr, "%d.%d.%d.%d", cIp[0], cIp[1], cIp[2], cIp[3]);
	return pStr;
}

unsigned int Str2Ip(const char *pStr)
{
	signed char j = 0;
	unsigned char cIp;
	unsigned int uiRet = 0;
	const char *p = pStr;

	cIp = (unsigned char)atoi(p);
	uiRet |= ( ((unsigned int)(cIp)) << (j*8) );
	j++;

	while ( (*p != 0)
	        && j >= 0)
	{
		if (*p != '.')
		{
			p++;
			continue;
		}

		cIp = (unsigned char)atoi(++p);
		uiRet |= ( ((unsigned int)(cIp)) << (j*8) );
		j++;

	}

	return uiRet;
}


#ifdef WIN32
/*
 *	日期：字符串->结构体类型DEVTIME
 */
int	ConvertTimeType(char* src, int srclen, DEVICE_TIME* des)
{
	if(src==NULL || srclen<=0 || des==NULL)
	{
		return -1;
	}

	int nRet = 0;
	string strDataTime = src;
	size_t lpos = string::npos;
	size_t spos = string::npos;
	//区分data和time
	spos = strDataTime.rfind(' ',lpos);
	string strData = strDataTime.substr(0,spos);
	string strTime = strDataTime.substr(spos+1,lpos);
	
	spos = strData.rfind('-',lpos);
	des->day = atoi(strData.substr(spos+1,lpos).c_str());
	strData = strData.substr(0,spos);
	spos = strData.rfind('-',lpos);
	des->month = atoi(strData.substr(spos+1,lpos).c_str());
	strData = strData.substr(0,spos);
	spos = strData.rfind('-',lpos);
	des->year = atoi(strData.substr(spos+1,lpos).c_str());
	
	spos = strTime.rfind(':',lpos);
	des->second = atoi(strTime.substr(spos+1,lpos).c_str());
	strTime = strTime.substr(0,spos);
	spos = strTime.rfind(':',lpos);
	des->minute = atoi(strTime.substr(spos+1,lpos).c_str());
	strTime = strTime.substr(0,spos);
	spos = strTime.rfind(':',lpos);
	des->hour = atoi(strTime.substr(spos+1,lpos).c_str());
	strTime = strTime.substr(0,spos);

	return nRet;
}

int Convert3GDateType(char* src, int srclen, NET_3G_TIMESECT* des)
{
	if(src==NULL || srclen<=0 || des==NULL)
	{
		return -1;
	}
	
	int nRet = 0;
	string strDataTime = src;
	size_t lpos = string::npos;
	size_t spos = string::npos;
	//区分起始时间和结束时间
	spos = strDataTime.rfind('-',lpos);
	string strStartTime= strDataTime.substr(0,spos);
	string strEndTime = strDataTime.substr(spos+1,lpos);
	
	spos = strStartTime.rfind(':',lpos);
	des->bBeginSec = atoi(strStartTime.substr(spos+1,lpos).c_str());
	strStartTime = strStartTime.substr(0,spos);
	spos = strStartTime.rfind(':',lpos);
	des->bBeginMin = atoi(strStartTime.substr(spos+1,lpos).c_str());
	strStartTime = strStartTime.substr(0,spos);
	spos = strStartTime.rfind(':',lpos);
	des->bBeginHour = atoi(strStartTime.substr(spos+1,lpos).c_str());
	
	spos = strEndTime.rfind(':',lpos);
	des->bEndSec = atoi(strEndTime.substr(spos+1,lpos).c_str());
	strEndTime = strEndTime.substr(0,spos);
	spos = strEndTime.rfind(':',lpos);
	des->bEndMin = atoi(strEndTime.substr(spos+1,lpos).c_str());
	strEndTime = strEndTime.substr(0,spos);
	spos = strEndTime.rfind(':',lpos);
	des->bHourEnd = atoi(strEndTime.substr(spos+1,lpos).c_str());
	strEndTime = strEndTime.substr(0,spos);
	
	return nRet;

}
#endif

struct NET_RECORD 
{
	NET_RECORDFILE_INFO *pRecord;
	bool operator < (const NET_RECORD& other)
	{
		return operator>=(other.pRecord->starttime, this->pRecord->starttime);
	}
};
void SortRecordFileList(std::list<NET_RECORDFILE_INFO *> &lstRecordFiles)
{
	std::list<NET_RECORD> temp;
	
	{
		std::list<NET_RECORDFILE_INFO*>::iterator it = lstRecordFiles.begin();
		while (it != lstRecordFiles.end())
		{
			NET_RECORD stuRecord = {0};
			stuRecord.pRecord = (*it);
			temp.push_back(stuRecord);
			
			it++;
		}
		lstRecordFiles.clear();
	}
	
	temp.sort();

	{
		std::list<NET_RECORD>::iterator it = temp.begin();
		while (it != temp.end())
		{
			lstRecordFiles.push_back((*it).pRecord);

			it++;
		}
		temp.clear();
	}
}








































