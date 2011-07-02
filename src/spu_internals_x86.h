#pragma once
#ifndef __SPU_INTERNALS_X86_H__
#define __SPU_INTERNALS_X86_H__

#include "spu_emu.h"
#include <cstdint>
#include <intrin.h>
#include "sse_extensions.h"


// Helpers
//////////////////////////////////////////////////////////////////////////

inline uint64_t si_to_ullong_aux(__m128i ra)
{ 
const uint64_t r0 = (uint16_t)_mm_extract_epi16((ra), 7);
const uint64_t r1 = (uint16_t)_mm_extract_epi16((ra), 6);
const uint64_t r2 = (uint16_t)_mm_extract_epi16((ra), 5);
const uint64_t r3 = (uint16_t)_mm_extract_epi16((ra), 4);
return (r0 << 48) | (r1 << 32) | (r2 << 16) | r3;
	/*uint64_t r; 
	uint16_t* pr = (uint16_t*)&r;
	*pr =	(uint16_t)_mm_extract_epi16((ra), 7);
	*(pr+1) =	(uint16_t)_mm_extract_epi16((ra), 6);
	*(pr+2) =	(uint16_t)_mm_extract_epi16((ra), 5);
	*(pr+3) =	(uint16_t)_mm_extract_epi16((ra), 4);
	return r;*/
}

inline float si_to_float_aux(__m128 ra)
{
const uint32_t u = (uint32_t)si_to_ullong_aux(_mm_castps_si128(ra));
return (float&)u;
}

inline double si_to_double_aux(__m128 ra)
{
const uint64_t u = si_to_ullong_aux(_mm_castps_si128(ra));
return (double&)u;
}

/*
 * Constant-Formation Instructions 
 */

inline GPR_t si_ilh	( const int16_t I16 )	{ return _mm_castsi128_ps( _mm_set1_epi16( I16 ) ); }
inline GPR_t si_ilhu( const int16_t I16 )	{ return _mm_castsi128_ps( _mm_set1_epi32( (int32_t)I16 << 16 ) ); }
inline GPR_t si_il	( const int16_t I16 )	{ return _mm_castsi128_ps( _mm_set1_epi32( (int32_t)I16 ) ); }
inline GPR_t si_ila	( const int32_t I18 )	{ return _mm_castsi128_ps( _mm_set1_epi32( I18 ) ); }
inline GPR_t si_iohl( GPR_t RA, const int16_t I16 ){ return _mm_castsi128_ps( _mm_or_si128( _mm_castps_si128( RA ), _mm_set1_epi32( I16 ) ) ); }
//inline GPR_t si_iohl( GPR_t RA, const int16_t I16 ){	return _mm_or_ps( RA, _mm_castsi128_ps(_mm_set1_epi32( I16 ) ) ); }
inline GPR_t si_fsmbi( const int16_t I16 ){
	const __m128i mask_all_set = _mm_setr_epi8( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80i8,
		0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80i8 );

	const __m128i r1 = _mm_set1_epi8( 0xff & I16 );
	const __m128i r2 = _mm_set1_epi8( 0xff & (I16 >> 8) );
	const __m128i r3 = _mm_srli_si128( r1, 8 );
	const __m128i r4 = _mm_slli_si128( r2, 8 );
	const __m128i r5 = _mm_and_si128( r3, mask_all_set );
	const __m128i r6 = _mm_and_si128( r4, mask_all_set );
	const __m128i r7 = _mm_or_si128( r5, r6 );
	return _mm_castsi128_ps( r7 );
}

/*
 * Add Instructions 
 */



inline GPR_t si_ah( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_add_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ahi( GPR_t RA, int16_t I10 ){
	return _mm_castsi128_ps( _mm_add_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( I10 ) ) );
}

inline GPR_t si_a( GPR_t RA, GPR_t RB ){
	return (v_u32)RA + (v_u32)RB;
	//return _mm_castsi128_ps( _mm_add_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ai( GPR_t RA, int16_t I10 ){
	return (v_u32)RA + v_u32(I10);
	//return _mm_castsi128_ps( _mm_add_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( I10 ) ) );
}

