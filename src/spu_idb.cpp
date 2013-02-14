#include <iostream>
#include <set>
#include <algorithm>
#include <string>
#include "spu_idb.h"
#include "spu_emu.h"



static const size_t LSLR = 0x3ffff;
static const size_t LSLR_INSN = LSLR >> 2;

using namespace std;

static vector<SPU_OP_TYPE>		db_op_type( SPU_MAX_INSTRUCTION_COUNT, SPU_OP_TYPE(-1) );
static vector<string>			db_op_mnemonic( SPU_MAX_INSTRUCTION_COUNT );
vector<vector<SPU_ARGLIST>>		db_op_arglist( SPU_MAX_INSTRUCTION_COUNT );


void spu_build_op_db()
{
	
#define _A0()        {0xFF,{0xFF,0xFF,0xFF,0}}
#define _A1(a)       {1,{a,0xFF,0xFF,0}}
#define _A2(a,b)     {2,{a,b,0xFF,0}}
#define _A3(a,b,c)   {3,{a,b,c,0}}
#define _A4(a,b,c,d) {4,{a,b,c,d}}

#define APUOP(TAG,		FORMAT,	OPCODE,	MNEMONIC,	ASM_FORMAT,	DEPENDENCY,	PIPE) \
{ \
	db_op_type[OPCODE] = SPU_OP_TYPE_##FORMAT; \
	db_op_mnemonic[OPCODE] = MNEMONIC; \
}

#define APUOPFB(TAG,		FORMAT,	OPCODE,	FEATUREBIT, MNEMONIC,	ASM_FORMAT,	DEPENDENCY,	PIPE)

#include "spu-insns.h"

#undef APUOP
#undef APUOPFB
	
}

#undef _A0
#undef _A1
#undef _A2
#undef _A3
#undef _A4

size_t spu_decode_op_opcode( uint32_t op )
{
	static bool opcodes_parsed = false;

	if ( !opcodes_parsed )
	{
		spu_build_op_db();
		opcodes_parsed = true;
	}

	op >>= 21;

	const SPU_OP_TYPE* type_tbl =  db_op_type.data();

	if ( SPU_OP_TYPE_RR == type_tbl[op] || SPU_OP_TYPE_RI7 == type_tbl[op] || SPU_OP_TYPE_LBTI == type_tbl[op] ) 
		return op;
	else if ( SPU_OP_TYPE_RRR == type_tbl[op & 0x780] ) 
		return op & 0x780;
	else if ( SPU_OP_TYPE_RI18 == type_tbl[op & 0x7F0] || SPU_OP_TYPE_LBT == type_tbl[op & 0x7F0] ) 
		return op & 0x7F0;
	else if ( SPU_OP_TYPE_RI10 == type_tbl[op & 0x7F8] ) 
		return op & 0x7F8;
	else if ( SPU_OP_TYPE_RI16 == type_tbl[op & 0x7FC] ) 
		return op & 0x7FC;
	else if ( SPU_OP_TYPE_RI18 == type_tbl[op & 0x7FE] ) 
		return op & 0x7FE;
	else
		return 0x7FF;
}

SPU_OP_TYPE spu_decode_op_type( uint32_t op )
{
	return db_op_type[spu_decode_op_opcode( op )];
}

string spu_decode_op_mnemonic( uint32_t op )
{
	return db_op_mnemonic[spu_decode_op_opcode( op )];
}

ptrdiff_t spu_op_decode_branch_offset( uint32_t op )
{
	const SPU_OP_COMPONENTS OPC = spu_decode_op_components( op );

	return OPC.IMM;
}

