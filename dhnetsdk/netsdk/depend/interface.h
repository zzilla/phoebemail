#ifndef DEFINE_H
#define DEFINE_H

#define SAMPLE_FREQ_4000	1
#define SAMPLE_FREQ_8000	2
#define SAMPLE_FREQ_11025	3
#define SAMPLE_FREQ_16000	4
#define SAMPLE_FREQ_20000	5
#define SAMPLE_FREQ_22050	6
#define SAMPLE_FREQ_32000	7
#define SAMPLE_FREQ_44100	8
#define SAMPLE_FREQ_48000	9

// 输入类型
enum InputType
{
	File = 0,
	Stream = 1
};

// 编码类型
enum EncodeType {
	Encode_None = 0,

	// 视频编码类型
	Encode_MPEG4,
	Encode_H264,

	// 音频编码类型
	Encode_PCM = 7,
	Encode_G729,
	Encode_IMA,
	Encode_PCM_MULAW,
	Encode_G721,
	Encode_PCM8_VWIS,
	Encode_MS_ADPCM
};

enum PackageType
{
	PackageInvalid = -1,// 无法识别的打包方式
	PackageNew = 0,		// 新的打包方式
	PackageNone,		// 没有其他头,通用MPEG4适用
	PackageHB			// 视豪2打包方式
};

// 帧类型
enum FrameType {
	InvialdFrame = 0,	// 无效帧
	VideoFrame,			// 视频帧
	AudioFrame,			// 音频帧
	DataFrame			// 可识别的数据帧
};

// 视频帧类型
enum VideoFrameType {
	N_Frame = 0, // 
	I_Frame,
	P_Frame
};

/************************************************************************
* 视频解码器接口
************************************************************************/
struct IVideoDecoder {
	// 初始化
	virtual int init() = 0;
	
	// 清除
    virtual int clean() = 0;	
	
	// 重置
	virtual int reset() = 0;	
	
	// 改变图像大小，可以不用
    virtual int resize(int w, int h) = 0; 
	
	// 解码, 其中param为自定义信息
    virtual int decode(unsigned char *buf, unsigned int buflen, int param1, int param2) = 0;
};

struct tagFrameInfo {
	unsigned char	*header;	// 包含头的数据指针
	unsigned char	*content;	// 真正数据的偏移
	unsigned int	length;		// 数据长度(包括头部)
	unsigned int	datalen;	// 数据长度.	

	int type; // VIDEO, AUDIO, DATA
	int subtype; // I-FRAME, P-FRAME, etc.
	
	int encode; // MPEG4/H264, PCM, MSADPCM, etc.
	
	int rate;
	
	int width;
	int height;
	
	// 只有I帧才有的数据
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	
	// 音频才有的数据
	int channels;
	int bitsPerSample;
	int samplesPerSecond;
	
	int param1;
	int param2;
};

struct IStreamParser 
{
	virtual int init() = 0;
	
	virtual void clean() = 0;
	
	// 分析数据接口
	/*! 返回:
	0: 没有得到一个Frame
	N>0: 得到N个完整的帧 
	*/
	virtual int parse(unsigned char *data, int datalen) = 0;
	
	// 得到分离的数据
	/*!
	返回: 类型值(0x01FD, 0x01FC, 0x01FB, 0x01FA....)
	参数: index为索引号
	parm:为参数
	*/
	virtual int getFrameData(int index, void **parm) = 0;
	
	// 重置 如果force == true 清理所有数据包括
	virtual void reset(bool force = false) = 0;
};

struct YUVDataInfo
{
	int width;		// 宽
	int height;		// 高
	int rate;		// 指示YUV格式,可以用于描述是否为两个分离的CIF等
	int reserved;	// 保留,目前填充0
};

struct IFifoBuffer
{
	virtual int init(int count, int blocksize, const char *name) = 0;
	virtual int reset() = 0;
	virtual int clear() = 0;
	virtual int write(unsigned char *data, unsigned int datalen) = 0;
	virtual int read(unsigned char *data, unsigned int &datalen) = 0;
	virtual bool readReady() = 0;
	virtual bool writeReady() = 0;
	virtual int chunkCount() = 0;
	virtual int maxCount() = 0;
	virtual int restCount() = 0;
};

#endif // DEFINE_H
