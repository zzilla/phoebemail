#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "JpegInfo.h"
#include "JpegEncoder.h"

#define TRUE    0
#define FALSE   1
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

#define BYTE unsigned char
#define BOOL int

BYTE * wBuffer;

///////////////////////////////////////////////////////////////////////
#define CONST_BITS  13
#define PASS1_BITS  2

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
* causing a lot of useless floating-point operations at run time.
* To get around this we use the following pre-calculated constants.
* If you change CONST_BITS you may want to add appropriate values.
* (With a reasonable C compiler, you can just rely on the FIX() macro...)
*/

#define FIX_0_298631336  ((int)  2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ((int)  3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ((int)  4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ((int)  6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ((int)  7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ((int)  9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ((int)  12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ((int)  15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ((int)  16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ((int)  16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ((int)  20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ((int)  25172)	/* FIX(3.072711026) */


/* Multiply an INT32 variable by an INT32 constant to yield an INT32 result.
* For 8-bit samples with the recommended scaling, all the variable
* and constant values involved are no more than 16 bits wide, so a
* 16x16->32 bit multiply can be used instead of a full 32x32 multiply.
* For 12-bit samples, a full 32-bit multiplication will be needed.
*/
#define ONE ((int) 1)
#define MULTIPLY(var,const)  ((var) * (const))
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

//////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned int mcu_per_row, mcu_per_col, block_in_mcu, block_in_y;

static int SampleType;

// size after padding 
static long width_pad, height_pad;
static long width_chp, height_chp;

static BYTE * YBuffer, * CrBuffer, * CbBuffer;

static int MCU_Y[4*64], MCU_Cr[64], MCU_Cb[64], zz[65];



/////////////////////////////////////////////////////////////
static int restart;
static int lastdc[4];
static int bitBuf, bitPos;
static BYTE SOIBuffer[2] = { 0xFF, M_SOI };
static BYTE EOIBuffer[2] = { 0xFF, M_EOI };
static BYTE SOFBuffer[19] = { 0xFF, M_SOF0, 0x00, 0x11, 8, 0, 1, 0, 1, 3, 1, 0x22, 0, 2, 0x11, 1, 3, 0x11, 1 };
static BYTE DRIBuffer[6] = { 0xFF, M_DRI, 0x00, 0x04, 0x00, 0x04 };
static BYTE SOSBuffer[14] = { 0xFF, M_SOS, 0x00, 0x0C, 3, 1, 0x00, 2, 0x11, 3, 0x11, 0, 63, 0 };

// standard luminance quant table, in ZigZag order.
const static BYTE standard_luminance_quant[64] = {
     16, 11, 12, 14, 12, 10, 16, 14,
     13, 14, 18, 17, 16, 19, 24, 40,
     26, 24, 22, 22, 24, 49, 35, 37,
     29, 40, 58, 51, 61, 60, 57, 51,
     56, 55, 64, 72, 92, 78, 64, 68,
     87, 69, 55, 56, 80,109, 81, 87,
     95, 98,103,104,103, 62, 77,113,
    121,112,100,120, 92,101,103, 99 };
// standard chominance quant table, in ZigZag order.
const static BYTE standard_chominance_quant[64] = {
     17, 18, 18, 24, 21, 24, 47, 26,
     26, 47, 99, 66, 56, 66, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99  };

// 量化表格.
// 这个表格将根据 quality 的值进行调整，考虑到需要多次使用，
// 源数据都是从 standard_luminance_quant, 和 standard_chominance_quant 得到.
static BYTE DQTBuffer[134] = { 0xFF, M_DQT, 0x00, 0x84,
     0x00,
     16, 11, 12, 14, 12, 10, 16, 14,
     13, 14, 18, 17, 16, 19, 24, 40,
     26, 24, 22, 22, 24, 49, 35, 37,
     29, 40, 58, 51, 61, 60, 57, 51,
     56, 55, 64, 72, 92, 78, 64, 68,
     87, 69, 55, 56, 80,109, 81, 87,
     95, 98,103,104,103, 62, 77,113,
    121,112,100,120, 92,101,103, 99,
     0x01,
     17, 18, 18, 24, 21, 24, 47, 26,
     26, 47, 99, 66, 56, 66, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99  };

