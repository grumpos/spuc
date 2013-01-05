#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <cassert>
#include <memory>
#include "spu_idb.h"
#include "elf.h"
#include "elf_helper.h"
#include "basic_blocks.h"
#include "src_file.h"
#include "basicblock.h"
#include "fn.h"

using namespace std;

struct cfgnode
{
	cfgnode()
		: block(nullptr) 
	{}

	vector<cfgnode*> pred;	
	vector<cfgnode*> succ;
	bb* block;
};

void spuGatherLoads( const vector<uint32_t>& Binary, spu::op_distrib_t& OPDistrib,
					size_t VirtBase )
{	
	auto GatherAbsAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& AbsMemOPs = OPDistrib[mnem];

		std::transform( 
			AbsMemOPs.begin(), AbsMemOPs.end(), 
			std::inserter(Addresses, Addresses.end()), 
			[&]( size_t Offset )->uint32_t
		{
			const uint32_t LSLR = 0x3ffff & -16;

			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[Offset]);

			return ((uint32_t)OPComponents.IMM << 2) & LSLR;
		});
	};

	auto GatherRelAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& RelMemOPs = OPDistrib[mnem];

		std::transform( RelMemOPs.begin(), RelMemOPs.end(), 
			std::inserter(Addresses, Addresses.end()), 
			[&]( size_t Offset )->uint32_t
		{
			const uint32_t LSLR = 0x3ffff & -16;

			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[Offset]);

			return (VirtBase + (Offset*4) + ((int32_t)OPComponents.IMM << 2)) & LSLR;
		});
	};

	std::set<uint32_t> AbsLoadTargets;
	std::set<uint32_t> RelLoadTargets;
	std::set<uint32_t> AbsStoreTargets;
	std::set<uint32_t> RelStoreTargets;

	GatherAbsAddresses( "lqa", AbsLoadTargets );
	GatherRelAddresses( "lqr", RelLoadTargets );
	GatherAbsAddresses( "stqa", AbsStoreTargets );
	GatherRelAddresses( "stqr", RelStoreTargets );
}


string print_bb( const bb& block )
{
	ostringstream oss;
	oss << "\"";

	for ( auto iter = block.ibegin; iter != block.iend; ++iter )
	{
		oss << spu_decode_op_mnemonic(iter->raw) << "\\n";
	}

	oss << "\"";
	return oss.str();
}

vector<uint8_t> LoadFileBin( string path );

struct fn_sym
{
	string name;
};



struct spu_image_info
{
	uint32_t imgsize;
	uint32_t txt_off;
	uint32_t txt_len;
	uint32_t data_off;
	uint32_t data_len;
};

const string OSFolder = "F:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\";


vector<uint32_t> LoadBootloaderBin()
{
	ifstream iff( (OSFolder + "bootldr.elf").c_str(), 
		ios::in | ios::binary );
	const size_t filesize = iff.tellg();
	vector<uint32_t> BootldrImg;
	BootldrImg.resize( (0x40000  - 0x400) / sizeof(uint32_t) );
	iff.seekg( 0x400 );
	iff.read( (char*)BootldrImg.data(), 0x40000 - 0x400 );
	iff.close();
	return BootldrImg;
}




