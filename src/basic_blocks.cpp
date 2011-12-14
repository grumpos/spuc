
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdint>
#include <string>
#include <set>
#include <iterator>
#include <cassert>
#include <iostream>
#include <functional>
#include <deque>
#include <sstream>

#include "spu_idb.h"
#include "elf_helper.h"
#include "spu_pseudo.h"

#include "basic_blocks.h"

namespace spu
{
	using namespace std;

	typedef pair<size_t, size_t> range_t;

	struct CodeBlock
	{
		vector<string> LOC;
	};

	struct CFlowNode
	{
		CFlowNode* Parent;
		CFlowNode* Branch;
		CFlowNode* Follow;

		CodeBlock Block;

		string BROP;
	};

	bool IsValidFEPAddr( uint32_t addr )
	{
		bool Is8ByteAligned = (addr % 16 == 0) || (addr % 16 == 8);

		bool IsWithinLSSize = addr < 0x3FFFF;

		bool NonZero = 0 != addr;

		return IsWithinLSSize && Is8ByteAligned && NonZero;
	}

	
	bool IsScopedBrach(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"brz", "brnz", "brhz", "brhnz",
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		return IsBranch;
	}

	

	bool IsUncondReturn(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic(op);

		const bool IsIndirectBranch = "bi" == mnem;

		SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);

