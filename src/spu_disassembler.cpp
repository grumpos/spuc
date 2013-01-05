#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "spu_emu.h"
#include "spu_idb.h"

using namespace std;

std::string reg2str( uint8_t r )
{
	switch (r)
	{
	case 0: return "LR";
	case 1: return "SR";

	default:
		{
			char buf[8];
			_itoa_s( r, buf, 10 );
			return buf;
		}
	};
}




void printBasicInsnInfo(uint32_t op, ostream& os)
{
	os << setw(8) << setfill('0') << hex << op;

	string mnemonic = spu_decode_op_mnemonic(op);
	mnemonic.resize(8, ' ');

	os << ":\t" << mnemonic << "\t";
}

auto DefaultDisasm = [](uint32_t op, ostream& os)
{
	printBasicInsnInfo(op, os);

	/*const SPU_OP_COMPONENTS OPComp = spu_decode_op_components(op);



	switch ( spu_decode_op_type(op) )
	{
	case SPU_OP_TYPE_RRR:
	os << " $" << reg2str(OPComp.RT) << ", $" << reg2str(OPComp.RA) 
	<< ", $" << reg2str(OPComp.RB) << ", $" << reg2str(OPComp.RC) << endl;
	break;
	default:
	os << "ERROR unknown instruction format:";
	};*/
};

static bool initDone = false;
static vector<decltype(DefaultDisasm)> op2strLTBL(0x800, DefaultDisasm);

void InitLTBL()
{

}

std::string spu_disassemble( uint32_t instr_raw )
{
	std::ostringstream rout;

	SPU_INSTRUCTION instr;
	instr.Instruction = instr_raw;

	std::string mnemonic = spu_decode_op_mnemonic(instr_raw);
	mnemonic.resize( 8, ' ' );
	rout << mnemonic << "\t";

	switch ( spu_decode_op_type(instr_raw) )
	{
	case SPU_OP_TYPE_RRR:
		{
			rout << " $" << reg2str(instr.RRR.RT) 
				<< ", $" << reg2str(instr.RRR.RA) 
				<< ", $" << reg2str(instr.RRR.RB) 
				<< ", $" << reg2str(instr.RRR.RC);
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
				"$6",
				"$7",
				"$SPU_WrDec",
				"$SPU_RdDec",
				"$MFC_WrMSSyncReq",
				"$10",
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
				"$SPU_WrOutMbox", 
				"$SPU_RdInMbox", 
				"$SPU_WrOutIntrMbox"
			};

			if ( mnemonic == "rdch" || mnemonic == "rchcnt" )
			{
				rout << " $" << reg2str(instr.RR.RT); 
				if ( instr.RR.RA < 31 )
				{
					rout << ", " << ch_ltb[instr.RR.RA];
				}
				else
				{
					rout << ", $" << reg2str(instr.RR.RA);
				}
			}
			else if (  mnemonic == "wrch" )
			{				
				if ( instr.RR.RA < 31 )
				{
					rout << ch_ltb[instr.RR.RA];
				}
				else
				{
					rout << "$" << reg2str(instr.RR.RA);
				}
				rout << ", $" << reg2str(instr.RR.RT); 
			}
			else if ( mnemonic == "nop" ||
				mnemonic == "lnop" ||
				mnemonic == "sync" ||
				mnemonic == "dsync" ||
				mnemonic == "iret" )
			{
			}
			else if ( mnemonic == "bi" )
			{
				rout << " $" << reg2str(instr.RR.RA) << " ;----------]";
			}
			else if ( mnemonic == "stop" )
			{
				union stopi
				{
					struct 
					{
						uint32_t snstype : 14;
						uint32_t pad : 7;
						uint32_t OP : 11;						
					} stop_fmt;
					uint32_t raw;
				};

				stopi si = {instr_raw};

				rout << std::hex << si.stop_fmt.snstype;
			}
			else
			{
				rout << " $" << reg2str(instr.RR.RT) 
					<< ", $" << reg2str(instr.RR.RA) 
					<< ", $" << reg2str(instr.RR.RB);
			}
			break;
		}
	case SPU_OP_TYPE_RI7:
		{
			rout << " $" << reg2str(instr.RI7.RT) 
				<< ", $" << reg2str(instr.RI7.RA) 
				<< ", " << SignExtend( instr.RI7.I7, 7 );
			break;
		}
	case SPU_OP_TYPE_RI8:
		{
			rout << " $" << reg2str(instr.RI8.RT) 
				<< ", $" << reg2str(instr.RI8.RA) 
				<< ", " << SignExtend( instr.RI8.I8, 8 );
			break;
		}
	case SPU_OP_TYPE_RI10:
		{
			if ( mnemonic == "lqd" ||  mnemonic == "stqd" )
			{
				rout << " $" << reg2str(instr.RI10.RT) 
					<< ", " << SignExtend( instr.RI10.I10 << 4, 14 ) 
					<< "($" << reg2str(instr.RI10.RA) << ")";
				break;
			}

			rout << " $" << reg2str(instr.RI10.RT) 
				<< ", $" << reg2str(instr.RI10.RA) 
				<< ", " << SignExtend( instr.RI10.I10, 10 ) ;
			break;
		}
	case SPU_OP_TYPE_RI16:
		{
			if ( mnemonic == "lqa" || mnemonic == "lqr" || mnemonic == "stqa" || mnemonic == "stqr" )
			{
				rout << " $" << reg2str(instr.RI16.RT)
					<< ", 0x" << std::hex << SignExtend( instr.RI16.I16 << 2, 18 ) ;
				break;
			}

			rout << " $" << reg2str(instr.RI16.RT) 
				<< ", 0x" << std::hex << SignExtend( instr.RI16.I16, 16 ) ;
			break;
		}
	case SPU_OP_TYPE_RI18:
		{
			rout << " $" << reg2str(instr.RI18.RT) 
				<<  ", " << instr.RI18.I18; 
			break;
		}
	case SPU_OP_TYPE_LBT:
		{
			rout << " " << SignExtend( (((uint32_t)instr.LBT.ROH << 2) | (uint32_t)instr.LBT.ROL) << 2, 11 ) 
				<< ", " << SignExtend( instr.LBT.I16 << 2, 18 );
			break;
		}
	case SPU_OP_TYPE_LBTI:
		{
			rout << " " << SignExtend( (((uint32_t)instr.LBTI.ROH << 2) | (uint32_t)instr.LBTI.ROL) << 2, 11 ) 
				<< ", $" << SignExtend( instr.LBTI.RA, 18 );
			break;
		}
	default:
		{
			rout << "ERROR unknown instruction format:" << mnemonic;
		}
	};

	return rout.str();
}