// Huffman 表格.
static BYTE DHTBuffer[420] = { 0xFF, M_DHT, 0x01, 0xA2, 
0x00,
    0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
0x10,
    0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,
0x01,
    0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
0x11,
    0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77,
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa };


//static int huffdone;
static int dchuffcode[2][16];
static int dchuffsize[2][16];
static int achuffcode[2][256];
static int achuffsize[2][256];

/////////////////////////////////////////////////////////////////////////////////////


static void AdjustDQT(int quality);
static void MakeHuffTables(void);
static BOOL padding(BYTE * YUVBuffer, long width, long height, int YUVtype);
static void GetMCUData(int mcurow, int mcu);
static void fdct (int * data);
//////////////////////////////////////////////////////////////////////////////////////

void WriteJPEGHeader(long width, long height, int SampleType, int quality, int mcu_per_row)
{
    // 对文件头的各部分进行修正.

    // SOF 
    SOFBuffer[5] = (BYTE) (height>>8);
    SOFBuffer[6] = (BYTE) (height & 0xFF);
    SOFBuffer[7] = (BYTE) (width>>8);
    SOFBuffer[8] = (BYTE) (width & 0xFF);
    switch(SampleType)
    {
        case H1V1: SOFBuffer[11] = 0x11; break;
        case H2V1: SOFBuffer[11] = 0x21; break;
        case H2V2: SOFBuffer[11] = 0x22; break;
        default: break;
    } 

    // DRI
    DRIBuffer[4] = (BYTE) (mcu_per_row>>8);
    DRIBuffer[5] = (BYTE) (mcu_per_row & 0xFF);

    // 根据 quality 调整量化表
    AdjustDQT(quality);

    // 建立 Huffman 表查找表
    MakeHuffTables();

    // 复制文件头各部分.
    #define JCOPYBUFFER(src)  memcpy(wBuffer, src, sizeof(src));   wBuffer += sizeof(src)
    JCOPYBUFFER(SOIBuffer);
    JCOPYBUFFER(DQTBuffer);
    JCOPYBUFFER(SOFBuffer);
    JCOPYBUFFER(DRIBuffer);
    JCOPYBUFFER(DHTBuffer);
    JCOPYBUFFER(SOSBuffer);
    #undef JCOPYBUFFER

    memset(lastdc, 0, sizeof(lastdc));
    bitBuf = 0;
    bitPos = 24;
    restart = 0;
    return;
}

static void AdjustDQT(int quality)
{
    BYTE * q1 = DQTBuffer + 5;
    BYTE * q2 = DQTBuffer + 5 + 65;

    int i, temp;

    if( quality <= 0 ) quality = 1;
    if( quality > 100 ) quality = 100;

    if (quality < 50)
        quality = 5000 / quality;
    else
        quality = 200 - quality*2;

    for( i=0; i<DCTSIZE2; i++) 
    {
        temp = (standard_luminance_quant[i] * quality + 50) / 100;
        if( temp <= 0 )  temp = 1;
        if( temp >255 )  temp = 255;
        q1[i] = temp;
    }

    for( i=0; i<DCTSIZE2; i++) 
    {
        temp = (standard_chominance_quant[i] * quality + 50) / 100;
        if( temp <= 0 )  temp = 1;
        if( temp >255 )  temp = 255;
        q2[i] = temp;
    }
}

static void MakeCodeSize(const unsigned char * bits, const unsigned char * val, int * code, int * size)
{
    int len, n, i, value, nextcode;
    i = 0;
    nextcode = 0;
    for( len=1; len<=16; len++ )
    {
        nextcode = nextcode << 1;
        n = bits[len];
        while( n-- )
        {
            value = val[i++];

            code[value] = nextcode;
            size[value] = len;

            nextcode  = nextcode + 1;
        }
    }
}

