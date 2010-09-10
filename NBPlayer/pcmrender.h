#ifndef PCMRENDER_H
#define PCMRENDER_H

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>

#define PCM_BUF_COUNT	4
#define BUFFER_SIZE		32768

class CAudioRender
{
public:
	virtual int init(int channels, int samplePerSec, int bitPerSample, HWND hWnd)=0;
	virtual int clean() = 0;
	// 写数据
	virtual int write(unsigned char *pcm, int len) = 0;
	// 跟工作线程相关的方法
	virtual void terminating() = 0; // 中止工作
	virtual int getVolume() = 0;
	virtual void setVolume(int nVol) = 0;
	virtual ~CAudioRender(){} ;
	
	int m_bitsPerSample ;
	int m_samplesPerSecond ;
};

class CPcmRender : public CAudioRender
{
	struct PcmBuffer {
		PcmBuffer() {
			hWaveOut = NULL;
			memset(&header, 0, sizeof(header));
			header.dwFlags |= WHDR_DONE;
		}
		
		DWORD Prepare(HWAVEOUT hOut, unsigned int count)
		{
			Release();
			
			memset(&header, 0, sizeof(header));
			
			header.lpData = data;
			header.dwBufferLength = count;
			header.dwUser = (DWORD)this;

			hWaveOut = hOut;

			return waveOutPrepareHeader(hWaveOut, &header, sizeof(header));
		}

		DWORD Release()
		{
			DWORD err = MMSYSERR_NOERROR;
			if (hWaveOut!=NULL) {
				err = waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));
				if (err== WAVERR_STILLPLAYING) {
					return err;
				}

				hWaveOut = NULL;
			}

			header.dwFlags |= WHDR_DONE;
			
			return err;
		}

		char data[BUFFER_SIZE];
		WAVEHDR header;
		HWAVEOUT hWaveOut;

	};
public:
	CPcmRender();
	~CPcmRender();

	// 参数分别为通道数，采样率
	virtual int init(int channels, int samplePerSec, int bitPerSample, HWND hWnd);
	virtual int clean();

	// 写数据
	virtual int write(unsigned char *pcm, int len);
	virtual int getVolume();
	virtual void setVolume(int nVol);

	// 跟工作线程相关的方法
	virtual void terminating(); // 中止工作
	void loop(); // 开始工作循环（不一定进入线程）
	
private:
	// 内部方法
	void initParam(); // 初始化参数
	void setFormat(int channels, int samplePerSec, int bitPerSample);
	
	void reset(); // 重置缓冲块状态
	int start(); // 启动
	
	int Abort();
private:
	// 公共信息
	int m_nVolumn;

	WAVEFORMATEX m_waveFormat;

    HWAVEOUT m_hWaveOut; // WAVEOUT句柄

	HANDLE m_hEventDone;

	int m_bufferIndex;

	HANDLE m_mutex;
	
	// 播放缓冲区 
	PcmBuffer m_buffer[PCM_BUF_COUNT];
};

#endif /* PCMRENDER_H */
