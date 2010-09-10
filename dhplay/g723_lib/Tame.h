
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.0
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
#ifndef _Tame_H_
#define _Tame_H_

extern void Update_Err(Word16 Olp, Word16 AcLg, Word16 AcGn,ENC_HANDLE *handle);
extern Word16 Test_Err(Word16 Lag1, Word16 Lag2,ENC_HANDLE *handle);

#endif
