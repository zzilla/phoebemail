/*
**
** File:        "util_cng.h"
**
** Description:     Function prototypes for "util_cng.c"
**
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#ifndef _Util_cng_H
#define _Util_cng_H

extern void Calc_Exc_Rand(Word16 cur_gain, Word16 *PrevExc, Word16 *DataExc,
                                      Word16 *nRandom, LINEDEF *Line,eG723Mode mode);
extern Word16 Qua_SidGain(Word16 *Ener, Word16 *shEner, Word16 nq);
extern Word16 Dec_SidGain(Word16 i_gain);

#endif


