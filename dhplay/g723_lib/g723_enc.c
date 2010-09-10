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





int g723enc_init(void** handle, eG723Mode mode)
{
	ENC_HANDLE* tmp;
	int i;
	tmp =malloc(sizeof(ENC_HANDLE));
	if (tmp == NULL)
		return -1;

	memset(tmp, 0, sizeof(ENC_HANDLE));
	/* Initialize the previously decoded LSP vector to the DC vector */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        tmp->CodStat.PrevLsp[i] = LspDcTable[i] ;

    /* Initialize the taming procedure */
    for(i=0; i<SizErr; i++)  
		tmp->CodStat.Err[i] = Err0;
	
	tmp->UseHp=True;
	tmp->UseVx=False;

	if( tmp->UseVx ) 
	{
        Init_Vad(tmp);
        Init_Cod_Cng(tmp);
    }
	tmp->mode=mode;
	tmp->First_Init=1;

	*handle = tmp;
	return 0;
}


int g723enc_mode(void* handle, eG723Mode mode)
{
	ENC_HANDLE* tmp=(ENC_HANDLE*)handle;
	if(handle==NULL)
		return -1;
	
	tmp->mode=mode;
	return 0;
}

int g723enc_close(void** handle)
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

int g723enc(void* handle, short* sample, char* bits, int src_len, int* out_len)
{
	 int     i,j;
	char stride;
    Word16   UnqLpc[SubFrames*LpcOrder] ;
    Word16   QntLpc[SubFrames*LpcOrder] ;
    Word16   PerLpc[2*SubFrames*LpcOrder] ;
    Word16   LspVect[LpcOrder] ;
    LINEDEF  Line  ;
    PWDEF    Pw[SubFrames]  ;
    Word16   ImpResp[SubFrLen] ;
    Word16  *Dpnt  ;
    Word16  Ftyp = 1 ;

	ENC_HANDLE *enc = (ENC_HANDLE*)handle;

	if (handle == NULL)
		return -1;
	if(enc->mode==G723_63)
		stride=24;
	else
		stride=20;

	Line.Crc=(Word16) 0 ;


		Line.Crc=(Word16) 0 ;
		if(enc->mode==G723_53)
			reset_max_time();
		Rem_Dc(sample,enc);
		/* Compute the Unquantized Lpc set for whole frame */
		Comp_Lpc( UnqLpc, enc, sample ) ;
		/* Convert to Lsp */
		AtoLsp( LspVect, &UnqLpc[LpcOrder*(SubFrames-1)], enc->CodStat.PrevLsp ) ;
		/* Compute the Vad */
		Ftyp = (Word16) Comp_Vad( sample,enc) ;
	    /* VQ Lsp vector */
		Line.LspId = Lsp_Qnt( LspVect, enc->CodStat.PrevLsp ) ;

		Mem_Shift( enc->CodStat.PrevDat, sample ) ;

		/* Compute Perceptual filter Lpc coefficients */
		Wght_Lpc( PerLpc, UnqLpc ) ;
		/* Apply the perceptual weighting filter */
		Error_Wght( sample, PerLpc,enc ) ;
		// Compute Open loop pitch estimates
		Dpnt = (Word16 *) malloc( sizeof(Word16)*(PitchMax+Frame) ) ;
		/* Construct the buffer */
		for ( i = 0 ; i < PitchMax ; i ++ )
			Dpnt[i] = enc->CodStat.PrevWgt[i] ;
		for ( i = 0 ; i < Frame ; i ++ )
			Dpnt[PitchMax+i] = sample[i] ;

		Vec_Norm( Dpnt, (Word16) (PitchMax+Frame) ) ;

		j = PitchMax ;
		for ( i = 0 ; i < SubFrames/2 ; i ++ ) 
		{
			Line.Olp[i] = Estim_Pitch( Dpnt, (Word16) j ) ;
			enc->VadStat.Polp[i+2] = Line.Olp[i] ;
			j += 2*SubFrLen ;
		}

		if(Ftyp != 1) 
		{

			/*
			// Case of inactive signal
			*/
			free ( (char *) Dpnt ) ;

			/* Save PrevWgt */
			for ( i = 0 ; i < PitchMax ; i ++ )
				enc->CodStat.PrevWgt[i] = sample[i+Frame-PitchMax] ;

			/* CodCng => Ftyp = 0 (untransmitted) or 2 (SID) */
			Cod_Cng(sample, &Ftyp, &Line, QntLpc,enc);

			/* Update the ringing delays */
			Dpnt = sample;
			for( i = 0 ; i < SubFrames; i++ ) 
			{

				/* Update exc_err */
				Update_Err(Line.Olp[i>>1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn,enc);

				Upd_Ring( Dpnt, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
															enc->CodStat.PrevErr,enc ) ;
				Dpnt += SubFrLen;
			}
		}
		else
		{
			/*
			// Case of Active signal  (Ftyp=1)
			*/

			/* Compute the Hmw */
			j = PitchMax ;
			for ( i = 0 ; i < SubFrames ; i ++ ) 
			{
				Pw[i] = Comp_Pw( Dpnt, (Word16) j, Line.Olp[i>>1] ) ;
				j += SubFrLen ;
			}
			/* Reload the buffer */
			for ( i = 0 ; i < PitchMax ; i ++ )
				Dpnt[i] = enc->CodStat.PrevWgt[i] ;
			for ( i = 0 ; i < Frame ; i ++ )
				Dpnt[PitchMax+i] = sample[i] ;

			/* Save PrevWgt */
			for ( i = 0 ; i < PitchMax ; i ++ )
				enc->CodStat.PrevWgt[i] = Dpnt[Frame+i] ;
			        /* Apply the Harmonic filter */
			j = 0 ;
			for ( i = 0 ; i < SubFrames ; i ++ )
			{
				Filt_Pw( sample, Dpnt, (Word16) j , Pw[i] ) ;
				j += SubFrLen ;
			}
			free ( (char *) Dpnt ) ;
			/* Inverse quantization of the LSP */
			Lsp_Inq( LspVect, enc->CodStat.PrevLsp, Line.LspId, Line.Crc ) ;
			/* Interpolate the Lsp vectors */
			Lsp_Int( QntLpc, LspVect, enc->CodStat.PrevLsp ) ;
			        /* Copy the LSP vector for the next frame */
			for ( i = 0 ; i < LpcOrder ; i ++ )
				enc->CodStat.PrevLsp[i] = LspVect[i] ;

			/*
			// Start the sub frame processing loop
			*/
			Dpnt = sample ;
			for ( i = 0 ; i < SubFrames ; i ++ ) 
			{
				            /* Compute full impulse response */
				Comp_Ir( ImpResp, &QntLpc[i*LpcOrder],&PerLpc[i*2*LpcOrder], Pw[i] ) ;
				            /* Subtract the ringing of previous sub-frame */
				Sub_Ring( Dpnt, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],enc, Pw[i] ) ;
				            /* Compute adaptive code book contribution */
				Find_Acbk( Dpnt, ImpResp, enc, &Line, (Word16) i ) ;
				            /* Compute fixed code book contribution */
				Find_Fcbk( Dpnt, ImpResp, &Line, (Word16) i ,enc) ;
				Decod_Acbk( ImpResp, enc->CodStat.PrevExc, Line.Olp[i>>1],
                        Line.Sfs[i].AcLg, Line.Sfs[i].AcGn,enc->mode ) ;
				
				for ( j = SubFrLen ; j < PitchMax ; j ++ )
					enc->CodStat.PrevExc[j-SubFrLen] =enc->CodStat.PrevExc[j] ;

				for ( j = 0 ; j < SubFrLen ; j ++ )
				{
					Dpnt[j] = shl( Dpnt[j], (Word16) 1 ) ;
					Dpnt[j] = add( Dpnt[j], ImpResp[j] ) ;
					enc->CodStat.PrevExc[PitchMax-SubFrLen+j] = Dpnt[j] ;
				}
				/* Update exc_err */
				Update_Err(Line.Olp[i>>1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn,enc);
				/* Update the ringing delays */
				Upd_Ring( Dpnt, &QntLpc[i*LpcOrder], &PerLpc[i*2*LpcOrder],
                                                       enc->CodStat.PrevErr ,enc) ;
				Dpnt += SubFrLen ;
			}
			enc->CodCng.PastFtyp = 1;
			enc->CodCng.RandSeed = 12345;
		}
		Line_Pack( &Line, bits, Ftyp ,enc,stride) ;
		if(enc->mode==G723_63)
			*out_len=24;
		else
			*out_len=20;
	return 0;
}

