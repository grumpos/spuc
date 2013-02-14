#pragma once
#ifndef __SPU_EMU_H__
#define __SPU_EMU_H__


#include <cstdint>
#include <intrin.h>

#define _BF_FIELD(bits, field1) uint##bits##_t field1;

#ifdef _WIN32
#define _BF_BE1(bits, field1) \
	struct { _BF_FIELD(bits, field1) }
#define _BF_BE2(bits, field1, field2) \
	struct { _BF_FIELD(bits, field2) _BF_FIELD(bits, field2) }
#define _BF_BE3(bits, field1, field2, field3) \
	struct { _BF_FIELD(bits, field3) _BF_FIELD(bits, field2) _BF_FIELD(bits, field1) }
#define _BF_BE4(bits, field1, field2, field3, field4) \
	struct { _BF_FIELD(bits, field4) _BF_FIELD(bits, field3) _BF_FIELD(bits, field2) _BF_FIELD(bits, field1) }
#define _BF_BE5(bits, field1, field2, field3, field4, field5) \
	struct { _BF_FIELD(bits, field5) _BF_FIELD(bits, field4) _BF_FIELD(bits, field3) _BF_FIELD(bits, field2) _BF_FIELD(bits, field1) }
#else
#define _BF_BE1(bits, field1) \
	struct { _BF_FIELD(bits, field1) }
#define _BF_BE2(bits, field1, field2) \
	struct { _BF_FIELD(bits, field1) _BF_FIELD(bits, field2) }
#define _BF_BE3(bits, field1, field2, field3) \
	struct { _BF_FIELD(bits, field1) _BF_FIELD(bits, field2) _BF_FIELD(bits, field3) }
#define _BF_BE4(bits, field1, field2, field3, field4) \
	struct { _BF_FIELD(bits, field1) _BF_FIELD(bits, field2) _BF_FIELD(bits, field3) _BF_FIELD(bits, field4) }
#define _BF_BE5(bits, field1, field2, field3, field4, field5) \
	struct { _BF_FIELD(bits, field1) _BF_FIELD(bits, field2) _BF_FIELD(bits, field3) _BF_FIELD(bits, field4) _BF_FIELD(bits, field5) }
#endif

union SPU_INSTRUCTION
{
	_BF_BE5( 32,  OP : 4,	RT : 7,		RB : 7,  RA : 7,  RC : 7 ) RRR;
	_BF_BE4( 32,  OP : 11,	RB : 7,		RA : 7,			RT : 7 ) RR;
	_BF_BE4( 32,  OP : 11,	I7 : 7,		RA : 7,			RT : 7 ) RI7;
	_BF_BE4( 32,  OP : 10,	I8 : 8,		RA : 7,			RT : 7 ) RI8;
	_BF_BE4( 32,  OP : 8,	I10 : 10,	RA : 7,			RT : 7 ) RI10;
	_BF_BE3( 32,  OP : 9,	I16 : 16,					RT : 7 ) RI16;
	_BF_BE3( 32,  OP : 7,	I18 : 18,					RT : 7 ) RI18;
	_BF_BE4( 32,  OP : 7,	ROH : 2,	I16 : 16, ROL : 7 )	LBT;
	_BF_BE5( 32,  OP : 11,	reserved : 5, ROH : 2, RA : 7, ROL : 7 ) LBTI;
	uint32_t Instruction;
};

static_assert(sizeof(SPU_INSTRUCTION) == 4, "");

//union SPU_INSTRUCTION
//{
//	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t RB : 7;						uint32_t OP : 11; } RR;
//	struct { uint32_t RC : 7; uint32_t RA : 7;		uint32_t RB : 7; uint32_t RT : 7; 		uint32_t OP : 4; } RRR;
//	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t I7 : 7;						uint32_t OP : 11; } RI7;
//	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t I8 : 8;						uint32_t OP : 10; } RI8;
//	struct { uint32_t RT : 7; uint32_t RA : 7;		uint32_t I10 : 10;						uint32_t OP : 8; } RI10;
//	struct { uint32_t RT : 7; uint32_t I16 : 16; 									  		uint32_t OP : 9; } RI16;
//	struct { uint32_t RT : 7; uint32_t I18 : 18; 									  		uint32_t OP : 7; } RI18;
//	struct { uint32_t ROL : 7; uint32_t I16 : 16;	uint32_t ROH : 2; 						uint32_t OP : 7; } LBT;
//	struct { uint32_t ROL : 7; uint32_t RA : 7;		uint32_t ROH : 2; uint32_t reserved : 5;	uint32_t OP : 11; } LBTI;
//	uint32_t Instruction;
//};

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

//#define GPR_PREF_SLOT 3
//
//#define SPU_GPR_MASK 	0x000000000000007F
//#define SPU_I7_MASK 	0x000000000000007F
//#define SPU_I10_MASK 	0x00000000000003ff
//#define SPU_I16_MASK 	0x000000000000ffff
//#define SPU_I18_MASK 	0x000000000003ffff


inline int64_t SignExtend( int64_t t, size_t Bits )
{
	const size_t Offset = 8 * sizeof( int64_t ) - Bits;
	return (t << Offset) >> Offset;
}

#endif