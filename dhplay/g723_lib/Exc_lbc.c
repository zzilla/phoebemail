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


Word16   Estim_Pitch( Word16 *Dpnt, Word16 Start )
{
    int   i,j   ;

    Word32   Acc0,Acc1   ;

    Word16   Exp,Tmp  ;
    Word16   Ccr,Enr  ;

    Word16   Indx = (Word16) PitchMin ;

    Word16   Mxp = (Word16) 30 ;
    Word16   Mcr = (Word16) 0x4000 ;
    Word16   Mnr = (Word16) 0x7fff ;

    Word16   Pr ;

    /* Init the energy estimate */
    Pr = Start - (Word16)PitchMin + (Word16)1 ;
    Acc1 = (Word32) 0 ;
    for ( j = 0 ; j < 2*SubFrLen ; j ++ )
        Acc1 = L_mac( Acc1, Dpnt[Pr+j], Dpnt[Pr+j] ) ;

    /* Main Olp search loop */
    for ( i = PitchMin ; i <= PitchMax-3 ; i ++ ) {

        Pr = sub( Pr, (Word16) 1 ) ;

        /* Energy update */
        Acc1 = L_msu( Acc1, Dpnt[Pr+2*SubFrLen], Dpnt[Pr+2*SubFrLen] ) ;
        Acc1 = L_mac( Acc1, Dpnt[Pr], Dpnt[Pr] ) ;

        /*  Compute the cross */
        Acc0 = (Word32) 0 ;
        for ( j = 0 ; j < 2*SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Dpnt[Start+j], Dpnt[Pr+j] ) ;

        if ( Acc0 > (Word32) 0 ) {

            /* Compute Exp and mant of the cross */
            Exp = norm_l( Acc0 ) ;
            Acc0 = L_shl( Acc0, Exp ) ;
            Exp = shl( Exp, (Word16) 1 ) ;
            Ccr = round( Acc0 ) ;
            Acc0 = L_mult( Ccr, Ccr ) ;
            Ccr = norm_l( Acc0 ) ;
            Acc0 = L_shl( Acc0, Ccr ) ;
            Exp = add( Exp, Ccr ) ;
            Ccr = extract_h( Acc0 ) ;

            /* Do the same with energy */
            Acc0 = Acc1 ;
            Enr = norm_l( Acc0 ) ;
            Acc0 = L_shl( Acc0, Enr ) ;
            Exp = sub( Exp, Enr ) ;
            Enr = round( Acc0 ) ;

            if ( Ccr >= Enr ) {
                Exp = sub( Exp, (Word16) 1 ) ;
                Ccr = shr( Ccr, (Word16) 1 ) ;
            }

            if ( Exp <= Mxp ) {

                if ( (Exp+1) < Mxp ) {
                    Indx = (Word16) i ;
                    Mxp = Exp ;
                    Mcr = Ccr ;
                    Mnr = Enr ;
                    continue ;
                }

                if ( (Exp+1) == Mxp )
                    Tmp = shr( Mcr, (Word16) 1 ) ;
                else
                    Tmp = Mcr ;

                /* Compare with equal exponents */
                Acc0 = L_mult( Ccr, Mnr ) ;
                Acc0 = L_msu( Acc0, Enr, Tmp ) ;
                if ( Acc0 > (Word32) 0 ) {

                    if ( ((Word16)i - Indx) < (Word16) PitchMin ) {
                        Indx = (Word16) i ;
                        Mxp = Exp ;
                        Mcr = Ccr ;
                        Mnr = Enr ;
                    }

                    else {
                        Acc0 = L_mult( Ccr, Mnr ) ;
                        Acc0 = L_negate(L_shr( Acc0, (Word16) 2 ) ) ;
                        Acc0 = L_mac( Acc0, Ccr, Mnr ) ;
                        Acc0 = L_msu( Acc0, Enr, Tmp ) ;
                        if ( Acc0 > (Word32) 0 ) {
                            Indx = (Word16) i ;
                            Mxp = Exp ;
                            Mcr = Ccr ;
                            Mnr = Enr ;
                        }
                    }
                }
            }
        }
    }

    return Indx ;
}

void  Get_Rez( Word16 *Tv, Word16 *PrevExc, Word16 Lag )
{
    int   i  ;

    for ( i = 0 ; i < ClPitchOrd/2 ; i ++ )
        Tv[i] = PrevExc[PitchMax - (int) Lag - ClPitchOrd/2 + i] ;

    for ( i = 0 ; i < SubFrLen+ClPitchOrd/2 ; i ++ )
        Tv[ClPitchOrd/2+i] = PrevExc[PitchMax - (int)Lag + i%(int)Lag] ;

    return;
}

void  Decod_Acbk( Word16 *Tv, Word16 *PrevExc, Word16 Olp, Word16 Lid,Word16 Gid ,eG723Mode mode)
{
    int   i,j   ;

    Word32   Acc0  ;
    Word16   RezBuf[SubFrLen+ClPitchOrd-1] ;
    Word16  *sPnt ;

    Get_Rez( RezBuf, PrevExc, (Word16)(Olp - (Word16)Pstep + Lid) ) ;

    /* Select Quantization tables */
    i = 0 ;
    if ( mode == G723_63 ) {
        if ( Olp >= (Word16) (SubFrLen-2) ) i ++ ;
    }
    else {
        i = 1;
    }
    sPnt = AcbkGainTablePtr[i] ;

    sPnt += (int)Gid*20 ;

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Word32) 0 ;
        for ( j = 0 ; j < ClPitchOrd ; j ++ )
            Acc0 = L_mac( Acc0, RezBuf[i+j], sPnt[j] ) ;
        Acc0 = L_shl( Acc0, (Word16) 1 ) ;
        Tv[i] = round( Acc0 ) ;
    }

    return;
}

PWDEF Comp_Pw( Word16 *Dpnt, Word16 Start, Word16 Olp )
{

    int   i,j   ;

    Word32   Lcr[15] ;
    Word16   Scr[15] ;
    PWDEF    Pw ;

    Word32   Acc0,Acc1   ;
    Word16   Exp   ;

    Word16   Ccr,Enr  ;
    Word16   Mcr,Mnr  ;

    /* Compute and save target energy */
    Lcr[0] = (Word32) 0 ;
    for ( i = 0 ; i < SubFrLen ; i ++ )
        Lcr[0] = L_mac( Lcr[0], Dpnt[Start+i], Dpnt[Start+i] ) ;

    /* Compute all Crosses and energys */
    for ( i = 0 ; i <= 2*PwRange ; i ++ ) {

        Acc1 = Acc0 = (Word32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ ) {
            Acc0 = L_mac( Acc0, Dpnt[Start+j],
                                            Dpnt[Start-(Olp-PwRange+i)+j]) ;
            Acc1 = L_mac( Acc1, Dpnt[Start-(Olp-PwRange+i)+j],
                                            Dpnt[Start-(Olp-PwRange+i)+j] ) ;
        }

        /* Save both */
        Lcr[2*i+1] = Acc1 ;
        Lcr[2*i+2] = Acc0 ;
    }

    /* Normalize to maximum */
    Acc1 = (Word32) 0 ;
    for ( i = 0 ; i < 15 ; i ++ ) {
        Acc0 = Lcr[i] ;
        Acc0 = L_abs( Acc0 ) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    Exp = norm_l( Acc1 ) ;
    for ( i = 0 ; i < 15 ; i ++ ) {
        Acc0 = L_shl( Lcr[i], Exp ) ;
        Scr[i] = round( Acc0 ) ;

    }

    /* Find the best pair */
    Pw.Indx = (Word16) -1 ;
    Pw.Gain = (Word16) 0  ;

    Mcr = (Word16) 1 ;
    Mnr = (Word16) 0x7fff ;

    for ( i = 0 ; i <= 2*PwRange ; i ++ ) {

        Enr = Scr[2*i+1] ;
        Ccr = Scr[2*i+2] ;

        if ( Ccr <= (Word16) 0 )
            continue ;

        Exp = mult_r( Ccr, Ccr ) ;

        /* Compute the cross */
        Acc0 = L_mult( Exp, Mnr ) ;
        Acc0 = L_msu ( Acc0, Enr, Mcr ) ;

        if ( Acc0 > (Word32) 0 ) {
            Mcr = Exp ;
            Mnr = Enr ;
            Pw.Indx = (Word16)i ;
        }
    }

    if ( Pw.Indx == -1 ) {
        Pw.Indx = Olp ;
        return Pw ;
    }

    /* Check the db limit */
    Acc0 = L_mult( Scr[0], Mnr ) ;
    Acc1 = Acc0 ;
    Acc0 = L_shr( Acc0, (Word16) 2 ) ;
    Acc1 = L_shr( Acc1, (Word16) 3 ) ;
    Acc0 = L_add( Acc0, Acc1 ) ;
    Acc1 = L_mult( Scr[2*Pw.Indx+2], Scr[2*Pw.Indx+2] ) ;
    Acc0 = L_sub( Acc0, Acc1 ) ;

    if ( Acc0 < (Word32) 0 ) {

        Exp = Scr[2*Pw.Indx + 2] ;

        if ( Exp >= Mnr )
            Pw.Gain = PwConst ;
        else {
            Pw.Gain = div_s( Exp, Mnr ) ;
            Pw.Gain = mult_r( Pw.Gain, PwConst ) ;
        }
    }

    Pw.Indx = Olp - PwRange + Pw.Indx ;

    return Pw ;

}

void  Filt_Pw( Word16 *DataBuff, Word16 *Dpnt, Word16 Start, PWDEF Pw )
{
    int   i  ;

    Word32   Acc0 ;

    /* Perform the harmonic weighting */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_deposit_h( Dpnt[PitchMax+Start+i] ) ;
        Acc0 = L_msu( Acc0, Pw.Gain, Dpnt[PitchMax+Start-Pw.Indx+i] ) ;
        DataBuff[Start+(Word16)i] = round( Acc0 ) ;
    }

    return;
}



