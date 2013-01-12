#include <algorithm>
#include <cassert>
#include "basicblock.h"
#include "spu_idb.h"

using namespace std;

vector<bb> bb_genblocks( 
	const vector<size_t>& block_leads,
	vector<spu_insn>& insninfo )
{
	vector<bb> blocks;
	vector<spu_insn*> bb_lead_insn;

	bb_lead_insn.reserve(block_leads.size());

	transform(block_leads.begin(), block_leads.end(),
		back_inserter(bb_lead_insn),
		[&insninfo](size_t off) -> spu_insn*
	{
		return insninfo.data() + off;
	});

	blocks.reserve(block_leads.size() - 1);

	// iterate by pairs
	transform(bb_lead_insn.begin(), bb_lead_insn.end() - 1,
		bb_lead_insn.begin() + 1,
		back_inserter(blocks),
		[](spu_insn* first, spu_insn* last) -> bb
	{
		bb newblock = { first, last - 1, last, bbtype::code };

		return newblock;
	});

	// assign parent blocks to insns
	for (auto& block : blocks)
	{
		for (auto* insn = block.ibegin; insn != block.iend; ++insn)
		{
			insn->parent = &block;
		}
	}

	return blocks;
}

void bb_calctypes(
	vector<bb>& blocks )
{
	for ( auto& block : blocks )
	{
		const SPU_OP_COMPONENTS& opc = block.branch->comps;

		bbtype type = bbtype::code;

		// static jump
		if ( spu_op::M_BR == block.branch->op )
		{
			type = opc.IMM > 0 ? bbtype::sjumpf : bbtype::sjumpb;
			if ( 0 == opc.IMM ) 
			{
				type = bbtype::infloop;
			}				
		}
		// static conditional jump
		else if ( spu_op::M_BRZ == block.branch->op
			|| spu_op::M_BRNZ == block.branch->op
			|| spu_op::M_BRHZ == block.branch->op
			|| spu_op::M_BRHNZ == block.branch->op )
		{
			type = opc.IMM > 0 ? bbtype::cjumpf : bbtype::cjumpb;
			if ( 0 == opc.IMM ) type = bbtype::infloop;
		}
		else if ( spu_op::M_BRSL == block.branch->op )
		{
			type = bbtype::scall;
		}
		else if ( spu_op::M_BISL == block.branch->op )
		{
			type = bbtype::dcall;
		}
		else if ( spu_op::M_BIZ == block.branch->op
			|| spu_op::M_BINZ == block.branch->op
			|| spu_op::M_BIHZ == block.branch->op
			|| spu_op::M_BIHNZ == block.branch->op )
		{
			type = bbtype::cdjump;
		}
		else if ( spu_op::M_STOP == block.branch->op
			|| spu_op::M_STOPD == block.branch->op )
		{
			type = !((block.iend - 1)->raw) ? bbtype::stop : bbtype::stopsignal;
		}
		else if ( spu_op::M_BI == block.branch->op )
		{
			type = bbtype::ret;
		}

		block.type = type;
	}
}

void bb_find_unconditional_blocks(
	vector<bb>& blocks,
	set<bb*>& blocks_uncond )
{
	transform(blocks.begin(), blocks.end(), inserter(blocks_uncond, blocks_uncond.end()),
		[](bb& block)
	{
		return &block;
	});

	for (auto& block : blocks)
	{
		bb* bb_cond_first = nullptr;
		bb* bb_cond_last = nullptr;

		if ( block.type == bbtype::cjumpf
			|| block.type == bbtype::cjumpb )
		{
			const spu_insn* jmp_target = block.branch + block.branch->comps.IMM;
			bb* target_block = jmp_target->parent;

			if ( block.type == bbtype::cjumpf )
			{
				bb_cond_first = &block + 1;
				bb_cond_last = target_block;
			}
			else
			{
				bb_cond_first = target_block;
				bb_cond_last = &block + 1;
			}
		}

		while (bb_cond_first != bb_cond_last)
		{
			blocks_uncond.erase(bb_cond_first++);
		}
	}

	//for ( bb* block = &blocks[0]; block != (&blocks[0] + blocks.size()); )
	//{			
	//	if ( block->type == bbtype::cjumpf )
	//	{
	//		const spu_insn* jmp_target = block->branch + block->branch->comps.IMM;
	//		block = jmp_target->parent;
	//	}
	//	else
	//	{ 
	//		blocks_uncond.insert(block);
	//		++block;
	//	}
	//}

	//// remove blocks that belong to while() constructs
	//for ( bb* block = &blocks[blocks.size()-1]; block != &blocks[0]; )
	//{
	//	if ( block->type == bbtype::cjumpb )
	//	{
	//		const spu_insn* jmp_target = block->branch + block->branch->comps.IMM;
	//		bb* target_block = jmp_target->parent;
	//		while ( block != target_block )
	//		{
	//			blocks_uncond.erase(block);
	//			--block;
	//		}
	//		--block;
	//	}
	//	else
	//	{ 
	//		--block;
	//	}
	//}
}