inline GPR_t si_addx( GPR_t RA, GPR_t RB, GPR_t RT ){
	auto sum	= (v_u32)RA + (v_u32)RB;
	auto carry	= (v_u32)RT + v_u32(1);
	return sum & carry;
	//const __m128i sum		= _mm_add_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	//const __m128i carry	= _mm_and_si128( _mm_castps_si128( RT ), _mm_set1_epi32( 1 ) );
	//return _mm_castsi128_ps( _mm_add_epi32( sum, carry ) );
}

inline GPR_t si_cg( GPR_t RA, GPR_t RB ){
	const __m128i RA_and_RB		= _mm_and_si128( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i RA_and_RB_LSB = _mm_srli_epi32( _mm_slli_epi32( RA_and_RB, 31 ), 31 );
	const __m128i RA_shift		= _mm_srli_epi32( _mm_castps_si128( RA ), 1 );
	const __m128i RB_shift		= _mm_srli_epi32( _mm_castps_si128( RB ), 1 );
	const __m128i sum			= _mm_add_epi32( RA_shift, RB_shift );
	const __m128i sum_adjusted	= _mm_add_epi32( sum, RA_and_RB_LSB );
	return _mm_castsi128_ps( _mm_srli_epi32( sum_adjusted, 31 ) );
}

inline GPR_t si_cgx( GPR_t RA, GPR_t RB, GPR_t RT ){
	const __m128i RA_and_RB		= _mm_and_si128( _mm_castps_si128( RA ), _mm_castps_si128( RB ) );
	const __m128i RA_and_RB_LSB = _mm_srli_epi32( _mm_slli_epi32( RA_and_RB, 31 ), 31 );
	const __m128i RA_shift		= _mm_srli_epi32( _mm_castps_si128( RA ), 1 );
	const __m128i RB_shift		= _mm_srli_epi32( _mm_castps_si128( RB ), 1 );
	const __m128i sum			= _mm_add_epi32( RA_shift, RB_shift );
	const __m128i sum_adjusted	= _mm_add_epi32( sum, RA_and_RB_LSB );
	const __m128i RT_carry		= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128( RT ), 31 ), 31 );
	return _mm_castsi128_ps( _mm_srli_epi32( _mm_add_epi32( RT_carry, sum_adjusted ), 31 ) );
}


/*
 * Sub Instructions 
 */

