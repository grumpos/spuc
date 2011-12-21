#include <cstdint>

struct BE_MemoryMaps
{
	uint64_t StartAddress;
	uint64_t Length;
	char	 ImageName[64];
};

static const BE_MemoryMaps MMList[] =
{
	{ 0x20000000000, 0x800000, "" } // Memory mapped registers
};

//unsigned char XDR[256 * 1024 * 1024];

// Based on 65nm configuration ring values
#define BE_MMIO_Base		0x20000000000
#define SPE0_BE_MMIO_Base	BE_MMIO_Base
#define SPE1_BE_MMIO_Base	BE_MMIO_Base
#define SPE2_BE_MMIO_Base	BE_MMIO_Base
#define SPE3_BE_MMIO_Base	BE_MMIO_Base
#define SPE4_BE_MMIO_Base	BE_MMIO_Base
#define SPE5_BE_MMIO_Base	BE_MMIO_Base
#define SPE6_BE_MMIO_Base	BE_MMIO_Base
#define SPE7_BE_MMIO_Base	BE_MMIO_Base
#define SPE0_7_BE_MMIO_Base BE_MMIO_Base
#define PPE_BE_MMIO_Base	BE_MMIO_Base
#define MIC_BE_MMIO_Base	BE_MMIO_Base
#define PRV_BE_MMIO_Base	BE_MMIO_Base
#define BEI_BE_MMIO_Base	BE_MMIO_Base

#define SPE0_LS (SPE0_BE_MMIO_Base + 0x0)
#define SPE0_PS (SPE0_BE_MMIO_Base + 0x40000)
#define SPE0_P2 (SPE0_BE_MMIO_Base + 0x60000)
#define SPE1_LS (SPE1_BE_MMIO_Base + 0x80000)
#define SPE1_PS (SPE1_BE_MMIO_Base + 0xC0000)
#define SPE1_P2 (SPE1_BE_MMIO_Base + 0xE0000)
#define SPE2_LS (SPE2_BE_MMIO_Base + 0x100000)
#define SPE2_PS (SPE2_BE_MMIO_Base + 0x140000)
#define SPE2_P2 (SPE2_BE_MMIO_Base + 0x160000)
#define SPE3_LS (SPE3_BE_MMIO_Base + 0x180000)
#define SPE3_PS (SPE3_BE_MMIO_Base + 0x1C0000)
#define SPE3_P2 (SPE3_BE_MMIO_Base + 0x1E0000)
#define SPE4_LS (SPE4_BE_MMIO_Base + 0x200000)
#define SPE4_PS (SPE4_BE_MMIO_Base + 0x240000)
#define SPE4_P2 (SPE4_BE_MMIO_Base + 0x260000)
#define SPE5_LS (SPE5_BE_MMIO_Base + 0x280000)
#define SPE5_PS (SPE5_BE_MMIO_Base + 0x2C0000)
#define SPE5_P2 (SPE5_BE_MMIO_Base + 0x2E0000)
#define SPE6_LS (SPE6_BE_MMIO_Base + 0x300000)
#define SPE6_PS (SPE6_BE_MMIO_Base + 0x340000)
#define SPE6_P2 (SPE6_BE_MMIO_Base + 0x360000)
#define SPE7_LS (SPE7_BE_MMIO_Base + 0x380000)
#define SPE7_PS (SPE7_BE_MMIO_Base + 0x3C0000)
#define SPE7_P2 (SPE7_BE_MMIO_Base + 0x3E0000)
#define SPE0_P1 (SPE0_7_BE_MMIO_Base + 0x400000)
#define SPE1_P1 (SPE0_7_BE_MMIO_Base + 0x402000)
#define SPE2_P1 (SPE0_7_BE_MMIO_Base + 0x404000)
#define SPE3_P1 (SPE0_7_BE_MMIO_Base + 0x406000)
#define SPE4_P1 (SPE0_7_BE_MMIO_Base + 0x408000)
#define SPE5_P1 (SPE0_7_BE_MMIO_Base + 0x40A000)
#define SPE6_P1 (SPE0_7_BE_MMIO_Base + 0x40C000)
#define SPE7_P1 (SPE0_7_BE_MMIO_Base + 0x40E000)
// 0x411000 - 0x41FFFF Reserved
#define PPE_Priv	(PPE_BE_MMIO_Base + 0x500000)
// 0x501000 - 0x507FFF Reserved
#define IIC			(BEI_BE_MMIO_Base + 0x508000)
// 0x509000 - 0x5093FF Reserved
#define PRV_PM		(PRV_BE_MMIO_Base + 0x509400)
#define PRV_TPM		(PRV_BE_MMIO_Base + 0x509800)
#define PRV_RAS		(PRV_BE_MMIO_Base + 0x509C00)
#define MIC_TKM		(MIC_BE_MMIO_Base + 0x50A000)
// 0x50B000 - 0x50FFFF Reserved
#define IOC_IOAT	(BEI_BE_MMIO_Base + 0x510000)
#define BIC0_NClk	(BEI_BE_MMIO_Base + 0x511000)
#define BIC1_NClk	(BEI_BE_MMIO_Base + 0x511400)
#define EIB			(BEI_BE_MMIO_Base + 0x511800)
#define IOC_IOC		(BEI_BE_MMIO_Base + 0x511C00)
#define BIC0_BClk	(BEI_BE_MMIO_Base + 0x512000)
#define BIC1_BClk	(BEI_BE_MMIO_Base + 0x513000)
// 0x514000 - 0x7FFFFF Reserved

