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



void  Comp_Lpc( Word16 *UnqLpc, ENC_HANDLE *handle, Word16 *DataBuff )
{
    int   i,j,k ;

    Word16   Dpnt[Frame+LpcFrame-SubFrLen] ;
    Word16   Vect[LpcFrame] ;
    Word16   Acf_sf[LpcOrderP1*SubFrames];
    Word16   ShAcf_sf[SubFrames];
    Word16   Exp   ;
    Word16   *curAcf;
    Word16   Pk2;
    Word32   Acc0,Acc1   ;
	Word16   *PrevDat=handle->CodStat.PrevDat;

    /*
     * Generate a buffer of 360 samples.  This consists of 120 samples
     * from the previous frame and 240 samples from the current frame.
     */
    for ( i = 0 ; i < LpcFrame-SubFrLen ; i ++ )
        Dpnt[i] = PrevDat[i] ;
    for ( i = 0 ; i < Frame ; i ++ )
        Dpnt[i+LpcFrame-SubFrLen] = DataBuff[i] ;


    /*
     * Repeat for all subframes
     */
    curAcf = Acf_sf;
    for ( k = 0 ; k < SubFrames ; k ++ ) 
	{

        /* Get block of 180 samples centered around current subframe */
        for ( i = 0 ; i < LpcFrame ; i ++ )
            Vect[i] = Dpnt[k*SubFrLen+i] ;

        /* Normalize */
        ShAcf_sf[k] = Vec_Norm( Vect, (Word16) LpcFrame ) ;

        /* Apply the Hamming window */
        for ( i = 0 ; i < LpcFrame ; i ++ )
            Vect[i] = mult_r(Vect[i], HammingWindowTable[i]) ;


        /*
        * Compute the autocorrelation coefficients
        */

        /* Compute the zeroth-order coefficient (energy) */
        Acc1 = (Word32) 0 ;
        for ( i = 0 ; i < LpcFrame ; i ++ ) 
		{
            Acc0 = L_mult( Vect[i], Vect[i] ) ;
            Acc0 = L_shr( Acc0, (Word16) 1 ) ;
            Acc1 = L_add( Acc1, Acc0 ) ;
        }

        /* Apply a white noise correction factor of (1025/1024) */
        Acc0 = L_shr( Acc1, (Word16) RidgeFact ) ;
        Acc1 = L_add( Acc1, Acc0 ) ;

        /* Normalize the energy */
        Exp = norm_l( Acc1 ) ;
        Acc1 = L_shl( Acc1, Exp ) ;

        curAcf[0] = round( Acc1 ) ;
        if(curAcf[0] == 0)
		{
            for ( i = 1 ; i <= LpcOrder ; i ++ )
                curAcf[i] = 0;
            ShAcf_sf[k] = 40;
        }

        else 
		{
            /* Compute the rest of the autocorrelation coefficients.
               Multiply them by a binomial coefficients lag window. */
            for ( i = 1 ; i <= LpcOrder ; i ++ )
			{
                Acc1 = (Word32) 0 ;
                for ( j = i ; j < LpcFrame ; j ++ ) 
				{
                    Acc0 = L_mult( Vect[j], Vect[j-i] ) ;
                    Acc0 = L_shr( Acc0, (Word16) 1 ) ;
                    Acc1 = L_add( Acc1, Acc0 ) ;
                }
                Acc0 = L_shl( Acc1, Exp ) ;
                Acc0 = L_mls( Acc0, BinomialWindowTable[i-1] ) ;
                curAcf[i] = round(Acc0) ;
            }
            /* Save Acf scaling factor */
            ShAcf_sf[k] = add(Exp, shl(ShAcf_sf[k], 1));
        }

        /*
         * Apply the Levinson-Durbin algorithm to generate the LPC
         * coefficients
        */
        Durbin( &UnqLpc[k*LpcOrder], &curAcf[1], curAcf[0], &Pk2 );
        handle->CodStat.SinDet <<= 1;
        if ( Pk2 > 0x799a ) {
            handle->CodStat.SinDet ++ ;
        }
        curAcf += LpcOrderP1;
    }

    /* Update sine detector */
    handle->CodStat.SinDet &= 0x7fff ;

    j = handle->CodStat.SinDet ;
    k = 0 ;
    for ( i = 0 ; i < 15 ; i ++ ) {
        k += j & 1 ;
        j >>= 1 ;
    }
    if ( k >= 14 )
        handle->CodStat.SinDet |= 0x8000 ;

    /* Update CNG Acf memories */
    Update_Acf(Acf_sf, ShAcf_sf,handle);

}

