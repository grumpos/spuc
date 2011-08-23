#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <string>
#include <fstream>
#include <numeric>
#include <iomanip>
//#include <regex>
#include "spu_idb.h"
#include "spu_pseudo.h"


void replace_all( std::string& where, const std::string& what, const std::string& with_what )
{		
	for ( size_t offset = where.find( what ); offset != std::string::npos; )
	{
		where.replace( offset, what.length(), with_what );
		offset += with_what.length();
		offset = where.find( what, offset );
	}
}

template <class T>
std::string lexical_cast( T t )
{
	std::ostringstream os;
	os << t;
	return os.str();
}

template <class T>
std::string lexical_cast_gpr( T t )
{
	std::ostringstream os;
	os << std::setw(3) << std::setfill(' ');
	os << t;
	return os.str();
}

template <class T>
std::string lexical_cast_hex( T t )
{
	std::ostringstream os;
	//os << "0x" << std::hex << std::setw(8) << std::setfill('0') << t << std::dec << "/*" << t << "*/";
	os << t << "/*0x" << std::hex << t << "*/";
	return os.str();
}

template <class T>
std::string lexical_cast_hex2( T t )
{
	std::ostringstream os;
	os << "// 0x" << std::hex << t;
	return os.str();
}

template <class T>
std::string lexical_cast_hex_addr( T t )
{
	std::ostringstream os;
	//os << "0x" << std::hex << std::setw(8) << std::setfill('0') << t << std::dec << "/*" << t << "*/";
	os << std::hex << t;// << std::dec << "/*" << t << "*/";
	return os.str();
}

typedef std::pair<std::string, std::string> mn_intr_pair_t;