static void MakeHuffTables(void)
{

    BYTE * p;

    p = DHTBuffer + 5;
    MakeCodeSize(p-1, p+16, dchuffcode[0], dchuffsize[0]);

    p = p + 16 + 12 + 1;
    MakeCodeSize(p-1, p+16, achuffcode[0], achuffsize[0]);
    
    p = p + 16 + 162 + 1;
    MakeCodeSize(p-1, p+16, dchuffcode[1], dchuffsize[1]);

    p = p + 16 + 12 + 1;
    MakeCodeSize(p-1, p+16, achuffcode[1], achuffsize[1]);
}

static const int jpeg_natural_order[DCTSIZE2] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// 量化，同时完成 zigzag 扫描.
static void quantize(int coefficients[], int zz[], int index)
{
    const unsigned char * quantTbl;
    int temp1, temp2, i;
    
    if(index)
        quantTbl = DQTBuffer + 5 + 65;
    else
        quantTbl = DQTBuffer + 5;

    for(i=0; i<DCTSIZE2; i++)
    {
        temp1 = coefficients[jpeg_natural_order[i]];
        temp2 = quantTbl[i];
        if( temp1 < 0 )
        {
            temp1 = (temp2>>1) - temp1;
            temp1 = temp1 / temp2;
            zz[i] = -temp1;
        }
        else
        {
            temp1 = (temp2>>1) + temp1;
            temp1 = temp1 / temp2;
            zz[i] = temp1;
        }
    }
}

static int getCategory(int * value)
{
    int val, temp, category;
    val = *value;
    if( val < 0 )
    {
        temp = -val;
        *value = val - 1;
    }
    else
    {
        temp = val;
    }

    category = 0;
    while( temp )
    {
        category ++;
        temp >>= 1;
    }
    
    return category;
}

static void append(int code, int size)
{
    int outbyte;

    code = code & ( (1<<size) - 1 );
    bitPos = bitPos - size;
    code = code << bitPos;
    bitBuf = bitBuf | code;

    while( bitPos <= 16 )  
    {
        bitPos = bitPos + 8;                // 更新指针位置
        outbyte = ( bitBuf >> 16 ) & 0xFF;  // 提取出位于最前面的一个字节
        bitBuf = bitBuf << 8;               // 调整缓冲区的内容
        // write outbyte out and check it. if 0xff, padding 0x00.        
        *(wBuffer++) = outbyte;
        if( outbyte == 0xFF )
            *(wBuffer++) = 0x00;
    }
}

static void EncodeBlock(int coefficients[], int index)
{
    int category, code, size;
    int k, r, * huffcode, * huffsize;
    int ac, rs;

    // DC 编码
    int dcdiff = coefficients[0] - lastdc[index];
    lastdc[index] = coefficients[0];

    // 对 dcdiff 进行编码
    category = getCategory(&dcdiff);
    code = dchuffcode[index!=0][category];
    size = dchuffsize[index!=0][category];

    append(code, size);

    append(dcdiff, category);

    // AC 编码
    huffcode = achuffcode[index!=0];
    huffsize = achuffsize[index!=0];

    //*//
    for(r=0, k=1; k<64; k++)
    {
        if( coefficients[k] == 0 )
        {
            r ++;
            continue;
        }
        while( r > 15 )
        {
            append(huffcode[0xF0], huffsize[0xF0]);
            r = r - 16;
        }
        ac = coefficients[k];
        category = getCategory(&ac);
        rs = (r<<4) + category; 
        append(huffcode[rs], huffsize[rs]);
        append(ac, category);
        r = 0;
    }
    /*///
    k=0; 
    r=-1;
    do {
        r++;
        ac = coefficients[++k];
    } while( ac == 0 );

    while( k < 64 )
    {
        while( r > 15 )    // ZRL encode
        {
            // 对 0xF0 进行 Huffman 编码
            code = huffcode[0xF0];
            size = huffsize[0xF0];
            
            append(code, size);

            r = r - 16;
        }
        
        category = getCategory(&ac);
        // 对 rs 进行 Huffman 编码
        rs = (r<<4) + category;
        code = huffcode[rs];
        size = huffsize[rs];
        
        append(code, size);
        
        // 对附加码进行编码
        append(ac, category);

        r = -1;
        do {
            r++;
            ac = coefficients[++k];
        } while( ac == 0 );
    }
    ////*/
    if( coefficients[63] == 0 )   // EOB encode.
    {
        // 对 0x00 进行 Huffman 编码.
        code = huffcode[0x00];
        size = huffsize[0x00];

        append(code, size);
    }

    return;
}