void  Find_Acbk( Word16 *Tv, Word16 *ImpResp, ENC_HANDLE *handle, LINEDEF
*Line, Word16 Sfc )
{
    int   i,j,k,l  ;

    Word32   Acc0,Acc1 ;

    Word16   RezBuf[SubFrLen+ClPitchOrd-1] ;
    Word16   FltBuf[ClPitchOrd][SubFrLen] ;
    Word32   CorBuf[4*(2*ClPitchOrd + ClPitchOrd*(ClPitchOrd-1)/2)] ;
    Word32   *lPnt ;

    Word16   CorVct[4*(2*ClPitchOrd + ClPitchOrd*(ClPitchOrd-1)/2)] ;
    Word16   *sPnt ;

    Word16   Olp ;
    Word16   Lid ;
    Word16   Gid ;
    Word16   Hb  ;
    Word16   Exp ;
    Word16   Bound[2] ;

    Word16   Lag1, Lag2;
    Word16   off_filt;

    /* Init constants */
    Olp = (*Line).Olp[shr(Sfc, (Word16) 1)] ;
    Lid = (Word16) Pstep ;
    Gid = (Word16) 0 ;
    Hb  = (Word16) 3 + (Sfc & (Word16) 1 ) ;

    /* For even frames only */
    if ( (Sfc & (Word16)1) == (Word16) 0 ) {
        if ( Olp == (Word16) PitchMin )
            Olp = add( Olp, (Word16) 1 ) ;
#ifdef VC
        if ( Olp > (Word16) (PitchMax-5) )
            Olp = (Word16)(PitchMax-5) ;
#endif
#ifdef CCS
		Olp=_min2(Olp,(Word16) (PitchMax-5));
#endif
    }

    lPnt = CorBuf ;
    for ( k = 0 ; k < (int) Hb ; k ++ ) {

        /* Get residual from the excitation buffer */
        Get_Rez( RezBuf, handle->CodStat.PrevExc, (Word16)(Olp-(Word16)Pstep+k) ) ;

        /* Filter the last one using the impulse response */
        for ( i = 0 ; i < SubFrLen ; i ++ ) {
            Acc0 = (Word32) 0 ;
            for ( j = 0 ; j <= i ; j ++ )
                Acc0 = L_mac( Acc0, RezBuf[ClPitchOrd-1+j], ImpResp[i-j] ) ;
            FltBuf[ClPitchOrd-1][i] = round( Acc0 ) ;
        }

        /* Update all the others */
        for ( i = ClPitchOrd-2 ; i >= 0 ; i -- ) {
            FltBuf[i][0] = mult_r( RezBuf[i], (Word16) 0x2000 ) ;
            for ( j = 1 ; j < SubFrLen ; j ++ ) {
                Acc0 = L_deposit_h( FltBuf[i+1][j-1] ) ;
                Acc0 = L_mac( Acc0, RezBuf[i], ImpResp[j] ) ;
                FltBuf[i][j] = round( Acc0 ) ;
            }
        }

        /* Compute the cross with the signal */
        for ( i = 0 ; i < ClPitchOrd ; i ++ ) {
            Acc1 = (Word32) 0 ;
            for ( j = 0 ; j < SubFrLen ; j ++ ) {
                Acc0 = L_mult( Tv[j], FltBuf[i][j] ) ;
                Acc1 = L_add( Acc1, L_shr( Acc0, (Word16) 1 ) ) ;
            }
            *lPnt ++ = L_shl( Acc1, (Word16) 1 ) ;
        }

        /* Compute the energies */
        for ( i = 0 ; i < ClPitchOrd ; i ++ ) {
            Acc1 = (Word32) 0 ;
            for ( j = 0 ; j < SubFrLen ; j ++ )
                Acc1 = L_mac( Acc1, FltBuf[i][j], FltBuf[i][j] ) ;
            *lPnt ++ = Acc1 ;
        }

        /* Compute the between crosses */
        for ( i = 1 ; i < ClPitchOrd ; i ++ ) {
            for ( j = 0 ; j < i ; j ++ ) {
                Acc1 = (Word32) 0 ;
                for ( l = 0 ; l < SubFrLen ; l ++ ) {
                    Acc0 = L_mult( FltBuf[i][l], FltBuf[j][l] ) ;
                    Acc1 = L_add( Acc1, L_shr( Acc0, (Word16) 1 ) ) ;
                }
                *lPnt ++ = L_shl( Acc1, (Word16) 2 ) ;
            }
        }
    }


    /* Find Max and normalize */
    Acc1 = (Word32) 0 ;
    for ( i = 0 ; i < Hb*20 ; i ++ ) {
        Acc0 = L_abs(CorBuf[i]) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    Exp = norm_l( Acc1 ) ;
    /* Convert to shorts */
    for ( i = 0 ; i < Hb*20 ; i ++ ) {
        Acc0 = L_shl( CorBuf[i], Exp ) ;
        CorVct[i] = round( Acc0 ) ;
    }

    /* Test potential error */
    Lag1 = Olp-(Word16)Pstep;
    Lag2 = Olp-(Word16)Pstep+ Hb -(Word16)1;
    off_filt = Test_Err(Lag1, Lag2,handle);
    Bound[0] =  NbFilt085_min + shl(off_filt,2);
#ifdef VC
    if(Bound[0] > NbFilt085) Bound[0] = NbFilt085;
#endif

#ifdef CCS
	Bound[0]=_min2(Bound[0],NbFilt085);
#endif
    Bound[1] =  NbFilt170_min + shl(off_filt,3);

#ifdef VC
    if(Bound[1] > NbFilt170) Bound[1] = NbFilt170;
#endif
#ifdef CCS
	Bound[1]=_min2(Bound[1],NbFilt170);
#endif
    /* Init the search loop */
    Acc1 = (Word32) 0 ;

    for ( k = 0 ; k < (int) Hb ; k ++ ) {

        /* Select Quantization tables */
        l = 0 ;
        if ( handle->mode == G723_63 ){
            if ( (Sfc & (Word16) 1) == (Word16) 0 ) {
                if ( (int)Olp-Pstep+k >= SubFrLen-2 )l ++ ;
            }
            else {
                if ( (int)Olp >= SubFrLen-2 ) l ++ ;
            }
        }
        else {
            l = 1;
        }


        sPnt = AcbkGainTablePtr[l] ;

        for ( i = 0 ; i < (int) Bound[l] ; i ++ ) {

            Acc0 = (Word32) 0 ;
            for ( j = 0 ; j < 20 ; j ++ )
                Acc0 = L_add( Acc0, L_shr( L_mult(CorVct[k*20+j], *sPnt ++),
(Word16) 1 ) ) ;

            if ( Acc0 > Acc1 ) {
                Acc1 = Acc0 ;
                Gid = (Word16) i ;
                Lid = (Word16) k ;
            }
        }
    }

    /* Modify Olp for even sub frames */
    if ( (Sfc & (Word16) 1 ) == (Word16) 0 ) {
        Olp = Olp - (Word16) Pstep + Lid ;
        Lid = (Word16) Pstep ;
    }

    /* Save Gains and Olp */
    (*Line).Sfs[Sfc].AcLg = Lid ;
    (*Line).Sfs[Sfc].AcGn = Gid ;
    (*Line).Olp[shr(Sfc, (Word16) 1)] = Olp ;

    /* Decode the Acbk contribution and subtract it */
    Decod_Acbk( RezBuf,handle->CodStat.PrevExc, Olp, Lid, Gid,handle->mode ) ;

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_deposit_h( Tv[i] ) ;
        Acc0 = L_shr( Acc0, (Word16) 1 ) ;

        for ( j = 0 ; j <= i ; j ++ )
            Acc0 = L_msu( Acc0, RezBuf[j], ImpResp[i-j] ) ;
        Acc0 = L_shl( Acc0, (Word16) 1 ) ;
        Tv[i] = round( Acc0 ) ;
    }

    return;
}

