#pragma once
#ifndef __SPU_IDB_H__
#define __SPU_IDB_H__

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <set>

#define SPU_IS_BRANCH				(1 << 0)
#define SPU_IS_BRANCH_CONDITIONAL	(1 << 1)
#define SPU_IS_BRANCH_TARGET		(1 << 2)
#define SPU_IS_ASSIGNMENT			(1 << 3)
#define SPU_IS_BRANCH_STATIC		(1 << 4)
#define SPU_IS_BRANCH_DYNAIMC		(1 << 5)


#define SPU_OP_INVALID_GPR			(0x80)

const size_t SPU_MAX_INSTRUCTION_COUNT = 0x800;

enum SPU_OP_TYPE
{
	SPU_OP_TYPE_RRR		= 0,
	SPU_OP_TYPE_RR		= 1,
	SPU_OP_TYPE_RI7		= 2,
	SPU_OP_TYPE_RI8		= 3,
	SPU_OP_TYPE_RI10	= 4,
	SPU_OP_TYPE_RI16	= 5,
	SPU_OP_TYPE_RI18	= 6,
	SPU_OP_TYPE_LBT		= 7,
	SPU_OP_TYPE_LBTI	= 8
};

static const uint8_t A_T	= 0;
static const uint8_t A_A	= 1;
static const uint8_t A_B	= 2;
static const uint8_t A_C	= 3;
static const uint8_t A_H	= A_A;
static const uint8_t A_P	= 5;
static const uint8_t A_R18	= 21; // >> 2
static const uint8_t A_S	= A_A;
static const uint8_t A_S10	= 10;
static const uint8_t A_S10B	= 10;
static const uint8_t A_S11	= 19;
static const uint8_t A_S11I	= 19;
static const uint8_t A_S14	= 14; // >> 4
static const uint8_t A_S16	= 16;
static const uint8_t A_S18	= 21; // >> 2
static const uint8_t A_S3	= 7;
static const uint8_t A_S6	= 7;
static const uint8_t A_S7	= 7;
static const uint8_t A_S7N	= 7;
static const uint8_t A_U3	= 7;
static const uint8_t A_U5	= 7;
static const uint8_t A_U6	= 7;
static const uint8_t A_U7	= 7;
static const uint8_t A_U7A	= 7;
static const uint8_t A_U7B	= 7;
static const uint8_t A_U14	= 20; // copy to status reg
static const uint8_t A_U18	= 18;
static const uint8_t A_X16	= 16;

typedef void (*SPU_INSTR_PTR)( struct SPU_t* SPU, union SPU_INSTRUCTION op );
typedef std::pair<std::string, SPU_INSTR_PTR> spu_mnem_op_t;

struct SPU_ARGLIST
{
	uint8_t ArgCount;
	uint8_t ArgType[4];
};

struct SPU_OP_COMPONENTS
{
	uint8_t RT, RA, RB, RC;
	int64_t IMM;
};

enum class spu_op : uint16_t
{
	#define APUOP(TAG,		FORMAT,	OPCODE,	MNEMONIC,	ASM_FORMAT,	DEPENDENCY,	PIPE) \
	TAG = OPCODE,

	#include "spu-insns.h"

	#undef APUOP
};

extern std::vector<std::vector<SPU_ARGLIST>>	db_op_arglist;

SPU_OP_TYPE		spu_decode_op_type( uint32_t op );
std::string		spu_decode_op_mnemonic( uint32_t op );
size_t			spu_decode_op_opcode( uint32_t op );
SPU_INSTR_PTR	spu_decode_op_solver( uint32_t op );
SPU_OP_COMPONENTS spu_decode_op_components( uint32_t raw_instr );
ptrdiff_t		spu_op_decode_branch_offset( uint32_t Instr );

struct bb;

struct spu_insn
{	
	uint32_t vaddr;
	spu_op op;
	uint32_t raw;
	//uint64_t flags;
	//SPU_OP_TYPE type;	
	SPU_OP_COMPONENTS comps;
	bb* parent;
};

struct jump_table
{
	const spu_insn* jump;
	std::set<const spu_insn*> jump_targets;
};

spu_insn* vaddr2insn( size_t vaddr, const std::vector<spu_insn>& insns );

void spu_insn_process_bin( const std::vector<uint32_t>& binary, 
						  std::vector<spu_insn>& insninfo, 
						  size_t vbase );

//void spu_insn_process_flags( std::vector<spu_insn>& insninfo,
//							std::map<std::string, std::vector<size_t>>& histogram );

std::vector<jump_table> enum_jump_tables(const std::vector<spu_insn>& insninfo);

std::vector<size_t> spu_find_basicblock_leader_offsets(
	std::map<spu_op, std::vector<spu_insn*>>& opdistrib,
	std::vector<spu_insn>& insninfo );

std::set<size_t> spu_get_brsl_targets(
	std::map<spu_op, std::vector<spu_insn*>>& histogram,
	const std::vector<spu_insn>& insninfo,
	size_t entry_vaddr );

//std::set<size_t> spu_get_br_targets(
//	std::map<std::string, std::vector<size_t>>& histogram,
//	const std::vector<spu_insn>& insninfo );

std::set<size_t> spu_get_initial_fn_entries(
	std::map<std::string, std::vector<size_t>>& histogram,
	const std::vector<spu_insn>& insninfo,
	size_t entry_vaddr );


#endif