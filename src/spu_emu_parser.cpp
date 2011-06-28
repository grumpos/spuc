#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "spu_internals_x86.h"
#include "raw.h"
#include "spu_emu.h"
#include "spu_idb.h"
#include "spu_pseudo.h"
#include "elf_helper.h"

using std::string;
using std::vector;

enum
{
	MASK_IGNORED = 0xFFFFFFFF,
	MASK_RETURN = 0xEEEEEEEE,
	MASK_NOP = 0xDDDDDDDD,
	MASK_STOP = 0xCCCCCCCC,
	MASK_START = 0xBBBBBBBB
};

void spu_extecute_single( SPU_t* SPU, uint32_t op )
{
	auto solver = spu_decode_op_solver( op );	

	SPU_INSTRUCTION instr;
	instr.Instruction = op;
	solver( SPU, instr );	
}

//static const uint64_t ENDIAN_HELPER = 1;
//static const uint64_t SYSTEM_ENDIAN_LE = (uint64_t)((uint8_t&)ENDIAN_HELPER&1);

void ReverseBytesInRange( void* data, const char* range_map )
{
	uint8_t* raw = (uint8_t*)data;

	while ( *range_map )
	{
		uint8_t*  first = raw;
		uint8_t*  last = raw + *range_map - 1;

		while ( first < last )
		{
			uint8_t tmp = *first;
			*first = *last;
			*last = tmp;
			++first;
			--last;			
		}

		raw += *range_map;
		++range_map;
	}	
}
/*
string WordToBitString( uint32_t w )
{
	const char* bitrep[] = 
	{
		"0000", "0001", "0010", "0011", 
		"0100", "0101", "0110", "0111", 
		"1000", "1001", "1010", "1011", 
		"1100", "1101", "1110", "1111"
	};

	string res = "0b";
	for ( ptrdiff_t i = 0; i < 8; i++ )
	{
		res += bitrep[(w>>28)&0xF]; 
		w <<= 4;
	}
	return res;
	
}*/


//
//template <class Container, class Pred>
//std::vector<std::pair<typename Container::const_iterator, typename Container::const_iterator>>
//	find_ranges_if( const Container& c, Pred p )
//{
//	std::vector<std::pair<typename Container::const_iterator, typename Container::const_iterator>> result;
//	/*auto it = std::find_if( c.begin(), c.end(), p );
//	auto section_begin = it;
//	bool last_section = true;*/
//
//	auto first	= c.cbegin();
//	auto last	= c.cbegin();
//
//	do 
//	{
//		first	= std::find_if( first, c.cend(), p );
//		last	= std::find_if( first, c.cend(), p );
//
//		if ( 0 != std::distance( first, last ) )
//		{
//			result.push_back( std::make_pair(first, last) );
//
//			first = last;
//		}
//		else
//		{
//			break;
//		}
//	} while ( first != c.cend() );
//
//	/*while (it != c.end())
//	{
//		if ( p(*it) != last_section )
//		{
//			result.push_back( std::make_pair(section_begin, it) );
//			section_begin = it;
//			last_section = !last_section;
//		}
//		++it;
//	}
//
//	result.push_back( std::make_pair(it, c.end()) );*/
//
//	return result;
//}




/*
struct Ticker
{
	std::vector<uint64_t> ticks_;
	uint64_t freq_;
	uint64_t t0_;

	Ticker() 
		: freq_(0),
		t0_(0)
	{
		ticks_.reserve(32);
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq_);
		QueryPerformanceCounter((LARGE_INTEGER*)&t0_);
	}

	void Tick()
	{
		uint64_t t1 = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&t1);
		ticks_.push_back(t1 - t0_);
		t0_ = t1;
	}

	void Reset()
	{
		ticks_.clear();
		QueryPerformanceCounter((LARGE_INTEGER*)&t0_);
	}

	string Stats()
	{
		std::ostringstream oss;
		std::for_each( ticks_.cbegin(), ticks_.cend(), [&oss](uint64_t t){ oss << t << ", "; });
		return oss.str();
	}
};*/

struct spu_program_t
{
	size_t EntryPoint;
	size_t VirtualBaseAddress;
	std::vector<uint32_t> Binary;
};

struct spu_data_t
{
	size_t virtual_offset;
	vector<uint8_t> data;
};

struct spu_branch_t
{
	//uint32_t BranchFrom;
	int32_t  AdvanceAmount;
	uint32_t BranchBlockBegin;
	uint32_t BranchBlockEnd;
};

struct spu_function_t
{
	uint32_t begin;
	uint32_t end;
};

struct spu_basic_block_t
{
	size_t first, last;
};


//string RawToCode( const spu_data_t* r, string name )
//{
//	std::ostringstream oss;
//	oss << "static const size_t " << name << "_length = " << r->data.size()*4 << ";\n\n";
//	oss << "static uint32_t " << name << "_data[] = \n{\n";
//
//	auto begin	= (uint32_t*)r->data.data();
//	auto end	= begin + r->data.size()*4;
//
//	size_t counter = 0;
//	while(begin != end)
//	{
//		oss << "0x" << std::setw(8) << std::setfill('0') << std::hex << *begin++ << ", ";
//
//		if ( 0 == (counter++ % 4) )
//		{
//			oss << "\n";
//		}
//	}
//
//	oss << "};\n\n";
//
//	return oss.str();
//}

