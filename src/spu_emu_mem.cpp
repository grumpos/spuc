#include "spu_emu.h"
//#include <cassert>
#include <string>
#include <sstream>
#include <algorithm>

#if 1 // reverse byte order when moving qwords between GPRS and local storage. LS: big endian, GPR: little endian

#define COPY_LSA_TO_GPR		SPU->GPR[_RT] = _mm_loadr_ps( (const float*)&SPU->LocalStorage[LSA] );
#define COPY_GPR_TO_LSA		_mm_storer_ps( (float*)&SPU->LocalStorage[LSA], SPU->GPR[_RT] );

#else

#define COPY_LSA_TO_GPR		SPU->GPR[_RT] = _mm_load_ps( (const float*)&SPU->LocalStorage[LSA] );
#define COPY_GPR_TO_LSA		_mm_store_ps( (float*)&SPU->LocalStorage[LSA], SPU->GPR[_RT] );

#endif


void spu_lqd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA = ( SPU->GPR[op.RI10.RA].m128_u32[GPR_PREF_SLOT] + (uint32_t)SignExtend( op.RI10.I10 << 4, 14 )) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RI10.RT;
	if ( SPU->LocalStorage )
	{
		COPY_LSA_TO_GPR;
	}
}

void spu_lqx( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  ( SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] + SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RR.RT;
	if ( SPU->LocalStorage )
	{
		COPY_LSA_TO_GPR;
	}
}

void spu_lqa( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  (uint32_t)SignExtend( op.RI16.I16 << 2, 18 ) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RI16.RT;
	if ( SPU->LocalStorage )
	{
		COPY_LSA_TO_GPR;
	}
}

void spu_lqr( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  ((uint32_t)SignExtend( op.RI16.I16 << 2, 18 ) + SPU->IP) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RI16.RT;
	if ( SPU->LocalStorage )
	{
		COPY_LSA_TO_GPR;
	}
}

void spu_stqd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  ( SPU->GPR[op.RI10.RA].m128_u32[GPR_PREF_SLOT] + (uint32_t)SignExtend( op.RI10.I10 << 4, 14 )) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RI10.RT;
	if ( SPU->LocalStorage )
	{
		COPY_GPR_TO_LSA;
	}
}

void spu_stqx( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  ( SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] + SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RR.RT;
	if ( SPU->LocalStorage )
	{
		COPY_GPR_TO_LSA;
	}
}

void spu_stqa( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  (uint32_t)SignExtend( op.RI16.I16 << 2, 18 ) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RI16.RT;
	if ( SPU->LocalStorage )
	{
		COPY_GPR_TO_LSA;
	}
}

void spu_stqr( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t LSA =  ((uint32_t)SignExtend( op.RI16.I16 << 2, 18 ) + SPU->IP) & SPU_t::SPU_LSLR & (uint32_t)(-16);
	const size_t _RT = op.RI16.RT;
	if ( SPU->LocalStorage )
	{
		COPY_GPR_TO_LSA;
	}
}

void spu_cbd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RI7.RA].m128_u32[GPR_PREF_SLOT] + (uint32_t)SignExtend(op.RI7.I7, 7)) & 0xF;

	SPU->GPR[op.RI7.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RI7.RT].m128_u64[1] = 0x1011121314151617;

	SPU->GPR[op.RI7.RT].m128_u8[0xF-t] = 0x03;
}

void spu_cbx( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] + SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]) & 0xF;

	SPU->GPR[op.RR.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RR.RT].m128_u64[1] = 0x1011121314151617;

	SPU->GPR[op.RR.RT].m128_u8[0xF-t] = 0x03;
}

void spu_chd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RI7.RA].m128_u32[GPR_PREF_SLOT] + (uint32_t)SignExtend(op.RI7.I7, 7)) & 0xE;

	SPU->GPR[op.RI7.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RI7.RT].m128_u64[1] = 0x1011121314151617;

	*(uint16_t*)(SPU->GPR[op.RI7.RT].m128_u8 + 0xE - t) = 0x0203;
}

void spu_chx( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] + SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]) & 0xE;

	SPU->GPR[op.RR.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RR.RT].m128_u64[1] = 0x1011121314151617;

	*(uint16_t*)(SPU->GPR[op.RR.RT].m128_u8 + 0xE - t) = 0x0203;
}
/*
GPR_t si_cwd( GPR_t RA, uint64_t IMM )
{
	const size_t t = ((RA.m128_u32[GPR_PREF_SLOT] + IMM) & 0xC) >> 2;

	__m128i ctrl = _mm_set_epi8( 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F );

	ctrl.m128i_u8[t] = 0x00010203;

	return _mm_castsi128_ps(ctrl);
}*/

/*
__m128 si_cwd( __m128 RA, int IMM )
{
	const __m128i ctrl = _mm_set_epi8( 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F );
	const __m128i insrt = _mm_set1_epi32( 0x00010203 );
	const __m128i mask = _mm_set_epi32( -1, 0, 0, 0 );
	const __m128i mask_aligned = _mm_slli_si128( mask, 4 );
	const __m128i mask_aligned_invert = _mm_andnot_si128( mask_aligned, _mm_set1_epi32(-1) );
	const __m128i control_keep = _mm_and_si128( ctrl, mask_aligned_invert );
	const __m128i control_insert = _mm_and_si128( insrt, mask_aligned );

	return _mm_castsi128_ps( _mm_or_si128( control_keep, control_insert ) );
}
*/

void spu_cwd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RI7.RA].m128_u32[GPR_PREF_SLOT] + (uint32_t)SignExtend(op.RI7.I7, 7)) & 0xC;

	SPU->GPR[op.RI7.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RI7.RT].m128_u64[1] = 0x1011121314151617;

	*(uint32_t*)(SPU->GPR[op.RI7.RT].m128_u8 + 0xC - t) = 0x00010203;
}

void spu_cwx( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] + SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]) & 0xC;

	SPU->GPR[op.RR.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RR.RT].m128_u64[1] = 0x1011121314151617;

	*(uint32_t*)(SPU->GPR[op.RR.RT].m128_u8 + 0xC - t) = 0x00010203;
}

void spu_cdd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RI7.RA].m128_u32[GPR_PREF_SLOT] + (uint32_t)SignExtend(op.RI7.I7, 7)) & 0x8;

	SPU->GPR[op.RI7.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RI7.RT].m128_u64[1] = 0x1011121314151617;

	*(uint64_t*)(SPU->GPR[op.RI7.RT].m128_u8 + 0x8 - t) = 0x0001020304050607;
}

void spu_cdx( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const size_t t = (SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] + SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT]) & 0x8;

	SPU->GPR[op.RR.RT].m128_u64[0] = 0x18191A1B1C1D1E1F;
	SPU->GPR[op.RR.RT].m128_u64[1] = 0x1011121314151617;

	*(uint64_t*)(SPU->GPR[op.RR.RT].m128_u8 + 0x8 - t) = 0x0001020304050607;
}