/* 
 * 4:4:4 formats, 32 bits per pixel. 
 * V0, U0, Y0, A0, V1, U1, Y1, A1
 */
#define AYUV    8


/*
 * 4:2:2 formats, 16 bits per pixel.
 * Y0, U0, Y1, V0, Y2, U1, Y3, V1
 */
#define YUY2    2


/*
 * 4:2:2 formats, 16 bits per pixel.
 * U0, Y0, V0, Y1, U1, Y2, V1, Y3
 */
#define UYVY    1


/*
 * 4:2:0 formats, 16 bits per pixel.
 * Y0, Y1, Y2, Y3, ....
 * ....
 * V0, V1, ....|///////|
 * ....        |///////|
 * U0, U1, ....|///////|
 * ....        |///////|
 */
#define IMC1    4


/*
 * 4:2:0 formats, 16 bits per pixel.
 * Y0, Y1, Y2, Y3, ....
 * ....
 * U0, U1, ....|///////|
 * ....        |///////|
 * V0, V1, ....|///////|
 * ....        |///////|
 */
#define IMC3    5


/*
 * 4:2:0 formats, 12 bits per pixel.
 * Y0, Y1, Y2, Y3, ....
 * ....
 * V0, V1, ..., U0, U1, ...
 * ....         ....
 */
#define IMC2    6


/* 
 * 4:2:0 formats, 12 bits per pixel.
 * Y0, Y1, Y2, Y3, ....
 * ....
 * U0, U1, ..., V0, V1, ...
 * ....         ....
 */
#define IMC4    7


/*
 * 4:2:0 formats, 12 bits per pixel.
 * Y0, Y1, Y2, Y3, ....
 * ....
 * V0, V1, ....
 * ....        
 * U0, U1, ....
 * ....        
 */
#define YV12    3


/*
 * 4:2:0 formats, 12 bits per pixel.
 * Y0, Y1, Y2, Y3, ....
 * ....
 * U0, V0, U1, V1, ....
 * ....        
 */
#define NV12    9


#define H1V1    1
#define H2V1    2
#define H2V2    4

#define M_SOF0      0xC0
#define M_SOF1      0xC1
#define M_SOF2      0xC2
#define M_SOF3      0xC3
#define M_SOF5      0xC5
#define M_SOF6      0xC6
#define M_SOF7      0xC7
#define M_JPG       0xC8
#define M_SOF9      0xC9
#define M_SOF10     0xCA
#define M_SOF11     0xCB
#define M_SOF13     0xCD
#define M_SOF14     0xCE
#define M_SOF15     0xCF

#define M_DHT       0xC4
#define M_DAC       0xCC
#define M_RST0      0xD0
#define M_RST1      0xD1
#define M_RST2      0xD2
#define M_RST3      0xD3
#define M_RST4      0xD4
#define M_RST5      0xD5
#define M_RST6      0xD6
#define M_RST7      0xD7
#define M_SOI       0xD8
#define M_EOI       0xD9
#define M_SOS       0xDA
#define M_DQT       0xDB
#define M_DNL       0xDC
#define M_DRI       0xDD
#define M_DHP       0xDE
#define M_EXP       0xDF

#define M_APP0      0xE0
#define M_APP15     0xEF
#define M_JPG0      0xF0
#define M_JPG13     0xFD
#define M_COM       0xFE


#define DCTSIZE     8
#define DCTSIZE2    (DCTSIZE * DCTSIZE)