inline GPR_t si_sfh( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_sub_epi16( _mm_castps_si128( RB ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_sfhi( GPR_t RA, int16_t IMM ){
	return _mm_castsi128_ps( _mm_sub_epi16( _mm_set1_epi16( IMM ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_sf( GPR_t RA, GPR_t RB ){
	return _mm_castsi128_ps( _mm_sub_epi32( _mm_castps_si128( RB ), _mm_castps_si128( RA ) ) );
}

inline GPR_t si_sfi( GPR_t RA, int16_t IMM ){
	return _mm_castsi128_ps( _mm_sub_epi16( _mm_set1_epi32( (int32_t)IMM ), _mm_castps_si128( RA ) ) );
}



inline GPR_t si_bg( GPR_t RA, GPR_t RB )
{
	// returns 0 if it borrows ((RB - RA) < 0)
	const __m128i unsigned_less_than = _mm_cmplt_epu32( _mm_castps_si128( RB ), _mm_castps_si128( RA ) );
	return _mm_castsi128_ps( _mm_andnot_si128( unsigned_less_than, _MM_CONST_1(RA)/*_mm_set1_epi32(1)*/ ) );
}

inline GPR_t si_bgx( GPR_t RA, GPR_t RB, GPR_t RT )
{
	// returns 0 if it borrows ((RB - RA - (RT_LSB-1)) < 0)
	const __m128i RT_LSB = _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128( RT ), 31 ), 31 );
	const __m128i borrow = _mm_sub_epi32( RT_LSB, _mm_set1_epi32(1) ); // 1 -> 0, 0 -> -1
	const __m128i RB_adjusted = _mm_add_epi32( _mm_castps_si128( RB ), borrow );
	const __m128i unsigned_less_than = _mm_cmplt_epu32( RB_adjusted, _mm_castps_si128( RB ) );
	return _mm_castsi128_ps( _mm_andnot_si128( unsigned_less_than, _MM_CONST_1(RA)/*_mm_set1_epi32(1)*/ ) );
}

inline GPR_t si_sfx( GPR_t RA, GPR_t RB, GPR_t RT )
{
	const __m128i diff = _mm_sub_epi32( _mm_castps_si128( RB ), _mm_castps_si128( RA ) );
	// sub. 1 if RT_LSB == 0
	const __m128i RT_LSB = _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128( RT ), 31 ), 31 );
	const __m128i borrow = _mm_sub_epi32( RT_LSB, _mm_set1_epi32(1) ); // 1 -> 0, 0 -> -1
	return _mm_castsi128_ps( _mm_add_epi32( diff, borrow ) );
}


/*
 * Multiply Instructions 
 */

inline GPR_t si_mpy( GPR_t RA, GPR_t RB )
{
	const __m128i RA_lo		= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128(RA), 16 ), 16 );
	const __m128i RB_lo		= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128(RB), 16 ), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_lo,  RB_lo ); 
	const __m128i ires_hi	= _mm_mulhi_epi16( RA_lo,  RB_lo );

	return _mm_castsi128_ps( _mm_or_si128( res_lo, _mm_slli_epi32(ires_hi, 16) ) );
}

inline GPR_t si_mpyu( GPR_t RA, GPR_t RB )
{
	const __m128i RA_lo		= _mm_and_si128( _mm_castps_si128(RA), _mm_set1_epi32(0x0000FFFF) );
	const __m128i RB_lo		= _mm_and_si128( _mm_castps_si128(RB), _mm_set1_epi32(0x0000FFFF) );

	const __m128i res_lo	= _mm_mullo_epi16( RA_lo,  RB_lo );
	const __m128i ures_hi	= _mm_mulhi_epu16( RA_lo,  RB_lo );

	return _mm_castsi128_ps( _mm_or_si128( res_lo, _mm_slli_epi32(ures_hi, 16) ) );
}

inline GPR_t si_mpyi( GPR_t RA, int64_t IMM )
{
	return si_mpy( RA, _mm_castsi128_ps( _mm_set1_epi32( (uint16_t)IMM ) ) );
}

inline GPR_t si_mpyui( GPR_t RA, int64_t IMM )
{
	return si_mpyu( RA, _mm_castsi128_ps( _mm_set1_epi32( (uint16_t)IMM ) ) );
}

inline GPR_t si_mpya( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128i product = _mm_castps_si128( si_mpy( RA, RB ) );
	return _mm_castsi128_ps( _mm_add_epi32( product, _mm_castps_si128( RC ) ) );
}

inline GPR_t si_mpyh( GPR_t RA, GPR_t RB )
{
	const __m128i RA_hi		= _mm_srli_epi32( _mm_castps_si128(RA), 16 );
	const __m128i RB_lo		= _mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128(RB), 16 ), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_hi,  RB_lo );

	return _mm_castsi128_ps( res_lo );
}

inline GPR_t si_mpys( GPR_t RA, GPR_t RB )
{
	const __m128i product = _mm_castps_si128( si_mpy( RA, RB ) );
	return _mm_castsi128_ps( _mm_srai_epi32( _mm_slli_epi32( product, 16 ), 16 ) );
}

inline GPR_t si_mpyhh( GPR_t RA, GPR_t RB )
{
	const __m128i RA_hi		= _mm_srli_epi32( _mm_castps_si128(RA), 16 );
	const __m128i RB_hi		= _mm_srli_epi32( _mm_castps_si128(RB), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_hi,  RB_hi );
	const __m128i ires_hi	= _mm_mulhi_epi16( RA_hi,  RB_hi );

	return _mm_castsi128_ps( _mm_or_si128( res_lo, _mm_slli_epi32(ires_hi, 16) ) );
}

