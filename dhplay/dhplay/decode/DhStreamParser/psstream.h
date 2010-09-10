#if !defined(PSSTREAM_H)
#define PSSTREAM_H

#include "StreamParser.h"

#define  PSIFRAMELEN 614400

//	ADI 264 resolution  

typedef enum  {
	DH_CAPTURE_SIZE_D1 = 0, // 704*576
	DH_CAPTURE_SIZE_HD1,	// 352*576
	DH_CAPTURE_SIZE_DCIF,	// 704*288
	DH_CAPTURE_SIZE_CIF,    // 352*288
	DH_CAPTURE_SIZE_QCIF,	// 176*144
	DH_CAPTURE_SIZE_VGA,	// 640*480
	DH_CAPTURE_SIZE_QVGA,   // 320*240
	DH_CAPTURE_SIZE_SVCD,   // 480*480
	DH_CAPTURE_SIZE_SVGA,   // 800*592
	DH_CAPTURE_SIZE_XVGA,   //1024*768
	DH_CAPTURE_SIZE_WXGA,   //1280*800
	DH_CAPTURE_SIZE_SXGA,   //1280*1024
	DH_CAPTURE_SIZE_WSXGA,  //1600*1024
	DH_CAPTURE_SIZE_UXGA,   //1600*1200
	DH_CAPTURE_SIZE_720P,
	DH_CAPTURE_SIZE_1080P,
	DH_CAPTURE_SIZE_NR
}capture_size_t;

typedef enum  {
	VIDEO_STANDARD_PAL,
	VIDEO_STANDARD_NTSC
}video_standard_t;


class PSStream : public StreamParser  
{
public:
	PSStream(unsigned char* rawBuf);
	virtual ~PSStream();

	bool CheckSign(const unsigned int& Code) ;
	bool ParseOneFrame() ;
	bool CheckIfFrameValid() ; 
	 int Reset(int level);
	 int AnalyseStreamType(unsigned char *buf, unsigned long len);
private:
	unsigned char m_iFrameTemp[PSIFRAMELEN];
	unsigned int  m_iFrameTempLength;
	bool m_bIFrameFirstIndex;
	bool m_bIFrameSecondIndex;
	bool m_ifAnalysed;
	//用于记录I帧第一个包的信息
	int m_width;
	int m_height;
	int m_rate;

	int m_EncodeType;
};

#endif // !defined(PSSTREAM_H)
