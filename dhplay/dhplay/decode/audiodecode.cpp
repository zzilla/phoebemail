#include "audiodecode.h"
#include <windows.h>


#define PCM8			7
#define G729			8
#define IMA				9
#define PCM_MULAW		10
#define G721			11
#define PCM8_VWIS		12
#define MS_ADPCM		13
#define G711A			14
#define DHAMR			15
#define PCM16			16

#define HiSi_1			16
#define HiSi_2			17
#define HiSi_3			18
#define HiSi_4			19
#define HiSi_5			20

#define MPEG_AUDIO		21
#define G711U			22
#define G723_1			25


#define TALK_AUDIO		30
#define AVI_AUDIO		31

#define SAMPLE_FREQ_4000	1
#define SAMPLE_FREQ_8000	2
#define SAMPLE_FREQ_11025	3
#define SAMPLE_FREQ_16000	4
#define SAMPLE_FREQ_20000	5
#define SAMPLE_FREQ_22050	6
#define SAMPLE_FREQ_32000	7
#define SAMPLE_FREQ_44100	8
#define SAMPLE_FREQ_48000	9

DhAudioDecoder::DhAudioDecoder()
{
	audio_decode_init(&adpcm);
	mad.init();
	m_HiSi_handle = NULL;
	m_amr_handle = NULL;
	temp_len = 0;

	g723dec_init(&hG723_Dec);
}

DhAudioDecoder::~DhAudioDecoder()
{
	if (adpcm != NULL)
	{
		avcodec_close(adpcm);
		adpcm = NULL;
	}
	mad.finish();

	if (hG723_Dec != NULL)
	{
		g723dec_close(&hG723_Dec);
		hG723_Dec = NULL;
	}
}

int DhAudioDecoder::reset()
{
	if (m_amr_handle != NULL)
	{
		AMR_Decode_Exit(&m_amr_handle);
		m_amr_handle = NULL;
	}

	m_HiSi_handle = NULL;

	return 0 ;
}

int DhAudioDecoder::resize(int w, int h) 
{
	return 0 ;
}

void HiSi_decoder(unsigned char *src, char *dest, int srclen, int *dstlen)
{
	
}

void DhAudioDecoder::adpcm_decoder(unsigned char *src, char *dest, int srclen, int *dstlen)
{
	unsigned char *outbuf=(unsigned char *)dest;
	unsigned char *inbuf_ptr=src;

	int out_size;
	
	audio_decode_example(adpcm ,inbuf_ptr,srclen,outbuf,&out_size);
	
	*dstlen = out_size;
}

static void ms_adpcm_dec(char* src, short* dest, int src_len, int* dest_len);
int g711a_Decode( unsigned char *src, char *dest, int srclen, int *dstlen);
int g711u_Decode(IN const char *src,OUT char *dest,IN int srclen,OUT int *dstlen);

int DhAudioDecoder::decode(unsigned char *sample,unsigned int len,unsigned char* dest_buf,  int param1,  int param2)
{
	int srcLen = 0;

	m_length = 0 ;
	
	if (param1 == MS_ADPCM) 
	{
		if (len >= MAX_AUDIO_LEN/2)
		{
			return 0;
		}

		ms_adpcm_dec((char*)sample, (short *)dest_buf, len, &m_length);
	} 
	else if(param1 == G711A) 
	{
		if (len >= MAX_AUDIO_LEN/2)
		{
			return 0;
		}
		g711a_Decode((unsigned char *)sample,(char *)dest_buf,len,&m_length);
	} 
// 	else if (param1 == PCM8_VWIS) 
// 	{
// 		short * samples=(short *)dest_buf;
// 		short * src_sample=(short *)sample;
// 		
// 		for(int i=0;i<(int)len/2;i++) 
// 		{
// 			samples[i] = src_sample[i]; 
// 		}
// 		m_length = len;
// 	}
	else if(param1 == G721)
	{
		
	}
	else if (param1 == G711U)
	{
 		g711u_Decode((const char*)sample, (char*)dest_buf, len, &m_length);
	}
	else if (param1 == G723_1)
	{
		g723dec(hG723_Dec, (char*)sample, (short*)dest_buf, len, &m_length);
	}
	else if(param1==AVI_AUDIO || param1 == TALK_AUDIO)
	{
		if (len >= MAX_AUDIO_LEN)
		{
			return 0;
		}
		m_length = len;
		memcpy(dest_buf,sample,m_length);
	}
	else if (param1==IMA)
	{
		adpcm_decoder((unsigned char *)sample,(char*)dest_buf,len,&m_length);
	}
	else if (param1 == DHAMR || param1 == HiSi_5)
	{
		if (m_amr_handle == NULL)
		{
			if (AMR_Decode_Init(&m_amr_handle))
			{
				return -1;
			}
		}
		
		short* dest_short = (short*)dest_buf;

		__try
		{
			int iRet = AMR_Decode_Frame(m_amr_handle, sample, dest_short, MIME);
		}
		__except(0,1)
		{
			int wlj = 0;
		}
		
		m_length = L_FRAME*2;
	}//旧码流，PCM16音频类型标识为12，大华新码流PCM16音频类型标识为16
	else if ((param1 == PCM8_VWIS) || (param1 == PCM16))
	{
		if (len >= MAX_AUDIO_LEN)
		{
			return 0;
		}
		m_length = len;
		memcpy(dest_buf,sample,m_length);
	}
	else if (param1 > HiSi_1 && param1 < HiSi_5)
	{
		if (m_HiSi_handle == NULL)
		{
			m_HiSi_handle = (HI_VOID*)VoiceEngineState;
			HI_VOICE_DecReset(m_HiSi_handle, param1-15);
		}

		short* sample_short = (short*)sample;
		short* dest_short = (short*)dest_buf;

		short slen = 0;

		int zgf = HI_VOICE_DecodeFrame(m_HiSi_handle, sample_short, dest_short, &slen);

		m_length = slen*2;
	}
	else if (param1==MPEG_AUDIO) 
	{
		if (temp_len + len > MAD_BUF_LEN*2)
		{
			temp_len = 0;
			m_length = 0;
			return 0;
		}

		memcpy(temp_buf+temp_len,sample,len);
		temp_len+=len;

		if (MAD_BUF_LEN <= temp_len)
		{
			unsigned char *frame_data;
			mad.decode(temp_buf,temp_len,&frame_data,&m_length);
			memcpy(dest_buf, frame_data, m_length);
			temp_len = 0;
		}
		else
		{
			m_length = 0;
		}
	}
	else 
	{
		//安全性检查,因为我们分配的静态buffer为2048;所以必须检查数据源的长度
		if(len > MAX_AUDIO_LEN) 
		{
			return 0;//数据源太长,直接返回
		}

		for (unsigned int i = 0; i<len; ++i)
		{
			dest_buf[i] = sample[i] + 128;
		}

		m_length = len;
	}

	return m_length;
}

