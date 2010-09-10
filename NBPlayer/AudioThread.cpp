
#include "AudioThread.h"
#include "AX_Thread_Guard.h"

CAudioThread::CAudioThread()
{
	m_render = NULL;
	m_id = -1;
}

CAudioThread::~CAudioThread()
{
	stop();
	AX_Guard<AX_Mutex> guard(m_mutex);

	if ( NULL != m_render )
	{
		delete m_render;
	}

	while ( !m_activeFrames.empty() )
	{
		Zeno_Audio_Frame_t* frame = m_activeFrames.front();
		delete frame;
		m_activeFrames.pop_front();
	}

	while ( !m_framePool.empty() )
	{
		Zeno_Audio_Frame_t* frame = m_framePool.front();
		delete frame;
		m_framePool.pop_front();
	}
}

int CAudioThread::openSound(int id)
{
	AX_Guard<AX_Mutex> guard(m_mutex);
	if ( id == m_id )
	{
		//同一个，不用处理
		return 1;
	}

	m_id = id;

	if ( NULL == m_render )
	{
		// 第一次打开
		m_render = new CPcmRender();
		int res = m_render->init(1, 8000, 8, NULL); //先用默认值打开，每次送入数据时，如果发现帧率不符，会重新初始化
		if ( res < 0 )
		{
			delete m_render;
			m_render = NULL;
			return -1;
		}
	}
	else
	{
		//将缓冲清空
		clearFrames();
	}

	start();

	return 0;
}

int CAudioThread::closeSound()
{
	AX_Guard<AX_Mutex> guard(m_mutex);
	if ( m_id != -1 )
	{
		m_id = -1;
		clearFrames();
		//if ( NULL != m_render )
		//{
		//	delete m_render;
		//	m_render = NULL;
		//}
	}
	return 0;
}

int CAudioThread::inputData(int id, char* buf, int len, int samplesPerSecond, int bitsPerSample)
{
	AX_Guard<AX_Mutex> guard(m_mutex);
	if ( id != m_id )
	{
		return -1; //id 不符
	}

	if ( len > AUDIO_BUF_LEN )
	{
		return -2;
	}

	if ( NULL == m_render )
	{
		return -3;
	}

	Zeno_Audio_Frame_t* frame = NULL;

	if ( m_framePool.empty() )
	{
		frame = new Zeno_Audio_Frame_t;
	}
	else
	{
		frame = m_framePool.front();
		m_framePool.pop_front();
	}

	memcpy(frame->data, buf, len);
	frame->len = len;
	frame->samplesPerSecond = samplesPerSecond;
	frame->bitsPerSample = bitsPerSample;

	m_activeFrames.push_back(frame);

	return 0;
}

int CAudioThread::getVolume()
{
	if (m_render)
	{
		return m_render->getVolume();
	}
	return 0;
}

void CAudioThread::setVolume(int nVol)
{
	if (m_render)
	{
		m_render->setVolume(nVol);
	}
}

int CAudioThread::getId()
{
	return m_id;
}

void CAudioThread::clearBuf()
{
	AX_Guard<AX_Mutex> guard(m_mutex);
	if ( m_id != -1 )
	{
		clearFrames();
	}
}

int CAudioThread::run()
{
	m_mutex.acquire();
	if ( NULL == m_render || m_activeFrames.empty() )
	{
		//没有播放对象,或没有数据可以播放
		m_mutex.release();
		Sleep(10);
		return 0;
	}

	Zeno_Audio_Frame_t* frame = m_activeFrames.front();
	m_activeFrames.pop_front();
	m_mutex.release();

	//执行播放
	int trytimes = 0;
	int res = -1;
	do 
	{
		res = renderAudio(frame);
		if ( res < 0 )
		{
			Sleep(100);
		}
	} while (res < 0 && trytimes++ < 2);

	m_mutex.acquire();
	m_framePool.push_back(frame);
	m_mutex.release();

	return 0;
}

int CAudioThread::renderAudio(Zeno_Audio_Frame_t* frame)
{
	if ( frame->samplesPerSecond != m_render->m_samplesPerSecond 
		|| frame->bitsPerSample != m_render->m_bitsPerSample )
	{
		m_render->clean();
		if ( m_render->init(1, frame->samplesPerSecond, frame->bitsPerSample, NULL) < 0 )
		{
			return -1;
		}
	}

	return m_render->write((unsigned char*)frame->data, frame->len);
}

inline void CAudioThread::clearFrames()
{
	while ( !m_activeFrames.empty() )
	{
		m_framePool.push_back(m_activeFrames.front());
		m_activeFrames.pop_front();
	}
}
