/*
**
** File:        "decod.h"
**
** Description:     Function prototypes and external declarations 
**          for "decod.c"
**  
*/


/*
    ITU-T AV.25Y Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#ifndef _Decod_H_
#define _Decod_H_

void  Init_Decod(void);
Flag    Decod( Word16 *DataBuff, char *Vinp, Word16 Crc );

#endif