bool operator==(const spu_function_t& lhs, const spu_function_t& rhs)
{
	return 0 == memcmp( &lhs, &rhs, sizeof(lhs) );
}

bool operator<(const spu_function_t& lhs, const spu_function_t& rhs)
{
	return lhs.begin < rhs.begin;
}

bool operator<(const spu_branch_t& lhs, const spu_branch_t& rhs)
{
	return lhs.BranchBlockBegin < rhs.BranchBlockBegin;
}

struct spu_info_t
{
	std::map<string, vector<uint32_t>> heuristics;
	vector<spu_branch_t>				staticBranches;
	vector<spu_function_t>				FunctionRanges;
	std::map<uint32_t, string>			functionSymbols;
	std::map<uint32_t, uint32_t>		jumps;
	std::map<uint32_t, string>			jumpSymbols;
};



spu_program_t spuExtractProgram( ElfFile<ELF_32_traits_t>& elfFile )
{
	const size_t ELFPD_EXECUTABLE_OFFSET = 0;

	assert( 0 != elfFile.PData_.size() );	
	assert( 0 != elfFile.ProgramHeaders_.size() );
	const range_t RawData = elfFile.PData_[ELFPD_EXECUTABLE_OFFSET];
	assert( 0 == RawData.size()%4 );

	spu_program_t result = 
	{ 
		elfFile.HeaderLE_.e_entry,
		elfFile.ProgramHeaders_[ELFPD_EXECUTABLE_OFFSET].p_vaddr, 		
		vector<uint32_t>( (const uint32_t*)RawData.begin(), (const uint32_t*)RawData.end() )
	};
	return result;
}

spu_data_t spuExtractData( ElfFile<ELF_32_traits_t>& ElfFile )
{
	const size_t ELFPD_DATA_OFFSET = 1;

	assert( 0 != ElfFile.PData_.size() );	
	assert( 0 != ElfFile.ProgramHeaders_.size() );
	if (ElfFile.PData_.size() == 3)
	{
		const range_t RawData = ElfFile.PData_[ELFPD_DATA_OFFSET];
		assert( 0 == RawData.size()%4 );

		spu_data_t result = 
		{			
			ElfFile.ProgramHeaders_[ELFPD_DATA_OFFSET].p_vaddr,
			vector<uint8_t>((const uint8_t*)RawData.begin(), (const uint8_t*)RawData.end())
		};
		return result;
	}

	spu_data_t result;
	result.virtual_offset = 0;
	return result;
}

void spuSwitchEndianness( spu_program_t* program )
{
	std::transform( program->Binary.cbegin(), program->Binary.cend(), program->Binary.begin(),
		[](uint32_t off)->uint32_t
	{
		auto tmp = off;
		ReverseBytesInRange(&tmp, "\4");
		return tmp;
	});
}

void spuSwitchEndianness( spu_data_t* data )
{
	/*std::transform( data->data.cbegin(), data->data.cend(), data->data.begin(),
		[](__m128 dat)->__m128
	{
		auto tmp = dat;
		ReverseBytesInRange(&tmp, "\x10");
		return tmp;
	});*/
}

void spuGatherProgramHeuristics( spu_info_t* info, const spu_program_t* program )
{
	uint32_t i = 0;
	std::for_each( program->Binary.cbegin(), program->Binary.cend(),
		[info, &i](uint32_t code)
	{		
		info->heuristics[spu_decode_op_mnemonic(code)].push_back(i++);
	});

	info->functionSymbols[(program->EntryPoint - program->VirtualBaseAddress)>>2] = "main";
}


std::vector<size_t> spuFindSPUOffsetsInELFFile( const string& rawFile )
{
	const uint8_t ELF_SPU_ARCH_ID = 23;
	std::vector<size_t> ELFOffsets;
	for ( size_t off = 0; off != string::npos; )
	{
		const Elf32_Ehdr* eh = (Elf32_Ehdr*)&rawFile[off];
		if ( ELF_SPU_ARCH_ID == ((uint8_t*)&eh->e_machine)[1] )
		{
			ELFOffsets.push_back( off );
		}
		off += sizeof(Elf32_Ehdr);
		off = rawFile.find(ELFMAG, off);
	}
	return ELFOffsets;
}

	
template<class Pred>
spu_basic_block_t FindBasicBlock( size_t begin, size_t end, const spu_program_t* program, Pred IsBrach )
{
	spu_basic_block_t NewBlock;
	NewBlock.first = begin;

	while ( begin != end && !IsBrach(program->Binary[begin]) )
	{
		++begin;
	}

	if ( begin != end )
	{
		NewBlock.last = ++begin;
	}
	else
	{
		NewBlock.first = NewBlock.last = 0;
	}

	return NewBlock;
};

