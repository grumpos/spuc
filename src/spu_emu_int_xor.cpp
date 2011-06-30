#include "spu_emu.h"

inline GPR_t si_xor( GPR_t RA, GPR_t RB )
{
	return _mm_xor_ps( RA, RB );
}

inline GPR_t si_xorbi( GPR_t RA, int64_t IMM )
{
	return _mm_xor_ps( RA, _mm_castsi128_ps( _mm_set1_epi8((uint8_t)IMM) ) );
}

inline GPR_t si_xorhi( GPR_t RA, int64_t IMM )
{
	return _mm_xor_ps( RA, _mm_castsi128_ps( _mm_set1_epi16((int16_t)IMM) ) );
}

inline GPR_t si_xori( GPR_t RA, int64_t IMM )
{
	return _mm_xor_ps( RA, _mm_castsi128_ps( _mm_set1_epi32((int32_t)IMM) ) );
}


void spu_xor( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_xor( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
}

void spu_xorbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
		_mm_xor_si128( 
			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
			_mm_set1_epi8( 0xff & op.RI10.I10 ) ) );
}

void spu_xorhi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
		_mm_xor_si128( 
			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
			_mm_set1_epi16( static_cast<int16_t>(SignExtend( op.RI10.I10, 10 ) ) ) ) );
}

void spu_xori( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
		_mm_xor_si128( 
			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
			_mm_set1_epi32( static_cast<int32_t>(SignExtend( op.RI10.I10, 10 ) ) ) ) );
}