static const mn_intr_pair_t IntrinsicTemplateTable[] = 
{	
	/*mn_intr_pair_t("cbd", "$RT$ = si_cbd($RA$,$IMM$)"),
	mn_intr_pair_t("cbx", "$RT$ = si_cbx($RA$,$RB$)"),
	mn_intr_pair_t("chd", "$RT$ = si_chd($RA$,$IMM$)"),
	mn_intr_pair_t("chx", "$RT$ = si_chx($RA$,$RB$)"),
	mn_intr_pair_t("cwd", "$RT$ = si_cwd($RA$,$IMM$)"),
	mn_intr_pair_t("cwx", "$RT$ = si_cwx($RA$,$RB$)"),
	mn_intr_pair_t("cdd", "$RT$ = si_cdd($RA$,$IMM$)"),
	mn_intr_pair_t("cdx", "$RT$ = si_cdx($RA$,$RB$)"),*/
	mn_intr_pair_t("ilh", "$RT$ = si_ilh($IMM$)"),
	mn_intr_pair_t("ilhu", "$RT$ = si_ilhu($IMM$)"),
	mn_intr_pair_t("il", "$RT$ = si_il($IMM$)"),
	mn_intr_pair_t("ila", "$RT$ = si_ila($IMM18$)"),
	mn_intr_pair_t("iohl", "$RT$ = si_iohl($RA$,$IMM$)"),
	mn_intr_pair_t("fsmbi", "$RT$ = si_fsmbi($IMM$)"),
	/*mn_intr_pair_t("ah", "$RT$ = si_ah($RA$,$RB$)"),
	mn_intr_pair_t("ahi", "$RT$ = si_ahi($RA$,$IMM$)"),
	mn_intr_pair_t("a", "$RT$ = si_a($RA$,$RB$)"),
	mn_intr_pair_t("ai", "$RT$ = si_ai($RA$,$IMM$)"),*/
	mn_intr_pair_t("addx", "$RT$ = si_addx($RA$,$RB$,$RT$)"),
	/*mn_intr_pair_t("cg", "$RT$ = si_cg($RA$,$RB$)"),*/
	mn_intr_pair_t("cgx", "$RT$ = si_cgx($RA$,$RB$,$RT$)"),
	/*mn_intr_pair_t("sfh", "$RT$ = si_sfh($RA$,$RB$)"),
	mn_intr_pair_t("sfhi", "$RT$ = si_sfhi($IMM$,$RA$)"),
	mn_intr_pair_t("sf", "$RT$ = si_sf($RA$,$RB$)"),
	mn_intr_pair_t("sfi", "$RT$ = si_sfi($RA$,$IMM$)"),*/
	mn_intr_pair_t("sfx", "$RT$ = si_sfx($RA$,$RB$,$RT$)"),
	/*mn_intr_pair_t("bg", "$RT$ = si_bg($RA$,$RB$)"),*/
	mn_intr_pair_t("bgx", "$RT$ = si_bgx($RA$,$RB$,$RT$)"),
	/*mn_intr_pair_t("mpy", "$RT$ = si_mpy($RA$,$RB$)"),
	mn_intr_pair_t("mpyu", "$RT$ = si_mpyu($RA$,$RB$)"),
	mn_intr_pair_t("mpyi", "$RT$ = si_mpyi($RA$,$IMM$)"),
	mn_intr_pair_t("mpyui", "$RT$ = si_mpyui($RA$,$IMM$)"),
	mn_intr_pair_t("mpya", "$RT$ = si_mpya($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("mpyh", "$RT$ = si_mpyh($RA$,$RB$)"),
	mn_intr_pair_t("mpys", "$RT$ = si_mpys($RA$,$RB$)"),
	mn_intr_pair_t("mpyhh", "$RT$ = si_mpyhh($RA$,$RB$)"),
	mn_intr_pair_t("mpyhhu", "$RT$ = si_mpyhhu($RA$,$RB$)"),
	mn_intr_pair_t("mpyhha", "$RT$ = si_mpyhha($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("mpyhhau", "$RT$ = si_mpyhhau($RA$,$RB$,$RC$)"),*/
	mn_intr_pair_t("clz", "$RT$ = si_clz($RA$)"),
	mn_intr_pair_t("cntb", "$RT$ = si_cntb($RA$)"),
	mn_intr_pair_t("fsmb", "$RT$ = si_fsmb($RA$)"),
	mn_intr_pair_t("fsmh", "$RT$ = si_fsmh($RA$)"),
	mn_intr_pair_t("fsm", "$RT$ = si_fsm($RA$)"),
	mn_intr_pair_t("gbb", "$RT$ = si_gbb($RA$)"),
	mn_intr_pair_t("gbh", "$RT$ = si_gbh($RA$)"),
	mn_intr_pair_t("gb", "$RT$ = si_gb($RA$)"),
	/*mn_intr_pair_t("avgb", "$RT$ = si_avgb($RA$,$RB$)"),
	mn_intr_pair_t("absdb", "$RT$ = si_absdb($RA$,$RB$)"),
	mn_intr_pair_t("sumb", "$RT$ = si_sumb($RA$,$RB$)"),*/
	mn_intr_pair_t("xsbh", "$RT$ = si_xsbh($RA$)"),
	mn_intr_pair_t("xshw", "$RT$ = si_xshw($RA$)"),
	mn_intr_pair_t("xswd", "$RT$ = si_xswd($RA$)"),
	/*mn_intr_pair_t("and", "$RT$ = si_and($RA$,$RB$)"),
	mn_intr_pair_t("andc", "$RT$ = si_andc($RA$,$RB$)"),
	mn_intr_pair_t("andbi", "$RT$ = si_andbi($RA$,$IMM$)"),
	mn_intr_pair_t("andhi", "$RT$ = si_andhi($RA$,$IMM$)"),
	mn_intr_pair_t("andi", "$RT$ = si_andi($RA$,$IMM$)"),
	mn_intr_pair_t("or", "$RT$ = si_or($RA$,$RB$)"),
	mn_intr_pair_t("orc", "$RT$ = si_o$RC$($RA$,$RB$)"),
	mn_intr_pair_t("orbi", "$RT$ = si_o$RB$i($RA$,$IMM$)"),
	mn_intr_pair_t("orhi", "$RT$ = si_orhi($RA$,$IMM$)"),
	mn_intr_pair_t("ori", "$RT$ = si_ori($RA$,$IMM$)"),*/
	mn_intr_pair_t("orx", "$RT$ = si_orx($RA$)"),
	/*mn_intr_pair_t("xor", "$RT$ = si_xor($RA$,$RB$)"),
	mn_intr_pair_t("xorbi", "$RT$ = si_xo$RB$i($RA$,$IMM$)"),
	mn_intr_pair_t("xorhi", "$RT$ = si_xorhi($RA$,$IMM$)"),
	mn_intr_pair_t("xori", "$RT$ = si_xori($RA$,$IMM$)"),
	mn_intr_pair_t("nand", "$RT$ = si_nand($RA$,$RB$)"),
	mn_intr_pair_t("nor", "$RT$ = si_nor($RA$,$RB$)"),
	mn_intr_pair_t("eqv", "$RT$ = si_eqv($RA$,$RB$)"),
	mn_intr_pair_t("selb", "$RT$ = si_selb($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("shufb", "$RT$ = si_shufb($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("shlh", "$RT$ = si_shlh($RA$,$RB$)"),
	mn_intr_pair_t("shlhi", "$RT$ = si_shlhi($RA$,$IMM$)"),
	mn_intr_pair_t("shl", "$RT$ = si_shl($RA$,$RB$)"),
	mn_intr_pair_t("shli", "$RT$ = si_shli($RA$,$IMM$)"),
	mn_intr_pair_t("shlqbi", "$RT$ = si_shlqbi($RA$,$RB$)"),
	mn_intr_pair_t("shlqbii", "$RT$ = si_shlqbii($RA$,$IMM$)"),
	mn_intr_pair_t("shlqby", "$RT$ = si_shlqby($RA$,$RB$)"),
	mn_intr_pair_t("shlqbyi", "$RT$ = si_shlqbyi($RA$,$IMM$)"),
	mn_intr_pair_t("shlqbybi", "$RT$ = si_shlqbybi($RA$,$RB$)"),
	mn_intr_pair_t("roth", "$RT$ = si_roth($RA$,$RB$)"),
	mn_intr_pair_t("rothi", "$RT$ = si_rothi($RA$,$IMM$)"),
	mn_intr_pair_t("rot", "$RT$ = si_rot($RA$,$RB$)"),
	mn_intr_pair_t("roti", "$RT$ = si_roti($RA$,$IMM$)"),
	mn_intr_pair_t("rotqby", "$RT$ = si_rotqby($RA$,$RB$)"),
	mn_intr_pair_t("rotqbyi", "$RT$ = si_rotqbyi($RA$,$IMM$)"),
	mn_intr_pair_t("rotqbybi", "$RT$ = si_rotqbybi($RA$,$RB$)"),
	mn_intr_pair_t("rotqbi", "$RT$ = si_rotqbi($RA$,$RB$)"),
	mn_intr_pair_t("rotqbii", "$RT$ = si_rotqbii($RA$,$IMM$)"),
	mn_intr_pair_t("rothm", "$RT$ = si_rothm($RA$,$RB$)"),
	mn_intr_pair_t("rothmi", "$RT$ = si_rothmi($RA$,$IMM$)"),
	mn_intr_pair_t("rotm", "$RT$ = si_rotm($RA$,$RB$)"),
	mn_intr_pair_t("rotmi", "$RT$ = si_rotmi($RA$,$IMM$)"),
	mn_intr_pair_t("rotqmby", "$RT$ = si_rotqmby($RA$,$RB$)"),
	mn_intr_pair_t("rotqmbyi", "$RT$ = si_rotqmbyi($RA$,$IMM$)"),
	mn_intr_pair_t("rotqmbi", "$RT$ = si_rotqmbi($RA$,$RB$)"),
	mn_intr_pair_t("rotqmbii", "$RT$ = si_rotqmbii($RA$,$IMM$)"),
	mn_intr_pair_t("rotqmbybi", "$RT$ = si_rotqmbybi($RA$,$RB$)"),
	mn_intr_pair_t("rotmah", "$RT$ = si_rotmah($RA$,$RB$)"),
	mn_intr_pair_t("rotmahi", "$RT$ = si_rotmahi($RA$,$IMM$)"),
	mn_intr_pair_t("rotma", "$RT$ = si_rotma($RA$,$RB$)"),
	mn_intr_pair_t("rotmai", "$RT$ = si_rotmai($RA$,$IMM$)"),*/
	mn_intr_pair_t("heq", "si_heq($RA$,$RB$)"),
	mn_intr_pair_t("heqi", "si_heqi($RA$,$IMM$)"),
	mn_intr_pair_t("hgt", "si_hgt($RA$,$RB$)"),
	mn_intr_pair_t("hgti", "si_hgti($RA$,$IMM$)"),
	mn_intr_pair_t("hlgt", "si_hlgt($RA$,$RB$)"),
	mn_intr_pair_t("hlgti", "si_hlgti($RA$,$IMM$)"),
	/*mn_intr_pair_t("ceqb", "$RT$ = si_ceqb($RA$,$RB$)"),
	mn_intr_pair_t("ceqbi", "$RT$ = si_ceqbi($RA$,$IMM$)"),
	mn_intr_pair_t("ceqh", "$RT$ = si_ceqh($RA$,$RB$)"),
	mn_intr_pair_t("ceqhi", "$RT$ = si_ceqhi($RA$,$IMM$)"),
	mn_intr_pair_t("ceq", "$RT$ = si_ceq($RA$,$RB$)"),
	mn_intr_pair_t("ceqi", "$RT$ = si_ceqi($RA$,$IMM$)"),
	mn_intr_pair_t("cgtb", "$RT$ = si_cgtb($RA$,$RB$)"),
	mn_intr_pair_t("cgtbi", "$RT$ = si_cgtbi($RA$,$IMM$)"),
	mn_intr_pair_t("cgth", "$RT$ = si_cgth($RA$,$RB$)"),
	mn_intr_pair_t("cgthi", "$RT$ = si_cgthi($RA$,$IMM$)"),
	mn_intr_pair_t("cgt", "$RT$ = si_cgt($RA$,$RB$)"),
	mn_intr_pair_t("cgti", "$RT$ = si_cgti($RA$,$IMM$)"),
	mn_intr_pair_t("clgtb", "$RT$ = si_clgtb($RA$,$RB$)"),
	mn_intr_pair_t("clgtbi", "$RT$ = si_clgtbi($RA$,$IMM$)"),
	mn_intr_pair_t("clgth", "$RT$ = si_clgth($RA$,$RB$)"),
	mn_intr_pair_t("clgthi", "$RT$ = si_clgthi($RA$,$IMM$)"),
	mn_intr_pair_t("clgt", "$RT$ = si_clgt($RA$,$RB$)"),
	mn_intr_pair_t("clgti", "$RT$ = si_clgti($RA$,$IMM$)"),
	mn_intr_pair_t("bisled", "si_bisled($RA$)"),
	mn_intr_pair_t("bisledd", "si_bisledd($RA$)"),
	mn_intr_pair_t("bislede", "si_bislede($RA$)"),
	mn_intr_pair_t("fa", "$RT$ = si_fa($RA$,$RB$)"),
	mn_intr_pair_t("dfa", "$RT$ = si_dfa($RA$,$RB$)"),
	mn_intr_pair_t("fs", "$RT$ = si_fs($RA$,$RB$)"),
	mn_intr_pair_t("dfs", "$RT$ = si_dfs($RA$,$RB$)"),
	mn_intr_pair_t("fm", "$RT$ = si_fm($RA$,$RB$)"),
	mn_intr_pair_t("dfm", "$RT$ = si_dfm($RA$,$RB$)"),
	mn_intr_pair_t("fma", "$RT$ = si_fma($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("dfma", "$RT$ = si_dfma($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("dfnma", "$RT$ = si_dfnma($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("fnms", "$RT$ = si_fnms($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("dfnms", "$RT$ = si_dfnms($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("fms", "$RT$ = si_fms($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("dfms", "$RT$ = si_dfms($RA$,$RB$,$RC$)"),
	mn_intr_pair_t("frest", "$RT$ = si_frest($RA$)"),
	mn_intr_pair_t("frsqest", "$RT$ = si_frsqest($RA$)"),
	mn_intr_pair_t("fi", "$RT$ = si_fi($RA$,$RB$)"),

	mn_intr_pair_t("dfceq", "$RT$ = si_dfceq($RA$,$RB$)"),
	mn_intr_pair_t("dfcmeq", "$RT$ = si_dfcmeq($RA$,$RB$)"),
	mn_intr_pair_t("dfcgt", "$RT$ = si_dfcgt($RA$,$RB$)"),
	mn_intr_pair_t("dfcmgt", "$RT$ = si_dfcmgt($RA$,$RB$)"),
	mn_intr_pair_t("dftsv", "$RT$ = si_dftsv($RA$,$RB$)"),*/

	mn_intr_pair_t("csflt", "$RT$ = si_csflt($RA$,$IMM$)"),
	mn_intr_pair_t("cflts", "$RT$ = si_cflts($RA$,$IMM$)"),
	mn_intr_pair_t("cuflt", "$RT$ = si_cuflt($RA$,$IMM$)"),
	mn_intr_pair_t("cfltu", "$RT$ = si_cfltu($RA$,$IMM$)"),
	mn_intr_pair_t("frds", "$RT$ = si_frds($RA$)"),
	mn_intr_pair_t("fesd", "$RT$ = si_fesd($RA$)"),
	mn_intr_pair_t("fceq", "$RT$ = si_fceq($RA$,$RB$)"),
	mn_intr_pair_t("fcmeq", "$RT$ = si_fcmeq($RA$,$RB$)"),
	mn_intr_pair_t("fcgt", "$RT$ = si_fcgt($RA$,$RB$)"),
	mn_intr_pair_t("fcmgt", "$RT$ = si_fcmgt($RA$,$RB$)"),

	mn_intr_pair_t("stop", "si_stop($IMM$)"),
	mn_intr_pair_t("stopd", "si_stopd($RA$,$RB$,$RT$)"),
	mn_intr_pair_t("sync", "si_sync()"),
	mn_intr_pair_t("syncc", "si_syncc()"),
	mn_intr_pair_t("dsync", "si_dsync()"),
	mn_intr_pair_t("mfspr", "si_mfspr($IMM$)"),
	mn_intr_pair_t("mtspr", "si_mtspr($IMM$,$RA$)"),
	mn_intr_pair_t("fscrrd", "si_fscrrd()"),
	mn_intr_pair_t("fscrwr", "si_fscrwr($RA$)"),	

	// Channel
	mn_intr_pair_t("rdch", "$RT$ = si_rdch($CA$)"),
	mn_intr_pair_t("rchcnt", "$RT$ = si_rchcnt($CA$)"),
	mn_intr_pair_t("wrch", "si_wrch($CA$,$RA$)"),

	// Hint for branch
	mn_intr_pair_t("hbr", "// HINT: hbr $$RA$, PC + $BRTARG$"),
	mn_intr_pair_t("hbra", "// HINT: hbra $BRINST$, $BRTARG$"),
	mn_intr_pair_t("hbrr", "// HINT: hbrr PC + $BRINST$, PC + $BRTARG$"),

	// NOP
	mn_intr_pair_t("lnop", "// LNOP"),
	mn_intr_pair_t("nop", "// NOP"),

	// Memory
	mn_intr_pair_t("lqd", "$RT$ = si_lqd($RA$,$IMM$)"),
	mn_intr_pair_t("lqx", "$RT$ = si_lqx($RA$,$RB$)"),
	mn_intr_pair_t("lqa", "$RT$ = si_lqa($WADDR_ABS$)"),
	mn_intr_pair_t("lqr", "$RT$ = si_lqr($WADDR_REL$)"),
	mn_intr_pair_t("stqd", "si_stqd($RT$,$RA$,$IMM$)"),
	mn_intr_pair_t("stqx", "si_stqx($RT$,$RA$,$RB$)"),
	mn_intr_pair_t("stqa", "si_stqa($RT$,$WADDR_ABS$)"),
	mn_intr_pair_t("stqr", "si_stqr($RT$,$WADDR_REL$)"),

	// Branch
	mn_intr_pair_t("brhnz", "_16_NZ_IMM($RT$,$LABEL$)"),
	mn_intr_pair_t("brhz",	"_16_Z_IMM($RT$,$LABEL$)"),
	mn_intr_pair_t("brnz",	"_32_NZ_IMM($RT$,$LABEL$)"),
	mn_intr_pair_t("brz",	"_32_Z_IMM($RT$,$LABEL$)"),

	/*mn_intr_pair_t("bihnz", "BRANCH16NZ($RT$,$RA$)"),
	mn_intr_pair_t("bihz",	"BRANCH16Z($RT$,$RA$)"),
	mn_intr_pair_t("binz",	"BRANCH32NZ($RT$,$RA$)"),
	mn_intr_pair_t("biz",	"BRANCH32Z($RT$,$RA$)"),*/	

	mn_intr_pair_t("bihnz", "RETURN_IF_16_NZ_IMM($RT$)"),
	mn_intr_pair_t("bihz",	"RETURN_IF_16_Z_IMM($RT$)"),
	mn_intr_pair_t("binz",	"RETURN_IF_32_NZ_IMM($RT$)"),
	mn_intr_pair_t("biz",	"RETURN_IF_32_Z_IMM($RT$)"),

	mn_intr_pair_t("br",	"goto loc_$LABEL$"),
	mn_intr_pair_t("bra",	"JUMP_ABS($IMM$)"),
	mn_intr_pair_t("brsl",	"$FN_ADDR$//$RT$"),
	mn_intr_pair_t("brasl", "$FN_ADDR$//$RT$"),

	mn_intr_pair_t("bi",	"return;//$RA$"),
	mn_intr_pair_t("iret",	"// IRET"),
	mn_intr_pair_t("bisled","// BISLED"),
	mn_intr_pair_t("bisl",	"CALL_ABS($RA$)"),
};

