#include "spu_emu.h"





#define SPU_LGT_AUX( steps, field, lhs, rhs, res ) \
	for(ptrdiff_t i = 0; i < (steps); ++i) \
	{ \
		SPU->GPR[op.RR.RT].field[i] = lhs.field[i] > rhs.field[i] ? (res) : 0; \
	}
/*
#define SPU_LGT_AUX( REGA, REGB, REGT, TYPE, RES ) \
	const TYPE* a = &REGA; \
	const TYPE* b = &REGB; \
		  TYPE* t = &REGT; \
	for(ptrdiff_t i = 0; i < (sizeof(REGA) / sizeof(TYPE)); ++i) \
	{ \
		*t = *a > *b ? (RES) : 0; \
	}
	*/
#define SPU_LGTI_AUX( steps, field, lhs, rhs, res ) \
	for(ptrdiff_t i = 0; i < (steps); ++i) \
	{ \
		SPU->GPR[op.RI10.RT].field[i] = lhs.field[i] > rhs ? (res) : 0; \
	}

void spu_clgtb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	//SPU_LGT_AUX( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB], SPU->GPR[op.RR.RT], uint8_t, 0xff );
	SPU_LGT_AUX( 16, m128_u8, SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB], 0xff );
	
	//SPU->GPR[RT] = _mm_cmpgt_ps( _mm_and_ps( SPU->GPR[RA], _mm_set1_ps(0xefffffff) ) , SPU->GPR[RB] );
}

void spu_clgtbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	//SPU->GPR[RT] = _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( SPU->GPR[RA] ), _mm_set1_epi8( op.RI10.I10 ) ) );
	SPU_LGTI_AUX( 16, m128_u8, SPU->GPR[op.RI10.RA], op.RI10.I10, 0xff );
}

void spu_clgth( SPU_t* SPU, SPU_INSTRUCTION op )
{
	//SPU->GPR[RT] = _mm_castsi128_ps( _mm_cmplgt_epi16( _mm_castps_si128( SPU->GPR[RA] ), _mm_castps_si128( SPU->GPR[RB] ) ) );
	SPU_LGT_AUX( 8, m128_u16, SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB], 0xffff );
}

void spu_clgthi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	//SPU->GPR[RT] = _mm_castsi128_ps( _mm_cmplgt_epi16( _mm_castps_si128( SPU->GPR[RA] ), _mm_set1_epi16( SignExtend(op.RI10.I10, 10) ) ) );
	SPU_LGTI_AUX( 8, m128_u16, SPU->GPR[op.RI10.RA], (uint16_t)SignExtend(op.RI10.I10, 10), 0xffff );
}

void spu_clgt( SPU_t* SPU, SPU_INSTRUCTION op )
{
	//SPU->GPR[RT] = _mm_castsi128_ps( _mm_cmplgt_epi32( _mm_castps_si128( SPU->GPR[RA] ), _mm_castps_si128( SPU->GPR[RB] ) ) );
	SPU_LGT_AUX( 4, m128_u32, SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB], 0xffffffff );
}

void spu_clgti( SPU_t* SPU, SPU_INSTRUCTION op )
{
	//SPU->GPR[RT] = _mm_castsi128_ps( _mm_cmplgt_epi32( _mm_castps_si128( SPU->GPR[RA] ), _mm_set1_epi32( SignExtend(op.RI10.I10, 10) ) ) );
	SPU_LGTI_AUX( 4, m128_u32, SPU->GPR[op.RI10.RA], (uint32_t)SignExtend(op.RI10.I10, 10), 0xffffffff );
}

#undef SPU_LGT_AUX
#undef SPU_LGTI_AUX