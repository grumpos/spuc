#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "spu_emu.h"
#include "spu_idb.h"

using namespace std;

string reg2str( uint8_t r )
{
	switch (r)
	{
	case 0: return "lr";
	case 1: return "sr";

	default:
		{
			char buf[8];
			_itoa_s( r, buf, 10 );
			return buf;
		}
	};
}

inline uint32_t addr( int64_t val )
{
	return 0x3FFFF & (uint32_t)(uint64_t)val;
}

static bool initDone = false;

void InitLTBL()
{

}

string spu_disassemble( const spu_insn* insn )
{
	ostringstream oss;

	oss << setw(8) << hex << insn->vaddr << ":\t";

	oss << setw(2) << setfill('0') << hex << (int)(((uint8_t*)&insn->raw)[3]) << " ";
	oss << setw(2) << setfill('0') << hex << (int)(((uint8_t*)&insn->raw)[2]) << " ";
	oss << setw(2) << setfill('0') << hex << (int)(((uint8_t*)&insn->raw)[1]) << " ";
	oss << setw(2) << setfill('0') << hex << (int)(((uint8_t*)&insn->raw)[0]) << " ";

	oss << spu_decode_op_mnemonic(insn->raw) << "\t";

	switch ( spu_decode_op_type(insn->raw) )
	{
	case SPU_OP_TYPE_RRR:
		{
			oss << " $" << reg2str(insn->comps.RT) 
				<< ", $" << reg2str(insn->comps.RA) 
				<< ", $" << reg2str(insn->comps.RB) 
				<< ", $" << reg2str(insn->comps.RC);
			break;
		}

	case SPU_OP_TYPE_RR:
		{
			const char* ch_ltb[31] = 
			{
				"$SPU_RdEventStat",
				"$SPU_WrEventMask",
				"$SPU_WrEventAck",
				"$SPU_RdSigNotify1",
				"$SPU_RdSigNotify2",
				"$ch6",
				"$ch7",
				"$SPU_WrDec",
				"$SPU_RdDec",
				"$MFC_WrMSSyncReq",
				"$ch10",
				"$SPU_RdEventMask",
				"$MFC_RdTagMask",
				"$SPU_RdMachStat",
				"$SPU_WrSRR0",
				"$SPU_RdSRR0",
				"$MFC_LSA",
				"$MFC_EAH",
				"$MFC_EAL",
				"$MFC_Size",
				"$MFC_TagID",
				"$MFC_Cmd",
				"$MFC_WrTagMask",
				"$MFC_WrTagUpdate",
				"$MFC_RdTagStat",
				"$MFC_RdListStallStat",
				"$MFC_WrListStallAck",
				"$MFC_RdAtomicStat", 
				"$SPU_WossMbox", 
				"$SPU_RdInMbox", 
				"$SPU_WossIntrMbox"
			};

			if ( insn->op == spu_op::M_RDCH || insn->op == spu_op::M_RCHCNT )
			{
				oss << " $" << reg2str(insn->comps.RT); 
				if ( insn->comps.RA < 31 )
				{
					oss << ", " << ch_ltb[insn->comps.RA];
				}
				else
				{
					oss << ", $ch" << reg2str(insn->comps.RA);
				}
			}
			else if ( insn->op == spu_op::M_WRCH )
			{				
				if ( insn->comps.RA < 31 )
				{
					oss << ch_ltb[insn->comps.RA];
				}
				else
				{
					oss << "$ch" << reg2str(insn->comps.RA);
				}
				oss << ", $" << reg2str(insn->comps.RT); 
			}
			else if ( insn->op == spu_op::M_NOP ||
				insn->op == spu_op::M_LNOP ||
				insn->op == spu_op::M_SYNC ||
				insn->op == spu_op::M_DSYNC ||
				insn->op == spu_op::M_IRET ||
				insn->op == spu_op::M_STOP )
			{
				// do nothing
			}
			else if ( insn->op == spu_op::M_BI )
			{
				oss << " $" << reg2str(insn->comps.RA);
			}
			else
			{
				oss << " $" << reg2str(insn->comps.RT) 
					<< ", $" << reg2str(insn->comps.RA) 
					<< ", $" << reg2str(insn->comps.RB);
			}
			break;
		}
	case SPU_OP_TYPE_RI7:
		{
			oss << " $" << reg2str(insn->comps.RT) 
				<< ", $" << reg2str(insn->comps.RA) 
				<< ", " << dec << insn->comps.IMM;
			if (insn->comps.IMM) 
				oss << "\t#" << hex << (uint32_t)insn->comps.IMM;
			break;
		}
	case SPU_OP_TYPE_RI8:
		{
			oss << " $" << reg2str(insn->comps.RT) 
				<< ", $" << reg2str(insn->comps.RA) 
				<< ", " << dec << insn->comps.IMM;
			if (insn->comps.IMM) 
				oss << "\t#" << hex << (uint32_t)insn->comps.IMM;
			break;
		}
	case SPU_OP_TYPE_RI10:
		{
			if ( insn->op == spu_op::M_LQD 
				||  insn->op == spu_op::M_STQD )
			{
				oss << " $" << reg2str(insn->comps.RT) 
					<< ", " << dec << insn->comps.IMM
					<< "($" << reg2str(insn->comps.RA) << ")";
				break;
			}

			oss << " $" << reg2str(insn->comps.RT) 
				<< ", $" << reg2str(insn->comps.RA) 
				<< ", " << dec << insn->comps.IMM;
			if (insn->comps.IMM) 
				oss << "\t#" << hex << (uint32_t)insn->comps.IMM;
			break;
		}
	case SPU_OP_TYPE_RI16:
		{
			/*if ( insn->op == spu_op::M_LQA 
			|| insn->op == spu_op::M_LQR 
			|| insn->op == spu_op::M_STQA 
			|| insn->op == spu_op::M_STQR )
			{
			oss << " $" << reg2str(insn->comps.RT)
			<< ", " << dec << insn->comps.IMM;
			if (insn->comps.IMM) 
			oss << "\t#" << hex << (uint32_t)insn->comps.IMM;
			break;
			}*/

			if ( insn->op == spu_op::M_BR )
			{
				oss << "0x" << hex << addr(insn->vaddr + insn->comps.IMM * 4);
			}
			else if ( insn->op == spu_op::M_BRZ
				||  insn->op == spu_op::M_BRNZ
				||  insn->op == spu_op::M_BRHZ
				||  insn->op == spu_op::M_BRHNZ
				||  insn->op == spu_op::M_BRSL)
			{
				oss << " $" << reg2str(insn->comps.RT) 
					<< ", 0x" << hex << addr(insn->vaddr + insn->comps.IMM * 4);
			}
			else
			{
				oss << " $" << reg2str(insn->comps.RT) 
					<< ", " << dec << insn->comps.IMM;
				if (insn->comps.IMM) 
					oss << "\t#" << hex << (uint32_t)insn->comps.IMM;				
			}			
			break;
		}
	case SPU_OP_TYPE_RI18:
		{
			oss << " $" << reg2str(insn->comps.RT) 
				<<  ", " << dec << addr(insn->comps.IMM);
			if (insn->comps.IMM) 
				oss << "\t#" << hex << addr(insn->comps.IMM);
			break;
		}
	case SPU_OP_TYPE_LBT:
		{
			SPU_INSTRUCTION instr = {insn->raw};
			oss << " " << SignExtend( (((uint32_t)instr.LBT.ROH << 2) | (uint32_t)instr.LBT.ROL) << 2, 11 ) 
				<< ", " << SignExtend( instr.LBT.I16 << 2, 18 );
			break;
		}
	case SPU_OP_TYPE_LBTI:
		{
			SPU_INSTRUCTION instr = {insn->raw};
			oss << " " << SignExtend( (((uint32_t)instr.LBTI.ROH << 2) | (uint32_t)instr.LBTI.ROL) << 2, 11 ) 
				<< ", $" << SignExtend( instr.LBTI.RA, 18 );
			break;
		}
	default:
		{
			oss << "ERROR unknown instruction format:" << hex << insn->raw;
		}
	};

	return oss.str();
}