// 把 bit 缓冲区内所有数据输出(不完整字节填充 1-bits).
static void BitsAllOut(void)
{
    append(0x7F, 7);
    bitPos = 24;  // bit 缓冲区复位.
    bitBuf = 0;  
}

static void WriteRSTMarker(void)
{
    // 先输出缓冲区内的数据.
    BitsAllOut();

    // 然后写 Restart-i 标志.
    *(wBuffer++) = 0xFF;
    *(wBuffer++) = M_RST0 + restart;
    restart ++;
    restart &= 7;

    // 将直流预测值重新设置为零.
    memset(lastdc, 0, sizeof(lastdc));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 将不同的存储方式转化为统一的存储方式，即
// Y, Cr, Cb 三个分量均按像素顺序存储，并且填充数据到 MCU 的倍数.
// 此处根据需要给 YBuffer, CrBuffer, CbBuffer 分配相应的空间.
static BOOL padding(BYTE * YUVBuffer, long width, long height, int YUVtype)
{

    long width4, width2;
    BYTE * Yread, * Uread, * Vread, * wp;
    int Yoffset, Uoffset, Voffset;
    int Yskip, Uskip, Vskip;
    int i, j;
    long width_ch, height_ch;   
    // 两个色度分量实际占用的宽度和高度，不填充/填充
    BYTE value;
    
    // 计算并分配空间.
    switch( YUVtype )
    {
    case AYUV:  // 4:4:4
        SampleType = H1V1;
        width_pad = (width+7) & ~7;
        height_pad = (height+7) & ~7;
        width_ch = width;
        width_chp = width_pad;
        height_ch = height;
        height_chp = height_pad;
        mcu_per_row = width_pad >> 3;
        mcu_per_col = height_pad >> 3;
        block_in_y = 1;
        break;

    case YUY2:
    case UYVY:  // 4:2:2
        SampleType = H2V1;
        width_pad = (width+15) & ~15;
        height_pad = (height+7) & ~7;
        width_ch = (width+1) >> 1;
        width_chp = width_pad >> 1;
        height_ch = height;
        height_chp = height_pad;
        mcu_per_row = width_pad >> 4;
        mcu_per_col = height_pad >> 3;
        block_in_y = 2;
        break;

    case IMC1:
    case IMC3:
    case IMC2:
    case IMC4:
    case YV12:
    case NV12:  // 4:2:0        
        SampleType = H2V2;
        width_pad = (width+15) & ~15;
        height_pad = (height+15) & ~15;
        width_ch = (width+1) >> 1;
        width_chp = width_pad >> 1;
        height_ch = (height+1) >> 1;
        height_chp = height_pad >> 1;
        mcu_per_row = width_pad >> 4;
        mcu_per_col = height_pad >> 4;
        block_in_y = 4;
        break;
    }

    block_in_mcu = block_in_y + 2;
    YBuffer = malloc( width_pad * height_pad * sizeof(BYTE) );
    CrBuffer = malloc( width_chp * height_chp * sizeof(BYTE) );
    CbBuffer = malloc( width_chp * height_chp * sizeof(BYTE) );
    if( YBuffer == NULL || CrBuffer == NULL || CbBuffer == NULL )
    {
        puts("memory error!");
        free(YBuffer);
        free(CrBuffer);
        free(CbBuffer);
        return FALSE;
    }


    // 按照不同的存储方式起始地址/偏移量/跨度。
    switch( YUVtype )
    {
    // 下面一种情形无需任何填充.
    case AYUV:  
        Yread = YUVBuffer + 2;
        Uread = YUVBuffer + 1;
        Vread = YUVBuffer;
        Yoffset = Uoffset = Voffset = 4;
        Yskip = Uskip = Vskip = 0;
        break;
    
    // 下面两种情形是 width 为偶数时无需填充.
    case YUY2:  
        Yread = YUVBuffer;
        Uread = YUVBuffer + 1;
        Vread = YUVBuffer + 3;
        Yoffset = 2;
        Uoffset = Voffset = 4;
        Yskip = (width & 1) << 1;
        Uskip = Vskip = 0; 
        break;
    case UYVY:  
        Yread = YUVBuffer + 1;
        Uread = YUVBuffer;
        Vread = YUVBuffer + 2;
        Yoffset = 2;
        Uoffset = Voffset = 4;
        Yskip = (width & 1) << 1;
        Uskip = Vskip = 0;
        break;

    // 下面各种情形可能需要填充到 32bit.
    case IMC1:
        width4 = (width + 3) & ~3;
        Yread = YUVBuffer;
        Vread = Yread + width4 * height;
        Uread = Vread + width4 * height_ch;
        Yoffset = Uoffset = Voffset = 1;
        Yskip = width4 - width;
        Uskip = Vskip = width4 - width_ch;
        break;
    case IMC3:
        width4 = (width + 3) & ~3;
        Yread = YUVBuffer;
        Uread = Yread + width4 * height;
        Vread = Uread + width4 * height_ch;
        Yoffset = Uoffset = Voffset = 1;
        Yskip = width4 - width;
        Uskip = Vskip = width4 - width_ch;
        break;
    case IMC2:
        width4 = (width+3) & ~3;
        Yread = YUVBuffer;
        Vread = Yread + width4 * height;
        Uread = Vread + (width4>>1);
        Yoffset = Uoffset = Voffset = 1;
        Yskip = width4 - width;
        Uskip = Vskip = width4 - width_ch; 
        break;
    case IMC4:
        width4 = (width+3) & ~3;
        Yread = YUVBuffer;
        Uread = Yread + width4 * height;
        Vread = Uread + (width4>>1);
        Yoffset = Uoffset = Voffset = 1;
        Yskip = width4 - width;
        Uskip = Vskip = width4 - width_ch;
        break;
    case YV12: 
        // U, V 分量每行 32 bit 对齐? 16 bit 对齐?
        // 暂时假设 16 bit 对齐.
        width4 = (width+3) & ~3;
        width2 = width4>>1;
        //width2 = (width2+3) & ~3;  // 若 32 bit 对齐，使用此语句.
        Yread = YUVBuffer;
        Uread = Yread + width4 * height;
        Vread = Uread + width2 * height_ch;
        Yoffset = Uoffset = Voffset = 1;
        Yskip = width4 - width;
        Uskip = Vskip = width2 - width_ch;
        break;
    case NV12:
        width4 = (width+3) & ~3;
        Yread = YUVBuffer;
        Uread = Yread + width4 * height;
        Vread = Uread + 1;
        Yoffset = 1;
        Uoffset = Voffset = 2;
        Yskip = width4 - width;
        Uskip = Vskip = width4 - (width_ch<<1);
        break;
    default:
        puts("Illegal YUVtype.");
        return FALSE;
        break;
    }
    

    // Copy the Y component.
    wp = YBuffer;
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            value = *(Yread);
            *(wp++) = MAX(0,MIN(255, (value-16)*255/(235-16)));
            Yread += Yoffset;
        }
        for(; j<width_pad; j++)
        {
            *(wp++) = value;
        }
        Yread += Yskip;
    }
    for(; i<height_pad; i++)
    {
        memcpy(wp, wp-width_pad, width_pad);
        wp += width_pad;
    } 

    // Copy the Cr component.
    wp = CrBuffer;
    for(i=0; i<height_ch; i++)
    {
        for(j=0; j<width_ch; j++)
        {
            value = * (Vread);
            *(wp++) = value;
            Vread += Voffset;
        }
        for(; j<width_chp; j++)
        {
            *(wp++) = value;
        }
        Vread += Vskip;
    }
    for(; i<height_chp; i++)
    {
        memcpy(wp, wp-width_chp, width_chp);
        wp += width_chp;
    }

    // Copy the Cb component.
    wp = CbBuffer;
    for(i=0; i<height_ch; i++)
    {
        for(j=0; j<width_ch; j++)
        {
            value = * (Uread);
            *(wp++) = value;
            Uread += Uoffset;
        }
        for(; j<width_chp; j++)
        {
            *(wp++) = value;
        }
        Uread += Uskip;
    }
    for(; i<height_chp; i++)
    {
        memcpy(wp, wp-width_chp, width_chp);
        wp += width_chp;
    }

    return TRUE;
}