void  Find_Fcbk( Word16 *Dpnt, Word16 *ImpResp, LINEDEF *Line, Word16 Sfc,ENC_HANDLE *handle )
{
    int   i  ;
    Word16 T0_acelp, gain_T0;
    Word16   Srate ;


    BESTDEF  Best ;

    switch(handle->mode)  {

        case G723_63: {

            Srate = Nb_puls[(int)Sfc] ;
            Best.MaxErr = (Word32) 0xc0000000L ;
            Find_Best( &Best, Dpnt, ImpResp, Srate, (Word16) SubFrLen ) ;
            if ( (*Line).Olp[Sfc>>1] < (Word16) (SubFrLen-2) ) {
                Find_Best( &Best, Dpnt, ImpResp, Srate, (*Line).Olp[Sfc>>1]);
            }

            /* Reconstruct the excitation */
#ifdef VC
            for ( i = 0 ; i <  SubFrLen ; i ++ )
                Dpnt[i] = (Word16) 0 ;
#endif

#ifdef CCS
			for ( i = 0 ; i < SubFrLen ; i =i+4 )
				_memd8( & Dpnt[i])=0;
#endif

            for ( i = 0 ; i < Srate ; i ++ )
                Dpnt[Best.Ploc[i]] = Best.Pamp[i] ;

            /* Code the excitation */
            Fcbk_Pack( Dpnt, &((*Line).Sfs[Sfc]), &Best, Srate ) ;

            if ( Best.UseTrn == (Word16) 1 )
                Gen_Trn( Dpnt, Dpnt, (*Line).Olp[Sfc>>1] ) ;

            break;
        }

        case G723_53: {

            T0_acelp = search_T0(
                    (Word16) ((*Line).Olp[Sfc>>1]-1+(*Line).Sfs[Sfc].AcLg),
                    (*Line).Sfs[Sfc].AcGn, &gain_T0 );

            (*Line).Sfs[Sfc].Ppos = ACELP_LBC_code(
                    Dpnt, ImpResp, T0_acelp, Dpnt, &(*Line).Sfs[Sfc].Mamp,
                    &(*Line).Sfs[Sfc].Grid, &(*Line).Sfs[Sfc].Pamp, gain_T0 );

            (*Line).Sfs[Sfc].Tran = 0;

            break;
        }
    }

    return;
}
void  Gen_Trn( Word16 *Dst, Word16 *Src, Word16 Olp )
{
    int   i  ;

    Word16   Tmp0,Tmp1   ;
    Word16   Tmp[SubFrLen] ;

    Tmp0 = Olp ;
#ifdef VC
    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Tmp[i] = Src[i] ;
        Dst[i] = Src[i] ;
    }
#endif

#ifdef CCS
	for ( i = 0 ; i < SubFrLen ; i=i+2 ) {
		_mem4( & Tmp[i])=_pack2(Src[i+1],Src[i]);
		_mem4( & Dst[i])=_pack2(Src[i+1],Src[i]);
    }

#endif

    while ( Tmp0 < SubFrLen ) {
        for ( i = (int) Tmp0 ; i < SubFrLen ; i ++ ) {
            Tmp1 = add( Dst[i], Tmp[i-(int)Tmp0] ) ;
            Dst[i] = Tmp1 ;
        }
        Tmp0 = add( Tmp0, Olp ) ;
    }

    return;
}

void  Find_Best( BESTDEF *Best, Word16 *Tv, Word16 *ImpResp, Word16 Np,Word16 Olp )
{

    int   i,j,k,l  ;
    BESTDEF  Temp  ;

    Word16   Exp ,Exp_tmp  ;
    Word16   MaxAmpId ;
    Word16   MaxAmp   ;
    Word32   Acc0,Acc0_tmp,Acc1,Acc2 ;

    Word16   Imr[SubFrLen]  ;
    Word16   OccPos[SubFrLen] ;
    Word16   ImrCorr[SubFrLen] ;
    Word32   ErrBlk[SubFrLen] ;
    Word32   WrkBlk[SubFrLen] ;


    /* Update Impulse response */
    if ( Olp < (Word16) (SubFrLen-2) ) {
        Temp.UseTrn = (Word16) 1 ;
        Gen_Trn( Imr, ImpResp, Olp ) ;
    }
    else {
        Temp.UseTrn = (Word16) 0 ;
        for ( i = 0 ; i < SubFrLen ; i ++ )
            Imr[i] = ImpResp[i] ;
    }

    /* Scale Imr to avoid overflow */
    for ( i = 0 ; i < SubFrLen ; i ++ )
        OccPos[i] = shr( Imr[i], (Word16) 1 ) ;

    /* Compute Imr AutoCorr function */
    Acc0 = (Word32) 0 ;
    for ( i = 0 ; i < SubFrLen ; i ++ )
        Acc0 = L_mac( Acc0, OccPos[i], OccPos[i] ) ;

    Exp = norm_l( Acc0 ) ;
    Acc0 = L_shl( Acc0, Exp ) ;
    ImrCorr[0] = round( Acc0 ) ;

    /* Compute all the other */
/*    for ( i = 1 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Word32) 0 ;
        for ( j = i ; j < SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, OccPos[j], OccPos[j-i] ) ;
        Acc0 = L_shl( Acc0, Exp ) ;
        ImrCorr[i] = round( Acc0 ) ;
    }
*/
    /* Cross correlation with the signal */
/*    Exp = sub( Exp, 4 ) ;
    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Word32) 0 ;
        for ( j = i ; j < SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Tv[j], Imr[j-i] ) ;
        ErrBlk[i] = L_shl( Acc0, Exp ) ;
    }*/
	Exp_tmp=sub( Exp, 4 ) ;
	for ( i = 1 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Word32) 0 ;
		Acc0_tmp = (Word32) 0 ;
        for ( j = i ; j < SubFrLen ; j ++ )
		{
            Acc0 = L_mac( Acc0, OccPos[j], OccPos[j-i] ) ;
			Acc0_tmp = L_mac( Acc0_tmp, Tv[j], Imr[j-i] ) ;
		}
        Acc0 = L_shl( Acc0, Exp ) ;
        ImrCorr[i] = round( Acc0 ) ;
		ErrBlk[i] = L_shl( Acc0_tmp, Exp_tmp ) ;
    }



    /* Search for the best sequence */
    for ( k = 0 ; k < Sgrid ; k ++ ) {

        Temp.GridId = (Word16) k ;

        /* Find maximum amplitude */
        Acc1 = (Word32) 0 ;
        for ( i = k ; i < SubFrLen ; i += Sgrid ) {
            Acc0 = L_abs( ErrBlk[i] ) ;
            if ( Acc0 >= Acc1 ) {
                Acc1 = Acc0 ;
                Temp.Ploc[0] = (Word16) i ;
            }
        }

        /* Quantize the maximum amplitude */
        Acc2 = Acc1 ;
        Acc1 = (Word32) 0x40000000L ;
        MaxAmpId = (Word16) (NumOfGainLev - MlqSteps) ;

        for ( i = MaxAmpId ; i >= MlqSteps ; i=i-2 ) {
            Acc0 = L_mult( FcbkGainTable[i], ImrCorr[0] ) ;
            Acc0 = L_sub( Acc0, Acc2 ) ;
            Acc0 = L_abs( Acc0 ) ;
   
			Acc0_tmp = L_mult( FcbkGainTable[i-1], ImrCorr[0] ) ;
            Acc0_tmp = L_sub( Acc0_tmp, Acc2 ) ;
            Acc0_tmp = L_abs( Acc0_tmp ) ;	 
                   
            if ( Acc0 < Acc1 ) {
                Acc1 = Acc0 ;
                MaxAmpId = (Word16) i ;
				if(Acc0<Acc0_tmp) break;
            }
            if ( Acc0_tmp < Acc1 ) {
                Acc1 = Acc0_tmp ;
                MaxAmpId = (Word16) i ;
            }
        }
        MaxAmpId -- ;

        for ( i = 1 ; i <=2*MlqSteps ; i ++ ) {

            for ( j = k ; j < SubFrLen ; j += Sgrid ) {
                WrkBlk[j] = ErrBlk[j] ;
                OccPos[j] = (Word16) 0 ;
            }
            Temp.MampId = MaxAmpId - (Word16) MlqSteps + (Word16) i ;

            MaxAmp = FcbkGainTable[Temp.MampId] ;

            if ( WrkBlk[Temp.Ploc[0]] >= (Word32) 0 )
                Temp.Pamp[0] = MaxAmp ;
            else
                Temp.Pamp[0] = negate(MaxAmp) ;

            OccPos[Temp.Ploc[0]] = (Word16) 1 ;

            for ( j = 1 ; j < Np ; j ++ ) {

                Acc1 = (Word32) 0xc0000000L ;

                for ( l = k ; l < SubFrLen ; l += Sgrid ) {

                    if ( OccPos[l] != (Word16) 0 )
                        continue ;

                    Acc0 = WrkBlk[l] ;
                    Acc0 = L_msu( Acc0, Temp.Pamp[j-1],
                            ImrCorr[abs_s((Word16)(l-Temp.Ploc[j-1]))] ) ;
                    WrkBlk[l] = Acc0 ;
                    Acc0 = L_abs( Acc0 ) ;
                    if ( Acc0 > Acc1 ) {
                        Acc1 = Acc0 ;
                        Temp.Ploc[j] = (Word16) l ;
                    }
                }

                if ( WrkBlk[Temp.Ploc[j]] >= (Word32) 0 )
                    Temp.Pamp[j] = MaxAmp ;
                else
                    Temp.Pamp[j] = negate(MaxAmp) ;

                OccPos[Temp.Ploc[j]] = (Word16) 1 ;
            }

            /* Compute error vector */
#ifdef VC
            for ( j = 0 ; j < SubFrLen ; j ++ )
                OccPos[j] = (Word16) 0 ;
#endif

#ifdef CCS
			for ( j = 0 ; j < SubFrLen ; j =j+4 )
			{
				_memd8( & OccPos[j])=0;
			}
#endif

            for ( j = 0 ; j < Np ; j ++ )
                OccPos[Temp.Ploc[j]] = Temp.Pamp[j] ;

            for ( l = SubFrLen-1 ; l >= 0 ; l -- ) {
                Acc0 = (Word32) 0 ;
                for ( j = 0 ; j <= l ; j ++ )
                    Acc0 = L_mac( Acc0, OccPos[j], Imr[l-j] ) ;
//				Acc0 = L_mac( Acc0, OccPos[0], Imr[l] ) ;
//				Acc0 = L_mac( Acc0, OccPos[1], Imr[l-1] ) ;
                Acc0 = L_shl( Acc0, (Word16) 2 ) ;
                OccPos[l] = extract_h( Acc0 ) ;
            }

            /* Evaluate error */
            Acc1 = (Word32) 0 ;
            for ( j = 0 ; j < SubFrLen ; j ++ ) {
                Acc1 = L_mac( Acc1, Tv[j], OccPos[j] ) ;
                Acc0 = L_mult( OccPos[j], OccPos[j] ) ;
                Acc1 = L_sub( Acc1, L_shr( Acc0, (Word16) 1 ) ) ;
            }

            if ( Acc1 > (*Best).MaxErr ) {
                (*Best).MaxErr = Acc1 ;
                (*Best).GridId = Temp.GridId ;
                (*Best).MampId = Temp.MampId ;
                (*Best).UseTrn = Temp.UseTrn ;
#ifdef VC
                for ( j = 0 ; j < Np ; j ++ ) {
                    (*Best).Pamp[j] = Temp.Pamp[j] ;
                    (*Best).Ploc[j] = Temp.Ploc[j] ;
                }
#endif
#ifdef CCS
				for ( j = 0 ; j < Np ; j=j+2 ) {
					_mem4( & (*Best).Pamp[j])=_pack2(Temp.Pamp[j+1],Temp.Pamp[j]);
					_mem4( & (*Best).Ploc[j])=_pack2(Temp.Ploc[j+1],Temp.Ploc[j]);
                }
#endif
            }
        }
    }
    return;
}

