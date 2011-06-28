#include "spu_emu.h"

#define REP_N(expr, n) \
	size_t i = 0; \
	do \
	{ \
		expr; \
	} while ( ++i < n );

#define SPU_INVAR_RR \
	const uint32_t* a = SPU->GPR[op.RR.RA].m128_u32; \
	const uint32_t* b = SPU->GPR[op.RR.RB].m128_u32; \
	uint32_t*		t = SPU->GPR[op.RR.RT].m128_u32;

#define SPU_INVAR_RI7 \
	const uint32_t* a = SPU->GPR[op.RI7.RA].m128_u32; \
	const uint32_t	i7 = op.RI7.I7; \
	uint32_t*		t = SPU->GPR[op.RI7.RT].m128_u32;

#define SPU_INVAR_RR_H \
	const uint16_t* a = SPU->GPR[op.RR.RA].m128_u16; \
	const uint16_t* b = SPU->GPR[op.RR.RB].m128_u16; \
	uint16_t*		t = SPU->GPR[op.RR.RT].m128_u16;

#define SPU_INVAR_RI7_H \
	const uint16_t* a = SPU->GPR[op.RI7.RA].m128_u16; \
	const uint32_t	i7 = op.RI7.I7; \
	uint16_t*		t = SPU->GPR[op.RI7.RT].m128_u16;

GPR_t si_shlh( GPR_t _RA, GPR_t _RB )
{
	__m128 _RES;

	_RES.m128_u16[0] = _RA.m128_u16[0] << _RB.m128_u16[0];
	_RES.m128_u16[1] = _RA.m128_u16[1] << _RB.m128_u16[1];
	_RES.m128_u16[2] = _RA.m128_u16[2] << _RB.m128_u16[2];
	_RES.m128_u16[3] = _RA.m128_u16[3] << _RB.m128_u16[3];
	_RES.m128_u16[4] = _RA.m128_u16[4] << _RB.m128_u16[4];
	_RES.m128_u16[5] = _RA.m128_u16[5] << _RB.m128_u16[5];
	_RES.m128_u16[6] = _RA.m128_u16[6] << _RB.m128_u16[6];
	_RES.m128_u16[7] = _RA.m128_u16[7] << _RB.m128_u16[7];

	return _RES;
}

void spu_shlh( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_shlh( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );

	//REP_N( t[i] = a[i] << b[i], 8 );

	/*uint16_t temp[8];
	__m128i _RA = _mm_castps_si128( SPU->GPR[op.RR.RA] );
	__m128i _RB = _mm_castps_si128( SPU->GPR[op.RR.RB] );
	__m128i _RES;

	_RES.m128i_u16[0] = _RA.m128i_u16[0] << _RB.m128i_u16[0];
	_RES.m128i_u16[1] = _RA.m128i_u16[1] << _RB.m128i_u16[1];
	_RES.m128i_u16[2] = _RA.m128i_u16[2] << _RB.m128i_u16[2];
	_RES.m128i_u16[3] = _RA.m128i_u16[3] << _RB.m128i_u16[3];
	_RES.m128i_u16[4] = _RA.m128i_u16[4] << _RB.m128i_u16[4];
	_RES.m128i_u16[5] = _RA.m128i_u16[5] << _RB.m128i_u16[5];
	_RES.m128i_u16[6] = _RA.m128i_u16[6] << _RB.m128i_u16[6];
	_RES.m128i_u16[7] = _RA.m128i_u16[7] << _RB.m128i_u16[7];

	SPU->GPR[op.RR.RT] = _mm_castsi128_ps( _RES );*/
/*
// 	size_t i = 0;
// 	do
// 	{
// 		const size_t s = 0x1f & SPU->GPR[op.RR.RB].m128_u16[i];
// 		SPU->GPR[op.RR.RT].m128_u16[i] = s > 15 ? 0 : (SPU->GPR[op.RR.RA].m128_u16[i] << s);
// 	} while ( ++i < 8 );
*/



}

void spu_shlhi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU_INVAR_RI7_H;

	REP_N( t[i] = a[i] << i7, 8 );

	/*const int s = 0x1f & SignExtend( op.RI7.I7, 7 );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_slli_epi16( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s ) );*/
}

void spu_shl( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU_INVAR_RR;

	REP_N( t[i] = a[i] << b[i], 4 );

	/*size_t i = 0;
	do
	{
		const size_t s = 0x3f & SPU->GPR[op.RR.RB].m128_u32[i];
		SPU->GPR[op.RR.RT].m128_u32[i] = s > 31 ? 0 : (SPU->GPR[op.RR.RA].m128_u32[i] << s);
	} while ( ++i < 4 );*/
}

void spu_shli( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU_INVAR_RI7;

	REP_N( t[i] = a[i] << i7, 4 );

	/*const int s = 0x3f & SignExtend( op.RI7.I7, 7 );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_slli_epi32( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s ) );*/
}


void spu_shlqbii( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t s = 0x7 & op.RI7.I7;
	if ( 0 == s || s > 7 )
		SPU->GPR[op.RI7.RT] = SPU->GPR[op.RI7.RA];
	else
	{
		uint64_t leftover = SPU->GPR[op.RI7.RA].m128_u64[1] >> (64 - s);
		SPU->GPR[op.RI7.RT].m128_u64[1] = (SPU->GPR[op.RI7.RA].m128_u64[1] << s) | leftover;
		SPU->GPR[op.RI7.RT].m128_u64[0] = (SPU->GPR[op.RI7.RA].m128_u64[0] << s);
	}
}

void spu_shlqbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	op.RI7.I7 = 0x7 & SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
	spu_shlqbii( SPU, op );
}

void shift_byte_l( void* d, const size_t l, const size_t c )
{
	size_t i = 0;
	do
	{
		((uint8_t*)d)[i] = ((i+c) > (l-1)) ? 0 : ((uint8_t*)d)[i+c];
	} while ( ++i < l );
}

#define si_shlqbyi( RA, IMM ) _mm_slli_si128( (RA), (IMM) )

void spu_shlqbyi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const ptrdiff_t s = 0x1f & op.RI7.I7;
	ptrdiff_t i = 15;
	do
	{
		SPU->GPR[op.RI7.RT].m128_u8[i] = i-s < 0 ? 0 : SPU->GPR[op.RI7.RA].m128_u8[i-s]; 
	} while ( --i >= 0 );
}

void spu_shlqby( SPU_t* SPU, SPU_INSTRUCTION op )
{
	op.RI7.I7 = 0x1f & SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
	spu_shlqbyi( SPU, op );
}

void spu_shlqbybi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	op.RI7.I7 = 0x1f & (SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT] >> 3);
	spu_shlqbyi( SPU, op );
}



//10111213 14151617 18191A1B 1C1D1E1F