#include "spu_emu.h"

//void spu_fa( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_add_ps( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_fs( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_sub_ps( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_fm( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_mul_ps( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_fma( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RRR.RT] = _mm_add_ps( _mm_mul_ps( SPU->GPR[op.RRR.RA], SPU->GPR[op.RRR.RB] ), SPU->GPR[op.RRR.RC] );
//}
//
//// RA*RB-RC
//void spu_fms( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RRR.RT] = _mm_sub_ps( _mm_mul_ps( SPU->GPR[op.RRR.RA], SPU->GPR[op.RRR.RB] ), SPU->GPR[op.RRR.RC] );
//}
//
//// RC-RA*RB -> RC-RA*RB == -(RA*RB-RC)
//void spu_fnms( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RRR.RT] = _mm_sub_ps( SPU->GPR[op.RRR.RC], _mm_mul_ps( SPU->GPR[op.RRR.RA], SPU->GPR[op.RRR.RB] ) );
//}
//
//void spu_fceq( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_cmpeq_ps( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//GPR_t si_fcmeq( GPR_t RA, GPR_t RB )
//{
//	__m128 ra_abs = _mm_and_ps( RA, _mm_castsi128_ps( _mm_set1_epi32( 0xEFFFFFFF ) ) );
//	__m128 rb_abs = _mm_and_ps( RB, _mm_castsi128_ps( _mm_set1_epi32( 0xEFFFFFFF ) ) );
//	return _mm_cmpeq_ps( ra_abs, rb_abs );
//}
//
//void spu_fcmeq( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = si_fcmeq( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_fcgt( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_cmpgt_ps( SPU->GPR[op.RR.RA], SPU->GPR[op.RR.RB] );
//}
//
//void spu_fcmgt( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	__m128 ra_abs = _mm_castsi128_ps(_mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128(SPU->GPR[op.RR.RA]), 1 ), 1 ));
//	__m128 rb_abs = _mm_castsi128_ps(_mm_srli_epi32( _mm_slli_epi32( _mm_castps_si128(SPU->GPR[op.RR.RB]), 1 ), 1 ));
//	SPU->GPR[op.RR.RT] = _mm_cmpgt_ps( ra_abs, rb_abs );
//
//}
//
//void spu_dfceq( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_castpd_ps( _mm_cmpeq_pd( 
//		_mm_castps_pd( SPU->GPR[op.RR.RA] ), 
//		_mm_castps_pd( SPU->GPR[op.RR.RB] ) ) );
//}
//
//void spu_dfcmeq( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	__m128d absmask = { (double)0xEFFFFFFFFFFFFFFF, (double)0xEFFFFFFFFFFFFFFF };
//	__m128d ra_abs = _mm_and_pd( _mm_castps_pd( SPU->GPR[op.RR.RA] ), absmask );
//	__m128d rb_abs = _mm_and_pd( _mm_castps_pd( SPU->GPR[op.RR.RB] ), absmask );
//	SPU->GPR[op.RR.RT] =  _mm_castpd_ps( _mm_cmpeq_pd( ra_abs, rb_abs ) );
//}
//
//void spu_dfcgt( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_castpd_ps( _mm_cmpgt_pd( 
//		_mm_castps_pd( SPU->GPR[op.RR.RA] ), 
//		_mm_castps_pd( SPU->GPR[op.RR.RB] ) ) );
//}
//
//void spu_dfcmgt( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	__m128d absmask = { (double)0xEFFFFFFFFFFFFFFF, (double)0xEFFFFFFFFFFFFFFF };
//	__m128d ra_abs = _mm_and_pd( _mm_castps_pd( SPU->GPR[op.RR.RA] ), absmask );
//	__m128d rb_abs = _mm_and_pd( _mm_castps_pd( SPU->GPR[op.RR.RB] ), absmask );
//	SPU->GPR[op.RR.RT] = _mm_castpd_ps( _mm_cmpgt_pd( ra_abs, rb_abs ) );
//
//}
//
//void spu_dfa( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_castpd_ps(_mm_add_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ));
//}
//
//void spu_dfs( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_castpd_ps(_mm_sub_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ));
//}
//
//void spu_dfm( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_castpd_ps(_mm_mul_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ));
//}
//
//void spu_dfma( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = 
//		_mm_castpd_ps( 
//			_mm_add_pd( 
//				_mm_mul_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ), _mm_castps_pd(SPU->GPR[op.RR.RT]) ) );
//}
//
//void spu_dfms( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = 
//		_mm_castpd_ps( 
//			_mm_sub_pd( 
//				_mm_mul_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ), _mm_castps_pd(SPU->GPR[op.RR.RT]) ) );
//}
//
//void spu_dfnma( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = 
//		_mm_castpd_ps( 
//		_mm_add_pd( 
//		_mm_mul_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ), _mm_castps_pd(SPU->GPR[op.RR.RT]) ) );
//
//	_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[0] = -_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[0];
//	_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[1] = -_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[1];
//}
//
//void spu_dfnms( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = 
//		_mm_castpd_ps( 
//		_mm_sub_pd( 
//		_mm_mul_pd( _mm_castps_pd(SPU->GPR[op.RR.RA]), _mm_castps_pd(SPU->GPR[op.RR.RB]) ), _mm_castps_pd(SPU->GPR[op.RR.RT]) ) );
//
//	_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[0] = -_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[0];
//	_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[1] = -_mm_castps_pd(SPU->GPR[op.RR.RT]).m128d_f64[1];
//}
//
//void spu_csflt( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint8_t scale = 155 - (uint8_t)op.RI8.I8;
//	for ( int i = 0; i < 4; ++i )
//	{
//		SPU->GPR[op.RI8.RT].m128_f32[i] = (float)SPU->GPR[op.RI8.RA].m128_i32[i] / scale;
//	}
//}
//
//void spu_cflts( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint8_t scale = 173 - (uint8_t)op.RI8.I8;
//	for ( int i = 0; i < 4; ++i )
//	{
//		SPU->GPR[op.RI8.RT].m128_i32[i] = (int32_t)(SPU->GPR[op.RI8.RA].m128_f32[i] * (float)scale);
//	}
//}
//
//void spu_cuflt( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint8_t scale = 155 - (uint8_t)op.RI8.I8;
//	for ( int i = 0; i < 4; ++i )
//	{
//		SPU->GPR[op.RI8.RT].m128_f32[i] = (float)SPU->GPR[op.RI8.RA].m128_u32[i] / scale;
//	}
//}
//
//void spu_cfltu( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	uint8_t scale = 173 - (uint8_t)op.RI8.I8;
//	for ( int i = 0; i < 4; ++i )
//	{
//		SPU->GPR[op.RI8.RT].m128_u32[i] = (uint32_t)(SPU->GPR[op.RI8.RA].m128_f32[i] * (float)scale);
//	}
//}