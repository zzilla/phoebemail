#include "../Net/TcpSocket.h"
#include "../Net/UdpSocket.h"
#include "dvrpacket_dvr2.h"
#include "dvr2cfg.h"
#include "dvrdevice.h"
#include "../def.h"
#include "dvrsearchchannel.h"
#include "dvrpreviewchannel.h"
#include "dvrmediachannel.h"

/****************************************************************************/
//录像查询返回信息
typedef struct _FB						        /*1*/
{
	unsigned char usCardNo	:4;					/* 卡号 */
	unsigned char usAorV	:2;				    /* 01：音频；10：视频；00：图片 */
	unsigned char usAlarm	:2;					/* 00：普通文件；01：报警文件,10：动态检测报警；11：媒体文件中断 */
}FS;

typedef struct _DIRINFO
{
	FS		      DirInfo;					//卡信息
	FRAME_TIME	      DirStartTime;				//开始时间
	FRAME_TIME	      DirEndTime;				//结束时间
	unsigned int  FileSize;
	unsigned int  StartCluster;
	unsigned char DriveNo;
}DIRINFO;

typedef struct _DIRINFO2
{
	FS		      DirInfo;					//卡信息
	FRAME_TIME	      DirStartTime;				//开始时间
	FRAME_TIME	      DirEndTime;				//结束时间
	unsigned int  FileSize;
	unsigned int  StartCluster;
	unsigned char DriveNo;
    unsigned char HInt;
	unsigned char ImportantRecID;			//0:普通录像 1:重要录像				
	unsigned char Reserved;					//保留
}DIRINFO2;

#define UD_BCD_NR 20 //用户数据结构能表示的BCD数字位数

typedef struct _tagA5QueryInfo
{
    FRAME_TIME      DirEndTime ;
    BYTE        CardNo[UD_BCD_NR]; 
    BYTE        CardNoLen;
    BYTE        Rev[3];
}A5_QUERY_INFO;

typedef struct _DIRINFO3
{
	BYTE        SelfSize;				//本结构自身大小
	BYTE        DriveNo ;
	BYTE        Hint;
	BYTE        SpecialUse;				//这个对应于硬盘下载器
	//使用
	FRAME_TIME      DirStartTime ;
	FRAME_TIME      DirEndTime ;
	DWORD       FileSize ;				//文件内容大小
	DWORD       FileSpace ;				// 文件占用磁盘空间
	DWORD       StartCluster ;
	BYTE        CardNo[UD_BCD_NR];  
	BYTE        CardNoLen;
	unsigned char  ImportantRecID;		//0:普通录像 1:重要录像	
	BYTE		Reserved_1[2];
	DWORD        channel;				//通道号 > 0是有郊             
	BYTE        Reserved_2[24];			//扩展用
	FS			DirInfo ;
	BYTE        Reserved_3[3];
}DIRINFO3;

typedef struct _tagRestoreDefCmd
{
    unsigned long dwFlags;
}RESTORE_DEF_CMD;


//	组播
typedef struct __para_req_media_udpmode
{
	unsigned long 	tRemoteIp;		// 
	unsigned short	tRemotePort;	// 0-65535
	char			iChannel;		// 0-n
	char			iOpr;			// 0-Stop, 1-Start
	char			iSteamType;
	char			iReserved[31];	// reserved
}PARA_REQ_MEDIA_UDPMODE;


/****************************************************************************/

bool sendMonitor_dvr2(CDvrDevice* device, int channel, int subtype, bool bstart, 
					  int nConnType, int nConnectID, char* szLocalIp, int nLocalPort)
{
	int nProtocalVer = 0;
	device->device_get_info(dit_protocol_version, &nProtocalVer);

	if (nProtocalVer < 6)
	{
		int channelcount = device->GetVideoChannelCount();
		if (channelcount <= 16)
		{
			unsigned int nExtlen = 16;
			unsigned char data[HEADER_SIZE + 64];
			memset(data, 0, HEADER_SIZE + 64);
			
			data[0] = 0x11;
			
			CDvrChannel *pch = 0;
			for (int i=0; i<channelcount; i++)
			{
				{
					pch = device->device_get_channel(AFK_CHANNEL_TYPE_MEDIA, i);
					data[8 + i] = (pch != 0);
					//for subtype
					if (pch)
					{
						data[HEADER_SIZE + i] = ((CDvrMediaChannel *)(pch))->GetChannelSubtype();
					}
					if (pch)
					{
						pch->channel_decRef();
					}
				}
				if (channel == i)
				{
					data[8 + i] = bstart;
					if (subtype != 0)
					{
						data[HEADER_SIZE + i] = subtype;
					}
				}
			}
			
			CDvrPreviewChannel *pPreviewChannel = 
				(CDvrPreviewChannel*)device->device_get_channel(AFK_CHANNEL_TYPE_PREVIEW, channelcount);
			if (pPreviewChannel)
			{
				int type1, type2;
				pPreviewChannel->GetPreviewType(&type1, &type2);
				//多画面预览中的几画面值(1, 4, 9, 16)
				data[24] = type1;
				//多画面预览中的几画面的相应模式(1:16, 4:4, 9:2, 16:1)
				data[25] = type2;
				pPreviewChannel->channel_decRef();
			}
			
			if (nConnType == channel_connect_multicast)	//	组播
			{
				data[26] = 2;

				if (nProtocalVer < 5)
				{
					nExtlen = sizeof(PARA_REQ_MEDIA_UDPMODE);
					
					PARA_REQ_MEDIA_UDPMODE stuMulticast = {0};
					stuMulticast.iChannel = channel;
					stuMulticast.iOpr = bstart;
					stuMulticast.iSteamType = subtype;
					
					memcpy(data+HEADER_SIZE, &stuMulticast, sizeof(PARA_REQ_MEDIA_UDPMODE));
				}
				else
				{
					nExtlen = 16;
				}
			}
			else if (nConnType == channel_connect_udp)	//	UDP
			{
				char szSockIp[32] ={0};
				int nSockPort = 0;
				
				data[26] = 1;
				nExtlen = sizeof(PARA_REQ_MEDIA_UDPMODE);
				PARA_REQ_MEDIA_UDPMODE stuUdpInfo = {0};
				stuUdpInfo.iChannel = channel;
				stuUdpInfo.iOpr = bstart;
				stuUdpInfo.iSteamType = subtype;
				
				stuUdpInfo.tRemoteIp = inet_addr(szLocalIp);
				stuUdpInfo.tRemotePort = nLocalPort;
				
				memcpy(data+HEADER_SIZE, &stuUdpInfo, sizeof(PARA_REQ_MEDIA_UDPMODE));
			}
			else	//	TCP
			{
				nExtlen = 16;
			}
			
			(*(DWORD*)(data+4)) = nExtlen;
			
			return (sendcammand_dvr2(device, data, HEADER_SIZE + nExtlen) >= 0);
		}
		else
		{
			// SVR OR HDVR
			unsigned int nExtlen = 0;
			unsigned char data[128] = {0};
			data[0] = 0x11;
			data[26] = 8;

			if (nConnType == channel_connect_multicast)	//	组播
			{
				sprintf((char*)(data+HEADER_SIZE), "ChannelName:%d\r\nStream:%d\r\nOperate:%d\r\nProtocol:multicast\r\n\r\n", channel, subtype, bstart?1:0);
			}
			else if (nConnType == channel_connect_udp)	//	UDP
			{
				sprintf((char*)(data+HEADER_SIZE), "ChannelName:%d\r\nStream:%d\r\nOperate:%d\r\nProtocol:udp\r\nip:%s\r\nport:%d\r\n\r\n", channel, subtype, bstart?1:0, szLocalIp, nLocalPort);
			}
			else
			{
				sprintf((char*)(data+HEADER_SIZE), "ChannelName:%d\r\nStream:%d\r\nOperate:%d\r\nProtocol:tcp\r\n\r\n", channel, subtype, bstart?1:0);
			}
			
			nExtlen = strlen((char*)(data+HEADER_SIZE));
			(*(DWORD*)(data+4)) = nExtlen;
			
			return (sendcammand_dvr2(device, data, HEADER_SIZE + nExtlen) >= 0);
		}
	}
	else
	{
		int nExtLen = 0;
		unsigned char data[HEADER_SIZE+512] = {0};
		data[0] = 0xF4;
		
		if (nConnType == channel_connect_udp)
		{
			sprintf((char *)(data+HEADER_SIZE), 
			"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.Monitor.General\r\nchannel:%d\r\nstate:%d\r\n\
ConnectionID:%d\r\nstream:%d\r\nIP:%s\r\nPort:%d\r\n\r\n", channel, bstart?1:0, nConnectID, subtype, szLocalIp, nLocalPort);
		}
		else
		{
			sprintf((char *)(data+HEADER_SIZE), 
			"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.Monitor.General\r\nchannel:%d\r\nstate:%d\r\n\
ConnectionID:%d\r\nstream:%d\r\n\r\n", channel, bstart?1:0, nConnectID, subtype);
		}
		
		nExtLen = strlen((char *)(data+HEADER_SIZE));
		
		*(DWORD *)(data +4) = nExtLen;

		return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
	}
	
	return false;
}

//0:所有录像文件 1:外部报警 2:动态检测录像 3:所有报警 4:按卡号查询录像 5:组合条件查询 8:按卡号查询图片 9:查询图片 10:按字段查询
bool sendQueryRecord_dvr2(CDvrDevice* device, 
						  afk_query_record_s queryrecord, int nParam)
{
    int extlen = 0;
    unsigned char data[HEADER_SIZE + 60];
	memset(data, 0, HEADER_SIZE + 60);
	
    data[0] = 0xA5;
	data[8] = queryrecord.ch==255? 255:queryrecord.ch + 1;
    unsigned short *year = (unsigned short*)&data[9];
    *year = queryrecord.starttime.year;
	data[11] = queryrecord.starttime.month;
	data[12] = queryrecord.starttime.day;
	data[13] = queryrecord.starttime.hour;
	data[14] = queryrecord.starttime.minute;
	data[15] = queryrecord.starttime.second;
    data[16] = queryrecord.type;
	
	//重要录像连续查找 add by cqs 2009.1.9
	int nImportantQuery = 0;
	device->get_info(device, dit_firstqueryrecord_flag, &nImportantQuery);
	if (nImportantQuery > 0)
	{
		data[19] = 1;
	}
	
    if (queryrecord.type == 4 || queryrecord.type == 5 || queryrecord.type == 10)
    {
        extlen = strlen(queryrecord.cardid);
        *(unsigned long*)(data + 4) = extlen;
        memcpy(data + HEADER_SIZE, queryrecord.cardid, extlen);
    }
	
	if(queryrecord.type == 8)
	{
		if(nParam == 0)
		{
			return false;
		}
		else if(nParam == 1)
		{
			data[16] = 15;
			A5_QUERY_INFO cardInfo = {0};
			cardInfo.DirEndTime.year = queryrecord.endtime.year - 2000;
			cardInfo.DirEndTime.month = queryrecord.endtime.month;
			cardInfo.DirEndTime.day = queryrecord.endtime.day;
			cardInfo.DirEndTime.hour = queryrecord.endtime.hour;
			cardInfo.DirEndTime.minute = queryrecord.endtime.minute;
			cardInfo.DirEndTime.second = queryrecord.endtime.second;
			cardInfo.CardNoLen = strlen(queryrecord.cardid);
			memcpy(cardInfo.CardNo, queryrecord.cardid, strlen(queryrecord.cardid));
			extlen = sizeof(A5_QUERY_INFO);
			*(unsigned long*)(data + 4) = extlen;
			memcpy(data + HEADER_SIZE, &cardInfo, extlen);
		}	
	}
	
	if(queryrecord.type == 9)
	{
		if(nParam == 0)
		{
			data[16] = 6;
			data[25] = 16;
		}
		else if(nParam == 1)
		{
			data[16] = 0;
		}
	}
	
    if (queryrecord.bytime && !(queryrecord.type == 4 || queryrecord.type == 8))
    {
        *(unsigned long*)(data + 4) = extlen = 7;
        unsigned short *year = (unsigned short*)&data[HEADER_SIZE];
        *year = queryrecord.endtime.year;
		data[HEADER_SIZE + 2] = queryrecord.endtime.month;
		data[HEADER_SIZE + 3] = queryrecord.endtime.day;
        data[HEADER_SIZE + 4] = queryrecord.endtime.hour;
		data[HEADER_SIZE + 5] = queryrecord.endtime.minute;
		data[HEADER_SIZE + 6] = queryrecord.endtime.second;
        data[16] = 6;
    }
	return (sendcammand_dvr2(device, data, HEADER_SIZE + extlen) >= 0);
}

