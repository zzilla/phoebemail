#ifndef _Util_lbc_H
#define _Util_lbc_H

void    Rem_Dc( Word16 *Dpnt,ENC_HANDLE *handle  );
Word16  Vec_Norm( Word16 *Vect, Word16 Len );
void    Mem_Shift( Word16 *PrevDat, Word16 *DataBuff );
void    Line_Pack( LINEDEF *Line, char *Vout, Word16 Ftyp,ENC_HANDLE *handle,char stride );
Word16* Par2Ser( Word32 Inp, Word16 *Pnt, int BitNum );
LINEDEF  Line_Unpk( char *Vinp, Word16 *Ftyp, Word16 Crc ,DEC_HANDLE *handle);
Word32  Ser2Par( Word16 **Pnt, int Count );
Word32  Comp_En( Word16 *Dpnt );
Word16  Sqrt_lbc( Word32 Num );
Word16  Rand_lbc( Word16 *p );
void    Scale( Word16 *Tv, Word32 Sen, DEC_HANDLE *handle);


#endif

