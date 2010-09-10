#include "mpegaudiodecoder.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

void mpeg_audio_decoder::init() {
	mad_stream_init(&stream);
	mad_synth_init(&synth);
	mad_frame_init(&frame);
}

void mpeg_audio_decoder::finish() {
	mad_frame_finish(&frame);
	mad_synth_finish(&synth);
	mad_stream_finish(&stream);
}

static int transfer(struct mad_pcm *pcm, unsigned char *out_buf);

int mpeg_audio_decoder::decode(char *buf, unsigned len,unsigned char **frame_data, int *frame_len) {
	
	unsigned char *ptr = out;

	if (remain) {
		memmove(buffer,buffer+total_len-remain,remain);
//		printf("remain=%d\n",remain);
	}

	memcpy(buffer+remain,buf,len);
	
	total_len = remain + len;
	remain = 0;
	
	mad_stream_buffer(&stream,buffer,total_len);
	
	while (1) {
		mad_frame_decode(&frame, &stream) ;
		printf("%X %X %X %X\n", buffer[0],buffer[1],buffer[2],buffer[3]);
		
		if (stream.error!=MAD_ERROR_NONE) {
			*frame_data = out;
			*frame_len = 0;
			return 0;
		}
		
		mad_synth_frame(&synth, &frame);
		
		if (stream.error!=MAD_ERROR_NONE) {
			*frame_data = out;
			*frame_len = 0;
			return 0;
		}
		
		len = transfer(&synth.pcm,ptr);
		ptr += len;
		
		remain = stream.bufend-stream.next_frame;
		if (remain<576) {
			break;
		}
	}
	
	*frame_data = out;
	*frame_len = ptr-out;
	
	return 0;
}

static signed int scale(mad_fixed_t sample)
{
	/* round 好象是四舍五入 为了减少误差*/
	sample += (1L << (MAD_F_FRACBITS - 16));
	
	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;
	
	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static int transfer(struct mad_pcm *pcm, unsigned char *out_buf)
{
	unsigned int nchannels, nsamples;
    mad_fixed_t const *left_ch, *right_ch;
	int len;
	
	/* pcm->samplerate contains the sampling frequency */
	
	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];
	
	unsigned char *p = out_buf;
	len = 0;
	
	while (nsamples--) {
		signed int sample;
		
		/* output sample(s) in 16-bit signed little-endian PCM */
		
		sample = scale(*left_ch++);
		*p++ = (sample >> 0) & 0xff;
		len++;
		*p++ = (sample >> 8) & 0xff;
		len++;
		
		if (nchannels == 2) 
		{
			sample = scale(*right_ch++);
			*p++ = (sample >> 0) & 0xff;
			len++;
			*p++ = (sample >> 8) & 0xff;
			len++;
		}
	}

	return len;
}