#include "spu_emu.h"
#include "spu_internals_x86.h"
//#include <string>
//#include <sstream>
// Constant-Formation Instructions

/*
inline GPR_t si_ilh( const int16_t I16 )
{
	return _mm_castsi128_ps( _mm_set1_epi16( I16 ) );
}
inline GPR_t si_ilhu( const int16_t I16 )
{
	return _mm_castsi128_ps( _mm_set1_epi32( (int32_t)I16 << 16 ) );
}
inline GPR_t si_il( const int16_t I16 )
{
	return _mm_castsi128_ps( _mm_set1_epi32( (int32_t)I16 ) );
}
inline GPR_t si_ila( const int32_t I18 )
{
	return _mm_castsi128_ps( _mm_set1_epi32( I18 ) );
}
inline GPR_t si_iohl( GPR_t RA, const int16_t I16 )
{
	return _mm_castsi128_ps( _mm_or_si128( _mm_castps_si128( RA ), _mm_set1_epi32( I16 ) ) );
}
inline GPR_t si_fsmbi( const int16_t I16 )
{
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
*/
void spu_ilh( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = si_ilh( op.RI16.I16 );
}

void spu_ilhu( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = si_ilhu( op.RI16.I16 );
}

void spu_il( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = si_il( op.RI16.I16 );
}

void spu_ila( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI18.RT] = si_ila( op.RI18.I18 );
}

void spu_iohl( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = si_iohl( SPU->GPR[op.RI16.RT], op.RI16.I16 );
}

void spu_fsmbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = si_fsmbi( op.RI16.I16 );
}