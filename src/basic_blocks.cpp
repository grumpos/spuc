
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

#include "spu_idb.h"
#include "elf_helper.h"
#include "spu_pseudo.h"

#include "basic_blocks.h"

namespace spu
{
	using namespace std;

	typedef pair<size_t, size_t> range_t;

	void DumpLOC( const vector<string>& Binary )
	{
		for_each( Binary.cbegin(), Binary.cend(),
			[](string OP)
		{
			cout << OP << endl;
		});

		cout << endl;
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

	bool IsFnCall(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"br", "brsl", "bra", "brasl", "brz", "brnz", "brhz", "brhnz",
			"bi", "bisl", "bisled", "iret", "biz", "binz", "bihz", "bihnz"
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		return IsBranch;
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
	};

	bool IsSTOP(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "stop" == mnem;
	};

	bool IsCFlowOP(uint32_t op)
	{
		return IsBranch(op) || IsSTOP(op);
	};

	

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

	CFlowNode* CFlowBuilder( const vector<uint32_t>& Binary, size_t begin, size_t end )
	{
		/*static size_t counter = 11;
		if ( 0 == --counter )
		{
			return nullptr; 
		}*/

		assert( begin < Binary.size() );
		assert( end <= Binary.size() );

		if ( begin == end )
		{
			return nullptr; 
		}

		CodeBlock NewBlock;

		while ( begin != end && !IsAnyBranch(Binary[begin]) && !IsSTOP(Binary[begin]) )
		{
			NewBlock.LOC.push_back( spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin++], 0 ) );
		}		

		const string mnem = spu_decode_op_mnemonic( Binary[begin] );

		//DumpLOC( NewBlock.LOC );

		//cout << spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 ) << endl << "------------------" << endl;

