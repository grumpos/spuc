#ifndef __MFC_H__
#define __MFC_H__

#include <cstdint>

enum SPEChannels
{
	$SPU_RdEventStat = 0,
	$SPU_WrEventMask,
	$SPU_WrEventAck,
	$SPU_RdSigNotify1,
	$SPU_RdSigNotify2,
	$5,
	$6,
	$SPU_WrDec,
	$SPU_RdDec,
	$MFC_WrMSSyncReq,
	$11,
	$SPU_RdEventMask,
	$MFC_RdTagMask,
	$SPU_RdMachStat,
	$SPU_WrSRR0,
	$SPU_RdSRR0,
	$MFC_LSA,
	$MFC_EAH,
	$MFC_EAL,
	$MFC_Size,
	$MFC_TagID,
	$MFC_Cmd,
	$MFC_WrTagMask,
	$MFC_WrTagUpdate,
	$MFC_RdTagStat,
	$MFC_RdListStallStat,
	$MFC_WrListStallAck,
	$MFC_RdAtomicStat, 
	$SPU_WrOutMbox, 
	$SPU_RdInMbox, 
	$SPU_WrOutIntrMbox
};

uint32_t	MFC_ReadChannel( uint8_t SPEIndex, uint8_t Channel );
uint8_t		MFC_ReadChannelCount( uint8_t SPEIndex, uint8_t Channel );
void		MFC_WriteChannel( uint8_t SPEIndex, uint8_t Channel, uint32_t Data );

#endif