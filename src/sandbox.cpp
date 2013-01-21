#include <vector>

#include "spu_idb.h"

#include "..\\..\\spu_intrin\\src\spu_internals_x86.h"

using namespace std;


//#define si_lqd(ra,imm)      (*(const GPR_t*)(&__LS[((si_to_uint((ra)))+(imm<<4)) & LS_MEM_OP_MASK]))
#define si_lqx(ra,rb)       (*(const GPR_t*)(&__LS[((si_to_uint((ra)))+(si_to_uint((rb)))) & LS_MEM_OP_MASK])) 
#define si_lqa(imm)         *((const GPR_t*)(__LS + (imm & LS_MEM_OP_MASK)))
#define si_lqr(imm)         si_lqa(imm)
#define si_stqd(rt,ra,imm)  *(GPR_t*)(&__LS[((si_to_uint((ra)))+(imm<<4)) & LS_MEM_OP_MASK]) = (rt)
#define si_stqx(rt,ra,rb)   *(GPR_t*)(&__LS[((si_to_uint((ra)))+(si_to_uint((rb)))) & LS_MEM_OP_MASK]) = (rt) 
#define si_stqa(rt,imm)     *((GPR_t*)(__LS + (imm & LS_MEM_OP_MASK))) = (rt)
#define si_stqr(rt,imm)		si_stqa(rt,imm)

static const size_t LSLR = 0x3FFFF;
static const size_t LS_MEM_OP_MASK = LSLR & -16;

typedef __m128 GPR_t;

static __m128 __Registers[128];

static uint8_t* __LS = nullptr;
static __m128* __LS_128;



//inline __m128 si_lqd( __m128 r, size_t o )
//{
//	const size_t base = si_to_uint(r);
//	const size_t addr = base + (o<<4);
//	return (*(const GPR_t*)(&__LS[addr & LS_MEM_OP_MASK]));	
//}

__m128 si_lqd( __m128 r, size_t o )
{
	const size_t base = si_to_uint(r);
	const size_t addr = base + (o<<4);
	GPR_t gpr = (*(const GPR_t*)(&__LS[addr & LS_MEM_OP_MASK]));
	return gpr;	
	//const auto all = _MM_CONST_ALL(_mm_castps_si128(r));
	//const auto offset = _mm_set1_epi32(o);
	//const auto offset_vaddr = _mm_shl_epi32(offset,_mm_srli_si128(_mm_slli_si128(all, 31), 29));
	//const auto addr = _mm_add_epi32(_mm_castps_si128(r), offset_vaddr);
	//const auto addr_masked = _mm_and_si128(addr, _mm_srli_si128(_mm_slli_si128(all, 14), 10));
	//return (*(const GPR_t*)(&__LS[si_to_uint(_mm_castsi128_ps(addr_masked))]));
}

#define CALL_RRR(type, handler) case spu_op::type: RT = handler(RA, RB, RC); break;
#define CALL_RR(type, handler) case spu_op::type: RT = handler(RA, RB); break;
#define CALL_RR_RA(type, handler) case spu_op::type: RT = handler(RA); break;
#define CALL_I7(type, handler) case spu_op::type: RT = handler(RA, IMM); break;
#define CALL_I8(type, handler) case spu_op::type: RT = handler(RA, IMM); break;
#define CALL_I10(type, handler) case spu_op::type: RT = handler(RA, IMM); break;
#define CALL_I16(type, handler) case spu_op::type: RT = handler(IMM); break;
#define CALL_I18(type, handler) case spu_op::type: RT = handler(IMM); break;
#define CALL_VOID(type, handler) case spu_op::type: handler(); break;


auto jump = [](spu_insn* PC, uint32_t vbase, uint32_t target)
{
	auto ibase = PC - (PC->vaddr - vbase) / 4; 
	PC = ibase + (target - vbase) / 4; 
	return PC;
};

