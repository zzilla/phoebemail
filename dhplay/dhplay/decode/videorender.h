#ifndef VIDEORENDER_H
#define VIDEORENDER_H

#include "decode.h"
#include <windows.h>
#include "FifoBuffer.h"

#define _TIMER_1 1 //Only 16 timers for every process.Default TIMER;
#define _TIMER_2 2

class DhVideoRender
{
public:
    DhVideoRender();
    virtual ~DhVideoRender();

	DhAVFrame* m_item ;
	FifoBuffer m_YuvDataList;	//已解码数据队列
	FifoBuffer m_PcmDataList;
	int m_ifPause ;//用户设置了暂停
	CRITICAL_SECTION  m_CritSec ;//用于同步定时器回调和清空缓冲操作
	CRITICAL_SECTION  m_ResetTimerCritSec;
	BOOL m_bTimerDisable; //定时器操作开关

	dec_callback m_callback ;//回调指针，回调视频数据

    int init(int port, int imgcount, int chunksize,dec_callback cb);//初始化

    int clear();//清理

	int call_render() ;//定时器消息被触发

    int render();//视频回调

    int changeRate(int rate);//改变帧率
	
	int changeInterval(int interval);

	int getRate();//得到帧率
    
    int startTimer();//起动定时器

    void stopTimer();//停止定时器

	void StopTimerSafely(void);//安全地关闭定时器

	void SetPause(int value) ;//设置暂停

	int   GetPause() ;

	int  StopSoundThread();
	int  StartSoundThread();

	void SetACTimerObj(CAccurateTimer *pACTimer) { m_pACTimer = pACTimer; }
	int  GetTimerType(){return m_timertype;}

	double GetRealFrameBitRate(void) { return m_dRealBitRate; } //Kbps

	void SetIVSCallback(void *pIVSCallback, long userData){
		m_IVSCallback = (IVS_callback)pIVSCallback;
		m_IVS_user = userData;
	}

public:
	int		m_interval;		// 定时器间隔	
	int     m_port ;  //通道号
	int     m_timerId ; // 定时器号
	int     m_rate; //当前帧率
	int		m_timertype;
	HANDLE	m_hTimerThread;
	DWORD	m_preFrameTime;
	HANDLE	m_hMMTimerEvent;
	int		m_iMMTimerSigRef;

	CAccurateTimer *m_pACTimer;
#ifdef THREAD_RENDER
	HANDLE m_hVideoRender;
#endif //THREAD_RENDER

	int		m_iIntervalSum;
	int		m_iFrameLenSum;
	double	m_dRealBitRate;

public:
	int		m_tmpinterval ;
	bool	m_time2_state;
	int		m_flagvalue;
	int		m_flagnum;
	bool    m_audio_state;
	HANDLE  m_audioThread;
	int m_flagcount;
	
	IVS_callback m_IVSCallback; //智能IVS回调
	long m_IVS_user;

private:
	static DWORD  __stdcall timer(LPVOID pParam);
	static DWORD  __stdcall audiothread(LPVOID pParam);
};

#endif /* VIDEORENDER_H */