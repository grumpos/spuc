#include "spu_emu.h"




//void spu_xor( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = si_xor( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_xorbi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
//		_mm_xor_si128( 
//			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
//			_mm_set1_epi8( 0xff & op.RI10.I10 ) ) );
//}
//
//void spu_xorhi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
//		_mm_xor_si128( 
//			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
//			_mm_set1_epi16( static_cast<int16_t>(SignExtend( op.RI10.I10, 10 ) ) ) ) );
//}
//
//void spu_xori( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( 
//		_mm_xor_si128( 
//			_mm_castps_si128( SPU->GPR[op.RI10.RA] ), 
//			_mm_set1_epi32( static_cast<int32_t>(SignExtend( op.RI10.I10, 10 ) ) ) ) );
//}