#include "spu_emu.h"

//#include "spu_unittest.h"







/* SPU impl */


static const __m128i mask_all_set8 = _mm_setr_epi8( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80i8,
	0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80i8 );
static const __m128i mask_all_set16 = _mm_setr_epi16( 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 );
static const __m128i mask_all_set32 = _mm_setr_epi32( 0x1, 0x2, 0x4, 0x8 );

void spu_fsmb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	uint16_t word_mask = 0xffff & SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT];

	__m128i mask_lo_byte_sat = _mm_srli_si128( _mm_set1_epi8( word_mask & 0xff ), 8 );
	__m128i mask_hi_byte_sat = _mm_slli_si128( _mm_set1_epi8( (word_mask >> 8) & 0xff ), 8 );
	mask_lo_byte_sat = _mm_and_si128( mask_lo_byte_sat, mask_all_set8 );
	mask_hi_byte_sat = _mm_and_si128( mask_hi_byte_sat, mask_all_set8 );
	__m128i mask5 = _mm_or_si128( mask_lo_byte_sat, mask_hi_byte_sat );
	SPU->GPR[op.RI16.RT] = _mm_castsi128_ps( _mm_cmpeq_epi8( mask_all_set8, mask5 ) );
}

void spu_fsmh( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = _mm_castsi128_ps( 
		_mm_cmpeq_epi16( 
			mask_all_set16, 
			_mm_and_si128( _mm_set1_epi16( 0xff & SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] ), mask_all_set16 ) ) );
}

void spu_fsm( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RI16.RT] = _mm_castsi128_ps( 
		_mm_cmpeq_epi32( 
			mask_all_set32, 
			_mm_and_si128( _mm_set1_epi32( 0xf & SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] ), mask_all_set32 ) ) );
}

void spu_gbb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	uint32_t bits = 0;

	for ( int i = 0; i < 16; ++i )
		bits |= (SPU->GPR[op.RR.RA].m128_u8[i] & 0x1) << (i);
	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = bits;
}

void spu_gbh( SPU_t* SPU, SPU_INSTRUCTION op )
{
	uint32_t bits = 0;

	for ( int i = 0; i < 8; ++i )
		bits |= (SPU->GPR[op.RR.RA].m128_u16[i] & 0x1) << (i);
	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = bits;
}

void spu_gb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	uint32_t bits = 0;

	for ( int i = 0; i < 4; ++i )
		bits |= (SPU->GPR[op.RR.RA].m128_u32[i] & 0x1) << (i);
	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = bits;
}

void spu_avgb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	for ( int i = 0; i < 16; ++i )
		SPU->GPR[op.RR.RT].m128_u8[i] = ((uint16_t)SPU->GPR[op.RR.RA].m128_u8[i] + (uint16_t)SPU->GPR[op.RR.RB].m128_u8[i] + 1) / 2;
}

void spu_absdb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	/*const uint8_t* RA = SPU->GPR[op.RR.RA].m128_u8;
	const uint8_t* RB = SPU->GPR[op.RR.RB].m128_u8;
	uint8_t* RT = SPU->GPR[op.RR.RT].m128_u8;

	for ( int i = 0; i < 16; ++i )
	{
		RT[i] = RA[i] > RB[i] ? (RA[i] - RB[i]) : (RB[i] - RA[i]);
	}*/
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

GPR_t si_xsbh( GPR_t RA )
{
	return _mm_castsi128_ps(_mm_srai_epi16( _mm_slli_epi16( _mm_castps_si128(RA), 8 ), 8 ));
}

GPR_t si_xshw( GPR_t RA )
{
	return _mm_castsi128_ps(_mm_srai_epi32( _mm_slli_epi32( _mm_castps_si128(RA), 16 ), 16 ));
}

void spu_xsbh( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_xsbh( SPU->GPR[op.RR.RA] );
}

void spu_xshw( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_xshw( SPU->GPR[op.RR.RA] );
}

GPR_t si_xswd( GPR_t RA )
{
	__m128i RA_unsigned			= _mm_and_si128( _mm_castps_si128( RA ), _mm_set_epi32( 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 ) );
	__m128i RA_has_sign_bit		= _mm_and_si128( _mm_castps_si128( RA ), _mm_set_epi32( 0x80000000, 0, 0x80000000, 0 ) );
	__m128i RA_sign_extension	= _mm_and_si128( RA_has_sign_bit, _mm_set_epi32( 0, 0xFFFFFFFF, 0, 0xFFFFFFFF ) );
	return _mm_castsi128_ps( _mm_or_si128( RA_unsigned, RA_sign_extension ) );
}

void spu_xswd( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = si_xswd( SPU->GPR[op.RR.RA] );
}

void spu_eqv( SPU_t* SPU, SPU_INSTRUCTION op )
{
	SPU->GPR[op.RR.RT] = _mm_castsi128_ps( _mm_and_si128( _mm_cmpeq_epi32( _mm_castps_si128( SPU->GPR[op.RR.RA] ), _mm_castps_si128( SPU->GPR[op.RR.RB] ) ), _mm_set1_epi32( 1 ) ) );
}

//void spu_selb( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	//SPU->GPR[op.RRR.RT] = si_selb( SPU->GPR[op.RRR.RA], SPU->GPR[op.RRR.RB], SPU->GPR[op.RRR.RC] );
//}

void spu_shufb( SPU_t* SPU, SPU_INSTRUCTION op )
{
	_CRT_ALIGN(16) uint8_t RA_RB[32];
	*(uint64_t*)(RA_RB + 0) = SPU->GPR[op.RRR.RA].m128_i64[0];
	*(uint64_t*)(RA_RB + 8) = SPU->GPR[op.RRR.RA].m128_i64[1];
	*(uint64_t*)(RA_RB + 16) = SPU->GPR[op.RRR.RB].m128_i64[0];
	*(uint64_t*)(RA_RB + 24) = SPU->GPR[op.RRR.RB].m128_i64[1];

	for( int i = 0; i < 16; ++i )
	{
		if ( (0xE & SPU->GPR[op.RRR.RC].m128_u8[i]) == 0xE )
			SPU->GPR[op.RRR.RT].m128_u8[i] = 0x80;
		else if ( (0xE & SPU->GPR[op.RRR.RC].m128_u8[i]) == 0xC )
			SPU->GPR[op.RRR.RT].m128_u8[i] = 0xFF;
		else if ( (0xE & SPU->GPR[op.RRR.RC].m128_u8[i]) == 0x80 )
			SPU->GPR[op.RRR.RT].m128_u8[i] = 0x00;
		else
			SPU->GPR[op.RRR.RT].m128_u8[i] = RA_RB[ 0x1F & SPU->GPR[op.RRR.RC].m128_u8[i] ];
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