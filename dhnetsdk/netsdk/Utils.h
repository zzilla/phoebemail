
#ifndef NETSDK_UTIL_H 
#define NETSDK_UTIL_H 

#include "netsdk.h"
#include "../dvr/dvrdevice/dvr2cfg.h"

#define STR_STRATEGY_IMAGE_FIRST	"ImgQlty-First"
#define STR_STRATEGY_FLUENCY_FIRST	"Fluency-First"
#define STR_STRATEGY_AUTO			"Auto"

void Change_Utf8_Assic(unsigned char * pUTF8, char *destbuf);
////////////////////////////////////////////////////////////////////////////
void Change_Assic_UTF8(char *pStrGBKData, int nlen, char *pbuf, int buflen);
//UTF8删除残留字符
int Cut_UTF8_Tail(unsigned char *pUTF8, int nLen = 0);
//解析设备返回用户信息
/////////////parse returned userinfo///////////////////
/*不再使用
int ParseListInfoEx(int listType, char *buf, int bufLen, void *des, DWORD *lstLen, int nMaxLength,void* special = NULL);
*/
int ParseListInfo(int listType, char *buf, int bufLen, void *des, DWORD *lstLen, void* special = NULL);
///////////parse operation-right item//////////
int ParseRightItemEx(char *buf, int bufLen, OPR_RIGHT_EX *rItem,DWORD *dwListLength,int nMaxLength);
int ParseRightItem(char *buf, int bufLen, OPR_RIGHT *rItem);
///////////parse right-code string////////////
int ParseRightCode(char *buf, int bufLen, DWORD *dwList, DWORD *lstLen);
///////////parse group item//////////
int ParseGroupItemEx(char *buf, int bufLen, USER_GROUP_INFO_EX *gpItem,DWORD *dwListLength,int nMaxLength);
int ParseGroupItem(char *buf, int bufLen, USER_GROUP_INFO *gpItem);
///////////parse user item//////////
int ParseUserItemEx(char *buf, int bufLen, USER_INFO_EX *urItem, DWORD *dwListLength,void* special,int nMaxLength);
int ParseUserItem(char *buf, int bufLen, USER_INFO *urItem, void* special);

//将上层的用户信息结构打成协议包
///////////Construct right-code string/////////////////
int ConstructRightCode(char *des, DWORD *rights, int rNum, int *len, int maxlen);
///////////Build packet////////////////
int BulidUserInfoBuf(int type, void *opParam, void *subParam, char *buf, int *buflen, int maxlen, void* special=0);
int BulidUserInfoBufEx(int type, void *opParam, void *subParam, char *buf, int *buflen, int maxlen, int nMaxLength,void* special=0);
//NET_TIME结构处理函数
//比较时间
bool operator<= (const NET_TIME& lhs, const NET_TIME& rhs);
bool operator>= (const NET_TIME& lhs, const NET_TIME& rhs);
bool operator>= (const FRAME_TIME& lhs, const FRAME_TIME& rhs);
//计算偏移时间
DWORD GetOffsetTime(NET_TIME st, NET_TIME et);
NET_TIME GetSeekTimeByOffsetTime(const NET_TIME& bgtime, unsigned int offsettime);

//写视频文件头
void  WriteVideoFileHeader(FILE *file, int nType, int nFrameRate, 
                           int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);

int split_str(
		const char * const src,
		char *left,
		char *right,
		const char * const spliter,
		size_t src_len);

size_t trim_blank(char *src, size_t src_len);

//新的通道名协议
int ParseChannelNameBuf(char* src, int srclen, char* des, int deslen);
//int BuildChannelNameBuf(char* src, int srclen, char* des, int* deslen);

//设备版本号
int ParseVersionString(char* src, int srclen, DWORD* des);

//新的ddns配置协议
//Begin: Modify by li_deming(11517) 2008-2-20
//int ParseMultiDdnsString(char* src, int srclen, DEV_MULTI_DDNS_CFG* des);
//int ParseDdnsItem(char* src, int srclen, DDNS_SERVER_CFG* des);
//int BuildMultiDdnsString(DEV_MULTI_DDNS_CFG* src, char* des, int* deslen);
//int BuildDdnsItem(DDNS_SERVER_CFG* src, char* des, int* deslen);
int ParseMultiDdnsString(char* src, int srclen, DEV_MULTI_DDNS_CFG* des,DWORD* dwsign);
int ParseDdnsItem(char* src, int srclen, DDNS_SERVER_CFG* des,DWORD* dwsign);
int BuildMultiDdnsString(DEV_MULTI_DDNS_CFG* src, char* des, int* deslen,DWORD dwsign);
int BuildDdnsItem(DDNS_SERVER_CFG* src, char* des, int* deslen,DWORD dwsign);
//End:li_deming(11517)

//URL配置协议
int	ParseUrlCfgString(char* src, int srclen, DEVWEB_URL_CFG* des);
int BuildUrlCfgString(DEVWEB_URL_CFG* src, char* des, int* deslen);

//传输策略配置协议
int	ParseTransStrategyCfgString(char* src, int srclen, DEV_TRANSFER_STRATEGY_CFG* des);
int BuildTransStrategyCfgString(DEV_TRANSFER_STRATEGY_CFG* src, char* des, int* deslen);

//录象下载配置协议
int	ParseDownloadStrategyCfgString(char* src, int srclen, DEV_DOWNLOAD_STRATEGY_CFG* des);
int BuildDownloadStrategyCfgString(DEV_DOWNLOAD_STRATEGY_CFG* src, char* des, int* deslen);

#ifdef WIN32
//日期：字符串->结构体类型DEVTIME
int	ConvertTimeType(char* src, int srclen, DEVICE_TIME* des);
int Convert3GDateType(char* src, int srclen, NET_3G_TIMESECT* des);
#endif

//IP转换
char * Ip2Str(const unsigned int iIp, char *pStr);
unsigned int Str2Ip(const char *pStr);

//NET_RECORDFILE_INFO按起始时间由小到大排序
void SortRecordFileList(std::list<NET_RECORDFILE_INFO *> &lstRecordFiles);


#endif






















