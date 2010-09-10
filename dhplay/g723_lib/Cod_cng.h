/*
**
** File:        "cod_cng.h"
**
** Description:     Function prototypes for "cod_cng.c"
**
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#ifndef _Cod_cng_H_
#define _Cod_cng_H_

void Init_Cod_Cng(ENC_HANDLE *handle);
void Cod_Cng(Word16 *DataExc, Word16 *Ftyp, LINEDEF *Line, Word16 *QntLpc,ENC_HANDLE *handle);
void Update_Acf(Word16 *Acfsf, Word16 *Shsf,ENC_HANDLE *handle);

#endif


