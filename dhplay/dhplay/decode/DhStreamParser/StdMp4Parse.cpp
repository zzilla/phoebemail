#include "StdMp4Parse.h"


/*************************************************************************/
/*                                                                       */
/*                         H264 BitStream½âÎöÏà¹Ø                        */
/*												                         */
/*							  2008-1-30                                  */
/*                                                                       */
/*************************************************************************/

int unaligned32_be(const unsigned char *v);

#define OPEN_READER(name, gb)\
        int name##_index= (gb)->index;\
        int name##_cache= 0;\

#define CLOSE_READER(name, gb)\
        (gb)->index= name##_index;\


#   define UPDATE_CACHE(name, gb)\
        name##_cache= unaligned32_be( ((const unsigned char *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#   define SKIP_CACHE(name, gb, num)\
        name##_cache <<= (num);

#    define NEG_USR32(a,s) (((unsigned long)(a))>>(32-(s)))

#   define SKIP_COUNTER(name, gb, num)\
        name##_index += (num);\

#   define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#   define SHOW_UBITS(name, gb, num) NEG_USR32(name##_cache, num)

typedef struct {
    unsigned char *buffer;
	unsigned char *buffer_end;
    int index;
    int size_in_bits;
} GetBitContext;

static unsigned char _ff_golomb_vlc_len[32]={
0, 9, 7, 7, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static unsigned char ff_log2_table[128]={
0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
};


static unsigned char _ff_ue_golomb_vlc_code[256]={ 
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

static char ff_se_golomb_vlc_code_[256]={ 
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, -8,  9, -9, 10,-10, 11,-11, 12,-12, 13,-13, 14,-14, 15,-15,
  4,  4,  4,  4, -4, -4, -4, -4,  5,  5,  5,  5, -5, -5, -5, -5,  6,  6,  6,  6, -6, -6, -6, -6,  7,  7,  7,  7, -7, -7, -7, -7,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  };


unsigned int get_int(void *ptr)
{
	unsigned char *buffer = (unsigned char *)ptr;
	return (*buffer) | ((*(buffer + 1)) << 8)| ((*(buffer + 2)) << 16)| ((*(buffer + 3)) << 24);
}

int H264_unaligned32_be(void *v)
{      
	//unsigned int x =  *(unsigned int *) v;
	unsigned int x =  get_int(v);
	return  (x&0xFF)<<24|(x&0xFF00)<<8|(x&0xFF0000)>>8|(x&0xFF000000)>>24;	 
}

void H264_init_get_bits(GetBitContext *s, unsigned char *buffer, int bit_size)
{
    int buffer_size= (bit_size+7)>>3;

    s->buffer= buffer;
    s->size_in_bits= bit_size;
    s->buffer_end= buffer + buffer_size;    
	s->index=0;
    {
		int re_index= s->index;
        int re_cache=  H264_unaligned32_be( ((unsigned char *)s->buffer)+(re_index>>3) ) << (re_index&0x07);	
        
		s->index= re_index;
    }
}

void H264_skip_bits(GetBitContext *s, int n)
{
	s->index += n;
}

void H264_skip_one_bits(GetBitContext *s)
{
    H264_skip_bits(s, 1);
}


int unaligned32_be(const unsigned char *v)
{      
	unsigned long x =  *(unsigned long *) v;
	
	return  ((x&0x000000FF)<<24)|((x&0x0000ff00)<<8)|((x&0x00ff0000)>>8)|((x&0xff000000)>>24);	 
}

unsigned int H264_get_bits(GetBitContext *s, int n)
{
#if 0
	int tmp;
    int re_index= s->index;
    int re_cache= 0;
	int zz =  *(int*)((unsigned char *)(s->buffer)+(re_index>>3)) ;
	zz = unaligned32_be(&zz);
	/*int zz = (s->buffer[(re_index>>3)+3]<<24)|
		(s->buffer[(re_index>>3)+2]<<16)|
		(s->buffer[(re_index>>3)+1]<<8)|
		(s->buffer[(re_index>>3)]);
		*/
	re_cache = zz<<(re_index&0x07);
    //re_cache= unaligned32_be( ((unsigned char *)(s)->buffer)+(re_index>>3) ) << (re_index&0x07);
	tmp= (((unsigned long)(re_cache))>>(32-(n)));
    re_index += n;
    s->index= re_index;
    return tmp;
#endif
    
	int tmp;
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)
    tmp= SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n)
    CLOSE_READER(re, s)
    return tmp;
}
unsigned int H264_get_one_bit(GetBitContext *s)
{
#if 0
    int index = s->index;
    unsigned char result = s->buffer[index>>3];
    result <<= (index&0x07);
    result >>= 7;
    index++;
    s->index = index;
    return result;
#endif
	return H264_get_bits(s, 1);
}


