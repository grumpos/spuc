#pragma once
#ifndef __SPU_EMU_H__
#define __SPU_EMU_H__


#include <cstdint>
#include <intrin.h>
//#include <vector>

union SPU_INSTRUCTION
{
	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t RB : 7;						uint32_t OP : 11; } RR;
	struct { uint32_t RC : 7; uint32_t RA : 7;		uint32_t RB : 7; uint32_t RT : 7; 		uint32_t OP : 4; } RRR;
	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t I7 : 7;						uint32_t OP : 11; } RI7;
	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t I8 : 8;						uint32_t OP : 10; } RI8;
	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t I10 : 10;						uint32_t OP : 8; } RI10;
	struct { uint32_t RT : 7; uint32_t I16 : 16; 									  		uint32_t OP : 9; } RI16;
	struct { uint32_t RT : 7; uint32_t I18 : 18; 									  		uint32_t OP : 7; } RI18;
	struct { uint32_t ROL : 7; uint32_t I16 : 16;	uint32_t ROH : 2; 						uint32_t OP : 7; } LBT;
	struct { uint32_t ROL : 7; uint32_t RA : 7;		uint32_t ROH : 2; uint32_t unused : 5;	uint32_t OP : 11; } LBTI;
	uint32_t Instruction;
};

//#define QWORD_REVERSE_COPY 0
//
//#define SPU_RunCntl_STOP_REQUEST			 0x00000000ui32
//#define SPU_RunCntl_RUN_REQUEST				 0x00000001ui32
//#define SPU_RunCntl_ISOLATION_EXIT_REQUEST	 0x00000002ui32
//#define SPU_RunCntl_ISOLATION_LOAD_REQUEST	 0x00000003ui32
//#define SPU_PrivCntl_ISOLATION_LOAD_REQUEST	 0x0000000000000001ui64
//#define SPU_PrivCntl_ATTENTION_EVENT_REQUEST 0x0000000000000002ui64
//#define SPU_PrivCntl_SINGLE_STEP_MODE		 0x0000000000000004ui64
//#define SPU_Status_RUN						 0x00000001ui32

// SPU Channels
/*#define SPU_RdEventStat 0x0
#define SPU_WrEventMask 0x1
#define SPU_WrEventAck 0x2

#define SPU_RdSigNotify1 0x3
#define SPU_RdSigNotify2 0x4

#define SPU_WrDec 0x7
#define SPU_RdDec 0x8

#define MFC_WrMSSyncReq 0x9

#define SPU_RdEventMask 0xB
#define MFC_RdTagMask 0xC

#define SPU_RdMachStat 0xD
#define SPU_WrSRR0 0xE
#define SPU_RdSRR0 0xF

#define MFC_LSA 0x10
#define MFC_EAH 0x11
#define MFC_EAL 0x12
#define MFC_Size 0x13
#define MFC_TagID 0x14*/


typedef __m128 GPR_t;

//struct SPU_t
//{
//	SPU_t();
//	~SPU_t();
//	
//	GPR_t* GPR;
//	uint8_t* LocalStorage;
//
//	uint32_t IP;
//
//	// User
//
//	struct
//	{
//		uint32_t Run : 2;
//		uint32_t Reserved_0_29 : 30;
//	} SPU_RunCntl;
//
//	uint32_t SPU_Status;
//
//	struct
//	{
//		uint32_t InterruptEnable : 1;
//		uint32_t Reserved_30 : 1;
//		uint32_t LocalStorageAddress : 30;
//	} SPU_NPC;
//
//
//	// Privileged
//	uint64_t SPU_PrivCntl;
//	static const uint64_t SPU_LSLR = 0x000000000003ffff;
//};

//#define GPR_PREF_SLOT 3
//
//#define SPU_GPR_MASK 	0x000000000000007F
//#define SPU_I7_MASK 	0x000000000000007F
//#define SPU_I10_MASK 	0x00000000000003ff
//#define SPU_I16_MASK 	0x000000000000ffff
//#define SPU_I18_MASK 	0x000000000003ffff


__inline int64_t SignExtend( int64_t t, size_t Bits )
{
	const size_t Offset = 8 * sizeof( int64_t ) - Bits;
	return (t << Offset) >> Offset;
}

//uint32_t spu_assemble_one_text( SPU_t* targetSPU, const char* instr );
//void spu_parse_file( SPU_t* targetSPU, const char* path );
//void spu_extecute_single( SPU_t* SPU, uint32_t i );
//void spu_execute( SPU_t* SPU );

#endif