inline GPR_t si_mpyhha( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128i product = _mm_castps_si128( si_mpyhh( RA, RB ) );
	return _mm_castsi128_ps( _mm_add_epi32( product, _mm_castps_si128( RC ) ) );
}

inline GPR_t si_mpyhhu( GPR_t RA, GPR_t RB )
{
	const __m128i RA_hi		= _mm_srli_epi32( _mm_castps_si128(RA), 16 );
	const __m128i RB_hi		= _mm_srli_epi32( _mm_castps_si128(RB), 16 );

	const __m128i res_lo	= _mm_mullo_epi16( RA_hi,  RB_hi );
	const __m128i ures_hi	= _mm_mulhi_epu16( RA_hi,  RB_hi );

	return _mm_castsi128_ps( _mm_or_si128( res_lo, _mm_slli_epi32(ures_hi, 16) ) );
}

inline GPR_t si_mpyhhau( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128i product = _mm_castps_si128( si_mpyhhu( RA, RB ) );
	return _mm_castsi128_ps( _mm_add_epi32( product, _mm_castps_si128( RC ) ) );
}


/*
 * Integer Misc. Instructions 
 */

/* Some SSE extensions*/



inline GPR_t si_selb( GPR_t RA, GPR_t RB, GPR_t RC )
{
	const __m128 bitsRA = _mm_andnot_ps( RC, RA );
	const __m128 bitsRB = _mm_and_ps( RC, RB );
	return _mm_or_ps( bitsRA, bitsRB );
}

inline GPR_t si_cntb( GPR_t RA )
{
	return _mm_castsi128_ps( _mm_cntb_epui8( _mm_castps_si128( RA ) ) );
}

inline GPR_t si_clz( GPR_t RA )
{
	const __m128i t0 = _mm_or_si128( _mm_castps_si128( RA ), _mm_srli_epi32( _mm_castps_si128( RA ), 1 ) );
	const __m128i t1 = _mm_or_si128( t0, _mm_srli_epi32( t0, 2 ) );
	const __m128i t2 = _mm_or_si128( t1, _mm_srli_epi32( t1, 4 ) );
	const __m128i t3 = _mm_or_si128( t2, _mm_srli_epi32( t2, 8 ) );
	const __m128i t4 = _mm_or_si128( t3, _mm_srli_epi32( t3, 16 ) );
	const __m128i ones = _mm_cntb_epi32( t4 );
	return _mm_castsi128_ps( _mm_sub_epi32( _mm_set1_epi32(32), ones ) );
}

/*
 * Convert to scalar
 */

#define si_to_char(ra)		(int8_t)si_to_ullong_aux((ra))
#define si_to_uchar(ra)     (uint8_t)si_to_ullong_aux((ra))
#define si_to_short(ra)     (int16_t)si_to_ullong_aux((ra))
#define si_to_ushort(ra)    (uint16_t)si_to_ullong_aux((ra))
#define si_to_int(ra)       (int32_t)si_to_ullong_aux((ra))
#define si_to_uint(ra)      (uint32_t)si_to_ullong_aux((ra))
#define si_to_llong(ra)     (int64_t)si_to_ullong_aux((ra))
#define si_to_ullong(ra)    (uint64_t)si_to_ullong_aux((ra))
#define si_to_float(ra)     si_to_float_aux((ra))
#define si_to_double(ra)    si_to_double_aux((ra))
#define si_to_ptr(ra)       (void*)si_to_ullong_aux((ra))


/************************************************************************/
/* Comparison: EQ, GT, LGT                                              */
/************************************************************************/ 

