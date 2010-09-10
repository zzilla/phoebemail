/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：协议栈
* 摘　要：负责打成亿蛙协议包。
*         不同设备功能上不一样，根据设备类型区分。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRPACKET_COMM_H_
#define _DVRPACKET_COMM_H_

#include "../kernel/afkinc.h"

class CDvrDevice;
class CDvrSearchChannel;

/**********************************
目前需要控制权的功能:
对讲
重传补录
校时
保存系统配置
回放
************************************/

//实视监视
bool sendMonitor_comm(CDvrDevice* device, int channel, int subtype, bool bstart, 
					  int nConnType, int nConnectID, char* szLocalIp, int nLocalPort);

//查询录像
bool sendQueryRecord_comm(CDvrDevice* device, 
                      afk_query_record_s queryrecord, int nParam);

//查询日志
bool sendQueryLog_comm(CDvrDevice* device, int logtype, int nParam, char* szPacket = NULL, int nPacketLen = 0);

//下载
bool sendDownload_comm(CDvrDevice* device, 
                  afk_record_file_info_s recordinfo, bool bstart, int nByTime, int nConnectID, DWORD dwParm = 0);

//导出配置文件
bool sendExportCfgFile_comm(CDvrDevice* device, bool bStart, int nParam = 0);

//导入配置文件
bool sendImportCfgFile_comm(CDvrDevice* device, afk_upgrade_channel_param_s *ucparam);

int sendImportCfgData_comm(CDvrDevice* device, int packetId, char *buf, int len, afk_upgrade_channel_param_s *ucparam);


//下载控制
bool sendDownload_control_comm(CDvrDevice* device, int channel,
                               int offset_time, int offset_data, int nConnectID);

//发送升级准备
//Edited by Linjy-2006-11-2
bool sendUpgradeControl_comm(CDvrDevice* device, afk_upgrade_channel_param_s *ucparam/*, bool bstart*/);

//发送升级信息
int sendUpgradeData_comm(CDvrDevice* device, int packetId, char *buf, int len, afk_upgrade_channel_param_s *ucparam);

//发送预览信息
bool sendPreview_comm(CDvrDevice* device, 
                 int type1, int type2, bool bstart);

//请求报警信息
bool sendAlarmQuery_comm(CDvrDevice* device, bool bRecordState/*该参数没用？*/, int proType = 0);

//发送控制信息
bool sendControl_comm(CDvrDevice* device, 
                      int ch, int type1, int type2, int parm1, int parm2, int parm3, char *pParam4 = NULL);

//查询通道名
bool sendQueryChannelName_comm(CDvrDevice* device);

//查询报警布撤防输入
bool sendQueryAlarmInput_comm(CDvrDevice* device);

//查询报警布撤防输出
bool sendQueryAlarmOutput_comm(CDvrDevice* device);

//查询无线报警输出
bool sendQueryWirelessAlarmOutput_comm(CDvrDevice* device);

//查询报警触发方式
bool sendQueryAlarmTrrigerMode_comm(CDvrDevice* device);

//设置报警触发方式
bool sendSettingAlarmTrrigerMode_comm(CDvrDevice* device, afk_alarm_trriger_mode_s dainfo);

//设置报警布撤防
bool sendSettingAlarm_comm(CDvrDevice* device, afk_alarm_control_info_s alarminfo);

//设置报警解码器输出
bool sendSetingDecoderAlarm_comm(CDvrDevice* device, afk_decoder_alarm_info_s dainfo);

//设置透明串口
bool sendSettingSerial_comm(CDvrDevice* device, afk_trans_channel_param_s transinfo);

//发送透明串口信息
bool sendTransSerialData_comm(CDvrDevice* device, int transtype, char *buffer, int buflen);

//发送对讲要求
bool sendTalkRequest_comm(CDvrDevice* device, int channel, bool bstart, int encodetype=0);

//发送对讲信息
bool sendTalkData_comm(CDvrDevice* device, int channel, char *buffer, int buflen, int encodetype=0);

//转化帧率
int  getframerate_comm(CDvrDevice* device, unsigned char chframerate);

//查询配置
bool sendQueryConfig_comm(CDvrDevice* device, unsigned short configtype, int param);

//设置配置
bool sendSetupConfig_comm(CDvrDevice* device, unsigned short configtype, int param, unsigned char *buffer, int buflen);

//设置录像状态
bool sendSetupRecordState_comm(CDvrDevice* device, unsigned char *buffer, int buflen);

//查询设备时间
bool sendQueryDeviceTime_comm(CDvrDevice* device);

//设置设备时间
bool sendSetupDeviceTime_comm(CDvrDevice* device, int year, int month, int day,
                              int hour, int minute, int second);

//查询协议(串口协议与解码器协议)
bool sendQueryProtocol_comm(CDvrDevice* device, int protocoltype,DWORD dwProtocolIndex=0);

//查询系统信息
bool sendQuerySystemInfo_comm(CDvrDevice* device, int type, int param);

//发送用户操作
bool sendUserOperate_comm(CDvrDevice* device, int type, char *buffer, int bufferlen);

//强制I桢
bool sendForceIframe_comm(CDvrDevice* device,  unsigned char mainchn, unsigned char subchn);

//设置限制码流
bool sendLimitFlux_comm(CDvrDevice* device, unsigned short limitflux);

//查询设备工作状态
bool sendQuerydevstate_comm(CDvrDevice* device, unsigned char chn, unsigned char type);

//通过名字向DDNS查询IP
bool sendQueryIPByName_comm(CDvrDevice* device, char *name, int namelen);

//查询设备报警、状态
bool sendQueryAlarmState_comm(CDvrDevice* device, int statetype,int deviceid = 0);

//远程抓图
bool sendCapture_comm(CDvrDevice* device, int channel, int pipeno, bool bstart);

//设置画面OSD叠加
bool sendSetupChannelOsdString_comm(CDvrDevice* device, int channelno, int enable, int* position, unsigned char *buf, int buflen);

//抓图发送
bool sendSnap_comm(CDvrDevice* device, void* snapParam, unsigned char type);

//Gps报警设置发送
bool sendGpsAlarm_comm(CDvrDevice* device,void* gpsParam);

//发送心跳包
bool sendHeartBeat_comm(CDvrDevice* device);

//发送请求包
bool sendRequestPacket_comm(CDvrDevice* device, int type, int nSequence, int subtype, void *pCondition);

//查询串口状态
bool sendQueryTransCom_comm(CDvrDevice* device, int nTransComType);

//新配置发送
bool sendNewConfigPacket_comm(CDvrDevice* device,int nSequence, void *pCondition);


#endif // _DVRPACKET_COMM_H_