Word16 search_T0 ( Word16 T0, Word16 Gid, Word16 *gain_T0)
{

    Word16 T0_mod;

    T0_mod = T0+epsi170[Gid];
    *gain_T0 = gain170[Gid];

    return(T0_mod);
}

void  Fcbk_Pack( Word16 *Dpnt, SFSDEF *Sfs, BESTDEF *Best, Word16 Np )
{
    int   i,j   ;


    /* Code the amplitudes and positions */
    j = MaxPulseNum - (int) Np ;

    (*Sfs).Pamp = (Word16) 0 ;
    (*Sfs).Ppos = (Word32) 0 ;

    for ( i = 0 ; i < SubFrLen/Sgrid ; i ++ ) {

        if ( Dpnt[(int)(*Best).GridId + Sgrid*i] == (Word16) 0 )
            (*Sfs).Ppos = L_add( (*Sfs).Ppos, CombinatorialTable[j][i] ) ;
        else {
            (*Sfs).Pamp = shl( (*Sfs).Pamp, (Word16) 1 ) ;
            if ( Dpnt[(int)(*Best).GridId + Sgrid*i] < (Word16) 0 )
                (*Sfs).Pamp = add( (*Sfs).Pamp, (Word16) 1 ) ;

            j ++ ;
            /* Check for end */
            if ( j == MaxPulseNum )
                break ;
        }
    }

    (*Sfs).Mamp = (*Best).MampId ;
    (*Sfs).Grid = (*Best).GridId ;
    (*Sfs).Tran = (*Best).UseTrn ;

    return;
}

Word16  ACELP_LBC_code(Word16 X[], Word16 h[], Word16 T0, Word16 code[],
        Word16 *ind_gain, Word16 *shift, Word16 *sign, Word16 gain_T0)
{
    Word16 i, index, gain_q;
    Word16 Dn[SubFrLen2], tmp_code[SubFrLen2];
    Word16 rr[DIM_RR];

 /*
  * Include fixed-gain pitch contribution into impulse resp. h[]
  * Find correlations of h[] needed for the codebook search.
 */
    for (i = 0; i < SubFrLen; i++)    /* Q13 -->  Q12*/
        h[i] = shr(h[i], 1);

    if (T0 < SubFrLen-2) {
        for (i = T0; i < SubFrLen; i++)    /* h[i] += gain_T0*h[i-T0] */
        h[i] = add(h[i], mult(h[i-T0], gain_T0));
    }

    Cor_h(h, rr);

 /*
  * Compute correlation of target vector with impulse response.
  */

    Cor_h_X(h, X, Dn);

 /*
  * Find innovative codebook.
  * rr input matrix autocorrelation
  *    output filtered codeword
  */

    index = D4i64_LBC(Dn, rr, h, tmp_code, rr, shift, sign);

 /*
  * Compute innovation vector gain.
  * Include fixed-gain pitch contribution into code[].
  */

    *ind_gain = G_code_g723(X, rr, &gain_q);

    for (i = 0; i < SubFrLen; i++)   {
        code[i] = i_mult(tmp_code[i], gain_q);
    }

    if(T0 < SubFrLen-2)
        for (i = T0; i < SubFrLen; i++)    /* code[i] += gain_T0*code[i-T0] */
            code[i] = add(code[i], mult(code[i-T0], gain_T0));


    return index;
}


void Cor_h(Word16 *H, Word16 *rr)
{
    Word16 *rri0i0, *rri1i1, *rri2i2, *rri3i3;
    Word16 *rri0i1, *rri0i2, *rri0i3;
    Word16 *rri1i2, *rri1i3, *rri2i3;

    Word16 *p0, *p1, *p2, *p3;

    Word16 *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
    Word32 cor;
    Word16 i, k, ldec, l_fin_sup, l_fin_inf;
    Word16 h[SubFrLen2];

    /* Scaling for maximum precision */

    cor = 0;
    for(i=0; i<SubFrLen; i++)
        cor = L_mac(cor, H[i], H[i]);

    if(extract_h(cor) > 32000 ) {
        for(i=0; i<SubFrLen; i++) h[i+4] = shr(H[i], 1);
    }
    else {
        k = norm_l(cor);
        k = shr(k, 1);
        for(i=0; i<SubFrLen; i++) h[i+4] = shl(H[i], k);
    }

    for(i=0; i<4; i++) h[i] = 0;

    /* Init pointers */

    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;

    rri0i1 = rri3i3 + NB_POS;
    rri0i2 = rri0i1 + MSIZE;
    rri0i3 = rri0i2 + MSIZE;
    rri1i2 = rri0i3 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri2i3 = rri1i3 + MSIZE;

 /*
  * Compute rri0i0[], rri1i1[], rri2i2[] and rri3i3[]
  */

    p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
    p1 = rri1i1 + NB_POS-1;
    p2 = rri2i2 + NB_POS-1;
    p3 = rri3i3 + NB_POS-1;

    ptr_h1 = h;
    cor    = 0;
    for(i=0;  i<NB_POS; i++) {
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p3-- = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p2-- = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p1-- = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p0-- = extract_h(cor);
    }


 /*
  * Compute elements of: rri0i1[], rri0i3[], rri1i2[] and rri2i3[]
  */

    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    ldec = NB_POS+1;

    ptr_hd = h;
    ptr_hf = ptr_hd + 2;

    for(k=0; k<NB_POS; k++) {

        p3 = rri2i3 + l_fin_sup;
        p2 = rri1i2 + l_fin_sup;
        p1 = rri0i1 + l_fin_sup;
        p0 = rri0i3 + l_fin_inf;
        cor = 0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;

        for(i=k+(Word16)1; i<NB_POS; i++ ) {

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p1 = extract_h(cor);

        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }

 /*
  * Compute elements of: rri0i2[], rri1i3[]
  */

    ptr_hd = h;
    ptr_hf = ptr_hd + 4;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    for(k=0; k<NB_POS; k++) {
        p3 = rri1i3 + l_fin_sup;
        p2 = rri0i2 + l_fin_sup;
        p1 = rri1i3 + l_fin_inf;
        p0 = rri0i2 + l_fin_inf;

        cor = 0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);


        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }

 /*
  * Compute elements of: rri0i1[], rri0i3[], rri1i2[] and rri2i3[]
  */

    ptr_hd = h;
    ptr_hf = ptr_hd + 6;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Word16)1;
    for(k=0; k<NB_POS; k++) {

        p3 = rri0i3 + l_fin_sup;
        p2 = rri2i3 + l_fin_inf;
        p1 = rri1i2 + l_fin_inf;
        p0 = rri0i1 + l_fin_inf;

        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        cor = 0;
        for(i=k+(Word16)1; i<NB_POS; i++ ) {

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);

        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }

    return;
}