unsigned int H264_get_bits_long(GetBitContext *s, int n)
{
    if(n<=17) 
	{
		return H264_get_bits(s, n);
	}
    else
	{
        int ret= H264_get_bits(s, 16) << (n-16);
        return ret | H264_get_bits(s, n-16);
    }
}
int H264_av_log2(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) 	
	{
        v >>= 16;
        n += 16;
		if(v & 0xff00)
		{
			v >>= 8;
			n += 8;
		}
    }
	else if (v & 0xff00) 
	{
        v >>= 8;
        n += 8;
    }
	n += ff_log2_table[v>>1];

    return n;
}

int H264_get_ue_golomb(GetBitContext *gb)
{
    unsigned int	buf;
    int				log;
    int				re_index = gb->index;
    int				re_cache = 0;
	re_cache		= H264_unaligned32_be( ((unsigned char *)(gb)->buffer)+(re_index>>3) ) << (re_index&0x07);
    buf				= (unsigned int)re_cache;
    
    if(buf >= (1<<27))
	{
        buf >>= 32 - 9;
		re_index += _ff_golomb_vlc_len[buf>>4];
    	
		gb->index = re_index;  

		if(buf<256)
		{
			return _ff_ue_golomb_vlc_code[buf];
		}
		else
		{
			return 0;
		}    
    }
	else
	{
        log = 2*H264_av_log2(buf) - 31;
        buf>>= log;
        buf--;
        re_index += (32 - log);
		gb->index= re_index;    
        return buf;
    }
}

int H264_get_se_golomb(GetBitContext *gb)
{
    unsigned int	buf;
    int				log;
    int				re_index= gb->index;
    int				re_cache= 0;
    re_cache		= H264_unaligned32_be( ((unsigned char *)(gb)->buffer)+(re_index>>3) ) << (re_index&0x07);    
    buf				= re_cache;
    
    if(buf >= (1<<27))
	{
        buf >>= 32 - 9;       		
		re_index += _ff_golomb_vlc_len[buf>>4];		
		gb->index= re_index;
		if(buf<256)
		{
			return ff_se_golomb_vlc_code_[buf];
		}
		else
		{
			return 0;
		}           
    }
	else
	{
        log = 2*H264_av_log2(buf) - 31;
        buf >>= log;
        re_index += (32 - log);
		gb->index = re_index;
    
        if(buf&1) 
			buf = -(buf>>1);
        else      
			buf = (buf>>1);

        return buf;
    }
}

void move_03(unsigned char *dst, unsigned char* src,int len)
{
	
	unsigned char* psrc = src;
	unsigned char* pdst = dst;
	int i= 0;
	*pdst = *psrc;
	pdst++;
	psrc++;
	*pdst = *psrc;
	pdst++;
	psrc++;
	for (i = 0;i<len-3;i++)
	{
		if((*(psrc-2)==0x00)&&(*(psrc-1)==0x00)&&(*(psrc)==0x03))
		{
			if((*(psrc+1)==0x00)||(*(psrc+1)==0x01)||
			(*(psrc+1)==0x02)||(*(psrc+1)==0x03))
			{
				
			}
			else
			{
				pdst++;
			}
			psrc++;
		}
		else
		{
			*pdst = *psrc;
			psrc++;
			pdst++;
		}
	}
}


