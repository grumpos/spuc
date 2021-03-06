#include "spu_emu.h"

inline GPR_t si_or( GPR_t RA, GPR_t RB )
{
	return _mm_or_ps( RA, RB );
}

inline GPR_t si_orc( GPR_t RA, GPR_t RB )
{
	const __m128 not_RB = _mm_andnot_ps( RB, _mm_castsi128_ps( _mm_set1_epi32(0xffffffff) ) );
	return _mm_or_ps( RA, not_RB );
}

inline GPR_t si_orbi( GPR_t RA, int64_t IMM )
{
	return _mm_or_ps( RA, _mm_castsi128_ps( _mm_set1_epi8( (int8_t)IMM ) ) );
}

inline GPR_t si_orhi( GPR_t RA, int64_t IMM )
{
	return _mm_or_ps( RA, _mm_castsi128_ps( _mm_set1_epi16( (int16_t)IMM ) ) );
}

inline GPR_t si_ori( GPR_t RA, int64_t IMM )
{
	return _mm_or_ps( RA, _mm_castsi128_ps( _mm_set1_epi32( (int32_t)IMM ) ) );
}

inline GPR_t si_orx( GPR_t RA )
{
	const uint32_t shuf_reverse = _MM_SHUFFLE(0,3,2,1);
	const __m128 temp1 = _mm_or_ps( RA, _mm_shuffle_ps( RA, RA, shuf_reverse ) );
	const __m128 temp2 = _mm_or_ps( temp1, _mm_shuffle_ps( temp1, temp1, shuf_reverse ) );
	const __m128 temp3 = _mm_or_ps( temp2, _mm_shuffle_ps( temp2, temp2, shuf_reverse ) );
	return _mm_move_ss( _mm_setzero_ps(), temp3 );
}

inline GPR_t si_nor( GPR_t RA, GPR_t RB )
{
	const __m128 RA_or_RB = _mm_or_ps( RA, RB );
	return _mm_andnot_ps( RA_or_RB, _mm_castsi128_ps( _mm_set1_epi32(0xffffffff) ) );
}