		if ( "brsl" == mnem )
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[begin]);

			const size_t Target = begin + (int16_t)OPComponents.IMM;

			CFlowNode* Node = new CFlowNode;
			Node->Parent = nullptr;
			Node->Block = NewBlock;
			Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
			Node->Branch = nullptr;//CFlowBuilder( Binary, Target, end );
			Node->Follow = CFlowBuilder( Binary, begin + 1, end );

			return Node;
		}
		else if ( "stop" == mnem, "stopd" == mnem )
		{
			CFlowNode* Node = new CFlowNode;
			Node->Parent = nullptr;
			Node->Block = NewBlock;
			Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
			Node->Branch = nullptr;
			Node->Follow = nullptr;

			return Node;
		}

		return nullptr;

		if ( "brz" == mnem, "brnz" == mnem, "brhz" == mnem, "brhnz" == mnem )
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[begin]);

			const size_t Target = begin + (int16_t)OPComponents.IMM;

			CFlowNode* Node = new CFlowNode;

			Node->Block = NewBlock;
			Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
			Node->Branch = CFlowBuilder( Binary, begin + 1, Target );
			Node->Follow = CFlowBuilder( Binary, Target, end );

			return Node;
		}
		else if ( "brsl" == mnem )
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[begin]);

			const size_t Target = begin + (int16_t)OPComponents.IMM;

			CFlowNode* Node = new CFlowNode;

			Node->Block = NewBlock;
			Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
			Node->Branch = CFlowBuilder( Binary, Target, end );
			Node->Follow = CFlowBuilder( Binary, begin + 1, end );

			return Node;
		}
		else if ( "bi" == mnem )
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[begin]);

			if ( 0 == OPComponents.RT )
			{
				return nullptr;
			}
		}
		else if ( "stop" == mnem, "stopd" == mnem )
		{
			CFlowNode* Node = new CFlowNode;

			Node->Block = NewBlock;
			Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
			Node->Branch = nullptr;
			Node->Follow = nullptr;

			return Node;
		}

		return nullptr;
	}

	void WalkCFlowTree( CFlowNode* Root, size_t Depth )
	{
		if ( !Root )
		{
			return;
		}

		const string Indent( Depth, ' ' );

		for ( int i = 0; i != Root->Block.LOC.size() ; ++i )
		{
			cout << Indent << Root->Block.LOC[i] << endl;
		}

		//copy( Root->Block.LOC.cbegin(), Root->Block.LOC.cend(), ostream_iterator<string>(cout, "\n" ) );
		cout << Indent << Root->BROP << endl;
		cout << Indent << "----------" << endl;		

		WalkCFlowTree( Root->Branch, Depth + 1 );
		WalkCFlowTree( Root->Follow, Depth );
	}

	vector<basic_block_t> BuildInitialBlocks( const vector<uint32_t>& Binary, op_distrib_t& Distrib, size_t EntryIndex )
	{	
		CFlowNode* Root = CFlowBuilder( Binary, EntryIndex, Binary.size() );

		WalkCFlowTree( Root, 0 );
		


		set<size_t> StaticCallTargets;
		{
			const auto& FunCallInstr = Distrib["brsl"];

			transform( 
				FunCallInstr.cbegin(), FunCallInstr.cend(), 
				std::inserter(StaticCallTargets, StaticCallTargets.end()),
				[Binary](size_t IOffset)->size_t
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

				return IOffset + (int16_t)OPComponents.IMM;
			});

			// main()
			StaticCallTargets.insert( EntryIndex );
		}

		

		vector<range_t> FunctionBoundaries;
		{
			transform( 
				StaticCallTargets.cbegin(), --StaticCallTargets.cend(), ++StaticCallTargets.cbegin(),
				std::back_inserter(FunctionBoundaries),
				[](size_t Offset, size_t NextOffset )->range_t
			{
				return make_pair( Offset, NextOffset );
			});

			// add [last entry, EOF[
			FunctionBoundaries.push_back( make_pair( FunctionBoundaries.back().second, Binary.size() ) );
		}

		set<size_t> JumpTargets;
		{
			const auto& JumpInstr = Distrib["br"];

			transform( 
				JumpInstr.cbegin(), JumpInstr.cend(), 
				std::inserter(JumpTargets, JumpTargets.end()),
				[Binary](size_t IOffset)->size_t
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

				return IOffset + (int16_t)OPComponents.IMM;
			});
		}

		set<size_t> JumpsToFunctionEntry;
		{
			const auto& JumpInstr = Distrib["br"];

			for_each( 
				JumpInstr.cbegin(), JumpInstr.cend(),
				[&Binary, &FunctionBoundaries, &JumpsToFunctionEntry](size_t IOffset)
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

				const size_t JumpBase = IOffset;
				const size_t JumpTarget = IOffset + (int16_t)OPComponents.IMM;

				auto JumpBaseParentBlock = find_if( FunctionBoundaries.begin(), FunctionBoundaries.end(),
					[JumpBase](range_t r)->bool
				{
					return r.first <= JumpBase && JumpBase < r.second;
				});

				auto JumpTargetParentBlock = find_if( FunctionBoundaries.begin(), FunctionBoundaries.end(),
					[JumpTarget](range_t r)->bool
				{
					return r.first <= JumpTarget && JumpTarget < r.second;
				});

				if ( JumpBaseParentBlock != JumpTargetParentBlock )
				{
					JumpsToFunctionEntry.insert( JumpBase );
				}
			});
		}
		

		{
			auto BranchExtent = []( uint32_t BROP, size_t IOffset ) -> range_t
			{
				SPU_OP_COMPONENTS OPComponents = spu_decode_op_components( BROP );

				if ( OPComponents.IMM > 0 )
				{
					return make_pair( IOffset + 1, IOffset + OPComponents.IMM );
				}
				else
				{
					return make_pair( IOffset + OPComponents.IMM, IOffset );
				}
			};

			vector<size_t> ScopeLevel;
			ScopeLevel.resize( Binary.size() );
			{
				for ( size_t i = 0; i < Binary.size(); ++i )
				{
					if ( IsScopedBrach( Binary[i] ) )
					{
						range_t BrExt = BranchExtent( Binary[i], i );

						for( size_t j = BrExt.first; j != BrExt.second; ++j )
						{
							++ScopeLevel[j];
						}
					}
				}
			}

			// test dump
			{
				
			}

		}
		
		auto Fun = FunctionBoundaries[0];

		function_t f;

		const uint32_t* b = &Binary[Fun.first];
		const uint32_t* e = &Binary[Fun.second];
		const uint32_t* i = b;

		while ( i != e )
		{
			basic_block_t NewBlock;

			NewBlock.begin = Fun.first + (i-b);

			while ( i != e && !IsCFlowOP(*i) )
				++i;
			i += 1;

			NewBlock.end = Fun.first + (i-b);

			f.blocks.push_back( NewBlock );
		}		

		return vector<basic_block_t>();
	}

	

	op_distrib_t GatherOPDistribution( const vector<uint32_t>& Binary )
	{
		op_distrib_t Distrib;
		{
			uint32_t i = 0;

			for_each( Binary.cbegin(), Binary.cend(),
				[&Distrib, &i](uint32_t Instr)
			{		
				Distrib[spu_decode_op_mnemonic(Instr)].push_back(i++);
			});
		}	

		return Distrib;
	}
};

