#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <iomanip>
#include <intrin.h>
#include <stack>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <memory>
#include <map>
#include <cassert>
#include "spu_idb.h"
#include "elf.h"
//#include "raw.h"
#include "elf_helper.h"
#include "basic_blocks.h"
#include "src_file.h"
#include "spu_pseudo.h"
#include "basicblock.h"
#include "string_tools.h"

using namespace std;

struct cfgnode
{
	vector<cfgnode*> pred;
	bb* block;
	vector<cfgnode*> succ;
};

void spuGatherLoads( const vector<uint32_t>& Binary, spu::op_distrib_t& OPDistrib,
					size_t VirtBase )
{	
	auto GatherAbsAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& AbsMemOPs = OPDistrib[mnem];

		std::transform( 
			AbsMemOPs.begin(), AbsMemOPs.end(), 
			std::inserter(Addresses, Addresses.end()), 
			[&]( size_t Offset )->uint32_t
		{
			const uint32_t LSLR = 0x3ffff & -16;

			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[Offset]);

			return ((uint32_t)OPComponents.IMM << 2) & LSLR;
		});
	};

	auto GatherRelAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& RelMemOPs = OPDistrib[mnem];

		std::transform( RelMemOPs.begin(), RelMemOPs.end(), 
			std::inserter(Addresses, Addresses.end()), 
			[&]( size_t Offset )->uint32_t
		{
			const uint32_t LSLR = 0x3ffff & -16;

			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[Offset]);

			return (VirtBase + (Offset*4) + ((int32_t)OPComponents.IMM << 2)) & LSLR;
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

//struct edge
//{
//	shared_ptr<bb> src;
//	shared_ptr<bb> dst;
//};
//
//struct node
//{
//	vector<shared_ptr<edge>> pred;
//	vector<shared_ptr<edge>> succ;
//	shared_ptr<bb> block;
//};

string print_bb( const bb& block )
{
	ostringstream oss;
	oss << "\"";

	for ( auto iter = block.ibegin; iter != block.iend; ++iter )
	{
		oss << spu_decode_op_mnemonic(iter->raw) << "\\n";
	}

	oss << "\"";
	return oss.str();
}

vector<uint8_t> LoadFileBin( int argc, char** argv );


struct fn
{
	const bb* entry;
	const bb* exit;
};


int main( int argc, char** argv )
{
	vector<uint8_t> ELFFile = LoadFileBin(argc, argv);

	vector<uint32_t> SPUBinary;
	size_t EntryIndex = 0;
	size_t vbase = 0;
	uint8_t* SPU0 = (uint8_t*)ELFFile.data();// + SPUELFOffsets[0];
	{
		elf::HeadersToSystemEndian( SPU0 );

		SPUBinary = elf::spu::LoadExecutable( SPU0 );

		for ( size_t i = 0; i != SPUBinary.size(); ++i )
		{
			SPUBinary[i] = _byteswap_ulong(SPUBinary[i]);
		}

		EntryIndex = elf::EntryPointIndex( SPU0 );

		vbase = elf::VirtualBaseAddr( SPU0 );
	}

	// Isolate binary from data

	SPUBinary.resize( 0x190C0 / 4 ); // FIXME hardcoded for now

	vector<spu_insn> insninfo;
	{
		spu_insn_process_bin( SPUBinary, insninfo, vbase );
	}

	spu::op_distrib_t OPDistrib;
	{
		OPDistrib = spu::GatherOPDistribution( SPUBinary );
	}

	//spu_insn_process_flags( insninfo, OPDistrib );

	set<size_t> brsl_targets = spu_get_brsl_targets(OPDistrib, insninfo, vbase);

	//t<size_t> br_targets = spu_get_br_targets(OPDistrib, insninfo);

	vector<size_t> bb_leads = spu_find_basicblock_leader_offsets(
		OPDistrib, insninfo );

	vector<bb> blocks = bb_genblocks( bb_leads, insninfo );

	bb_calctypes( blocks, insninfo );

	// helper
	map<const spu_insn*, bb*> insn2block;
	{
		for ( auto& block : blocks )
		{
			insn2block.insert(make_pair(block.ibegin, &block));
		}
	}

	{


		/*
		needs a two-phase solution.
		look for fn lead blocks and fn exit blocks.
		one can be deriver from the other.
		keep calculating the two until no more fn range is found?
		*/

		/*
		scan static fn entry points
		find the terminators right before them
		if it's sjump, see if they cross over prev or next
		if they do, it's a tailcall opt.
		*/
		//vector<size_t> fn_entry_vaddr;

		//fn_entry_vaddr.assign( brsl_targets.begin(), brsl_targets.end() );

		// check for tail call opt.

		//resolve_tailcall_opt( blocks, br_tailopt_targets );

		/***
		-Function Terminators

		Every function must have an unconditional terminator.
		This means that it has to be a point in the code that
		can not be skipped.
		Thus anything that can be skipped by either a forward
		unconditional jump (if construct) or a backward 
		unconditional jump (do-while construct) can be 
		disregarded when searching for function terminators.
		*/


		// By default, we can't assume that ever return or like will terminate
		// a function because of early exit.


		// Remove basic blocks that belong to if()/while() constructs.
		// Blocks that remain are guaranted to contain only control flow 
		// breakers or reversers that will terminate a function.
		// Returns in in() blocks are discarded.
		set<bb*> blocks_uncond;

		bb_find_unconditional_blocks( blocks, blocks_uncond, insn2block );

		set<bb*> fn_term_blocks;		

		set<bb*> fn_entry_block;
		{
			// Gather in here blocks that are right after a control flow
			// stopper or reverser.

			/*
			// ...
			return ...;
			}
			*/
			copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
				inserter(fn_term_blocks, fn_term_blocks.end()),
				[](bb*const& block) { return block->type == bbtype::ret; });

			/*
			// ...
			// suspend thread
			}
			*/
			copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
				inserter(fn_term_blocks, fn_term_blocks.end()),
				[](bb*const& block) { return block->type == bbtype::stop; });

			/*
			// ...
			while (1)
			{
			};
			}
			*/
			copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
				inserter(fn_term_blocks, fn_term_blocks.end()),
				[](bb*const& block) { return block->type == bbtype::infloop; });

			/*
			// ...
			while (...)
			{
			// ...
			return ...;
			};
			}
			*/
			copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
				inserter(fn_term_blocks, fn_term_blocks.end()),
				[](bb*const& block) { return block->type == bbtype::sjumpb; });

			fn_entry_block.insert(&blocks[0]);

			// Skip over all the lnop filler blocks
			for ( auto block : fn_term_blocks )
			{
				auto bb_fn_entry = block + 1;

				if (bb_fn_entry == (&blocks[0] + blocks.size()))
					continue;

				// skip lnop fillers between function bodies
				while (bb_fn_entry->ibegin->op == spu_op::M_LNOP 
					&& bb_fn_entry->ibegin->vaddr % 8 != 0
					&& bb_insn_count(bb_fn_entry) == 1)
				{
					++bb_fn_entry;
				}

				fn_entry_block.insert(bb_fn_entry);
			}			
		}

		// turn known fn entry blocks into vaddrs
		set<size_t> fn_entry_after_term;
		{
			fn_entry_after_term.insert(brsl_targets.begin(), brsl_targets.end());


			for ( auto block : fn_entry_block )
			{
				fn_entry_after_term.insert(block->ibegin->vaddr);
			}
		}

		// Tail Call Optimisation: jumps that jump to a function
		// without setting up stack or argument passing
		// WARNING: this will give false positives on dead code that is 
		//			jumped over

		/*
		Now only unconditional jumps remain.
		Backward jumps:
		* On one hand, these are always function terminators.
		* On the other hand, it's not obvious if they are just part of a loop
		or a TCO to a function earlier in the binary.
		* We now have a base set of function entry point. If the backwad jump target
		is earlier than the function entry point right before the jump, 
		then it's a TCO for sure.
		* Must jump to a mod8 address to be considered TCO
		Forward jumps:
		* Could be a TCO or a jump into a loop's conditional check.
		* If the target of the jump is beyond the next upcoming function entry
		point in the binary then it's sure to be a TCO.
		* Must jump to a mod8 address to be considered TCO
		*/

		auto is_dword_aligned = [](bb* block)
		{
			return 0 == block->ibegin->vaddr % 8;
		};

		set<size_t> fn_entry_candidates;

		{
			set<bb*> fn_term_block_sjumpf;
			set<bb*> TCO_blocks;
			set<bb*> TCO_targets;

			copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
				inserter(fn_term_block_sjumpf, fn_term_block_sjumpf.end()),
				[](bb*const& block) { return block->type == bbtype::sjumpf; });

			copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
				inserter(fn_term_block_sjumpf, fn_term_block_sjumpf.end()),
				[](bb*const& block) { return block->type == bbtype::sjumpb; });

			for (auto block : fn_term_block_sjumpf)
			{
				const size_t target_vaddr = block->branch->vaddr + block->branch->comps.IMM * 4;

				// 1) must jump to modulo8 address
				if (target_vaddr % 8 != 0)
					continue;

				// instant pass if it jumps to an already known fn entry
				if (fn_entry_after_term.end() != 
					find(fn_entry_after_term.begin(), fn_entry_after_term.end(), target_vaddr))
					continue;

				// 2) must not jump INTO "conditional" block
				//	  It can jump to a first block of a conditional section, however.
				//	  The latter can be a function that is just a do-while().
				bb* target_block = insn2block[block->branch + block->branch->comps.IMM];
				const bool target_in_conditional = 
					blocks_uncond.cend() == 
					find(blocks_uncond.cbegin(), blocks_uncond.cend(), target_block);
				const bool prev_in_conditional = 
					blocks_uncond.cend() == 
					find(blocks_uncond.cbegin(), blocks_uncond.cend(), target_block - 1);
				if (target_in_conditional && prev_in_conditional)
					continue;

				// 3) target must be preceded by a valid fn terminator
				bb* prev_block = target_block - 1;

				if (1 == bb_insn_count(prev_block)
					&& prev_block->ibegin->op == spu_op::M_LNOP)
				{
					--prev_block;
				}

				if (fn_term_blocks.cend() == 
					find(fn_term_blocks.cbegin(), fn_term_blocks.cend(), prev_block))
				{
					continue;
				}

				
				TCO_blocks.insert(block);
				TCO_targets.insert(target_block);
				fn_entry_candidates.insert(target_block->ibegin->vaddr);
			}
		}

		// TODO
		// iterate over fn entry pairs. they make up a possible function
		// every static unconditional that leaves its parent function
		// becomes a TCO jump and jumps to a function lead block
		// jump before fn entry or jump after itself
		// forward jumps might be jumping over dead code but we assume those were
		// elimenated at the oroginal compilation
		// this should work since every TCO jump must be preceded by an
		// unconditional function terminator
		// no chance for mistaking a TCO jump for a do..while()


		vector<fn> functions;

		for ( auto fn_entry_vaddr : fn_entry_after_term )
		{
			const spu_insn* entry_insn = &insninfo[(fn_entry_vaddr - vbase) / 4];
			const bb* entry_block = insn2block[entry_insn];
			fn new_fn = { entry_block, entry_block };
			functions.push_back(new_fn);
		}

		// adjust fn exits. skip lnops
		for (size_t ii = 0; ii < functions.size() - 1; ++ ii)
		{
			fn* function = &functions[ii];
			const fn* function_next = &functions[ii+1];
			const bb* function_exit_block = function_next->entry - 1;

			while (function_exit_block->ibegin->op == spu_op::M_LNOP 
				&& bb_insn_count(function_exit_block) == 1)
			{
				--function_exit_block;
			}

			function->exit = function_exit_block;
		}

		functions[functions.size()-1].exit = blocks.data() + blocks.size();


		for (auto function : functions)
		{
			if (function.exit->type == bbtype::sjumpb || function.exit->type == bbtype::sjumpf)
			{
				const spu_insn* target_insn = 
					function.exit->branch + function.exit->branch->comps.IMM;

				if (target_insn->vaddr < function.entry->ibegin->vaddr
					|| target_insn->vaddr >= function.exit->iend->vaddr)
				{
					fn_entry_after_term.insert(target_insn->vaddr);
				}
			}
		}

		// blocks can't be terminators either if...
		set<bb*> code_only;
		// ...they don't change the control flow.
		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(code_only, code_only.end()),
			[](bb*const& block) { return block->type == bbtype::code; });
		// ...are calls. They eventually return and continue.
		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(code_only, code_only.end()),
			[](bb*const& block) { return block->type == bbtype::scall
			|| block->type == bbtype::dcall; });
		// ...actually are known function begins.

		// ...actually are known returns. ???
		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(code_only, code_only.end()),
			[](bb*const& block) { return block->type == bbtype::ret; });
		// ...leftover if statements.

		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(code_only, code_only.end()),
			[](bb*const& block) { return block->type == bbtype::cjumpf; });
		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(code_only, code_only.end()),
			[](bb*const& block) { return block->type == bbtype::cjumpb; });
		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(code_only, code_only.end()),
			[](bb*const& block) { return block->type == bbtype::cdjump; });

		set<bb*> best_cand;
		set_difference(blocks_uncond.begin(), blocks_uncond.end(),
			code_only.begin(), code_only.end(),
			inserter(best_cand, best_cand.end()));

		set<size_t> fne_new;

		for ( auto block : best_cand )
		{
			if (block->type == bbtype::sjumpb || block->type == bbtype::sjumpf)
			{
				const size_t target_vaddr = ((block + block->branch->comps.IMM) - &blocks[0]) * 4 + vbase;
				if (target_vaddr % 8 == 0)
					fne_new.insert(target_vaddr);
			}
		}
	}

	vector<uint32_t> LS(0x40000/4);
	elf::spu::LoadImage( (uint8_t*)&LS[0], SPU0 );

	set<size_t> Ctors;
	std::vector<uint32_t> FnCalls;
	set<size_t> Text;
	set<string> TextString;
	{
		for ( size_t i = 0; i < LS.size(); i += 4 )
		{
			if ( PossibleCtorDtorList( i, LS, FnCalls ) )
			{
				Ctors.insert( (4 * i) );
			}

			if ( PossibleString( i, LS ) )
			{
				Text.insert( (4 * i) );
				TextString.insert( (const char*)LS.data() + (4 * i) );
			}
		}
	}

	auto FnRanges = spu::BuildInitialBlocks( 
		SPUBinary, OPDistrib, elf::VirtualBaseAddr(SPU0), 
		EntryIndex, FnCalls );

	spu::MakeSPUSrcFile( SPUBinary, FnRanges, 0, 
		elf::VirtualBaseAddr(SPU0), elf::EntryPointIndex(SPU0)*4 );

	return 0;
}

vector<uint8_t> LoadFileBin( int argc, char** argv )
{
	vector<uint8_t> ELFFile;
	{
		const string path( (argc > 1) ? argv[1] : 
			"F:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\lv1ldr.elf" );
		ifstream iff( path.c_str(), ios::in | ios::ate | ios::binary );
		const size_t filesize = iff.tellg();
		ELFFile.resize( filesize );
		iff.seekg( 0 );
		iff.read( (char*)ELFFile.data(), filesize );
		iff.close();
	}
	{
		if ( ELFFile.empty() )
		{
			cout << "Input file not found" << endl;
		}
	}

	return ELFFile;
}
