/*
* Copyright (c) 2010, 杭州智诺视觉科技有限公司
* All rights reserved.
*
* 文件名称：AudioThread.h
* 文件标识：
* 摘　　要：音频播放线程
*

* 当前版本：1.0
* 原作者　：赵一宇
* 完成日期：2010年5月18日
* 修订记录：创建
*/
#ifndef ZENO_AUDIO_THREAD_H
#define ZENO_AUDIO_THREAD_H

#include "ThreadBase.h"
#include "Singleton.h"
#include "pcmrender.h"
#include "AX_Mutex.h"
#include <list>

#define AUDIO_BUF_LEN 16*1024
typedef struct Zeno_Audio_Frame 
{
	char data[AUDIO_BUF_LEN];
	int  len;
	int  samplesPerSecond;
	int	 bitsPerSample;
}Zeno_Audio_Frame_t;

typedef std::list<Zeno_Audio_Frame_t*> AudioFrameList;

class CAudioThread : public ThreadBase
{
public:
	CAudioThread(void);
	virtual ~CAudioThread(void);

	int openSound(int id);
	int closeSound(void);
	int inputData(int id, char* buf, int len, int samplesPerSecond, int bitsPerSample);
	int getVolume();
	void setVolume(int nVol);
	int getId();
	void clearBuf();

protected:
	virtual int run(void);

	int renderAudio(Zeno_Audio_Frame_t* frame);
	void clearFrames(void);

private:
	CAudioRender*	m_render;		//音频播放
	int				m_id;			//当前播放的ID
	AudioFrameList	m_activeFrames; //等待播放的帧
	AudioFrameList  m_framePool;	//数据池
	AX_Mutex		m_mutex;
};

typedef Singleton<CAudioThread> CAudioThreadSingleton;

#endif
