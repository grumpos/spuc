
//#include <fstream>
//#include <vector>
//#include <algorithm>
//#include <cstdint>
//#include <string>
//#include <cassert>
//#include <iostream>

#include "tools.h"
#include "spu_idb.h"
#include "elf_helper.h"
#include "spu_pseudo.h"

#include "basic_blocks.h"

namespace spu
{
	using namespace std;	

	/*op_distrib_t GatherOPDistribution( const vector<uint32_t>& Binary )
	{
		op_distrib_t Distrib;
		{
			size_t index = 0;

			for( auto insn : Binary )
			{
				Distrib[spu_decode_op_mnemonic(insn)].push_back(index++);
			}
		}	

		return Distrib;
	}*/
}



/*	
//
//	vector<uint64_t> BuildOPFlags( const vector<uint32_t>& Binary, op_distrib_t& Distrib )
//	{
//		vector<uint64_t> Flags;
//
//		Flags.resize( Binary.size() + 1 ); // +1 for the BB_LEAD flags to avoid checking for EOF
//
//#define FLAG_STATIC( name )\
//	auto& name = Distrib[#name];\
//	for_each( name.begin(), name.end(), [&Flags, Binary](size_t Index)\
//		{ \
//		Flags[Index] |= SPU_IS_BRANCH | SPU_IS_BRANCH_STATIC;\
//		\
//		Flags[Index + spu_op_decode_branch_offset(Binary[Index])] |= SPU_IS_BRANCH_TARGET;\
//		} );
//
//		auto& br = Distrib["br"];
//		for_each( br.begin(), br.end(), [&Flags, Binary](size_t Index)
//		{ 
//			Flags[Index] |= SPU_IS_BRANCH | SPU_IS_BRANCH_STATIC; 
//
//			const int32_t offset = spu_op_decode_branch_offset(Binary[Index]);
//			Flags[Index + offset] |= SPU_IS_BRANCH_TARGET;
//
//		});
//
//		auto& brsl = Distrib["brsl"];
//		for_each( brsl.begin(), brsl.end(), [&Flags, Binary](size_t Index)
//		{ 
//			Flags[Index] |= SPU_IS_BRANCH | SPU_IS_BRANCH_STATIC;
//
//			const int32_t offset = spu_op_decode_branch_offset(Binary[Index]);
//			Flags[Index + offset] |= SPU_IS_BRANCH_TARGET;
//
//		});
//
//		//auto& bra = Distrib["bra"];
//		//for_each( bra.begin(), bra.end(), [&Flags](size_t Index)
//		//{ Flags[Index] |= SPU_IS_BRANCH; } );
//
//		//auto& brasl = Distrib["brasl"];
//		//for_each( brasl.begin(), brasl.end(), [&Flags](size_t Index)
//		//{ Flags[Index] |= SPU_IS_BRANCH; } );
//
//#define FLAG_STATIC_COND( name )\
//	auto& name = Distrib[#name];\
//		for_each( name.begin(), name.end(), [&Flags, Binary](size_t Index)\
//		{ \
//			Flags[Index] |= SPU_IS_BRANCH | SPU_IS_BRANCH_STATIC | SPU_IS_BRANCH_CONDITIONAL;\
//			\
//			Flags[Index + spu_op_decode_branch_offset(Binary[Index])] |= SPU_IS_BRANCH_TARGET;\
//		} );
//
//		FLAG_STATIC_COND( brz );
//		FLAG_STATIC_COND( brnz );
//		FLAG_STATIC_COND( brhz );
//		FLAG_STATIC_COND( brhnz );
//
//#define FLAG_DYNAMIC( name )\
//	auto& name = Distrib[#name];\
//		for_each( name.begin(), name.end(), [&Flags, Binary](size_t Index)\
//		{ \
//			Flags[Index] |= SPU_IS_BRANCH | SPU_IS_BRANCH_DYNAIMC;\
//		} );
//
//		FLAG_DYNAMIC( bi );
//		FLAG_DYNAMIC( iret );
//		FLAG_DYNAMIC( bisled );
//		FLAG_DYNAMIC( bisl );
//
//#define FLAG_DYNAMIC_COND( name )\
//	auto& name = Distrib[#name];\
//		for_each( name.begin(), name.end(), [&Flags, Binary](size_t Index)\
//		{ \
//			Flags[Index] |= SPU_IS_BRANCH | SPU_IS_BRANCH_DYNAIMC | SPU_IS_BRANCH_CONDITIONAL;\
//		} );
//		
//		FLAG_DYNAMIC_COND( biz );
//		FLAG_DYNAMIC_COND( binz );
//		FLAG_DYNAMIC_COND( bihz );
//		FLAG_DYNAMIC_COND( bihnz );
//
//		// Flags instructions that essentially do nothing but move registers around.
//		// These are needed for there is no dedicated move register instruction for the SPU.
//#define FLAG_ASSIGNMENT(NAME, AVAL) \
//	auto& NAME = Distrib[#NAME];\
//		for_each( NAME.begin(), NAME.end(), [&Flags, Binary](size_t Index)\
//		{ \
//			const SPU_OP_COMPONENTS OPC = spu_decode_op_components( Binary[Index] );\
//			\
//			if ( OPC.IMM == AVAL )\
//				Flags[Index] |= SPU_IS_ASSIGNMENT;\
//		} );
//
//	auto flag_assignment = [&]( string mnem, uint32_t IMM )
//	{
//		auto& filter = Distrib[mnem];
//		for ( uint32_t index : filter )
//		{
//			const SPU_OP_COMPONENTS OPC = spu_decode_op_components( Binary[index] );
//			
//			if ( OPC.IMM == IMM )
//				Flags[index] |= SPU_IS_ASSIGNMENT;
//		}
//	};
//
//		FLAG_ASSIGNMENT( ahi, 0 );
//		FLAG_ASSIGNMENT( ai, 0 );
//		FLAG_ASSIGNMENT( sfhi, 0 );
//		FLAG_ASSIGNMENT( sfi, 0 );
//		FLAG_ASSIGNMENT( andbi, 0xFF );
//		FLAG_ASSIGNMENT( andhi, 0x3FF );
//		FLAG_ASSIGNMENT( andi, 0x3FF );
//		FLAG_ASSIGNMENT( orbi, 0 );
//		FLAG_ASSIGNMENT( orhi, 0 );
//		FLAG_ASSIGNMENT( ori, 0 );
//		FLAG_ASSIGNMENT( shlhi, 0 );
//		FLAG_ASSIGNMENT( shli, 0 );
//		FLAG_ASSIGNMENT( shlqbii, 0 );
//		FLAG_ASSIGNMENT( shlqbyi, 0 );
//		FLAG_ASSIGNMENT( rothi, 0 );
//		FLAG_ASSIGNMENT( roti, 0 );
//		FLAG_ASSIGNMENT( rotqbii, 0 );
//		FLAG_ASSIGNMENT( rotqbyi, 0 );
//		FLAG_ASSIGNMENT( rothmi, 0 );
//		FLAG_ASSIGNMENT( rotmi, 0 );
//		FLAG_ASSIGNMENT( rotqmbii, 0 );
//		FLAG_ASSIGNMENT( rotqmbyi, 0 );
//		FLAG_ASSIGNMENT( rotmahi, 0 );
//		FLAG_ASSIGNMENT( rotmai, 0 );
//
//		return Flags;
//	}
};

bool PossibleShufbMask( void* start )
{
	// align: 16 byte
	// each byte b: 0<b<=0x1f | b == {0x80|0xC0|0xE0}

	const __m128i mask = _mm_loadu_si128( (__m128i*)start );

	const __m128i allzero = 
		_mm_cmpeq_epi8( mask, _mm_setzero_si128() );

	if ( 0xFFFF == _mm_movemask_epi8(allzero) )
		return false;

	const __m128i is_80 = 
		_mm_cmpeq_epi8( mask, _mm_set1_epi8(0x80) );
	const __m128i is_E0 = 
		_mm_cmpeq_epi8( mask, _mm_set1_epi8(0xE0) );
	const __m128i is_C0 = 
		_mm_cmpeq_epi8( mask, _mm_set1_epi8(0xC0) );
	const __m128i is_gt_zero = 
		_mm_cmpgt_epi8( mask, _mm_setzero_si128() );
	const __m128i is_lt_20 = 
		_mm_cmplt_epi8( mask, _mm_set1_epi8(0x20) );

	const __m128i t0 = _mm_and_si128( is_gt_zero, is_lt_20 );
	const __m128i t1 = _mm_or_si128( allzero, t0 );
	const __m128i t2 = _mm_or_si128( is_C0, t1 );
	const __m128i t3 = _mm_or_si128( is_E0, t2 );
	const __m128i t4 = _mm_or_si128( is_80, t3 );

	if ( 0xFFFF == _mm_movemask_epi8(t4) )
		return true;
	else
		return false;
}

bool PossibleCtorDtorList( size_t offset, const std::vector<uint32_t>& Binary,
	std::vector<uint32_t>& FnCalls )
{
	// align: 16 byte
	// format: FFFFFFFF, n x fptr, FFFFFFFF, m x fptr
	// pad with 0 for 16 byte boundary

	if ( 0xFFFFFFFF != Binary[offset++] )
		return false;

	//std::vector<uint32_t> FnCalls;

	const uint32_t* val = &Binary[offset-1];

	while ( offset < Binary.size() && 
		( spu::IsValidFEPAddr(BE32(Binary[offset])) || 0 == Binary[offset]) )
	{
		FnCalls.push_back( BE32(Binary[offset]) );
		++offset;
	}

	// hit dtor
	if ( 0xFFFFFFFF != Binary[offset++] )
		return false;

	while ( offset < Binary.size() && 
		( spu::IsValidFEPAddr(BE32(Binary[offset])) && 0 != Binary[offset]) )
	{
		FnCalls.push_back( BE32(Binary[offset]) );
		++offset;
	}

	if ( 0 == Binary[offset] )
	{
		// hit 0, check padding. 
		// pads for 4 words
		const size_t ZeroDelimIndex = offset % 4;

		for ( size_t i = ZeroDelimIndex; i%4 != 0; ++i )
		{
			if ( Binary[offset++] != 0 ) 
				return false;
		}

		int sdfasdf = 3;
	}
	else
	{
		if ( offset%4 != 0 ) 
			return false;
	}

	return true;
}
*/