inline GPR_t si_ceqb( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ceqh( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ceq( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_ceqbi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_ceqhi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_ceqi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}



inline GPR_t si_cgtb( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_cgth( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_cgt( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpgt_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_cgtbi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_cgthi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_cgti( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpgt_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}


inline GPR_t si_clgtb( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpgt_epu8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_clgth( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpgt_epu16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_clgt( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpgt_epu32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

inline GPR_t si_clgtbi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpgt_epu8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_clgthi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpgt_epu16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_clgti( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpgt_epu32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}




#endif


/* the orignals SPU intrinsics */

// si_lqd(ra,imm)       
// si_lqx(ra,rb)        
// si_lqa(imm)          
// si_lqr(imm)          
// si_stqd(rt,ra,imm)   
// si_stqx(rt,ra,rb)    
// si_stqa(rt,imm)      
// si_stqr(rt,imm)      
// si_cbd(ra,imm)       
// si_cbx(ra,rb)        
// si_chd(ra,imm)       
// si_chx(ra,rb)        
// si_cwd(ra,imm)       
// si_cwx(ra,rb)        
// si_cdd(ra,imm)       
// si_cdx(ra,rb)        
// si_ilh(imm)          
// si_ilhu(imm)         
// si_il(imm)           
// si_ila(imm)          
// si_iohl(ra,imm)      
// si_fsmbi(imm)        
// si_ah(ra,rb)         
// si_ahi(ra,imm)       
// si_a(ra,rb)          
// si_ai(ra,imm)        
// si_addx(ra,rb,rt)    
// si_cg(ra,rb)         
// si_cgx(ra,rb,rt)     
// si_sfh(ra,rb)        
// si_sfhi(imm,ra)      
// si_sf(ra,rb)         
// si_sfi(ra,imm)       
// si_sfx(ra,rb,rt)     
// si_bg(ra,rb)         
// si_bgx(ra,rb,rt)     
// si_mpy(ra,rb)        
// si_mpyu(ra,rb)       
// si_mpyi(ra,imm)      
// si_mpyui(ra,imm)     
// si_mpya(ra,rb,rc)    
// si_mpyh(ra,rb)       
// si_mpys(ra,rb)       
// si_mpyhh(ra,rb)      
// si_mpyhhu(ra,rb)     
// si_mpyhha(ra,rb,rc)  
// si_mpyhhau(ra,rb,rc) 
// si_clz(ra)           
// si_cntb(ra)          
// si_fsmb(ra)          
// si_fsmh(ra)          
// si_fsm(ra)           
// si_gbb(ra)           
// si_gbh(ra)           
// si_gb(ra)            
// si_avgb(ra,rb)       
// si_absdb(ra,rb)      
// si_sumb(ra,rb)       
// si_xsbh(ra)          
// si_xshw(ra)          
// si_xswd(ra)          
// si_and(ra,rb)        
// si_andc(ra,rb)       
// si_andbi(ra,imm)     
// si_andhi(ra,imm)     
// si_andi(ra,imm)      
// si_or(ra,rb)         
// si_orc(ra,rb)        
// si_orbi(ra,imm)      
// si_orhi(ra,imm)      
// si_ori(ra,imm)       
// si_orx(ra)           
// si_xor(ra,rb)        
// si_xorbi(ra,imm)     
// si_xorhi(ra,imm)     
// si_xori(ra,imm)      
// si_nand(ra,rb)       
// si_nor(ra,rb)        
// si_eqv(ra,rb)        
// si_selb(ra,rb,rc)    
// si_shufb(ra,rb,rc)   
// si_shlh(ra,rb)       
// si_shlhi(ra,imm)     
// si_shl(ra,rb)        
// si_shli(ra,imm)      
// si_shlqbi(ra,rb)     
// si_shlqbii(ra,imm)   
// si_shlqby(ra,rb)     
// si_shlqbyi(ra,imm)   
// si_shlqbybi(ra,rb)   
// si_roth(ra,rb)       
// si_rothi(ra,imm)     
// si_rot(ra,rb)        
// si_roti(ra,imm)      
// si_rotqby(ra,rb)     
// si_rotqbyi(ra,imm)   
// si_rotqbybi(ra,rb)   
// si_rotqbi(ra,rb)     
// si_rotqbii(ra,imm)   
// si_rothm(ra,rb)      
// si_rothmi(ra,imm)    
// si_rotm(ra,rb)       
// si_rotmi(ra,imm)     
// si_rotqmby(ra,rb)    
// si_rotqmbyi(ra,imm)  
// si_rotqmbi(ra,rb)    
// si_rotqmbii(ra,imm)  
// si_rotqmbybi(ra,rb)  
// si_rotmah(ra,rb)     
// si_rotmahi(ra,imm)   
// si_rotma(ra,rb)      
// si_rotmai(ra,imm)    
// si_heq(ra,rb)        
// si_heqi(ra,imm)      
// si_hgt(ra,rb)        
// si_hgti(ra,imm)      
// si_hlgt(ra,rb)       
// si_hlgti(ra,imm)     
// si_ceqb(ra,rb)       
// si_ceqbi(ra,imm)     
// si_ceqh(ra,rb)       
// si_ceqhi(ra,imm)     
// si_ceq(ra,rb)        
// si_ceqi(ra,imm)      
// si_cgtb(ra,rb)       
// si_cgtbi(ra,imm)     
// si_cgth(ra,rb)       
// si_cgthi(ra,imm)     
// si_cgt(ra,rb)        
// si_cgti(ra,imm)      
// si_clgtb(ra,rb)      
// si_clgtbi(ra,imm)    
// si_clgth(ra,rb)      
// si_clgthi(ra,imm)    
// si_clgt(ra,rb)       
// si_clgti(ra,imm)     
// si_bisled(ra)        
// si_bisledd(ra)       
// si_bislede(ra)       
// si_fa(ra,rb)         
// si_dfa(ra,rb)        
// si_fs(ra,rb)         
// si_dfs(ra,rb)        
// si_fm(ra,rb)         
// si_dfm(ra,rb)        
// si_fma(ra,rb,rc)     
// si_dfma(ra,rb,rc)    
// si_dfnma(ra,rb,rc)   
// si_fnms(ra,rb,rc)    
// si_dfnms(ra,rb,rc)   
// si_fms(ra,rb,rc)     
// si_dfms(ra,rb,rc)    
// si_frest(ra)         
// si_frsqest(ra)       
// si_fi(ra,rb)         
// si_csflt(ra,imm)     
// si_cflts(ra,imm)     
// si_cuflt(ra,imm)     
// si_cfltu(ra,imm)     
// si_frds(ra)          
// si_fesd(ra)          
// si_fceq(ra,rb)       
// si_fcmeq(ra,rb)      
// si_fcgt(ra,rb)       
// si_fcmgt(ra,rb)      
// si_stop(imm)         
// si_stopd(ra,rb,rc)   
// si_lnop()            
// si_nop()             
// si_sync()            
// si_syncc()           
// si_dsync()           
// si_mfspr(imm)        
// si_mtspr(imm,ra)     
// si_fscrrd()          
// si_fscrwr(ra)        
// si_rdch(imm)         
// si_rchcnt(imm)       
// si_wrch(imm,ra)      

// si_from_char(scalar)    
// si_from_uchar(scalar)   
// si_from_short(scalar)   
// si_from_ushort(scalar)  
// si_from_int(scalar)     
// si_from_uint(scalar)    
// si_from_llong(scalar)   
// si_from_ullong(scalar)  
// si_from_float(scalar)   
// si_from_double(scalar)  
// si_from_ptr(scalar)     

// si_to_char(ra)      
// si_to_uchar(ra)     
// si_to_short(ra)     
// si_to_ushort(ra)    
// si_to_int(ra)       
// si_to_uint(ra)      
// si_to_llong(ra)     
// si_to_ullong(ra)    
// si_to_float(ra)     
// si_to_double(ra)    
// si_to_ptr(ra)       