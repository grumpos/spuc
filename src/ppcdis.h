#pragma once

#define PPU_EXTS(i, bits) ((((int64_t)i)<<(64-bits))>>(64-bits))

#define PPU_BRANCH_ADDR_ABS(i) (((((int64_t)(i))<<30)>>31)&PPU_EXTS(((int64_t)(((ppc_i_t&)(i))._I.LI))<<2, 24))

#define PPU_IS_BRANCH_UNC(i) (18==((i)>>24))
#define PPU_IS_BRANCH_CND(i) (16==((i)>>24))
#define PPU_IS_BRANCH_CLR(i) ((19==((i)>>24))&&(16==((((uint32_t)(i))<<21)>>22)))
#define PPU_IS_BRANCH_CCR(i) ((19==((i)>>24))&&(528==((((uint32_t)(i))<<21)>>22)))

#define PPU_IS_BRANCH(i) (PPU_IS_BRANCH_UNC(i)||PPU_IS_BRANCH_CND(i)||PPU_IS_BRANCH_CLR(i)||PPU_IS_BRANCH_CCR(i))
#define PPU_IS_FNCALL(i) ((i)&1)

#define BO_MASK 0x03E00000
#define BO_BRANCH_ALYWAS 0x02800000

const char* ppc_decode_mnem( uint32_t i );