static void GetMCUData(int mcurow, int mcu)
{
    int i, j, k;
    long base;
    long base1, base2, base3;

    // Y component.
    switch(SampleType)
    {
    case H2V2:
        base  = width_pad * mcurow * 16 + mcu * 16;
        base1 = base + 8;
        base2 = base + width_pad * 8;
        base3 = base2 + 8;
        k = 0;
        for(i=0; i<8; i++)
        {
            for(j=0; j<8; j++)
            {
                MCU_Y[k]     = YBuffer[base +i*width_pad+j] - 128;
                MCU_Y[64+k]  = YBuffer[base1+i*width_pad+j] - 128;
                MCU_Y[128+k] = YBuffer[base2+i*width_pad+j] - 128;
                MCU_Y[192+k] = YBuffer[base3+i*width_pad+j] - 128;
                k++;
            }
        }
        break;

    case H2V1:
        base  = width_pad * mcurow * 8 + mcu * 16;
        base1 = base + 8;
        k = 0;
        for(i=0; i<8; i++)
        {
            for(j=0; j<8; j++)
            {
                MCU_Y[k]    = YBuffer[base +i*width_pad+j] - 128;
                MCU_Y[64+k] = YBuffer[base1+i*width_pad+j] - 128;
                k++;
            }
        }
        break;

    case H1V1:
        base = width_pad * mcurow * 8 + mcu * 8;
        k = 0;
        for(i=0; i<8; i++)
        {
            for(j=0; j<8; j++)
            {
                MCU_Y[k] = YBuffer[base+i*width_pad+j] - 128;
                k++;
            }
        }
        break;
    }

    // Cr/Cb component.
    base = width_chp * mcurow * 8 + mcu * 8;
    k = 0;
    for(i=0; i<8; i++)
    {
        for(j=0; j<8; j++)
        {
            MCU_Cr[k] = CrBuffer[base+i*width_chp+j] - 128;
            MCU_Cb[k] = CbBuffer[base+i*width_chp+j] - 128;
            k++;
        }
    }

    return;
}