int main( int /*argc*/, char** /*argv*/ )
{
	vector<uint8_t> ELFFile = LoadFileBin((OSFolder + "lv1ldr.elf").c_str());

	//ElfFile<SPU_ELF> ELFFile2((OSFolder + "bootldr.elf").c_str(), 0x29480);
	ElfFile<SPU_ELF> ELFFile2((OSFolder + "lv1ldr.elf").c_str(), 0);

	//auto internalELF = elf::EnumEmbeddedSPUOffsets(ELFFile);


	vector<uint32_t> SPUBinary;
	vector<uint8_t> BootldrData;
	BootldrData.resize(0x40000 - 0x22000 - 0x400);
	
//	size_t EntryIndex = 0;
	size_t vbase = 0;
	uint8_t* SPU0 = (uint8_t*)ELFFile.data();// + SPUELFOffsets[0];
	{
		elf::HeadersToSystemEndian( SPU0 );

		//SPUBinary = elf::spu::LoadExecutable( SPU0 );
		SPUBinary = LoadBootloaderBin();

		memcpy(BootldrData.data(), (const uint8_t*)SPUBinary.data() + 0x22000, BootldrData.size());

		for ( size_t i = 0; i != SPUBinary.size(); ++i )
		{
			SPUBinary[i] = _byteswap_ulong(SPUBinary[i]);
		}

//		EntryIndex = elf::EntryPointIndex( SPU0 );

		vbase = 0x400;
		//vbase = elf::VirtualBaseAddr( SPU0 );
	}

	/*string disasm;

	for (auto op : SPUBinary)
	{
		std::string spu_disassemble( uint32_t instr_raw );

		disasm += spu_disassemble(op) += "\n";
	}

	ofstream bootldrdis("F:\\boot.dis");
	bootldrdis << disasm;
	bootldrdis.close();*/



	
	
	

	SPUBinary.resize( 0x22000 / 4 ); // FIXME hardcoded for now bootldr
	//SPUBinary.resize( 0x190C0 / 4 ); // FIXME hardcoded for now lv1ldr
	//SPUBinary.resize( 0x12ef0 / 4 ); // FIXME hardcoded for now lv2ldr

	

	vector<spu_insn> insninfo;
	{
		spu_insn_process_bin( SPUBinary, insninfo, vbase );
	}

	spu::op_distrib_t OPDistrib;
	{
		OPDistrib = spu::GatherOPDistribution( SPUBinary );
	}

	//spuGatherLoads( SPUBinary, OPDistrib, vbase );

	set<size_t> brsl_targets = spu_get_brsl_targets(OPDistrib, insninfo, vbase);

	vector<size_t> bb_leads = spu_find_basicblock_leader_offsets(
		OPDistrib, insninfo );

	vector<bb> blocks = bb_genblocks( bb_leads, insninfo );

	bb_calctypes( blocks );

	//ifstream disasm_file("F:\\Dropbox\\lv1ldr.dis");
	//assert(disasm_file.is_open());
	//string line;
	//do { getline(disasm_file, line); } while (line.find("12c00:") == string::npos);
	//vector<string> bin_disasm;
	//bin_disasm.push_back(line);
	//while (getline(disasm_file, line)) { bin_disasm.push_back(line); }

	vector<fn> functions = bb_genfn(blocks, insninfo, brsl_targets);

	// scan function insns for register usage
	// registers 3-74 are used for argument passing
	// any register that is used as source operand before it was
	// used as destination operand must be a function argument
	for (auto& fun : functions)
	{
		// unused operands have index SPU_OP_INVALID_GPR
		// allocating enough space for this index removes lots of ifs
		uint8_t Registers[SPU_OP_INVALID_GPR + 1] = {0};
		uint8_t ArgCount = 0;

		auto check_gpr = [&Registers, &ArgCount](uint8_t gpr)
		{
			const bool is_arg_reg = gpr > 2 && gpr < 75;
			
			if (is_arg_reg && !Registers[gpr])
			{
				ArgCount = max<uint8_t>( gpr - 2, ArgCount );
			}
		};

		for (spu_insn* insn = fun.entry->ibegin; insn != fun.exit->iend; ++insn)
		{
			SPU_OP_COMPONENTS& OPComp = insn->comps;

			check_gpr(OPComp.RA);
			check_gpr(OPComp.RB);
			check_gpr(OPComp.RC);

			// flag register as written
			Registers[OPComp.RT] = 1;
		}

		fun.argcnt = ArgCount;
	}

	//auto old_entries = known_fn_entries;

	//for (auto function : functions)
	//{
	//	if (function.exit->type == bbtype::sjumpb || function.exit->type == bbtype::sjumpf)
	//	{
	//		const spu_insn* target_insn = 
	//			function.exit->branch + function.exit->branch->comps.IMM;
	//		if (target_insn->vaddr < function.entry->ibegin->vaddr
	//			|| target_insn->vaddr >= function.exit->iend->vaddr)
	//		{
	//			//fn_entry_after_term.insert(target_insn->vaddr);
	//			known_fn_entries.insert(insn2block[target_insn]);
	//		}
	//	}
	//}
		
	set<const spu_insn*> unused_insn;
	set<bool> dumper;

	transform(functions.begin(), functions.end()-1,
		functions.begin()+1,
		inserter(dumper, dumper.end()),
		[&](const fn& a, const fn& b)
	{
		auto en = a.exit->iend;
		auto ex = b.entry->ibegin;
		while (en != ex)
			unused_insn.insert(en++);

		return false;
	});
		

/*	decltype(known_fn_entries) dff;
	set_difference(known_fn_entries.begin(), known_fn_entries.end(),
		old_entries.begin(), old_entries.end(),
		inserter(dff, dff.end()));*/

	auto cfg_connect = [](cfgnode& a, cfgnode& b)
	{
		a.succ.push_back(&b);
		b.pred.push_back(&a);

	};

	cfgnode root;
	vector<cfgnode> nodelist;
	nodelist.reserve(blocks.size());

	for (auto& fun : functions)
	{
		if (fun.exit->type == bbtype::sjumpf 
			|| fun.exit->type == bbtype::sjumpb)
		{
			auto jump_target = (fun.exit->branch + fun.exit->branch->comps.IMM)->parent;

			if (jump_target < fun.entry || jump_target > fun.exit)
			{
				fun.exit->type = bbtype::ret_tco;
			}
		}
	}

	for (auto& block : blocks)
	{
		cfgnode newnode;
		newnode.block = &block;
		nodelist.push_back(newnode);
	}

	for (auto& fun : functions)
	{
		const size_t entry_offset = fun.entry - &blocks[0];
		cfg_connect(root, nodelist[entry_offset]);
	}

	for ( size_t ii = 0; ii < nodelist.size(); ++ii )
	{
		bb* block = nodelist[ii].block;
		switch (block->type)
		{
		case bbtype::code:
			{
				cfg_connect(nodelist[ii], nodelist[ii + 1]);
				break;
			}
		case bbtype::sjumpf:
		case bbtype::sjumpb:
			{
				bb* from = nodelist[ii].block;
				bb* to = (from->branch + from->branch->comps.IMM)->parent;
				cfg_connect(nodelist[ii], nodelist[ii + (to - from)]);
				break;
			}
		case bbtype::cjumpf:
		case bbtype::cjumpb:
			{
				bb* from = nodelist[ii].block;
				bb* to = (from->branch + from->branch->comps.IMM)->parent;
				cfg_connect(nodelist[ii], nodelist[ii + (to - from)]);
				cfg_connect(nodelist[ii], nodelist[ii + 1]);
				break;
			}
		case bbtype::scall:
		case bbtype::dcall:
			{
				cfg_connect(nodelist[ii], nodelist[ii + 1]);
				break;
			}
		default:
			break;
		}
	}

	vector<spu_insn*> uncalled;
	for (auto& node : nodelist)
	{
		if (node.pred.empty() && node.block->ibegin->op != spu_op::M_LNOP)
		{
			uncalled.push_back(node.block->ibegin);
		}
	}

	

/*	spu::MakeSPUSrcFile( SPUBinary, FnRanges, 0, 
		elf::VirtualBaseAddr(SPU0), elf::EntryPointIndex(SPU0)*4 );
	*/
	return 0;
}

vector<uint8_t> LoadFileBin( string path )
{
	vector<uint8_t> ELFFile;
	{
		ifstream iff( path.c_str(), ios::in | ios::ate | ios::binary );
		const size_t filesize = iff.tellg();
		ELFFile.resize( filesize );
		iff.seekg( 0 );
		iff.read( (char*)ELFFile.data(), filesize );
		iff.close();
	}
	{
		if ( ELFFile.empty() )
		{
			cout << "Input file not found" << endl;
		}
	}

	return ELFFile;
}