int H264_Analyse(unsigned char* pBuf,int nSize,int* nWidth,int* nHeight,int* framerate)
{
	GetBitContext	gb;    
	int				i					= 0;
	int				poc_type			= 0;
	int				poc_cycle_length	= 0;	
	int				tmp					= 0;
	unsigned char	tmpbuf[1024];
	move_03(tmpbuf,pBuf,nSize>1024?1024:nSize);
	H264_init_get_bits(&gb, tmpbuf+4, (nSize>1024?1024:nSize)-4);
	H264_skip_bits(&gb,24);
	H264_get_ue_golomb(&gb);

	H264_get_ue_golomb(&gb);
	poc_type = H264_get_ue_golomb(&gb);    
	if(poc_type == 0)
	{					
		H264_get_ue_golomb(&gb);
	} 
	else if(poc_type == 1)
	{					
		H264_skip_one_bits(&gb);
		H264_get_se_golomb(&gb);	
		H264_get_se_golomb(&gb);
		poc_cycle_length = H264_get_ue_golomb(&gb);
    
		for(i=0; i<poc_cycle_length; i++)
		{
			H264_get_se_golomb(&gb);
		}
	}
	if(poc_type > 2)
	{
		return -1;
	}

	H264_get_ue_golomb(&gb);		
	H264_skip_one_bits(&gb);
	*nWidth	 = (H264_get_ue_golomb(&gb) + 1)*16;
	*nHeight = (H264_get_ue_golomb(&gb) + 1)*16;

	tmp = H264_get_one_bit(&gb);
	if(!tmp)
	{
		H264_skip_one_bits(&gb);
	}

	 H264_get_one_bit(&gb);		

	tmp = H264_get_one_bit(&gb);
	if(tmp)
	{
		H264_get_ue_golomb(&gb);
		H264_get_ue_golomb(&gb);
		H264_get_ue_golomb(&gb);
		H264_get_ue_golomb(&gb);
	}
	
	if(H264_get_one_bit(&gb))
	{
		if(H264_get_one_bit(&gb))
		{
			if(H264_get_bits(&gb, 8)==255)
			{
				H264_get_bits(&gb, 16);
				H264_get_bits(&gb, 16);
			}
		}

		if(H264_get_one_bit(&gb))
		{      
			H264_get_one_bit(&gb);     
		}

		if(H264_get_one_bit(&gb)){   
			H264_get_bits(&gb, 3);
			H264_get_one_bit(&gb);   
			if(H264_get_one_bit(&gb)){ 
				H264_get_bits(&gb, 8);
				H264_get_bits(&gb, 8); 
				H264_get_bits(&gb, 8); 
			}
		}

		if(H264_get_one_bit(&gb)){   
			H264_get_ue_golomb(&gb); 
			H264_get_ue_golomb(&gb); 
		}

		if(H264_get_one_bit(&gb)){
			int tmp = 0;
			tmp = H264_get_bits_long(&gb, 32);
			tmp = H264_get_bits_long(&gb, 32);
			*framerate = tmp/2;
		
		}

	}
	
	return 1;
}
/*************************************************************************/
/*                                                                       */
/*                         MPEG4 Analyse function                        */
/*												                         */
/*							  2008-1-30                                  */
/*                                                                       */
/*************************************************************************/
typedef struct
{
	unsigned int bufa;
	unsigned int bufb;
	unsigned int buf;
	unsigned int pos;
	unsigned int *tail;
	unsigned int *start;
	unsigned int length;
}
Bitstream;

int MPEG4_BSWAP(void *v)
{      
	unsigned int x =  *(unsigned int *) v;
	
	*(int*)v = (x&0xFF)<<24|(x&0xFF00)<<8|(x&0xFF0000)>>8|(x&0xFF000000)>>24;	 
	return 0;
}

void MPEG4_BitstreamInit(Bitstream *  bs, void * bitstream, unsigned int length)
{
	unsigned int tmp;

	bs->start = bs->tail = (unsigned int *) bitstream;

	//tmp = *(unsigned int *) bitstream;
	tmp = get_int(bitstream);
	
	MPEG4_BSWAP(&tmp);

	bs->bufa = tmp;

	//tmp = *((unsigned int *) bitstream + 1);
	tmp = get_int((void*)((int)bitstream+sizeof(int)));
	MPEG4_BSWAP(&tmp);
	bs->bufb = tmp;

	bs->buf = 0;
	bs->pos = 0;
	bs->length = length;
}

unsigned int MPEG4_BitstreamShowBits(Bitstream *  bs, unsigned int bits)
{
	int nbit = (bits + bs->pos) - 32;
	
	if (nbit > 0) {
		return ((bs->bufa & (0xffffffff >> bs->pos)) << nbit) | (bs->bufb >> (32 - nbit));
	} else {
		return (bs->bufa & (0xffffffff >> bs->pos)) >> (32 - bs->pos - bits);
	}
}

