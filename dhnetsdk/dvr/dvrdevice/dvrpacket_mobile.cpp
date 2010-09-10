///////////////////////////////////////////////////////////////////////////////////////////
// 文件名: dvrpacket_mobile.cpp
// 创建者: jinyx
// 创建时间: 2007-4-2
//           2007-4-11 增加GPS发送请求
// 内容描述: mobile dvr的packet
///////////////////////////////////////////////////////////////////////////////////////////
#include "../Net/TcpSocket.h"
#include "dvrpacket_mobile.h"
#include "dvrdevice.h"
#include "../def.h"
#include "../kernel/afkinc.h"

/****************************************************************************/

/*****************抓图结构体******************/
//	车载DVR抓图结构体
typedef struct _snap_param
{
   int Channel;        //抓图的通道
   int Quality;       //画质1~6
   int ImageSize;     //画面大小 0:QCIF,1:CIF,2:D1
   int mode;       //抓图模式 0：表示请求一帧,1：表示定时发送请求,2：表示连续请求
   int InterSnap;     //时间单位秒 若mode=1表示定时发送请求时，且为车载设备。此时间有效。
   int CmdSerial;    //请求序列号
   int Reserved[4];
}SNAP_PARAMS_IN,*LPSNAP_PARAMS_IN;

//	普通DVR抓图结构体
typedef struct _snap_param_new
{
   int Channel;        //抓图的通道
   int res[2];       
   int mode;       //抓图模式 0：表示请求一帧,1：表示定时发送请求
   int res1;     
   int CmdSerial;    //请求序列号
   BYTE  Opr;// 0: stop 1:start
   BYTE  Res[3];
   int Reserved[3];
}SNAP_PARAMS_NEW,*LPSNAP_PARAMS_NEW;


/****************************************************************************/

//抓图
bool sendSnap_mobile(CDvrDevice* device, void* snapParam, unsigned char type)
{
	bool bRet = false;

	if (0 == type)		//车载的
	{
		LPSNAP_PARAMS_IN par = (LPSNAP_PARAMS_IN)snapParam;
		int len = sizeof(*par);
		char *pbuf = new char[HEADER_SIZE + len];
		memset(pbuf, 0, HEADER_SIZE + len);
		
		*pbuf = 0x11;                          //抓图命令字
		
		if(par->mode!=-1)
		{
			*(pbuf+8+par->Channel) = 1;            //对应通道置1
		}
		else
		{
			*(pbuf+8+par->Channel) = 0;           //mode = -1 表示停止对应通道的监视发送
		}
		
		*(pbuf + 28) = 10;                     //抓图识别
		
		*(unsigned int*)(pbuf + 4) = len;      //扩展数据包长度
		
		memcpy(pbuf + HEADER_SIZE,par,len);    //扩展区写抓图参数
		
		CTcpSocket *tcp = device->GetTcpSocket();
		if (tcp)
		{
			if (tcp->WriteData(pbuf, HEADER_SIZE + len) >= 0)
			{
				bRet = true;
			}
		}

		delete[] pbuf;
	}
	else		//普通DVR
	{
		LPSNAP_PARAMS_IN lpSnapParam = (LPSNAP_PARAMS_IN)snapParam;
		SNAP_PARAMS_NEW NewSnapParam = {0};

		int nLen = sizeof(SNAP_PARAMS_NEW);
		char *pBuf = new char[HEADER_SIZE + nLen];
		memset(pBuf, 0, HEADER_SIZE + nLen);

		NewSnapParam.Channel = lpSnapParam->Channel;
		NewSnapParam.CmdSerial = lpSnapParam->CmdSerial;
		NewSnapParam.mode = lpSnapParam->mode;

		if (lpSnapParam->mode == -1)
		{
			NewSnapParam.Opr = 0;
		}
		else
		{
			NewSnapParam.Opr = 1;
		}

		pBuf[0] = 0x11;
		pBuf[28] = 10;
		*(unsigned int*)(pBuf + 4) = nLen;
		memcpy(pBuf + HEADER_SIZE, &NewSnapParam, nLen);

		CTcpSocket *tcp = device->GetTcpSocket();
		if (tcp)
		{
			if (tcp->WriteData(pBuf, HEADER_SIZE + nLen) >= 0)
			{
				bRet = true;
			}
		}

		delete[] pBuf;
	}
  
    return bRet;	
}

//GPS发送
bool sendGpsAlarm_mobile(CDvrDevice* device,void* gpsParam)
{
    afk_gps_channel_param_s* parm = (afk_gps_channel_param_s*) gpsParam;

    unsigned char data[HEADER_SIZE];
	memset(data, 0, HEADER_SIZE);

	data[0] = 0x68;
    data[8] = parm->active==0 ? 3 : 2;
	data[12] = 155;
	*(unsigned long *)(data+24) = parm->InterlTime;
	*(unsigned long *)(data+28) = parm->keeptime;

    CTcpSocket *tcp = device->GetTcpSocket();
    if (tcp)
    {
        if (tcp->WriteData((char*)data, HEADER_SIZE) >= 0)
        {
            return true;
        }
    }
    return false;
}


