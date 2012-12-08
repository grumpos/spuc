#ifndef __BASICBLOCK_H__
#define __BASICBLOCK_H__

#include <cstdint>
#include <string>
#include <vector>
#include "spu_idb.h"


enum class bbtype : uint8_t
{
	code = 0,
	scall,
	scall_ret, // tail call optimizations
	dcall,
	sjumpf,
	sjumpb,
	cjumpf,
	cjumpb,
	stop,
	stopsignal,
	cdjump,
	djump,
	ret,
	infloop
};

const uint32_t bbtype_fallthru_mask = 
	(1ui32<<(uint8_t)bbtype::code) |
	(1ui32<<(uint8_t)bbtype::scall) |
	(1ui32<<(uint8_t)bbtype::dcall) |
	(1ui32<<(uint8_t)bbtype::cjumpf) |
	(1ui32<<(uint8_t)bbtype::cjumpb) |
	(1ui32<<(uint8_t)bbtype::cdjump);

const uint32_t bbtype_staticjmp_mask = 
	(1ui32<<(uint8_t)bbtype::scall) |
	(1ui32<<(uint8_t)bbtype::scall_ret) |
	(1ui32<<(uint8_t)bbtype::sjumpf) |
	(1ui32<<(uint8_t)bbtype::sjumpb) |
	(1ui32<<(uint8_t)bbtype::cjumpf) |
	(1ui32<<(uint8_t)bbtype::cjumpb);

const uint32_t bbtype_dynamicjmp_mask = 
	(1ui32<<(uint8_t)bbtype::dcall) |
	(1ui32<<(uint8_t)bbtype::cdjump) |
	(1ui32<<(uint8_t)bbtype::djump);

struct bb
{
	const spu_insn* ibegin;
	const spu_insn* branch; // basically always (iend-1) 
	const spu_insn* iend;
	bbtype type;
};


std::vector<bb> bb_genblocks( 
	const std::vector<size_t>& block_leads,
	const std::vector<spu_insn>& insninfo );

void bb_calctypes(
	std::vector<bb>& blocks,
	const std::vector<spu_insn>& insninfo );

void resolve_tailcall_opt(
	std::vector<bb>& blocks,
	const std::set<size_t>& fn_entries );

void bb_find_unconditional_blocks(
	std::vector<bb>& blocks,
	std::set<bb*>& blocks_uncond,
	std::map<const spu_insn*, bb*>& insn2block );

// helpers

inline size_t bb_insn_count(const bb* block)
{
	return block->iend - block->ibegin;
}

#endif