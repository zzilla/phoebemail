/*
**
** File:        "coder.h"
**
** Description:     Function prototypes and external declarations 
**          for "coder.c"
**  
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#ifndef _Coder_H_
#define _Coder_H_

void    Init_Coder( void);
Flag    Coder( Word16 *DataBuff, char *Vout );

#endif