void Cor_h_X(Word16 h[], Word16 X[], Word16 D[])
{
    Word16 i, j;
    Word32 s, max;
    Word32 y32[SubFrLen];

    /* first keep the result on 32 bits and find absolute maximum */

    max = 0;

    for (i = 0; i < SubFrLen; i++) {
        s = 0;
        for (j = i; j <  SubFrLen; j++)
            s = L_mac(s, X[j], h[j-i]);

        y32[i] = s;

        s = L_abs(s);

        if(s > max) max = s;


    }

    /*
     * Find the number of right shifts to do on y32[]
     * so that maximum is on 13 bits
     */

    j = norm_l(max);
    if( sub(j,16) > 0) j = 16;

    j = sub(18, j);

    for(i=0; i<SubFrLen; i++)
        D[i] = extract_l( L_shr(y32[i], j) );

    return;
}

static Word16 extra;
void reset_max_time(void)
{
    extra = 120;
    return;
}

Word16 D4i64_LBC(Word16 Dn[], Word16 rr[], Word16 h[], Word16 cod[],
                 Word16 y[], Word16 *code_shift, Word16 *sign)
{
    Word16  i0, i1, i2, i3, ip0, ip1, ip2, ip3;
    Word16  i, j, time;
    Word16  shif, shift;
    Word16  ps0, ps1, ps2, ps3, alp, alp0;
    Word32  alp1, alp2, alp3, L32;
    Word16  ps0a, ps1a, ps2a;
    Word16  ps3c, psc, alpha;
    Word16  means, max0, max1, max2, thres;

    Word16  *rri0i0, *rri1i1, *rri2i2, *rri3i3;
    Word16  *rri0i1, *rri0i2, *rri0i3;
    Word16  *rri1i2, *rri1i3, *rri2i3;

    Word16  *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3;
    Word16  *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3;
    Word16  *ptr_ri1i2, *ptr_ri1i3, *ptr_ri2i3;

    Word16  *ptr1_ri0i1, *ptr1_ri0i2, *ptr1_ri0i3;
    Word16  *ptr1_ri1i2, *ptr1_ri1i3, *ptr1_ri2i3;

    Word16  p_sign[SubFrLen2/2];

    /* Init pointers */

    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;

    rri0i1 = rri3i3 + NB_POS;
    rri0i2 = rri0i1 + MSIZE;
    rri0i3 = rri0i2 + MSIZE;
    rri1i2 = rri0i3 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri2i3 = rri1i3 + MSIZE;

 /*
  * Extend the backward filtered target vector by zeros
  */

    for (i = SubFrLen; i < SubFrLen2; i++) Dn[i] = 0;

 /*
  * Chose the sign of the impulse.
  */

    for (i=0; i<SubFrLen; i+=2) {
        if( add(Dn[i],Dn[i+1]) >= 0) {
            p_sign[i/2] = 1;
        }
        else {
            p_sign[i/2] = -1;
            Dn[i] = -Dn[i];
            Dn[i+1] = -Dn[i+1];
        }
    }
    p_sign[30] = p_sign[31] = 1;

 /*
  *   Compute the search threshold after three pulses
  */

    /* odd positions */
    /* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

    max0 = Dn[0];
    max1 = Dn[2];
    max2 = Dn[4];
    for (i = 8; i < SubFrLen; i+=STEP) {
#ifdef VC
        if (Dn[i]   > max0) max0 = Dn[i];
        if (Dn[i+2] > max1) max1 = Dn[i+2];
        if (Dn[i+4] > max2) max2 = Dn[i+4];
#endif
#ifdef CCS
		max0=_max2(Dn[i],max0);
		max1=_max2(Dn[i+2],max1);
		max2=_max2(Dn[i+4],max2);
#endif
    }
    max0 = add(max0, max1);
    max0 = add(max0, max2);

    /* Find means of Dn[i0]+Dn[i1]+Dn[i2] */

    L32 = 0;
    for (i = 0; i < SubFrLen; i+=STEP) {
        L32 = L_mac(L32, Dn[i], 1);
        L32 = L_mac(L32, Dn[i+2], 1);
        L32 = L_mac(L32, Dn[i+4], 1);
    }
    means =extract_l( L_shr(L32, 4));

    /* thres = means + (max0-means)*threshold; */

    thres = sub(max0, means);
    thres = mult(thres, threshold);
    thres = add(thres, means);

    /* even positions */
    /* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

    max0 = Dn[1];
    max1 = Dn[3];
    max2 = Dn[5];
    for (i = 9; i < SubFrLen; i+=STEP) {
#ifdef VC
        if (Dn[i]   > max0) max0 = Dn[i];
        if (Dn[i+2] > max1) max1 = Dn[i+2];
        if (Dn[i+4] > max2) max2 = Dn[i+4];
#endif
#ifdef CCS
		max0=_max2(Dn[i],max0);
		max1=_max2(Dn[i+2],max1);
		max2=_max2(Dn[i+4],max2);
#endif
    }
    max0 = add(max0, max1);
    max0 = add(max0, max2);

    /* Find means of Dn[i0]+Dn[i1]+Dn[i2] */

    L32 = 0;
    for (i = 1; i < SubFrLen; i+=STEP) {
        L32 = L_mac(L32, Dn[i], 1);
        L32 = L_mac(L32, Dn[i+2], 1);
        L32 = L_mac(L32, Dn[i+4], 1);
    }
    means =extract_l( L_shr(L32, 4));


    /* max1 = means + (max0-means)*threshold */

    max1 = sub(max0, means);
    max1 = mult(max1, threshold);
    max1 = add(max1, means);

    /* Keep maximum threshold between odd and even position */
#ifdef VC
    if(max1 > thres) thres = max1;
#endif

#ifdef CCS
	thres=_max2(max1,thres);
#endif

 /*
  * Modification of rrixiy[] to take signs into account.
  */

    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr1_ri0i1 = rri0i1;
    ptr1_ri0i2 = rri0i2;
    ptr1_ri0i3 = rri0i3;

    for(i0=0; i0<SubFrLen/2; i0+=STEP/2) {
        for(i1=2/2; i1<SubFrLen/2; i1+=STEP/2) {
            *ptr_ri0i1++ = i_mult(*ptr1_ri0i1++,
                                    i_mult(p_sign[i0], p_sign[i1]));
            *ptr_ri0i2++ = i_mult(*ptr1_ri0i2++,
                                    i_mult(p_sign[i0], p_sign[i1+1]));
            *ptr_ri0i3++ = i_mult(*ptr1_ri0i3++,
                                    i_mult(p_sign[i0], p_sign[i1+2]));
        }
    }

    ptr_ri1i2 = rri1i2;
    ptr_ri1i3 = rri1i3;
    ptr1_ri1i2 = rri1i2;
    ptr1_ri1i3 = rri1i3;
    for(i1=2/2; i1<SubFrLen/2; i1+=STEP/2) {
        for(i2=4/2; i2<SubFrLen2/2; i2+=STEP/2) {
            *ptr_ri1i2++ = i_mult(*ptr1_ri1i2++,
                                    i_mult(p_sign[i1], p_sign[i2]));
            *ptr_ri1i3++ = i_mult(*ptr1_ri1i3++,
                                    i_mult(p_sign[i1], p_sign[i2+1]));

        }
    }

    ptr_ri2i3 = rri2i3;

    ptr1_ri2i3 = rri2i3;
    for(i2=4/2; i2<SubFrLen2/2; i2+=STEP/2) {
        for(i3=6/2; i3<SubFrLen2/2; i3+=STEP/2)
        *ptr_ri2i3++ = i_mult(*ptr1_ri2i3++, i_mult(p_sign[i2], p_sign[i3]));
    }

 /*
  * Search the optimum positions of the four  pulses which maximize
  *     square(correlation) / energy
  * The search is performed in four  nested loops. At each loop, one
  * pulse contribution is added to the correlation and energy.
  *
  * The fourth loop is entered only if the correlation due to the
  *  contribution of the first three pulses exceeds the preset
  *  threshold.
  */

    /* Default values */
    ip0    = 0;
    ip1    = 2;
    ip2    = 4;
    ip3    = 6;
    shif   = 0;
    psc    = 0;
    alpha  = 32767;
    time   = add(max_time, extra);



    /* Four loops to search innovation code. */

    /* Init. pointers that depend on first loop */
    ptr_ri0i0 = rri0i0;
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;

    /* first pulse loop  */
    for (i0 = 0; i0 < SubFrLen; i0 += STEP) {

        ps0  = Dn[i0];
        ps0a = Dn[i0+1];
        alp0 = *ptr_ri0i0++;

        /* Init. pointers that depend on second loop */
        ptr_ri1i1 = rri1i1;
        ptr_ri1i2 = rri1i2;
        ptr_ri1i3 = rri1i3;

        /* second pulse loop */
        for (i1 = 2; i1 < SubFrLen; i1 += STEP) {

            ps1  = add(ps0, Dn[i1]);
            ps1a = add(ps0a, Dn[i1+1]);

            /* alp1 = alp0 + *ptr_ri1i1++ + 2.0 * ( *ptr_ri0i1++); */

            alp1 = L_mult(alp0, 1);
            alp1 = L_mac(alp1, *ptr_ri1i1++, 1);
            alp1 = L_mac(alp1, *ptr_ri0i1++, 2);

            /* Init. pointers that depend on third loop */
            ptr_ri2i2 = rri2i2;
            ptr_ri2i3 = rri2i3;

            /* third pulse loop */
            for (i2 = 4; i2 < SubFrLen2; i2 += STEP) {

                ps2  = add(ps1, Dn[i2]);
                ps2a = add(ps1a, Dn[i2+1]);

                /* alp2 = alp1 + *ptr_ri2i2++
                               + 2.0 * (*ptr_ri0i2++ + *ptr_ri1i2++); */

                alp2 = L_mac(alp1, *ptr_ri2i2++, 1);
                alp2 = L_mac(alp2, *ptr_ri0i2++, 2);
                alp2 = L_mac(alp2, *ptr_ri1i2++, 2);

                /* Decide the shift */

                shift = 0;
                if(ps2a > ps2) {
                    shift = 1;
                    ps2   = ps2a;
                }

                /* Test threshold */

                if ( ps2 > thres) {

                    /* Init. pointers that depend on 4th loop */
                    ptr_ri3i3 = rri3i3;

                    /* 4th pulse loop */
                    for (i3 = 6; i3 < SubFrLen2; i3 += STEP) {

                        ps3 = add(ps2, Dn[i3+shift]);

                        /* alp3 = alp2 + (*ptr_ri3i3++) +
                                         2 x ( (*ptr_ri0i3++) +
                                               (*ptr_ri1i3++) +
                                               (*ptr_ri2i3++) ) */

                        alp3 = L_mac(alp2, *ptr_ri3i3++, 1);
                        alp3 = L_mac(alp3, *ptr_ri0i3++, 2);
                        alp3 = L_mac(alp3, *ptr_ri1i3++, 2);
                        alp3 = L_mac(alp3, *ptr_ri2i3++, 2);
                        alp  = extract_l(L_shr(alp3, 5));

                        ps3c = mult(ps3, ps3);
                        if( L_mult(ps3c, alpha) > L_mult(psc, alp) ) {
                            psc = ps3c;
                            alpha = alp;
                            ip0 = i0;
                            ip1 = i1;
                            ip2 = i2;
                            ip3 = i3;
                            shif = shift;
                        }
                    }  /*  end of for i3 = */

                    time --;
                    if(time <= 0 ) goto end_search;   /* Max time finish */
                    ptr_ri0i3 -= NB_POS;
                    ptr_ri1i3 -= NB_POS;

                }  /* end of if >thres */

                else {
                    ptr_ri2i3 += NB_POS;
                }

            } /* end of for i2 = */

            ptr_ri0i2 -= NB_POS;
            ptr_ri1i3 += NB_POS;

        } /* end of for i1 = */

        ptr_ri0i2 += NB_POS;
        ptr_ri0i3 += NB_POS;

    } /* end of for i0 = */

end_search:

    extra = time;

    /* Set the sign of impulses */

    i0 = p_sign[shr(ip0, 1)];
    i1 = p_sign[shr(ip1, 1)];
    i2 = p_sign[shr(ip2, 1)];
    i3 = p_sign[shr(ip3, 1)];

    /* Find the codeword corresponding to the selected positions */

    for(i=0; i<SubFrLen; i++) cod[i] = 0;

    if(shif > 0) {
        ip0 = add(ip0 ,1);
        ip1 = add(ip1 ,1);
        ip2 = add(ip2 ,1);
        ip3 = add(ip3 ,1);
    }

    cod[ip0] =  i0;
    cod[ip1] =  i1;
    if(ip2<SubFrLen) cod[ip2] = i2;
    if(ip3<SubFrLen) cod[ip3] = i3;

    /* find the filtered codeword */

    for (i = 0; i < SubFrLen; i++) y[i] = 0;

    if(i0 > 0)
        for(i=ip0, j=0; i<SubFrLen; i++, j++)
            y[i] = add(y[i], h[j]);
    else
        for(i=ip0, j=0; i<SubFrLen; i++, j++)
            y[i] = sub(y[i], h[j]);

    if(i1 > 0)
        for(i=ip1, j=0; i<SubFrLen; i++, j++)
            y[i] = add(y[i], h[j]);
    else
        for(i=ip1, j=0; i<SubFrLen; i++, j++)
            y[i] = sub(y[i], h[j]);

    if(ip2 < SubFrLen) {

        if(i2 > 0)
            for(i=ip2, j=0; i<SubFrLen; i++, j++)
                y[i] = add(y[i], h[j]);
        else
            for(i=ip2, j=0; i<SubFrLen; i++, j++)
                y[i] = sub(y[i], h[j]);
    }

    if(ip3 < SubFrLen) {

        if(i3 > 0)
            for(i=ip3, j=0; i<SubFrLen; i++, j++)
                y[i] = add(y[i], h[j]);
        else
            for(i=ip3, j=0; i<SubFrLen; i++, j++)
                y[i] = sub(y[i], h[j]);
    }

    /* find codebook index;  17-bit address */

    *code_shift = shif;

    *sign = 0;
    if(i0 > 0) *sign = add(*sign, 1);
    if(i1 > 0) *sign = add(*sign, 2);
    if(i2 > 0) *sign = add(*sign, 4);
    if(i3 > 0) *sign = add(*sign, 8);

    i = shr(ip0, 3);
    i = add(i, shl(shr(ip1, 3), 3));
    i = add(i, shl(shr(ip2, 3), 6));
    i = add(i, shl(shr(ip3, 3), 9));

    return i;
}

