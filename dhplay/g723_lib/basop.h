#ifndef _Basop_H_
#define _Basop_H_
#include"typedef.h"

/*________________________________________________________
___________________
 |
                  |
 |   Constants and Globals
                  |

|_________________________________________________________
__________________|
*/

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000


/*________________________________________________________
___________________
 |
                  |
 |   Operators prototypes
                  |

|_________________________________________________________
__________________|
*/

extern Flag Overflow_g723;
extern Flag Carry_g723;

#ifdef CCS

#define add( L_var1,  L_var2) _sadd( L_var1,  L_var2)
#define sub( L_var1,  L_var2) _ssub( L_var1,  L_var2)
#define abs_s( L_var1) _abs(L_var1)
#define shl( L_var1,  var2) _sshvl( L_var1, var2)
#define shr( L_var1,  var2) _sshvr( L_var1, var2)
#define mult( var1,  var2) _mpylir(var1,var2)
#define L_mult( var1,  var2) _smpy(var1,var2)
#define negate(var1) _ssub(0,var1)
#define L_add( L_var1,  L_var2) _sadd( L_var1,  L_var2)
#define L_sub( L_var1,  L_var2) _ssub( L_var1,  L_var2)
#define L_sub_c( L_var1,  L_var2) _subc( L_var1,  L_var2)

#define mult_r( var1,  var2) _mpylir(var1,var2)
#define L_shl( L_var1,  var2) _sshvl( L_var1, var2)
#define L_shr( L_var1,  var2) _sshvr( L_var1, var2)
#define L_abs( L_var1) _abs(L_var1)
#define norm_l( L_var1)  _norm(L_var1)
#define i_mult( var1,  var2) _mpy(var1,var2)

#endif


#ifdef VC

_inline Word16 add(Word16 var1, Word16 var2);     
_inline Word16 sub(Word16 var1, Word16 var2);     
_inline Word16 abs_s(Word16 var1);               
_inline Word16 shl(Word16 var1, Word16 var2);     
_inline Word16 shr(Word16 var1, Word16 var2);     
_inline Word16 mult(Word16 var1, Word16 var2);    
_inline Word32 L_mult(Word16 var1, Word16 var2);  
_inline Word16 negate(Word16 var1);               
_inline Word16 extract_h(Word32 L_var1);          
_inline Word16 extract_l(Word32 L_var1);         
_inline Word16 round(Word32 L_var1);             
_inline Word32 L_mac(Word32 L_var3, Word16 var1, Word16 var2);
_inline Word32 L_msu(Word32 L_var3, Word16 var1, Word16 var2); 
_inline Word32 L_macNs(Word32 L_var3, Word16 var1, Word16 var2);
_inline Word32 L_msuNs(Word32 L_var3, Word16 var1, Word16 var2);

_inline Word32 L_add(Word32 L_var1, Word32 L_var2);  
_inline Word32 L_sub(Word32 L_var1, Word32 L_var2);  
_inline Word32 L_add_c(Word32 L_var1, Word32 L_var2);
_inline Word32 L_sub_c(Word32 L_var1, Word32 L_var2); 
_inline Word32 L_negate(Word32 L_var1);             
_inline Word16 mult_r(Word16 var1, Word16 var2);  
_inline Word32 L_shl(Word32 L_var1, Word16 var2); 
_inline Word32 L_shr(Word32 L_var1, Word16 var2); 
_inline Word16 shr_r(Word16 var1, Word16 var2);
_inline Word16 mac_r(Word32 L_var3, Word16 var1, Word16 var2);
_inline Word16 msu_r(Word32 L_var3, Word16 var1, Word16 var2);
_inline Word32 L_deposit_h(Word16 var1);      
_inline Word32 L_deposit_l(Word16 var1);       

_inline Word32 L_shr_r(Word32 L_var1, Word16 var2);
_inline Word32 L_abs(Word32 L_var1);           

_inline Word32 L_sat(Word32 L_var1);           

_inline Word16 norm_s(Word16 var1);             

_inline Word16 div_s(Word16 var1, Word16 var2); 

_inline Word16 norm_l(Word32 L_var1);          

_inline Word32 L_mls( Word32, Word16 ) ;       
_inline Word16 div_l( Word32, Word16 ) ;
_inline Word16 i_mult(Word16 a, Word16 b);

#endif

#endif