vector<uint32_t> spuGatherStaticCallTargets( const spu_program_t* program, spu_info_t* info )
{
	auto& program_local = program;

	const auto& StaticCalls = info->heuristics["brsl"];

	std::set<uint32_t> StaticCallTargets;
	std::transform( StaticCalls.cbegin(), StaticCalls.cend(), std::inserter(StaticCallTargets, StaticCallTargets.end()),
		[program](uint32_t IOffset)->uint32_t
	{
		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[IOffset]);
		return IOffset + OPComponents.IMM;
	});

	std::transform( StaticCallTargets.cbegin(), StaticCallTargets.cend(), std::back_inserter(info->FunctionRanges),
		[](uint32_t FnBeginOffset)->spu_function_t
	{
		const spu_function_t result = { FnBeginOffset, 0 };
		return result;
	});

	if ( 0 < info->FunctionRanges.size() )
	{
		std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end() - 1,
			[]( spu_function_t& FnRange )
		{
			auto Next = (&FnRange + 1);
			FnRange.end = Next->begin;
		});
		info->FunctionRanges.back().end = program->Binary.size();
	}	

	//////////////////////////////////////////////////////////////////////////


	auto IsNOP = [](uint32_t op)->bool
	{
		return 1 == (op >> 21) || 513 == (op >> 21);
	};

	auto IsSTOP = [](uint32_t op)->bool
	{
		return 0 == (op >> 21) || 320 == (op >> 21);
	};

	auto IsUnconditionalReturn = [](uint32_t op)->bool
	{
		if ( 0x1a8 == spu_decode_op_opcode(op) ) // bi
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
			if ( 0 == OPComponents.RA )
			{
				return true;
			}
		}
		return false;
	};

	auto IsConditionalReturn = [](uint32_t op)->bool
	{
		/*const uint32_t opcode = op >> 21;
		if ( 296 == opcode || 297 == opcode || 298 == opcode || 299 == opcode || 424 == opcode ) // bi*
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
			if ( 0 == OPComponents.RA )
			{
				return true;
			}
		}*/

		/* Indirect conditional jumps to the link register is equivalent to:

			if( condition )
				return;

			Indirect unconditional jumps to the link register is equivalent to:

			return;
		*/

		switch(spu_decode_op_opcode(op))
		{
		case 0x128: // biz
		case 0x129: // binz
		case 0x12a: // bihz
		case 0x12b: // bihnz
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
				if ( 0 == OPComponents.RA )
				{
					return true;
				}
			}
		default:
			return false;
		}
	};

	auto GetJumpAmount = [](uint32_t op) -> int32_t
	{
		switch(spu_decode_op_opcode(op))
		{
		case 0x100: // brz
		case 0x108: // brnz
		case 0x110: // brhz
		case 0x118: // brhnz
		case 0x190: // br
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
				return (int32_t)OPComponents.IMM;
			}
		default:
			return 0;
		}
	};

	auto IsBrach = [](uint32_t op)->bool
	{
		//uint32_t opcode = spu_decode_op_opcode(op);
		//const string BR_I16[] = { 0x100, 0x108, 0x110, 0x118, 0x180, /*0x188,*/ 0x190, /*0x198*/ };
		//auto a = std::count(BR_I16, BR_I16 + _countof(BR_I16), op >> 23 );

		//const string BR_RR[] = { 0x1a8, /*0x1a9,*/ 0x1aa, /*0x1ab,*/ 0x128, 0x129, 0x12a, 0x12b };
		//auto b = std::count(BR_RR, BR_RR + _countof(BR_RR), op >> 21 );

		string mnem = spu_decode_op_mnemonic( op );

		const string BR_I16[] = { "br", /*"brsl",*/ "bra", /*"brasl",*/ "brz", "brnz", "brhz", "brhnz" };
		auto a = std::count(BR_I16, BR_I16 + _countof(BR_I16), mnem );

		const string BR_RR[] = { "bi", /*"bisl",*/ /*"bisled",*/ "iret", "biz", "binz", "bihz", "bihnz" };
		auto b = std::count(BR_RR, BR_RR + _countof(BR_RR), mnem );

		return 0 != (a + b);
	};

	vector<spu_basic_block_t> blocks;

	

	

	auto FindBasicBlockInRange = [&blocks, program_local, IsBrach, GetJumpAmount]( size_t begin, size_t end )
	{
		do
		{
			spu_basic_block_t NewBlock = FindBasicBlock( begin, end, program_local, IsBrach );

			if ( NewBlock.first != NewBlock.last )
			{
				blocks.push_back(NewBlock);
				const uint32_t LastOp = program_local->Binary[NewBlock.last - 1];

				begin = NewBlock.last - 1;
				begin += GetJumpAmount(LastOp);

				if ( begin == NewBlock.first )
				{
					begin = NewBlock.last;
				}
			}
			else
			{
				break;
			}

		} while ( begin != end );
	};

	
	// skip SPU GUID
	FindBasicBlockInRange( 4, 24 );

	auto& info_local = info;

	vector<spu_basic_block_t> blocks2;

	/*std::for_each( blocks.begin(), blocks.end(),
		[info_local, program_local, &blocks2](const spu_basic_block_t& block)
	{
		const uint32_t LastOP = program_local->Binary[block.last-1];
		if ( 0x190 == spu_decode_op_opcode(LastOP) ) // br
		{
			const uint32_t JumpTo = info_local->jumps[block.last-1];
			const spu_basic_block_t NewBlock = FindBasicBlock( JumpTo, program_local->Binary.size() );
			if ( NewBlock.first != NewBlock.last )
			{
				blocks2.push_back(NewBlock);
			}
		}
	});*/


	/*auto InsideBranch = [](uint32_t op, const spu_branch_t& br)->bool
	{
		return op >= br.BranchBlockBegin && op < br.BranchBlockEnd;
	};*/

