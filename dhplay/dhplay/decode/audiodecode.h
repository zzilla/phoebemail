#ifndef _AUDIODECODE_H
#define _AUDIODECODE_H

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../adpcm/avcodec.h"
#include "../depend/hi_voice_api.h"
#include "../depend/amr_dec.h"

#include "../depend/g723.h"

#include "mpegaudiodecoder.h"

#define MAD_BUF_LEN 576

#define MAX_AUDIO_LEN 8192 

class DhAudioDecoder:public IDecode
{
public:
	DhAudioDecoder();
	virtual ~DhAudioDecoder();

	int init();
	int close();
	int reset() ;
	// enc: 编码类型, sampling: 采样信息
	int decode(unsigned char *src_buf,unsigned int buflen,unsigned char* dest_buf, int param1, int param2); // 解码	
    int resize(int w, int h) ; 

private:
	unsigned char m_buffer[MAX_AUDIO_LEN];
	int m_length;

	AVCodecContext* adpcm;
	void adpcm_decoder(unsigned char *src, char *dest, int srclen, int *dstlen);

	void*		hG723_Dec;
	HI_VOID*	m_HiSi_handle;
	HI_VOID*	m_amr_handle;
	HI_S32		VoiceEngineState[0x100];
	
	mpeg_audio_decoder mad;
	int temp_len;
	char temp_buf[MAD_BUF_LEN*2];

};

#ifdef __cplusplus
}
#endif


#endif /* AUDIODECODE_H */
