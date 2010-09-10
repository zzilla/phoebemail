/*
**
** File:        "lpc.h"
**
** Description:     Function prototypes for "lpc.c"
**  
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#ifndef _Lpc_H_
#define _Lpc_H_

void    Comp_Lpc( Word16 *UnqLpc, ENC_HANDLE *handle, Word16 *DataBuff );
Word16  Durbin( Word16 *Lpc, Word16 *Corr, Word16 Err, Word16 *Pk2 );
void    Wght_Lpc( Word16 *PerLpc, Word16 *UnqLpc );
void    Error_Wght( Word16 *Dpnt, Word16 *PerLpc,ENC_HANDLE *handle );
void    Comp_Ir( Word16 *ImpResp, Word16 *QntLpc, Word16 *PerLpc, PWDEF Pw );
void    Sub_Ring( Word16 *Dpnt, Word16 *QntLpc, Word16 *PerLpc, ENC_HANDLE *handle, PWDEF Pw );
void    Upd_Ring( Word16 *Dpnt, Word16 *QntLpc, Word16 *PerLpc, Word16 *PrevErr ,ENC_HANDLE *handle );
void    Synt( Word16 *Dpnt, Word16 *Lpc ,DEC_HANDLE *handle);   
Word32  Spf( Word16 *Tv, Word16 *Lpc,DEC_HANDLE *handle );

#endif
