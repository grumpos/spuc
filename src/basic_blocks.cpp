
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

	void DumpLOC( const vector<string>& Binary )
	{
		for_each( Binary.cbegin(), Binary.cend(),
			[](string OP)
		{
			cout << OP << endl;
		});

		cout << endl;
	}

	bool IsNOP(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "nop" == mnem || "lnop" == mnem;
	}

	bool IsBranch(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"br", /*"brsl",*/ "bra", /*"brasl",*/ "brz", "brnz", "brhz", "brhnz",
			"bi", /*"bisl",*/ /*"bisled",*/ "iret", "biz", "binz", "bihz", "bihnz"
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );
		
		return IsBranch;
	};

	bool IsBranchUncond(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"br", "brsl", "bra", "brasl", /*"brz", "brnz", "brhz", "brhnz",*/
			"bi", "bisl", "bisled", "iret", /*"biz", "binz", "bihz", "bihnz"*/
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		return IsBranch;
	};

	bool IsAnyBranch(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"br", "brsl", "bra", "brasl", "brz", "brnz", "brhz", "brhnz",
			"bi", "bisl", "bisled", "iret", "biz", "binz", "bihz", "bihnz"
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		return IsBranch;
	};
	
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

	bool IsSTOP(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "stop" == mnem;
	}

	bool IsFnCall(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return ("brsl" == mnem) || ("brasl" == mnem);
	}

	bool IsReturn(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic(op);

		const string BranchOPs[] =
		{
			"bi", "biz", "binz", "bihz", "bihnz",
		};

		const bool IsIndirectBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);

		return IsIndirectBranch && 0 == OPComponents.RA;
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

	bool IsCFlowOP(uint32_t op)
	{
		return IsReturn(op) || IsSTOP(op) || IsFnCall(op) || IsJump(op);
	}	

	

	size_t Next8ByteAlignedOPOffset( size_t CurrentOP )
	{
		// used to skip LNOPs at the end of subroutines

		if ( CurrentOP & 1 )
			return CurrentOP + 1;
		else
			return CurrentOP + 2;
	}

	uint8_t GetFnArgCount( const vector<uint32_t>& Binary, pair<size_t, size_t> FnRange );

	vector<vector<pair<size_t, size_t>>> BuildInitialBlocks( 
		vector<uint32_t>& Binary, op_distrib_t& Distrib, size_t VirtualBase, size_t EntryIndex )
	{	
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

		vector<size_t> ScopeDepth;			
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
					return make_pair( IOffset + OPComponents.IMM, IOffset );
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

							/*if ( IOffset >= ( -OPComponents.IMM + 1) )
							{
								size_t JmpTargetPrev = IOffset + OPComponents.IMM;

								while ( string("lnop") == spu_decode_op_mnemonic(Binary[--JmpTargetPrev]) )
									;

								if ( IsBranchUncond(Binary[JmpTargetPrev]) )
									JumpsToFnEntry.insert( IOffset );
							}*/
						}
						else
						{
							if ( 0 == ScopeDepth[IOffset + OPComponents.IMM] )
							{
								Jumps.insert(IOffset + OPComponents.IMM);
							}
							/*size_t b = IOffset + 1;
							const size_t e = IOffset + OPComponents.IMM;

							for ( ; b != e; ++b )
							{
								if ( 0 != ScopeDepth[b] )
								{
									break;
								}
							}

							if ( b == e )
							{
								Jumps.insert(IOffset);
							}*/
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
			
			for_each(
				FnEntries.cbegin(), FnEntries.cend(),				
				[Binary, &FnRanges2, &ScopeDepth](size_t b)->void
			{
				vector<range_t> NewRange;

				size_t e = b;

				while ( e != Binary.size() )
				{
					if ( 0 == ScopeDepth[e] )
					{
						if ( IsUncondReturn( Binary[e] ) || IsSTOP( Binary[e] ))
						{
							NewRange.push_back( make_pair( b, Next8ByteAlignedOPOffset(e) ) );

							FnRanges2.push_back(NewRange);

							return;
						}
					
						if ( IsJump( Binary[e] ))
						{
							const SPU_OP_COMPONENTS OPC = spu_decode_op_components(Binary[e]);

							if ( OPC.IMM < 0 )
							{
								NewRange.push_back( make_pair( b, Next8ByteAlignedOPOffset(e) ) );

								FnRanges2.push_back(NewRange);

								return;
							}
							else if ( OPC.IMM > 0 && (0 == ScopeDepth[e + OPC.IMM]) )
							{
								NewRange.push_back( make_pair( b, e+1 ) );

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
			});

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


			//// turn jumps to function entries into brsl calls
			//{
			//	auto& br = Distrib["br"];

			//	for ( size_t i = 0; i != br.size(); ++i )
			//	{
			//		const size_t IOffset = br[i];

			//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

			//		const size_t Jumptarget = IOffset + OPComponents.IMM;

			//		if ( FnEntries.cend() != find(FnEntries.cbegin(), FnEntries.cend(), Jumptarget) )
			//		{
			//			SPU_INSTRUCTION SI;
			//			SI.RI16.OP = 0x66;
			//			SI.RI16.I16 = OPComponents.IMM;
			//			SI.RI16.RT = 0;

			//			Binary[IOffset] = SI.Instruction;
			//		}
			//	}
			//}
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
		uint8_t WrittenRegisters[128] = {0};
		uint8_t ArgCount = 0;

		for ( size_t i = FnRange.first; i != FnRange.second; ++i )
		{
			/*string mnem = spu_decode_op_mnemonic(Binary[i]);

			if ( mnem == "wrch" || mnem == "rdch" || mnem == "rchcnt" )
				continue;*/

			SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[i]);

			if ( OPComponents.RA != 0xFF && OPComponents.RA > 2 && OPComponents.RA < 75 )
			{
				if ( 0 == WrittenRegisters[OPComponents.RA] )
					ArgCount = max<uint8_t>( OPComponents.RA - 2, ArgCount );
			}
			if ( OPComponents.RB != 0xFF && OPComponents.RB > 2 && OPComponents.RB < 75 )
			{
				if ( 0 == WrittenRegisters[OPComponents.RB] )
					ArgCount = max<uint8_t>( OPComponents.RB - 2, ArgCount );
			}
			if ( OPComponents.RC != 0xFF && OPComponents.RC > 2 && OPComponents.RC < 75 )
			{
				if ( 0 == WrittenRegisters[OPComponents.RC] )
					ArgCount = max<uint8_t>( OPComponents.RC - 2, ArgCount );
			}
			if ( OPComponents.RT != 0xFF )
				WrittenRegisters[OPComponents.RT] = 1;
		}

		return ArgCount;
	}

	op_distrib_t GatherOPDistribution( const vector<uint32_t>& Binary )
	{
		op_distrib_t Distrib;
		{
			size_t i = 0;

			for_each( Binary.cbegin(), Binary.cend(),
				[&Distrib, &i](uint32_t Instr)
			{		
				Distrib[spu_decode_op_mnemonic(Instr)].push_back(i++);
			});
		}	

		return Distrib;
	}

#define IS_BRANCH (1 << 0)
#define IS_BRANCH_CONDITIONAL (1 << 1)

	vector<uint64_t> BuildOPFlags( const vector<uint32_t>& Binary, op_distrib_t& Distrib )
	{
		vector<uint64_t> Flags;

		Flags.resize( Binary.size() );

		auto& br = Distrib["br"];
		for_each( br.begin(), br.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH; } );

		auto& bra = Distrib["bra"];
		for_each( bra.begin(), bra.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH; } );

		auto& brsl = Distrib["brsl"];
		for_each( brsl.begin(), brsl.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH; } );

		auto& brasl = Distrib["brasl"];
		for_each( brasl.begin(), brasl.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH; } );

		auto& brz = Distrib["brz"];
		for_each( brz.begin(), brz.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH & IS_BRANCH_CONDITIONAL; } );

		auto& brnz = Distrib["brnz"];
		for_each( brnz.begin(), brnz.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH & IS_BRANCH_CONDITIONAL; } );

		auto& brhz = Distrib["brhz"];
		for_each( brhz.begin(), brhz.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH & IS_BRANCH_CONDITIONAL; } );

		auto& brhnz = Distrib["brhnz"];
		for_each( brhnz.begin(), brhnz.end(), [&Flags](size_t Index)
			{ Flags[Index] |= IS_BRANCH & IS_BRANCH_CONDITIONAL; } );

		return Flags;
	}
};