struct SPE_P1_MM
{
	uint64_t MFC_SR1;					// MFC
	uint64_t MFC_LPID;
	uint64_t SPU_ID;
	uint64_t MFC_VR;
	uint64_t SPU_VR;
	//uint8_t Reserved0[0x0100-0x0028]; 
	uint64_t INT_Mask_class0;			// Interrupt
	uint64_t INT_Mask_class1;
	uint64_t INT_Mask_class2;
	//uint8_t Reserved1[0x0140-0x0118];
	uint64_t INT_Stat_class0;
	uint64_t INT_Stat_class1;
	uint64_t INT_Stat_class2;
	//uint8_t Reserved2[0x0180-0x0158];
	uint64_t INT_Route;
	//uint8_t Reserved3[0x0200-0x0198];
	uint64_t MFC_Atomic_Flush;			// Atomic Unit Control
	uint64_t RA_Group_ID;
	uint64_t RA_Enable;
	//uint8_t Reserved4[0x0380-0x0290];
	uint64_t MFC_FIR;					// Fault Isolation
	uint64_t MFC_FIR_Set;
	uint64_t MFC_FIR_Reset;
	uint64_t MFC_FIR_Err;
	uint64_t MFC_FIR_Err_Set;
	uint64_t MFC_FIR_Err_Reset;
	uint64_t MFC_FIR_ChkStpEnbl;	
	uint64_t MFC_SBI_Derr_Addr;			// Misc
	uint64_t MFC_CMDQ_Err_ID;
	//uint8_t Reserved5[0x0400-0x03C8];
	uint64_t MFC_SDR;					// MFC TLB Management
	//uint8_t Reserved6[0x0500-0x0408];
	uint64_t MFC_TLB_Index_Hint;
	uint64_t MFC_TLB_Index;
	uint64_t MFC_TLB_VPN;
	uint64_t MFC_TLB_RPN;
	//uint8_t Reserved7[0x0540-0x0520];
	uint64_t MFC_TLB_Invalidate_Entry;
	uint64_t MFC_TLB_Invalidate_All;
	uint64_t SMM_HID;
	uint64_t MFC_ACCR;
	uint64_t MFC_DSISR;
	uint64_t MFC_DAR;
	uint64_t MFC_TLB_RMT_Index;
	uint64_t MFC_TLB_RMT_Data;
	uint64_t MFC_DSIPR;
	uint64_t MFC_LSACR;
	uint64_t MFC_LSCRR;
	uint64_t MFC_TClassID;
	uint64_t DMAC_PMCR;
	uint64_t MFC_RMAB;
	uint64_t MFC_CER;
	uint64_t SPU_ECC_Cntl;
	uint64_t SPU_ECC_Stat;
	uint64_t SPU_ECC_Addr;
	uint64_t SPU_ERR_Mask;
	uint64_t PM_Trace_Tag_Wait_Mask;
};

