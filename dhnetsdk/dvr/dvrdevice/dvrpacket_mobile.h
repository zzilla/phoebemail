///////////////////////////////////////////////////////////////////////////////////////////
// 文件名: dvrpacket_mobile.h
// 创建者: jinyx
// 创建时间: 2007-4-2
// 内容描述: mobile dvr的packet
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef DVRPACKETMOBILE_H
#define DVRPACKETMOBILE_H

class CDvrDevice;

//抓图
bool sendSnap_mobile(CDvrDevice* device, void* snapParam, unsigned char type);

//gps发送
bool sendGpsAlarm_mobile(CDvrDevice* device, void* gpsParam);

#endif

