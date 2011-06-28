#include "spu_emu.h"

static inline void HaltSPUAux( SPU_t* SPU )
{
	SPU->SPU_RunCntl.Run = SPU_RunCntl_STOP_REQUEST;
}

void spu_heq( SPU_t* SPU, SPU_INSTRUCTION op )
{
	if ( SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] == SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT] )
	{
		HaltSPUAux( SPU );
	}
}

void spu_heqi( SPU_t* SPU, SPU_INSTRUCTION op )
{
	if ( SPU->GPR[op.RI10.RA].m128_u32[GPR_PREF_SLOT] == (uint32_t)SignExtend(op.RI10.I10, 10) )
	{
		HaltSPUAux( SPU );
	}
}

void spu_hgt( SPU_t* SPU, SPU_INSTRUCTION op )
{
	if ( SPU->GPR[op.RR.RA].m128_i32[GPR_PREF_SLOT] > SPU->GPR[op.RR.RB].m128_i32[GPR_PREF_SLOT] )
	{
		HaltSPUAux( SPU );
	}
}

void spu_hgti( SPU_t* SPU, SPU_INSTRUCTION op )
{
	if ( SPU->GPR[op.RI10.RA].m128_i32[GPR_PREF_SLOT] > (int32_t)SignExtend(op.RI10.I10, 10) )
	{
		HaltSPUAux( SPU );
	}
}

void spu_hlgt( SPU_t* SPU, SPU_INSTRUCTION op )
{
	if ( SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] > SPU->GPR[op.RR.RB].m128_u32[GPR_PREF_SLOT] )
	{
		HaltSPUAux( SPU );
	}
}

void spu_hlgti( SPU_t* SPU, SPU_INSTRUCTION op )
{
	if ( SPU->GPR[op.RI10.RA].m128_u32[GPR_PREF_SLOT] > (uint32_t)SignExtend(op.RI10.I10, 10) )
	{
		HaltSPUAux( SPU );
	}
}
