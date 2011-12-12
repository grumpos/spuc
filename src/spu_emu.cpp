//#include "spu_emu.h"

/*int64_t SignExtend( int64_t t, size_t Bits )
{
	const size_t Offset = 8 * sizeof( int64_t ) - Bits;
	return (t << Offset) >> Offset;
}*/

//SPU_t::SPU_t()
//{
//	memset( this, 0, sizeof(SPU_t) );
//
//	GPR = new GPR_t[128];
//	memset( GPR, 0, 128*sizeof(GPR_t) );
//
//	LocalStorage = new uint8_t[SPU_t::SPU_LSLR + 1];
//	memset( LocalStorage, 0, SPU_t::SPU_LSLR + 1 );	
//}
//
//SPU_t::~SPU_t()
//{
//	delete[] LocalStorage;
//	delete[] GPR;
//}