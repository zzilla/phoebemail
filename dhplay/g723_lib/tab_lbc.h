/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code Version 5.0
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


/*
**
** File:        tab_lbc.c
**
** Description: Tables used for G.723 encoding and decoding
**  
** Tables:      HammingWindowTable[180]
**
**              LPC Computation and Filtering
**
**                  BinomialWindowTable[10]
**                  BandExpTable[10]
**
**              LSP calculation and quantization
**
**                  CosineTable[512]
**                  LspDcTable[10]
**                  BandInfoTable[3][2]
**                  Band0Tb8[256*3]
**                  Band1Tb8[256*3]
**                  Band2Tb8[256*4]
**                  BandQntTable[3]
**
**              Perceptual Filtering and Post Filtering
**

**                  PerFiltZeroTable[10]
**                  PerFiltPoleTable[10]
**                  PostFiltZeroTable[10]
**                  PostFiltPoleTable[10]
**                  LpfConstTable[2] (pitch postfilter)
**
**              ACELP or MP-MLQ
**
**                  Nb_puls[4] (MP-MLQ)
**                  FcbkGainTable[24] (ACELP and MP-MLQ)
**                  MaxPosTable[4]  (MP-MLQ), Word32
**                  CombinatorialTable[6][30](MP-MLQ),Word32
**                  epsi170[170] (ACELP)
**                  gain170[170] (ACELP)
**
**              Pitch Prediction
**
**                  AcbkGainTable085[85*20]
**                  AcbkGainTable170[170*20]
**                  AcbkGainTablePtr[2]
**
**              Taming procedure
**
**                  tabgain170[170]
**                  tabgain85[85]
**
**              Comfort Noise Generation
**
**                  fact[4]
**                  L_bseg[3], Word32
**                  base[3]
**
**
**              All tables are Word16 unless separately denoted
*/

/*
**  HammingWindowTable:
**
**  Hamming Window coefficients scaled by 32768 (Q15).
**
*/
#ifndef _TAB_LBC_
#define _TAB_LBC_

extern Word16   HammingWindowTable[LpcFrame];

/*
**  BinomialWindowTable:
**
**  Purpose:
**     Binomial Window coefficients used to weight the autocorrelation before
**     Levinson-Durbin in the LPC coefficient calculation.  
**
**     Table Structure:
**     Coefficients are scaled by 32768 (Q15).
**
*/

extern Word16   BinomialWindowTable[LpcOrder];

/*
**  BandExpTable:
**
**  Purpose:
**      Do bandwidth expansion on the LPC coefficients by scaling the
**      poles of the LPC synthesis filter by a factor of 0.994

**
**  Table Structure:
**      Table values correspond to (0.994) to the power of x,
**      where x = [0,..,10].
**      These values are scaled by 32768 (Q15).
**
*/

extern Word16  BandExpTable[LpcOrder] ;
   
/*
**  CosineTable:
**
**  Purpose:
**      Used to evaluate polynomial for LSP-LPC conversion
**
**  Table Structure:
**      Contains one period of a cosine wave.  Amplitude has been
**      scaled to go between 0 and 16384 instead of 0 and 1.
**
*/

extern Word16   CosineTable[CosineTableSize];

/*
**  LspDcTable: LSP long term Dc component
**
*/

extern Word16   LspDcTable[LpcOrder] ;

/*
**  BandInfoTable:
**
**  Purpose:
**      Used to index in arrays. Called by LSP sub vector 
**      quantization routines.
**
**  Table Structure:
**      The unquantized LSP vector, quantized LSP vector, and residual
**      LSP error vector are each divided into three subvectors.  
**              The table format is as follows:
**
**      1st pair corresponds to subvector 0
**      2nd pair corresponds to subvector 1
**      3rd pair corresponds to subvector 2
**
**      The 1st slot in each pair is used to index the location of 
**      the vector in a 10-element array.  For example, for subvector
**      1, dimensions [0,..,2] get automatically mapped to array
**      location [3,..,5], and for subvector 3, dimensions
**      [0,..,3] automatically get mapped to array location [6,..,9].
**
**      The 2nd slot in each pair corresponds to the dimension of
**      the subvector
*/

extern Word16   BandInfoTable[LspQntBands][2] ;

/*
**  Band0Tb8:
**
**  Purpose:
**      Vector Quantize the first 3-vector of the 10 LSP parameters.
**
**  Table Structure:
**      8 bit, 256 entry table.
*/

extern Word16   Band0Tb8[LspCbSize*3];

/*
**  Band1Tb8:
**
**  Purpose:
**      Vector Quantize the second 3-vector of the 10 LSP parameters.
**
**  Table Structure:
**      8 bit, 256 entry table.
*/