void MPEG4_BitstreamSkip(Bitstream * bs, unsigned int bits)
{
	bs->pos += bits;

	if (bs->pos >= 32) 
	{
		unsigned int tmp;

		bs->bufa = bs->bufb;
		//tmp = *((unsigned int *) bs->tail + 2);
		tmp = get_int((void *)((unsigned int *) bs->tail + 2));
		MPEG4_BSWAP(&tmp);
		bs->bufb = tmp;
		bs->tail++;
		bs->pos -= 32;
	}

}

void MPEG4_BitstreamByteAlign(Bitstream * bs)
{
	unsigned int remainder = bs->pos % 8;

	if (remainder) 
	{
		MPEG4_BitstreamSkip(bs, 8 - remainder);
	}
}

unsigned int MPEG4_BitstreamGetBits(Bitstream * const bs,unsigned int n)
{
	unsigned int ret = MPEG4_BitstreamShowBits(bs, n);
	
	MPEG4_BitstreamSkip(bs, n);

	return ret;
}

unsigned int MPEG4_BitstreamGetBit(Bitstream * const bs)
{
	return MPEG4_BitstreamGetBits(bs, 1);
}


unsigned int MPEG4_log2bin(unsigned int value)
{
	int n = 0;
	if(value)
	{
		while (value) 
		{
			value >>= 1;
			n++;
		}
	}
	else
	{
		n = 1;
	}
	return n;
}

unsigned int MPEG4_BitstreamPos(Bitstream * bs)
{
	return((unsigned int)(8*((unsigned int)bs->tail - (unsigned int)bs->start) + bs->pos));
}