SPU_OP_COMPONENTS spu_decode_op_components( uint32_t raw_instr )
{
	const SPU_INSTRUCTION op = (SPU_INSTRUCTION&)raw_instr;
	const SPU_OP_TYPE itype = spu_decode_op_type(raw_instr);

	switch (itype)
	{
	case SPU_OP_TYPE_RRR:
		{
			const SPU_OP_COMPONENTS result = { op.RRR.RT,	op.RRR.RA,	op.RRR.RB,	op.RRR.RC,	0 };
			return result;
		}
	case SPU_OP_TYPE_RR:
		{
			const SPU_OP_COMPONENTS result = { op.RR.RT,	op.RR.RA,	op.RR.RB,	SPU_OP_INVALID_GPR,		0 };
			return result;
		}
	case SPU_OP_TYPE_RI7:
		{
			const SPU_OP_COMPONENTS result = { op.RI7.RT,	op.RI7.RA,	SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SignExtend(op.RI7.I7, 7) };
			return result;
		}
	case SPU_OP_TYPE_RI8:
		{
			const SPU_OP_COMPONENTS result = { op.RI8.RT,	op.RI8.RA,	SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SignExtend(op.RI8.I8, 8) };
			return result;
		}
	case SPU_OP_TYPE_RI10:
		{
			const SPU_OP_COMPONENTS result = { op.RI10.RT,	op.RI10.RA,	SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SignExtend(op.RI10.I10, 10) };
			return result;
		}
	case SPU_OP_TYPE_RI16:
		{
			const SPU_OP_COMPONENTS result = { op.RI16.RT,	SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SignExtend(op.RI16.I16, 16) };
			return result;
		}
	case SPU_OP_TYPE_RI18:
		{
			const SPU_OP_COMPONENTS result = { op.RI18.RT,	SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SPU_OP_INVALID_GPR,		SignExtend(op.RI18.I18, 18) };
			return result;
		}
	case SPU_OP_TYPE_LBT:
		{
			const uint32_t BRTARG = (uint32_t)SignExtend( op.LBT.I16, 16 );
			const uint32_t BRINST = (uint32_t)SignExtend( ((uint32_t)op.LBT.ROH << 7) | (uint32_t)op.LBT.ROL, 11 );
			const SPU_OP_COMPONENTS result = { SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, (int64_t)(((uint64_t)BRTARG << 32) | (uint64_t)BRINST) };
			return result;
		}
	case SPU_OP_TYPE_LBTI:
		{
			const uint32_t BRTARG = (uint32_t)SignExtend( ((uint32_t)op.LBTI.ROH << 7) | (uint32_t)op.LBTI.ROL, 11 );
			const uint32_t BRINST = 0;
			const SPU_OP_COMPONENTS result = { SPU_OP_INVALID_GPR, op.LBTI.RA, SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, (int64_t)(((uint64_t)BRTARG << 32) | (uint64_t)BRINST) };
			return result;
		}
	default:
		{
			const SPU_OP_COMPONENTS result = { SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, SPU_OP_INVALID_GPR, 0 };
			return result;
		}
	}
}

