/*
**
** File:        "vad.h"
**
** Description:     Function prototypes for "vad.c"
**  
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#ifndef _Vad_H_
#define _Vad_H_


extern void    Init_Vad(ENC_HANDLE *handle) ;
extern Flag Comp_Vad( Word16 *Dpnt,ENC_HANDLE *handle);

#endif
