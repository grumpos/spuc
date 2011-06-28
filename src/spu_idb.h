#pragma once
#ifndef __SPU_IDB_H__
#define __SPU_IDB_H__

#include <cstdint>
#include <vector>
#include <string>

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

extern std::vector<std::vector<SPU_ARGLIST>>	db_op_arglist;

SPU_OP_TYPE		spu_decode_op_type( uint32_t op );
const char*		spu_decode_op_mnemonic( uint32_t op );
size_t			spu_decode_op_opcode( uint32_t op );
SPU_INSTR_PTR	spu_decode_op_solver( uint32_t op );
SPU_OP_COMPONENTS spu_decode_op_components( uint32_t raw_instr );

#endif