void spu_insn_process_bin( const vector<uint32_t>& binary, 
						  vector<spu_insn>& ilist, 
						  size_t vbase )
{
	ilist.reserve( binary.size() );

	uint32_t vaddr = (uint32_t)vbase;

	for ( auto insn : binary )
	{
		spu_insn insn_info;
		insn_info.raw = insn;
		insn_info.vaddr = vaddr;
		vaddr += 4;
		insn_info.op = (spu_op)spu_decode_op_opcode( insn );
//		insn_info.type = spu_decode_op_type( insn );
		insn_info.comps = spu_decode_op_components( insn );
		//insn_info.flags = 0;
		ilist.push_back( insn_info );
	}
}
//
//void spu_insn_process_flags( vector<spu_insn>& ilist,
//							map<string, vector<size_t>>& histogram )
//{
//	auto flag_by_op = [&]( string mnem, uint32_t flag )
//	{
//		auto& filter = histogram[mnem];
//		for ( size_t index : filter )
//		{
//			ilist[index].flags |= flag;
//		}
//	};
//
//#define FLAG_DYNAMIC( MNEM ) \
//	flag_by_op( #MNEM, SPU_IS_BRANCH | SPU_IS_BRANCH_DYNAIMC );
//#define FLAG_DYNAMIC_COND( MNEM ) \
//	flag_by_op( #MNEM, SPU_IS_BRANCH | SPU_IS_BRANCH_DYNAIMC | SPU_IS_BRANCH_CONDITIONAL );
//#define FLAG_STATIC( MNEM ) \
//	flag_by_op( #MNEM, SPU_IS_BRANCH | SPU_IS_BRANCH_STATIC );
//#define FLAG_STATIC_COND( MNEM ) \
//	flag_by_op( #MNEM, SPU_IS_BRANCH | SPU_IS_BRANCH_STATIC | SPU_IS_BRANCH_CONDITIONAL );
//
//
//	FLAG_STATIC( br );
//	FLAG_STATIC( brsl );
//	FLAG_STATIC( bra );
//	FLAG_STATIC( brasl );
//
//	FLAG_STATIC_COND( brz );
//	FLAG_STATIC_COND( brnz );
//	FLAG_STATIC_COND( brhz );
//	FLAG_STATIC_COND( brhnz );
//
//	FLAG_DYNAMIC( bi );
//	FLAG_DYNAMIC( iret );
//	FLAG_DYNAMIC( bisled );
//	FLAG_DYNAMIC( bisl );	
//		
//	FLAG_DYNAMIC_COND( biz );
//	FLAG_DYNAMIC_COND( binz );
//	FLAG_DYNAMIC_COND( bihz );
//	FLAG_DYNAMIC_COND( bihnz );
//
//	auto flag_branch_target = [&]( string mnem )
//	{
//		auto& filter = histogram[mnem];
//		for ( size_t index : filter )
//		{
//			const ptrdiff_t offset = ilist[index].comps.IMM;
//			ilist[index + offset].flags |= SPU_IS_BRANCH_TARGET;
//		}
//	};
//
//	flag_branch_target( "br" );
//	flag_branch_target( "brsl" );
//
//	// Flags instructions that essentially do nothing but move registers around.
//	// These are needed for there is no dedicated move register instruction for the SPU.
//		
//	auto flag_assignment = [&]( string mnem, uint32_t IMM )
//	{
//		auto& filter = histogram[mnem];
//		for ( size_t index : filter )
//		{			
//			if ( IMM == ilist[index].comps.IMM )
//				ilist[index].flags |= SPU_IS_ASSIGNMENT;
//		}
//	};
//
//#define FLAG_ASSIGNMENT(NAME, IMM) flag_assignment( #NAME, IMM );
//
//	FLAG_ASSIGNMENT( ahi, 0 );
//	FLAG_ASSIGNMENT( ai, 0 );
//	FLAG_ASSIGNMENT( sfhi, 0 );
//	FLAG_ASSIGNMENT( sfi, 0 );
//	FLAG_ASSIGNMENT( andbi, 0xFF );
//	FLAG_ASSIGNMENT( andhi, 0x3FF );
//	FLAG_ASSIGNMENT( andi, 0x3FF );
//	FLAG_ASSIGNMENT( orbi, 0 );
//	FLAG_ASSIGNMENT( orhi, 0 );
//	FLAG_ASSIGNMENT( ori, 0 );
//	FLAG_ASSIGNMENT( shlhi, 0 );
//	FLAG_ASSIGNMENT( shli, 0 );
//	FLAG_ASSIGNMENT( shlqbii, 0 );
//	FLAG_ASSIGNMENT( shlqbyi, 0 );
//	FLAG_ASSIGNMENT( rothi, 0 );
//	FLAG_ASSIGNMENT( roti, 0 );
//	FLAG_ASSIGNMENT( rotqbii, 0 );
//	FLAG_ASSIGNMENT( rotqbyi, 0 );
//	FLAG_ASSIGNMENT( rothmi, 0 );
//	FLAG_ASSIGNMENT( rotmi, 0 );
//	FLAG_ASSIGNMENT( rotqmbii, 0 );
//	FLAG_ASSIGNMENT( rotqmbyi, 0 );
//	FLAG_ASSIGNMENT( rotmahi, 0 );
//	FLAG_ASSIGNMENT( rotmai, 0 );
//
//
//}

spu_insn* vaddr2insn( uint32_t vaddr, vector<spu_insn>& insns )
{
	auto base_vaddr = insns[0].vaddr;
	auto offset = (vaddr - base_vaddr) / 4;
	return &insns[offset];
}

std::vector<jump_table> enum_jump_tables( std::vector<spu_insn>& ilist )
{
	vector<jump_table> tables;

	// find jump tables embedded in the text section
	// bi, followed by nonzero stops
	// each stop is an IP
	// the highest IP must belong to the bi's function
	// thus bbs starting with thost IPs can't be terminators except the last one
	
	//set<bb*> cond_blocks;

	auto vaddr_of_code = [=](const spu_insn* insn)
	{
		return insn->raw >= ilist.front().vaddr
			&& insn->raw <= ilist.back().vaddr;
	};

	auto is_jump_vaddr = [&](const spu_insn* insn)
	{
		return insn->op == spu_op::M_STOP
			&& vaddr_of_code(insn);
	};

	// TODO: use heuristics table to find all bi's fast
	for (auto& insn : ilist)
	{
		const spu_insn* curr_insn = &insn;
		const spu_insn* next_insn = curr_insn + 1;

		if (curr_insn->op == spu_op::M_BI
			&& is_jump_vaddr(next_insn))
		{
			// curr_insn: bi, the jump
			// next_insn: first jump address
			jump_table new_table;
			new_table.jump = curr_insn;			

			for (const spu_insn* jumptarget_iter = next_insn; 
				is_jump_vaddr(jumptarget_iter);
				++jumptarget_iter)
			{
				new_table.jump_targets.insert(to_insn(ilist, jumptarget_iter->raw));
//				cond_blocks.insert(to_insn(ilist, jumptbl_end->raw)->parent);
			}

			tables.push_back(new_table);
		}
	}

	return tables;
}