Word16  Durbin( Word16 *Lpc, Word16 *Corr, Word16 Err, Word16 *Pk2 )
{
    int   i,j   ;

    Word16   Temp[LpcOrder] ;
    Word16   Pk ;

    Word32   Acc0,Acc1,Acc2 ;

 /*
  * Initialize the LPC vector
  */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        Lpc[i] = (Word16) 0 ;

 /*
  * Do the recursion.  At the ith step, the algorithm computes the
  * (i+1)th - order MMSE linear prediction filter.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ ) {

/*
 * Compute the partial correlation (parcor) coefficient
 */

        /* Start parcor computation */
        Acc0 = L_deposit_h( Corr[i] ) ;
        Acc0 = L_shr( Acc0, (Word16) 2 ) ;
        for ( j = 0 ; j < i ; j ++ )
            Acc0 = L_msu( Acc0, Lpc[j], Corr[i-j-1] ) ;
        Acc0 = L_shl( Acc0, (Word16) 2 ) ;

        /* Save sign */
        Acc1 = Acc0 ;
        Acc0 = L_abs( Acc0 ) ;

        /* Finish parcor computation */
        Acc2 = L_deposit_h( Err ) ;
        if ( Acc0 >= Acc2 ) {
            *Pk2 = 32767;
            break ;
        }

        Pk = div_l( Acc0, Err ) ;

        if ( Acc1 >= 0 )
            Pk = negate(Pk) ;

 /*
  * Sine detector
  */
        if ( i == 1 ) *Pk2 = Pk;

 /*
  * Compute the ith LPC coefficient
  */
        Acc0 = L_deposit_h( negate(Pk) ) ;
        Acc0 = L_shr( Acc0, (Word16) 2 ) ;
        Lpc[i] = round( Acc0 ) ;

 /*
  * Update the prediction error
  */
        Acc1 = L_mls( Acc1, Pk ) ;
        Acc1 = L_add( Acc1, Acc2 ) ;
        Err = round( Acc1 ) ;

 /*
  * Compute the remaining LPC coefficients
  */
        for ( j = 0 ; j < i ; j ++ )
            Temp[j] = Lpc[j] ;

        for ( j = 0 ; j < i ; j ++ ) {
            Acc0 = L_deposit_h( Lpc[j] ) ;
            Acc0 = L_mac( Acc0, Pk, Temp[i-j-1] ) ;
            Lpc[j] = round( Acc0 ) ;
        }
    }

    return Err ;
}


void  Wght_Lpc( Word16 *PerLpc, Word16 *UnqLpc )
{
    int   i,j   ;


 /*
  * Do for all subframes
  */
    for ( i = 0 ; i < SubFrames ; i ++ ) {


 /*
  * Compute the jth FIR coefficient by multiplying the jth LPC
  * coefficient by (0.9)^j.
  */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            PerLpc[j] = mult_r( UnqLpc[j], PerFiltZeroTable[j] ) ;
        PerLpc += LpcOrder ;


/*
 * Compute the jth IIR coefficient by multiplying the jth LPC
 * coefficient by (0.5)^j.
 */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            PerLpc[j] = mult_r( UnqLpc[j], PerFiltPoleTable[j] ) ;
        PerLpc += LpcOrder ;
        UnqLpc += LpcOrder ;
    }
}