void spu_vm_dostep(spu_vm* vm)
{
	spu_insn* PC = vm->next;
	__LS = vm->LS;
	SPU_OP_COMPONENTS& OPComp = PC->comps;
	spu_gpr& RT = vm->GPR[OPComp.RT];
	spu_gpr& RA = vm->GPR[OPComp.RA];
	spu_gpr& RB = vm->GPR[OPComp.RB];
	spu_gpr& RC = vm->GPR[OPComp.RC];
	int32_t IMM = PC->comps.IMM;

	// Default advance
	++vm->next;

	switch (PC->op)
	{
		CALL_I16(M_ILH, si_ilh)
		CALL_I16(M_ILHU, si_ilhu)
		CALL_I16(M_IL, si_il)
		CALL_I18(M_ILA, si_ila)
		case spu_op::M_IOHL: RT = si_iohl(RT, IMM); break;
		CALL_I16(M_FSMBI, si_fsmbi)

		CALL_RR(M_AH, si_ah)
		CALL_I10(M_AHI, si_ahi)
		CALL_RR(M_A, si_a)
		CALL_I10(M_AI, si_ai)
		CALL_RR(M_CG, si_cg)
		case spu_op::M_CGX: RT = si_cgx(RA, RB, RT); break;
		case spu_op::M_ADDX: RT = si_addx(RA, RB, RT); break;

		CALL_RR(M_BG,	si_bg)
		case spu_op::M_BGX: RT = si_bgx(RA, RB, RT); break;
		case spu_op::M_SFX: RT = si_sfx(RA, RB, RT); break;



		CALL_RR(M_ABSDB,	si_absdb)
		CALL_RR(M_AND,	si_and)
		CALL_I10(M_ANDBI,	si_andbi)
		CALL_RR(M_ANDC,	si_andc)
		CALL_I10(M_ANDHI,	si_andhi)
		CALL_I10(M_ANDI,	si_andi)
		CALL_RR(M_AVGB,	si_avgb)

		case spu_op::M_BR: vm->next += (IMM-1); break;
		case spu_op::M_BRHNZ: if ( 0 != si_to_short((RT)) ) vm->next += (IMM-1); break;
		case spu_op::M_BRHZ: if ( 0 == si_to_short((RT)) ) vm->next += (IMM-1); break;
		case spu_op::M_BRNZ: if ( 0 != si_to_int((RT)) ) vm->next += (IMM-1); break;
		case spu_op::M_BRZ: if ( 0 == si_to_int((RT)) ) vm->next += (IMM-1); break;

#define JMP_TO_GPR vm->next = jump(PC, vm->vbase, si_to_uint(RA));
		case spu_op::M_BI: 
			{
				if (0 != OPComp.RA)
				{
					JMP_TO_GPR;
				}
				else
				{
					vm->next = vm->LR;
				}
			}
			break;
		case spu_op::M_BIHNZ: if ( 0 != si_to_short((RT)) ) JMP_TO_GPR; break;
		case spu_op::M_BIHZ: if ( 0 == si_to_short((RT)) ) JMP_TO_GPR; break;
		case spu_op::M_BINZ: if ( 0 != si_to_int((RT)) ) JMP_TO_GPR; break;
		case spu_op::M_BIZ: if ( 0 == si_to_int((RT)) ) JMP_TO_GPR; break;
#undef JMP_TO_GPR

		
		

		case spu_op::M_BRSL: 
			{
				RT = _mm_castsi128_ps(si_from_uint(vm->next->vaddr));
				vm->LR = vm->next;
				vm->next += (IMM-1); break;
			}
		case spu_op::M_BISL: 
			{
				RT = _mm_castsi128_ps(si_from_uint(vm->next->vaddr));
				vm->LR = vm->next;
				auto t = si_to_uint(RA);
				auto ibase = vm->next - (vm->next->vaddr - vm->vbase) / 4;
				vm->next = ibase + (t - vm->vbase) / 4; break;
			}
			
		/*
		CALL_RR(M_BRA,	si_bra)
		CALL_RR(M_BRASL,	si_brasl)*/
		CALL_I7(M_CBD,	si_cbd)
		CALL_RR(M_CBX,	si_cbx)
		CALL_I7(M_CDD,	si_cdd)
		CALL_RR(M_CDX,	si_cdx)
		CALL_RR(M_CEQ,	si_ceq)
		CALL_RR(M_CEQB,	si_ceqb)
		CALL_I10(M_CEQBI,	si_ceqbi)
		CALL_RR(M_CEQH,	si_ceqh)
		CALL_I10(M_CEQHI,	si_ceqhi)
		CALL_I10(M_CEQI,	si_ceqi)
		//CALL_RR(M_CFLTS,	si_cflts)
		//CALL_RR(M_CFLTU,	si_cfltu)
		CALL_RR(M_CGT,	si_cgt)
		CALL_RR(M_CGTB,	si_cgtb)
		CALL_I10(M_CGTBI,	si_cgtbi)
		CALL_RR(M_CGTH,	si_cgth)
		CALL_I10(M_CGTHI,	si_cgthi)
		CALL_I10(M_CGTI,	si_cgti)
		CALL_I7(M_CHD,	si_chd)
		CALL_RR(M_CHX,	si_chx)
		CALL_RR(M_CLGT,	si_clgt)
		CALL_RR(M_CLGTB,	si_clgtb)
		CALL_I10(M_CLGTBI,	si_clgtbi)
		CALL_RR(M_CLGTH,	si_clgth)
		CALL_I10(M_CLGTHI,	si_clgthi)
		CALL_I10(M_CLGTI,	si_clgti)
		CALL_RR_RA(M_CLZ,	si_clz)
		CALL_RR_RA(M_CNTB,	si_cntb)
		//CALL_I8(M_CSFLT,	si_csflt)
		//CALL_I8(M_CUFLT,	si_cuflt)
		CALL_I7(M_CWD,	si_cwd)
		CALL_RR(M_CWX,	si_cwx)
		//CALL_RR(M_DFA,	si_dfa)
		//CALL_RR(M_DFM,	si_dfm)
		//CALL_RR(M_DFMA,	si_dfma)
		//CALL_RR(M_DFMS,	si_dfms)
		//CALL_RR(M_DFNMA,	si_dfnma)
		//CALL_RR(M_DFNMS,	si_dfnms)
		//CALL_RR(M_DFS,	si_dfs)
		CALL_VOID(M_DSYNC,	si_dsync)
		CALL_RR(M_EQV,	si_eqv)
		/*CALL_RR(M_FA,	si_fa)
		CALL_RR(M_FCEQ,	si_fceq)
		CALL_RR(M_FCGT,	si_fcgt)
		CALL_RR(M_FCMEQ,	si_fcmeq)
		CALL_RR(M_FCMGT,	si_fcmgt)
		CALL_RR(M_FESD,	si_fesd)
		CALL_RR(M_FI,	si_fi)
		CALL_RR(M_FM,	si_fm)
		CALL_RR(M_FMA,	si_fma)
		CALL_RR(M_FMS,	si_fms)
		CALL_RR(M_FNMS,	si_fnms)
		CALL_RR(M_FRDS,	si_frds)
		CALL_RR(M_FREST,	si_frest)
		CALL_RR(M_FRSQEST,	si_frsqest)
		CALL_RR(M_FS,	si_fs)
		CALL_RR(M_FSCRRD,	si_fscrrd)
		CALL_RR(M_FSCRWR,	si_fscrwr)
		CALL_RR(M_FSCRWR2,	si_fscrwr)*/
		CALL_RR_RA(M_FSM,	si_fsm)
		CALL_RR_RA(M_FSMB,	si_fsmb)
		CALL_RR_RA(M_FSMH,	si_fsmh)
		CALL_RR_RA(M_GB,	si_gb)
		CALL_RR_RA(M_GBB,	si_gbb)
		CALL_RR_RA(M_GBH,	si_gbh)
		/*CALL_RR(M_HBR,	si_hbr)
		CALL_RR(M_HBRA,	si_hbra)
		CALL_RR(M_HBRR,	si_hbrr)*/

		//CALL_RR(M_HEQ,	si_heq)
		//CALL_RR(M_HEQ2,	si_heq)
		//CALL_RR(M_HEQI,	si_heqi)
		//CALL_RR(M_HEQI2,	si_heqi)
		//CALL_RR(M_HGT,	si_hgt)
		//CALL_RR(M_HGT2,	si_hgt)
		//CALL_RR(M_HGTI,	si_hgti)
		//CALL_RR(M_HGTI2,	si_hgti)
		//CALL_RR(M_HLGT,	si_hlgt)
		//CALL_RR(M_HLGT2,	si_hlgt)
		//CALL_RR(M_HLGTI,	si_hlgti)
		//CALL_RR(M_HLGTI2,	si_hlgti)

		/*CALL_RR(M_IRET,	si_iret)
		CALL_RR(M_IRET2,	si_iret)
		CALL_RR(M_LNOP,	si_lnop)*/
		CALL_I16(M_LQA,	si_lqa)
		CALL_I10(M_LQD,	si_lqd)
		CALL_I16(M_LQR,	si_lqr)
		CALL_RR(M_LQX,	si_lqx)
		/*CALL_RR(M_MFSPR,	si_mfspr)*/
		CALL_RR(M_MPY,	si_mpy)
		CALL_RRR(M_MPYA,	si_mpya)
		CALL_RR(M_MPYH,	si_mpyh)
		CALL_RR(M_MPYHH,	si_mpyhh)
		case spu_op::M_MPYHHA: RT = si_mpyhha(RA, RB, RT); break;
		case spu_op::M_MPYHHAU: RT = si_mpyhhau(RA, RB, RT); break;
		CALL_RR(M_MPYHHU,	si_mpyhhu)
		CALL_I10(M_MPYI,	si_mpyi)
		CALL_RR(M_MPYS,	si_mpys)
		CALL_RR(M_MPYU,	si_mpyu)
		CALL_I10(M_MPYUI,	si_mpyui)
		/*CALL_RR(M_MTSPR,	si_mtspr)*/
		CALL_RR(M_NAND,	si_nand)
		/*CALL_RR(M_NOP,	si_nop)
		CALL_RR(M_NOP2,	si_nop)*/
		CALL_RR(M_NOR,	si_nor)
		CALL_RR(M_OR,	si_or)
		CALL_I10(M_ORBI,	si_orbi)
		CALL_RR(M_ORC,	si_orc)
		CALL_I10(M_ORHI,	si_orhi)
		CALL_I10(M_ORI,	si_ori)
		CALL_RR_RA(M_ORX,	si_orx)
		case spu_op::M_RCHCNT: si_rchcnt(si_to_int(RA)); break;
		case spu_op::M_RDCH: si_rdch(si_to_int(RA)); break;

		CALL_RR(M_ROT,	si_rot)
		CALL_RR(M_ROTH,	si_roth)
		CALL_I7(M_ROTHI,	si_rothi)
		CALL_RR(M_ROTHM,	si_rothm)
		CALL_I7(M_ROTHMI,	si_rothmi)
		CALL_I7(M_ROTI,	si_roti)
		CALL_RR(M_ROTM,	si_rotm)
		CALL_RR(M_ROTMA,	si_rotma)
		CALL_RR(M_ROTMAH,	si_rotmah)
		CALL_I7(M_ROTMAHI,	si_rotmahi)
		CALL_I7(M_ROTMAI,	si_rotmai)
		CALL_I7(M_ROTMI,	si_rotmi)
		CALL_RR(M_ROTQBI,	si_rotqbi)
		CALL_I7(M_ROTQBII,	si_rotqbii)
		CALL_RR(M_ROTQBY,	si_rotqby)
		CALL_RR(M_ROTQBYBI,	si_rotqbybi)
//		CALL_I7(M_ROTQBYI,	si_rotqbyi)
		CALL_RR(M_ROTQMBI,	si_rotqmbi)
		CALL_I7(M_ROTQMBII,	si_rotqmbii)
		CALL_RR(M_ROTQMBY,	si_rotqmby)
		CALL_RR(M_ROTQMBYBI,	si_rotqmbybi)
//		CALL_I7(M_ROTQMBYI,	si_rotqmbyi)
		CALL_RRR(M_SELB,	si_selb)
		CALL_RR(M_SF,	si_sf)
		CALL_RR(M_SFH,	si_sfh)
		CALL_I10(M_SFHI,	si_sfhi)
		CALL_I10(M_SFI,	si_sfi)
		CALL_RR(M_SHL,	si_shl)
		CALL_RR(M_SHLH,	si_shlh)
		CALL_I7(M_SHLHI,	si_shlhi)
		CALL_I7(M_SHLI,	si_shli)
		CALL_RR(M_SHLQBI,	si_shlqbi)
		CALL_I7(M_SHLQBII,	si_shlqbii)
		CALL_RR(M_SHLQBY,	si_shlqby)
		CALL_RR(M_SHLQBYBI,	si_shlqbybi)
//		CALL_I7(M_SHLQBYI,	si_shlqbyi)
		CALL_RRR(M_SHUFB,	si_shufb)
		case spu_op::M_STOP: vm->next = nullptr; break;
		//CALL_RR(M_STOPD,	si_stopd)
		case spu_op::M_STQA: si_stqa(RT, IMM); break;
		case spu_op::M_STQD: si_stqd(RT, RA, IMM); break;
		case spu_op::M_STQR: si_stqr(RT, IMM); break;
		case spu_op::M_STQX: si_stqx(RT, RA, RB); break;
		CALL_RR(M_SUMB,	si_sumb)
		CALL_VOID(M_SYNC,	si_sync)
		case spu_op::M_WRCH: si_wrch(si_to_int(RA), RA); break;
		CALL_RR(M_XOR,	si_xor)
		CALL_I10(M_XORBI,	si_xorbi)
		CALL_I10(M_XORHI,	si_xorhi)
		CALL_I10(M_XORI,	si_xori)
		CALL_RR_RA(M_XSBH,	si_xsbh)
		CALL_RR_RA(M_XSHW,	si_xshw)
		CALL_RR_RA(M_XSWD,	si_xswd)
		//CALL_RR(M_SYSCALL,    si_syscall)

		//CALL_RR(M_DFCEQ,	si_dfceq)
		//CALL_RR(M_DFCMEQ,	si_dfcmeq)
		//CALL_RR(M_DFCGT,	si_dfcgt)
		//CALL_RR(M_DFCMGT,	si_dfcmgt)
		//CALL_RR(M_DFTSV,	si_dftsv)

	default:
		break;
	}

}