// 	/*auto Instructions = program->Binary;
// 
// 	std::for_each( info->staticBranches.begin(), info->staticBranches.end(),
// 		[&Instructions](const spu_branch_t& br)
// 	{
// 		auto i = br.BranchBlockBegin;
// 		while ( i != br.BranchBlockEnd )
// 			Instructions[i++] = MASK_IGNORED;
// 	});
// 
// 	std::transform( Instructions.begin(), Instructions.end(), Instructions.begin(),
// 		[=](uint32_t op)->uint32_t
// 	{
// 		if ( IsJumpToLR(op) )
// 			return MASK_RETURN;
// 		else if ( IsNOP(op) )
// 			return MASK_NOP;
// 		else if ( IsSTOP(op) )
// 			return MASK_STOP;
// 		else
// 			return op;
// 	});
// 
// 	std::set<spu_function_t> FnSimple;
// 	std::set<spu_function_t> FnMultiReturn;
// 	std::set<spu_function_t> FnNoReturn;
// 	std::set<spu_function_t> WUT;
// 
// 	
// 
// 	std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end(),
// 		[&](const spu_function_t& fn)
// 	{
// 		auto fnb = Instructions.begin() + fn.begin;
// 		auto fne = Instructions.begin() + fn.end;
// 
// 		auto RetCount = std::count( fnb, fne, MASK_RETURN );
// 		auto STOPCount = std::count( fnb, fne, MASK_STOP );
// 
// 		/ *std::set<uint32_t> Jumps;
// 
// 		auto i = 0;
// 		std::for_each( fnb, fne, 
// 			[&](uint32_t op)
// 		{
// 			if ( 0x64 == (op >> 23) )
// 			{
// 				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
// 				if ( OPComponents.IMM < 0 )
// 				{
// 					if (abs(OPComponents.IMM) > i)
// 					{
// 						WUT.insert(fn);
// 					}
// 				}
// 				else
// 				{
// 					if (fnb + i + OPComponents.IMM >= fne)
// 					{
// 						WUT.insert(fn);
// 					}
// 				}				
// 			}
// 			++i;
// 		});* /
// 
// 		switch ( RetCount )
// 		{
// 		case 0:		
// 			{
// 				FnNoReturn.insert(fn);		break;
// 			}
// 		case 1:	
// 			{
// 				// handle case when there is .data/.bss after return. mostly at the end of the binary
// 				auto ret_it = std::find( fnb, fne, MASK_RETURN );				
// 				if ( ret_it != fne )
// 				{
// 					++ret_it;
// 					if ( MASK_NOP == *ret_it )
// 						++ret_it;
// 						
// 					const spu_function_t NewRange = { 
// 						(uint32_t)std::distance(Instructions.begin(), fnb), 
// 						(uint32_t)std::distance(Instructions.begin(), ret_it) };
// 					FnSimple.insert(NewRange);	
// 				}
// 				break;
// 			}
// 		default:	
// 			{
// 				auto b = fnb;
// 
// 				while ( b != fne )
// 				{
// 					auto e = std::find( b, fne, MASK_RETURN );
// 					
// 					if ( e != fne )
// 					{		
// 						++e;
// 						if ( MASK_NOP == *e )
// 							++e;
// 					}
// 
// 					const spu_function_t NewRange = { 
// 						(uint32_t)std::distance(Instructions.begin(), b), 
// 						(uint32_t)std::distance(Instructions.begin(), e) };
// 					FnMultiReturn.insert(NewRange);	
// 					b = e;
// 				};
// 				break;
// 			}
// 		};
// 	});
// 
// 	info->FunctionRanges.clear();
// 	std::copy( FnSimple.cbegin(), FnSimple.cend(), std::back_inserter(info->FunctionRanges));
// 	std::copy( FnMultiReturn.cbegin(), FnMultiReturn.cend(), std::back_inserter(info->FunctionRanges));
// 	std::copy( FnNoReturn.cbegin(), FnNoReturn.cend(), std::back_inserter(info->FunctionRanges));*/

	/*auto FnRangeBegin = Instructions.begin() + 4; // skip SPU GUID
	auto FnRangeEnd = FnRangeBegin;

	vector<spu_function_t> FnRanges;
	
	while ( FnRangeEnd != Instructions.end() )
	{
		FnRangeEnd = std::find( FnRangeBegin, Instructions.end(), MASK_RETURN );
		if ( FnRangeEnd != Instructions.end() )
		{
			do 
			{
				++FnRangeEnd;
			} while ( FnRangeEnd != Instructions.end() && *FnRangeEnd == MASK_NOP );
			spu_function_t NewFnRange = { std::distance( Instructions.begin(), FnRangeBegin), std::distance( Instructions.begin(), FnRangeEnd) };
			FnRanges.push_back(NewFnRange);
			FnRangeBegin = FnRangeEnd;
		}
	}*/

	/*vector<spu_function_t> FnUniq = FnRanges;
	std::copy( info->FunctionRanges.begin(), info->FunctionRanges.end(), std::back_inserter(FnUniq) );
	std::sort( FnUniq.begin(), FnUniq.end() );
	auto NewEnd = std::unique(FnUniq.begin(), FnUniq.end() );
	FnUniq.swap( vector<spu_function_t>(FnUniq.begin(), NewEnd));*/
	//std::unique_copy()

	/*vector<spu_function_t> FnUnref;

	auto ScanFnRangeForSubranges = [=, &FnUnref](spu_function_t& FnRange)
	{
		auto NextBrRange = std::find_if(info->staticBranches.begin(), info->staticBranches.end(), 
			[&FnRange](const spu_branch_t& br)->bool
		{
			return FnRange.begin <= br.BranchBlockBegin;
		});


		uint32_t i = FnRange.begin;
		uint32_t FnBegin = FnRange.begin;

		while ( i < FnRange.end )
		{			
			if ( i == NextBrRange->BranchBlockBegin )	// skip branches
			{
				while ( InsideBranch(i, *NextBrRange) )
					++NextBrRange;
				i = (NextBrRange-1)->BranchBlockEnd;
			}
			else if ( IsJumpToLR(program->Binary[i]) ) // function local level return found
			{			
				// anything else after it?
				auto NextNonNOPInstruction = i+1; 
				while ( IsNOP(program->Binary[NextNonNOPInstruction]) )
					++NextNonNOPInstruction;
				
				spu_function_t NewFnRange = { FnBegin, NextNonNOPInstruction };
				FnUnref.push_back(NewFnRange);
				FnBegin = NextNonNOPInstruction;	
				i = NextNonNOPInstruction;
			}
			else
			{
				++i;
			}		
		}
	};*/

	/*do 
	{
		const size_t NewFnCount = FnUnref.size();
		std::copy( FnUnref.begin(), FnUnref.end(), std::back_inserter(info->FunctionRanges) );
		FnUnref.clear();
		std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end(), ScanFnRangeForSubranges );
				
	} while ( !FnUnref.empty() );

	
	std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end(), ScanFnRangeForSubranges );	*/

	/*vector<uint32_t> nops, rets;
	uint32_t i = 0;
	std::for_each( program->Binary.cbegin(), program->Binary.cend(),
		[&](uint32_t op)
	{
		if ( IsNOP(op) )
		{
			nops.push_back(0x3000 + i*4);
		}
		if ( IsJumpToLR(op) )
		{
			rets.push_back(0x3000 + i*4);
		}

		++i;
	});*/

	return vector<uint32_t>( StaticCallTargets.begin(), StaticCallTargets.end() );
}

