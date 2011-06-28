#include "spu_emu.h"

void spu_roth( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 8; ++i )
	{
		SPU->GPR[op.RR.RT].m128_u16[i] = _rotl16( SPU->GPR[op.RR.RA].m128_u16[i], 0xf&SPU->GPR[op.RR.RB].m128_u16[i] );
	}
}

void spu_rothi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const int s = 0xf&op.RI7.I7;
	__m128i lo = _mm_slli_epi16( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s );
	__m128i hi = _mm_srli_epi16( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), 16 - s );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_or_si128( hi, lo ) );
}

void spu_rot( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 4; ++i )
	{
		SPU->GPR[op.RR.RT].m128_u32[i] = _rotl( SPU->GPR[op.RR.RA].m128_u32[i], 0x1f&SPU->GPR[op.RR.RB].m128_u32[i] );
	}
}

void spu_roti( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const int s = 0xf&op.RI7.I7;
	__m128i lo = _mm_slli_epi32( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s );
	__m128i hi = _mm_srli_epi32( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), 32 - s );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_or_si128( hi, lo ) );
}



void spu_rotqbii( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t s = 0x7 & op.RI7.I7;
	
	const uint64_t _0 = SPU->GPR[op.RI7.RA].m128_u64[0];
	const uint64_t _1 = SPU->GPR[op.RI7.RA].m128_u64[1];
	const uint64_t leftover0 = _0 >> (64 - s);
	const uint64_t leftover1 = _1 >> (64 - s);

	SPU->GPR[op.RI7.RT].m128_u64[0] = (_0 << s) | leftover1;
	SPU->GPR[op.RI7.RT].m128_u64[1] = (_1 << s) | leftover0;
}

void spu_rotqbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU_INSTRUCTION op_RI7;
	op_RI7.RI7.OP = op.RR.OP;
	op_RI7.RI7.RT = op.RR.RT;
	op_RI7.RI7.RA = op.RR.RA;
	op_RI7.RI7.I7 = 0x7&SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
	spu_rotqbii( SPU, op_RI7 );
}

void spu_rotqbyi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	/*const size_t s = 0xf & op.RI7.I7;
	int i = 0;
	do
	{
		const uint8_t raw = SPU->GPR[op.RI7.RA].m128_u8[i];
		const uint8_t shifted = SPU->GPR[op.RI7.RA].m128_u8[(i - s) % 16];
		SPU->GPR[op.RI7.RT].m128_u8[i] = s <= 15 ? shifted : raw;
	} while ( ++i < 16 );*/
	for ( size_t s = 0xF & op.RI7.I7, i = 0; i < 16; ++i )
	{
		const uint8_t raw = SPU->GPR[op.RI7.RA].m128_u8[i];
		const uint8_t shifted = SPU->GPR[op.RI7.RA].m128_u8[(i - s) % 16];
		SPU->GPR[op.RI7.RT].m128_u8[i] = s <= 15 ? shifted : raw;
	}
}

void spu_rotqby( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU_INSTRUCTION op_RI7;
	op_RI7.RI7.OP = op.RR.OP;
	op_RI7.RI7.RT = op.RR.RT;
	op_RI7.RI7.RA = op.RR.RA;
	op_RI7.RI7.I7 = 0xf&SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
	spu_rotqbyi( SPU, op_RI7 );
}

void spu_rotqbybi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU_INSTRUCTION op_RI7;
	op_RI7.RI7.OP = op.RR.OP;
	op_RI7.RI7.RT = op.RR.RT;
	op_RI7.RI7.RA = op.RR.RA;
	op_RI7.RI7.I7 = 0xf&(SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT] >> 3);
	spu_rotqbyi( SPU, op_RI7 );
}