Word16 G_code_g723(Word16 X[], Word16 Y[], Word16 *gain_q)
{
    Word16 i;
    Word16 xy, yy, exp_xy, exp_yy, gain, gain_nq;
    Word32 L_xy, L_yy;
    Word16 dist, dist_min;


    /* Scale down Y[] by 8 to avoid overflow */
    for(i=0; i<SubFrLen; i++)
        Y[i] = shr(Y[i], 3);

    /* Compute scalar product <X[],Y[]> */
    L_xy = 0L;
    for(i=0; i<SubFrLen; i++)
        L_xy = L_mac(L_xy, X[i], Y[i]);

    exp_xy = norm_l(L_xy);
    xy = extract_h( L_shl(L_xy, exp_xy) );

    if(xy <= 0) {
        gain = 0;
        *gain_q =FcbkGainTable[gain];
        return(gain);
    }

    /* Compute scalar product <Y[],Y[]> */
    L_yy = 0L;
    for(i=0; i<SubFrLen; i++)
        L_yy = L_mac(L_yy, Y[i], Y[i]);

    exp_yy = norm_l(L_yy);
    yy     = extract_h( L_shl(L_yy, exp_yy) );

    /* compute gain = xy/yy */
    xy = shr(xy, 1);             /* Be sure xy < yy */
    gain_nq = div_s( xy, yy);

    i = add(exp_xy, 5);          /* Denormalization of division */
    i = sub(i, exp_yy);

    gain_nq = shr(gain_nq, i);

    gain = (Word16) 0;
    dist_min = sub(gain_nq, FcbkGainTable[0]);
    dist_min = abs_s(dist_min);
    for ( i =  1; i <NumOfGainLev ; i ++ ) {
        dist = sub(gain_nq, FcbkGainTable[i]);
        dist =abs_s(dist);
        if ( dist< dist_min) {
            dist_min = dist;
            gain = (Word16) i ;
        }
    }
    *gain_q = FcbkGainTable[gain];

    return(gain);
}