extern Word16   Band1Tb8[LspCbSize*3];


/*
**  Band2Tb8:
**
**  Purpose:
**      Vector Quantize the last 4-vector of the 10 LSP parameters.
**
**  Table Structure:
**      8 bit, 256 entry table.
*/

extern Word16   Band2Tb8[LspCbSize*4] ;

/*
**  BandQntTable:
**
**  Purpose:
**      collects the three subvector tables.
*/


extern Word16  *BandQntTable[LspQntBands] ;

/*
**  PerFiltZeroTable:
**
**  Purpose:
**     Creates the FIR part of the formant perceptual weighting filter. 
**     Corresponds to gamma1 in section 2.8.
**
**  Table Structure:
**     (0.9) to the x power, where x = [1,..,10]
**     These values are scaled by 32768.
*/

extern Word16   PerFiltZeroTable[LpcOrder] ;

/*
**  PerFiltPoleTable:
**
**  Purpose:
**     Creates the IIR part of the formant perceptual weighting filter. 
**     Corresponds to gamma2 in section 2.8.
**
**  Table Structure:
**     (0.5) to the x power, where x = [1,..,10]
**     These values are scaled by 32768
*/

extern Word16   PerFiltPoleTable[LpcOrder] ;

/*
**  PostFiltZeroTable:
**
**  Purpose:
**     Creates the FIR part of the formant postfilter.  Corresponds to
**     lambda1 in section 3.8.
**
**  Table Structure:
**     (0.65) to the x power, where x = [1,..,10]
**     These values are scaled by 32768
*/

extern Word16   PostFiltZeroTable[LpcOrder] ;

/*
**  PostFiltPoleTable:
**
**  Purpose:
**     Creates the IIR part of the formant postfilter.  Corresponds to
**     lambda2 in section 3.8.
**
**  Table Structure:
**     (0.75) to the x power, where x = [1,..,10]
**     These values are scaled by 32768
*/

extern Word16   PostFiltPoleTable[LpcOrder] ;

/*
**  Nb_puls:
**
**  Purpose:
**      Indexing
**
**  Table Structure:
**      Table values are the number of non-zero pulses in the high-rate
**      excitation (MP-MLQ), indexed by subframe number (0,..,3).
*/

extern Word16   Nb_puls[4];

/*
**  FcbkGainTable:
**
**  Purpose:
**      Logarithmic scalar quantizer in 24 steps of 3.2 dB each
**
**  Table Structure:
**      Contains x where 20*log10(x) = 3.2*i,  i = 1,..,24
*/

extern Word16   FcbkGainTable[NumOfGainLev] ;

/*
**  MaxPosTable:
**
**  Purpose:
**      size of the high rate fixed excitation codebooks (MP-MLQ)
**
**  Table Structure:
**      Table values are the number of codewords in the high rate fixed
**      excitation codebook (MP-MLQ), indexed by subframe number (0,..,3).
**      MaxPosTable[i] is the number of combinations of i elements among 30
**      non-zero pulses in the high-rate
*/

extern Word32   MaxPosTable[4] ;

/*
**  CombinatorialTable:
**
**  Purpose:
**      used for the coding and the decoding of the pulses positions
**      for the high-rate fixed excitation codebook
**
**  Table Structure:
**      CombinatorialTable[i][j] is the number of combinations of
**      (MaxPulsNum-1-i) elements among (SubFrLen-2-j)
*/

extern Word32   CombinatorialTable[MaxPulseNum][SubFrLen/Sgrid];

/*
**  AcbkGainTable085:
**
**  Purpose:
**      Used to calculate the error expression in pitch prediction
**      optimization (also described as an adaptive codebook approach)
**
**      Table Structure:
**      The table is structured as 85 20-element vectors.  These
**      vectors are structured as precalculated values in the error
**      expression for the pitch predictor.  Gi is the gain value
**      multiplying the signal delayed by a pitch period (+/- offset).
**      In equation 41.2, Gi would be equivalent to bij.
**      
**      1st 5 elements:   G1  G2  G3  G4  G5 in Q14 (i.e. scaled by 16384)
**      2nd 5 elements:  -G1^2  -G2^2  -G3^2  -G4^2  -G5^2
**           These are scaled down by 14 bits (factor of 16384) after
**           being squared.
**      Next 10 elements: These are the off-diagonal elements
**           -G1*G2  -G1*G3  -G2*G3  -G1*G4  -G2*G4
**           -G3*G4  -G1*G5  -G2*G5  -G3*G5  -G4*G5
**           These are scaled down by 14 bits (factor of 16384)
*/

extern Word16 AcbkGainTable085[85*20];