//查询日志
bool sendQueryLog_dvr2(CDvrDevice* device, int logtype, int nParam, char* szPacket, int nPacketLen)
{
    unsigned char data[HEADER_SIZE+1024];
	memset(data, 0, HEADER_SIZE+1024);
	
    data[0] = 0xA2;
	int nDataLen = HEADER_SIZE;
	if(0 == nParam)
	{
		if (0 == logtype) 
		{
			data[9] = 0x01;
		}
		if (logtype != -1)	//为-1表示是4K包的协议
		{
			data[16] = logtype;
		}
	}
	else if(1 == nParam)
	{
		if (0 == logtype) 
		{
			data[9] = 0x01;
			data[17] = 1;
		}
		if (logtype != -1)
		{
			data[16] = logtype;
			data[17] = 1;
		}
	}
	else if(2 == nParam)//分页方式查询日志
	{	
		nDataLen = nPacketLen+HEADER_SIZE;
		*(unsigned char *)(data+4) = nPacketLen;
		data[9] = 2;
		data[16] = logtype;
		memcpy(data+HEADER_SIZE, szPacket, nPacketLen);
	}
	
	return (sendcammand_dvr2(device, data, nDataLen) >= 0);
}

void parseSearchRecordResult_dvr2(CDvrSearchChannel *ch,  
								  unsigned char *recordresult, unsigned int len, 
								  afk_record_file_info_s **p, int &filecount, bool bQueryEndTime, int nParam)
{
   	if(15 == nParam)	//查询类别为15
	{
		int count = len / sizeof(DIRINFO3);
		if (count <= 0)
		{
			*p = 0;
			filecount = 0;
			return;
		}
		
		*p = new afk_record_file_info_s[count];
		memset(*p, 0, sizeof(afk_record_file_info_s)*count);
		filecount = count;
		DIRINFO3 *dirinfo = (DIRINFO3*)recordresult;
		for (int i=0; i<count; i++)
		{
			DIRINFO3 *tmpdirinfo = dirinfo + i;
			afk_record_file_info_s *tmpfileinfo = *p + i;
			
			//modify by cqs 支持全部通道（255）查询时返回的录像文件通道表识
			//tmpfileinfo->ch = ch->getchannelindex();
			if (tmpdirinfo->channel > 0)
			{
				tmpfileinfo->ch = tmpdirinfo->channel - 1;
			}
			else
			{
				tmpfileinfo->ch = tmpdirinfo->DirInfo.usCardNo;
			}

			memcpy(tmpfileinfo->filename,tmpdirinfo->CardNo,UD_BCD_NR);
			tmpfileinfo->size = tmpdirinfo->FileSize;
			
			tmpfileinfo->starttime.year = tmpdirinfo->DirStartTime.year + 2000;
			tmpfileinfo->starttime.month = tmpdirinfo->DirStartTime.month;
			tmpfileinfo->starttime.day = tmpdirinfo->DirStartTime.day;
			tmpfileinfo->starttime.hour = tmpdirinfo->DirStartTime.hour;
			tmpfileinfo->starttime.minute = tmpdirinfo->DirStartTime.minute;
			tmpfileinfo->starttime.second = tmpdirinfo->DirStartTime.second;
			
			tmpfileinfo->endtime.year = tmpdirinfo->DirEndTime.year + 2000;
			tmpfileinfo->endtime.month = tmpdirinfo->DirEndTime.month;
			tmpfileinfo->endtime.day = tmpdirinfo->DirEndTime.day;
			tmpfileinfo->endtime.hour = tmpdirinfo->DirEndTime.hour;
			tmpfileinfo->endtime.minute = tmpdirinfo->DirEndTime.minute;
			tmpfileinfo->endtime.second = tmpdirinfo->DirEndTime.second;
			
			tmpfileinfo->driveno = tmpdirinfo->DriveNo;
			tmpfileinfo->startcluster = tmpdirinfo->StartCluster;
			
			tmpfileinfo->nRecordFileType = tmpdirinfo->DirInfo.usAlarm;
			tmpfileinfo->nRecordFileType = tmpdirinfo->DirInfo.usAorV == 0x00 ? 4 : tmpfileinfo->nRecordFileType;

			tmpfileinfo->ImportantRecID = tmpdirinfo->ImportantRecID;
		}
	}
	else				//其它
	{
		if(bQueryEndTime)
		{
			int count = len / sizeof(DIRINFO);
			if (count <= 0)
			{
				*p = 0;
				filecount = 0;
				return;
			}
			
			*p = new afk_record_file_info_s[count];
			memset(*p, 0, sizeof(afk_record_file_info_s)*count);
			filecount = count;
			DIRINFO *dirinfo = (DIRINFO*)recordresult;
			for (int i=0; i<count; i++)
			{
				DIRINFO *tmpdirinfo = dirinfo + i;
				afk_record_file_info_s *tmpfileinfo = *p + i;
				
				//modify by cqs 支持全部通道（255）查询时返回的录像文件通道表识
				//tmpfileinfo->ch = ch->getchannelindex();
				tmpfileinfo->ch = tmpdirinfo->DirInfo.usCardNo;

				tmpfileinfo->filename[0] = '\0';
				tmpfileinfo->size = tmpdirinfo->FileSize;
				
				tmpfileinfo->starttime.year = tmpdirinfo->DirStartTime.year + 2000;
				tmpfileinfo->starttime.month = tmpdirinfo->DirStartTime.month;
				tmpfileinfo->starttime.day = tmpdirinfo->DirStartTime.day;
				tmpfileinfo->starttime.hour = tmpdirinfo->DirStartTime.hour;
				tmpfileinfo->starttime.minute = tmpdirinfo->DirStartTime.minute;
				tmpfileinfo->starttime.second = tmpdirinfo->DirStartTime.second;
				
				tmpfileinfo->endtime.year = tmpdirinfo->DirEndTime.year + 2000;
				tmpfileinfo->endtime.month = tmpdirinfo->DirEndTime.month;
				tmpfileinfo->endtime.day = tmpdirinfo->DirEndTime.day;
				tmpfileinfo->endtime.hour = tmpdirinfo->DirEndTime.hour;
				tmpfileinfo->endtime.minute = tmpdirinfo->DirEndTime.minute;
				tmpfileinfo->endtime.second = tmpdirinfo->DirEndTime.second;
				
				tmpfileinfo->driveno = tmpdirinfo->DriveNo;
				tmpfileinfo->startcluster = tmpdirinfo->StartCluster;
				
				tmpfileinfo->nRecordFileType = tmpdirinfo->DirInfo.usAlarm;
				tmpfileinfo->nRecordFileType = tmpdirinfo->DirInfo.usAorV == 0x00 ? 4 : tmpfileinfo->nRecordFileType;
			}
		}
		else
		{
			int count = len / sizeof(DIRINFO);
			if (count <= 0)
			{
				*p = 0;
				filecount = 0;
				return;
			}
			
			*p = new afk_record_file_info_s[count];
			memset(*p, 0, sizeof(afk_record_file_info_s)*count);
			filecount = count;
			DIRINFO2 *dirinfo = (DIRINFO2*)recordresult;
			for (int i=0; i<count; i++)
			{
				DIRINFO2 *tmpdirinfo = dirinfo + i;
				afk_record_file_info_s *tmpfileinfo = *p + i;
				
				//modify by cqs 支持全部通道（255）查询时返回的录像文件通道表识
				//tmpfileinfo->ch = ch->getchannelindex();
				tmpfileinfo->ch = tmpdirinfo->DirInfo.usCardNo;

				tmpfileinfo->filename[0] = '\0';
				tmpfileinfo->size = tmpdirinfo->FileSize;
				
				tmpfileinfo->starttime.year = tmpdirinfo->DirStartTime.year + 2000;
				tmpfileinfo->starttime.month = tmpdirinfo->DirStartTime.month;
				tmpfileinfo->starttime.day = tmpdirinfo->DirStartTime.day;
				tmpfileinfo->starttime.hour = tmpdirinfo->DirStartTime.hour;
				tmpfileinfo->starttime.minute = tmpdirinfo->DirStartTime.minute;
				tmpfileinfo->starttime.second = tmpdirinfo->DirStartTime.second;
				
				tmpfileinfo->endtime.year = tmpdirinfo->DirEndTime.year + 2000;
				tmpfileinfo->endtime.month = tmpdirinfo->DirEndTime.month;
				tmpfileinfo->endtime.day = tmpdirinfo->DirEndTime.day;
				tmpfileinfo->endtime.hour = tmpdirinfo->DirEndTime.hour;
				tmpfileinfo->endtime.minute = tmpdirinfo->DirEndTime.minute;
				tmpfileinfo->endtime.second = tmpdirinfo->DirEndTime.second;
				
				tmpfileinfo->driveno = tmpdirinfo->DriveNo;
				tmpfileinfo->startcluster = tmpdirinfo->StartCluster;
				
				tmpfileinfo->nRecordFileType = tmpdirinfo->DirInfo.usAlarm;
				tmpfileinfo->nRecordFileType = tmpdirinfo->DirInfo.usAorV == 0x00 ? 4 : tmpfileinfo->nRecordFileType;
				tmpfileinfo->ImportantRecID = tmpdirinfo->ImportantRecID;//标识重要录像
				tmpfileinfo->Hint = tmpdirinfo->HInt;//文件索引  add 2009.1.7
			}
		}
	}
}

bool sendDownload_dvr2(CDvrDevice* device, 
					   afk_record_file_info_s recordinfo, bool bstart, int nByTime, int nConnectID, DWORD dwParm)
{
	int nProtocalVer = 0;
	device->device_get_info(dit_protocol_version, &nProtocalVer);
	
	if (nProtocalVer < 6)
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, HEADER_SIZE);
		
		if (bstart)
		{
			data[0] = dwParm==0 ? 0xC2 : 0xCB;
			
			data[8] = recordinfo.ch + 1;
			
			unsigned short *year = (unsigned short*)&data[9];
			*year = (unsigned short)recordinfo.starttime.year;
			data[11] = recordinfo.starttime.month;
			data[12] = recordinfo.starttime.day;
			data[13] = recordinfo.starttime.hour;
			data[14] = recordinfo.starttime.minute;
			data[15] = recordinfo.starttime.second;
			data[16] = recordinfo.driveno;
			data[21] = recordinfo.nRecordFileType == 0x04 ? 1:0;
			data[22] = recordinfo.Hint; //add 2009.1.9文件精确定位
			if (1 == nByTime) //按时间段回放录像
			{
				data[27] = nByTime;
				FRAME_TIME* p = (FRAME_TIME*)(data+28);
				p->year = recordinfo.endtime.year%100;
				p->month= recordinfo.endtime.month%100;
				p->day  = recordinfo.endtime.day%100;
				p->hour = recordinfo.endtime.hour%100;
				p->minute = recordinfo.endtime.minute%100;
				p->second = recordinfo.endtime.second%100;
			}
			
			unsigned int *startcluster = (unsigned int*)&data[17];
			*startcluster = recordinfo.startcluster;
		}
		else
		{
			data[0] = 0xC9;
			data[8] = recordinfo.ch + 1;
		}
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	else
	{
		int nExtLen = 0;
		unsigned char data[HEADER_SIZE+512] = {0};
		data[0] = 0xF4;

		if (dwParm==0)	// 回放
		{
			if (bstart)
			{
				sprintf((char*)(data+HEADER_SIZE), 
				"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.PlayBack.General\r\nchannel:%d\r\nConnectionID:%d\r\n\
StartTime:%d&%d&%d&%d&%d&%d\r\nEndTime:%d&%d&%d&%d&%d&%d\r\nDriveNo:%d\r\nClusterNo:%d\r\nHint:%d\r\nType:%d\r\nIsTime:%d\r\n\r\n", recordinfo.ch+1, nConnectID, 
recordinfo.starttime.year, recordinfo.starttime.month, recordinfo.starttime.day, 
recordinfo.starttime.hour, recordinfo.starttime.minute, recordinfo.starttime.second, 
recordinfo.endtime.year, recordinfo.endtime.month, recordinfo.endtime.day, 
recordinfo.endtime.hour, recordinfo.endtime.minute, recordinfo.endtime.second, 
recordinfo.driveno, recordinfo.startcluster, recordinfo.Hint, recordinfo.nRecordFileType, nByTime);
			}
			else
			{
				sprintf((char*)(data+HEADER_SIZE), 
					"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.PlayBack.Stop\r\nchannel:%d\r\nConnectionID:%d\r\n\r\n", recordinfo.ch+1, nConnectID);
			}
		}
		else	// 下载
		{
			if (bstart)
			{
				sprintf((char*)(data+HEADER_SIZE), 
				"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.PlayBack.download\r\nchannel:%d\r\nConnectionID:%d\r\n\
StartTime:%d&%d&%d&%d&%d&%d\r\nEndTime:%d&%d&%d&%d&%d&%d\r\nDriveNo:%d\r\nClusterNo:%d\r\nHint:%d\r\nType:%d\r\n\r\n", recordinfo.ch+1, nConnectID, 
recordinfo.starttime.year, recordinfo.starttime.month, recordinfo.starttime.day, 
recordinfo.starttime.hour, recordinfo.starttime.minute, recordinfo.starttime.second, 
recordinfo.endtime.year, recordinfo.endtime.month, recordinfo.endtime.day, 
recordinfo.endtime.hour, recordinfo.endtime.minute, recordinfo.endtime.second, 
recordinfo.driveno, recordinfo.startcluster, recordinfo.Hint, recordinfo.nRecordFileType);
			}
			else
			{
				sprintf((char*)(data+HEADER_SIZE), 
					"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.PlayBack.Stop\r\nchannel:%d\r\nConnectionID:%d\r\n\r\n", recordinfo.ch+1, nConnectID);
			}
		}

		nExtLen = strlen((char *)(data+HEADER_SIZE));
		*(DWORD *)(data +4) = nExtLen;

		return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
	}

	return false;
}

