/*
*****************************************************************************
*
*                       AMR-NB speech codec
*   
*****************************************************************************
*
*      File             : amr_dec.h
*      Purpose          : Decoding of one speech frame.
*
*****************************************************************************
*/
#ifndef amr_dec_h
#define amr_dec_h "$Id $"

#ifdef __cplusplus
    #if __cplusplus
    extern "C" {
    #endif  /* __cpluscplus */
#endif  /* __cpluscplus */


/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/

#include "amr_enc.h" 

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/

/***************************************************************************
* Function:      AMR_Decode_Init
* Description:   initialize decoder device handle
* Input:         **pDecState:                        decoder device handle
* Output:        none
* Return:        HI_SUCCESS                          success
*                HI_ERR_AMRNB_INIT_FAIL              decoder device initial failed
******************************************************************************/
        
HI_VOICE_API HI_S32 AMR_Decode_Init(HI_VOID **pDecState);

/***************************************************************************
* Function:      AMR_Decode_Exit
* Description:   free decoder device handle
* Input:         **pDecState:                        decoder device handle
* Output:        none
* Return:        none
******************************************************************************/
HI_VOICE_API HI_VOID AMR_Decode_Exit (HI_VOID **pDecState);

/***************************************************************************
* Function:      AMR_Decode_Frame
* Description:   decode one frame speech data  
* Input:         *pDecState:     decoder device handle
*                *pInBuf:        input serial data addr 
*                 frame_type:    frame format
* Output:        *pOutBuf        output addr of decoded speech data buffer
* Return:        HI_SUCCESS                          success
*                HI_ERR_AMRNB_INVALID_DEVICE         invalid decoder device handle
*                HI_ERR_AMRNB_INVALID_INBUF          invalid input serial data addr
*                HI_ERR_AMRNB_INVALID_OUTBUF         invalid output addr
*                HI_ERR_AMRNB_FORMAT_TYPE            invalid encoder frame type   
*                HI_ERR_AMRNB_DECODE_FAIL            decode failed
******************************************************************************/
HI_VOICE_API HI_S32 AMR_Decode_Frame (HI_VOID *pDecState, HI_U8 *pInbuf, HI_S16 *pOutBuf, enum Format frame_type);

/***************************************************************************
* Function:      AMR_Get_Length
* Description:   get length of the frame to be decoded 
* Input:         frame_type:    frame format
*                toc:           header of frame     
* Output:        none
* Return:        positive number                     length of the frame to be decoded(unit: HI_U8) 
*                HI_ERR_AMRNB_FORMAT_TYPE            invalid encoder frame type   
******************************************************************************/
HI_VOICE_API HI_S32 AMR_Get_Length(enum Format frame_type, HI_U8 toc);

#ifdef __cplusplus
    #if __cplusplus
	}
    #endif  /* __cpluscplus */
#endif  /* __cpluscplus */

#endif