/*
**  AcbkGainTable170:
**
**  Purpose:
**      Used to calculate the error expression in pitch prediction
**      optimization (also expressed as an adaptive codebook approach)
**
**  Table Structure:
**      The table is structured as 170 20-element vectors.  These
**      vectors are structured as precalculated values in the error
**      expression for the pitch
**      predictor.  Gi is the gain value multiplying the signal
**      delayed by a pitch period (+/- offset).  In equation 41.2,
**      Gi would be equivalent to bij.
**
**      1st 5 elements:   G1  G2  G3  G4  G5 in Q14 (i.e. scaled by 16384)
**      2nd 5 elements:  -G1^2  -G2^2  -G3^2  -G4^2  -G5^2
**           These are scaled down by 14 bits (factor of 16384) after
**           being squared.
**      Next 10 elements: These are the off-diagonal elements
**           -G1*G2  -G1*G3  -G2*G3  -G1*G4  -G2*G4
**           -G3*G4  -G1*G5  -G2*G5  -G3*G5  -G4*G5
**           These are scaled down by 14 bits (factor of 16384)
*/

extern Word16 AcbkGainTable170[170*20] ;

/*
**  AcbkGainTablePtr:
**
**  Purpose:
**      Contains pointers to 85 and 170 element codebooks
**
*/

extern Word16  *AcbkGainTablePtr[2];


/*
**  LpfConstTable:
**
**  Purpose:
**      Sets the postfilter gain weighting factor.
**
**  Table Structure:
**      (0.1875 , 0.25) in Q15 ( i.e. scale by 32768 ).

*/

extern Word16   LpfConstTable[2] ;


/*
**  epsi170:
**
**  Purpose:
**      At the lower bitrate, the pitch contribution is added
**      into the output vector of ACELP when the pitch period is
**      less than 60 samples (one subframe). The pitch contribution
**      is estimated by a 1 tap long term predictor (LTP) instead of
**      the 5 tap LTP used in pitch prediction. The values in epsi170
**      effectively choose one of the 5 vectors used in the 5 tap LTP.
**
**  Table Structure:
**      The table is indexed by the index into the AcbkGainTable170
**      table (determined in Find_Acbk). The value, 60, is used only
**      as a place holder. They correspond to zeros in the gain170
**      table, which is the coefficient of the 1 tap long term
**      predictor (LTP).  The rest of the values range from -2 to +2.
**      These values pick one vector among the 5 vectors centered one
**      pitch period behind the current vector. (In contrast, the 5
**      tap LTP uses all five vectors centered one pitch period back.)
*/
extern Word16 epsi170[170];

/*
**  gain170:
**
**  Purpose:
**      At the lower bitrate, the pitch contribution is added into
**      the output vector of ACELP when the pitch period is less than
**      60 samples (one subframe). The pitch contribution
**      is estimated by a 1 tap long term predictor (LTP) instead of
**      the 5 tap LTP used in pitch prediction. This table holds the
**      coefficient for the one tap filter.
**
**  Table Structure:
**      The table is indexed by the index into the AcbkGainTable170
**      table (determined in Find_Acbk). The values are in Q12.
*/

extern Word16 gain170[170] ;

/*
**  tabgain170:
**
**  Purpose:
**      In the taming procedure at the encoder the 170 5-taps LT filters
**      are modelled as 1-tap filter. This table gives the gain
**      value attributed to each filter as a worst case gain
**
**  Table Structure:
**      The table is indexed by the index into the AcbkGainTable170
**      table (determined in Find_Acbk). The values are in Q13.
*/
extern Word16 tabgain170[170] ;

/*
**  tabgain85:
**
**  Purpose:
**      In the taming procedure at the encoder the 85 5-taps LT filters
**      are modelled as 1-tap filter. This table gives the gain
**      value attributed to each filter as a worst case gain
**
**  Table Structure:
**      The table is indexed by the index into the AcbkGainTable085
**      table (determined in Find_Acbk). The values are in Q13.
*/
extern Word16 tabgain85[85] ;

/*
**  fact   :
**  L_bseg :
**  base   :
**
**  Purpose:
**      Quantization of the SID gain procedure
**      table fact :
**      for n = 0 :  used to quantize the excitation energy estimation
**                   in Comp_Info()
**                   fact(0) = 32768 / (2 * SubFrLen)
**
**      for n >= 1 : used for average energy calculation
**                   fact(n) = fact_mul**2 * 32768 / n * Frame
**      table L_bseg : for each segment of the pseudo-log quantizer,
**                     gives the energy level corresponding to
**                     the first value of the next segment
**      table base : first code of each segment
**
*/
extern Word16 fact[4];
extern Word32 L_bseg[3]; /* Bug fixed 24-09-96 */
extern Word16 base[3];

#endif

