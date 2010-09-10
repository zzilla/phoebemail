#ifndef MPEGAUDIODECODER_H
#define MPEGAUDIODECODER_H

#include "mad.h"

class mpeg_audio_decoder 
{
public:	
	mpeg_audio_decoder() : remain(0),total_len(0) {}
	~mpeg_audio_decoder(){}
	
	void init();
	void finish();
	
	int decode(char *buf, unsigned len,
		unsigned char **frame_data, int *frame_len);
private:
	mad_stream stream;
	mad_frame frame;
	mad_synth synth;
	
	unsigned char buffer[4096*2];
	unsigned char out[1152*8];
	
	unsigned remain;
	unsigned total_len;
};

#endif // MPEGAUDIODECODER_H

