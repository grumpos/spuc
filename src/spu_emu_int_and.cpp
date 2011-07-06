#include "spu_emu.h"

/*

10 bits for immediate value limits it for +511 and -512
byte mask limit: [0xFF...0] 
half mask limit: [0x1FF...0] [0xFE00...0xFFFF]
word mask limit: [0x1FF...0] [0xFFFFFE00...0xFFFFFFFF]
*/

inline GPR_t si_and( GPR_t RA, GPR_t RB )
{
	return _mm_and_ps( RA, RB );
}

inline GPR_t si_andc( GPR_t RA, GPR_t RB )
{
	return _mm_andnot_ps( RB, RA );
}

inline GPR_t si_andbi( GPR_t RA, uint8_t I10 )
{
	return _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( RA ), _mm_set1_epi8( I10 ) ) );
}

inline GPR_t si_andhi( GPR_t RA, int16_t I10 )
{
	return _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( RA ), _mm_set1_epi16( static_cast<int16_t>(SignExtend( I10, 10 ) ) ) ) );
}

inline GPR_t si_andi( GPR_t RA, int16_t I10 )
{
	return _mm_castsi128_ps( _mm_and_si128( _mm_castps_si128( RA ), _mm_set1_epi32( static_cast<int32_t>(SignExtend( I10, 10 ) ) ) ) );
}

inline GPR_t si_nand( GPR_t RA, GPR_t RB )
{
	return _mm_andnot_ps( _mm_and_ps( RA, RB ), _mm_castsi128_ps( _mm_set1_epi32( 0xffffffff ) ) );
}