struct SPE_P2_MM
{
	uint64_t SLB_Index;
	uint64_t SLB_ESID;
	uint64_t SLB_VSID;
	uint64_t SLB_Invalidate_Entry;
	uint64_t SLB_Invalidate_All;
	uint64_t MFC_CQ_SR;
	uint64_t MFC_CNTL;
	uint64_t SPU_OutIntrMbox;
	uint64_t SPU_PrivCntl;
	uint64_t SPU_LSLR;
	uint64_t SPU_ChnlIndex;
	uint64_t SPU_ChnlCnt;
	uint64_t SPU_ChnlData;
	uint64_t SPU_Cfg;
	uint64_t MFC_CSR_TSQ;
	uint64_t MFC_CSR_CMD1;
	uint64_t MFC_CSR_CMD2;
	uint64_t MFC_CSR_ATO;
};

struct SPE_PS_MM
{
	uint64_t MFC_MSSync;
	uint64_t MFC_LSA;
	uint64_t MFC_EAH;
	uint64_t MFC_EAL;
	uint64_t MFC_Size;
	uint64_t MFC_Tag;
	uint64_t MFC_ClassID_CMD;
	uint64_t MFC_CMDStatus;
	uint64_t MFC_QStatus;
	uint64_t Prxy_QueryType;
	uint64_t Prxy_QueryMask;
	uint64_t Prxy_TagStatus;
	uint64_t SPU_Out_Mbox;
	uint64_t SPU_In_Mbox;
	uint64_t SPU_Mbox_Stat;
	uint64_t SPU_RunCntl;
	uint64_t SPU_Status;
	uint64_t SPU_NPC;
	uint64_t SPU_Sig_Notify_1;
	uint64_t SPU_Sig_Notify_2;
};

struct EIB_MM
{
	uint64_t EIB_AC0_CTL;
	uint64_t EIB_Int;
	uint64_t EIB_LBAR0;
	uint64_t EIB_LBAMR0;
	uint64_t EIB_LBAR1;
	uint64_t EIB_LBAMR1;
	uint64_t EIB_Cfg;
};

struct PRV_MM
{
	uint64_t checkstop_fir;
	uint64_t recoverable_fir;
	uint64_t spec_att_mchk_fir;
	uint64_t fir_mode_reg;
	uint64_t fir_enable_mask;
	uint64_t SPE_available;
	uint64_t serial_number;
	uint64_t group_control;
	uint64_t debug_bus_control;
	uint64_t trace_buffer_high;
	uint64_t trace_buffer_low;
	uint64_t trace_address;
	uint64_t ext_tr_timer;
	uint64_t pm_status;
	uint64_t pm_control;
	uint64_t pm_interval;
	uint64_t pmM_0;
	uint64_t pmM_1;
	uint64_t pmM_2;
	uint64_t pmM_3;
	uint64_t pm_start_stop;
	uint64_t pm0_control;
	uint64_t pm1_control;
	uint64_t pm2_control;
	uint64_t pm3_control;
	uint64_t pm4_control;
	uint64_t pm5_control;
	uint64_t pm6_control;
	uint64_t pm7_control;
	uint64_t PMCR;
	uint64_t PMSR;
	uint64_t TS_CTSR1;
	uint64_t TS_CTSR2;
	uint64_t TS_MTSR1;
	uint64_t TS_MTSR2;
	uint64_t TS_ITR1;
	uint64_t TS_ITR2;
	uint64_t TS_GITR;
	uint64_t TS_ISR;
	uint64_t TS_IMR;
	uint64_t TM_CR1;
	uint64_t TM_CR2;
	uint64_t TM_SIMR;
	uint64_t TM_TPR;
	uint64_t TM_STR1;
	uint64_t TM_STR2;
	uint64_t TM_TSR;
	uint64_t TBR;
};