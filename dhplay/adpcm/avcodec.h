#ifndef AVCODEC_H
#define AVCODEC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32 
	#define inline __inline
#else
	#include "custom_defs.h"
	#define __inline inline
#endif

#ifndef restrict
	#define restrict
#endif

#define always_inline inline

#define uint8_t		unsigned char
#define int8_t		signed char
#define uint16_t	unsigned short
#define int16_t		signed short
#define uint32_t	unsigned int
#define int32_t		signed int

#    define M_PI    3.14159265358979323846
#    define M_SQRT2 1.41421356237309504880  /* sqrt(2) */

enum CodecID {
    CODEC_ID_NONE, 

    /* various adpcm codecs */
    CODEC_ID_ADPCM_IMA_QT,
    CODEC_ID_ADPCM_IMA_WAV,
    CODEC_ID_ADPCM_MS,
};

enum CodecType {
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO,
    CODEC_TYPE_AUDIO,
};

/* currently unused, may be used if 24/32 bits samples ever supported */
enum SampleFormat {
    SAMPLE_FMT_S16 = 0,         /* signed 16 bits */
};

/* in bytes */
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 131072

/**
 * Required number of zero bytes at the end of the input bitstream for decoding.
 * to avoid overreading (and possibly segfaulting)
 */
#define FF_INPUT_BUFFER_PADDING_SIZE 8

#define FF_COMMON_FRAME \
    uint8_t *data[4];\
    int key_frame;\
    int type;\

typedef struct AVFrame {
    FF_COMMON_FRAME
} AVFrame;

typedef struct AVCodecContext {
    int bit_rate;
    int flags;

    /* audio only */
    int sample_rate; /* samples per sec */
    int channels;
    int sample_fmt;  /* sample format, currenly unused */

    /* the following data should not be initialized */
    int frame_size;     /* in samples, initialized when calling 'init' */
    int frame_number;   /* audio or video frame number */

    int delay;
    
    struct AVCodec *codec;
    
    void *priv_data;

    char codec_name[32];
    enum CodecType codec_type; /* see CODEC_TYPE_xxx */
    enum CodecID codec_id; /* see CODEC_ID_xxx */
    unsigned int codec_tag;  /* codec tag, only used if unknown codec */

    int block_align; /* used by some WAV based audio codecs */    

    AVFrame *coded_frame;

} AVCodecContext;

typedef struct AVCodec {
    const char *name;
    int type;
    int id;
    int priv_data_size;
    int (*init)(AVCodecContext *);
    int (*encode)(AVCodecContext *, uint8_t *buf, int buf_size, void *data);
    int (*close)(AVCodecContext *);
    int (*decode)(AVCodecContext *, void *outdata, int *outdata_size,
                  uint8_t *buf, int buf_size);
    int capabilities;
    struct AVCodec *next;
} AVCodec;


/* pcm codecs */
#define PCM_CODEC(id, name) \
extern AVCodec name ## _decoder; \
extern AVCodec name ## _encoder

PCM_CODEC(CODEC_ID_ADPCM_IMA_QT, adpcm_ima_qt);
PCM_CODEC(CODEC_ID_ADPCM_IMA_WAV, adpcm_ima_wav);
PCM_CODEC(CODEC_ID_ADPCM_MS, adpcm_ms);

#undef PCM_CODEC


/* resample.c */

struct ReSampleContext;

typedef struct ReSampleContext ReSampleContext;

ReSampleContext *audio_resample_init(int output_channels, int input_channels, 
                                     int output_rate, int input_rate);
int audio_resample(ReSampleContext *s, short *output, short *input, int nb_samples);
void audio_resample_close(ReSampleContext *s);

extern AVCodec *first_avcodec;

/* returns LIBAVCODEC_VERSION_INT constant */
unsigned avcodec_version(void);
/* returns LIBAVCODEC_BUILD constant */
unsigned avcodec_build(void);

void avcodec_set_bit_exact(void);

void register_avcodec(AVCodec *format);
AVCodec *avcodec_find_encoder(enum CodecID id);
AVCodec *avcodec_find_decoder(enum CodecID id);


AVCodecContext *avcodec_alloc_context(void);
AVFrame *avcodec_alloc_frame(void);

int avcodec_open(AVCodecContext *avctx, AVCodec *codec);
int avcodec_decode_audio(AVCodecContext *avctx, int16_t *samples, 
                         int *frame_size_ptr,
                         uint8_t *buf, int buf_size);

int avcodec_parse_frame(AVCodecContext *avctx, uint8_t **pdata, 
                        int *data_size_ptr,
                        uint8_t *buf, int buf_size);
int avcodec_encode_audio(AVCodecContext *avctx, uint8_t *buf, int buf_size, 
                         const short *samples);

int avcodec_close(AVCodecContext *avctx);

void avcodec_register_all(void);

void avcodec_flush_buffers(AVCodecContext *avctx);

/* memory */
void *av_malloc(unsigned int size);
void *av_mallocz(unsigned int size);
void av_free(void *ptr);
void __av_freep(void **ptr);
#define av_freep(p) __av_freep((void **)(p))

int audio_decode_init(AVCodecContext **c);
void audio_decode_example(AVCodecContext *c ,uint8_t *adpcm_buf,int in_size,uint8_t *pcm_buf,int *pcm_size);

#include "ima_rw.h"
#include "g72x.h"

#endif /* AVCODEC_H */