void spuGatherStaticBranches( const spu_program_t* program, spu_info_t* info )
{
	auto spuGatherStaticBranchesAux = [program, info](const string& branchMnem)
	{
		const auto& branches = info->heuristics[branchMnem];
		std::transform( branches.cbegin(), branches.cend(), std::back_inserter(info->staticBranches),
			[=](const uint32_t InstructionOffset)->spu_branch_t
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[InstructionOffset]);
			const uint32_t			BranchFrom = InstructionOffset;
			const uint32_t			BranchBlockBegin	= (OPComponents.IMM > 0) ? BranchFrom : (BranchFrom + (int32_t)OPComponents.IMM);
			const uint32_t			BranchBlockEnd		= (OPComponents.IMM > 0) ? (BranchFrom + (int32_t)OPComponents.IMM) : (BranchFrom + 1);
			const spu_branch_t		br = { /*BranchFrom,*/ (int32_t)OPComponents.IMM, BranchBlockBegin, BranchBlockEnd };
			return br;
		});
	};

	spuGatherStaticBranchesAux("brz");
	spuGatherStaticBranchesAux("brnz");
	spuGatherStaticBranchesAux("brhz");
	spuGatherStaticBranchesAux("brhnz");

	std::sort( info->staticBranches.begin(), info->staticBranches.end() );

	auto Overlaps = []( const spu_branch_t& lhs, const spu_branch_t& rhs ) -> bool
	{
		//if ( lhs.BranchBlockBegin <= rhs.BranchBlockBegin )
			return (lhs.BranchBlockBegin <= rhs.BranchBlockBegin) && (rhs.BranchBlockBegin < lhs.BranchBlockEnd);
		//else
			//return rhs.BranchBlockEnd < lhs.BranchBlockBegin;
	};

	decltype(info->staticBranches) BrIfVector;
	auto first1 = info->staticBranches.begin();
	auto first2 = info->staticBranches.begin() + 1;
	auto last = info->staticBranches.end() - 1;

	do
	{
		if ( (first1->AdvanceAmount > 0) && !Overlaps( *first1, *first2 ) )
		{
			BrIfVector.push_back(*first1);
		}
		++first1;
		++first2;
	} while ( first1 != last );

	if ( info->staticBranches.back().AdvanceAmount > 0 )
	{
		BrIfVector.push_back(*first1);
	}

	/*decltype(info->staticBranches) BrIfComplex;
	std::set_difference( info->staticBranches.begin(), info->staticBranches.end(), 
		BrIfVector.begin(), BrIfVector.end(), std::inserter( BrIfComplex, BrIfComplex.end() ) );*/
}