/*
 * Perform the forward DCT on one block of samples.
 */

static void fdct (int * data)
{
  int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  int tmp10, tmp11, tmp12, tmp13;
  int z1, z2, z3, z4, z5;
  int *dataptr;
  int ctr;

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true DCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[0] + dataptr[7];
    tmp7 = dataptr[0] - dataptr[7];
    tmp1 = dataptr[1] + dataptr[6];
    tmp6 = dataptr[1] - dataptr[6];
    tmp2 = dataptr[2] + dataptr[5];
    tmp5 = dataptr[2] - dataptr[5];
    tmp3 = dataptr[3] + dataptr[4];
    tmp4 = dataptr[3] - dataptr[4];
    
    /* Even part per LL&M figure 1 --- note that published figure is faulty;
     * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
     */
    
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[0] = (int) ((tmp10 + tmp11) << PASS1_BITS);
    dataptr[4] = (int) ((tmp10 - tmp11) << PASS1_BITS);
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
    dataptr[2] = (int) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
				   CONST_BITS-PASS1_BITS);
    dataptr[6] = (int) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
				   CONST_BITS-PASS1_BITS);
    
    /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
     * cK represents cos(K*pi/16).
     * i0..i3 in the paper are tmp4..tmp7 here.
     */
    
    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
    
    tmp4 = MULTIPLY(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp5 = MULTIPLY(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp6 = MULTIPLY(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp7 = MULTIPLY(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
    
    z3 += z5;
    z4 += z5;
    
    dataptr[7] = (int) DESCALE(tmp4 + z1 + z3, CONST_BITS-PASS1_BITS);
    dataptr[5] = (int) DESCALE(tmp5 + z2 + z4, CONST_BITS-PASS1_BITS);
    dataptr[3] = (int) DESCALE(tmp6 + z2 + z3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (int) DESCALE(tmp7 + z1 + z4, CONST_BITS-PASS1_BITS);
    
    dataptr += DCTSIZE;		/* advance pointer to next row */
  }

  /* Pass 2: process columns.
   * We remove the PASS1_BITS scaling, but leave the results scaled up
   * by an overall factor of 8.
   */

  dataptr = data;
  for (ctr = DCTSIZE-1; ctr >= 0; ctr--) {
    tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
    tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
    tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
    tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
    tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
    tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
    tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
    tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];
    
    /* Even part per LL&M figure 1 --- note that published figure is faulty;
     * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
     */
    
    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    dataptr[DCTSIZE*0] = (int) DESCALE(tmp10 + tmp11, PASS1_BITS+3);
    dataptr[DCTSIZE*4] = (int) DESCALE(tmp10 - tmp11, PASS1_BITS+3);
    
    z1 = MULTIPLY(tmp12 + tmp13, FIX_0_541196100);
    dataptr[DCTSIZE*2] = (int) DESCALE(z1 + MULTIPLY(tmp13, FIX_0_765366865),
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*6] = (int) DESCALE(z1 + MULTIPLY(tmp12, - FIX_1_847759065),
					   CONST_BITS+PASS1_BITS+3);
    
    /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
     * cK represents cos(K*pi/16).
     * i0..i3 in the paper are tmp4..tmp7 here.
     */
    
    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
    
    tmp4 = MULTIPLY(tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp5 = MULTIPLY(tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp6 = MULTIPLY(tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp7 = MULTIPLY(tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
    
    z3 += z5;
    z4 += z5;
    
    dataptr[DCTSIZE*7] = (int) DESCALE(tmp4 + z1 + z3,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*5] = (int) DESCALE(tmp5 + z2 + z4,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*3] = (int) DESCALE(tmp6 + z2 + z3,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*1] = (int) DESCALE(tmp7 + z1 + z4,
					   CONST_BITS+PASS1_BITS+3);
    
    dataptr++;			/* advance pointer to next column */
  }
}
/////////////////////////////////////////////////////////////////////////////////////////
// JPEG encoder from YU(Cb)V(Cr) buffer among various storages.
// return the size of the jpeg file. 
// zero(0) is returned if any error occurs.
// JPEGBuffer must be large enough to store the whole jpeg file.
int JpegEncode(unsigned char * JPEGBuffer, unsigned char * YUVBuffer, int *outsize, long width, long height, int YUVtype, int quality)
{
    unsigned int mcurow, mcu, block;
	
    if( padding(YUVBuffer, width, height, YUVtype) == FALSE )    
        return 0;
	
    wBuffer = JPEGBuffer;
	
    WriteJPEGHeader(width, height, SampleType, quality, mcu_per_row);
    zz[64] = 1;
	
    for( mcurow=0; mcurow<mcu_per_col; mcurow++ )
    {
        for( mcu=0; mcu<mcu_per_row; mcu++ )
        {
            // copy data for this MCU.
            GetMCUData(mcurow, mcu);
			
            // Y Component.
            for( block=0; block<block_in_y; block++)
            {
                fdct(MCU_Y+64*block);
                quantize(MCU_Y+64*block, zz, 0);
                EncodeBlock(zz, 0);
            }
			
            // Cb component.
            fdct(MCU_Cb);
            quantize(MCU_Cb, zz, 1);
            EncodeBlock(zz, 1);
			
            // Cr component.
            fdct(MCU_Cr);
            quantize(MCU_Cr, zz, 2);
            EncodeBlock(zz, 2);
			
        }
		
        if( mcurow != mcu_per_col - 1 )
            WriteRSTMarker();
    }
	
    BitsAllOut();
	
    *(wBuffer++) = 0xFF;
    *(wBuffer++) = M_EOI;
	
    free(YBuffer); 
    free(CrBuffer);
    free(CbBuffer);
	
    *outsize = (int)(wBuffer - JPEGBuffer);

	return 1;
}


