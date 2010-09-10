/*
**
** File:            "cod_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the encoder part
**
** Functions:       Init_Cod_Cng()
**                  Cod_Cng()
**                  Update_Cng()
**
** Local functions:
**                  ComputePastAvFilter()
**                  CalcRC()
**                  LpcDiff()
**
**
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


/* Declaration of local functions */
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



static void ComputePastAvFilter(Word16 *Coeff,ENC_HANDLE *handle);
static void CalcRC(Word16 *Coeff, Word16 *RC, Word16 *shRC);
static Flag LpcDiff(Word16 *RC, Word16 shRC, Word16 *Acf, Word16 alpha);

/* Global Variables */


/*
**
** Function:        Init_Cod_Cng()
**
** Description:     Initialize Cod_Cng static variables
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
void Init_Cod_Cng(ENC_HANDLE *handle)
{
    int i;

    handle->CodCng.CurGain = 0;

    for(i=0; i< SizAcf; i++) handle->CodCng.Acf[i] = 0;

    for(i=0; i <= NbAvAcf; i++) handle->CodCng.ShAcf[i] = 40;

    for(i=0; i < LpcOrder; i++) handle->CodCng.SidLpc[i] = 0;

    handle->CodCng.PastFtyp = 1;

    handle->CodCng.RandSeed = 12345;

    return;
}


void Update_Acf(Word16 *Acf_sf, Word16 *ShAcf_sf,ENC_HANDLE *handle)
{

    int i, i_subfr;
    Word16 *ptr1, *ptr2;
    Word32 L_temp[LpcOrderP1];
    Word16 sh1, temp;
    Word32 L_acc0;

    /* Update Acf and ShAcf */
    ptr2 = handle->CodCng.Acf + SizAcf;
    ptr1 = ptr2 - LpcOrderP1;
    for(i=LpcOrderP1; i<SizAcf; i++) *(--ptr2) = *(--ptr1);
    for(i=NbAvAcf; i>=1; i--) 
		handle->CodCng.ShAcf[i] = handle->CodCng.ShAcf[i-1];

    /* Search ShAcf_sf min for current frame */
    sh1 = ShAcf_sf[0];
    for(i_subfr=1; i_subfr<SubFrames; i_subfr++) {
#ifdef VC
        if(ShAcf_sf[i_subfr] < sh1) sh1 = ShAcf_sf[i_subfr];
#endif

#ifdef CCS
		sh1=_min2(sh1,ShAcf_sf[i_subfr]);
#endif
    }
    sh1 = add(sh1, 14);  /* 2 bits of margin */

    /* Compute current sum of acfs */
    for(i=0; i<= LpcOrder; i++) L_temp[i] = 0;

    ptr2 = Acf_sf;
    for(i_subfr=0; i_subfr<SubFrames; i_subfr++) {
        temp = sub(sh1, ShAcf_sf[i_subfr]);
        for(i=0; i <= LpcOrder; i++) {
            L_acc0 = L_deposit_l(*ptr2++);
            L_acc0 = L_shl(L_acc0, temp);  /* shift right if temp<0 */
            L_temp[i] = L_add(L_temp[i], L_acc0);
        }
    }
    /* Normalize */
    temp = norm_l(L_temp[0]);
    temp = sub(16, temp);
#ifdef VC
    if(temp < 0) temp = 0;
#endif

#ifdef CCS
	temp=_max2(temp,0);
#endif
    for(i=0; i <= LpcOrder; i++) {
        handle->CodCng.Acf[i] = extract_l(L_shr(L_temp[i],temp));
    }

    handle->CodCng.ShAcf[0] = sub(sh1, temp);

    return;
}