void spuDumpInstrTextToFile( const string& path, const vector<string>& Prefix, const spu_program_t& program, const vector<string>& Postfix )
{
	assert( Prefix.size() == program.Binary.size() && program.Binary.size() == Postfix.size() );

	std::ofstream fout_intrin( path );
	vector<string> FinalText(program.Binary.size(), "");
	
	if ( fout_intrin.is_open() )
	{
		const ptrdiff_t Count = program.Binary.size();
		
		for ( ptrdiff_t i = 0; i < Count; ++i )
		{
			FinalText[i] = Prefix[i] + spu_make_pseudo((SPU_INSTRUCTION&)program.Binary[i], program.VirtualBaseAddress + i*4) + Postfix[i];
		}
		std::copy( FinalText.cbegin(), FinalText.cend()-1, std::ostream_iterator<string>(fout_intrin, "\n") );
		fout_intrin << FinalText.back();
		/*std::transform( program.Binary.cbegin(), program.Binary.cend(), std::ostream_iterator<string>(fout_intrin, "\n"),
			[&](uint32_t op)->string
		{
			string result = Prefix[i] + spu_make_pseudo((SPU_INSTRUCTION&)op, 0) + Postfix[i];
			++i;
			return result;			
		});*/
	}
}

void spuBuildFunctionSymbols( spu_info_t* info, const vector<uint32_t>& fn_begins )
{
	std::ostringstream oss;
	std::for_each( fn_begins.cbegin(), fn_begins.cend(), 
		[info](uint32_t off)
	{
		std::ostringstream oss;
		oss << "sub" << std::hex << std::setw(8) << std::setfill('0') << off*4;
		info->functionSymbols[off] = oss.str();
	} );
}

void spuGatherJumps( const spu_program_t* program, spu_info_t* info )
{
	const auto& JumpsRel = info->heuristics["br"];

	std::transform( JumpsRel.cbegin(), JumpsRel.cend(), std::inserter(info->jumps, info->jumps.end()),
		[=](const uint32_t InstructionOffset)->std::pair<uint32_t, uint32_t>
	{
		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[InstructionOffset]);
		const uint32_t			JumpFrom	 = InstructionOffset;
		const uint32_t			JumpTo		 = InstructionOffset + (int32_t)OPComponents.IMM;		
		return std::make_pair( JumpFrom, JumpTo );
	});

	/*std::set<uint32_t> JumpTargets;
	std::transform( info->jumps.cbegin(), info->jumps.cend(), std::inserter(JumpTargets, JumpTargets.end()),
		[](const std::pair<uint32_t, uint32_t>& it)->uint32_t
	{
		return it.second;
	});

	std::transform( JumpTargets.begin(), JumpTargets.end(), std::inserter(info->jumpSymbols, info->jumpSymbols.end()),
		[](uint32_t JumpTo)->std::pair<uint32_t, string>
	{
		std::ostringstream oss;
		oss << "LB" << std::hex << std::setw(8) << std::setfill('0') << (JumpTo*4);
		return std::make_pair( JumpTo, oss.str() );
	});	*/
}


void spuGatherLoads( const spu_program_t* program, spu_info_t* info )
{	
	auto GatherAbsAddresses = [program, info]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& AbsMemOPs = info->heuristics[mnem];
		auto program_local = program;

		std::transform( AbsMemOPs.begin(), AbsMemOPs.end(), std::inserter(Addresses, Addresses.end()), 
			[program_local]( uint32_t InstructionOffset )->uint32_t
		{
			const uint32_t LSLR = 0x3ffff & -16;
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program_local->Binary[InstructionOffset]);
			return ((uint32_t)OPComponents.IMM << 2) & LSLR;
		});
	};

	auto GatherRelAddresses = [program, info]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& RelMemOPs = info->heuristics[mnem];
		auto program_local = program;

		std::transform( RelMemOPs.begin(), RelMemOPs.end(), std::inserter(Addresses, Addresses.end()), 
			[program_local]( uint32_t InstructionOffset )->uint32_t
		{
			const uint32_t LSLR = 0x3ffff & -16;
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program_local->Binary[InstructionOffset]);
			return (program_local->VirtualBaseAddress + (InstructionOffset*4) + ((int32_t)OPComponents.IMM << 2)) & LSLR;
		});
	};

	std::set<uint32_t> AbsLoadTargets;
	std::set<uint32_t> RelLoadTargets;
	std::set<uint32_t> AbsStoreTargets;
	std::set<uint32_t> RelStoreTargets;

	GatherAbsAddresses( "lqa", AbsLoadTargets );
	GatherRelAddresses( "lqr", RelLoadTargets );
	GatherAbsAddresses( "stqa", AbsStoreTargets );
	GatherRelAddresses( "stqr", RelStoreTargets );
}

