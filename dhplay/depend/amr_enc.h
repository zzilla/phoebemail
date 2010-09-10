/*
*****************************************************************************
*
*                       AMR-NB speech codec            
*
*****************************************************************************
*
*      File             : amr_enc.h
*      Purpose          : Encoding of one speech frame.
*
*****************************************************************************
*/
#ifndef amr_enc_h
#define amr_enc_h "$Id $"

 
#ifdef __cplusplus
    #if __cplusplus
    extern "C" {
    #endif  /* __cpluscplus */
#endif  /* __cpluscplus */

#ifndef HI_VOICE_API
#ifdef WIN32
	#define HI_VOICE_API     __declspec(dllexport)
#else
	#define HI_VOICE_API
#endif	
#endif

#ifndef HI_S8
  #define HI_S8  char
#endif

#ifndef HI_S16
  #define HI_S16 short
#endif

#ifndef HI_S32
  #define HI_S32 int
#endif

#ifndef HI_U8
  #define HI_U8  unsigned char
#endif

#ifndef HI_U16 
  #define HI_U16 unsigned short
#endif

#ifndef HI_U32
  #define HI_U32 unsigned int
#endif

#ifndef HI_VOID
  #define HI_VOID void
#endif


#ifndef L_FRAME
#define L_FRAME      160            /* Frame size */
#endif

#ifndef MAX_PACKED_SIZE
#define MAX_PACKED_SIZE   35       /* max. num. of packed bytes*/
#endif

#ifndef AMR_MAGIC_NUMBER
#define AMR_MAGIC_NUMBER "#!AMR\n"  /*magic number for AMR-NB*/
#endif

enum Mode { MR475 = 0,     /*4.75kbit/s*/
            MR515,             
            MR59,         
            MR67,         
            MR74,         
            MR795,
            MR102,
            MR122,            
	    MRDTX,         /*SID mode*/
            N_MODES        /* number of (SPC) modes */
          };

enum Format {MIME, IF1, IF2}; 

/* HISI_AMRNB right code */
#define HI_SUCCESS                    0
#define HI_ERR_AMRNB_PREFIX           0xB1222000

/* HISI_AMRNB error code */
#define HI_ERR_AMRNB_MODE_TYPE        (HI_ERR_AMRNB_PREFIX | 0x0001)
#define HI_ERR_AMRNB_FORMAT_TYPE      (HI_ERR_AMRNB_PREFIX | 0x0002)


#define HI_ERR_AMRNB_INVALID_DEVICE   (HI_ERR_AMRNB_PREFIX | 0x0011)
#define HI_ERR_AMRNB_INVALID_INBUF    (HI_ERR_AMRNB_PREFIX | 0x0012)
#define HI_ERR_AMRNB_INVALID_OUTBUF   (HI_ERR_AMRNB_PREFIX | 0x0013)

#define HI_ERR_AMRNB_INIT_FAIL        (HI_ERR_AMRNB_PREFIX | 0x0101)
#define HI_ERR_AMRNB_ENCODE_FAIL      (HI_ERR_AMRNB_PREFIX | 0x0102)
#define HI_ERR_AMRNB_DECODE_FAIL      (HI_ERR_AMRNB_PREFIX | 0x0103)
/*
*****************************************************************************
*                         ENCLARATION OF PROTOTYPES
*****************************************************************************
*/

/***************************************************************************
* Function:      AMR_Encode_Init
* Description:   initialize encoder device handle
* Input:         **pEncState:                        encoder device handle
                 dtx                                 DTX controller 
* Output:        none
* Return:        HI_SUCCESS                          success
*                HI_ERR_AMRNB_INIT_FAIL              encode device initial failed
******************************************************************************/
HI_VOICE_API HI_S32 AMR_Encode_Init (HI_VOID **pEncState, HI_S16 dtx);

/***************************************************************************
* Function:      AMR_Encode_Exit
* Description:   free encoder device handle
* Input:         **pEncState:                        encoder device handle
* Output:        none
* Return:        none
******************************************************************************/ 
HI_VOICE_API HI_VOID AMR_Encode_Exit (HI_VOID **pEncState);
 
/***************************************************************************
* Function:      AMR_Encode_Frame
* Description:   encode one frame speech data  
* Input:         *pEncState:     encoder device handle
*                *pInBuf:        input speech data addr 
*                 mode:          speech coder mode  
*                 frame_type:    frame format
* Output:        *pOutBuf        output addr of compressed speech data buffer
* Return:        positive number                     length of coded serial(unit: HI_U8) 
*                HI_ERR_AMRNB_INVALID_DEVICE         invalid encoder device handle
*                HI_ERR_AMRNB_INVALID_INBUF          invalid input speech data addr
*                HI_ERR_AMRNB_INVALID_OUTBUF         invalid output addr
*                HI_ERR_AMRNB_MODE_TYPE              invalid speech coder mode 
*                HI_ERR_AMRNB_FORMAT_TYPE            invalid encoder frame type
*                HI_ERR_AMRNB_ENCODE_FAIL            encode failed   
******************************************************************************/
HI_VOICE_API HI_S32 AMR_Encode_Frame (HI_VOID *pEncState,HI_S16 *pInBuf,HI_U8 *pOutBuf,enum Mode mode,enum Format frame_type);
 
#ifdef __cplusplus
    #if __cplusplus
	}
    #endif  /* __cpluscplus */
#endif  /* __cpluscplus */

#endif