bool sendExportCfgFile_dvr2(CDvrDevice* device, bool bStart, int nParam)
{
	int nProtocalVer = 0;
	device->device_get_info(dit_protocol_version, &nProtocalVer);
	
	if (nProtocalVer < 6)
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, HEADER_SIZE);
		
		if (bStart)
		{
			data[0] = 0xF3;
			data[8] = 2;
			data[9] = nParam;
		}
		else
		{
			data[0] = 0xF3;
			data[8] = 4;
			data[9] = nParam;
		}
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	else
	{
		
	}
	
	return false;
}

//下载控制
bool sendDownload_control_dvr2(CDvrDevice* device, int channel,
							   int offset_time, int offset_data, int nConnectID)
{
	int nProtocalVer = 0;
	device->device_get_info(dit_protocol_version, &nProtocalVer);
	
	if (nProtocalVer < 6)
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, HEADER_SIZE);
		
		data[0] = 0xCA;
		data[8] = channel + 1;
		unsigned int *offsettime = (unsigned int*)&data[9];
		*offsettime = offset_time;
		unsigned int *offsetdata = (unsigned int*)&data[16];
		*offsetdata = offset_data;
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	else
	{
		int nExtLen = 0;
		unsigned char data[HEADER_SIZE+256] = {0};
		data[0] = 0xF4;

		int nOffsetType = (offset_data != -1) ? 0:1;
		int nOffsetValue = (nOffsetType==0) ? offset_data:offset_time;

		sprintf((char*)(data+HEADER_SIZE), 
			"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.PlayBack.Offset\r\nchannel:%d\r\nConnectionID:%d\r\nOffsetType:%d\r\nLength:%d\r\n\r\n", channel+1, nConnectID, nOffsetType, nOffsetValue);
		
		nExtLen = strlen((char *)(data+HEADER_SIZE));
		*(DWORD *)(data +4) = nExtLen;

		return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
	}

	return false;
}

//下载继续与暂停
bool sendDownLoad_pause_dvr2(CDvrDevice* device, unsigned int ch, bool pause, int nConnectID)
{
	int nProtocalVer = 0;
	device->device_get_info(dit_protocol_version, &nProtocalVer);
	
	if (nProtocalVer < 6)
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, HEADER_SIZE);
		
		data[0] = 0xCC;
		data[1] = !(pause);
		data[8] = ch + 1;
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	else
	{
		int nExtLen = 0;
		unsigned char data[HEADER_SIZE+256] = {0};
		data[0] = 0xF4;

		sprintf((char*)(data+HEADER_SIZE), 
			"TransactionID:0\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.Network.PlayBack.Pause\r\nchannel:%d\r\nConnectionID:%d\r\nPause:%d\r\n\r\n", ch+1, nConnectID, pause?0:1);
		
		nExtLen = strlen((char *)(data+HEADER_SIZE));
		*(DWORD *)(data +4) = nExtLen;

		return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
	}

	return false;
}

//Edited by Linjy-2006-11-2
bool sendUpgradeControl_dvr2(CDvrDevice* device, afk_upgrade_channel_param_s *ucparam/*, bool bstart*/)
{
	unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	data[0] = 0xC7;
	data[8] = ucparam->filetype;
	data[24] = 1;
	*(DWORD*)&data[28] = ucparam->size;
	memcpy(data + 10, "start", 6);
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

bool sendImportCfgFile_dvr2(CDvrDevice* device, afk_upgrade_channel_param_s *ucparam)
{
	unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0xF3;
	data[8] = 3;
	data[10] = 1;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//Edited by Linjy-2006-11-2
int sendUpgradeData_dvr2(CDvrDevice* device, int packetId, char *buf, int len, afk_upgrade_channel_param_s *ucparam)
{
	if (!device || len > 1024)
	{
		return -1;
	}
	
	if (len >=0)
	{
		int packlen = HEADER_SIZE + len;
		unsigned char data[1056];
		memset(data, 0, 1056);
		
		data[0] = 0xC7;
		*(DWORD*)&data[10] = packetId;
		(*(DWORD*)(data+4)) = len;
		data[8] = ucparam->filetype;
		data[24] = 3;
		memcpy(data + HEADER_SIZE, buf, len);
		
		return sendcammand_dvr2(device, data, packlen);
	}
	else //发送结束标志
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, 32);
		data[0] = 0xC7;
		data[8] = ucparam->filetype;
		data[9] = packetId; //升级类型标志
		data[24] = 2;
		memcpy(data + 10,"End", 6);
		
		return sendcammand_dvr2(device, data, HEADER_SIZE);
	}
	
	return -1;
}

int sendImportCfgData_dvr2(CDvrDevice* device, int packetId, char *buf, int len, afk_upgrade_channel_param_s *ucparam)
{
	if (!device || len > 1024)
	{
		return -1;
	}
	
	if (len >=0)
	{
		unsigned char data[1110] = {0};
		CFG_INFO stuConfigData = {0};
		
		data[0] = 0xF3;
		(*(DWORD*)(data+4)) = sizeof(CFG_INFO);
		data[8] = 3;
		data[10] = 2;
		data[11] = 1;
		stuConfigData.AllConfigLen = ucparam->size;
		stuConfigData.CurrentFileNo = 0;
		stuConfigData.TotalConfigFileNum = 1;
		stuConfigData.ConfigFile.FileTotalLen = ucparam->size;
		stuConfigData.ConfigFile.ThisFrameNo = packetId;
		stuConfigData.ConfigFile.ThisFrameLen = len;
		memcpy(stuConfigData.ConfigFile.DataBuffer, buf, len);
		
		std::string strFilePath = ucparam->filename;
		std::string::size_type pos = strFilePath.rfind("\\");
		size_t lpos = string::npos;
		if (lpos != pos)
		{
			strcpy(stuConfigData.ConfigFile.FileName, strFilePath.substr(pos, lpos).c_str() + 1);
		}
		
		memcpy(data + HEADER_SIZE, &stuConfigData, sizeof(CFG_INFO));
		
		return sendcammand_dvr2(device, data, HEADER_SIZE + sizeof(CFG_INFO));
	}
	else //发送结束标志
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, 32);
		data[0] = 0xF3;
		data[8] = 3;
		data[10] = 3;
		data[11] = 1;
		
		return sendcammand_dvr2(device, data, HEADER_SIZE);
	}
	
	return -1;
}

