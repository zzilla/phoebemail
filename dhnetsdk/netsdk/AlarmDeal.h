/*
* Copyright (c) 2009, 浙江亿蛙技术股份有限公司
* All rights reserved.
*
* 类名称：报警处理类
* 摘　要：报警处理功能模块。
*
*/
//////////////////////////////////////////////////////////////////////////

#ifndef _ALARMDEAL_H_
#define _ALARMDEAL_H_

class CManager;

class CAlarmDeal  
{
public:

	/* 报警信息结构 */
	typedef struct
	{
		afk_channel_s  *channel;
	} st_Alarm_Info;


	class SearchAIbydevice;
	friend class SearchAIbydevice;

public:
	CAlarmDeal(CManager *pManager);
	virtual ~CAlarmDeal();

public:
	
	/* 初始化 */
	int							Init(void);

	/* 清理 */
	int							Uninit(void);

	/* 开始帧听 */
	int							StartListen(LONG lLoginID);
	int							StartListenEx(LONG lLoginID);

	/* 停止帧听 */
	int							StopListen(LONG lLoginID);

public:

	static	int	__stdcall		DeviceStateFunc(
									afk_handle_t object,	/* 数据提供者 */
									unsigned char *data,	/* 数据体 */
									unsigned int datalen,	/* 数据长度 */
									void *param,			/* 回调参数 */
									void *userdata);
private:

	list<st_Alarm_Info*>		m_lstAlarm;
	DEVMutex						m_csAlarm;

private:
	CManager*		m_pManager;

};

class CAlarmDeal::SearchAIbydevice
{
	LONG m_ldevice;
public:
	SearchAIbydevice(LONG ldevice):m_ldevice(ldevice){}

	bool operator()(st_Alarm_Info*& ai)
	{
		return m_ldevice == (LONG)(ai?ai->channel->get_device(ai->channel):0);
	}
};


#endif // _ALARMDEAL_H_

