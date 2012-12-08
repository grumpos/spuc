#include "basicblock.h"
#include "spu_idb.h"


using namespace std;


vector<bb> bb_genblocks( 
	const vector<size_t>& block_leads,
	const vector<spu_insn>& insninfo )
{
	vector<bb> blocks;

	blocks.reserve( block_leads.size() - 1 );

	for ( size_t b = 0, e = 1; e < block_leads.size(); ++b, ++e )
	{
		const size_t begin_insn_idx = block_leads[b];
		const size_t end_insn_idx = block_leads[e];

		bb newblock = { 			
			insninfo.data() + begin_insn_idx, 
			insninfo.data() + end_insn_idx - 1, 
			insninfo.data() + end_insn_idx,
			bbtype::code
		};

		blocks.push_back(newblock);
	}

	return blocks;
}

void bb_calctypes(
	vector<bb>& blocks,
	const vector<spu_insn>& insninfo )
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

void resolve_tailcall_opt(
	vector<bb>& blocks,
	const set<size_t>& fn_entries )
{
	for ( auto& block : blocks )
		{
			const uint32_t target_vaddr = 
				block.branch->vaddr + block.branch->comps.IMM * 4;
			if ( target_vaddr % 8 == 0
				&& (block.type == bbtype::sjumpf || block.type == bbtype::sjumpb )
				&& fn_entries.end() != 
				find( fn_entries.begin(), fn_entries.end(), target_vaddr ) )
			{
				block.type = bbtype::scall_ret;
			}
		}
}

void bb_find_unconditional_blocks(
	vector<bb>& blocks,
	set<bb*>& blocks_uncond,
	map<const spu_insn*, bb*>& insn2block )
{
	for ( bb* block = &blocks[0]; block != (&blocks[0] + blocks.size()); )
	{			
		if ( block->type == bbtype::cjumpf )
		{
			const spu_insn* jmp_target = block->branch + block->branch->comps.IMM;
			block = insn2block[jmp_target];
		}
		else
		{ 
			blocks_uncond.insert(block);
			++block;
		}
	}

	// remove blocks that belong to while() constructs
	//		for ( auto block : blocks_no_fwd_jmp )
	for ( bb* block = &blocks[blocks.size()-1]; block != &blocks[0]; )
	{
		if ( block->type == bbtype::cjumpb )
		{
			const spu_insn* jmp_target = block->branch + block->branch->comps.IMM;
			bb* target_block = insn2block[jmp_target];
			while ( block != target_block )
			{
				blocks_uncond.erase(block);
				--block;
			}
			--block;
		}
		else
		{ 
			--block;
		}
	}
}