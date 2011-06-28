#include "spu_emu.h"

// PC will be set to next instruction after executing the current one, branches need to be offset by -4
// Interrupts not implemented
//
//#define BRANCH_INTERRUPT_HANDLER \
//	const uint8_t D = 0x20 & op.RR.RB;\
//	const uint8_t E = 0x10 & op.RR.RB;\
//	if ( E && !D )\
//		SPU->InterruptEnabled = true;\
//	else if ( !E && D )\
//		SPU->InterruptEnabled = false;
//
//void spu_bi( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->IP = SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] & SPU_t::SPU_LSLR & 0xFFFFFFFC;
//	SPU->IP = SPU->IP - 4;
//
//	// BRANCH_INTERRUPT_HANDLER
//}
//
//void spu_iret( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	// BRANCH_INTERRUPT_HANDLER
//}
//
//void spu_bisled( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	// TODO: external event?
//
//	// BRANCH_INTERRUPT_HANDLER
//}
//
//void spu_bisl( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->GPR[op.RR.RT] = _mm_setzero_ps();
//	SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] = ( SPU->IP + 4 ) & SPU_t::SPU_LSLR;
//
//	SPU->IP = SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] & SPU_t::SPU_LSLR & 0xFFFFFFFC;
//	SPU->IP = SPU->IP - 4;
//
//	// BRANCH_INTERRUPT_HANDLER
//}
//
//void spu_biz( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	if ( 0 == SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] )
//	{
//		SPU->IP = SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] & SPU_t::SPU_LSLR & 0xFFFFFFFC;
//		SPU->IP = SPU->IP - 4;
//
//		// BRANCH_INTERRUPT_HANDLER
//	}
//}
//
//void spu_binz( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	if ( 0 != SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] )
//	{
//		SPU->IP = SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] & SPU_t::SPU_LSLR & 0xFFFFFFFC;
//		SPU->IP = SPU->IP - 4;
//
//		// BRANCH_INTERRUPT_HANDLER
//	}
//}
//
//void spu_bihz( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	if ( 0 == (SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] & 0xffff) )
//	{
//		SPU->IP = SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] & SPU_t::SPU_LSLR & 0xFFFFFFFC;
//		SPU->IP = SPU->IP - 4;
//		
//		// BRANCH_INTERRUPT_HANDLER
//	}
//}
//
//void spu_bihnz( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	if ( 0 != (SPU->GPR[op.RR.RT].m128_u32[GPR_PREF_SLOT] & 0xffff) )
//	{
//		SPU->IP = SPU->GPR[op.RR.RA].m128_u32[GPR_PREF_SLOT] & SPU_t::SPU_LSLR & 0xFFFFFFFC;
//		SPU->IP = SPU->IP - 4;
//		
//		// BRANCH_INTERRUPT_HANDLER
//	}
//}
//
//
//// static brnaching, compile time
//
//
//// "SPU->IP - 4" Instruction Pointer now points to this instruction
//// SPU->IP is incremented after executing the instruction
//
//void spu_br( SPU_t* SPU, SPU_INSTRUCTION op )
//{	
//	SPU->IP = SPU_t::SPU_LSLR & ( SPU->IP + (int32_t)SignExtend(op.RI16.I16 << 2, 18) );
//	SPU->IP = SPU->IP - 4;
//}
//
//void spu_bra( SPU_t* SPU, SPU_INSTRUCTION op )
//{
//	SPU->IP = SPU_t::SPU_LSLR & (int32_t)SignExtend(op.RI16.I16 << 2, 18);
//	SPU->IP = SPU->IP - 4;
//}
//
//void spu_brsl( SPU_t* SPU, SPU_INSTRUCTION op ) // function call
//{
//	SPU->GPR[op.RI16.RT] = _mm_setzero_ps();
//	SPU->GPR[op.RI16.RT].m128_u32[GPR_PREF_SLOT] = SPU_t::SPU_LSLR & ( SPU->IP + 4 ); // Save next instr
//	spu_br( SPU, op );
//}
//
//void spu_brasl( SPU_t* SPU, SPU_INSTRUCTION op ) // function call
//{
//	SPU->GPR[op.RI16.RT] = _mm_setzero_ps();
//	SPU->GPR[op.RI16.RT].m128_u32[GPR_PREF_SLOT] = SPU_t::SPU_LSLR & ( SPU->IP + 4 );
//	spu_bra( SPU, op );
//}
//
//void spu_brnz( SPU_t* SPU, SPU_INSTRUCTION op ) // if zero {...}
//{
//	if ( 0 != SPU->GPR[op.RI16.RT].m128_u32[GPR_PREF_SLOT] )
//		spu_br( SPU, op );
//}
//
//void spu_brz( SPU_t* SPU, SPU_INSTRUCTION op ) // if non-zero {...}
//{
//	if ( 0 == SPU->GPR[op.RI16.RT].m128_u32[GPR_PREF_SLOT] )
//		spu_br( SPU, op );
//}
//
//void spu_brhnz( SPU_t* SPU, SPU_INSTRUCTION op ) // if zero {...}
//{
//	if ( 0 != ( 0xffff & SPU->GPR[op.RI16.RT].m128_u32[GPR_PREF_SLOT] ) )
//		spu_br( SPU, op );
//}
//
//void spu_brhz( SPU_t* SPU, SPU_INSTRUCTION op ) // if non-zero {...}
//{
//	if ( 0 == ( 0xffff & SPU->GPR[op.RI16.RT].m128_u32[GPR_PREF_SLOT] ) )
//		spu_br( SPU, op );
//}
//