int Mpeg4_Analyse(unsigned char* pBuf,int nSize,int* nWidth,int* nHeight,int*framerate)
{
	Bitstream		bs;
	int				time_increment				= 0;
	int				frame_type					= 0;
	int				shape						= 0;
	int				time_inc_bits				= 0;
	unsigned int	vol_ver_id					= 0;
	unsigned int	time_increment_resolution	= 0;
	unsigned int	coding_type					= 0;
	unsigned int	start_code					= 0;
	unsigned int	time_incr					= 0;	

	MPEG4_BitstreamInit(&bs, pBuf, nSize);

	do 
	{
		MPEG4_BitstreamByteAlign(&bs);
		start_code = MPEG4_BitstreamShowBits(&bs, 32);

		if (start_code == 0x01b0) 
		{	
			MPEG4_BitstreamSkip(&bs, 40);
		}
		else if (start_code == 0x01b1) 
		{
			MPEG4_BitstreamSkip(&bs, 32);	
		} 
		else if (start_code == 0x01b5) 
		{
			MPEG4_BitstreamSkip(&bs, 32);	
			if (MPEG4_BitstreamGetBit(&bs))
			{
				vol_ver_id = MPEG4_BitstreamGetBits(&bs, 4);					
				MPEG4_BitstreamSkip(&bs, 3);	
			} 
			else 
			{
				vol_ver_id = 1;
			}

			if (MPEG4_BitstreamShowBits(&bs, 4) != 1)
			{
				return -1;
			}
			MPEG4_BitstreamSkip(&bs, 4);

			if (MPEG4_BitstreamGetBit(&bs))
			{
				MPEG4_BitstreamSkip(&bs, 4);	
				
				if (MPEG4_BitstreamGetBit(&bs))
				{
					MPEG4_BitstreamSkip(&bs, 24);
				}
			}
		} 
		else if ((start_code & ~0x0000001F) == 0x00000100) 
		{
			MPEG4_BitstreamSkip(&bs, 32);
		} 
		else if ((start_code & ~0x0000000F) == 0x00000120)
		{
			MPEG4_BitstreamSkip(&bs, 32);
			
			MPEG4_BitstreamSkip(&bs, 1);

			if (MPEG4_BitstreamShowBits(&bs, 8) != 1 && 
				MPEG4_BitstreamShowBits(&bs, 8) != 3 && 
				MPEG4_BitstreamShowBits(&bs, 8) != 4 && 
				MPEG4_BitstreamShowBits(&bs, 8) != 0)
			{
				return -1;
			}

			MPEG4_BitstreamSkip(&bs, 8);

			if (MPEG4_BitstreamGetBit(&bs))	
			{
				vol_ver_id = MPEG4_BitstreamGetBits(&bs, 4);	
				MPEG4_BitstreamSkip(&bs, 3);	
			} 
			else 
			{
				vol_ver_id = 1;
			}

			if (MPEG4_BitstreamGetBits(&bs, 4) == 15)	
			{				
				MPEG4_BitstreamSkip(&bs, 16);
			}

			if (MPEG4_BitstreamGetBit(&bs))	
			{				
				MPEG4_BitstreamSkip(&bs, 3);				
				
				if (MPEG4_BitstreamGetBit(&bs))
				{
					MPEG4_BitstreamSkip(&bs, 81);
				}
			}

			shape = MPEG4_BitstreamGetBits(&bs, 2);
			if ( shape == 3 && vol_ver_id != 1) 
			{
				MPEG4_BitstreamSkip(&bs, 4);
			}

			MPEG4_BitstreamSkip(&bs, 1);

			time_increment_resolution = MPEG4_BitstreamGetBits(&bs, 16);	

			*framerate = time_increment_resolution;
			if (time_increment_resolution > 0) 
			{
				time_inc_bits = MPEG4_log2bin(time_increment_resolution-1);
			} 
			else 
			{
				time_inc_bits = 1;
			}

			MPEG4_BitstreamSkip(&bs, 1);

			if (MPEG4_BitstreamGetBit(&bs))
			{
					MPEG4_BitstreamSkip(&bs, time_inc_bits);				
			}

			if (shape != 2) 
			{
				if (shape == 0) 
				{
					MPEG4_BitstreamSkip(&bs, 1);
					*nWidth = MPEG4_BitstreamGetBits(&bs, 13);	
					MPEG4_BitstreamSkip(&bs, 1);
					*nHeight = MPEG4_BitstreamGetBits(&bs, 13);				
					return 0;
				}
				return -1;
			} 
			else
			{
				if (vol_ver_id != 1) 
				{
					if (MPEG4_BitstreamGetBit(&bs))
					{
						return -1;
					}
				}
				MPEG4_BitstreamSkip(&bs, 1);	
			}
		}
		else if (start_code == 0x01b3) 
		{
			MPEG4_BitstreamSkip(&bs, 32);
			{
				MPEG4_BitstreamSkip(&bs, 18);
			}
			MPEG4_BitstreamSkip(&bs, 2);	
		} 
		else if (start_code == 0x01b6) 
		{
			MPEG4_BitstreamSkip(&bs, 32);	
			coding_type = MPEG4_BitstreamGetBits(&bs, 2);
			frame_type	= coding_type;
			if(coding_type==3)
				coding_type=1;

			while (MPEG4_BitstreamGetBit(&bs) != 0)	
			{
				time_incr++;
			}
			
			MPEG4_BitstreamSkip(&bs, 1);

			if (time_inc_bits)
			{
				time_increment = (MPEG4_BitstreamGetBits(&bs, time_inc_bits));
			}

			MPEG4_BitstreamSkip(&bs, 1);

			if (!MPEG4_BitstreamGetBit(&bs))
			{				
				return -1;
			}

			if ((shape != 2) && (coding_type == 1)) 
			{
				MPEG4_BitstreamSkip(&bs,1);				
			}

			if (shape != 0) 
			{
				unsigned int width, height;
				unsigned int horiz_mc_ref, vert_mc_ref;

				width = MPEG4_BitstreamGetBits(&bs, 13);
				MPEG4_BitstreamSkip(&bs,1);
				height = MPEG4_BitstreamGetBits(&bs, 13);
				MPEG4_BitstreamSkip(&bs,1);
				horiz_mc_ref = MPEG4_BitstreamGetBits(&bs, 13);
				MPEG4_BitstreamSkip(&bs,1);
				vert_mc_ref = MPEG4_BitstreamGetBits(&bs, 13);				
				MPEG4_BitstreamSkip(&bs, 2);
				
				if (MPEG4_BitstreamGetBit(&bs))
				{
					MPEG4_BitstreamSkip(&bs, 8);
				}
			}			
			return -1;
		} 
		else if (start_code == 0x01b2) 
		{
			MPEG4_BitstreamSkip(&bs, 48);
		} 
		else
		{
			MPEG4_BitstreamSkip(&bs, 32);	
		}

	}
	while ((MPEG4_BitstreamPos(&bs) >> 3) < bs.length);

	return -1;	
}