vector<size_t> spu_find_basicblock_leader_offsets(
	map<spu_op, vector<spu_insn*>>& opdistrib,
	vector<spu_insn>& ilist )
{
	vector<spu_insn*> bb_leads;

	// gather insns that change the control flow
	auto append = [&](spu_op type) { 
		for (auto insn : opdistrib[type])
		{
			bb_leads.push_back(++insn);
		}
	};
	append( spu_op::M_BR );
	append( spu_op::M_BRSL );
	append( spu_op::M_BRZ );
	append( spu_op::M_BRNZ );
	append( spu_op::M_BRHZ );
	append( spu_op::M_BRHNZ );
	append( spu_op::M_BI );
	append( spu_op::M_IRET );
	append( spu_op::M_BISL );
	append( spu_op::M_BISLED );
	append( spu_op::M_BIZ );
	append( spu_op::M_BINZ );
	append( spu_op::M_BIHZ );
	append( spu_op::M_BIHNZ );

	/* 
	There is a paddig with (l)nops that throws off the basic block generation.
	Functions must start on a modulo 8 address and extra lnops are used to achieve that.
	However, due to how basic block generation algo works, this often gives us a false
	block where the first insn is an lnop where in fact that lnop shold be consdidered
	a dead code. To work around this, I'll treat nop/lnop as a (br $IP+4) op. That way
	they get their own basic blocks and does not interfere with the rest of the 
	processing, I hope. They will be treated as an unconditional jump to the next insn.
	*/
	append( spu_op::M_NOP );
	append( spu_op::M_LNOP );

	for (auto insn : opdistrib[spu_op::M_STOP])
	{
		if (0 == insn->raw)
		{
			bb_leads.push_back(++insn);
		}
	}

	// entry is a leader 
	bb_leads.push_back( &ilist[0] );

	// gather branch targets
	auto append_targets = [&](spu_op type) { 
		for (auto insn : opdistrib[type])
		{
			auto to_vaddr = LSLR & (insn->vaddr + insn->comps.IMM * 4);
			bb_leads.push_back(to_insn(ilist, to_vaddr));
		}
		//transform( opdistrib[type].cbegin(), opdistrib[type].cend(), 
		//	back_inserter(bb_leads),
		//	[&](size_t index) { return (index + ilist[index].comps.IMM) & 0xffff; } );
	};
	append_targets( spu_op::M_BR );
	append_targets( spu_op::M_BRSL );
	append_targets( spu_op::M_BRZ );
	append_targets( spu_op::M_BRNZ );
	append_targets( spu_op::M_BRHZ );
	append_targets( spu_op::M_BRHNZ );

	auto jmptbl = enum_jump_tables(ilist);

	for (auto& tbl : jmptbl)
	{
		for (auto target : tbl.jump_targets)
		{
			// FIXME: remove the const cast
			bb_leads.push_back(const_cast<spu_insn*>(target));
		}
	}

	sort( bb_leads.begin(), bb_leads.end() );

	bb_leads.erase( unique( bb_leads.begin(), bb_leads.end() ), bb_leads.end() );

	vector<size_t> leads_offsets;

	for (auto leader : bb_leads)
	{
		leads_offsets.push_back(leader - &ilist[0]);
	}

	return leads_offsets;
}



set<size_t> spu_get_brsl_targets(
	map<spu_op, vector<spu_insn*>>& histogram,
	const vector<spu_insn>&,
	size_t entry_vaddr )
{
	auto& brsl_insns = histogram[spu_op::M_BRSL];
	set<size_t> vaddr_list;

	vaddr_list.insert( entry_vaddr );

	for ( auto insn : brsl_insns )
	{
		const auto voff = insn->comps.IMM * 4;
		const auto vto = (insn->vaddr + voff) & LSLR;
		vaddr_list.insert( vto );
	}

	return vaddr_list;
}

//set<size_t> spu_get_br_targets(
//	map<string, vector<size_t>>& histogram,
//	const vector<spu_insn>& ilist )
//{
//	set<size_t> vaddr_list;
//
//	auto& jmp_op_offsets = histogram["br"];
//	for ( auto& offset : jmp_op_offsets )
//	{
//		const spu_insn* insn = &ilist[offset];
//		vaddr_list.insert( (insn + insn->comps.IMM)->vaddr );
//	}
//
//	return vaddr_list;
//}

//set<size_t> spu_get_initial_fn_entries(
//	map<string, vector<size_t>>& histogram,
//	const vector<spu_insn>& ilist,
//	size_t entry_vaddr )
//{
//	set<size_t> entries = 
//		spu_get_brsl_targets(histogram, ilist, entry_vaddr);
//
//	entries.insert( ilist[0].vaddr );
//
//	return entries;
//}