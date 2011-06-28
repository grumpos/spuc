#include <cstdint>
#include <algorithm>
#include "ppcdis.h"


enum ppc_iform_t
{
	IFORM_I = 0,
	IFORM_B,
	IFORM_SC,
	IFORM_D,
	IFORM_DS,
	IFORM_X,
	IFORM_XL,
	IFORM_XFX,
	IFORM_XFL,
	IFORM_XS,
	IFORM_XO,
	IFORM_A,
	IFORM_M,
	IFORM_MD,
	IFORM_MDS
};

union ppc_i_t
{	
	struct { uint32_t LK : 1; uint32_t AA : 1; uint32_t LI : 24;									uint32_t OPCD : 6; } _I;
	struct { uint32_t LK : 1; uint32_t AA : 1; uint32_t BD : 14; uint32_t BI : 5; uint32_t BO : 5;	uint32_t OPCD : 6; } _B;
	struct { uint32_t LK : 1; uint32_t pad1 : 3; uint32_t LEV : 7; uint32_t pad0 : 14;				uint32_t OPCD : 6; } _SC;
	struct { uint32_t D : 16; uint32_t RA : 5; uint32_t RT : 5;										uint32_t OPCD : 6; } _D;
	struct { uint32_t XO : 2; uint32_t DS : 14; uint32_t RA : 5; uint32_t RT : 5;					uint32_t OPCD : 6; } _DS;
	struct { uint32_t Rc : 1; uint32_t XO : 10; uint32_t pad0 : 15;									uint32_t OPCD : 6; } _X;
	struct { uint32_t Rc : 1; uint32_t XO : 10; uint32_t pad0 : 15;									uint32_t OPCD : 6; } _XL;
	struct { uint32_t pad1 : 1; uint32_t XO : 10; uint32_t pad0 : 15;								uint32_t OPCD : 6; } _XFX;
	struct { uint32_t Rc : 1; uint32_t XO : 10; uint32_t pad0 : 15;									uint32_t OPCD : 6; } _XFL;
	struct { uint32_t Rc : 1; uint32_t sh : 1; uint32_t XO : 9; uint32_t pad0 : 15;					uint32_t OPCD : 6; } _XS;
	struct { uint32_t Rc : 1; uint32_t XO : 9; uint32_t pad0 : 16;									uint32_t OPCD : 6; } _XO;
	struct { uint32_t Rc : 1; uint32_t XO : 5; uint32_t FRC : 5; uint32_t FRB : 5; uint32_t FRA : 5; uint32_t FRT : 5; uint32_t OPCD : 6; } _A;
	struct { uint32_t Rc : 1; uint32_t pad0 : 25;													uint32_t OPCD : 6; } _M;
	struct { uint32_t Rc : 1; uint32_t sh : 1; uint32_t XO : 3; uint32_t pad0 : 21;					uint32_t OPCD : 6; } _MD;
	struct { uint32_t Rc : 1; uint32_t XO : 4; uint32_t pad0 : 21;									uint32_t OPCD : 6; } _MDS;
	uint32_t Instuction;
};

static_assert( sizeof(ppc_i_t) == 4, "" );

struct ppc_instr_info
{
	const char* mnemonic;
	ppc_iform_t iform;
	uint8_t op;
	uint16_t xop;
};

#define DEFINST(NAME,IFORM,OP,XOP) { NAME,IFORM,(uint8_t)OP,(uint16_t)XOP },

static const ppc_instr_info db_ppc_instr_info[] =
{
	#include "ppc_ilist.h"
};

const char* ppc_decode_mnem( uint32_t i )
{
	const ppc_i_t instr = {i};
	const auto ilist_begin	= db_ppc_instr_info;
	const auto ilist_end	= ilist_begin + _countof(db_ppc_instr_info);

	auto find_res_op = std::find_if( ilist_begin, ilist_end, 
		[=]( const ppc_instr_info& ii ) -> bool
	{
		return ii.op == instr._I.OPCD;
	} );

	if ( find_res_op != ilist_end )
	{
		if ( find_res_op->xop == (uint16_t)-1 )
		{
			return find_res_op->mnemonic;
		}
		else
		{
			auto find_res_xop = std::find_if( find_res_op, ilist_end, 
				[=]( const ppc_instr_info& ii ) -> bool
			{
				switch ( ii.iform )
				{
					case IFORM_DS:	return instr._DS.XO == ii.xop;
					case IFORM_X:	return instr._X.XO == ii.xop;
					case IFORM_XL:	return instr._XL.XO == ii.xop;
					case IFORM_XFX: return instr._XFX.XO == ii.xop;
					case IFORM_XFL:	return instr._XFL.XO == ii.xop;
					case IFORM_XS:	return instr._XS.XO == ii.xop;
					case IFORM_XO:	return instr._XO.XO == ii.xop;
					case IFORM_A:	return instr._A.XO == ii.xop;
					case IFORM_MD:	return instr._MD.XO == ii.xop;
					case IFORM_MDS: return instr._MDS.XO == ii.xop;
					default : return false;
				}
			} );

			if ( find_res_xop != ilist_end )
			{
				return find_res_xop->mnemonic;
			}	
		}		
	}

	return "ERROR";
}