		return IsIndirectBranch && 0 == OPComponents.RA;
	}

	bool IsJump(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "br" == mnem;
	}
	

	size_t Next8ByteAlignedOPOffset( size_t CurrentOP )
	{
		// used to skip LNOPs at the end of subroutines

		if ( CurrentOP & 1 )
			return CurrentOP + 1;
		else
			return CurrentOP + 2;
	}

	bool PossibleCtorDtorList( size_t start, const vector<uint32_t>& Binary );
	bool PossibleShufbMask( void* start );

	uint8_t GetFnArgCount( const vector<uint32_t>& Binary, pair<size_t, size_t> FnRange );

	vector<vector<pair<size_t, size_t>>> BuildInitialBlocks( 
		vector<uint32_t>& Binary, op_distrib_t& Distrib, size_t /*VirtualBase*/, size_t EntryIndex )
	{	
		vector<size_t> PossCtor;
		{
			size_t i = 0;
			for_each( Binary.begin(), Binary.end()-4, 
				[&]( uint32_t val )
			{
				if ( true == PossibleShufbMask( &Binary[i] ) && 0 == i%4 )
				{
					PossCtor.push_back( i );
					
				}
				++i;
			});

		}



		vector<uint32_t> PossibleFEP;
		{
			for_each( Binary.begin(), Binary.end(), 
				[&PossibleFEP]( uint32_t val )
			{
				if ( true == IsValidFEPAddr( val ) )
				{
					PossibleFEP.push_back( val );
				}
			});

		}

		set<size_t> InvalidJumps;
		
		set<size_t> FnEntryByStaticCall;
		{
			auto GetJumpTargets = [Binary, &InvalidJumps](size_t IOffset)->size_t
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

				const size_t JmpTarget = IOffset + OPComponents.IMM;

				if ( JmpTarget >= Binary.size() )
					InvalidJumps.insert( JmpTarget );

				return JmpTarget;
			};

			const auto& brsl = Distrib["brsl"];
			const auto& brasl = Distrib["brasl"];

			transform( brsl.cbegin(), brsl.cend(), 
				std::inserter(FnEntryByStaticCall, FnEntryByStaticCall.end()),
				GetJumpTargets );

			transform( brasl.cbegin(), brasl.cend(), 
				std::inserter(FnEntryByStaticCall, FnEntryByStaticCall.end()),
				GetJumpTargets );
			
			// main()
			FnEntryByStaticCall.insert( EntryIndex );
		}

		vector<uint16_t> ScopeDepth;			
		{
			auto BranchExtent = []( uint32_t BROP, size_t IOffset ) -> range_t
			{
				SPU_OP_COMPONENTS OPComponents = spu_decode_op_components( BROP );

				if ( OPComponents.IMM > 0 )
				{
					return make_pair( IOffset, IOffset + OPComponents.IMM );
				}
				else if (OPComponents.IMM < 0)
				{
					return make_pair( IOffset + OPComponents.IMM, IOffset + 1 );
				}
				else
				{
					return make_pair(0,0);
				}
			};

			ScopeDepth.resize( Binary.size() );

			for ( size_t i = 0; i < Binary.size(); ++i )
			{
				if ( IsScopedBrach( Binary[i] ) )
				{
					range_t BrExt = BranchExtent( Binary[i], i );

					for( size_t j = BrExt.first; j != BrExt.second; ++j )
					{
						++ScopeDepth[j];
					}
				}
			}
		}

		

		size_t MainEnd = 0;
		{
			auto i = EntryIndex;

			while ( string("stop") != spu_decode_op_mnemonic(Binary[i]) )
			 ++i;

			++i;

			MainEnd = Next8ByteAlignedOPOffset(i);
		}

		set<size_t> FnEntryAfterReturn;
		{
			set<size_t> Returns;
			{
				auto& bi = Distrib["bi"];

				for_each( 
					bi.cbegin(), bi.cend(),
					[&Returns, &ScopeDepth, &Binary]( size_t IOffset )
				{
					if ( 0 == ScopeDepth[IOffset] && IsUncondReturn( Binary[IOffset]) )
					{
						Returns.insert(IOffset);
					}
				});
			}

			transform( 
				Returns.cbegin(), Returns.cend(),
				std::inserter(FnEntryAfterReturn, FnEntryAfterReturn.end()), 
				Next8ByteAlignedOPOffset );
		}

		set<size_t> FnEntryAfterJumps;
		{
			set<size_t> Jumps;
			{
				auto& br = Distrib["br"];

				for_each( br.cbegin(), br.cend(),
					[&Jumps, &ScopeDepth, &Binary]( size_t IOffset )
				{
					if ( 0 == ScopeDepth[IOffset] )
					{
						SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

						if ( OPComponents.IMM < 0 )
						{
							Jumps.insert(IOffset);
						}
						else
						{
							if ( 0 == ScopeDepth[IOffset + OPComponents.IMM] )
							{
								Jumps.insert(IOffset + OPComponents.IMM);
							}
						}
					}
				});

				transform( 
					Jumps.cbegin(), Jumps.cend(),
					std::inserter(FnEntryAfterJumps, FnEntryAfterJumps.end()),
					[]( size_t IOffset ) -> size_t
				{
					const bool EvenIP = 0 == (IOffset % 2);
					return IOffset + (EvenIP ? 2 : 1);
				});
			}
		}		

		vector<range_t> FnRanges;
		vector<vector<range_t>> FnRanges2;
		{			
			auto FnEntries = FnEntryByStaticCall;
			//FnEntries.insert( FnEntryAfterReturn.begin(), FnEntryAfterReturn.end() );
			//FnEntries.insert( FnEntryAfterJumps.begin(), FnEntryAfterJumps.end() );
			FnEntries.insert(MainEnd);
			//FnEntries.insert( JumpsToFnEntry.begin(), JumpsToFnEntry.end() );

			vector<size_t> DynFnEntries;
			
				
			auto ParseFunctions = [Binary, &FnRanges2, &ScopeDepth, &DynFnEntries](size_t b)->void
			{
				vector<range_t> NewRange;

				size_t e = b;

				while ( e != Binary.size() )
				{
					if ( 0 == ScopeDepth[e] )
					{
						if ( IsUncondReturn( Binary[e] ) || ( 0 == Binary[e] ) )
						{
							NewRange.push_back( make_pair( b, Next8ByteAlignedOPOffset(e) ) );

							FnRanges2.push_back(NewRange);

							DynFnEntries.push_back( Next8ByteAlignedOPOffset(e) );

							return;
						}
					
						if ( IsJump( Binary[e] ))
						{
							const SPU_OP_COMPONENTS OPC = spu_decode_op_components(Binary[e]);

							if ( OPC.IMM < 0 )
							{
								if ( (0 == ScopeDepth[e + OPC.IMM]) && (e + OPC.IMM) < b )
								{
									// skip the jump itself, otherwise it would be e+1
									NewRange.push_back( make_pair( b, e ) );

									b = e = (e + OPC.IMM);

									continue;
								}
								else if ( 0 != ScopeDepth[e + OPC.IMM] )
								{
									//continue;
								}
								else
								{
									NewRange.push_back( make_pair( b, Next8ByteAlignedOPOffset(e) ) );

									FnRanges2.push_back(NewRange);

									return;
								}								
							}
							else if ( OPC.IMM > 0 && (0 == ScopeDepth[e + OPC.IMM]) )
							{
								// skip the jump itself, otherwise it would be e+1
								NewRange.push_back( make_pair( b, e ) );

								b = e = (e + OPC.IMM);

								continue;
							}
							else if ( OPC.IMM == 0 )
							{
								NewRange.push_back( make_pair( b, Next8ByteAlignedOPOffset(e) ) );

								FnRanges2.push_back(NewRange);

								return;
							}
						}						
					}
					++e;
				}
			};

			for_each( FnEntries.cbegin(), FnEntries.cend(), ParseFunctions );

			//set<size_t> s0( DynFnEntries.cbegin(), DynFnEntries.cend() );			
			//set<size_t> s1;
			//set_difference( s0.begin(), s0.end(), FnEntries.begin(), FnEntries.end(), 
			//	inserter( s1, s1.end() ) );

			////for_each( s1.cbegin(), s1.cend(), ParseFunctions );


			// 1st pass
			//transform(
			//	FnEntries.cbegin(), --FnEntries.cend(), ++FnEntries.cbegin(),
			//	back_inserter( FnRanges ),
			//	[](size_t b, size_t e) -> range_t
			//{
			//	return make_pair( b, e );
			//});

			//set<size_t> JumpsToFnEntry;

			//auto IsBranchToFn = [Binary, &JumpsToFnEntry](range_t r)
			//{
			//	size_t LastOPOffset = r.second-1;

			//	if ( string("lnop") == spu_decode_op_mnemonic(Binary[LastOPOffset]) )
			//		--LastOPOffset;

			//	const uint32_t LastOP = Binary[LastOPOffset];

			//	const SPU_OP_COMPONENTS OPC = spu_decode_op_components(LastOP);

			//	if ( (string("br") == spu_decode_op_mnemonic(LastOP)) && (LastOPOffset + OPC.IMM < r.first) )
			//	{
			//		JumpsToFnEntry.insert(LastOPOffset + OPC.IMM);
			//	}
			//};

			//set<size_t> NewEntrys;

			//do 
			//{
			//	FnEntries.insert( NewEntrys.begin(), NewEntrys.end() );

			//	FnRanges.clear();
			//	JumpsToFnEntry.clear();
			//	NewEntrys.clear();

			//	transform(
			//		FnEntries.cbegin(), --FnEntries.cend(), ++FnEntries.cbegin(),
			//		back_inserter( FnRanges ),
			//		[](size_t b, size_t e) -> range_t
			//	{
			//		return make_pair( b, e );
			//	});

			//	for_each( FnRanges.cbegin(), FnRanges.cend(), IsBranchToFn );

			//	set_difference( 
			//		JumpsToFnEntry.cbegin(), JumpsToFnEntry.cend(),
			//		FnEntries.cbegin(), FnEntries.cend(), 
			//		inserter(NewEntrys, NewEntrys.end() ) );

			//} while ( !NewEntrys.empty() );


			// turn jumps to function entries into brsl calls
			{
				auto& br = Distrib["br"];

				for ( size_t i = 0; i != br.size(); ++i )
				{
					const size_t IOffset = br[i];

					const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

					const size_t Jumptarget = IOffset + OPComponents.IMM;

					if ( FnEntries.cend() != find(FnEntries.cbegin(), FnEntries.cend(), Jumptarget) )
					{
						SPU_INSTRUCTION SI;
						SI.RI16.OP = 0x66;
						SI.RI16.I16 = OPComponents.IMM;
						SI.RI16.RT = 0;

						Binary[IOffset] = SI.Instruction;
					}
				}
			}
		}

		

		/*vector<uint8_t> FnArgCounts;
		FnArgCounts.resize(FnRanges.size());

		transform( FnRanges.cbegin(), FnRanges.cend(), FnArgCounts.begin(),
		[Binary]( pair<size_t, size_t> FnRange )
		{
		return GetFnArgCount(Binary, FnRange);
		});*/

		return FnRanges2;
	}

	

	uint8_t GetFnArgCount( const vector<uint32_t>& Binary, pair<size_t, size_t> FnRange )
	{
		// registers 3-74 are used for argument passing

		uint8_t Registers[256] = {0};
		uint8_t ArgCount = 0;

#define CHECK_REG_ARG( gpr )\
		if ( OPComponents.gpr != 0xFF && OPComponents.gpr > 2 && OPComponents.gpr < 75 )\
		{\
			if ( 0 == Registers[OPComponents.gpr] )\
				ArgCount = max<uint8_t>( OPComponents.gpr - 2, ArgCount );\
		}

		for ( size_t i = FnRange.first; i != FnRange.second; ++i )
		{
			SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[i]);

// 			Registers[OPComponents.RA] = !Registers[OPComponents.RA] ? 0xFF : 1;
// 			Registers[OPComponents.RB] = !Registers[OPComponents.RA] ? 0xFF : 1;
// 			Registers[OPComponents.RC] = !Registers[OPComponents.RA] ? 0xFF : 1;
// 			Registers[OPComponents.RT] = 1;

			CHECK_REG_ARG( RA );
			CHECK_REG_ARG( RB );
			CHECK_REG_ARG( RC );
			
			if ( OPComponents.RT != 0xFF )
				Registers[OPComponents.RT] = 1;
		}

