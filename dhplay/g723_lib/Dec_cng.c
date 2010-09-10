/*
**
** File:            "dec_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the decoder part
**
** Functions:       Init_Dec_Cng()
**                  Dec_Cng()
**
**
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/



/* Global Variable */


/*
**
** Function:        Init_Dec_Cng()
**
** Description:     Initialize Dec_Cng static variables
**
** Links to text:
**
** Arguments:       None
**
** Outputs:         None
**
** Return value:    None
**
*/
#include"stdlib.h"
#include"string.h"

#include "Typedef.h"
#include "basop.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "coder.h"
#include "decod.h"
#include "vad.h"
#include "cod_cng.h"
#include "dec_cng.h"
#include "Util_lbc.h"
#include "Util_cng.h"
#include "Lpc.h"
#include "Lsp.h"
#include "Exc_lbc.h"
#include "Tame.h"
#include"g723.h"
#include "inline.h"


void Init_Dec_Cng(DEC_HANDLE *handle)
{
    int i;

    handle->DecCng.PastFtyp = 1;
    handle->DecCng.SidGain = 0;
    for(i=0; i<LpcOrder; i++) 
		handle->DecCng.LspSid[i] = LspDcTable[i] ;
    handle->DecCng.RandSeed = 12345;
    return;
}

void Dec_Cng(Word16 Ftyp, LINEDEF *Line, Word16 *DataExc, Word16 *QntLpc,DEC_HANDLE *handle)
{

    Word16 temp;
    int i;

    if(Ftyp == 2) {

 /*
  * SID Frame decoding
  */
        handle->DecCng.SidGain = Dec_SidGain(Line->Sfs[0].Mamp);

        /* Inverse quantization of the LSP */
        Lsp_Inq( handle->DecCng.LspSid, handle->DecStat.PrevLsp, Line->LspId, 0) ;
    }

    else {

/*
 * non SID Frame
 */
        if(handle->DecCng.PastFtyp == 1) {

 /*
  * Case of 1st SID frame erased : quantize-decode
  * energy estimate stored in DecCng.SidGain
  * scaling factor in DecCng.CurGain
  */
            temp = Qua_SidGain(&handle->DecCng.SidGain, &handle->DecCng.CurGain, 0);
            handle->DecCng.SidGain = Dec_SidGain(temp);
        }
    }


    if(handle->DecCng.PastFtyp == 1) {
        handle->DecCng.CurGain = handle->DecCng.SidGain;
    }
    else {
        handle->DecCng.CurGain = extract_h(L_add( L_mult(handle->DecCng.CurGain,0x7000),
                    L_mult(handle->DecCng.SidGain,0x1000) ) ) ;
    }
    Calc_Exc_Rand(handle->DecCng.CurGain, handle->DecStat.PrevExc, DataExc,
                    &handle->DecCng.RandSeed, Line,handle->mode);

    /* Interpolate the Lsp vectors */
    Lsp_Int( QntLpc, handle->DecCng.LspSid, handle->DecStat.PrevLsp ) ;

    /* Copy the LSP vector for the next frame */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        handle->DecStat.PrevLsp[i] = handle->DecCng.LspSid[i] ;

    return;
}