void PrintInstrAdresses( uint32_t BaseAddress, vector<string>& instrInfoText )
{
	std::for_each( instrInfoText.begin(), instrInfoText.end(), 
		[&BaseAddress]( string& s )
	{
		std::ostringstream oss;
		oss << std::hex << std::setw(8) << std::setfill('0') << BaseAddress << " ";
		BaseAddress += 4;
		s = oss.str();
	} );
}


void spu_execute( SPU_t* )
{	
	//GPR(6) = si_ila(0x0003ffd0/*262096*/);
	//// HINT: hbrr PC + 0x00000017/*23*/, PC + 0xffffffe7/*-25*/;
	//GPR(12) = si_ila(0x00010203/*66051*/);
	//GPR(7) = si_fsmbi(0x00000f00/*3840*/);
	//GPR(18) = si_ai(GPR(6),0x00000020/*32*/);
	//GPR(81) = si_shlqbyi(GPR(3),0x00000000/*0*/);
	//GPR(15) = si_ila(0x00038f80/*233344*/);
	//GPR(80) = si_shlqbyi(GPR(4),0x00000000/*0*/);
	//GPR(16) = si_il(0x00000000/*0*/);
	//si_stqd(GPR(18),GPR(6),0x00000000/*0*/);
	//GPR(8) = si_sf(GPR(15),GPR(6));
	//GPR(14) = si_lqr(0x000011f9/*4601*/);
	//si_stqr(GPR(16),0xfffff3d4/*-3116*/);
	//GPR(1) = si_selb(GPR(6),GPR(8),GPR(7));
	//GPR(13) = si_ai(GPR(1),0x00000030/*48*/);
	//GPR(11) = si_rotqbyi(GPR(14),0x00000004/*4*/);
	//si_stqr(GPR(13),0x0000117c/*4476*/);
	//GPR(10) = si_shufb(GPR(11),GPR(11),GPR(12));
	//GPR(3) = si_ai(GPR(10),0xffffffd0/*-48*/);
	//GPR(9) = si_clgt(GPR(8),GPR(3));
	//GPR(4) = si_clgti(GPR(3),0x00000000/*0*/);
	//GPR(2) = si_and(GPR(9),GPR(4));
	//GPR(5) = si_selb(GPR(8),GPR(3),GPR(2));
	//GPR(1) = si_selb(GPR(6),GPR(5),GPR(7));
	//CALL_REL(0xffffffd0/*-48*/);
	//GPR(4) = si_ori(GPR(80),0x00000000/*0*/);
	//GPR(3) = si_ori(GPR(81),0x00000000/*0*/);
	//CALL_REL(0x00000119/*281*/);
	//CALL_REL(0x000006da/*1754*/);
	
	memmap_t* bearelf = open("D:\\Torrents\\BLES00945\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf");
	
	string fraw;
	fraw.resize(size(bearelf));
	memcpy( &fraw[0], begin(bearelf), size(bearelf) );
	close(bearelf);
	
	vector<size_t> ELF_offsets = spuFindSPUOffsetsInELFFile(fraw);

	std::for_each( ELF_offsets.cbegin(), ELF_offsets.cbegin()+1,
		[](uint32_t off)
	{
		ElfFile<ELF_32_traits_t> SPU = ElfFile<ELF_32_traits_t>("D:\\Torrents\\BLES00945\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf", off);

		spu_program_t program = spuExtractProgram( SPU );
		spu_data_t data = spuExtractData( SPU );
		spuSwitchEndianness( &program );
		spuSwitchEndianness( &data );
		spu_info_t info;
		spuGatherProgramHeuristics( &info, &program );
		spuGatherStaticBranches( &program, &info );		
		spuGatherJumps( &program, &info );
		spuGatherLoads( &program, &info );
		const vector<uint32_t> fn_begins = spuGatherStaticCallTargets( &program, &info );
		spuBuildFunctionSymbols( &info, fn_begins );
		
		// write out extra info
		vector<string> instrInfoTextPrefix( program.Binary.size(), "" );
		vector<string> instrInfoTextPostfix( program.Binary.size(), "" );		
		vector<string> instrInfoText( program.Binary.size(), "" );

		// instruction addresses
		PrintInstrAdresses( (uint32_t)program.VirtualBaseAddress, instrInfoText );
		
		// function calls
		std::for_each( info.functionSymbols.cbegin(), info.functionSymbols.cend(), 
			[&]( const std::pair<uint32_t, string>& it )
		{
			if (it.first < instrInfoText.size())
			{
				instrInfoText[it.first] += string("begin ") + it.second;
			}
			
			//instrInfoTextPrefix[it.first] = string("void ") + it.second + string("()\n");
		} );

		// labels
		std::for_each( info.jumpSymbols.cbegin(), info.jumpSymbols.cend(), 
			[&instrInfoText]( const std::pair<uint32_t, string>& it )
		{
			instrInfoText[it.first] += it.second + ":";
		} );

		// gotos
		std::for_each( info.jumps.cbegin(), info.jumps.cend(), 
			[&instrInfoText, &info]( const std::pair<uint32_t, uint32_t>& it )
		{
			instrInfoText[it.first].append("goto ").append( info.jumpSymbols[it.second] ).append(":");
		} );

		// comment out stack load/store
		auto HideStackOPs = [&info, &program, &instrInfoTextPrefix](const std::string& mnem)
		{
			const auto& instr = info.heuristics[mnem];
			auto program_local = &program;
			auto instrInfoTextPrefix_local = &instrInfoTextPrefix;

			std::for_each( instr.cbegin(), instr.cend(),
				[program_local, instrInfoTextPrefix_local]( uint32_t fn_call_instr_offset )
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program_local->Binary[fn_call_instr_offset]);
				if ( 1 == OPComponents.RA )
				{
					instrInfoTextPrefix_local->at(fn_call_instr_offset) = string("//") + instrInfoTextPrefix_local->at(fn_call_instr_offset);
				}
			} );
		};

		HideStackOPs("lqd");
		HideStackOPs("lqx");
		HideStackOPs("stqd");
		HideStackOPs("stqx");
		
		// indent functions
		std::for_each( info.FunctionRanges.begin(), info.FunctionRanges.end(),
			[&instrInfoTextPrefix]( const spu_function_t& fn )
		{
			auto beg = instrInfoTextPrefix.begin() + fn.begin + 1;
			auto end = instrInfoTextPrefix.begin() + fn.end;		

			while ( beg != end )
			{
				beg->append("\t");
				++beg;
			}

		});

		// indent static branches
		/*std::for_each( info.staticBranches.begin(), info.staticBranches.end(),
			[&instrInfoTextPrefix, &instrInfoTextPostfix]( const spu_branch_t& br )
		{
			if (br.AdvanceAmount > 0)
			{
				auto beg = &instrInfoTextPrefix[br.BranchBlockBegin];
				auto end = &instrInfoTextPrefix[br.BranchBlockEnd];		

				while ( beg != end )
				{
					beg->append("\t");
					++beg;
				}
			}
		});*/

		/*
		// indent jumps
		std::for_each( info.jumps.cbegin(), info.jumps.cend(), 
			[&instrInfoTextPrefix]( const std::pair<uint32_t, uint32_t>& it )
		{
			if ( it.first > it.second )
			{
				auto beg = &instrInfoTextPrefix[it.second];
				auto end = &instrInfoTextPrefix[it.first];
				while ( beg != end )
				{
					beg->append("\t");
					++beg;
				}
			}			
		} );*/

		/*const auto& fn_calls = info.heuristics["brsl"];
		std::for_each( fn_calls.cbegin(), fn_calls.cend(), 
			[&instrInfoText, &info, &program]( uint32_t fn_call_instr_offset )
		{
			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program.Binary[fn_call_instr_offset]);
			instrInfoText[fn_call_instr_offset].append( "call " ).append( info.functionSymbols[fn_call_instr_offset + (int32_t)OPComponents.IMM] );
		} );*/

		static uint32_t counter = 0;
		
		std::ostringstream path;
		path << "spu_code_" << counter++;
		std::ofstream fout_info((path.str() + ".info").c_str());
		spuDumpInstrTextToFile((path.str() + ".intr"), instrInfoTextPrefix, program, instrInfoTextPostfix );
		

		if ( fout_info.is_open() )
		{
			std::copy( instrInfoText.cbegin(), instrInfoText.cend(), std::ostream_iterator<string>(fout_info, "\n") );
		}
	});
}

void spu_parse_file( SPU_t* targetSPU, const char* /*path */)
{		
	spu_execute( targetSPU );
}

string BytestreamToHexString( const void* data, size_t length, uint32_t stride )
{
	string out, hex_dump, txt_dump;

	for ( size_t i = 0; i < length; ++i )
	{	
		if ( 0 != i
			&& 0 == (i % stride) )
		{
			out += hex_dump;
			out += " | ";
			out += txt_dump;
			out += "\n";
			hex_dump.clear();
			txt_dump.clear();
		}

		uint8_t byte = ((const uint8_t*)data)[i];
		hex_dump.push_back( "0123456789ABCDEF"[0xF & (byte >> 4)] );
		hex_dump.push_back( "0123456789ABCDEF"[0xF & byte] );
		hex_dump.push_back( ' ' );
		txt_dump.push_back( byte >= 0x20 ? (char)byte : '.' );

	}

	if ( !hex_dump.empty() )
	{
		hex_dump.resize( 16*3, ' ' );
		txt_dump.resize( 16*3, ' ' );
		out += hex_dump;
		out += " | ";
		out += txt_dump;
		out += "\n";
	}

	return out;
}