bool sendPreview_dvr2(CDvrDevice* device, int type1, int type2, bool bstart, 
					  int nConnType, int nConnectID, char* szLocalIp, int nLocalPort)
{
	int nProtocalVer = 0;
	device->device_get_info(dit_protocol_version, &nProtocalVer);
	
	if (nProtocalVer < 6)
	{
		unsigned char data[HEADER_SIZE];
		memset(data, 0, HEADER_SIZE);
		
		data[0] = 0x11;
		
		int nChannelCount = device->channelcount(device);
		CDvrChannel *pch;
		for (int i=0; i<nChannelCount; i++)
		{
			pch = device->device_get_channel(AFK_CHANNEL_TYPE_MEDIA, i);
			data[8 + i] = (pch != 0);
			if (pch)
			{
				pch->channel_decRef();
			}
		}
		if (bstart)
		{
			//多画面预览中的几画面值(1, 4, 9, 16)
			data[24] = type1;
			//多画面预览中的几画面的相应模式(1:16, 4:4, 9:2, 16:1)
			data[25] = type2;
		}
		else
		{
			data[24] = 0;
			data[25] = 0;
		}
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	else
	{
		
	}

	return false;
}

bool sendAlarmQuery_dvr2(CDvrDevice* device, bool bRecordState, int proType)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	if (0 == proType) //老报警协议
	{
		data[0] = 0xA1;
		data[8] = bRecordState;
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	else if (1 == proType) //新报警协议
	{
		//增加单通道订阅报警功能，0 全部通道，1－16单通道
		DWORD dwAlarmMask[32] = {0};
		device->get_info(device, dit_alarm_listen_use_mode, dwAlarmMask);

		
		//订阅所有报警
		unsigned char AlarmType[] = {0/*保留*/, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 156, 161, 162};

		for (int i = 1; i <= 23; i++)//新添报警解码器报警。
		{
			memset(data, 0, HEADER_SIZE);
			data[0] = 0x68;
			data[8] = 0x02;
			data[12] = AlarmType[i];//156表示报警解码器报警

			if (device->GetVideoChannelCount() <= 16)
			{
				*(unsigned long*)(data + 28) = dwAlarmMask[0];
				if (sendcammand_dvr2(device, data, HEADER_SIZE) < 0)
				{
					return false;
				}
			}
			else // SVR
			{
				BOOL bAllChannel = TRUE;
				int j = 0;
				for (j = 0; j < 32; j++)
				{
					if (dwAlarmMask[j] != 0)
					{
						bAllChannel = FALSE;
						break;
					}
				}
				
				int nNum = device->GetVideoChannelCount() / 32;
				if (device->GetVideoChannelCount()%32 != 0)
				{
					nNum++;
				}
				for (j = 0; j < nNum; j++)
				{
					if (bAllChannel)
					{
						data[13] = j;
						*(unsigned long*)(data + 28) = 0xFFFFFFFF;
						if (sendcammand_dvr2(device, data, HEADER_SIZE) < 0)
						{
							return false;
						}
					}
					else if (dwAlarmMask[j] != 0)
					{
						data[13] = j;
						*(unsigned long*)(data + 28) = dwAlarmMask[j];
						if (sendcammand_dvr2(device, data, HEADER_SIZE) < 0)
						{
							return false;
						}
					}
				}
			}
		}
		
		// NVD
		int nDeviceType = device->device_type(device);
		if (nDeviceType == PRODUCT_NVD_SERIAL)
		{
			memset(data, 0, HEADER_SIZE);
			data[0] = 0x68;
			data[8] = 0x02;
			data[12] = 13;
			if (sendcammand_dvr2(device, data, HEADER_SIZE) < 0)
			{
				return false;
			}
		}
		
		return true;
	}
    return false;
}

//AFK_CONTROL_TYPE_PTZ
//AFK_CONTROL_TYPE_YWPTZ
//AFK_CONTROL_TYPE_RESET
//parm1:0:重启设备  1:关闭设备
bool sendControl_dvr2(CDvrDevice* device, 
					  int ch, int type1, int type2, int parm1, int parm2, int parm3, char *pParam4)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
    if (type1 == AFK_CONTROL_TYPE_PTZ)
    {
        data[0] = 0x12;
        data[9] = ch;
        switch (type2)
        {
        case PTZ_UP:
            data[10] = 0x00;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_DOWN:
            data[10] = 0x01;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_LEFT:
            data[10] = 0x02;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_RIGHT:
            data[10] = 0x03;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_ZOOM_ADD:
            data[10] = 0x04;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_ZOOM_DEC:
            data[10] = 0x05;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_FOCUS_ADD:
            data[10] = 0x07;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_FOCUS_DEC:
            data[10] = 0x08;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_APERTURE_ADD:
            data[10] = 0x09;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_APERTURE_DEC:
            data[10] = 0x0A;
            data[11] = 0x0A;
            data[12] = parm1;
            break;
        case PTZ_POINT_MOVE:
            data[10] = 0x10;
            data[11] = 0x0A;
            break;
        case PTZ_POINT_SET:
            data[10] = 0x11;
            data[11] = 0x0A;
            break;
        case PTZ_POINT_DEL:
            data[10] = 0x12;
            data[11] = 0x0A;
            break;
        case PTZ_POINT_LOOP:
            data[10] = 0x0F;
            data[11] = 0x0A;
            break;
        case PTZ_LAMP:
            data[10] = 0x0E;
            data[11] = parm1;
            if (parm2 == 0)
            {
                data[16] = 0x0F;
            }
            else if (parm2 == 1)
            {
                data[16] = 76;
            }
            else if (parm2 == 2)
            {
                data[16] = 99; 
            }
            break;
        default:
            break;
        }
    }
	
    else if (type1 == AFK_CONTROL_TYPE_YWPTZ)
    {
        data[0] = 0x12;
        data[9] = ch;
		data[10] = type2;
		if (type2 == 0x33) 
		{
			*(DWORD*)(data+20) = parm1;
			*(DWORD*)(data+24) = parm2;
			*(DWORD*)(data+28) = parm3;
		}
		else
		{
			data[11] = parm1;
			data[12] = parm2;
			data[16] = parm3;

			if(pParam4 == (char*)1)
			{
				data[8] = 0x01;//表示停止
				data[11] = 0;
				data[12] = 0;
				data[16] = 0;
			}
		}
    }
	
    else if (type1 == AFK_CONTROL_TYPE_RESET)
    {
        data[0] = 0x60;
        if (parm1 == 0)
        {
            data[8] = 1;
        }
        else if (parm1 == 1)
        {
            data[8] = 2;
        }
    }
	//Added by Linjy-2006-11-1
	else if (type1 == AFK_CONTROL_TYPE_DEVCONTROL)
	{
		switch(type2)
		{
		case 0:	//关闭、重启及硬盘操作(扩展字节里加"分区唯一标识&&")
			{
				unsigned char data[HEADER_SIZE + 512];
				memset(data, 0, HEADER_SIZE + 512);
				data[0] = 0x60;
				data[8] = parm1; //操作类型
				data[12] = parm2; //子设备号
				data[16] = parm3; //操作码
				
				int extlen = 0;
				if (/*pParam4 != NULL && */3 == parm1)
				{
					char info[28] = {0};
					sprintf(info, "%d&&", (int)pParam4);
					extlen = strlen(info);
					memcpy(data + HEADER_SIZE, info, extlen);
				}
				
				*(unsigned long*)(data + 4) = extlen;
				return (sendcammand_dvr2(device, data, HEADER_SIZE + extlen) >= 0);
			}
			break;
		case 1: //网络键盘
			{
				data[0] = 0x99;
				data[8] = parm1;
			}
			break;
		case 2: //触发、停止报警输入输出
			{
				if (parm3>16)
				{
					return false;
				}
				data[0] = parm1==0 ? 0x1a : 0x1b; //输入or输出
				memset(&data[12], 2, 20); 
				data[12+parm3] = parm2!=0 ? 1 : 0; //触发or停止
			}
			break;
		case 3:	//控制矩阵
			{
				data[0] = 0x60;
				data[8] = 6;
				data[12] = parm2;
				data[16] = parm1;
			}
			break;
		case 4: //刻录控制
			{
				data[0] = 0xC5;
				data[16] = parm1; //启动还是停止
				*(DWORD*)(&data[8]) = parm2; //通道号掩码
				data[12] = parm3; //设备号掩码
				data[28] = 1;
				//	data[29] = parm3; //录象时间段
			}
			break;
		case 5:	//叠加密码
		case 6:
		case 7:
		case 8:
			{
				return SendControlForBurning(device, type2, pParam4);
			}
			break;
		case 9: //恢复默认配置
			{
				unsigned char data[HEADER_SIZE+60];
				memset(data, 0, HEADER_SIZE+60);
				data[0] = 0x60;
				data[8] = 10;
				RESTORE_DEF_CMD RestoreComm = {0};
				RestoreComm.dwFlags = *(DWORD*)pParam4;
				int extlen = sizeof(RESTORE_DEF_CMD);
				*(unsigned long*)(data + 4) = extlen;
				memcpy(data + HEADER_SIZE, &RestoreComm, extlen);
				return (sendcammand_dvr2(device,data,HEADER_SIZE+extlen) >= 0);
			}
			break;
		case 10: //触发DSP抓图控制
			{
				if (pParam4 == NULL)
				{
					return false;
				}
				
				unsigned char data[HEADER_SIZE+20];
				memset(data, 0, HEADER_SIZE+20);
				data[0] = 0x60;
				data[8] = 8; //控制类型
				data[16] = parm1; //操作码
				int extlen = strlen(pParam4);
				*(unsigned long*)(data + 4) = extlen;
				memcpy(data + HEADER_SIZE, pParam4, extlen);
				return (sendcammand_dvr2(device,data,HEADER_SIZE+extlen) >= 0);
			}
			break;
		case 11: //清除日志
			{
				data[0] = 0x60;
				data[8] = 9;
				data[16] = parm1;
			}
			break;
		case 12://无线报警
			{
				data[0] = 0x1b;
				data[8] = 101;
				memset(&data[12], 2, 20); 
				data[12+parm3] = parm2!=0 ? 1 : 0; //触发or停止
			}
			break;		
		case 13: //设置重要录像
			{
				if (pParam4 == NULL)
				{
					return false;
				}
				unsigned char data[HEADER_SIZE+256];
				memset(data, 0, HEADER_SIZE+256);
				
				data[0] = 0xC5;
				data[28] = 2; //控制类型
				int extlen = sizeof(IMPORTANT_REC_DIRINFO);
				*(unsigned long*)(data + 4) = extlen;
				memcpy(data + HEADER_SIZE, pParam4, extlen); 
				return (sendcammand_dvr2(device,data,HEADER_SIZE+extlen) >= 0);
			}
			break;
		case 14: //OEM报停控制
			{
				unsigned char data[HEADER_SIZE];
				memset(data, 0, HEADER_SIZE);

				data[0] = 0x60;
				data[8] = 11;
				data[16] = parm1 > 0 ? 1 : 0;
				return (sendcammand_dvr2(device,data,HEADER_SIZE) >= 0);
			}
			break;
		default:
			return false;
			break;
		}
	}
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//发送控制刻录信息(叠加到刻录信息)
bool SendControlForBurning(CDvrDevice *pDevice, int nParam1, char *pParam2)
{
	if(pDevice == NULL || (pParam2 == NULL && nParam1 != 7))
	{
		return false;
	}
	
	int nDataLen = HEADER_SIZE;
	unsigned char data[2048];
	memset(data, 0, 2048);
	
	if(5 == nParam1)	//在刻录信息中增加密码
	{
		int nPwdLen = strlen(pParam2);
		if(8 < nPwdLen)		//密码长度最大为8位
		{
			return false;
		}
		data[0] = 0x61;
		*(DWORD*)&data[4] = nPwdLen + 2;
		data[8] = 2;
		data[12] = 0;
		memcpy(data + HEADER_SIZE, pParam2, nPwdLen);
		data[HEADER_SIZE + nPwdLen] = '\0';
		data[HEADER_SIZE + nPwdLen + 1] = '\0';
		Encrypt(data + HEADER_SIZE, nPwdLen + 2);  //对信息进行加密
		nDataLen = nDataLen + nPwdLen + 2;
	}
	else if(6 == nParam1)		//在刻录信息中增加片头
	{
		int nFilmHeadLen = strlen(pParam2);
		if(1024 < nFilmHeadLen)		//最大为1024位
		{
			return false;
		}
		for(int i=0;i<nFilmHeadLen;i++)
		{
			if(pParam2[i] == '\n')
			{
				pParam2[i] = '\0';
			}
		}
		data[0] = 0x61;
		*(DWORD*)&data[4] = nFilmHeadLen + 2;
		data[8] = 2;
		data[12] = 1;
		memcpy(data +HEADER_SIZE, pParam2, nFilmHeadLen);
		data[HEADER_SIZE + nFilmHeadLen] = '\0';
		data[HEADER_SIZE + nFilmHeadLen + 1] = '\0';
		Encrypt(data + HEADER_SIZE, nFilmHeadLen + 2);
		nDataLen = nDataLen + nFilmHeadLen + 2;
	}
	else if(7 == nParam1)		//打点
	{
		data[0] = 0x61;
		*(DWORD*)&data[4] = 0;
		data[8] = 2;
		data[12] = 2;
	}
	else if(8 == nParam1)		//自定义叠加
	{
		int nCustomDataLen = strlen(pParam2);
		if(1024 < nCustomDataLen)		//最大为1024位
		{
			return false;
		}
		for(int i=0;i<nCustomDataLen;i++)
		{
			if(pParam2[i] == '\n')
			{
				pParam2[i] = '\0';
			}
		}
		data[0] = 0x61;
		*(DWORD*)&data[4] = nCustomDataLen + 2;
		data[8] = 2;
		data[12] = 3;
		memcpy(data +HEADER_SIZE, pParam2, nCustomDataLen);
		data[HEADER_SIZE + nCustomDataLen] = '\0';
		data[HEADER_SIZE + nCustomDataLen + 1] = '\0';
		Encrypt(data + HEADER_SIZE, nCustomDataLen + 2);
		nDataLen = nDataLen + nCustomDataLen + 2;
	}
	else
	{
		return false;
	}
	
	return (sendcammand_dvr2(pDevice, data, nDataLen) >= 0);
}

//数据加密
void Encrypt(void *pdata, int len)
{
	char *data = (char *)pdata;
	
	while (len --) {
		(*data) ^= 'w';
		(*data) ^= '!';
		data++;
	}
}

//查询通道名
bool sendQueryChannelName_dvr2(CDvrDevice* device)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0xA8;
	
	bool b1 = (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	
	//再请求一次新的通道名
	data[8] = 1;
	bool b2 = (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	
	return b1 && b2;
}

void getchinesefont_dvr2(unsigned char *pscrbuf, unsigned char *pdstbuf)
{
    int nlen = strlen((char*)pscrbuf);
    
    int j = 0;
    for (int i=0; i<nlen; i++)
    {
        if (*(pscrbuf + i) > 0x80)
        {
            *(pdstbuf + j++) = *(pscrbuf + i);
        }
    }
    pdstbuf[j] = '\0';
}

//查询报警布撤防输入
bool sendQueryAlarmInput_dvr2(CDvrDevice* device)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x23;
    data[8] = 0x00;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//查询报警布撤防输出
bool sendQueryAlarmOutput_dvr2(CDvrDevice* device)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x23;
    data[8] = 0x01;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//查询无线报警输出
bool sendQueryWirelessAlarmOutput_dvr2(CDvrDevice* device)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x23;
    data[8] = 0x04;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}
//查询报警触发方式
bool sendQueryAlarmTrrigerMode_dvr2(CDvrDevice* device)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x23;
    data[8] = 0x05;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//解析报警布撤防
void parseSearchAlarmResult_dvr2(CDvrDevice* device, 
								 unsigned char *alarmresult, unsigned int len,
								 afk_alarm_info_s **p)
{
    *p = new afk_alarm_info_s;
    afk_alarm_info_s *pp = *p;
	
    memset(pp, 0, sizeof(afk_alarm_info_s));
	
    pp->isinput = (*(alarmresult + 8) == 0);
	pp->bSupportTriggerMode = *(alarmresult + 10);
	
    //没有出错
    if (*(alarmresult + 9) == 0)
    {
        pp->state = new unsigned char[20];
        pp->count = 20;
		
        for (int i=0; i<20; i++)
        {
            if (*(alarmresult + 12 + i) == 2)
            {
                pp->count = i;
                return;
            }
			
            *(pp->state + i) = *(alarmresult + 12 + i);
        }
    }
}

//设置报警布撤防
bool sendSettingAlarm_dvr2(CDvrDevice* device, afk_alarm_control_info_s alarminfo)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
    if (alarminfo.type == 1)//alarm in
    {
		data[0] = 0x23;
        data[8] = 2;
    }
    else if(alarminfo.type == 2)//alarm out
    {
        data[0] = 0x1B;
		data[8] = 0;
    }
	else if(alarminfo.type == 5)//wireless alarm out
	{
		data[0] = 0x1b;
		data[8] = 101;
	}
	else if ( alarminfo.type == 9 ) //send or stop pos ticket raw data
	{
		data[0] = 0x25 ;
		data[8] = alarminfo.controlflag ;
	}
	
    memset(data + 12, 2, 20);
    data[12 + alarminfo.alarmid] = alarminfo.controlflag;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >=0);
}
bool sendSetingAlarmTrrigerMode_dvr2(CDvrDevice* device, afk_alarm_trriger_mode_s trrigerinfo)
{
	unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x23;
	data[8] = 0x06;
	for(int i = 0; i < trrigerinfo.channel_count; i++)
	{
		switch(trrigerinfo.trriger_mode[i].mode) //（0-手动；1-自动；2-关闭）<- (0关闭1手动2自动）,为与录象控制保持一致。
		{
		case 0:
			data[12+i] = 2;
			break;
		case 1:
			data[12+i] = 0;
			break;
		case 2:
			data[12+i] = 1;
			break;
		default:
			data[12+i] = 1;
			break;
		}
		
	}
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//设置报警解码器输出
bool sendSetingDecoderAlarm_dvr2(CDvrDevice* device, afk_decoder_alarm_info_s dainfo)
{
	//	OutputDebugString("send decoder alarm data ++\n");
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
    data[0] = 0x1B;
    data[8] = dainfo.decoderNo + 1;
	
    memset(data + 12, 2, 20);
    data[12 + dainfo.alarmChn] = dainfo.alarmState;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//设置透明串口
bool sendSettingSerial_dvr2(CDvrDevice* device, afk_trans_channel_param_s transinfo, bool bstart)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	if (bstart)
	{
		data[0] = 0x20;
		//操作类型
		data[9] = 0x01;
		//串口设置
		data[8] = transinfo.trans_device_type + 1;
		data[10] = transinfo.baudrate;
		data[11] = transinfo.databits;
		data[12] = transinfo.stopbits;
		data[13] = transinfo.parity;
		//指定回复
		data[14] = 0x01;
		
		if (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0)
		{
			data[9] = 0x03;
			return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
		}
	}
	else
	{
		data[0] = 0x20;
		//操作类型
		data[9] = 0x03;
		//串口设置
		data[8] = transinfo.trans_device_type + 1;
		data[10] = transinfo.baudrate;
		data[11] = transinfo.databits;
		data[12] = transinfo.stopbits;
		data[13] = transinfo.parity;
		//指定回复
		data[14] = 0x00;
		
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
	
	return false;
}

//发送透明串口信息
bool sendTransSerialData_dvr2(CDvrDevice* device, int transtype, char *buffer, int buflen)
{
    unsigned char *data = new unsigned char[buflen + HEADER_SIZE];
	if (!data)
	{
		return false;
	}
	memset(data, 0, buflen + HEADER_SIZE);
	
	data[0] = 0x20;
	//历史原因，使用0，1。不是旧协议的1，2。
	data[8] = transtype != 0 ? 1 : 0;
//	data[8] = transtype + 1;
    //操作类型
    data[9] = 0x02;
	
    *(unsigned int*)(data + 4) = buflen;
    memcpy(data + HEADER_SIZE, buffer, buflen);
    
	bool b = (sendcammand_dvr2(device, data, buflen + HEADER_SIZE) >= 0);
	
	delete[] data;
    return b;
}

//发送对讲要求
bool sendTalkRequest_dvr2(CDvrDevice* device, int channel, bool bstart, int encodetype)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x1D;
    data[12] = 0;
	data[13] = encodetype;
    if (bstart)
    {
        data[8] = 0x02;
    }
    else
    {
        data[8] = 0x03;
    }
	
	if (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0)
	{
		memset(data, 0, HEADER_SIZE);
		data[0] = 0xC0;
		data[8] = channel;
		if (bstart)
		{
			data[12] = 0x01;
		}
		else
		{
			data[12] = 0x00;
		}
		return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}
    
    return false;
}

//发送对讲信息
bool sendTalkData_dvr2(CDvrDevice* device, int channel, char *buffer, int buflen, int encodetype)
{
	if (buflen <= 0 || NULL == buffer)
	{
		return false;
	}
	const int ciBufLen = 1040;
    unsigned char data[ciBufLen + HEADER_SIZE];
	memset(data, 0, ciBufLen + HEADER_SIZE);
	
	data[0] = 0x1D;
    //操作类型
    data[8] = 0x02;
    data[12] = channel;
	data[13] = encodetype;
	
    int count = buflen/ciBufLen;
    
    for (int i=0; i<count; i++)
    {
        *(unsigned int*)(data + 4) = ciBufLen;
        memcpy(data + HEADER_SIZE, buffer + ciBufLen*i, ciBufLen);
        if (sendcammand_dvr2(device, data, ciBufLen + HEADER_SIZE) < 0)
        {
            return false;
        }
    }
	
    if (buflen%ciBufLen > 0)
    {
        *(unsigned int*)(data + 4) = buflen%ciBufLen;
        memcpy(data + HEADER_SIZE, buffer + ciBufLen*count, buflen%ciBufLen);
        if (sendcammand_dvr2(device, data, buflen%ciBufLen + HEADER_SIZE) < 0)
        {
            return false;
        }
    }
    return true;
}

//查询配置
bool sendQueryConfig_dvr2(CDvrDevice* device, unsigned short configtype, int param)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	//Edited by Linjy-2006-11-7 
	if (CONFIG_TYPE_DSP_MASK == configtype) //获取通道编码格式和分辨率的掩码
	{
		data[0] = 0x83;
		//data[8] = 0xFF;
		data[8] = (param&0x0000ff00)>>8; //查询分辨率信息的时候,用作通道号
		data[9] = param&0x000000ff;		 //查询类型
	}
	else
	{
		memcpy(data + 8, "config", 8);
		data[0] = 0xA3;
		data[16] = configtype;
		//Begin: Add by yehao(10857) 2007-04-25
		if (configtype > CONFIG_TYPE_ALARM_BEGIN && configtype < CONFIG_TYPE_ALARM_END)
		{
			data[16] = 124;
			data[24] = configtype-CONFIG_TYPE_ALARM_BEGIN;
			data[25] = param&0xFF;
		}
		//End: yehao(10857) 2007-04-25
		if (CONFIG_TYPE_TIMESHEET == configtype)
		{
			data[16] = 125;
			data[24] = param>>16;
			data[25] = param&0xFF;
		}
		//modify by cqs (10842)2008－08－05
		if (CONFIG_TYPE_WIRELESS_ADDR == configtype)//无线报警配置
		{
			data[24] = param;
		}
		//Modify by zsc(11402) 2008-1-21
		if (CONFIG_TYPE_INTERVIDEO == configtype || CONFIG_TYPE_COVER == configtype)
		{
			data[17] = param;
		}
		if (CONFIG_TYPE_ETHERNET == configtype)
		{
			data[24] = param;
		}
		if(0x0a == configtype)
		{
			data[0] = 0xA8;
		}
		if (CONFIG_TYPE_VIDEO_MATRIX == configtype)
		{
			data[18] = param;
		}
		
		//add by cqs(10842) 2008-10-31 普通 CFG_GENERAL - 0,子类别w24为0代表之前的查询，w24为1代表DST的查询
		//普通 CONFIG_TYPE_WLAN - 0,W24为0代表之前的WLAN查询，w24为1代表CDMA\GPRS WIRELESS的查询
		if (CONFIG_TYPE_GENERAL == configtype|| CONFIG_TYPE_WLAN == configtype)
		{
			data[24] = param;
		}
		
    }
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//设置配置
bool sendSetupConfig_dvr2(CDvrDevice* device, unsigned short configtype, int param,
						  unsigned char *buffer, int buflen)
{
    unsigned char *data = new unsigned char[HEADER_SIZE + buflen];
	if (!data)
	{
		return false;
	}
    memset(data, 0, HEADER_SIZE);
	
    data[0] = 0xC1;
    memcpy(data + 8, "config", 8);
	
	if (CONFIG_TYPE_TITLE == configtype)
	{
		data[0] = 0xC6;
	}
	else if (configtype > CONFIG_TYPE_ALARM_BEGIN && configtype < CONFIG_TYPE_ALARM_END)
	{
		data[16] = 124;
		data[24] = configtype-CONFIG_TYPE_ALARM_BEGIN;
		data[25] = param&0xFF;
	}
	else if (CONFIG_TYPE_TIMESHEET == configtype)
	{
		data[16] = 125;
		data[24] = param>>16;
		data[25] = param&0xFF;
	}
	//modify by cqs (10842)2008－08－05
	else if (CONFIG_TYPE_WIRELESS_ADDR == configtype)//无线报警配置
	{
		data[16] = configtype;
		data[24] = param;
	}
	else
	{
		data[16] = configtype;
	}
	
	//Modify: by zsc(11402) 2008-1-23
	if (CONFIG_TYPE_INTERVIDEO == configtype || CONFIG_TYPE_COVER == configtype)
	{
		data[17] = param;
	}
	
	if (CONFIG_TYPE_PPPOE == configtype || CONFIG_TYPE_DDNS == configtype)
	{
		data[20] = *(unsigned long*)buffer;
		buffer += 4;
		buflen -= 4;
	}
	
	if (CONFIG_TYPE_ETHERNET == configtype)
	{
		data[24] = param;
	}
	
	if (CONFIG_TYPE_VIDEO_MATRIX == configtype)
	{
		data[18] = param;
	}
	
	if (CONFIG_TYPE_GENERAL == configtype || CONFIG_TYPE_WLAN == configtype)//夏令时时parm=1;GPRS/CDMA时parm=1
	{
		data[24] = param;
	}
	
    *(unsigned long*)(data + 4) = buflen;
    memcpy(data + HEADER_SIZE, buffer, buflen);
	
	bool b = (sendcammand_dvr2(device, data, HEADER_SIZE + buflen) >= 0);
	
	delete[] data;
    return b;
}

//设置录像状态
bool sendSetupRecordState_dvr2(CDvrDevice* device, unsigned char *buffer, int buflen)
{
	//	unsigned char data[HEADER_SIZE];
	//	memset(data, 0, HEADER_SIZE);
	//	
	//	data[0] = 0xC5;
	//	int channelcount = buflen>16 ? 16 : buflen;
	//	memcpy(data + 8,  buffer, channelcount);
	//	
	//	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	
	unsigned char *data = new unsigned char[HEADER_SIZE + buflen];
	if (!data)
	{
		return false;
	}
	bool b = false;
    memset(data, 0, HEADER_SIZE);

	data[0] = 0xC5;
	
	if (buflen > 16) //超过16个通道控制
	{
		*(unsigned long*)(data + 4) = buflen;
		data[28] = 3;
		memcpy(data + HEADER_SIZE, buffer, buflen);
		b = (sendcammand_dvr2(device, data, HEADER_SIZE + buflen) >= 0);
	}
	else
	{
		memcpy(data + 8,  buffer, buflen);	
		b = (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
	}

	delete[] data;
    return b;
}

//查询设备时间
bool sendQueryDeviceTime_dvr2(CDvrDevice* device)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x24;
    data[8] = 0x00;
    
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//设置设备时间
bool sendSetupDeviceTime_dvr2(CDvrDevice* device, int year, int month, int day,
                              int hour, int minute, int second)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x24;
    data[8] = 0x01;
	
    *(data + 16) = year%100;
    *(data + 17) = month;
    *(data + 18) = day;
    *(data + 19) = hour;
    *(data + 20) = minute;
    *(data + 21) = second;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//查询协议(串口协议与解码器协议)
bool sendQueryProtocol_dvr2(CDvrDevice* device, int protocoltype,DWORD dwProtocolIndex)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0xcd;
    data[8] = protocoltype;
	data[9] = (BYTE)dwProtocolIndex;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//查询系统信息
bool sendQuerySystemInfo_dvr2(CDvrDevice* device, int type, int param)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	//	switch(type)
	//	{
	//	default:
	{
		data[0] = 0xA4;
		data[8] = type;
		
		if (type == SYSTEM_INFO_VIDEO_ATTR/*get all*/)
		{
			data[12] = 0xFF;
		}
		else if(type == SYSTEM_INFO_DEV_SNAP)
		{
			data[20] = 0x01;
		}
		else
		{
			
		}
		
	}
	//		break;
	//	}
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//发送用户操作
bool sendUserOperate_dvr2(CDvrDevice* device, int type, char *buffer, int bufferlen)
{
    unsigned char *data = new unsigned char[HEADER_SIZE + bufferlen];
	if (!data)
	{
		return false;
	}
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0xA6;
    data[8] = type;
    *(unsigned int*)(data + 4) = bufferlen;
	
    memcpy(data + HEADER_SIZE, buffer, bufferlen);
	
	bool b = (sendcammand_dvr2(device, data, HEADER_SIZE + bufferlen) >= 0);
	
	delete[] data;
    return b;
}


//强制I桢
bool sendForceIframe_dvr2(CDvrDevice* device,  unsigned char mainchn, unsigned char subchn)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x80;
	data[8] = mainchn;
	data[9] = subchn;

	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//设置限制码流
bool sendLimitFlux_dvr2(CDvrDevice* device, unsigned short limitflux)
{
    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x81;
	memcpy(data+8, &limitflux, sizeof(unsigned short));
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//查询设备工作状态
bool sendQuerydevstate_dvr2( CDvrDevice* device, unsigned char chn, unsigned char type)
{
	unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	
	data[0] = 0x82;
	data[8] = type;
	data[9] = chn;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

//查询设备报警、状态
bool sendQueryAlarmState_dvr2(CDvrDevice* device, int statetype,int deviceid)
{
	//unsigned char data[HEADER_SIZE+sizeof(CHANNEL_INFO_T)];
	unsigned char data[1024];
	memset(data, 0, 1024);
	
	data[0] = 0xa1;
	int datalen = HEADER_SIZE;
	switch(statetype)
	{
	case 0:	//常规报警
		data[8] = 0x00;
		break;
	case 1: //视频遮挡 //临时兼容做法，以后所有报警都用0x69协议
		data[0] = 0x68;
		data[8] = 0x01;
		data[12] = 0x04;
		break;
	case 2: //录象状态
		data[8] = 0x00;
		break;
	case 3:	//系统资源状态
		data[8] = 0x0b;
		break;
	case 4:	//码流信息
		data[8] = 0x0c;
		{
			CHANNEL_INFO_T *pChanInfo = (CHANNEL_INFO_T *)(data+HEADER_SIZE);
			pChanInfo->iChannel = 0xff;
			pChanInfo->iSubChannel = 0;
		}
		datalen = HEADER_SIZE+sizeof(CHANNEL_INFO_T);
		*(DWORD *)(data+4) = sizeof(CHANNEL_INFO_T);
		break;
	case 5:
		data[0] = 0xa6;
		data[8] = 0x0b;
		break;
	case 6:	//刻录进度
		{
			data[8] = 4;
			data[12] = deviceid;
		}
		break;
	case 7: //无线网络信号强度
		{
			data[8] = 13;
		}
		break;
	default:
		return false;
	}
	
	return (sendcammand_dvr2(device, data, datalen) >= 0);
}

bool sendCapture_dvr2(CDvrDevice* device, int channel, int pipeno, bool bstart)
{
    unsigned char data[HEADER_SIZE + 16];
	memset(data, 0, HEADER_SIZE + 16);
	
	data[0] = 0x11;
	
    int channelcount = device->GetVideoChannelCount();
	channelcount = channelcount>16?16:channelcount;
	CDvrChannel *pch = 0;
    for (int i=0; i<channelcount; i++)
    {
		{
			pch = device->device_get_channel(AFK_CHANNEL_TYPE_MEDIA, i);
			data[8 + i] = (pch != 0);
			//for subtype
			if (pch)
			{
				data[HEADER_SIZE + i] = ((CDvrMediaChannel *)(pch))->GetChannelSubtype();
			}
			if (pch)
			{
				pch->channel_decRef();
			}
		}
		if (channel == i)
		{
			data[8 + i] = bstart;
			data[HEADER_SIZE + i] = 4; //抓图为4号副码流
		}
    }
    CDvrPreviewChannel *pPreviewChannel = 
        (CDvrPreviewChannel*)device->device_get_channel(AFK_CHANNEL_TYPE_PREVIEW, channelcount);
    if (pPreviewChannel)
    {
        int type1, type2;
        pPreviewChannel->GetPreviewType(&type1, &type2);
        //多画面预览中的几画面值(1, 4, 9, 16)
        data[24] = type1;
        //多画面预览中的几画面的相应模式(1:16, 4:4, 9:2, 16:1)
        data[25] = type2;
		pPreviewChannel->channel_decRef();
    }
	
	(*(DWORD*)(data+4)) = 16;
	
	return (sendcammand_dvr2(device, data, HEADER_SIZE + 16) >= 0);
}

bool sendSetupChannelOsdString_dvr2(CDvrDevice* device, int channelno, int enable, int* position, unsigned char *buf, int buflen)
{
	if (buflen != 6*20)
	{
		return false;
	}
	
	unsigned char *data = new unsigned char[HEADER_SIZE + 20];
	if (!data)
	{
		return false;
	}
    memset(data, 0, HEADER_SIZE + 20);
	
    data[0] = 0x61;
    data[8] = 1;
	data[16] = channelno;	//通道号
	
	bool b = false;
	for (int i=0; i<6; i++)
	{
		data[20] = enable!=0 ? 1 : 0;	//叠加使能
		if (strlen((char*)(buf+20*i))>0)
		{
			*(unsigned long*)(data + 4) = 20;
			data[24] = position[i];	//位置
			data[12] = i;	//行号
			memset(data + HEADER_SIZE, 0, 20);
			memcpy(data + HEADER_SIZE, buf+20*i, 20);
			b = (sendcammand_dvr2(device, data, HEADER_SIZE + 20) >= 0);
		}
		else
		{
			*(unsigned long*)(data + 4) = 0;
			data[12] = i;	//行号
			data[20] = 0;
			b = (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
		}
		
		if (!b)
		{
			break;
		}
	}
	
	delete[] data;
    return b;
}

bool sendHeartBeat_dvr2(CDvrDevice* device)
{
	unsigned char heartbeat[HEADER_SIZE];
    memset(heartbeat, 0, HEADER_SIZE);
	heartbeat[0] = 0xA1;
	
	return (sendcammand_dvr2(device, heartbeat, HEADER_SIZE) >= 0);
}

bool sendQueryTransCom_dvr2(CDvrDevice* device, int nTransComType)
{
	if(nTransComType != 0 && nTransComType != 1)
	{
		return false;
	}

	unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);
	data[0] = 0x20;
	data[9] = 5;
	data[8] = nTransComType+1;

	return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
}

bool sendRequestPacket_dvr2(CDvrDevice* device, int type, int nSequence, int subtype, void *pCondition)
{
	switch(type)
	{
	case AFK_REQUEST_DECODER_CTRLTV:	// 控制TV画面
		{
			REQUEST_DEC_CTRLTV *pRequestCondition = (REQUEST_DEC_CTRLTV *)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xC1;
			data[16] = 0xF7;
			
			sprintf((char*)(data+HEADER_SIZE), "ProtocolType:CfgTVCombin\r\nPacketSequence:%d\r\nDisTV:%d\r\nDisMode:%d\r\n", 
				nSequence, pRequestCondition->nMonitorID, pRequestCondition->nSplitType);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			for (int i = 0; i < pRequestCondition->nSplitType; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "DisChn:%d\r\n",  pRequestCondition->pEncoderChannel[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "Enable:%s\r\n\r\n", pRequestCondition->bEnable?"True":"False");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DECODER_SWITCH:	// 切换TV图象
		{
			REQUEST_DEC_SWITCHTV *pRequestCondition = (REQUEST_DEC_SWITCHTV *)pCondition;
			int nExtLen = 0;

			char szServiceType[8] = {0};
			char szProtoType[64] = {0};
			char szVideoType[64] = {0};
			char szManuFactory[128] = {0};
			char szDeviceType[128] = {0};
			char szStreamType[64] = {0};
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xC1;
			data[16] = 0xF7;
			
			if (pRequestCondition->byConnType == 0)
			{
				strcpy(szServiceType, "TCP");
			}
			else if (pRequestCondition->byConnType == 1)
			{
				strcpy(szServiceType, "UDP");
			}
			else if (pRequestCondition->byConnType == 2)
			{
				strcpy(szServiceType, "MCAST");
			}
			
			// 码流类型
			if (pRequestCondition->nStreamType == 0)
			{
				strcpy(szStreamType, "main");
			}
			else if (pRequestCondition->nStreamType == 1)
			{
				strcpy(szStreamType, "second");
			}
			else
			{
				strcpy(szStreamType, "snap");
			}

			// 协议类型
			if(pRequestCondition->dwProtoType == 1)
			{
				strcpy(szProtoType, "ProtoType:DahuaII\r\n");
			}
			else if(pRequestCondition->dwProtoType == 2)
			{
				strcpy(szProtoType, "ProtoType:DahuaXTJC\r\n");
			}
			else if(pRequestCondition->dwProtoType == 3)
			{
				strcpy(szProtoType, "ProtoType:DahuaDSS\r\n");
			}
			else if(pRequestCondition->dwProtoType == 4)
			{
				strcpy(szProtoType, "ProtoType:DahuaRtsp\r\n");
			}
			else
			{
				strcpy(szProtoType, "");
			}	

			//byVideoInType
			if (pRequestCondition->byVideoInType == 0) 
			{
				strcpy(szVideoType, "VideoIn_Type:SD\r\n");
			}
			else if (pRequestCondition->byVideoInType == 1) 
			{
				strcpy(szVideoType, "VideoIn_Type:HD\r\n");
			}

			//byManuFactory
			strcpy(szManuFactory, "ManuFactory:Private\r\n");
			if (pRequestCondition->byManuFactory != 0) 
			{
				//strcpy(szManuFactory, "ManuFactory:Private\r\n");
			}
		
			//byDeviceType
			strcpy(szDeviceType, "DeviceType:IPC\r\n");
			if (pRequestCondition->byDeviceType != 0) 
			{
				//strcpy(szDeviceType, "DeviceType:IPC\r\n");
			}
		
			sprintf((char*)(data+HEADER_SIZE), 
				"ProtocolType:CfgDigitalChn\r\nPacketSequence:%d\r\nDisChn:%d\r\nEnable:%s\r\nIP:%s\r\nPort:%d\r\nDstChn:%d\r\nDstPicType:%s\r\nUserName:%s\r\nPassWord:%s\r\nMode:%s\r\nServiceType:%s\r\nServicePort:%d\r\n%sDeviceName:%s\r\n%sSnapMode:%d\r\n%s%s\r\n", 
				nSequence, pRequestCondition->nEncoderID, pRequestCondition->wEnable? "True":"False", pRequestCondition->szDevIp, pRequestCondition->wDevPort, 
				pRequestCondition->nDevChannel, szStreamType, 
				pRequestCondition->szDevUser, pRequestCondition->szDevPwd, pRequestCondition->byWorkMode==0?"DirectConn":"Transmit", 
				szServiceType, pRequestCondition->wListenPort,
				szProtoType, pRequestCondition->szDevName,szVideoType,pRequestCondition->bySnapMode,
				szManuFactory, szDeviceType);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_QUERY_TV:		// 查询TV信息
		{
			int nMonitorID = (int)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xA3;
			data[16] = 0xF7;
			
			sprintf((char*)(data+HEADER_SIZE), "ProtocolType:CfgTVCombin\r\nPacketSequence:%d\r\nDisTV:%d\r\n\r\n", 
				nSequence, nMonitorID);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_QUERY_CHANNEL:	// 查询解码通道信息
		{
			int nDecoderChannel = (int)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xA3;
			data[16] = 0xF7;
			
			sprintf((char*)(data+HEADER_SIZE), "ProtocolType:CfgDigitalChn\r\nPacketSequence:%d\r\nDisChn:%d\r\n\r\n", 
				nSequence, nDecoderChannel);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_QUERY_SYS:		// 查询解码器系统信息
		{
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE),
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.CfgTv\r\n\r\n,",nSequence);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_QUERY_CHNNUM:	// 查询解码器解码通道个数
		{
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE),
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Decode.Cfg\r\n\r\n",nSequence);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_ADD_COMBIN:	// 增加TV画面组合
		{
			REQUEST_DEC_ADD_COMBIN *pRequestCondition = (REQUEST_DEC_ADD_COMBIN *)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:AddObject\r\nParameterName:Dahua.Device.VideoOut.TV.MultiCombin\r\nTV:%d\r\nDisMode:%d\r\n",
				nSequence,pRequestCondition->nMonitorID,pRequestCondition->nSplitType);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			for (int i = 0; i<pRequestCondition->nSplitType; i++)
			{	
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "DisChn:%d\r\n", pRequestCondition->pEncoderChannnel[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen),"\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_DEL_COMBIN:	// 删除TV画面组合
		{
			REQUEST_DEC_DEL_COMBIN *pRequestCondition = (REQUEST_DEC_DEL_COMBIN *)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:DeleteObject\r\nParameterName:Dahua.Device.VideoOut.TV.MultiCombin\r\nTV:%d\r\nCombinID:%d\r\n\r\n",
				nSequence,pRequestCondition->nMonitorID,pRequestCondition->nCombinID);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_SET_COMBIN:	// 修改TV画面组合
		{
			REQUEST_DEC_SET_COMBIN *pRequestCondition = (REQUEST_DEC_SET_COMBIN *)pCondition;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			int nExtLen = 0;
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:AddObject\r\nParameterName:Dahua.Device.VideoOut.TV.MultiCombin\r\nTV:%d\r\nCombinID:%d\r\nDisMode:%d\r\n",
				nSequence,pRequestCondition->nMonitorID, pRequestCondition->nCombinID, pRequestCondition->nSplitType);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			for (int i = 0; i < pRequestCondition->nSplitType; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "DisChn:%d\r\n", pRequestCondition->pEncoderChannnel[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen),"\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_QUERY_COMBIN:	// 查询TV画面组合
		{
			REQUEST_DEC_DEL_COMBIN *pRequestCondition = (REQUEST_DEC_DEL_COMBIN *)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			sprintf((char *)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.MultiCombin\r\nTV:%d\r\nCombinID:%d\r\n\r\n",
				nSequence, pRequestCondition->nMonitorID, pRequestCondition->nCombinID);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_SET_TOUR:		// 设置解码器轮巡
		{
			REQUEST_DEC_SET_TOUR * pRequestCondition = (REQUEST_DEC_SET_TOUR *)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 2048] = {0};
			data[0] = 0xF4;
			sprintf((char *)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.Scout.CfgScout\r\nTV:%d\r\nInterval:%d\r\n",
				nSequence, pRequestCondition->nMonitorID, pRequestCondition->nTourTime);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			for (int i = 0; i < pRequestCondition->nCombinCount; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "ComBinID:%d\r\n", pRequestCondition->pTourCombin[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			for (int j = 0; j < pRequestCondition->nCombinCount; j++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "CombinState:%s\r\n", pRequestCondition->pCombinState[j]? "True":"False");
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen),"\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_QUERY_TOUR:	// 查询解码器轮巡
		{
			int nMonitorID = (int)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.Scout.CfgScout\r\nTV:%d\r\n\r\n",
				nSequence, nMonitorID);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_SET_TV_ENABLE:	// 设置解码器TV使能
		{
			REQUEST_DEC_SET_TVENABLE *pRequestCondition = (REQUEST_DEC_SET_TVENABLE *)pCondition;
			int nExtLen = 0;
			
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			sprintf((char *)(data+HEADER_SIZE),
				"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.CfgTv\r\n",nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			for (int i = 0; i < pRequestCondition->nBuflen; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "TVEnable:%s\r\n",  pRequestCondition->pSetTVEnable[i] ? "True":"False");
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen),"\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_PLAYBACKBYDEC:
		{
			switch(subtype)
			{
			case AFK_REQUEST_DEC_BYDECBYFILE:	// 按文件回放前端设备
				{
					REQUEST_DEC_DEVBYFILE *pRequestCondition = (REQUEST_DEC_DEVBYFILE *)pCondition;
					int nExtLen = 0;
	
					unsigned char data[HEADER_SIZE + 512] = {0};
					data[0] = 0xF4;

					sprintf((char*)(data+HEADER_SIZE), 
						"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Decode.ControlPlayback.Start\r\nProtoType:DahuaII\r\nMode:ByFile\r\n\
						nDisChn:%d\r\nIP:%s\r\nPort:%d\r\nDstChn:%d\r\nDstPicType:%s\r\nUserName:%s\r\nPassWord:%s\r\nRecChn:%d\r\nRecDiskNO:%d\r\nRecClusNo:%d\r\n\
						RecStartTime:%d-%d-%d-%d-%d-%d\r\nRecEndTime:%d-%d-%d-%d-%d-%d\r\nRecHint:%d\r\nRecFileLength:%d\r\nRecFileType:%s\r\n\r\n", 
						nSequence, pRequestCondition->nEncoderID, pRequestCondition->szDevIp, pRequestCondition->wDevPort, pRequestCondition->nChannelID, "main", 
						pRequestCondition->szDevUser, pRequestCondition->szDevPwd, pRequestCondition->nChannelID, pRequestCondition->driveno, pRequestCondition->startcluster,
						pRequestCondition->startTime.year+2000, pRequestCondition->startTime.month, pRequestCondition->startTime.day, pRequestCondition->startTime.hour, pRequestCondition->startTime.minute, pRequestCondition->startTime.second, 
						pRequestCondition->endTime.year+2000, pRequestCondition->endTime.month, pRequestCondition->endTime.day, pRequestCondition->endTime.hour, pRequestCondition->endTime.minute, pRequestCondition->endTime.second, 
						pRequestCondition->bHint, pRequestCondition->nFileSize, pRequestCondition->nRecordFileType==0?"R":"A");
					nExtLen = strlen((char*)(data+HEADER_SIZE));
			
					*(DWORD*)(data + 4) = nExtLen;
					return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
				}
				break;
			case AFK_REQUEST_DEC_BYDECBYTIME:	// 按时间回放前端设备
				{
					REQUEST_DEC_DEVBYTIME *pRequestCondition = (REQUEST_DEC_DEVBYTIME *)pCondition;
					int nExtLen = 0;

					unsigned char data[HEADER_SIZE + 512] = {0};
					data[0] = 0xF4;

					sprintf((char*)(data+HEADER_SIZE), 
						"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Decode.ControlPlayback.Start\r\nProtoType:DahuaII\r\nMode:ByTime\r\n\
						nDisChn:%d\r\nIP:%s\r\nPort:%d\r\nDstChn:%d\r\nDstPicType:%s\r\nUserName:%s\r\nPassWord:%s\r\nRecChn:%d\r\nRecDiskNO:0\r\nRecClusNo:0\r\n\
						RecStartTime:%d-%d-%d-%d-%d-%d\r\nRecEndTime:%d-%d-%d-%d-%d-%d\r\nRecHint:0\r\n\r\n", 
						nSequence, pRequestCondition->nEncoderID, pRequestCondition->szDevIp, pRequestCondition->wDevPort, pRequestCondition->nChannelID, "main", 
						pRequestCondition->szDevUser, pRequestCondition->szDevPwd, pRequestCondition->nChannelID, 
						pRequestCondition->startTime.year+2000, pRequestCondition->startTime.month, pRequestCondition->startTime.day, pRequestCondition->startTime.hour, pRequestCondition->startTime.minute, pRequestCondition->startTime.second, 
						pRequestCondition->endTime.year+2000, pRequestCondition->endTime.month, pRequestCondition->endTime.day, pRequestCondition->endTime.hour, pRequestCondition->endTime.minute, pRequestCondition->endTime.second);
					nExtLen = strlen((char*)(data+HEADER_SIZE));
			
					*(DWORD*)(data + 4) = nExtLen;
					return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
				}
				break;
			default:
				break;
			}
		}
		break;
	case AFK_REQUEST_DEC_CTRLPLAYBACK:
		{
			REQUEST_DEC_SET_PLAYBACK *pRequestCondition = (REQUEST_DEC_SET_PLAYBACK *)pCondition;
			int nExtLen = 0;
			switch(pRequestCondition->nType)
			{
			case 0:
				{
					unsigned char data[HEADER_SIZE];
					memset(data, 0, HEADER_SIZE);
					
					data[0] = 0xCA;
					data[8] = pRequestCondition->nEncoderNum + 1;
					unsigned int *offsettime = (unsigned int*)&data[9];
					*offsettime = pRequestCondition->nValue;
					unsigned int *offsetdata = (unsigned int*)&data[16];
					*offsetdata = pRequestCondition->nValue;
					
					return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
				}
				break;
			case 1:
				{
					unsigned char data[HEADER_SIZE + 128] = {0};
					data[0] = 0xF4;

					sprintf((char*)(data+HEADER_SIZE), 
						"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Decode.ControlPlayback.Resume\r\nProtoType:DahuaII\r\nDisChn:%d\r\n\r\n", 
						nSequence, pRequestCondition->nEncoderNum);
					nExtLen = strlen((char*)(data+HEADER_SIZE));
			
					*(DWORD*)(data + 4) = nExtLen;
					return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
				}
				break;
			case 2:
				{
					unsigned char data[HEADER_SIZE + 128] = {0};
					data[0] = 0xF4;

					sprintf((char*)(data+HEADER_SIZE), 
						"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Decode.ControlPlayback.Pause\r\nProtoType:DahuaII\r\nDisChn:%d\r\n\r\n", 
						nSequence, pRequestCondition->nEncoderNum);
					nExtLen = strlen((char*)(data+HEADER_SIZE));
			
					*(DWORD*)(data + 4) = nExtLen;
					return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
				}
			    break;
			case 3:
				{
					unsigned char data[HEADER_SIZE + 128] = {0};
					data[0] = 0xF4;

					sprintf((char*)(data+HEADER_SIZE), 
						"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Decode.ControlPlayback.Stop\r\nProtoType:DahuaII\r\nDisChn:%d\r\n\r\n", 
						nSequence, pRequestCondition->nEncoderNum);
					nExtLen = strlen((char*)(data+HEADER_SIZE));
			
					*(DWORD*)(data + 4) = nExtLen;
					return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
				}
			    break;
			default:
			    break;
			}
		}
		break;
	case AFK_REQUEST_GET_IPFILTER:		// 查询IP地址过滤
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Network.IPFilter\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_IPFILTER:		// 修改IP地址过滤
		{
			int nExtLen = 0;
			REQUEST_SET_IPFILTER *pRequestCondition = (REQUEST_SET_IPFILTER *)pCondition;
			unsigned char data[HEADER_SIZE + 10000] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Network.IPFilter\r\nEnable:%d\r\nType:%d\r\n", 
				nSequence, pRequestCondition->nEnable, pRequestCondition->nType);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			//白名单
			for (int i = 0; i < pRequestCondition->nTrustIpNum; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "TrustIp:%s\r\n",  pRequestCondition->szTrustIp[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			//黑名单
			for (int j = 0; j < pRequestCondition->nBlackIpNum; j++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "BannedIp:%s\r\n",  pRequestCondition->szBlackIp[j]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_ADD_IPFILTER:		// 增加IP地址过滤
		{
			int nExtLen = 0;
			REQUEST_ADD_IPFILTER *pRequestCondition = (REQUEST_ADD_IPFILTER *)pCondition;
			unsigned char data[HEADER_SIZE + 256] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:AddObject\r\nParameterName:Dahua.Device.Network.IPFilter\r\n\
Enable:%d\r\nType:%d\r\nIp:%s\r\nList:%s,%s\r\n\r\n", 
nSequence, pRequestCondition->nEnable, pRequestCondition->nType, pRequestCondition->szIp, pRequestCondition->szListStart, pRequestCondition->szListEnd);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEL_IPFILTER:		// 删除IP地址过滤
		{
			int nExtLen = 0;
			REQUEST_ADD_IPFILTER *pRequestCondition = (REQUEST_ADD_IPFILTER *)pCondition;
			unsigned char data[HEADER_SIZE + 256] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:DeleteObject\r\nParameterName:Dahua.Device.Network.IPFilter\r\n\
Enable:%d\r\nType:%d\r\nIp:%s\r\nList:%s,%s\r\n\r\n", 
nSequence, pRequestCondition->nEnable, pRequestCondition->nType, pRequestCondition->szIp, pRequestCondition->szListStart, pRequestCondition->szListEnd);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;	
	case AFK_REQUEST_GET_POS_CFG:        //query pos config jfchen
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 256] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence , pCondition );
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break ;
	case AFK_REQUEST_SET_POS_CFG:
		{
			int nExtLen = 0;
			REQUEST_POS_INFO *pRequestCondition = (REQUEST_POS_INFO *)pCondition;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, pCondition );
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break ;
	case AFK_REQUEST_GET_AMRENCODE_CFG:	// 查询AMR编码配置
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Audio.Talkback.Cfg\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_AMRENCODE_CFG:	// 设置AMR编码配置
		{
			int nExtLen = 0;
			REQUEST_SET_TALKENCODE *pRequestCondition = (REQUEST_SET_TALKENCODE *)pCondition;
			unsigned char data[HEADER_SIZE + 256] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Audio.Talkback.Cfg\r\n\
				Compression:%d\r\nMode:%d\r\n\r\n", nSequence, pRequestCondition->nCompression, pRequestCondition->nMode);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_RECORDLEN_CFG:	// 查询录像打包长度配置
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.General.RecordLen\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_RECORDLEN_CFG:	// 设置录像打包长度配置
		{
			int nExtLen = 0;
			REQUEST_SET_RECORDLEN *pRequestCondition = (REQUEST_SET_RECORDLEN *)pCondition;
			unsigned char data[HEADER_SIZE + 256] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Record.General.RecordLen\r\nnType:%d\r\nnValue:%d\r\n\r\n",
				nSequence, pRequestCondition->nType, pRequestCondition->nValue);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;

	case AFK_REQUEST_AUTOREGISTER_GETCONID:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Network.CtrolRegister.Choose\r\n\r\n",nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_AUTOREGISTER_GETSERVER:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			int nConnectionID = (int)pCondition;
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Network.ControlConnection.Port\r\nConnectionID:%d\r\n\r\n",
				nSequence, nConnectionID);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_AUTOREGISTER_REGSERVER:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			int nConnectionID = (int)pCondition;
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Network.CtrolRegister.Choose\r\nConnectionID:%d\r\n\r\n",
				nSequence, nConnectionID);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_VIDEOFORMAT_INFO://查询支持的视频制式
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterNames\r\nParameterName:Dahua.Device.VideoOut.General\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_BURNFILE_TRANS:
		{
			int nExtLen = 0;
			REQUEST_SET_BURNFILE *pRequestCondition = (REQUEST_SET_BURNFILE *)pCondition;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;

			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Extra\r\nConnectionID:%d\r\nFileName:%s\r\nFileSize:%d\r\n\r\n",
				nSequence,
				pRequestCondition->dwConID,
				pRequestCondition->szFileName, 
				pRequestCondition->nFileSize);
			nExtLen = strlen((char*)(data+HEADER_SIZE));

			*(DWORD*)(data+4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_BURNFILE_TRANS:
		{
			int nExtLen = 0;
			REQUEST_SET_BURNFILE *pRequestCondition = (REQUEST_SET_BURNFILE *)pCondition;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Extra\r\n\r\n", nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data+4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_CONNECT:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), 
			"TransactionID:%d\r\nMethod:AddObject\r\nParameterName:Dahua.Device.Network.ControlConnection.Passive\r\nConnectProtocol:%d\r\n\r\n",
				nSequence, (int)pCondition);
			nExtLen = strlen((char*)(data+HEADER_SIZE));

			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
			
		}
		break;
	case AFK_REQUEST_DISCONNECT:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
			"TransactionID:%d\r\nMethod:DeleteObject\r\nParameterName:Dahua.Device.Network.ControlConnection.Passive\r\nConnectionID:%d\r\n\r\n",
					nSequence, (int)pCondition);
			nExtLen = strlen((char *)(data+HEADER_SIZE));

			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_BURN_ATTACH:
		{
			int nExtLen = 0;
			BOOL *pRequestCondition = (BOOL *)pCondition;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Extra.Option\r\nEnableExBackup:%d\r\n\r\n",
				nSequence,
				*pRequestCondition == 0 ? 0 : 1);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_BURN_ATTACH:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Extra.Option\r\n\r\n",
				nSequence);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_SNIFFER:
	case AFK_REQUEST_GET_SNIFFER:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE+1024*16] = {0};
			data[0] = 0xf4;

			sprintf((char *)(data+HEADER_SIZE),
					"TransactionID:%d\r\n%s",
					nSequence,
					(char *)pCondition);

			nExtLen = strlen((char *)(data+HEADER_SIZE));

			*(DWORD *)(data +4) = nExtLen;

			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_MMS_INFO:		// 查询MMS
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Network.Mobile.EventMessageSending.Cfg\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_MMS_INFO:		// 修改MMS
		{
			int nExtLen = 0;
			REQUEST_SET_MMS *pRequestCondition = (REQUEST_SET_MMS *)pCondition;
			unsigned char data[HEADER_SIZE + 10000] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Network.Mobile.EventMessageSending.Cfg\r\nEnable:%d\r\n", 
				nSequence, pRequestCondition->nEnable);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "Type:%d\r\n",  pRequestCondition->bType);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "Title:%s\r\n",  pRequestCondition->SZTitle);
			nExtLen = strlen((char*)(data+HEADER_SIZE));

			//接收者
			for (int i = 0; i < pRequestCondition->nReceiverNum; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "Receivers:%s\r\n",  pRequestCondition->szReceiver[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
	
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;	
	case AFK_REQUEST_GET_SMSACTIVE_INFO:		// 查询SMSACTIVATION
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Network.Mobile.SMSActivation.Cfg\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_SMSACTIVE_INFO:		// 修改SMSACTIVATION
		{
			int nExtLen = 0;
			REQUEST_SET_SMSACTIVATION *pRequestCondition = (REQUEST_SET_SMSACTIVATION *)pCondition;
			unsigned char data[HEADER_SIZE + 10000] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Network.Mobile.SMSActivation.Cfg\r\nEnable:%d\r\n", 
				nSequence, pRequestCondition->nEnable);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			//sender
			for (int i = 0; i < pRequestCondition->nSenderNum; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "Senders:%s\r\n",  pRequestCondition->szSender[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_DIALIN_INFO:		// 查询DIALINACTIVATION
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 128] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Network.Mobile.DialInActivation.Cfg\r\n\r\n", 
				nSequence);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_DIALIN_INFO:		// 修改DIALINACTIVATION
		{
			int nExtLen = 0;
			REQUEST_SET_DIALINACTIVATION *pRequestCondition = (REQUEST_SET_DIALINACTIVATION *)pCondition;
			unsigned char data[HEADER_SIZE + 10000] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Network.Mobile.DialInActivation.Cfg\r\nEnable:%d\r\n", 
				nSequence, pRequestCondition->nEnable);
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			//caller
			for (int i = 0; i < pRequestCondition->nCallerNum; i++)
			{
				sprintf((char*)(data+HEADER_SIZE+nExtLen), "Callers:%s\r\n",  pRequestCondition->szCaller[i]);
				nExtLen = strlen((char*)(data+HEADER_SIZE));
			}
			
			sprintf((char*)(data+HEADER_SIZE+nExtLen), "\r\n");
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;	
	case AFK_REQUEST_GET_DOWNLOAD_RATE:
		{	
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.NetWork.DownloadBitrate.Cfg\r\n\r\n", 
				nSequence);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_DOWNLOAD_RATE:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			int *pDownLoad = (int *)pCondition;
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.NetWork.DownloadBitrate.Cfg\r\nDownloadRate:%d\r\n\r\n", 
				nSequence, *pDownLoad);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_CHANNEL_STATE:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			int nEncodeID = (int)pCondition;
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Decode.ChannelState\r\nDisChn:%d\r\n\r\n", 
				nSequence, nEncodeID);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);

		}
		break;
	case AFK_REQUEST_SET_VIDEOOUT://设置视频输出
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;

			REQUEST_SET_VIDEOOUT *pVideoOut = (REQUEST_SET_VIDEOOUT*)pCondition;
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterNames\r\nParameterName:Dahua.Device.VideoOut.General\r\nDefaultItem:%d\r\nDefaultResolution:%d\r\n\r\n",
				nSequence, pVideoOut->nVideoStander, pVideoOut->nImageSize);

			nExtLen = strlen((char*)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_POINT_CFG:
	case AFK_REQUEST_SET_POINT_CFG:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 2048] = {0};
			data[0] = 0xf4;

			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, (char *)pCondition);

			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_OSDENABLE_CFG:
	case AFK_REQUEST_SET_OSDENABLE_CFG:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 2048] = {0};
			data[0] = 0xf4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, (char *)pCondition);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_LOGOUT:
		{
			int nSessionID = 0;
			device->device_get_info(dit_session_id, &nSessionID);
			unsigned char data[HEADER_SIZE] = {0};
			data[0] = 0x0A;
			*(DWORD*)(data + 8) = nSessionID;

			return (sendcammand_dvr2(device, data, HEADER_SIZE) >= 0);
		}
		break;
	case AFK_REQUEST_GET_TVADJUST_CFG://读取TV调节输出
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			int nTVID = (int)pCondition;
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.Cfg.Adjust\r\nTVNo:%d\r\n\r\n",
				nSequence, nTVID);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_SET_TVADJUST_CFG://设置TV调节输出
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 512] = {0};
			data[0] = 0xF4;
			
			REQUEST_SET_TVADJUST *pTVAdjust = (REQUEST_SET_TVADJUST *)pCondition;
			 sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.VideoOut.TV.Cfg.Adjust\r\nTVNo:%d\r\nTop:%d\r\nBottom:%d\r\nRight:%d\r\nLeft:%d\r\n\r\n",
				 nSequence, pTVAdjust->nTVID, pTVAdjust->nTop, pTVAdjust->nBotton, pTVAdjust->nRight, pTVAdjust->nLeft);
			
			nExtLen = strlen((char*)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_FUNCTION_MASK://功能能力查询
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 1024] = {0};
			data[0] = 0xf4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, (char *)pCondition);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_VEHICLE_CFG:
	case AFK_REQUEST_SET_VEHICLE_CFG:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 1024] = {0};
			data[0] = 0xf4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, (char *)pCondition);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_ATM_OVERLAY_CFG:
	case AFK_REQUEST_SET_ATM_OVERLAY_CFG:
	case AFK_REQUEST_GET_ATM_OVERLAY_ABILITY:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 1024] = {0};
			data[0] = 0xf4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, (char *)pCondition);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_QUERY_BACKUP_DEV: // 查询备份设备列表
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.General\r\n\r\n",
				nSequence);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_QUERY_BACKUP_DEV_INFO: // 查询备份详细信息
		{
			int nExtLen = 0;


			unsigned char data[HEADER_SIZE+1024] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Device\r\nDeviceName:%s\r\n\r\n",
				nSequence, (char *)pCondition);
			
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_BACKUP_START: 
		{
			int nExtLen = 0;

			REQUEST_BACKUP_RECORD *lpRecord = (REQUEST_BACKUP_RECORD *)pCondition;

			unsigned char *data = new unsigned char[512 * lpRecord->nRecordNum];
			if(NULL == data)
			{
				return false;
			}
			memset(data, 0, 512 * lpRecord->nRecordNum);

			data[0] = 0xF4;
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Control.Start\r\nDeviceName:%s\r\n",
				nSequence,lpRecord->szDeviceName);
			
			for (int i =0; i < lpRecord->nRecordNum; i++)
			{
				nExtLen = strlen((char *)(data+HEADER_SIZE));
				BACKUP_RECORDFILE_INFO &recordInfo = lpRecord->stuRecordInfo[i];
				char szAlarm[16] = {0};
				char szType[16] = {0};
				char szStartTime[64] = {0};
				char szEndTime[64] = {0};

				if (4 == recordInfo.byType)
				{
					strncpy(szType,"2",16); // Picture:2

					strncpy(szAlarm,"0",16); // General:0
				}
				else
				{
					strncpy(szType, "1", 16); // video:1
					if (0 == recordInfo.byType)
					{
						strncpy(szAlarm, "0", 16); // General:0
					}
					else if (1 == recordInfo.byType)
					{
						strncpy(szAlarm, "1",16); // Alarm:1
					}
					else if (2 == recordInfo.byType)
					{
						strncpy(szAlarm, "2", 16); // Moving:2
					}
				}

				sprintf(szStartTime, "%d-%d-%d-%d-%d-%d",
						recordInfo.DirStartTime.year+2000,
						recordInfo.DirStartTime.month,
						recordInfo.DirStartTime.day,
						recordInfo.DirStartTime.hour,
						recordInfo.DirStartTime.minute,
						recordInfo.DirStartTime.second);

				sprintf(szEndTime, "%d-%d-%d-%d-%d-%d",
						recordInfo.DirEndTime.year+2000,
						recordInfo.DirEndTime.month,
						recordInfo.DirEndTime.day,
						recordInfo.DirEndTime.hour,
						recordInfo.DirEndTime.minute,
						recordInfo.DirEndTime.second);
	
				sprintf((char *)(data+HEADER_SIZE+nExtLen),
						"Record.Channel[%d]:%d\r\n"\
						"Record.Type[%d]:%s\r\n"\
						"Record.Alarm[%d]:%s\r\n"\
						"Record.StartTime[%d]:%s\r\n"\
						"Record.EndTime[%d]:%s\r\n"\
						"Record.FileSize[%d]:%d\r\n"\
						"Record.DriveNo[%d]:%d\r\n"\
						"Record.Cluster[%d]:%d\r\n"\
						"Record.Hint[%d]:%d\r\n"\
						"Record.ImportantRecID[%d]:%d\r\n",
						i, lpRecord->stuRecordInfo[i].nChannel,
						i, szType,
						i, szAlarm,
						i, szStartTime,
						i, szEndTime,
						i, recordInfo.nFileSize,
						i, recordInfo.nDriveNo,
						i, recordInfo.nStartCluster,
						i, recordInfo.nHInt,
						i, recordInfo.nImportantRecID);
			}

			nExtLen = strlen((char *)(data+HEADER_SIZE));	
			sprintf((char *)(data+HEADER_SIZE+nExtLen),"\r\n");
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD *)(data +4) = nExtLen;

			bool bRet = (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
			if (NULL != data)
			{
				delete [] data;
			}		
			return bRet;
		}
		break;
	case AFK_REQUEST_BACKUP_STOP: 
		{
			int nExtLen = 0;
			
			
			unsigned char data[HEADER_SIZE+1024] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Control.Stop\r\nDeviceName:%s\r\n\r\n",
				nSequence, (char *)pCondition);
			
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
			
		}
		break;
	case AFK_REQUEST_BACKUP_FEEDBACK:  
		{
			int nExtLen = 0;	
			unsigned char data[HEADER_SIZE+1024] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.Record.Backup.Control.Progress\r\nDeviceName:%s\r\n\r\n",
				nSequence, (char *)pCondition);
			
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
			
		}
		break;
	case AFK_REQUEST_ATM_TRADE_TYPE: // ATM设备交易类型查询
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE+256] = {0};
			data[0] = 0xF4;
			
			sprintf((char *)(data+HEADER_SIZE), 
				"TransactionID:%d\r\nMethod:GetParameterValues\r\nParameterName:Dahua.Device.ATM.General\r\n\r\n",
				nSequence);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			
			*(DWORD *)(data +4) = nExtLen;
			return(sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_GET_DECODER_TOUR_CFG:
	case AFK_REQUEST_SET_DECODER_TOUR_CFG:
		{
			int nExtLen = 0;
			unsigned char data[HEADER_SIZE + 4096] = {0};
			data[0] = 0xf4;
			
			sprintf((char*)(data+HEADER_SIZE), "TransactionID:%d\r\n%s", nSequence, (char *)pCondition);
			
			nExtLen = strlen((char *)(data+HEADER_SIZE));
			*(DWORD*)(data + 4) = nExtLen;
			return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
		}
		break;
	case AFK_REQUEST_DEC_CONTROL:
		{
			REQUEST_DEC_CONTROL *pRequestCondition = (REQUEST_DEC_CONTROL *)pCondition;
			int nExtLen = 0;
			switch(pRequestCondition->nType)
			{
			case 0: //tour
				{
					unsigned char data[HEADER_SIZE + 128] = {0};
					data[0] = 0xF4;
					char szAction[64] = {0};

					if (pRequestCondition->nAction == 1)
					{
						strcpy(szAction, "Start");
					}
					else if (pRequestCondition->nAction == 2)
					{
						strcpy(szAction, "Pause");
					}
					else if (pRequestCondition->nAction == 3)
					{
						strcpy(szAction, "Resume");
					}
					else
					{
						strcpy(szAction, "Stop");
					}

					sprintf((char*)(data+HEADER_SIZE), 
						"TransactionID:%d\r\nMethod:SetParameterValues\r\nParameterName:Dahua.Device.Decode.Control\r\nOperateType:Tour\r\nAction:%s\r\nDisChn:%d\r\n\r\n", 
						nSequence, szAction, pRequestCondition->nMonitorID+1);
					nExtLen = strlen((char*)(data+HEADER_SIZE));
			
					*(DWORD*)(data + 4) = nExtLen;
					return (sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen) >= 0);
				}
				break;
			default:
			    break;
			}
		}
		break;
	default:
		break;
	}
	
	return false;
}

