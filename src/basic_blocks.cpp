
#include <vector>
#include <algorithm>
#include <map>
#include <cstdint>
#include <string>
#include <set>
#include <iterator>

#include "spu_idb.h"

#include "basic_blocks.h"

//struct spu_program_t
//{
//	size_t EntryPoint;
//	size_t VirtualBaseAddress;
//	vector<uint32_t> Binary;
//};
//
//struct spu_info_t
//{
//	std::map<string, vector<size_t>>	heuristics;
//	//vector<spu_branch_t>				staticBranches;
//	//vector<spu_function_t>				FunctionRanges;
//	std::map<size_t, string>			functionSymbols;
//	std::map<size_t, size_t>			jumps;
//	std::map<size_t, string>			jumpSymbols;
//};


namespace spu
{
	using namespace std;


	vector<basic_block_t> BuildInitialBlocks( const vector<uint32_t>& Binary )
	{
		auto Heuristics = BuildHeuristics( Binary );

		const auto& FunCallInstr = info->heuristics["brsl"];
		
		std::set<uint32_t> StaticCallTargets;
		
		StaticCallTargets.insert( (program->EntryPoint - program->VirtualBaseAddress) / 4 ); // main()
		
		std::transform( 
			FunCallInstr.cbegin(), FunCallInstr.cend(), 
			std::inserter(StaticCallTargets, StaticCallTargets.end()),
			[program](uint32_t IOffset)->uint32_t
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[IOffset]);
			return IOffset + (int16_t)OPComponents.IMM;
		});
		
		/*std::transform( StaticCallTargets.cbegin(), StaticCallTargets.cend(), std::back_inserter(info->FunctionRanges),
			[](uint32_t FnBeginOffset)->spu_function_t
		{
			const spu_function_t result = { FnBeginOffset, 0 };
			return result;
		});
		
		if ( !info->FunctionRanges.empty() )
		{
			std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end() - 1,
				[]( spu_function_t& FnRange )
			{
				auto Next = (&FnRange + 1);
				FnRange.end = Next->begin;
			});
			info->FunctionRanges.back().end = program->Binary.size();
		}	*/

		return vector<basic_block_t>();
	}

	map<string, vector<size_t>> BuildHeuristics( const vector<uint32_t>& Binary )
	{
		uint32_t i = 0;

		map<string, vector<size_t>> Heuristics;

		for_each( Binary.cbegin(), Binary.cend(),
			[&Heuristics, &i](uint32_t Instr)
		{		
			Heuristics[spu_decode_op_mnemonic(Instr)].push_back(i++);
		});

		return Heuristics;
	}
};
//
//vector<uint32_t> spuGatherStaticCallTargets( const spu_program_t* program, spu_info_t* info )
//{
//	auto& program_local = program;
//
//	const auto& FunCallInstr = info->heuristics["brsl"];
//
//	std::set<uint32_t> StaticCallTargets;
//
//	StaticCallTargets.insert( (program->EntryPoint - program->VirtualBaseAddress) / 4 ); // main()
//
//	std::transform( 
//		FunCallInstr.cbegin(), FunCallInstr.cend(), 
//		std::inserter(StaticCallTargets, StaticCallTargets.end()),
//		[program](uint32_t IOffset)->uint32_t
//	{
//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[IOffset]);
//		return IOffset + (int16_t)OPComponents.IMM;
//	});
//
//	/*std::transform( StaticCallTargets.cbegin(), StaticCallTargets.cend(), std::back_inserter(info->FunctionRanges),
//		[](uint32_t FnBeginOffset)->spu_function_t
//	{
//		const spu_function_t result = { FnBeginOffset, 0 };
//		return result;
//	});
//
//	if ( !info->FunctionRanges.empty() )
//	{
//		std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end() - 1,
//			[]( spu_function_t& FnRange )
//		{
//			auto Next = (&FnRange + 1);
//			FnRange.end = Next->begin;
//		});
//		info->FunctionRanges.back().end = program->Binary.size();
//	}	*/
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