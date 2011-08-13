//#include "spu_emu.h"
//
//#define REP_N(expr, n) \
//	size_t i = 0; \
//	do \
//	{ \
//		expr; \
//	} while ( ++i < n );
//
//#define SPU_INVAR_RR \
//	const uint32_t* a = SPU->GPR[op.RR.RA].m128_u32; \
//	const uint32_t* b = SPU->GPR[op.RR.RB].m128_u32; \
//	uint32_t*		t = SPU->GPR[op.RR.RT].m128_u32;
//
//#define SPU_INVAR_RI7 \
//	const uint32_t* a = SPU->GPR[op.RI7.RA].m128_u32; \
//	const uint32_t	i7 = op.RI7.I7; \
//	uint32_t*		t = SPU->GPR[op.RI7.RT].m128_u32;
//
//#define SPU_INVAR_RR_H \
//	const uint16_t* a = SPU->GPR[op.RR.RA].m128_u16; \
//	const uint16_t* b = SPU->GPR[op.RR.RB].m128_u16; \
//	uint16_t*		t = SPU->GPR[op.RR.RT].m128_u16;
//
//#define SPU_INVAR_RI7_H \
//	const uint16_t* a = SPU->GPR[op.RI7.RA].m128_u16; \
//	const uint32_t	i7 = op.RI7.I7; \
//	uint16_t*		t = SPU->GPR[op.RI7.RT].m128_u16;
//
//
//
//
//void spu_shlqbii( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	const size_t s = 0x7 & op.RI7.I7;
//	if ( 0 == s || s > 7 )
//		SPU->GPR[op.RI7.RT] = SPU->GPR[op.RI7.RA];
//	else
//	{
//		uint64_t leftover = SPU->GPR[op.RI7.RA].m128_u64[1] >> (64 - s);
//		SPU->GPR[op.RI7.RT].m128_u64[1] = (SPU->GPR[op.RI7.RA].m128_u64[1] << s) | leftover;
//		SPU->GPR[op.RI7.RT].m128_u64[0] = (SPU->GPR[op.RI7.RA].m128_u64[0] << s);
//	}
//}
//
//void spu_shlqbi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	op.RI7.I7 = 0x7 & SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
//	spu_shlqbii( SPU, op );
//}
//
//void shift_byte_l( void* d, const size_t l, const size_t c )
//{
//	size_t i = 0;
//	do
//	{
//		((uint8_t*)d)[i] = ((i+c) > (l-1)) ? 0 : ((uint8_t*)d)[i+c];
//	} while ( ++i < l );
//}
//
//#define si_shlqbyi( RA, IMM ) _mm_slli_si128( (RA), (IMM) )
//
//void spu_shlqbyi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	const ptrdiff_t s = 0x1f & op.RI7.I7;
//	ptrdiff_t i = 15;
//	do
//	{
//		SPU->GPR[op.RI7.RT].m128_u8[i] = i-s < 0 ? 0 : SPU->GPR[op.RI7.RA].m128_u8[i-s]; 
//	} while ( --i >= 0 );
//}
//
//void spu_shlqby( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	op.RI7.I7 = 0x1f & SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT];
//	spu_shlqbyi( SPU, op );
//}
//
//void spu_shlqbybi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	op.RI7.I7 = 0x1f & (SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT] >> 3);
//	spu_shlqbyi( SPU, op );
//}
//
//
//
////10111213 14151617 18191A1B 1C1D1E1F