#undef CHECK_REG_ARG

		return ArgCount;
	}

	op_distrib_t GatherOPDistribution( const vector<uint32_t>& Binary )
	{
		// TODO: make op_distrib_t use 11 bit integer indexing and a fixed array
		size_t count = 0;
		op_distrib_t Distrib;
		{
			size_t index = 0;

			for_each( Binary.cbegin(), Binary.cend(),
				[&Distrib, &index, &count](uint32_t Instr)
			{		
				++count;
				Distrib[spu_decode_op_mnemonic(Instr)].push_back(index++);
			});
		}	

		return Distrib;
	}

	bool PossibleCtorDtorList( size_t start, const vector<uint32_t>& Binary )
	{
		// align: 16 byte
		// format: FFFFFFFF, n x fptr, 0, FFFFFFFF, m x fptr, 0

		if ( 0xFFFFFFFF != Binary[start++] )
			return false;
		
		// size - 1 because the minimal ctor is {0xFFFFFFFF, 0}
		while ( start < (Binary.size() - 1) && IsValidFEPAddr(Binary[start]) )
		{
			++start;
		}

		if ( 0 == Binary[start] && start < (Binary.size() - 1)  )
			return true;

		return false;
	}

	bool PossibleShufbMask( void* start )
	{
		// align: 16 byte
		// each byte b: 0<b<=0x1f | b == {0x80|0xC0|0xE0}

		// special case all 0 not handled here
		if ( 0 == ((uint32_t*)start)[0] && 
			0 == ((uint32_t*)start)[1] && 
			0 == ((uint32_t*)start)[2] && 
			0 == ((uint32_t*)start)[3] )
			return false;

		for ( size_t i = 0; i < 16; ++i )
		{
			const uint8_t b = ((uint8_t*)start)[i];

			if (0 <= b && b <= 0x1F)
				continue;
			else if ( 0x80 == b )
				continue;
			else if ( 0xC0 == b )
				continue;
			else if ( 0xE0 == b )
				continue;
			else
				return false;
		}

		return true;
	}

	bool PossibleString( void* start )
	{
		// align: 16 byte
		// ends with padding of 0s. padding must reach the end of the 
		// last used 16 byte block. if the string length is multiple of
		// 16, a padding of 16 zero bytes will follow.
		/* how to tell if it's text? technically there are no constraints here.
		   we can tell for sure if everything is between 0x20 and 0x7F
		   also if it's declared as const char*, there is usually a 0x0A terminator for nl
		*/
		return false;
	}

	/*
	Also try and build an extensive list of named data patterns:
	inserter shufb
	reorder shufb
	AES matrices
	*/

	vector<uint64_t> BuildOPFlags( const vector<uint32_t>& Binary, op_distrib_t& Distrib )
	{
		vector<uint64_t> Flags;

		Flags.resize( Binary.size() + 1 ); // +1 for the BB_LEAD flags to avoid checking for EOF

		auto& br = Distrib["br"];
		for_each( br.begin(), br.end(), [&Flags, Binary](size_t Index)
		{ 
			Flags[Index] |= SPU_IS_BRANCH; 

			const int32_t offset = spu_op_decode_branch_offset(Binary[Index]);
			Flags[Index + offset] |= SPU_IS_BRANCH_TARGET;

			Flags[Index + 1] |= BB_LEAD;
		});		

		auto& brsl = Distrib["brsl"];
		for_each( brsl.begin(), brsl.end(), [&Flags, Binary](size_t Index)
		{ 
			Flags[Index] |= SPU_IS_BRANCH;

			Flags[Index + spu_op_decode_branch_offset(Binary[Index])] |= SPU_IS_BRANCH_TARGET;

			Flags[Index + 1] |= BB_LEAD;
		});


		// As far as I know C code never compiles into absolute static branches on the SPU

// 		auto& bra = Distrib["bra"];
// 		for_each( bra.begin(), bra.end(), [&Flags](size_t Index)
// 		{ Flags[Index] |= SPU_IS_BRANCH; } );
// 
// 		auto& brasl = Distrib["brasl"];
// 		for_each( brasl.begin(), brasl.end(), [&Flags](size_t Index)
// 			{ Flags[Index] |= SPU_IS_BRANCH; } );

#define FLAG_STATIC_COND( name )\
	auto& name = Distrib[#name];\
		for_each( name.begin(), name.end(), [&Flags, Binary](size_t Index)\
		{ \
			Flags[Index] |= SPU_IS_BRANCH & SPU_IS_BRANCH_CONDITIONAL;\
			\
			Flags[Index + spu_op_decode_branch_offset(Binary[Index])] |= SPU_IS_BRANCH_TARGET;\
			\
			Flags[Index + 1] |= BB_LEAD;\
		} );

		FLAG_STATIC_COND( brz );
		FLAG_STATIC_COND( brnz );
		FLAG_STATIC_COND( brhz );
		FLAG_STATIC_COND( brhnz );

#define FLAG_DYNAMIC_COND( name )\
	auto& name = Distrib[#name];\
		for_each( name.begin(), name.end(), [&Flags, Binary](size_t Index)\
		{ \
			Flags[Index] |= SPU_IS_BRANCH & SPU_IS_BRANCH_CONDITIONAL;\
			\
			Flags[Index + 1] |= BB_LEAD;\
		} );
		
		FLAG_DYNAMIC_COND( biz );
		FLAG_DYNAMIC_COND( binz );
		FLAG_DYNAMIC_COND( bihz );
		FLAG_DYNAMIC_COND( bihnz );

		// Flags instructions that essentially do nothing but move registers around.
		// These are needed for there is no dedicated move register instruction for the SPU.
#define FLAG_ASSIGNMENT(NAME, AVAL) \
	auto& NAME = Distrib[#NAME];\
		for_each( NAME.begin(), NAME.end(), [&Flags, Binary](size_t Index)\
		{ \
			const SPU_OP_COMPONENTS OPC = spu_decode_op_components( Binary[Index] );\
			\
			if ( OPC.IMM == AVAL )\
				Flags[Index] |= SPU_IS_ASSIGNMENT;\
		} );

		FLAG_ASSIGNMENT( ahi, 0 );
		FLAG_ASSIGNMENT( ai, 0 );
		FLAG_ASSIGNMENT( sfhi, 0 );
		FLAG_ASSIGNMENT( sfi, 0 );
		FLAG_ASSIGNMENT( andbi, 0xFF );
		FLAG_ASSIGNMENT( andhi, 0x3FF );
		FLAG_ASSIGNMENT( andi, 0x3FF );
		FLAG_ASSIGNMENT( orbi, 0 );
		FLAG_ASSIGNMENT( orhi, 0 );
		FLAG_ASSIGNMENT( ori, 0 );
		FLAG_ASSIGNMENT( shlhi, 0 );
		FLAG_ASSIGNMENT( shli, 0 );
		FLAG_ASSIGNMENT( shlqbii, 0 );
		FLAG_ASSIGNMENT( shlqbyi, 0 );
		FLAG_ASSIGNMENT( rothi, 0 );
		FLAG_ASSIGNMENT( roti, 0 );
		FLAG_ASSIGNMENT( rotqbii, 0 );
		FLAG_ASSIGNMENT( rotqbyi, 0 );
		FLAG_ASSIGNMENT( rothmi, 0 );
		FLAG_ASSIGNMENT( rotmi, 0 );
		FLAG_ASSIGNMENT( rotqmbii, 0 );
		FLAG_ASSIGNMENT( rotqmbyi, 0 );
		FLAG_ASSIGNMENT( rotmahi, 0 );
		FLAG_ASSIGNMENT( rotmai, 0 );
		return Flags;
	}
};