static const std::map<std::string, std::string> CustomIntrins( IntrinsicTemplateTable, IntrinsicTemplateTable + _countof(IntrinsicTemplateTable) );

/*
void gen_intrin_table()
{
	std::ifstream fin("gah.txt");
	std::ofstream fout("geh.txt");
	std::string line;
	while(std::getline(fin, line))
	{
		const size_t intrin_begin	= line.find("si_");
		const size_t intrin_end		= line.find(")", intrin_begin) + 1;
		const size_t mnem_begin		= intrin_begin + 3;
		const size_t mnem_end		= line.find("(", mnem_begin); 
		std::string intrin		= line.substr(intrin_begin, intrin_end - intrin_begin);
		const std::string mnem	= line.substr(mnem_begin, mnem_end - mnem_begin);
		replace_all( intrin, "ra", "$RA$");
		replace_all( intrin, "rb", "$RB$");
		replace_all( intrin, "rc", "$RC$");
		replace_all( intrin, "rt", "$RT$");
		replace_all( intrin, "imm", "$IMM$");

		fout << "mn_intr_pair_t(\"" << mnem << "\", " << "\"" << intrin << ";\")," << std::endl;
	}
	
}
*/

static const char* ChannelNames[31] = 
{
	"$SPU_RdEventStat",
	"$SPU_WrEventMask",
	"$SPU_WrEventAck",
	"$SPU_RdSigNotify1",
	"$SPU_RdSigNotify2",
	"$5",
	"$6",
	"$SPU_WrDec",
	"$SPU_RdDec",
	"$MFC_WrMSSyncReq",
	"$11",
	"$SPU_RdEventMask",
	"$MFC_RdTagMask",
	"$SPU_RdMachStat",
	"$SPU_WrSRR0",
	"$SPU_RdSRR0",
	"$MFC_LSA",
	"$MFC_EAH",
	"$MFC_EAL",
	"$MFC_Size",
	"$MFC_TagID",
	"$MFC_Cmd",
	"$MFC_WrTagMask",
	"$MFC_WrTagUpdate",
	"$MFC_RdTagStat",
	"$MFC_RdListStallStat",
	"$MFC_WrListStallAck",
	"$MFC_RdAtomicStat", 
	"$SPU_WrOutMbox", 
	"$SPU_RdInMbox", 
	"$SPU_WrOutIntrMbox"
};

