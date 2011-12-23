#include "spu_idb.h"
#include "spu_emu.h"
#include <iostream>
#include <set>
#include <algorithm>


void spu_dummy_solver( struct SPU_t*, union SPU_INSTRUCTION op )
{
	std::cout << "Invalid instruction: " << spu_decode_op_mnemonic( op.Instruction ) << std::endl; 
}

#define SPU_LINK_MNEM_TO_SOLVER(m) spu_mnem_op_t( #m, &spu_##m )

static const spu_mnem_op_t _solvers[] = 
{
	
	SPU_LINK_MNEM_TO_SOLVER( dummy_solver )
};



static std::vector<SPU_OP_TYPE>			db_op_type( SPU_MAX_INSTRUCTION_COUNT, size_t(-1) );
static std::vector<std::string>			db_op_mnemonic( SPU_MAX_INSTRUCTION_COUNT );
static std::vector<SPU_INSTR_PTR>		db_op_solver( SPU_MAX_INSTRUCTION_COUNT, spu_dummy_solver );
std::vector<std::vector<SPU_ARGLIST>>	db_op_arglist( SPU_MAX_INSTRUCTION_COUNT );

/*
struct SPU_IDB
{
	size_t type;
	const char* mnemonic;
};

static const SPU_IDB db_op_ltb[] =
{
#define _A0()        {0xFF,{0xFF,0xFF,0xFF,0}}
#define _A1(a)       {1,{a,0xFF,0xFF,0}}
#define _A2(a,b)     {2,{a,b,0xFF,0}}
#define _A3(a,b,c)   {3,{a,b,c,0}}
#define _A4(a,b,c,d) {4,{a,b,c,d}}

#define APUOP(TAG, FORMAT, OPCODE, MNEMONIC, ASM_FORMAT, DEPENDENCY, PIPE) \
	{ SPU_OP_TYPE_##FORMAT, MNEMONIC },

	APUOP(M_DFCEQ,		RR,	0x3C3,	"dfceq",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFCMEQ,		RR,	0x3CB,	"dfcmeq",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFCGT,		RR,	0x2C3,	"dfcgt",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFCMGT,		RR,	0x2CB,	"dfcmgt",	_A3(A_T,A_A,A_B),		0x00112,	FPD)
	APUOP(M_DFTSV,		RI7,0x3BF,	"dftsv",	_A3(A_T,A_A,A_U7),		0x00112,	FPD)

#include "spu-insns.h"

#undef APUOP
};*/

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
	SPU_ARGLIST al = ASM_FORMAT; \
	db_op_arglist[OPCODE].push_back( al ); \
}

#define APUOPFB(TAG,		FORMAT,	OPCODE,	FEATUREBIT, MNEMONIC,	ASM_FORMAT,	DEPENDENCY,	PIPE)/* \
{ \
	db_op_type[OPCODE] = SPU_OP_TYPE_##FORMAT; \
	db_op_mnemonic[OPCODE] = MNEMONIC; \
	SPU_ARGLIST al = ASM_FORMAT; \
	db_op_arglist[OPCODE].push_back( al ); \
}*/

#include "spu-insns.h"
	
}

#undef APUOP
#undef APUOPFB

void link_mnem_to_solver();

size_t spu_decode_op_opcode( uint32_t op )
{
	static bool opcodes_parsed = false;

	if ( !opcodes_parsed )
	{
		spu_build_op_db();
		//link_mnem_to_solver();
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

	return op;
}

SPU_OP_TYPE spu_decode_op_type( uint32_t op )
{
	return db_op_type[spu_decode_op_opcode( op )];
}

const char* spu_decode_op_mnemonic( uint32_t op )
{
	return db_op_mnemonic[spu_decode_op_opcode( op )].c_str();
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

//void link_mnem_to_solver()
//{
//	std::set<std::string> unimp;
//
//	size_t i = 0;
//	std::for_each( db_op_mnemonic.begin(), db_op_mnemonic.end(), [&unimp, &i]( const std::string& mnem ) {
//		if ( !mnem.empty() ) {
//			const auto mnem_solver = std::find_if( _solvers, _solvers + _countof(_solvers), [&]( const spu_mnem_op_t& x ) {
//				return mnem == x.first;
//			} );
//
//			if ( mnem_solver != (_solvers + _countof(_solvers)) ) {
//				db_op_solver[i] = mnem_solver->second;
//			}
//			else {		
//				unimp.insert( mnem );
//			}
//		}
//
//		++i;
//	});	
//
//	/*std::ofstream unimp_out("unimp.txt");
//	std::copy( unimp.begin(), unimp.end(), std::ostream_iterator<std::string>(unimp_out, "\n") );*/
//}