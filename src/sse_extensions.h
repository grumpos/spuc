#include <intrin.h>

#define _mm_srli_epi8( _A, _Imm ) _mm_and_si128( _mm_set1_epi8(0xFFui8 >> _Imm), _mm_srli_epi32( _A, _Imm ) )
#define _mm_slli_epi8( _A, _Imm ) _mm_and_si128( _mm_set1_epi8(0xFFui8 << _Imm), _mm_slli_epi32( _A, _Imm ) )

struct xmm_register
{
	__m128 data_;

	inline xmm_register( __m128 x )		: data_(x) {}
	inline xmm_register( __m128i x )	: data_(_mm_castsi128_ps(x)) {}
	inline xmm_register( __m128d x )	: data_(_mm_castpd_ps(x)) {}

	inline operator const __m128() const	{ return data_; }
	inline operator const __m128i() const	{ return _mm_castps_si128(data_); }
	inline operator const __m128d() const	{ return _mm_castps_pd(data_); }
};

struct v_u32 : public xmm_register
{
	//__m128i data_;

	//inline explicit v_u32( uint32_t d ) : data_(_mm_set1_epi32( d )) {}
	//inline v_u32( __m128 xmm ) : data_(_mm_castps_si128(xmm)) {}

	inline v_u32( __m128 x ) : xmm_register(x) {}
	inline v_u32( __m128i x ) : xmm_register(x) {}
	inline v_u32( __m128d x ) : xmm_register(x) {}

	inline explicit v_u32( uint32_t s ) : xmm_register(_mm_set1_epi32(s)) {}

	inline v_u32& operator+=( v_u32 RHS )
	{
		data_ = _mm_castsi128_ps( _mm_add_epi32( _mm_castps_si128( data_ ), _mm_castps_si128( RHS.data_ ) ) );
		return *this;
	}

	inline v_u32& operator-=( v_u32 RHS )
	{
		data_ = _mm_castsi128_ps( _mm_sub_epi32( _mm_castps_si128( data_ ), _mm_castps_si128( RHS.data_ ) ) );
		return *this;
	}

	inline v_u32& operator&=( v_u32 RHS ) { data_ = _mm_and_ps( data_, RHS.data_ ); return *this; }

	inline v_u32& operator|=( v_u32 RHS ) { data_ = _mm_or_ps( data_, RHS.data_ ); return *this; }

	inline v_u32& operator^=( v_u32 RHS ) { data_ = _mm_xor_ps( data_, RHS.data_ ); return *this; }

	// 	inline operator const __m128() const
	// 	{
	// 		return _mm_castsi128_ps(data_);
	// 	}
};

inline const v_u32 operator+( v_u32 LHS, v_u32 RHS )
{
	v_u32 Result = LHS;
	Result += RHS;
	return Result;
}

inline const v_u32 operator&( v_u32 LHS, v_u32 RHS )
{
	v_u32 Result = LHS;
	Result &= RHS;
	return Result;
}

//#define _MM_CONST_1 (_mm_srli_epi32(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()), 31))
#define _MM_CONST_1(r) (_mm_srli_epi32(_mm_cmpeq_epi32(*(__m128i*)&(r), *(__m128i*)&(r)), 31))


// hack around the fact that there is no unsigned cmp in SSE...
// (RB |<| RA) == (RB/2 < RA/2) | ((RB/2 == RA/2) & (RB_LSB < RA_LSB))
inline __m128i _mm_cmplt_epu32( __m128i _A, __m128i _B )
{
	const __m128i _A_shift		= _mm_srli_epi32( _A, 1 );
	const __m128i _B_shift		= _mm_srli_epi32( _B, 1 );
	const __m128i _A_LSB		= _mm_srli_epi32( _mm_slli_epi32( _A, 31 ), 31 );
	const __m128i _B_LSB		= _mm_srli_epi32( _mm_slli_epi32( _B, 31 ), 31 );
	const __m128i less_shifted  = _mm_cmplt_epi32( _A_shift, _B_shift );
	const __m128i equal_shifted = _mm_cmpeq_epi32( _A_shift, _B_shift );
	const __m128i less_LSB		= _mm_cmplt_epi32( _A_LSB, _B_LSB );
	return _mm_or_si128( less_shifted, _mm_and_si128( equal_shifted, less_LSB ) );
}