vector<fn> bb_genfn(vector<bb>& blocks,
					const vector<spu_insn>& insninfo,
					const set<size_t>& brsl_targets)
{
	/*** Finding function boundaries using basic blocks

	Assuming we have no symbol info for a given binary we need to 
	calucate the function boundaries using the basic blocks only.
	Two lists are kept:
	* known function entry points
	* known function exit points
	We also have a function that walks both and equalizes them,
	since ever function entry is preceded by an exit and vice versa.

	1>>>
	Seed the entry list.
	<SPU>: For the entntry points we can use the targets of brsl ops.
	Those are the equivalents of a static function call.
		
	2>>>
	Filter the basic blocks for dominators.
	By definition, a function must terminate with an op that always
	braks control flow. Thus it can't belong to a basic block that might
	or might not get executed. For example it can't be part of an 'if' 
	statement's body.
	What we need then is to filter out all the basic blocks that are
	conditionally entered.
	For every static conditional jump, remove the jump itself and
	anything between the jump destionation plus the destination itself.

	3>>>
	Seed the exit list.
	Once we have these dominator basic blocks, search them for ops that
	stop/redirect the control flow for sure, like returns.
	<SPU>: The ops: stop, stopd, bi, br #0, 

	4>>>
	Resolve static unconditional jumps
	Sometimes functions end wit ha simple jump, either because there is a 
	pre-tested loop at the end of the function or tail call optimization.
	To decide wether a jump terminates a function, we use these simple 
	criteria. The jump teriminates a function if:
	* it is a known exit
	* it is followed by a known entry
	* its destination is a known entry
	* its destination is preceded by a known exit
	Any of these being true means they are all true and can be added to 
	the lists. Of course we might get out of bounds of the basic block array
	but those are discarded later.
	<SPU>: Since functions must be 8 byte aligned, there often is an lnop
	between functions. These need to be recognized and skipped when we are
	looking at next and previous blocks.
	*/

	// Remove basic blocks that belong to if()/while() constructs.
	// Blocks that remain are guaranted to contain only control flow 
	// breakers or reversers that will terminate a function.
	// Returns in in() blocks are discarded.

	set<bb*> blocks_uncond;

	bb_find_unconditional_blocks( blocks, blocks_uncond );

	{
		// find jump tables embedded in the text section
		// bi, followed by nonzero stops
		// each stop is an IP
		// the highest IP must belong to the bi's function
		// thus bbs starting with thost IPs can't be terminators except the last one
		set<bb*> cond_blocks;

		auto to_insn = [&insninfo](size_t vaddr) -> const spu_insn*
		{
			const size_t offset = (vaddr - insninfo[0].vaddr) / 4;
			return &insninfo[offset];
		};

		for (auto& insn : insninfo)
		{
			const spu_insn* curr_insn = &insn;
			const spu_insn* next_insn = curr_insn + 1;

			

			if (curr_insn->op == spu_op::M_BI
				&& next_insn->op == spu_op::M_STOP 
				&& next_insn->raw >= 0x12c00)
			{
				// curr_insn == bi, the jump
				// next_insn == first jump address

				bb* first_block = next_insn->parent;

				const spu_insn* jumptbl_begin = next_insn;
				const spu_insn* jumptbl_end = next_insn;

				while (jumptbl_end->op == spu_op::M_STOP 
					&& jumptbl_end->raw >= 0x12c00)
				{
					cond_blocks.insert(to_insn(jumptbl_end->raw)->parent);
					++jumptbl_end;
				}

				bb* last_block = *--cond_blocks.end();

				cond_blocks.erase(--cond_blocks.end());

				while (first_block != last_block)
				{
					blocks_uncond.erase(first_block++);
				}
			}
		}

		for (auto block : cond_blocks)
		{
			blocks_uncond.erase(block);
		}
	}

	set<bb*> known_fn_entries;
	set<bb*> known_fn_exits;

	{
		/* bbtype::ret
		// ...
		return ...;
		}
		*/

		/* bbtype::stop
		// ...
		// suspend thread
		}
		*/

		/* bbtype::infloop
		// ...
		while (1)
		{
		};
		}
		*/
		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(known_fn_exits, known_fn_exits.end()),
			[](bb*const& block) 
		{ 
			return block->type == bbtype::infloop
				|| block->type == bbtype::stop
				|| block->type == bbtype::ret; 
		});	
	}

	for ( auto entry_vaddr : brsl_targets )
	{
		auto insn_index = (entry_vaddr - insninfo[0].vaddr) / 4;

		auto insn = &insninfo[insn_index];

		// by definition, the destination of a brsl call will be the first
		// insn of a basic block
		known_fn_entries.insert(insn->parent);
	}

	auto is_dword_aligned = [](const bb* block)
	{
		return 0 == block->ibegin->vaddr % 8;
	};

	auto next_not_lnop = [&](bb* block) -> bb*
	{
		bb* next_block = block + 1;

		if (1 == bb_insn_count(next_block)
			&& next_block->ibegin->op == spu_op::M_LNOP)
		{
			++next_block;
		}

		return next_block;
	};

	auto prev_not_lnop = [&](bb* block) -> bb*
	{
		bb* prev_block = block - 1;

		if (1 == bb_insn_count(prev_block)
			&& prev_block->ibegin->op == spu_op::M_LNOP)
		{
			--prev_block;
		}

		return prev_block;
	};

	auto is_fn_entry = [&](bb* block)
	{
		const bool aligned = is_dword_aligned(block);
		const bool preceded_by_exit = 
			(known_fn_exits.cend() != find(known_fn_exits.cbegin(), known_fn_exits.cend(), prev_not_lnop(block)));
		return aligned && preceded_by_exit;
	};

	auto is_fn_exit = [&](bb* block)
	{
		const bb* next_block = next_not_lnop(block);
		const bool next_aligned = is_dword_aligned(next_block);
		const bool followed_by_entry = 
			(known_fn_entries.cend() != find(known_fn_entries.cbegin(), known_fn_entries.cend(), next_block));
		return next_aligned && followed_by_entry;
	};
		
	auto recalc_boundaries = [&]()
	{
		size_t entry_count = 0;
		size_t exit_count = 0;

		do 
		{
			entry_count = known_fn_entries.size();
			exit_count = known_fn_exits.size();

			for (auto entry : known_fn_entries)
			{
				known_fn_exits.insert(prev_not_lnop(entry));
			}

			for (auto exit : known_fn_exits)
			{
				known_fn_entries.insert(next_not_lnop(exit));
			}
				
		} while (entry_count != known_fn_entries.size()
			|| exit_count != known_fn_exits.size());
	};

	auto can_be_fn_lead = [&](bb* block)
	{
		const bool is_uncond_bb = 
			blocks_uncond.end() != find(blocks_uncond.begin(), blocks_uncond.end(), block);

		if (is_uncond_bb)
			return true;

		// block is conditional, but it can still be a fn entry if it's the first
		// in a series of conditional blocks
		const bool is_uncond_prev_bb = 
			blocks_uncond.end() != find(blocks_uncond.begin(), blocks_uncond.end(), block - 1);

		return is_uncond_prev_bb;
	};

	recalc_boundaries();

	{
		set<bb*> sjumps;

		copy_if( blocks_uncond.begin(), blocks_uncond.end(), 
			inserter(sjumps, sjumps.end()),
			[](bb*const& block) 
		{ 
			return block->type == bbtype::sjumpf || block->type == bbtype::sjumpb; 
		});

		/*set<bb*> spills;

		for ( auto jump : sjumps )
		{
			auto to = (jump->branch + jump->branch->comps.IMM)->parent;
			if (to->ibegin->vaddr  % 8 != 0)
				continue;
			auto lb = lower_bound(known_fn_entries.begin(), known_fn_entries.end(), jump);
			auto ub = lb;
			while (*lb > jump)
				--lb;
			const bool spills_before = to < *lb;
			const bool spills_after = to >= *ub;

			if (spills_before || spills_after)
				known_fn_entries.insert(to);
		}

		recalc_boundaries();*/

		for (auto block : sjumps)
		{
			bb* target_block = (block->branch + block->branch->comps.IMM)->parent;

			if (is_fn_exit(block) 
				|| is_fn_entry(next_not_lnop(block))
				|| is_fn_entry(target_block) 
				|| is_fn_exit(prev_not_lnop(target_block)))
			{
				const size_t entry_old_size = known_fn_entries.size();
				const size_t exit_old_size = known_fn_exits.size();

				known_fn_exits.insert(block);
				if (can_be_fn_lead(target_block))
				{
					known_fn_entries.insert(target_block);
				}
				else
				{
					int z = 3;
				}

				if (entry_old_size != known_fn_entries.size()
					|| exit_old_size != known_fn_exits.size())
				{
					recalc_boundaries();
				}
			}

			//if (is_fn_entry(target_block) 
			//	|| is_fn_exit(prev_not_lnop(target_block)))
			//{
			//	known_fn_exits.insert(block);
			//	known_fn_entries.insert(target_block);

			//	recalc_boundaries();
			//}
		}
	}

	// drop false positives
	// TODO: feels like a guesswork hack. try to get rid of this
	known_fn_exits.erase(known_fn_exits.begin());
	known_fn_entries.erase(--known_fn_entries.end());

	vector<fn> functions;

	functions.reserve(known_fn_entries.size());

	transform(known_fn_entries.begin(), known_fn_entries.end(),
		known_fn_exits.begin(),
		back_inserter(functions),
		[](bb* entry, bb* exit) -> fn 
	{
		fn new_fn = { entry, exit };
		return new_fn;
	});

	for (auto& fun : functions)
	{
		bb* block = fun.entry;
		bb* block_last = fun.exit + 1;
		while (block != block_last)
		{
			block->parent = &fun;
			++block;
		}
	}

	return functions;
}