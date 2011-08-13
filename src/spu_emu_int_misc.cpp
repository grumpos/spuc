#include "spu_emu.h"



//void spu_gbb( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint32_t bits = 0;
//
//	for ( int i = 0; i < 16; ++i )
//		bits |= (SPU->GPR[op.RR.RA].m128_u8[i] & 0x1) << (i);
//	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
//	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = bits;
//}
//
//void spu_gbh( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint32_t bits = 0;
//
//	for ( int i = 0; i < 8; ++i )
//		bits |= (SPU->GPR[op.RR.RA].m128_u16[i] & 0x1) << (i);
//	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
//	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = bits;
//}
//
//void spu_gb( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint32_t bits = 0;
//
//	for ( int i = 0; i < 4; ++i )
//		bits |= (SPU->GPR[op.RR.RA].m128_u32[i] & 0x1) << (i);
//	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
//	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = bits;
//}

void spu_avgb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 16; ++i )
		SPU->GPR[op.RR.RT].m128_u8[i] = ((uint16_t)SPU->GPR[op.RR.RA].m128_u8[i] + (uint16_t)SPU->GPR[op.RR.RB].m128_u8[i] + 1) / 2;
}

void spu_absdb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const __m128i RA_sub_RB = _mm_sub_epi8( _mm_castps_si128( SPU->GPR[op.RR.RA] ), _mm_castps_si128( SPU->GPR[op.RR.RB] ) );
	const __m128i RB_sub_RA = _mm_sub_epi8( _mm_castps_si128( SPU->GPR[op.RR.RB] ), _mm_castps_si128( SPU->GPR[op.RR.RA] ) );
	const __m128i RA_gt_RB_mask = _mm_cmpgt_epi8( _mm_castps_si128( SPU->GPR[op.RR.RA] ), _mm_castps_si128( SPU->GPR[op.RR.RB] ) );
	const __m128i RB_gt_RA_mask = _mm_cmpgt_epi8( _mm_castps_si128( SPU->GPR[op.RR.RB] ), _mm_castps_si128( SPU->GPR[op.RR.RA] ) );
	SPU->GPR[op.RR.RT] = _mm_castsi128_ps(
		_mm_or_si128( _mm_and_si128( RA_gt_RB_mask, RA_sub_RB ), _mm_and_si128( RB_gt_RA_mask, RB_sub_RA ) ) );
}

void spu_sumb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	const uint8_t* a8 = SPU->GPR[op.RR.RA].m128_u8;
	const uint8_t* b8 = SPU->GPR[op.RR.RB].m128_u8;
	uint32_t* t32 = SPU->GPR[op.RR.RT].m128_u32;

	for ( int i = 0; i < 16; i += 4 )
	{
		uint32_t sum_a = 0;
		sum_a += a8[i+0];
		sum_a += a8[i+1];
		sum_a += a8[i+2];
		sum_a += a8[i+3];
		uint32_t sum_b = 0;
		sum_b += b8[i+0];
		sum_b += b8[i+1];
		sum_b += b8[i+2];
		sum_b += b8[i+3];
		t32[i/4] = (sum_b << 16) | sum_a;
	}
}
/*
std::string shufbDetailer( __m128i GPR )
{
	std::ostringstream oss;
	oss << "\n\n";

	for( int i = 15; i >= 0; --i )
	{
		uint8_t b = GPR.m128i_u8[i];
		if		( b >> 5 == 7 )
			oss << "0x80 ";
		else if ( b >> 5 == 6 )
			oss << "0xFF ";
		else if ( b >> 6 == 2 )
			oss << "0x00 ";
		else
			oss << "$" << (0x1F & GPR.m128i_u8[i]) << " ";
	}

	return oss.str();
}*/