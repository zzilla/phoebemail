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
#include "g723.h"
#include "inline.h"



int g723dec_init(void** handle)
{
	DEC_HANDLE* tmp;
	int i;
	tmp = (DEC_HANDLE*)malloc(sizeof(DEC_HANDLE));
	if (tmp == NULL)
		return -1;

	memset(tmp, 0, sizeof(DEC_HANDLE));

	/* Initialize the previously decoded LSP vector to the DC vector */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        tmp->DecStat.PrevLsp[i] = LspDcTable[i] ;

    /* Initialize the gain scaling unit memory to a constant */
    tmp->DecStat.Gain = (Word16) 0x1000 ;
	tmp->First_Init=1;

	Init_Dec_Cng(tmp);

	*handle = tmp;

	return 0;
}


int g723dec_close(void** handle)
{
	if(*handle==NULL)
		return -1;
	else
	{
		free (*handle);
		*handle=NULL;
	}
	return 0;
}


int g723dec(void* handle, char* bits,  short* out, int bits_len, int* out_len)
{
	int   i,j ;
    Word32   Senr ;
    Word16   QntLpc[SubFrames*LpcOrder] ;
    Word16   AcbkCont[SubFrLen] ;
    Word16   LspVect[LpcOrder] ;
    Word16   Temp[PitchMax+Frame] ;
    Word16  *Dpnt ;
    LINEDEF  Line ;
    PFDEF    Pf[SubFrames] ;
    Word16   Ftyp;
	Word16   Info;

	DEC_HANDLE *dec = (DEC_HANDLE*)handle;


	Info=bits[0] & (Word16)0x0003 ;
	if(Info==0) dec->mode=G723_63;
	if(Info==1) dec->mode=G723_53;

	if (handle == NULL)
		return -1;

		Line = Line_Unpk( bits, &Ftyp, 0,dec ) ;
		if ( Line.Crc != (Word16) 0 ) 
		{
			if(dec->DecCng.PastFtyp == 1) Ftyp = 1;  /* active */
			else Ftyp = 0;  /* untransmitted */
		}
		if(Ftyp != 1) 
		{

			/* Silence frame : do noise generation */
			Dec_Cng(Ftyp, &Line, out, QntLpc,dec);
		}
		else
		{
			/*
			* Update the frame erasure count (Text: Section 3.10)
			*/
			if ( Line.Crc != (Word16) 0 )
				dec->DecStat.Ecount = add( dec->DecStat.Ecount, (Word16) 1 ) ;
			else
				dec->DecStat.Ecount = (Word16) 0 ;

			if ( dec->DecStat.Ecount > (Word16) ErrMaxNum )
				dec->DecStat.Ecount = (Word16) ErrMaxNum ;

			/*
			* Decode the LSP vector for subframe 3.  (Text: Section 3.2)
			*/
			Lsp_Inq( LspVect, dec->DecStat.PrevLsp, Line.LspId, Line.Crc ) ;

			/*
			* Interpolate the LSP vectors for subframes 0--2.  Convert the
			* LSP vectors to LPC coefficients.  (Text: Section 3.3)
			*/
			Lsp_Int( QntLpc, LspVect, dec->DecStat.PrevLsp ) ;

			/* Copy the LSP vector for the next frame */
			for ( i = 0 ; i < LpcOrder ; i ++ )
				dec->DecStat.PrevLsp[i] = LspVect[i] ;
			if ( dec->DecStat.Ecount == (Word16) 0 ) 
			{
				dec->DecStat.InterGain = add( Line.Sfs[SubFrames-2].Mamp,
												Line.Sfs[SubFrames-1].Mamp ) ;
				dec->DecStat.InterGain = shr( dec->DecStat.InterGain, (Word16) 1 ) ;
				dec->DecStat.InterGain = FcbkGainTable[dec->DecStat.InterGain] ;
			}
			else
				dec->DecStat.InterGain = mult_r( dec->DecStat.InterGain, (Word16) 0x6000 ) ;
			for ( i = 0 ; i < PitchMax ; i ++ )
				Temp[i] = dec->DecStat.PrevExc[i] ;

			Dpnt = &Temp[PitchMax] ;
			if ( dec->DecStat.Ecount == (Word16) 0 )
			{
				 for ( i = 0 ; i < SubFrames ; i ++ ) 
				 {
					Fcbk_Unpk( Dpnt, Line.Sfs[i], Line.Olp[i>>1], (Word16) i ,dec) ;
					Decod_Acbk( AcbkCont, &Temp[SubFrLen*i], Line.Olp[i>>1],
								Line.Sfs[i].AcLg, Line.Sfs[i].AcGn,dec->mode) ;
					for ( j = 0 ; j < SubFrLen ; j ++ ) 
					{
						Dpnt[j] = shl( Dpnt[j], (Word16) 1 ) ;
						Dpnt[j] = add( Dpnt[j], AcbkCont[j] ) ;
					}
					Dpnt += SubFrLen ;
				 }
				             /* Save the excitation */
				for ( j = 0 ; j < Frame ; j ++ )
					out[j] = Temp[PitchMax+j] ;
				dec->DecStat.InterIndx = Comp_Info( Temp, Line.Olp[SubFrames/2-1],
                           &dec->DecCng.SidGain, &dec->DecCng.CurGain ) ;
				if ( dec->UsePf )
					 for ( i = 0 ; i < SubFrames ; i ++ )
						Pf[i] = Comp_Lpf( Temp, Line.Olp[i>>1], (Word16) i,dec) ;
								/* Reload the original excitation */
				for ( j = 0 ; j < PitchMax ; j ++ )
					Temp[j] = dec->DecStat.PrevExc[j] ;
				for ( j = 0 ; j < Frame ; j ++ )
					Temp[PitchMax+j] = out[j] ;
				if ( dec->UsePf )
					for ( i = 0 ; i < SubFrames ; i ++ )
						Filt_Lpf( out, Temp, Pf[i], (Word16) i ) ;
				for(i=0; i< LpcOrder; i++)
					dec->DecCng.LspSid[i] = dec->DecStat.PrevLsp[i];
			}
			else
			{
				Regen( out, Temp, dec->DecStat.InterIndx, dec->DecStat.InterGain,
                            dec->DecStat.Ecount, &dec->DecStat.Rseed ) ;
			}
			        /* Update the previous excitation for the next frame */
			for ( j = 0 ; j < PitchMax ; j ++ )
				dec->DecStat.PrevExc[j] = Temp[Frame+j] ;
			/* Resets random generator for CNG */
			dec->DecCng.RandSeed = 12345;
		}
				/* Save Ftyp information for next frame */
		dec->DecCng.PastFtyp = Ftyp;

		/*
		* Synthesize the speech for the frame
		*/
		Dpnt = out ;
		for ( i = 0 ; i < SubFrames ; i ++ ) 
		{
			Synt( Dpnt, &QntLpc[i*LpcOrder] ,dec) ;
			if ( dec->UsePf ) 
			{

				/* Do the formant post filter. (Text: Section 3.8) */
				Senr = Spf( Dpnt, &QntLpc[i*LpcOrder] ,dec) ;

				/* Do the gain scaling unit.  (Text: Section 3.9) */
				Scale( Dpnt, Senr,dec ) ;
			}

			Dpnt += SubFrLen ;
		}
		*out_len= 240*2;
	return 0;
}


