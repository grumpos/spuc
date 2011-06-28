#include "spu_emu.h"

void spu_stop( SPU_t* SPU, SPU_INSTRUCTION  )
{
	SPU->SPU_RunCntl.Run = SPU_RunCntl_STOP_REQUEST;
}

void spu_stopd( SPU_t* SPU, SPU_INSTRUCTION  )
{
	SPU->SPU_RunCntl.Run = SPU_RunCntl_STOP_REQUEST;
}

void spu_lnop( SPU_t* , SPU_INSTRUCTION  )
{
}

void spu_nop( SPU_t* , SPU_INSTRUCTION  )
{
}

void spu_sync( SPU_t* , SPU_INSTRUCTION  )
{
}

void spu_dsync( SPU_t* , SPU_INSTRUCTION  )
{
}

// not implemented
void spu_mfspr( SPU_t* , SPU_INSTRUCTION  )
{
}

void spu_mtspr( SPU_t* , SPU_INSTRUCTION  )
{
}