/************************************************************************/
/* MS_ADPCM 解码 16bits, 16K采样  */
/************************************************************************/
typedef struct
{
	short prev_sample;
	short quant;
} MS_ADPCM_HANDLE;

static const int adapt_step[] = 
{
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};


/****************************************************************
 * MS_ADPCM 解码一个采样
****************************************************************/
static inline short decode_one_sample(MS_ADPCM_HANDLE* p, int value)
{
	int predictor;
	int sb;

	sb = value & 8;
	value &= 7;
	predictor = p->prev_sample;

	if (sb)
		predictor -= value * p->quant;
	else
		predictor += value * p->quant;

	if (predictor > 32767) 
		predictor = 32767;
	else if (predictor < -32768) 
		predictor = -32768;

	p->prev_sample = predictor;
	p->quant = (adapt_step[value] * p->quant) >> 8;
	if (p->quant < 16) 
		p->quant = 16;

	return (short)predictor;
}

static void ms_adpcm_dec(char* src, short* dest, int src_len, int* dest_len)
{
	MS_ADPCM_HANDLE handle;
	
	int dest_index = 0;
	int src_index = 4;
	
	int code;
	
	handle.prev_sample = *(short*)src;
	handle.quant = *(short*)&src[2];
	
	for (;src_index < src_len;) 
	{
		code = src[src_index++];           /* 得到下一个数据 */
		dest[dest_index++] = decode_one_sample(&handle, code&0xF);
		dest[dest_index++] = decode_one_sample(&handle, (code&0xF0) >> 4);
	}
	
	*dest_len = dest_index*2;
}

/************************************************************************
 * G711 解码
 ************************************************************************/
signed short A2l[256] = 
{
	-5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
	-7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
	-2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
	-3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
	-22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
	-30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
	-11008,-10496,-12032,-11520, -8960, -8448, -9984, -9472,
	-15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
	-344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
	-472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
	-88,   -72,  -120,  -104,   -24,    -8,   -56,   -40,
	-216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
	-1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
	-1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
	-688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
	-944,  -912, -1008,  -976,  -816,  -784,  -880,  -848,
	5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
	7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
	2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
	3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
	22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
	30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
	11008, 10496, 12032, 11520,  8960,  8448,  9984,  9472,
	15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
	344,   328,   376,   360,   280,   264,   312,   296,
	472,   456,   504,   488,   408,   392,   440,   424,
	88,    72,   120,   104,    24,     8,    56,    40,
	216,   200,   248,   232,   152,   136,   184,   168,
	1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
	1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
	688,   656,   752,   720,   560,   528,   624,   592,
	944,   912,  1008,   976,   816,   784,   880,   848,
};


int g711a_Decode(unsigned char *src, char *dest, int srclen, int *dstlen)
{
	int	i;

	unsigned short *pd=(unsigned short*)dest;

	for(i=0; i<srclen; i++)	
	{	
		pd[i]=(unsigned short)A2l[src[i]];
	}

	*dstlen = srclen<<1;

	return 1;
}

signed short _u2l[256] = {

	-32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,

	-23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,

	-15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,

	-11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,

	 -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,

	 -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,

	 -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,

	 -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,

	 -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,

	 -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,

	  -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,

	  -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,

	  -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,

	  -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,

	  -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,

	   -56,   -48,   -40,   -32,   -24,   -16,    -8,    -2,

	 32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,

	 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,

	 15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,

	 11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,

	  7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,

	  5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,

	  3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,

	  2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,

	  1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,

	  1372,  1308,  1244,  1180,  1116,  1052,   988,   924,

	   876,   844,   812,   780,   748,   716,   684,   652,

	   620,   588,   556,   524,   492,   460,   428,   396,

	   372,   356,   340,   324,   308,   292,   276,   260,

	   244,   228,   212,   196,   180,   164,   148,   132,

	   120,   112,   104,    96,    88,    80,    72,    64,

	    56,    48,    40,    32,    24,    16,     8,     2,

};

signed short	ulaw2linear2(unsigned char u)

{

	return(_u2l[u]);

}


int g711u_Decode(IN const char *src,OUT char *dest,IN int srclen,OUT int *dstlen)

{

	int	i;

	*dstlen=0;

	signed short *pd = (signed short*)dest;

	for(i=0; i<srclen; i++)

	{	

		pd[i]=(signed short)ulaw2linear2(src[i]);

		(*dstlen)+=2;

	}

	return 1;

}
