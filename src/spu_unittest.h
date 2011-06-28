#include <string>
#include <iosfwd>
#include <cstdint>
#include <emmintrin.h>

void SPUTest_ReportErrors();

void SPUTest_Expect( __m128 Result, __m128 Expected, const char* ErrorMsg );

struct xmm_proxy
{
	__m128 data_;

	inline xmm_proxy( __m128 x ) : data_(x) {}
	inline xmm_proxy( __m128i x ) : data_(_mm_castsi128_ps(x)) {}
	inline xmm_proxy( __m128d x ) : data_(_mm_castpd_ps(x)) {}

	inline operator __m128() const { return data_; }
	inline operator __m128i() const { _mm_castps_si128(data_); }
	inline operator __m128d() const { _mm_castps_pd(data_); }
};


#define __SPU_TEST_RRR( f, a, b, c, e ) SPUTest_Expect( f(xmm_proxy(a),xmm_proxy(b),xmm_proxy(c)), xmm_proxy(e), \
						#f "(" #a ", " #b ", " #c " ) ... " );

#define __SPU_TEST_RR( f, a, b, e )		SPUTest_Expect( f(xmm_proxy(a),xmm_proxy(b)), xmm_proxy(e), \
						#f "(" #a ", " #b " ) ... " );

#define __SPU_TEST_RI( f, a, i, e )		SPUTest_Expect( f(xmm_proxy(a),(i)), xmm_proxy(e), \
						#f "(" #a ", " #i " ) ... " );

#define __SPU_TEST_R( f, a, e )			SPUTest_Expect( f(xmm_proxy(a)), xmm_proxy(e), \
						#f "(" #a " ) ... " );