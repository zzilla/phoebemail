/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：解码器功能类
* 摘　要：解码器功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DECODERDEVICE_H__BC11FEE5_2969_4260_97B8_6E884CD697AF__INCLUDED_)
#define AFX_DECODERDEVICE_H__BC11FEE5_2969_4260_97B8_6E884CD697AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "netsdk.h"

class CManager;


class CDecoderDevice  
{
public:
	CDecoderDevice(CManager *pManager);
	virtual ~CDecoderDevice();

public:
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 关闭一个设备的所有通道 */
	int							CloseChannelOfDevice(afk_device_s* device);

	/* 同步查询信息 */
	int							SysQueryInfo(LONG lLoginID, int nType, void* pCondition, char *pOutBuffer, int maxlen, int *pRetlen, int waittime=1000);


	/* 同步设置信息 */
	int							SysSetupInfo(LONG lLoginID, int nType, void* pCondition, int waittime);
	
	/* 查询解码器相关信息 */
	int							QueryDecoderInfo(LONG lLoginID, LPDEV_DECODER_INFO lpDecInfo, int waittime);

	/* 查询解码器TV信息 */
	int							QueryDecoderTVInfo(LONG lLoginID, int nMonitorID, LPDEV_DECODER_TV lpMonitorInfo, int waittime);

	/* 查询解码器解码通道信息 */
	int							QueryDecEncoderInfo(LONG lLoginID, int nEncoderID, LPDEV_ENCODER_INFO lpEncoderInfo, int waittime);

	/* 设置解码器TV输出使能*/
	int							SetDecoderTVEnable(LONG lLoginID, BYTE *pDecTVOutEnable, int nBufLen, int waittime);

	/* 设置异步回调函数 */
	int							SetOperateCallBack(LONG lLoginID, fMessDataCallBack cbMessData, DWORD dwUser);

	/* 控制解码器TV画面分割 */
	LONG						CtrlDecTVScreen(LONG lLoginID, int nMonitorID, BOOL bEnable, int nSplitType, BYTE *pEncoderChannel, int nBufLen, void* userdata);

	/* 解码器TV画面切换 */
	LONG						SwitchDecTVEncoder(LONG lLoginID, int nEncoderID, LPDEV_ENCODER_INFO lpEncoderInfo, void* userdata);

	/* 增加画面组合 */          
	int							AddTourCombin(LONG lLoginID, int nMonitorID, int nSplitType, BYTE *pEncoderChannnel, int nBufLen, int waittime);
	
	/* 删除画面组合 */          
	int							DelTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, int waittime);

	/* 修改画面组合 */
	int							SetTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, int nSplitType, BYTE *pEncoderChannel, int nBufLen, int waittime);

	/* 查询画面组合 */          
	int							QueryTourCombin(LONG lLoginID, int nMonitorID, int nCombinID, LPDEC_COMBIN_INFO lpDecCombinInfo, int waittime);

	/* 设置解码器轮巡 */
	int							SetDecoderTour(LONG lLoginID, int nMonitorID, LPDEC_TOUR_COMBIN lpDecTourInfo, int waittime);
	
	/* 查询解码器轮巡状态 */   
	int							QueryDecoderTour(LONG lLoginID, int nMonitorID, LPDEC_TOUR_COMBIN lpDecTourInfo, int waittime);

	/* 解码器TV画面回放 */
	LONG						DecTVPlayback(LONG lLoginID, int nEncoderID, DEC_PLAYBACK_MODE emPlaybackMode, LPVOID lpInBuffer, DWORD dwInBufferSize, void* userdata);

	/* 前端设备按文件方式回放 */
	LONG						PlayBackByDevByFile(LONG lLoginID, int nEncoderID, LPDEC_PLAYBACK_FILE_PARAM lpPlaybackParam, void* userdata);

	/* 前端设备按时间方式回放 */
	LONG						PlayBackByDevByTime(LONG lLoginID, int nEncoderID, LPDEC_PLAYBACK_TIME_PARAM lpPlaybackParam, void* userdata);

	/* 设置回放进度回调函数 */
	int							SetDecPlaybackPos(LONG lLoginID, fDecPlayBackPosCallBack cbPlaybackPos, DWORD dwUser);

	/* 回放控制 */
	int							CtrlDecPlayback(LONG lLoginID, int nEncoderID, DEC_CTRL_PLAYBACK_TYPE emCtrlType, int nValue, int waittime);

	/* 查询解码通道流量*/
	int							QuerChannelFlux(LONG lLoginID, int nEncoderID, DEV_DECCHANNEL_STATE* lpChannelState, int waittime);

	/* 轮巡控制*/
	int							CtrlDecTour(LONG lLoginID, int nMonitorID, DEC_CTRL_TOUR_TYPE emCtrlParm, int waittime);

private:
	CManager*		m_pManager;
};


#endif // !defined(AFX_DECODERDEVICE_H__BC11FEE5_2969_4260_97B8_6E884CD697AF__INCLUDED_)


