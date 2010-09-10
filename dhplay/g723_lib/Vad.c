
/*
**

** File:        "vad.c"
**
** Description:     Voice Activity Detection
**
** Functions:       Init_Vad()
**                  Vad()
**
**
*/
/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/
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



void  Init_Vad(ENC_HANDLE *handle)
{
    int i ;
    handle->VadStat.Hcnt = 3 ;
    handle->VadStat.Vcnt = 0 ;
    handle->VadStat.Penr = 0x00000400L ;
    handle->VadStat.Nlev = 0x00000400L ;

    handle->VadStat.Aen = 0 ;

    handle->VadStat.Polp[0] = 1 ;
    handle->VadStat.Polp[1] = 1 ;
    handle->VadStat.Polp[2] = SubFrLen ;
    handle->VadStat.Polp[3] = SubFrLen ;

    for(i=0; i < LpcOrder; i++) handle->VadStat.NLpc[i] = 0;

}


Flag Comp_Vad( Word16 *Dpnt,ENC_HANDLE *handle)
{
    int i,j ;

    Word32  Acc0,Acc1 ;
    Word16  Tm0, Tm1, Tm2 ;
    Word16  Minp ;

    Flag    VadState = 1 ;

    static  Word16  ScfTab[11] = {
         9170 ,
         9170 ,
         9170 ,
         9170 ,
        10289 ,
        11544 ,
        12953 ,
        14533 ,
        16306 ,
        18296 ,
        20529 ,
    } ;

    if ( !handle->UseVx )
        return VadState ;

    /* Find Minimum pitch period */
    Minp = PitchMax ;
    for ( i = 0 ; i < 4 ; i ++ ) 
	{
        if ( Minp > handle->VadStat.Polp[i] )
            Minp = handle->VadStat.Polp[i] ;
    }

    /* Check that all are multiplies of the minimum */
    Tm2 = 0 ;
    for ( i = 0 ; i < 4 ; i ++ )
	{
        Tm1 = Minp ;
        for ( j = 0 ; j < 8 ; j ++ ) 
		{
            Tm0 = sub( Tm1, handle->VadStat.Polp[i] ) ;
            Tm0 = abs_s( Tm0 ) ;
            if ( Tm0 <= 3 )
                Tm2 ++ ;
            Tm1 = add( Tm1, Minp ) ;
        }
    }

    /* Update adaptation enable counter if not periodic and not sine */
    if ( (Tm2 == 4) || (handle->CodStat.SinDet < 0) )
        handle->VadStat.Aen += 2 ;
    else
        handle->VadStat.Aen -- ;

    /* Clip it */
    if ( handle->VadStat.Aen > 6 )
        handle->VadStat.Aen = 6 ;
    if ( handle->VadStat.Aen < 0 )
        handle->VadStat.Aen = 0 ;

    /* Inverse filter the data */
    Acc1 = 0L ;
    for ( i = SubFrLen ; i < Frame ; i ++ ) 
	{

        Acc0 = L_mult( Dpnt[i], 0x2000 ) ;
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_msu( Acc0, Dpnt[i-j-1], handle->VadStat.NLpc[j] ) ;
        Tm0 = round ( Acc0 ) ;
        Acc1 = L_mac( Acc1, Tm0, Tm0 ) ;
    }

    /* Scale the rezidual energy */
    Acc1 = L_mls( Acc1, (Word16) 2913 ) ;

    /* Clip noise level in any case */
    if ( handle->VadStat.Nlev > handle->VadStat.Penr )
	{
        Acc0 = L_sub( handle->VadStat.Penr, L_shr( handle->VadStat.Penr, 2 ) ) ;
        handle->VadStat.Nlev = L_add( Acc0, L_shr( handle->VadStat.Nlev, 2 ) ) ;
    }


    /* Update the noise level, if adaptation is enabled */
    if ( !handle->VadStat.Aen ) 
	{
        handle->VadStat.Nlev = L_add( handle->VadStat.Nlev, L_shr( handle->VadStat.Nlev, 5 ) ) ;
    }
    /* Decay Nlev by small amount */
    else 
	{
        handle->VadStat.Nlev = L_sub( handle->VadStat.Nlev, L_shr( handle->VadStat.Nlev,11 ) ) ;
    }

    /* Update previous energy */
    handle->VadStat.Penr = Acc1 ;

    /* CLip Noise Level */
    if ( handle->VadStat.Nlev < 0x00000080L )
        handle->VadStat.Nlev = 0x00000080L ;
    if ( handle->VadStat.Nlev > 0x0001ffffL )
        handle->VadStat.Nlev = 0x0001ffffL ;

    /* Compute the treshold */
    Acc0 = L_shl( handle->VadStat.Nlev, 13 ) ;
    Tm0 = norm_l( Acc0 ) ;
    Acc0 = L_shl( Acc0, Tm0 ) ;
    Acc0 &= 0x3f000000L ;
    Acc0 <<= 1 ;
    Tm1 = extract_h( Acc0 ) ;
    Acc0 = L_deposit_h( ScfTab[Tm0] ) ;
    Acc0 = L_mac( Acc0, Tm1, ScfTab[Tm0-1] ) ;
    Acc0 = L_msu( Acc0, Tm1, ScfTab[Tm0] ) ;
    Tm1 = extract_h( Acc0 ) ;
    Tm0 = extract_l( L_shr( handle->VadStat.Nlev, 2 ) ) ;
    Acc0 = L_mult( Tm0, Tm1 ) ;
    Acc0 >>= 11 ;

    /* Compare with the treshold */
    if ( Acc0 > Acc1 )
        VadState = 0 ;

    /* Do the various counters */
    if ( VadState ) 
	{
        handle->VadStat.Vcnt ++ ;
        handle->VadStat.Hcnt ++ ;
    }
    else
	{
       handle->VadStat.Vcnt -- ;
        if ( handle->VadStat.Vcnt < 0 )
            handle->VadStat.Vcnt = 0 ;
    }

    if ( handle->VadStat.Vcnt >= 2 ) 
	{
        handle->VadStat.Hcnt = 6 ;
        if ( handle->VadStat.Vcnt >= 3 )
            handle->VadStat.Vcnt = 3 ;
    }

    if ( handle->VadStat.Hcnt )
	{
        VadState = 1 ;
        if ( handle->VadStat.Vcnt == 0 )
            handle->VadStat.Hcnt -- ;
    }

    /* Update Periodicy detector */
    handle->VadStat.Polp[0] = handle->VadStat.Polp[2] ;
    handle->VadStat.Polp[1] = handle->VadStat.Polp[3] ;

    return VadState ;
}










