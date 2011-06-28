#include "spu_emu.h"

void spu_rothm( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 8; ++i )
	{
		const size_t s = 0x1f&( 0 - SPU->GPR[op.RR.RB].m128_i16[i] );
		SPU->GPR[op.RR.RT].m128_u16[i] = s < 16 ? SPU->GPR[op.RR.RA].m128_u16[i] >> s : 0;
	}
}

void spu_rothmi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const int s = 0x1f&( 0 - SignExtend(op.RI7.I7, 7) );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_srli_epi16( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s ) );
}

void spu_rotmah( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 8; ++i )
	{
		const size_t s = 0x1f&( 0 - SPU->GPR[op.RR.RB].m128_i16[i] );
		SPU->GPR[op.RR.RT].m128_i16[i] = s < 16 ? SPU->GPR[op.RR.RA].m128_i16[i] >> s : -1;
	}
}

void spu_rotmahi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const int s = 0x1f&( 0 - SignExtend(op.RI7.I7, 7) );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_srai_epi16( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s ) );
}


void spu_rotm( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 4; ++i )
	{
		const size_t s = 0x3f&( 0 - SPU->GPR[op.RR.RB].m128_i32[i] );
		if ( s < 32 )
		{
			SPU->GPR[op.RR.RT].m128_u32[i] = SPU->GPR[op.RR.RA].m128_u32[i] >> s;
		}
		else
		{
			SPU->GPR[op.RR.RT].m128_u32[i] = 0;
		}
	}
}

void spu_rotmi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const int s = 0x3f&( 0 - SignExtend(op.RI7.I7, 7) );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_srli_epi32( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s ) );
}

void spu_rotma( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 4; ++i )
	{
		const size_t s = 0x3f&( 0 - SPU->GPR[op.RR.RB].m128_i32[i] );
		if ( s < 32 )
		{
			SPU->GPR[op.RR.RT].m128_i32[i] = SPU->GPR[op.RR.RA].m128_i32[i] >> s;
		}
		else
		{
			SPU->GPR[op.RR.RT].m128_i32[i] = SPU->GPR[op.RR.RA].m128_i32[i] >= 0 ? 0 : 0xffffffff;
		}
	}
}

void spu_rotmai( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const int s = 0x3f&( 0 - SignExtend(op.RI7.I7, 7) );
	SPU->GPR[op.RI7.RT] = _mm_castsi128_ps( _mm_srai_epi32( _mm_castps_si128( SPU->GPR[op.RI7.RA] ), s ) );
}

void spu_rotqmbyi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t s = 0x1f&(0 - SignExtend(op.RI7.I7, 7));
	for ( int i = 0; i < 16; ++i )
	{
		if ( 0 == s )
			SPU->GPR[op.RI7.RT].m128_u8[i] = SPU->GPR[op.RI7.RA].m128_u8[i];
		else if ( s > 15 )
			SPU->GPR[op.RI7.RT].m128_u8[i] = 0;
		else
			SPU->GPR[op.RI7.RT].m128_u8[i] = (i + s < 16) ? SPU->GPR[op.RI7.RA].m128_u8[i+s] : 0x00;
	}
}

void spu_rotqmby( SPU_t* SPU, SPU_INSTRUCTION op )
{
	/*const int s = 0x1f&(0 - SPU->GPR[RB].m128_u32[GPR_PREF_SLOT]);
	for ( int i = 0; i < 16; ++i )
	{
		if ( 0 == s )
			SPU->GPR[RT].m128_u8[i] = SPU->GPR[RA].m128_u8[i];
		else if ( s > 15 )
			SPU->GPR[RT].m128_u8[i] = 0;
		else
			SPU->GPR[RT].m128_u8[i] = (i + s < 16) ? SPU->GPR[RA].m128_u8[i+s] : 0x00;
	}*/
	SPU_INSTRUCTION op_RI7;
	op_RI7.RI7.OP = op.RR.OP;
	op_RI7.RI7.RT = op.RR.RT;
	op_RI7.RI7.RA = op.RR.RA;
	op_RI7.RI7.I7 = SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
	spu_rotqmbyi( SPU, op_RI7 );
}

void spu_rotqmbybi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	/*const size_t s = 0x1f&(0 - (SPU->GPR[RB].m128_u32[GPR_PREF_SLOT] >> 3));
	for ( int i = 0; i < 16; ++i )
	{
		if ( 0 == s )
			SPU->GPR[RT].m128_u8[i] = SPU->GPR[RA].m128_u8[i];
		else if ( s > 15 )
			SPU->GPR[RT].m128_u8[i] = 0;
		else
			SPU->GPR[RT].m128_u8[i] = (i + s < 16) ? SPU->GPR[RA].m128_u8[i+s] : 0x00;
	}*/
	SPU_INSTRUCTION op_RI7;
	op_RI7.RI7.OP = op.RR.OP;
	op_RI7.RI7.RT = op.RR.RT;
	op_RI7.RI7.RA = op.RR.RA;
	op_RI7.RI7.I7 = (SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT] >> 3);
	spu_rotqmbyi( SPU, op_RI7 );
}

void spu_rotqmbi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t s = 0x7&(0 - SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]);
	
	uint64_t leftover = SPU->GPR[op.RR.RA].m128_u64[1] ^ (SPU->GPR[op.RR.RA].m128_u64[1] >> s);
	SPU->GPR[op.RR.RT].m128_u64[0] = (SPU->GPR[op.RR.RA].m128_u64[0] >> s) | leftover;
	SPU->GPR[op.RR.RT].m128_u64[1] = SPU->GPR[op.RR.RA].m128_u64[1] >> s;
}

void spu_rotqmbii( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t s = 0x7&(0 - op.RI7.I7);

	uint64_t leftover = SPU->GPR[op.RI7.RA].m128_u64[1] ^ (SPU->GPR[op.RI7.RA].m128_u64[1] >> s);
	SPU->GPR[op.RI7.RT].m128_u64[0] = (SPU->GPR[op.RI7.RA].m128_u64[0] >> s) | leftover;
	SPU->GPR[op.RI7.RT].m128_u64[1] = SPU->GPR[op.RI7.RA].m128_u64[1] >> s;
}