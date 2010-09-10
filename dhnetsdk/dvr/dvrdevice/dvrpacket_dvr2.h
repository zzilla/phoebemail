/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：协议栈
* 摘　要：负责打成亿蛙协议包。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _DVRPACKET_DVR_H_
#define _DVRPACKET_DVR_H_

#include "../kernel/afkinc.h"

class CDvrDevice;
class CDvrSearchChannel;


//实视监视
bool sendMonitor_dvr2(CDvrDevice* device, int channel, int subtype, bool bstart, 
					  int nConnType, int nConnectID, char* szLocalIp, int nLocalPort);

//查询录像
bool sendQueryRecord_dvr2(CDvrDevice* device, 
                      afk_query_record_s queryrecord, int nParam);

//查询日志
bool sendQueryLog_dvr2(CDvrDevice* device, int logtype, int nParam, char* szPacket = NULL, int nPacketLen = 0);

//解析录像返回
void parseSearchRecordResult_dvr2(CDvrSearchChannel *ch,  
                             unsigned char *recordresult, unsigned int len, 
                             afk_record_file_info_s **p, int &filecount, bool bQueryEndTime, int nParam);

//下载
bool sendDownload_dvr2(CDvrDevice* device, 
                  afk_record_file_info_s recordinfo, bool bstart, int nByTime, int nConnectID, DWORD dwParm = 0);

//导出配置文件
bool sendExportCfgFile_dvr2(CDvrDevice* device, bool bStart, int nParam = 0);

//导入配置文件
bool sendImportCfgFile_dvr2(CDvrDevice* device, afk_upgrade_channel_param_s *ucparam);

int sendImportCfgData_dvr2(CDvrDevice* device, int packetId, char *buf, int len, afk_upgrade_channel_param_s *ucparam);


//下载控制
bool sendDownload_control_dvr2(CDvrDevice* device, int channel,
                               int offset_time, int offset_data, int nConnectID);

//下载继续与暂停
bool sendDownLoad_pause_dvr2(CDvrDevice* device, unsigned int ch, bool pause, int nConnectID);

//发送升级开始命令
//upgraeflag  1:bois  2:web  3:boot
//Edited by Linjy-2006-11-2
bool sendUpgradeControl_dvr2(CDvrDevice* device, afk_upgrade_channel_param_s *ucparam/*, bool bstart*/);

//发送升级信息
int sendUpgradeData_dvr2(CDvrDevice* device, int packetId, char *buf, int len, afk_upgrade_channel_param_s *ucparam);

//发送预览信息
bool sendPreview_dvr2(CDvrDevice* device, int type1, int type2, bool bstart, 
					  int nConnType, int nConnectID, char* szLocalIp, int nLocalPort);

//请求报警信息
bool sendAlarmQuery_dvr2(CDvrDevice* device, bool bRecordState, int proType);

//发送控制信息
bool sendControl_dvr2(CDvrDevice* device, 
                     int ch, int type1, int type2, int parm1, int parm2, int parm3, char *pParam4 = NULL);

//发送控制刻录信息
bool SendControlForBurning(CDvrDevice *pDevice, int nParam1, char *pParam2);

//数据加密
void Encrypt(void *pdata, int len);

//查询通道名
bool sendQueryChannelName_dvr2(CDvrDevice* device);

//查询报警布撤防输入
bool sendQueryAlarmInput_dvr2(CDvrDevice* device);

//查询报警布撤防输出
bool sendQueryAlarmOutput_dvr2(CDvrDevice* device);

//查询无线报警输出
bool sendQueryWirelessAlarmOutput_dvr2(CDvrDevice* device);

//查询报警触发方式
bool sendQueryAlarmTrrigerMode_dvr2(CDvrDevice* device);

//设置报警触发方式
bool sendSetingAlarmTrrigerMode_dvr2(CDvrDevice* device, afk_alarm_trriger_mode_s trrigerinfo);

//解析报警布撤防
void parseSearchAlarmResult_dvr2(CDvrDevice* device, 
                          unsigned char *alarmresult, unsigned int len,
                          afk_alarm_info_s **p);

//设置报警布撤防
bool sendSettingAlarm_dvr2(CDvrDevice* device, afk_alarm_control_info_s alarminfo);

//设置报警解码器输出
bool sendSetingDecoderAlarm_dvr2(CDvrDevice* device, afk_decoder_alarm_info_s dainfo);

//设置透明串口
bool sendSettingSerial_dvr2(CDvrDevice* device, afk_trans_channel_param_s transinfo, bool bstart=true);

//发送透明串口信息
bool sendTransSerialData_dvr2(CDvrDevice* device, int transtype, char *buffer, int buflen);

//发送对讲要求
bool sendTalkRequest_dvr2(CDvrDevice* device, int channel, bool bstart, int encodetype);

//发送对讲信息
bool sendTalkData_dvr2(CDvrDevice* device, int channel, char *buffer, int buflen, int encodetype);

//查询配置
bool sendQueryConfig_dvr2(CDvrDevice* device, unsigned short configtype, int param);

//设置配置
bool sendSetupConfig_dvr2(CDvrDevice* device, unsigned short configtype, int param,
                         unsigned char *buffer, int buflen);

//设置录像状态
bool sendSetupRecordState_dvr2(CDvrDevice* device, unsigned char *buffer, int buflen);

//查询设备时间
bool sendQueryDeviceTime_dvr2(CDvrDevice* device);

//设置设备时间
bool sendSetupDeviceTime_dvr2(CDvrDevice* device, int year, int month, int day,
                              int hour, int minute, int second);

//查询协议(串口协议与解码器协议)
bool sendQueryProtocol_dvr2(CDvrDevice* device, int protocoltype,DWORD dwProtocolIndex =0);

//查询系统信息
bool sendQuerySystemInfo_dvr2(CDvrDevice* device, int type, int param);

//发送用户操作
bool sendUserOperate_dvr2(CDvrDevice* device, int type, char *buffer, int bufferlen);

//强制I桢
bool sendForceIframe_dvr2(CDvrDevice* device,  unsigned char mainchn, unsigned char subchn);

//设置限制码流
bool sendLimitFlux_dvr2(CDvrDevice* device, unsigned short limitflux);

//查询设备工作状态
bool sendQuerydevstate_dvr2( CDvrDevice* device, unsigned char chn, unsigned char type);

//查询设备报警、状态
bool sendQueryAlarmState_dvr2(CDvrDevice* device, int statetype,int deviceid = 0);

//向网络发送数据
int sendcammand_dvr2(CDvrDevice* device, void *data, int datalen);

//远程抓图
bool sendCapture_dvr2(CDvrDevice* device, int channel, int pipeno, bool bstart);

//设置画面OSD叠加
bool sendSetupChannelOsdString_dvr2(CDvrDevice* device, int channelno, int enable, int* position, unsigned char *buf, int buflen);

//发送心跳包
bool sendHeartBeat_dvr2(CDvrDevice* device);

//发送请求包
bool sendRequestPacket_dvr2(CDvrDevice* device, int type, int nSequence, int subtype, void *pCondition);

//发送查询串口参数包
bool sendQueryTransCom_dvr2(CDvrDevice* device, int nTransComType);

//新配置发送
int sendNewConfigPacket_dvr2(CDvrDevice* device,int nSequence, void *pCondition);


#endif // _DVRPACKET_DVR_H_

