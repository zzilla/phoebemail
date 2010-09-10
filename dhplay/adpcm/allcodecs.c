#include "avcodec.h"

void avcodec_register_all(void)
{
    static int inited = 0;
    
    if (inited != 0)
	return;
    inited = 1;

    register_avcodec(&adpcm_ima_wav_decoder);
//    register_avcodec(&adpcm_ima_wav_encoder);
}

