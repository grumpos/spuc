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



//
//void spu_and( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = si_and( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_andc( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_andnot_ps( SPU->GPR[op.RR.RB], SPU->GPR[op.RR.RA] );
//}
//
//void spu_andbi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
//		_mm_and_si128( 
//			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
//			_mm_set1_epi8( 0xff & op.RI10.I10 ) ) );
//}
//
//void spu_andhi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
//		_mm_and_si128( 
//			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
//			_mm_set1_epi16( static_cast<int16_t>(SignExtend( op.RI10.I10, 10 ) ) ) ) );
//}
//
//void spu_andi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
//		_mm_and_si128( 
//			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
//			_mm_set1_epi32( static_cast<int32_t>(SignExtend( op.RI10.I10, 10 ) ) ) ) );
//}
//
//void spu_nand( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_andnot_ps( 
//		_mm_and_ps( SPU->GPR[op.RR.RB], SPU->GPR[op.RR.RA] ), 
//		_mm_castsi128_ps( _mm_set1_epi32( 0xffffffff ) ) );
//}