//
//	//////////////////////////////////////////////////////////////////////////
//
//
//	//auto IsNOP = [](uint32_t op)->bool
//	//{
//	//	return 1 == (op >> 21) || 513 == (op >> 21);
//	//};
//
//	//auto IsSTOP = [](uint32_t op)->bool
//	//{
//	//	return 0 == (op >> 21) || 320 == (op >> 21);
//	//};
//
//	//auto IsUnconditionalReturn = [](uint32_t op)->bool
//	//{
//	//	if ( 0x1a8 == spu_decode_op_opcode(op) ) // bi
//	//	{
//	//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
//	//		if ( 0 == OPComponents.RA )
//	//		{
//	//			return true;
//	//		}
//	//	}
//	//	return false;
//	//};
//
//	//auto IsConditionalReturn = [](uint32_t op)->bool
//	//{
//	//	/*const uint32_t opcode = op >> 21;
//	//	if ( 296 == opcode || 297 == opcode || 298 == opcode || 299 == opcode || 424 == opcode ) // bi*
//	//	{
//	//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
//	//		if ( 0 == OPComponents.RA )
//	//		{
//	//			return true;
//	//		}
//	//	}*/
//
//	//	/* Indirect conditional jumps to the link register is equivalent to:
//
//	//		if( condition )
//	//			return;
//
//	//		Indirect unconditional jumps to the link register is equivalent to:
//
//	//		return;
//	//	*/
//
//	//	switch(spu_decode_op_opcode(op))
//	//	{
//	//	case 0x128: // biz
//	//	case 0x129: // binz
//	//	case 0x12a: // bihz
//	//	case 0x12b: // bihnz
//	//		{
//	//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
//	//			if ( 0 == OPComponents.RA )
//	//			{
//	//				return true;
//	//			}
//	//		}
//	//	default:
//	//		return false;
//	//	}
//	//};
//
//	//auto GetJumpAmount = [](uint32_t op) -> int32_t
//	//{
//	//	switch(spu_decode_op_opcode(op))
//	//	{
//	//	case 0x100: // brz
//	//	case 0x108: // brnz
//	//	case 0x110: // brhz
//	//	case 0x118: // brhnz
//	//	case 0x190: // br
//	//		{
//	//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
//	//			return (int32_t)OPComponents.IMM;
//	//		}
//	//	default:
//	//		return 0;
//	//	}
//	//};
//
//	auto IsBrach = [](uint32_t op)->bool
//	{
//		//uint32_t opcode = spu_decode_op_opcode(op);
//		//const string BR_I16[] = { 0x100, 0x108, 0x110, 0x118, 0x180, /*0x188,*/ 0x190, /*0x198*/ };
//		//auto a = std::count(BR_I16, BR_I16 + _countof(BR_I16), op >> 23 );
//
//		//const string BR_RR[] = { 0x1a8, /*0x1a9,*/ 0x1aa, /*0x1ab,*/ 0x128, 0x129, 0x12a, 0x12b };
//		//auto b = std::count(BR_RR, BR_RR + _countof(BR_RR), op >> 21 );
//
//		string mnem = spu_decode_op_mnemonic( op );
//
//		const string BR_I16[] = { "br", /*"brsl",*/ "bra", /*"brasl",*/ "brz", "brnz", "brhz", "brhnz" };
//		auto a = std::count(BR_I16, BR_I16 + _countof(BR_I16), mnem );
//
//		const string BR_RR[] = { "bi", /*"bisl",*/ /*"bisled",*/ "iret", "biz", "binz", "bihz", "bihnz" };
//		auto b = std::count(BR_RR, BR_RR + _countof(BR_RR), mnem );
//
//		const string STOP[] = { "stop" };
//		auto c = std::count(STOP, STOP + _countof(STOP), mnem );
//
//		return 0 != (a + b + c);
//	};
//
//	vector<basic_block_t> blocks;
//
//	
//	enum spu_basic_block_type_t
//	{
//		function_end,
//		unconditional_static_jump,
//
//	};
//	
//
//	auto FindBasicBlockInRange = [&blocks, program_local, IsBrach]( size_t begin, size_t end )
//	{
//		while ( begin != end )
//		{
//			basic_block_t NewBlock = FindBasicBlock( begin, end, program_local, IsBrach );
//
//			if ( NewBlock.first != NewBlock.last )
//			{
//				blocks.push_back(NewBlock);
//				const uint32_t LastOp = program_local->Binary[NewBlock.last - 1];
//
//				if ( LastOp == 0 )
//					break;
//
//				begin = NewBlock.last;
//				/*begin += GetJumpAmount(LastOp);
//
//				if ( begin == NewBlock.first )
//				{
//				begin = NewBlock.last;
//				}*/
//			}
//			else
//				break;
//		}
//	};
//
//	//pflow_node_t* p = FindBasicBlock2( &program->Binary[4], &program->Binary[16], program, IsBrach );
//	
//	// skip SPU GUID
//	FindBasicBlockInRange( 66, 193 );
//
//	/*auto MakeFunctionText = [&]( const vector<basic_block_t>& blocks ) -> string
//	{
//		std::ostringstream oss;
//
//		for ( auto i = blocks.cbegin(); i != blocks.cend(); ++i )
//		{
//			for ( auto j = &program->Binary[i->first]; j != &program->Binary[i->last]; ++j )
//			{
//				oss << spu_make_pseudo( (SPU_INSTRUCTION&)(*j), 0 ) << std::endl;
//			}
//			oss << std::endl;
//		}
//
//		return oss.str();
//	};*/
//
//
//	//std::cout << MakeFunctionText( blocks );
//
//	auto& info_local = info;
//
//	vector<basic_block_t> blocks2;
//
//	
//	return vector<uint32_t>( StaticCallTargets.begin(), StaticCallTargets.end() );
//}