#include <cstdint>
#include <algorithm>
//#include <string>
#include "ppcdis.h"
#include "ppc-opcode.h"


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

typedef uint32_t u32;

union ppc_i_t
{	
	struct { u32 LK : 1; u32 AA : 1; u32 LI : 24;							u32 OPCD : 6; } _I;
	struct { u32 LK : 1; u32 AA : 1; u32 BD : 14; u32 BI : 5; u32 BO : 5;	u32 OPCD : 6; } _B;
	struct { u32 LK : 1; u32 pad1 : 3; u32 LEV : 7; u32 pad0 : 14;			u32 OPCD : 6; } _SC;
	struct { u32 D : 16; u32 RA : 5; u32 RT : 5;							u32 OPCD : 6; } _D;
	struct { u32 XO : 2; u32 DS : 14; u32 RA : 5; u32 RT : 5;				u32 OPCD : 6; } _DS;
	struct { u32 Rc : 1; u32 XO : 10; u32 pad0 : 15;						u32 OPCD : 6; } _X;
	struct { u32 Rc : 1; u32 XO : 10; u32 pad0 : 15;						u32 OPCD : 6; } _XL;
	struct { u32 un0 : 1; u32 XO : 10; u32 spr : 10; u32 RS : 5;			u32 OPCD : 6; } _XFX;
	struct { u32 Rc : 1; u32 XO : 10; u32 pad0 : 15;						u32 OPCD : 6; } _XFL;
	struct { u32 Rc : 1; u32 sh : 1; u32 XO : 9; u32 pad0 : 15;				u32 OPCD : 6; } _XS;
	struct { u32 Rc : 1; u32 XO : 9; u32 pad0 : 16;							u32 OPCD : 6; } _XO;
	struct { u32 Rc : 1; u32 XO : 5; u32 FRC : 5; u32 FRB : 5; u32 FRA : 5; u32 FRT : 5; u32 OPCD : 6; } _A;
	struct { u32 Rc : 1; u32 pad0 : 25;										u32 OPCD : 6; } _M;
	struct { u32 Rc : 1; u32 sh : 1; u32 XO : 3; u32 pad0 : 21;				u32 OPCD : 6; } _MD;
	struct { u32 Rc : 1; u32 XO : 4; u32 pad0 : 21;							u32 OPCD : 6; } _MDS;
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

const char* ppc_decode_mnem2( uint32_t Instr )
{
	for ( size_t i = 0; i < _countof(ppc_opcodes); ++i )
	{
		uint32_t opc = ppc_opcodes[i].opcode;
		if ( (opc>>26 == Instr>>26) && ((opc & Instr) == opc) )
			return ppc_opcodes[i].name;
	}

	return "";
}

const char* ppc_decode_mnem( uint32_t i )
{
	ppc_i_t instr;
	instr.Instuction = i;
	const auto ilist_begin	= db_ppc_instr_info;
	const auto ilist_end	= ilist_begin + _countof(db_ppc_instr_info);

	auto MatchByOP = std::find_if( ilist_begin, ilist_end, 
		[=]( const ppc_instr_info& ii ) -> bool
	{
		return ii.op == instr._I.OPCD;
	} );	

	if ( MatchByOP != ilist_end )
	{
		if ( MatchByOP->xop == (uint16_t)-1 )
		{
			/*if ( IFORM_D == MatchByOP->iform )
			{
				static char buf[256];
				sprintf_s( buf, "%s\t$%d, $%d, %%%d", MatchByOP->mnemonic, instr._D.RT,  instr._D.RA, instr._D.D );
				return buf;
			}*/

			return MatchByOP->mnemonic;
		}
		else
		{
			auto MatchByXOP = std::find_if( MatchByOP, ilist_end, 
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

			if ( MatchByXOP != ilist_end )
			{
				return MatchByXOP->mnemonic;
			}	
		}		
	}

	

	if ( 4 == instr._I.OPCD )
		return "AVX";

	static char buf[64];

	_itoa_s( i, buf, 64, 16 );
	return buf;
}

//#define LOAD_BYTE( base, off ) *(uin8_t*)((uin8_t*)(base) + (off));
//#define LOAD_HALFZ( base, off ) *(uin16_t*)((uin8_t*)(base) + (off));
//#define LOAD_HALFX( base, off ) *(in16_t*)((uin8_t*)(base) + (off));
//
//static const std::string templates[] =
//{
//	"lbz",		"GPR($RT$) = *(uin8_t*)MEM(GPR($RA$) + $IMM$);",
//	"lbzx",		"GPR($RT$) = *(uin8_t*)MEM(GPR($RA$) + GPR($RB$));",
//	"lbzu",		"GPR($RT$) = *(uin8_t*)MEM(GPR($RA$) + $IMM$); GPR($RA$) = GPR($RA$) + $IMM$;",
//	"lbzux",	"GPR($RT$) = *(uin8_t*)MEM(GPR($RA$) + GPR($RB$)); GPR($RA$) = GPR($RA$) + GPR($RB$);",
//	"lhz",		"GPR($RT$) = *(uin16_t*)MEM(GPR($RA$) + $IMM$);",
//	"lhzx",		"GPR($RT$) = *(uin16_t*)MEM(GPR($RA$) + GPR($RB$));",
//	"lhzu",		"GPR($RT$) = *(uin16_t*)MEM(GPR($RA$) + $IMM$); GPR($RA$) = GPR($RA$) + $IMM$;",
//	"lhzux",	"GPR($RT$) = *(uin16_t*)MEM(GPR($RA$) + GPR($RB$)); GPR($RA$) = GPR($RA$) + GPR($RB$);",
//};