static const std::string IntrinsicTemplates[] =
{
	"$RT$ = si_$MNEM$($RA$,$RB$,$RC$);",// SPU_TYPE_RRR
	"$RT$ = si_$MNEM$($RA$,$RB$);",		// SPU_TYPE_RR
	"$RT$ = si_$MNEM$($RA$,$IMM$);",	// SPU_OP_TYPE_RI7
	"$RT$ = si_$MNEM$($RA$,$IMM$);",	// SPU_OP_TYPE_RI8
	"$RT$ = si_$MNEM$($RA$,$IMM$);",	// SPU_OP_TYPE_RI10
	"$RT$ = si_$MNEM$($RA$,$IMM$);",	// SPU_OP_TYPE_RI16
	"$RT$ = si_$MNEM$($RA$,$IMM18$);",	// SPU_OP_TYPE_RI18
	"",									// SPU_OP_TYPE_LBT
	""									// SPU_OP_TYPE_LBTI
};

//static const std::regex RXMnem("\\$MNEM\\$");

static std::string ErrorUnknownInstruction( uint32_t Instruction )
{
	std::ostringstream oss;
	oss << "// UNKNOWN OPCODE: " << lexical_cast(Instruction);
	const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components( Instruction );
	oss << " -- RT:" << OPComponents.RT;
	oss << " RA:" << OPComponents.RA;
	oss << " RB:" << OPComponents.RB;
	oss << " RC:" << OPComponents.RC;
	oss << " IMM:" << OPComponents.IMM;
	return oss.str();
}

