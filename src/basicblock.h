#ifndef __BASICBLOCK_H__
#define __BASICBLOCK_H__

#include <cstdint>
#include <string>
#include <vector>
#include "spu_idb.h"
#include "fn.h"

enum class bbtype : uint8_t
{
	code = 0,
	scall,
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
	ret_tco, // tail call optimizations
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
	(1ui32<<(uint8_t)bbtype::ret_tco) |
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
	spu_insn* ibegin;
	spu_insn* branch; // basically always (iend-1) 
	spu_insn* iend;
	bbtype type;
	fn* parent;
};


std::vector<bb> bb_genblocks(
	std::vector<spu_insn>& ilist,
	std::map<spu_op, std::vector<spu_insn*>>& opdistrib );

void bb_calctypes(
	std::vector<bb>& blocks );

void resolve_tailcall_opt(
	std::vector<bb>& blocks,
	const std::set<size_t>& fn_entries );

void bb_find_unconditional_blocks(
	std::vector<bb>& blocks,
	std::set<bb*>& blocks_uncond );

std::vector<fn> bb_genfn(std::vector<bb>& blocks,
					const std::vector<spu_insn>& ilist,
					const std::set<size_t>& brsl_targets,
					size_t vbase);

// helpers

inline size_t bb_insn_count(const bb* block)
{
	return static_cast<size_t>(block->iend - block->ibegin);
}

#endif