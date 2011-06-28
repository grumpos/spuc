#include "spu_emu.h"

GPR_t si_ceqb( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi8( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

GPR_t si_ceqh( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi16( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

GPR_t si_ceq( GPR_t RA, GPR_t RB )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_castps_si128( RB ) ) );
}

GPR_t si_ceqbi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi8( _mm_castps_si128( RA ), _mm_set1_epi8( (int8_t)IMM ) ) );
}

GPR_t si_ceqhi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi16( _mm_castps_si128( RA ), _mm_set1_epi16( (int16_t)IMM ) ) );
}

GPR_t si_ceqi( GPR_t RA, int16_t IMM )
{
	return _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_castps_si128( RA ), _mm_set1_epi32( (int32_t)IMM ) ) );
}




void spu_ceqb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_ceqb( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
}

void spu_ceqbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = 	si_ceqbi( SPU->GPR[op.RR.RA], (int16_t)SignExtend(op.RI10.I10, 10) );
}

void spu_ceqh( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_ceqh( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
}

void spu_ceqhi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = 	si_ceqhi( SPU->GPR[op.RR.RA], (int16_t)SignExtend(op.RI10.I10, 10) );
}

void spu_ceq( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_ceq( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
}

void spu_ceqi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = 	si_ceqi( SPU->GPR[op.RR.RA], (int16_t)SignExtend(op.RI10.I10, 10) );
}

void spu_cgtb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( SPU->GPR[op.RR.RA] ), _mm_castps_si128( SPU->GPR[op.RR.RB] ) ) );
}

void spu_cgtbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_castps_si128( SPU->GPR[op.RI10.RA] ), _mm_set1_epi8( (int8_t)op.RI10.I10 ) ) );
}

void spu_cgth( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_castps_si128( SPU->GPR[op.RR.RA] ), _mm_castps_si128( SPU->GPR[op.RR.RB] ) ) );
}

void spu_cgthi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_castps_si128( SPU->GPR[op.RI10.RA] ), _mm_set1_epi16( (int16_t)SignExtend(op.RI10.I10, 10) ) ) );
}

void spu_cgt( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = _mm_castsi128_ps( _mm_cmpgt_epi32( _mm_castps_si128( SPU->GPR[op.RR.RA] ), _mm_castps_si128( SPU->GPR[op.RR.RB] ) ) );
}

void spu_cgti( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI10.RT] = _mm_castsi128_ps( _mm_cmpgt_epi32( _mm_castps_si128( SPU->GPR[op.RI10.RA] ), _mm_set1_epi32( (int32_t)SignExtend(op.RI10.I10, 10) ) ) );
}

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