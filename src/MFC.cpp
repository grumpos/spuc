#include "MFC.h"

static uint32_t ChannelData[128][8];
static uint8_t ChannelCount[128][8] =
{
	0,1,1,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0
};

uint32_t	MFC_ReadChannel( uint8_t SPEIndex, uint8_t Channel )
{
	switch ( Channel )
	{
	case $SPU_RdEventStat:
		// blocking
	case $SPU_RdSigNotify1:
		// blocking
	case $SPU_RdSigNotify2:
		// blocking
	case $SPU_RdDec:
	case $SPU_RdEventMask:
	case $MFC_RdTagMask:
	case $SPU_RdMachStat:
	case $SPU_RdSRR0:
		return ChannelData[Channel][SPEIndex];

	case $MFC_RdTagStat:
		// blocking
	case $MFC_RdListStallStat:
		// blocking
	case $MFC_RdAtomicStat: 
		// blocking
	case $SPU_RdInMbox: 
		// blocking
		return ChannelData[Channel][SPEIndex];
	default:
		return ChannelData[Channel][SPEIndex];
	};	
}

uint8_t	MFC_ReadChannelCount( uint8_t SPEIndex, uint8_t Channel )
{
	return ChannelCount[Channel][SPEIndex];
}

void MFC_WriteChannel( uint8_t SPEIndex, uint8_t Channel, uint32_t Data )
{
	switch ( Channel )
	{
	case $SPU_WrEventMask:
	case $SPU_WrEventAck:
	case $SPU_WrDec:
		ChannelData[Channel][SPEIndex] = Data; break;
	case $MFC_WrMSSyncReq:
		break;
	case $SPU_WrSRR0:
	case $MFC_LSA:
	case $MFC_EAH:
	case $MFC_EAL:
		ChannelData[Channel][SPEIndex] = Data; break;
	case $MFC_Size:
	case $MFC_TagID:
		ChannelData[Channel][SPEIndex] = Data; break;
	case $MFC_Cmd:
	case $MFC_WrTagMask:
		ChannelData[Channel][SPEIndex] = Data; break;
	case $MFC_WrTagUpdate:
		break;
	case $MFC_WrListStallAck:
		ChannelData[Channel][SPEIndex] = Data; break;
	case $SPU_WrOutMbox:
		break;
	case $SPU_WrOutIntrMbox:
		break;
	default:
		break;
	}
}