//	返回值>=0：发送成功；-1：发送失败；-2：已经断线；-3：发送缓存已满，需sleep再发。
int sendcammand_dvr2(CDvrDevice* device, void *data, int datalen)
{
	CTcpSocket *pTcpSocket = device->GetTcpSocket();
    if (pTcpSocket == NULL)
    {
        return -1;
    }
	
	int nRet = pTcpSocket->WriteData((char *)data, datalen);
	
	return nRet;
}

//新配置发送
int sendNewConfigPacket_dvr2(CDvrDevice* device,int nSequence, void *pCondition)
{	
	int nExtLen = 0;
	unsigned char *data = new unsigned char[HEADER_SIZE + 100*1024];
	if (data)
	{
		memset(data, 0, HEADER_SIZE + 100*1024);
	}
	else
	{
		return -1;
	}

	data[0] = 0xf5;
	
	sprintf((char*)(data+HEADER_SIZE), "%s", (char *)pCondition);
	
	nExtLen = strlen((char *)(data+HEADER_SIZE));
	*(DWORD*)(data + 4) = nExtLen;
	*(DWORD*)(data + 8) = nSequence;

	int nRet = sendcammand_dvr2(device, data, HEADER_SIZE+nExtLen);
	if (data)
	{
		delete []data;
		data = NULL;
	}

	return (nRet >= 0);
	
}

