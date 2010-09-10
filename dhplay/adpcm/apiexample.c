/* avcodec API use example.
 *
 * Note that this library only handles codecs (mpeg, mpeg4, etc...),
 * not file formats (avi, vob, etc...). See library 'libav' for the
 * format handling 
 */
#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
//#include <math.h>

#include "avcodec.h"

#define INBUF_SIZE 4096


void audio_encode_example(const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame_size, i, j, out_size, outbuf_size;
    short *samples;
    float t, tincr;
    uint8_t *outbuf;

    codec = avcodec_find_encoder(CODEC_ID_ADPCM_IMA_WAV);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    c= avcodec_alloc_context();
    
    /* put sample parameters */
    c->bit_rate = 64000;//64000;
    c->sample_rate = 8000;//44100;
    c->channels = 1;//2;

    /* open it */
    if (avcodec_open(c, codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }
    
    /* the codec gives us the frame size, in samples */
    frame_size = c->frame_size;
    samples = malloc(frame_size * 2 * c->channels);
    outbuf_size = 10000;
    outbuf = malloc(outbuf_size*2);
        
    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;

    for(i=0;i<200;i++) 
	{
        for(j=0;j<frame_size;j++) 
		{
            samples[2*j] = (int)(sin(t) * 10000);
            samples[2*j+1] = samples[2*j];
            t += tincr;
        }
        out_size = avcodec_encode_audio(c, outbuf, outbuf_size, samples);
//        fwrite(outbuf, 1, out_size, f);

    }

    free(outbuf);
    free(samples);

    avcodec_close(c);
    free(c);
}


int audio_decode_init(AVCodecContext **c)
{
	AVCodec *codec;

	avcodec_init();
    avcodec_register_all();
    codec = avcodec_find_decoder(CODEC_ID_ADPCM_IMA_WAV);
    if (!codec) {
        printf( "codec not found\n");
        return 0;
    }
    *c= avcodec_alloc_context();
    if (avcodec_open(*c, codec) < 0) {
        printf( "could not open codec\n");
        return 0;
    }

	return 1;
}

void audio_decode_example(AVCodecContext *c ,uint8_t *adpcm_buf,int in_size,uint8_t *pcm_buf,int *pcm_size)
{
    int  size, len,out_size;
    uint8_t *inbuf_ptr=adpcm_buf;
    uint8_t *outbuf=pcm_buf;

	*pcm_size=0;
	size=in_size;
    while (size > 0) 
	{
        len = avcodec_decode_audio(c, (short *)outbuf, &out_size, 
                                   inbuf_ptr, size);
        if (len < 0) {
            printf("Error while decoding\n");
            return ;
        }
        if (out_size > 0) {
            /* if a frame has been decoded, output it */
			*pcm_size+=out_size;
			outbuf+=out_size;
        }
        size -= len;
        inbuf_ptr += len;
    }
 }


