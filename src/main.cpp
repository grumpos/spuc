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
#include <iomanip>
#include "spu_idb.h"
#include "elf.h"
#include "elf_helper.h"
#include "basic_blocks.h"
#include "src_file.h"
#include "basicblock.h"
#include "fn.h"

using namespace std;

string spu_disassemble( const spu_insn* insn );

template<class T>
set<T> operator-(const set<T>& lhs, const set<T>& rhs)
{
	set<T> result;
	set_difference(lhs.begin(), lhs.end(),
		rhs.begin(), rhs.end(), inserter(result, result.end()));
	return result;
}

struct cfgnode
{
	cfgnode()
		: block(nullptr) 
	{}

	vector<cfgnode*> pred;	
	vector<cfgnode*> succ;
	bb* block;
};

//void spuGatherLoads( const vector<uint32_t>& Binary, spu::op_distrib_t& OPDistrib,
//					size_t VirtBase )
//{	
//	auto GatherAbsAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
//	{
//		const auto& AbsMemOPs = OPDistrib[mnem];
//
//		std::transform( 
//			AbsMemOPs.begin(), AbsMemOPs.end(), 
//			std::inserter(Addresses, Addresses.end()), 
//			[&]( size_t Offset )->uint32_t
//		{
//			const uint32_t LSLR = 0x3ffff & -16;
//
//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[Offset]);
//
//			return ((uint32_t)OPComponents.IMM << 2) & LSLR;
//		});
//	};
//
//	auto GatherRelAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
//	{
//		const auto& RelMemOPs = OPDistrib[mnem];
//
//		std::transform( RelMemOPs.begin(), RelMemOPs.end(), 
//			std::inserter(Addresses, Addresses.end()), 
//			[&]( size_t Offset )->uint32_t
//		{
//			const uint32_t LSLR = 0x3ffff & -16;
//
//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[Offset]);
//
//			return (VirtBase + (Offset*4) + ((int32_t)OPComponents.IMM << 2)) & LSLR;
//		});
//	};
//
//	std::set<uint32_t> AbsLoadTargets;
//	std::set<uint32_t> RelLoadTargets;
//	std::set<uint32_t> AbsStoreTargets;
//	std::set<uint32_t> RelStoreTargets;
//
//	GatherAbsAddresses( "lqa", AbsLoadTargets );
//	GatherRelAddresses( "lqr", RelLoadTargets );
//	GatherAbsAddresses( "stqa", AbsStoreTargets );
//	GatherRelAddresses( "stqr", RelStoreTargets );
//}

//vector<uint8_t> LoadFileBin( string path );

struct fn_sym
{
	string name;
};



struct spu_image_info
{
	uint32_t txt_off;
	uint32_t txt_len;
	uint32_t data_off;
	uint32_t data_len;
};

const string OSFolder = "F:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\";

vector<uint8_t> LoadBinFile( string path )
{
	ifstream iff( path.c_str(), ios::in | ios::binary | ios::ate );
	const size_t filesize = iff.tellg();
	vector<uint8_t> FileData(filesize);
	iff.seekg(0);
	iff.read( (char*)FileData.data(), filesize );
	return FileData;
}

//class SPUImage
//{
//public:
//	vector<uint32_t> SPUTextSection;
//	vector<uint8_t> SPUDataSection;
//	vector<uint8_t> SPULSImage;
//
//public:
//	SPUImage(vector<uint8_t>&& SPUBinary)
//		: SPULSImage(SPUBinary)
//	{
//		 ReverseLSQWords();
//	}
//private:
//	void ReverseLSQWords()
//	{
//		uint64_t* LSDataBegin = (uint64_t*)SPULSImage.data();
//		uint64_t* LSDataEnd = (uint64_t*)SPULSImage.data();
//
//		while (LSDataBegin != LSDataEnd)
//		{
//			const uint64_t Temp = _byteswap_uint64(*LSDataBegin);
//			*LSDataBegin = _byteswap_uint64(*++LSDataBegin);
//			*LSDataBegin++ = Temp;
//		}
//	}
//};