void  Error_Wght( Word16 *Dpnt, Word16 *PerLpc,ENC_HANDLE *handle)
{
    int   i,j,k ;

    Word32   Acc0  ;


/*
 * Do for all subframes
 */
    for ( k = 0 ; k < SubFrames ; k ++ ) {

        for ( i = 0 ; i < SubFrLen ; i ++ ) {

/*
 * Do the FIR part
 */
            /* Filter */
            Acc0 = L_mult( *Dpnt, (Word16) 0x2000 ) ;
            for ( j = 0 ; j < LpcOrder ; j ++ )
                Acc0 = L_msu( Acc0, PerLpc[j], handle->CodStat.WghtFirDl[j] ) ;

            /* Update memory */
            for ( j = LpcOrder-1 ; j > 0 ; j -- )
                handle->CodStat.WghtFirDl[j] = handle->CodStat.WghtFirDl[j-1] ;
            handle->CodStat.WghtFirDl[0] = *Dpnt ;

 /*
  * Do the IIR part
  */

            /* Filter */
            for ( j = 0 ; j < LpcOrder ; j ++ )
                Acc0 = L_mac( Acc0, PerLpc[LpcOrder+j],
                                                    handle->CodStat.WghtIirDl[j] ) ;
            for ( j = LpcOrder-1 ; j > 0 ; j -- )
                handle->CodStat.WghtIirDl[j] = handle->CodStat.WghtIirDl[j-1] ;
            Acc0 = L_shl( Acc0, (Word16) 2 ) ;

            /* Update memory */
            handle->CodStat.WghtIirDl[0] = round( Acc0 ) ;
            *Dpnt ++ = handle->CodStat.WghtIirDl[0] ;
        }
        PerLpc += 2*LpcOrder ;
    }
}

void  Upd_Ring( Word16 *Dpnt, Word16 *QntLpc, Word16 *PerLpc, Word16 *PrevErr,ENC_HANDLE *handle )
{
    int   i,j   ;

    Word32   Acc0,Acc1   ;


 /*
  * Shift the harmonic noise shaping filter memory
  */
    for ( i = SubFrLen ; i < PitchMax ; i ++ )
        PrevErr[i-SubFrLen] = PrevErr[i] ;


 /*
  * Do for all elements in the subframe
  */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

 /*
  * Input the current subframe's excitation
  */
        Acc0 = L_deposit_h( Dpnt[i] ) ;
        Acc0 = L_shr( Acc0, (Word16) 3 ) ;

 /*
  * Synthesis filter
  */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, QntLpc[j], handle->CodStat.RingFirDl[j] ) ;
        Acc1 = L_shl( Acc0, (Word16) 2 ) ;

        Dpnt[i] = shl( round( Acc1 ), (Word16) 1 ) ;

 /*
  * Perceptual weighting filter
  */

        /* FIR part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_msu( Acc0, PerLpc[j], handle->CodStat.RingFirDl[j] ) ;

        /* Update FIR memory */
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            handle->CodStat.RingFirDl[j] = handle->CodStat.RingFirDl[j-1] ;
        handle->CodStat.RingFirDl[0] = round( Acc1 ) ;

        /* IIR part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, PerLpc[LpcOrder+j], handle->CodStat.RingIirDl[j] ) ;
        Acc0 = L_shl( Acc0, (Word16) 2 ) ;

        /* Update IIR memory */
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            handle->CodStat.RingIirDl[j] = handle->CodStat.RingIirDl[j-1] ;
        handle->CodStat.RingIirDl[0] = round( Acc0 ) ;

        /* Update harmonic noise shaping memory */
        PrevErr[PitchMax-SubFrLen+i] = handle->CodStat.RingIirDl[0] ;
    }
}

void  Comp_Ir( Word16 *ImpResp, Word16 *QntLpc, Word16 *PerLpc, PWDEF Pw )
{
    int   i,j   ;

    Word16   FirDl[LpcOrder] ;
    Word16   IirDl[LpcOrder] ;
    Word16   Temp[PitchMax+SubFrLen] ;

    Word32   Acc0,Acc1 ;


 /*
  * Clear all memory.  Impulse response calculation requires
  * an all-zero initial state.
  */

    /* Perceptual weighting filter */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        FirDl[i] = IirDl[i] = (Word16) 0 ;

    /* Harmonic noise shaping filter */
    for ( i = 0 ; i < PitchMax+SubFrLen ; i ++ )
        Temp[i] = (Word16) 0 ;


 /*
  * Input a single impulse
  */
    Acc0 = (Word32) 0x04000000L ;

 /*
  * Do for all elements in a subframe
  */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

 /*
  * Synthesis filter
  */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, QntLpc[j], FirDl[j] ) ;
        Acc1 = L_shl( Acc0, (Word16) 2 ) ;

 /*
  * Perceptual weighting filter
  */

        /* FIR part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_msu( Acc0, PerLpc[j], FirDl[j] ) ;
                                Acc0 = L_shl( Acc0, (Word16) 1 ) ;
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            FirDl[j] = FirDl[j-1] ;
        FirDl[0] = round( Acc1 ) ;

        /* Iir part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, PerLpc[LpcOrder+j], IirDl[j] ) ;
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            IirDl[j] = IirDl[j-1] ;
        Acc0 = L_shl( Acc0, (Word16) 2 ) ;
        IirDl[0] = round( Acc0 ) ;
        Temp[PitchMax+i] = IirDl[0] ;

 /*
  * Harmonic noise shaping filter
  */

        Acc0 = L_deposit_h( IirDl[0] ) ;
        Acc0 = L_msu( Acc0, Pw.Gain, Temp[PitchMax-Pw.Indx+i] ) ;
        ImpResp[i] = round( Acc0 ) ;

        Acc0 = (Word32) 0 ;
    }
}