void Cod_Cng(Word16 *DataExc, Word16 *Ftyp, LINEDEF *Line, Word16 *QntLpc,ENC_HANDLE *handle)
{


    Word16 curCoeff[LpcOrder];
    Word16 curQGain;
    Word16 temp;
    int i;

 /*
  * Update Ener
  */
    for(i=NbAvGain-1; i>=1; i--) {
        handle->CodCng.Ener[i] = handle->CodCng.Ener[i-1];
    }

 /*
  * Compute LPC filter of present frame
  */
    handle->CodCng.Ener[0] = Durbin(curCoeff, &handle->CodCng.Acf[1], handle->CodCng.Acf[0], &temp);

 /*
  * if first frame of silence => SID frame
  */
    if(handle->CodCng.PastFtyp == 1) {
        *Ftyp = 2;
        handle->CodCng.NbEner = 1;
        curQGain = Qua_SidGain(handle->CodCng.Ener, handle->CodCng.ShAcf, handle->CodCng.NbEner);
    }

    else {
        handle->CodCng.NbEner++;
        if(handle->CodCng.NbEner > NbAvGain) handle->CodCng.NbEner = NbAvGain;
        curQGain = Qua_SidGain(handle->CodCng.Ener, handle->CodCng.ShAcf, handle->CodCng.NbEner);

 /*
  * Compute stationarity of current filter
  * versus reference filter
  */
        if(LpcDiff(handle->CodCng.RC,handle->CodCng.ShRC, handle->CodCng.Acf, *handle->CodCng.Ener) == 0) {
            /* transmit SID frame */
            *Ftyp = 2;
        }
        else {
            temp = abs_s(sub(curQGain, handle->CodCng.IRef));
            if(temp > ThreshGain) {
                *Ftyp = 2;
            }
            else {
                /* no transmission */
                *Ftyp = 0;
            }
        }
    }

 /*
  * If SID frame : Compute SID filter
  */
    if(*Ftyp == 2) {

 /*
  * Evaluates local stationnarity :
  * Computes difference between current filter and past average filter
  * if signal not locally stationary SID filter = current filter
  * else SID filter = past average filter
  */
        /* Compute past average filter */
        ComputePastAvFilter(handle->CodCng.SidLpc,handle) ;

        /* If adaptation enabled, fill noise filter */
        if ( !handle->VadStat.Aen ) {
            for(i=0; i<LpcOrder; i++) handle->VadStat.NLpc[i] = handle->CodCng.SidLpc[i];
        }

        /* Compute autocorr. of past average filter coefficients */
        CalcRC(handle->CodCng.SidLpc , handle->CodCng.RC, &handle->CodCng.ShRC);

        if(LpcDiff(handle->CodCng.RC, handle->CodCng.ShRC, handle->CodCng.Acf, *handle->CodCng.Ener) == 0){
            for(i=0; i<LpcOrder; i++) {
                handle->CodCng.SidLpc[i] = curCoeff[i];
            }
            CalcRC(curCoeff, handle->CodCng.RC, &handle->CodCng.ShRC);
        }

 /*
  * Compute SID frame codes
  */
        /* Compute LspSid */
        AtoLsp(handle->CodCng.LspSid, handle->CodCng.SidLpc, handle->CodStat.PrevLsp);
        Line->LspId = Lsp_Qnt(handle->CodCng.LspSid, handle->CodStat.PrevLsp);
        Lsp_Inq(handle->CodCng.LspSid, handle->CodStat.PrevLsp, Line->LspId, 0);

        Line->Sfs[0].Mamp = curQGain;
        handle->CodCng.IRef = curQGain;
        handle->CodCng.SidGain = Dec_SidGain(handle->CodCng.IRef);

    } /* end of Ftyp=2 case (SID frame) */

 /*
  * Compute new excitation
  */
    if(handle->CodCng.PastFtyp == 1) {
        handle->CodCng.CurGain = handle->CodCng.SidGain;
    }
    else {
          handle->CodCng.CurGain = extract_h(L_add( L_mult(handle->CodCng.CurGain,0x7000),
                    L_mult(handle->CodCng.SidGain,0x1000) ) ) ;
    }
    Calc_Exc_Rand(handle->CodCng.CurGain, handle->CodStat.PrevExc, DataExc,
                                                &handle->CodCng.RandSeed, Line,handle->mode);

 /*
  * Interpolate LSPs and update PrevLsp
  */
    Lsp_Int(QntLpc, handle->CodCng.LspSid, handle->CodStat.PrevLsp);
    for (i=0; i < LpcOrder ; i++) {
        handle->CodStat.PrevLsp[i] = handle->CodCng.LspSid[i];
    }

 /*
  * Output & save frame type info
  */
    handle->CodCng.PastFtyp = *Ftyp;
    return;
}