static std::string GetIntrinsicForm( const std::string& Mnemonic, SPU_OP_TYPE itype )
{
	std::string result;

	auto iter = CustomIntrins.find(Mnemonic);

	if( CustomIntrins.end() != iter )
	{
		result = iter->second + ";";
	}
	else
	{
		result = IntrinsicTemplates[itype];
		replace_all( result, "$MNEM$", Mnemonic );
	}

	return result;
}

//static const std::regex rxRT("\\$RT\\$");

std::string spu_make_pseudo( SPU_INSTRUCTION Instr, uint32_t IP )
{	
	const std::string		mnem = spu_decode_op_mnemonic(Instr.Instruction);

	if ( mnem.empty() )
		return ErrorUnknownInstruction(Instr.Instruction);

	const SPU_OP_TYPE		itype = spu_decode_op_type( Instr.Instruction );

	std::string				result = GetIntrinsicForm( mnem, itype );

	const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components( Instr.Instruction );

	const size_t LSLR_MASK = 0x3FFF0;
		
	replace_all( result, "$RT$", std::string("GPR(") + lexical_cast_gpr((uint16_t)OPComponents.RT) + std::string(")") ); 

	//result = std::regex_replace(result, rxRT, std::string("GPR(") + lexical_cast_gpr((uint16_t)OPComponents.RT) + std::string(")") );

	switch (itype)
	{
	case SPU_OP_TYPE_RRR:
		{				
			replace_all( result, "$RC$", std::string("GPR(") + lexical_cast((uint16_t)OPComponents.RC) + std::string(")") ); 
		}
	case SPU_OP_TYPE_RR:
		{
			replace_all( result, "$RB$", std::string("GPR(") + lexical_cast((uint16_t)OPComponents.RB) + std::string(")") ); 
			replace_all( result, "$CA$", (OPComponents.RA < 31) ? ChannelNames[OPComponents.RA] : lexical_cast((uint16_t)OPComponents.RA) );
		}
	case SPU_OP_TYPE_RI7:
	case SPU_OP_TYPE_RI8:
	case SPU_OP_TYPE_RI10:
	case SPU_OP_TYPE_RI16:
		{
			replace_all( result, "$RA$", std::string("GPR(") + lexical_cast((uint16_t)OPComponents.RA) + std::string(")") ); 

			replace_all( result, "$IMM$", lexical_cast_hex((int32_t)OPComponents.IMM) );
				
			replace_all( result, "$LABEL$", lexical_cast_hex_addr( IP + ((int32_t)OPComponents.IMM << 2) ) );

			//replace_all( result, "$QOFF$",		lexical_cast_hex( LSLR_MASK & ((int32_t)OPComponents.IMM << 4) ) );
			replace_all( result, "$WADDR_REL$", lexical_cast_hex( LSLR_MASK & (IP + ((int32_t)OPComponents.IMM << 2)) ) );
			replace_all( result, "$WADDR_ABS$", lexical_cast_hex( LSLR_MASK & ((int32_t)OPComponents.IMM << 2) ) );

			std::ostringstream oss;
			oss << "sub_" << std::hex << (IP + ((int32_t)OPComponents.IMM << 2)) << "();";
			replace_all( result, "$FN_ADDR$", oss.str() );

			if ( "brnz" == mnem || "brz" == mnem || "brhnz" == mnem || "brhz" == mnem )
			{
				if ( OPComponents.IMM > 0 )
				{
					result = std::string("IF") + result;
				}
				else
				{
					result = std::string("WHILE") + result;
				}
			}
			else if ( "stop" == mnem )
			{
				union stopi
				{
					struct 
					{
						uint32_t snstype : 14;
						uint32_t pad : 7;
						uint32_t OP : 11;						
					} stop_fmt;
					uint32_t raw;
				};

				stopi si = {Instr.Instruction};

				replace_all( result, "$IMM$", lexical_cast_hex(si.stop_fmt.snstype ) );
			}
			break;
		}
	case SPU_OP_TYPE_RI18:
		{
			replace_all( result, "$RA$", std::string("GPR(") + lexical_cast((uint16_t)OPComponents.RA) + std::string(")") ); 
			const size_t LSLR = 0x3FFFF;
			replace_all( result, "$IMM18$", lexical_cast(OPComponents.IMM & LSLR) );
			result.resize(32, ' ');
			result += lexical_cast_hex2(OPComponents.IMM & LSLR);
			break;
		}
	case SPU_OP_TYPE_LBT:
	case SPU_OP_TYPE_LBTI:
		{
			replace_all( result, "$RA$", std::string("GPR(") + lexical_cast((uint16_t)OPComponents.RA) + std::string(")") ); 

			replace_all( result, "$BRINST$", lexical_cast_hex((int32_t)OPComponents.IMM) );
			replace_all( result, "$BRTARG$", lexical_cast_hex((int32_t)((uint64_t)OPComponents.IMM >> 32)) );
			break;
		}
	}

	return result;
}