void  Sub_Ring( Word16 *Dpnt, Word16 *QntLpc, Word16 *PerLpc, ENC_HANDLE *handle, PWDEF Pw )
{
    int   i,j   ;
    Word32   Acc0,Acc1 ;

    Word16   FirDl[LpcOrder] ;
    Word16   IirDl[LpcOrder] ;
    Word16   Temp[PitchMax+SubFrLen] ;


 /*
  * Initialize the memory
  */
    for ( i = 0 ; i < PitchMax ; i ++ )
        Temp[i] = handle->CodStat.PrevErr[i] ;

    for ( i = 0 ; i < LpcOrder ; i ++ ) {
        FirDl[i] = handle->CodStat.RingFirDl[i] ;
        IirDl[i] = handle->CodStat.RingIirDl[i] ;
    }

 /*
  * Do for all elements in a subframe
  */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

 /*
  * Input zero
  */
        Acc0 = (Word32) 0 ;

 /*
  * Synthesis filter
  */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, QntLpc[j], FirDl[j] ) ;
        Acc1 = L_shl( Acc0, (Word16) 2 ) ;

 /*
  * Perceptual weighting filter
  */

        /* Fir part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_msu( Acc0, PerLpc[j], FirDl[j] ) ;
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            FirDl[j] = FirDl[j-1] ;
        FirDl[0] = round( Acc1 ) ;

        /* Iir part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, PerLpc[LpcOrder+j], IirDl[j] ) ;
        Acc0 = L_shl( Acc0, (Word16) 2 ) ;
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            IirDl[j] = IirDl[j-1] ;
        IirDl[0] = round( Acc0 ) ;
        Temp[PitchMax+i] = IirDl[0] ;

 /*
  * Do the harmonic noise shaping filter and subtract the result
  * from the harmonic noise weighted vector.
  */
        Acc0 = L_deposit_h( sub( Dpnt[i], IirDl[0] ) ) ;
        Acc0 = L_mac( Acc0, Pw.Gain, Temp[PitchMax-(int)Pw.Indx+i] ) ;
        Dpnt[i] = round ( Acc0 ) ;
    }
}


void     Synt( Word16 *Dpnt, Word16 *Lpc ,DEC_HANDLE *handle)
{
    int   i,j   ;

    Word32   Acc0  ;


 /*
  * Do for all elements in the subframe
  */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

 /*
  * Input the current subframe's excitation
  */
        Acc0 = L_deposit_h( Dpnt[i] ) ;
        Acc0 = L_shr( Acc0, (Word16) 3 ) ;

 /*
  * Synthesis
  */

        /* Filter */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, Lpc[j], handle->DecStat.SyntIirDl[j] ) ;

        /* Update memory */
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            handle->DecStat.SyntIirDl[j] = handle->DecStat.SyntIirDl[j-1] ;

        Acc0 = L_shl( Acc0, (Word16) 2 ) ;

        handle->DecStat.SyntIirDl[0] = round( Acc0 ) ;

 /*
  * Scale output if postfilter is off.  (Otherwise output is
  * scaled by the gain scaling unit.)
  */
        if ( handle->UsePf )
            Dpnt[i] = handle->DecStat.SyntIirDl[0] ;
        else
            Dpnt[i] = shl( handle->DecStat.SyntIirDl[0], (Word16) 1 ) ;

    }

}