void  Fcbk_Unpk( Word16 *Tv, SFSDEF Sfs, Word16 Olp, Word16 Sfc,DEC_HANDLE *handle)
{
    int   i,j   ;

    Word32   Acc0  ;
    Word16   Np ;
    Word16 Tv_tmp[SubFrLen+4];
    Word16 acelp_gain, acelp_sign, acelp_shift, acelp_pos;
    Word16 offset, ipos, T0_acelp, gain_T0;



    switch(handle->mode)  {
        case G723_63: {

            Np = Nb_puls[(int)Sfc] ;

            for ( i = 0 ; i < SubFrLen ; i ++ )
                Tv[i] = (Word16) 0 ;

            if ( Sfs.Ppos >= MaxPosTable[Sfc] )
                return ;

            /* Decode the amplitudes and positions */
            j = MaxPulseNum - (int) Np ;

            Acc0 = Sfs.Ppos ;

            for ( i = 0 ; i < SubFrLen/Sgrid ; i ++ )  {

                Acc0 = L_sub( Acc0, CombinatorialTable[j][i] ) ;

                if ( Acc0 < (Word32) 0 ) {
                    Acc0 = L_add( Acc0, CombinatorialTable[j][i] ) ;
                    j ++ ;
                    if ( (Sfs.Pamp & (1 << (MaxPulseNum-j) )) != (Word16) 0 )
                        Tv[(int)Sfs.Grid + Sgrid*i] = -FcbkGainTable[Sfs.Mamp] ;
                    else
                        Tv[(int)Sfs.Grid + Sgrid*i] =  FcbkGainTable[Sfs.Mamp] ;

                    if ( j == MaxPulseNum )
                        break ;
                }
            }

            if ( Sfs.Tran == (Word16) 1 )
                Gen_Trn( Tv, Tv, Olp ) ;
            break;
        }

        case G723_53: {
            for ( i = 0 ; i < SubFrLen+4 ; i ++ )
                Tv_tmp[i] = (Word16) 0 ;

            /* decoding gain */
            acelp_gain = FcbkGainTable[Sfs.Mamp];
            /* decoding grid */
            acelp_shift = Sfs.Grid;
            /* decoding Sign */
            acelp_sign = Sfs.Pamp;
            /* decoding Pos */
            acelp_pos = (short) Sfs.Ppos;

            offset  = 0;
            for(i=0; i<4; i++) {
                ipos = (acelp_pos & (Word16)0x0007) ;
                ipos = shl(ipos,3) + acelp_shift + offset;
                if( (acelp_sign & 1 )== 1) {
                    Tv_tmp[ipos] = acelp_gain;
                }
                else {
                    Tv_tmp[ipos] = -acelp_gain;
                }
                offset = add(offset,2);
                acelp_pos = shr(acelp_pos, 3);
                acelp_sign = shr(acelp_sign,1);
            }
            for (i = 0; i < SubFrLen; i++) Tv[i] = Tv_tmp[i];
            T0_acelp = search_T0( (Word16) (Olp-1+Sfs.AcLg), Sfs.AcGn,
                                                            &gain_T0);
            if(T0_acelp <SubFrLen-2) {
                /* code[i] += 0.8 * code[i-Olp] */
                for (i = T0_acelp ; i < SubFrLen; i++)
                    Tv[i] = add(Tv[i], mult(Tv[i-T0_acelp ], gain_T0));
            }

            break;
        }
    }
    return;
}

Word16   Comp_Info( Word16 *Buff, Word16 Olp, Word16 *Gain, Word16 *ShGain)
{
    int   i,j   ;

    Word32   Acc0,Acc1 ;

    Word16   Tenr ;
    Word16   Ccr,Enr ;
    Word16   Indx ;

    /* Normalize the excitation */
    *ShGain = Vec_Norm( Buff, (Word16) (PitchMax+Frame) ) ;
#ifdef VC
    if ( Olp > (Word16) (PitchMax-3) )
        Olp = (Word16) (PitchMax-3) ;
#endif

#ifdef CCS
	Olp=_min2(Olp,(Word16) (PitchMax-3));
#endif

    Indx = Olp ;

    Acc1 = (Word32) 0 ;

    for ( i = (int)Olp-3 ; i <= (int)Olp+3 ; i ++ ) {

        Acc0 = (Word32) 0 ;
        for ( j = 0 ; j < 2*SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Buff[PitchMax+Frame-2*SubFrLen+j],
                                    Buff[PitchMax+Frame-2*SubFrLen-i+j] ) ;

        if ( Acc0 > Acc1 ) {
            Acc1 = Acc0 ;
            Indx = (Word16) i ;
        }
    }

    /* Compute target energy */
    Acc0 = (Word32) 0 ;
    for ( j = 0 ; j < 2*SubFrLen ; j ++ )
        Acc0 = L_mac( Acc0, Buff[PitchMax+Frame-2*SubFrLen+j],
                                    Buff[PitchMax+Frame-2*SubFrLen+j] ) ;
    Tenr = round( Acc0 ) ;
    *Gain = Tenr;

    /* Compute best energy */
    Acc0 = (Word32) 0 ;
    for ( j = 0 ; j < 2*SubFrLen ; j ++ )
        Acc0 = L_mac( Acc0, Buff[PitchMax+Frame-2*SubFrLen-(int)Indx+j],
                            Buff[PitchMax+Frame-2*SubFrLen-(int)Indx+j] ) ;

    Ccr = round( Acc1 ) ;

    if ( Ccr <= (Word16) 0 )
        return (Word16) 0 ;

    Enr = round( Acc0 ) ;

    Acc0 = L_mult( Enr, Tenr ) ;
    Acc0 = L_shr( Acc0, (Word16) 3 ) ;

    Acc0 = L_msu( Acc0, Ccr, Ccr ) ;

    if ( Acc0 < (Word32) 0 )
        return Indx ;
    else
        return (Word16) 0 ;
}


