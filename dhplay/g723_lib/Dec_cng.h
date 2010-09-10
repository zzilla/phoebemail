/*
**
** File:        "dec_cng.h"
**
** Description:     Function prototypes for "dec_cng.c"
**
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#ifndef _Dec_cng_H_
#define _Dec_cng_H_

void Init_Dec_Cng(DEC_HANDLE *handle);
void Dec_Cng(Word16 Ftyp, LINEDEF *Line, Word16 *DataExc,Word16 *QntLpc,DEC_HANDLE *handle);

#endif