int main( int /*argc*/, char** /*argv*/ )
{
	//auto internalELF = elf::EnumEmbeddedSPUOffsets(ELFFile);
	
	vector<uint32_t> SPUTextSection;
	vector<uint8_t> SPUDataSection;
	vector<uint8_t> SPULSImage;
	uint32_t VirtualBase = 0;

#if 1
	{
		// bootldr.elf
		SPULSImage = LoadBinFile(OSFolder + "bootldr.elf");

		const spu_image_info BoorldrDesc =
		{
			0x400, 0x22000,
			0x22400, (0x40000 - 0x22400)
		};

		VirtualBase = 0x400;

		SPUTextSection.resize(BoorldrDesc.txt_len / 4);
		memcpy(SPUTextSection.data(), 
			SPULSImage.data() + BoorldrDesc.txt_off, 
			BoorldrDesc.txt_len);

		SPUDataSection.resize(BoorldrDesc.data_len);
		copy(SPULSImage.begin() + BoorldrDesc.data_off, 
			SPULSImage.begin() + BoorldrDesc.data_off + BoorldrDesc.data_len, 
			SPUDataSection.begin());
	}
#else
	{
		// lv1ldr.elf
		ElfFile<SPU_ELF> ELFFile((OSFolder + "lv1ldr.elf").c_str(), 0);

		SPULSImage.resize(0x40000);
		for (size_t ii = 0; ii < ELFFile.ProgramHeaders_.size(); ++ii)
		{
			auto& ph = ELFFile.ProgramHeaders_[ii];
			if (ph.p_type & PT_LOAD)
			{
				memcpy( SPULSImage.data() + ph.p_vaddr, 
					ELFFile.PData_[ii].begin(), 
					ELFFile.PData_[ii].size() );
			}
		}

		VirtualBase = ELFFile.HeaderLE_.e_entry;

		const spu_image_info ImageDesc =
		{
			VirtualBase, 0x190C0,
			VirtualBase + 0x190C0, (0x40000 - (VirtualBase + 0x190C0))
		};

		SPUTextSection.resize(ImageDesc.txt_len / 4);
		memcpy(SPUTextSection.data(), 
			SPULSImage.data() + ImageDesc.txt_off, 
			ImageDesc.txt_len);

		SPUDataSection.resize(ImageDesc.data_len);
		copy(SPULSImage.begin() + ImageDesc.data_off, 
			SPULSImage.begin() + ImageDesc.data_off + ImageDesc.data_len, 
			SPUDataSection.begin());

		ofstream bindump("F:\\lv1ldr.bin", ios::out | ios::binary);
		bindump.write((const char*)SPULSImage.data(), 0x40000 );
	}
#endif

	uint64_t* LSDataBegin = (uint64_t*)SPULSImage.data();
	uint64_t* LSDataEnd = (uint64_t*)SPULSImage.data() + SPULSImage.size() / sizeof(uint64_t);

	while (LSDataBegin != LSDataEnd)
	{
		const uint64_t Temp = _byteswap_uint64(LSDataBegin[0]);
		LSDataBegin[0] = _byteswap_uint64(LSDataBegin[1]);
		LSDataBegin[1] = Temp;
		LSDataBegin += 2;
	}

	for (auto& op : SPUTextSection)
	{
		op = _byteswap_ulong(op);
	}

	//SPUTextSection.resize( 0x22000 / 4 ); // FIXME hardcoded for now bootldr
	//SPUBinary.resize( 0x190C0 / 4 ); // FIXME hardcoded for now lv1ldr
	//SPUBinary.resize( 0x12ef0 / 4 ); // FIXME hardcoded for now lv2ldr

	vector<spu_insn> ilist;
	{
		spu_insn_process_bin( SPUTextSection, ilist, VirtualBase );
	}

	
	

	spu_vm spuVM;
	spuVM.next = &ilist[0];
	spuVM.LS = SPULSImage.data();
	spuVM.vbase = VirtualBase;
	memset(spuVM.GPR, 0, 128 * 16);

	while (spuVM.next)
	{
		void spu_vm_dostep(spu_vm* vm);
		spu_vm_dostep(&spuVM);
	}


	map<spu_op, vector<spu_insn*>> Distrib_new;
	{
		for (auto& insn : ilist)
		{
			Distrib_new[insn.op].push_back(&insn); 
		}
	}	

	

	//spu::op_distrib_t OPDistrib;
	//{
	//	OPDistrib = spu::GatherOPDistribution( SPUTextSection );
	//}

	//spuGatherLoads( SPUTextSection, OPDistrib, VirtualBase );

	set<size_t> brsl_targets = spu_get_brsl_targets(Distrib_new, ilist, VirtualBase);

	vector<size_t> bb_leads = spu_find_basicblock_leader_offsets(
		Distrib_new, ilist );

	vector<bb> blocks = bb_genblocks( bb_leads, ilist );

	bb_calctypes( blocks );

	vector<fn> functions = bb_genfn(blocks, ilist, brsl_targets);

	// try a validation pass maybe?
	// branches should jump inside the function only. except the TCO exits jumps

	auto is_jump = [](spu_insn* insn)
	{
		return insn->op == spu_op::M_BR
			|| insn->op == spu_op::M_BRHNZ
			|| insn->op == spu_op::M_BRHZ
			|| insn->op == spu_op::M_BRZ
			|| insn->op == spu_op::M_BRHZ;			
	};

	auto jump_target = [](spu_insn* insn)
	{
		return insn + insn->comps.IMM;
	};

	ostringstream oss;

	

	for (auto& fun : functions)
	{
		oss << "function " << "sub" 
			<< setw(8) << setfill('0') << hex << fun.entry->ibegin->vaddr 
			<< endl;

		for (auto block = fun.entry; block <= fun.exit; ++block)
		{
			oss << "\t" << "loc" 
				<< setw(8) << setfill('0') << hex << block->ibegin->vaddr 
				<< ":" << endl;
			for (auto insn = block->ibegin; insn != block->iend; ++insn)
			{
				oss << "\t" 
					<< spu_disassemble(insn)
					<< endl;
			}
		}
	}

	ofstream off("F:\\bootldr.dis");
	off << oss.str();
	off.close();

	for (auto& fun : functions)
	{
		auto fn_begin = fun.entry->ibegin;
		auto fn_end = fun.exit->iend;

		for (auto block = fun.entry; block != fun.exit; ++block)
		{
			for (auto insn = block->ibegin; insn != block->iend; ++insn)
			{
				if (is_jump(insn))
				{
					auto to = jump_target(insn);

					if (to < fn_begin 
						|| to >= fn_end)
						cout << hex << insn->vaddr << "->" << hex << to->vaddr << endl;
				}
			}
		}
	}
	
	
	
	fn_calc_argcount(functions);
		
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
		





	/*auto cfg_connect = [](cfgnode& a, cfgnode& b)
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
		case bbtype::scall:
		case bbtype::dcall:
		case bbtype::cdjump:
		case bbtype::code:
		case bbtype::stopsignal:
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
		default:
			break;
		}
	}

	set<spu_insn*> uncalled;
	set<spu_insn*> from_brsl;
	set<spu_insn*> fn_starters;
	set<spu_insn*> fn_hidden;
	for (size_t vaddr : brsl_targets)
	{
		from_brsl.insert(&ilist[(vaddr - 0x12c00) / 4]);
	}
	for (auto& fun : functions)
	{
		fn_starters.insert(fun.entry->ibegin);
	}
	fn_hidden = fn_starters - from_brsl;
	for (auto& node : nodelist)
	{
		if (node.pred.empty() 
			&& node.block->ibegin->op != spu_op::M_STOP)
		{
			if (node.block->ibegin->op == spu_op::M_LNOP
				&& node.block->ibegin->vaddr % 8 != 0)
			{
				uncalled.insert((node.block + 1)->ibegin);
			}
			else
			{
				uncalled.insert(node.block->ibegin);
			}
		}
	}

	set<spu_insn*> foo;
	set<spu_insn*> foo2;
	foo = uncalled - from_brsl;
	foo2 = foo - fn_hidden;*/
	

/*	spu::MakeSPUSrcFile( SPUBinary, FnRanges, 0, 
		elf::VirtualBaseAddr(SPU0), elf::EntryPointIndex(SPU0)*4 );
	*/
	return 0;
}