Word32  Spf( Word16 *Tv, Word16 *Lpc ,DEC_HANDLE *handle)
{
    int   i,j   ;

    Word32   Acc0,Acc1   ;
    Word32   Sen ;
    Word16   Tmp ;
    Word16   Exp ;

    Word16   FirCoef[LpcOrder] ;
    Word16   IirCoef[LpcOrder] ;

    Word16   TmpVect[SubFrLen] ;

 /*
  * Compute ARMA coefficients.  Compute the jth FIR coefficient by
  * multiplying the jth quantized LPC coefficient by (0.65)^j.
  * Compute the jth IIR coefficient by multiplying the jth quantized
  * LPC coefficient by (0.75)^j.  This emphasizes the formants in
  * the frequency response.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ ) {
        FirCoef[i] = mult_r( Lpc[i], PostFiltZeroTable[i] ) ;
        IirCoef[i] = mult_r( Lpc[i], PostFiltPoleTable[i] ) ;
    }

 /*
  * Normalize the speech vector.
  */
    for ( i = 0 ; i < SubFrLen ; i ++ )
        TmpVect[i] = Tv[i] ;
    Exp = Vec_Norm( TmpVect, (Word16) SubFrLen ) ;

 /*
  * Compute the first two autocorrelation coefficients R[0] and R[1]
  */
    Acc0 = (Word32) 0 ;
    Acc1 = L_mult( TmpVect[0], TmpVect[0] ) ;
    for ( i = 1 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_mac( Acc0, TmpVect[i], TmpVect[i-1] ) ;
        Acc1 = L_mac( Acc1, TmpVect[i], TmpVect[i] ) ;
    }

 /*
  * Scale the energy for the later use.
  */
    Sen = L_shr( Acc1, (Word16)(2*Exp + 4) ) ;

 /*
  * Compute the first-order partial correlation coefficient of the
  * input speech vector.
  */
    Tmp = extract_h( Acc1 ) ;
    if ( Tmp != (Word16) 0 ) {

        /* Compute first parkor */
        Acc0 = L_shr( Acc0, (Word16) 1 ) ;
        Acc1 = Acc0 ;
        Acc0 = L_abs( Acc0 ) ;

        Tmp = div_l( Acc0, Tmp ) ;

        if ( Acc1 < (Word32) 0 )
            Tmp = negate( Tmp ) ;
    }
    else
        Tmp = (Word16) 0 ;

 /*
  * Compute the compensation filter parameter and update the memory
  */
    Acc0 = L_deposit_h( handle->DecStat.Park ) ;
    Acc0 = L_msu( Acc0, handle->DecStat.Park, (Word16) 0x2000 ) ;
    Acc0 = L_mac( Acc0, Tmp, (Word16) 0x2000 ) ;
    handle->DecStat.Park = round( Acc0 ) ;

    Tmp  = mult( handle->DecStat.Park, PreCoef ) ;
    Tmp &= (Word16) 0xfffc ;


 /*
  *  Do for all elements in the subframe
  */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

 /*
  * Input the speech vector
  */
        Acc0 = L_deposit_h( Tv[i] ) ;
        Acc0 = L_shr( Acc0, (Word16) 2 ) ;

 /*
  * Formant postfilter
  */

        /* FIR part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_msu( Acc0, FirCoef[j], handle->DecStat.PostFirDl[j] ) ;

        /* Update FIR memory */
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            handle->DecStat.PostFirDl[j] = handle->DecStat.PostFirDl[j-1] ;
        handle->DecStat.PostFirDl[0] = Tv[i] ;

        /* IIR part */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, IirCoef[j], handle->DecStat.PostIirDl[j] ) ;

        /* Update IIR memory */
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            handle->DecStat.PostIirDl[j] = handle->DecStat.PostIirDl[j-1] ;

        Acc0 = L_shl( Acc0, (Word16) 2 ) ;
        Acc1 = Acc0 ;

        handle->DecStat.PostIirDl[0] = round( Acc0 ) ;

 /*
  * Compensation filter
  */
        Acc1 = L_mac( Acc1, handle->DecStat.PostIirDl[1], Tmp ) ;

        Tv[i] = round( Acc1 ) ;
    }
    return Sen ;
}