// (RB |>| RA) == (RB/2 > RA/2) | ((RB/2 == RA/2) & (RB_LSB > RA_LSB))
inline __m128i _mm_cmpgt_epu32( __m128i _A, __m128i _B )
{
	const __m128i _A_shift		= _mm_srli_epi32( _A, 1 );
	const __m128i _B_shift		= _mm_srli_epi32( _B, 1 );
	const __m128i _A_LSB		= _mm_srli_epi32( _mm_slli_epi32( _A, 31 ), 31 );
	const __m128i _B_LSB		= _mm_srli_epi32( _mm_slli_epi32( _B, 31 ), 31 );
	const __m128i less_shifted  = _mm_cmpgt_epi32( _A_shift, _B_shift );
	const __m128i equal_shifted = _mm_cmpeq_epi32( _A_shift, _B_shift );
	const __m128i less_LSB		= _mm_cmpgt_epi32( _A_LSB, _B_LSB );
	return _mm_or_si128( less_shifted, _mm_and_si128( equal_shifted, less_LSB ) );
}

inline __m128i _mm_cmpgt_epu16( __m128i _A, __m128i _B )
{
	const __m128i _A_shift		= _mm_srli_epi16( _A, 1 );
	const __m128i _B_shift		= _mm_srli_epi16( _B, 1 );
	const __m128i _A_LSB		= _mm_srli_epi16( _mm_slli_epi16( _A, 15 ), 15 );
	const __m128i _B_LSB		= _mm_srli_epi16( _mm_slli_epi16( _B, 15 ), 15 );
	const __m128i less_shifted  = _mm_cmpgt_epi16( _A_shift, _B_shift );
	const __m128i equal_shifted = _mm_cmpeq_epi16( _A_shift, _B_shift );
	const __m128i less_LSB		= _mm_cmpgt_epi16( _A_LSB, _B_LSB );
	return _mm_or_si128( less_shifted, _mm_and_si128( equal_shifted, less_LSB ) );
}

inline __m128i _mm_cmpgt_epu8( __m128i _A, __m128i _B )
{
	const __m128i _A_shift		= _mm_srli_epi8( _A, 1 );
	const __m128i _B_shift		= _mm_srli_epi8( _B, 1 );
	const __m128i _A_LSB		= _mm_srli_epi8( _mm_slli_epi8( _A, 7 ), 7 );
	const __m128i _B_LSB		= _mm_srli_epi8( _mm_slli_epi8( _B, 7 ), 7 );
	const __m128i less_shifted  = _mm_cmpgt_epi8( _A_shift, _B_shift );
	const __m128i equal_shifted = _mm_cmpeq_epi8( _A_shift, _B_shift );
	const __m128i less_LSB		= _mm_cmpgt_epi8( _A_LSB, _B_LSB );
	return _mm_or_si128( less_shifted, _mm_and_si128( equal_shifted, less_LSB ) );
}



inline __m128i _mm_cntb_epui8( __m128i _A )
{
	const __m128i _55 = _mm_set1_epi32(0x55555555);
	const __m128i _33 = _mm_set1_epi32(0x33333333);
	const __m128i _0F = _mm_set1_epi32(0x0F0F0F0F);

	const __m128i t0 = _mm_sub_epi8( _A, _mm_and_si128( _mm_srli_epi8( _A, 1 ), _55 ) );
	const __m128i t1 = _mm_add_epi8( _mm_and_si128( t0, _33 ), _mm_and_si128( _mm_srli_epi8( t0, 2 ), _33 )  );
	return _mm_and_si128( _mm_add_epi8( t1, _mm_srli_epi8( t1, 4 ) ), _0F );
}

inline __m128i _mm_cntb_epi32( __m128i _A )
{
	const __m128i _55 = _mm_set1_epi32(0x55555555);
	const __m128i _33 = _mm_set1_epi32(0x33333333);
	const __m128i _0F = _mm_set1_epi32(0x0F0F0F0F);
	const __m128i _01 = _mm_set1_epi32(0x01010101);	

	const __m128i t0 = _mm_sub_epi8( _A, _mm_and_si128( _mm_srli_epi32( _A, 1 ), _55 ) );
	const __m128i t1 = _mm_add_epi8( _mm_and_si128( t0, _33 ), _mm_and_si128( _mm_srli_epi32( t0, 2 ), _33 )  );
	const __m128i t2 = _mm_and_si128( _mm_add_epi8( t1, _mm_srli_epi32( t1, 4 ) ), _0F );

	// 32 bit mul
	auto p0 = _mm_mul_epu32( t2, _01 );
	auto p1 = _mm_mul_epu32( _mm_srli_si128(t2, 4), _mm_srli_si128(_01, 4) );
	auto p3 = _mm_shuffle_ps(_mm_castsi128_ps(p0), _mm_castsi128_ps(p1), _MM_SHUFFLE (2,0,2,0));

	return _mm_srli_epi32( _mm_castps_si128(p3), 24 );
}