void CalcRC(Word16 *Coeff, Word16 *RC, Word16 *ShRC)
{
    int i, j;
    Word16 sh1;
    Word32 L_acc;

    L_acc = 0L;
    for(j=0; j<LpcOrder; j++) {
        L_acc = L_mac(L_acc, Coeff[j], Coeff[j]);
    }
    L_acc = L_shr(L_acc, 1);
    L_acc = L_add(L_acc, 0x04000000L);  /* 1 << 2 * Lpc_justif. */
    sh1 = norm_l(L_acc) - (Word16)2;    /* 1 bit because of x2 in RC[i], i> 0*/
                                /* & 1 bit margin for Itakura distance */
    L_acc = L_shl(L_acc, sh1); /* shift right if < 0 */
    RC[0] = round(L_acc);

    for(i=1; i<=LpcOrder; i++) {
        L_acc = L_mult( (Word16) 0xE000, Coeff[i-1]);   /* - (1 << Lpc_justif.) */
        for(j=0; j<LpcOrder-i; j++) {
            L_acc = L_mac(L_acc, Coeff[j], Coeff[j+i]);
        }
        L_acc = L_shl(L_acc, sh1);
        RC[i] = round(L_acc);
    }
    *ShRC = sh1;
    return;
}

Flag LpcDiff(Word16 *RC, Word16 ShRC, Word16 *ptrAcf, Word16 alpha)
{
    Word32 L_temp0, L_temp1;
    Word16 temp;
    int i;
    Flag diff;

    L_temp0 = 0L;
    for(i=0; i<=LpcOrder; i++) {
        temp = shr(ptrAcf[i], 2);  /* + 2 margin bits */
        L_temp0 = L_mac(L_temp0, RC[i], temp);
    }

    temp = mult_r(alpha, FracThresh);
    L_temp1 = L_add((Word32)temp, (Word32)alpha);
    temp = add(ShRC, 9);  /* 9 = Lpc_justif. * 2 - 15 - 2 */
    L_temp1 = L_shl(L_temp1, temp);

    if(L_temp0 < L_temp1) diff = 1;
    else diff = 0;
    return(diff);
}

void ComputePastAvFilter(Word16 *Coeff,ENC_HANDLE *handle)
{
    int i, j;
    Word16 *ptr_Acf;
    Word32 L_sumAcf[LpcOrderP1];
    Word16 Corr[LpcOrder], Err;
    Word16 sh1, temp;
    Word32 L_acc0;

    /* Search ShAcf min */
    sh1 = handle->CodCng.ShAcf[1];
    for(i=2; i <= NbAvAcf; i ++) {
        temp = handle->CodCng.ShAcf[i];
#ifdef VC
        if(temp < sh1) sh1 = temp;
#endif

#ifdef CCS
	sh1=_min2(sh1,temp);
#endif
    }
    sh1 = add(sh1, 14);     /* 2 bits of margin : NbAvAcf <= 4 */

    /* Compute sum of NbAvAcf frame-Acfs  */
    for(j=0; j <= LpcOrder; j++) L_sumAcf[j] = 0;

    ptr_Acf = handle->CodCng.Acf + LpcOrderP1;
    for(i=1; i <= NbAvAcf; i ++) {
        temp = sub(sh1, handle->CodCng.ShAcf[i]);
        for(j=0; j <= LpcOrder; j++) {
            L_acc0 = L_deposit_l(*ptr_Acf++);
            L_acc0 = L_shl(L_acc0, temp); /* shift right if temp<0 */
            L_sumAcf[j] = L_add(L_sumAcf[j], L_acc0);
        }
    }

    /* Normalize */
    temp = norm_l(L_sumAcf[0]);
    temp = sub(16, temp);
    if(temp < 0) temp = 0;
    Err = extract_l(L_shr(L_sumAcf[0],temp));
    for(i=1; i<LpcOrderP1; i++) {
        Corr[i-1] = extract_l(L_shr(L_sumAcf[i],temp));
    }

    Durbin(Coeff, Corr, Err, &temp);

    return;
}