PFDEF Comp_Lpf( Word16 *Buff, Word16 Olp, Word16 Sfc ,DEC_HANDLE *handle)
{
    int   i,j   ;

    PFDEF Pf    ;
    Word32   Lcr[5] ;
    Word16   Scr[5] ;
    Word16   Bindx, Findx ;
    Word16   Exp ;

    Word32   Acc0,Acc1 ;

    /* Initialize */
    Pf.Indx = (Word16) 0 ;
    Pf.Gain = (Word16) 0 ;
    Pf.ScGn = (Word16) 0x7fff ;

    /* Find both indices */
    Bindx = Find_B( Buff, Olp, Sfc ) ;
    Findx = Find_F( Buff, Olp, Sfc ) ;

    /* Combine the results */
    if ( (Bindx == (Word16) 0) && (Findx == (Word16) 0) )
        return Pf ;

    /* Compute target energy */
    Acc0 = (Word32) 0 ;
    for ( j = 0 ; j < SubFrLen ; j ++ )
        Acc0 = L_mac( Acc0, Buff[PitchMax+(int)Sfc*SubFrLen+j],
                                    Buff[PitchMax+(int)Sfc*SubFrLen+j] ) ;
    Lcr[0] = Acc0 ;

    if ( Bindx != (Word16) 0 ) {
        Acc0 = (Word32) 0 ;
        Acc1 = (Word32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ ) {
            Acc0 = L_mac( Acc0, Buff[PitchMax+(int)Sfc*SubFrLen+j],
                        Buff[PitchMax+(int)Sfc*SubFrLen+(int)Bindx+j] ) ;
            Acc1 = L_mac( Acc1, Buff[PitchMax+(int)Sfc*SubFrLen+(int)Bindx+j],
                        Buff[PitchMax+(int)Sfc*SubFrLen+(int)Bindx+j] ) ;
        }
        Lcr[1] = Acc0 ;
        Lcr[2] = Acc1 ;
    }
    else {
        Lcr[1] = (Word32) 0 ;
        Lcr[2] = (Word32) 0 ;
    }

    if ( Findx != (Word16) 0 ) {
        Acc0 = (Word32) 0 ;
        Acc1 = (Word32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ ) {
            Acc0 = L_mac( Acc0, Buff[PitchMax+(int)Sfc*SubFrLen+j],
                        Buff[PitchMax+(int)Sfc*SubFrLen+(int)Findx+j] ) ;
            Acc1 = L_mac( Acc1, Buff[PitchMax+(int)Sfc*SubFrLen+(int)Findx+j],
                        Buff[PitchMax+(int)Sfc*SubFrLen+(int)Findx+j] ) ;
        }
        Lcr[3] = Acc0 ;
        Lcr[4] = Acc1 ;
    }
    else {
        Lcr[3] = (Word32) 0 ;
        Lcr[4] = (Word32) 0 ;
    }

    /* Normalize and convert to shorts */
    Acc1 = 0L ;
    for ( i = 0 ; i < 5 ; i ++ ) {
        Acc0 = Lcr[i] ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    Exp = norm_l( Acc1 ) ;
    for ( i = 0 ; i < 5 ; i ++ ) {
        Acc0 = L_shl( Lcr[i], Exp ) ;
        Scr[i] = extract_h( Acc0 ) ;
    }

    /* Select the best pair */
    if ( (Bindx != (Word16) 0) && ( Findx == (Word16) 0) )
        Pf = Get_Ind( Bindx, Scr[0], Scr[1], Scr[2],handle ) ;

    if ( (Bindx == (Word16) 0) && ( Findx != (Word16) 0) )
        Pf = Get_Ind( Findx, Scr[0], Scr[3], Scr[4],handle ) ;

    if ( (Bindx != (Word16) 0) && ( Findx != (Word16) 0) ) {
        Exp = mult_r( Scr[1], Scr[1] ) ;
        Acc0 = L_mult( Exp, Scr[4] ) ;
        Exp = mult_r( Scr[3], Scr[3] ) ;
        Acc1 = L_mult( Exp, Scr[2] ) ;
        if ( Acc0 > Acc1 )
            Pf = Get_Ind( Bindx, Scr[0], Scr[1], Scr[2] ,handle) ;
        else
            Pf = Get_Ind( Findx, Scr[0], Scr[3], Scr[4] ,handle) ;
    }

    return Pf ;
}


Word16   Find_B( Word16 *Buff, Word16 Olp, Word16 Sfc )
{
    int   i,j   ;

    Word16   Indx = 0 ;

    Word32   Acc0,Acc1 ;


#ifdef VC
    if ( Olp > (Word16) (PitchMax-3) )
        Olp = (Word16) (PitchMax-3) ;
#endif

#ifdef CCS
	Olp=_min2(Olp,(Word16) (PitchMax-3));
#endif

    Acc1 = (Word32) 0 ;

    for ( i = (int)Olp-3 ; i <= (int)Olp+3 ; i ++ ) {

        Acc0 = (Word32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Buff[PitchMax+(int)Sfc*SubFrLen+j],
                                    Buff[PitchMax+(int)Sfc*SubFrLen-i+j] ) ;
        if ( Acc0 > Acc1 ) {
            Acc1 = Acc0 ;
            Indx = -(Word16) i ;
        }
    }
    return Indx ;
}

/*
**
** Function:        Find_F()
**
** Description:     Computes best pitch postfilter forward lag by
**                  forward cross correlation maximization around the
**                  decoded pitch lag
**                  of the subframe 0 (for subframes 0 & 1)
**                  of the subframe 2 (for subframes 2 & 3)
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Word16 *Buff    decoded excitation
**  Word16 Olp      Decoded pitch lag
**  Word16 Sfc      Subframe index
**
** Outputs:     None
**
** Return value:
**
**  Word16    Pitch postfilter forward lag
*/
Word16   Find_F( Word16 *Buff, Word16 Olp, Word16 Sfc )
{
    int   i,j   ;

    Word16   Indx = 0 ;

    Word32   Acc0,Acc1 ;

#ifdef VC
    if ( Olp > (Word16) (PitchMax-3) )
        Olp = (Word16) (PitchMax-3) ;
#endif

#ifdef CCS
	Olp=_min2(Olp,(Word16) (PitchMax-3));
#endif

    Acc1 = (Word32) 0 ;

    for ( i = Olp-3 ; i <= Olp+3 ; i ++ ) {

        Acc0 = (Word32) 0 ;
        if ( ((int)Sfc*SubFrLen+SubFrLen+i) <= Frame ) {
            for ( j = 0 ; j < SubFrLen ; j ++ )
                Acc0 = L_mac( Acc0, Buff[PitchMax+(int)Sfc*SubFrLen+j],
                            Buff[PitchMax+(int)Sfc*SubFrLen+i+j] ) ;
        }


        if ( Acc0 > Acc1 ) {
            Acc1 = Acc0 ;
            Indx = (Word16) i ;
        }
    }

    return Indx ;
}

PFDEF Get_Ind( Word16 Ind, Word16 Ten, Word16 Ccr, Word16 Enr,DEC_HANDLE *handle )
{
    Word32   Acc0,Acc1 ;
    Word16   Exp   ;

    PFDEF Pf ;


    Pf.Indx = Ind ;

    /* Check valid gain */
    Acc0 = L_mult( Ten, Enr ) ;
    Acc0 = L_shr( Acc0, (Word16) 2 ) ;
    Acc1 = L_mult( Ccr, Ccr ) ;

    if ( Acc1 > Acc0 ) {

        if ( Ccr >= Enr )
            Pf.Gain = LpfConstTable[(int)handle->mode] ;
        else {
            Pf.Gain = div_s( Ccr, Enr ) ;
            Pf.Gain = mult( Pf.Gain, LpfConstTable[(int)handle->mode] ) ;
        }
        /* Compute scaling gain */
        Acc0 = L_deposit_h( Ten ) ;
        Acc0 = L_shr( Acc0, (Word16) 1 ) ;
        Acc0 = L_mac( Acc0, Ccr, Pf.Gain ) ;
        Exp  = mult( Pf.Gain, Pf.Gain ) ;
        Acc1 = L_mult( Enr, Exp ) ;
        Acc1 = L_shr( Acc1, (Word16) 1 ) ;
        Acc0 = L_add( Acc0, Acc1 ) ;
        Exp = round( Acc0 ) ;

        Acc1 = L_deposit_h( Ten ) ;
        Acc0 = L_deposit_h( Exp ) ;
        Acc1 = L_shr( Acc1, (Word16) 1 ) ;

        if ( Acc1 >= Acc0 )
            Exp = (Word16) 0x7fff ;
        else
            Exp = div_l( Acc1, Exp ) ;

        Acc0 = L_deposit_h( Exp ) ;
        Pf.ScGn = Sqrt_lbc( Acc0 ) ;
    }
    else {
        Pf.Gain = (Word16) 0 ;
        Pf.ScGn = (Word16) 0x7fff ;
    }

    Pf.Gain = mult( Pf.Gain, Pf.ScGn ) ;

    return Pf ;
}

void  Filt_Lpf( Word16 *Tv, Word16 *Buff, PFDEF Pf, Word16 Sfc )
{
    int   i  ;

    Word32   Acc0 ;

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_mult( Buff[PitchMax+(int)Sfc*SubFrLen+i], Pf.ScGn ) ;
        Acc0 = L_mac( Acc0, Buff[PitchMax+(int)Sfc*SubFrLen+(int)Pf.Indx+i],
                                                                Pf.Gain ) ;
        Tv[(int)Sfc*SubFrLen+i] = round( Acc0 ) ;
    }

    return;
}


void     Regen( Word16 *DataBuff, Word16 *Buff, Word16 Lag, Word16 Gain,
Word16 Ecount, Word16 *Sd )
{
    int   i  ;

    /* Test for clearing */
    if ( Ecount >= (Word16) ErrMaxNum ) {

#ifdef VC
        for ( i = 0 ; i < Frame ; i ++ )
            DataBuff[i] = (Word16) 0 ;
        for ( i = 0 ; i < Frame+PitchMax ; i ++ )
            Buff[i] = (Word16) 0 ;
#endif

#ifdef CCS
		for ( i = 0 ; i < Frame ; i=i+4 )
			_memd8( & DataBuff[i])=0;
        for ( i = 0 ; i < Frame+PitchMax ; i=i+4 )
            _memd8( & Buff[i])=0;		

#endif
    }
    else {
        /* Interpolate accordingly to the voicing estimation */
        if ( Lag != (Word16) 0 ) {
            /* Voiced case */
            for ( i = 0 ; i < Frame ; i ++ )
                Buff[PitchMax+i] = Buff[PitchMax-(int)Lag+i] ;
            for ( i = 0 ; i < Frame ; i ++ )
                DataBuff[i] = Buff[PitchMax+i] = mult( Buff[PitchMax+i],
                            (Word16) 0x6000 ) ;
        }
        else {
            /* Unvoiced case */
            for ( i = 0 ; i < Frame ; i ++ )
                DataBuff[i] = mult( Gain, Rand_lbc( Sd ) ) ;
            /* Clear buffer to reset memory */
            for ( i = 0 ; i < Frame+PitchMax ; i ++ )
                Buff[i] = (Word16) 0 ;
        }
    }

    return;
}


