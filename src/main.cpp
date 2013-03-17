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
#include "spu_emu.h"
#include "spu_idb.h"
#include "elf.h"
#include "elf_helper.h"
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

struct uint18_t
{
	uint18_t() : data(0) {}
	uint18_t(uint32_t u) : data(u) { mask(); }

	inline void mask() { data &= 0x3ffff; }

	uint18_t& operator+=(const uint18_t& rhs)
	{
		data += rhs;
		mask();
		return *this;
	}

	uint18_t& operator-=(const uint18_t& rhs)
	{
		data -= rhs;
		mask();
		return *this;
	}

	operator uint32_t() const { return data; }

	uint32_t data;
};

uint18_t operator+(const uint18_t& lhs, const uint18_t& rhs)
{
	uint18_t res(lhs);
	res += rhs;
	return res;
}

uint18_t operator-(const uint18_t& lhs, const uint18_t& rhs)
{
	uint18_t res(lhs);
	res -= rhs;
	return res;
}

template <class T>
struct ptr_range
{
	ptr_range() : _begin(nullptr), _end(nullptr) {}
	ptr_range(T* bb, T* ee) : _begin(bb), _end(ee) {}

	T* begin() { return _begin; }
	T* end() { return _end; }

	T* const _begin;
	T* const _end;
};

//template <class T> T*
//begin<ptr_range>( const ptr_range& pr )
//{
//	return pr._begin;
//}

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

struct loop_dowhile
{
	loop_dowhile( vector<spu_insn>& ilist, bb* brx_neg )
		: head(nullptr),
		cont(nullptr),
		brk(brx_neg + 1)
	{
		uint32_t to_vaddr = brx_neg->branch->vaddr + brx_neg->branch->comps.IMM * 4;
		to_vaddr &= 0x3ffff;
		spu_insn* target = &ilist[(to_vaddr - ilist[0].vaddr) / 4];
		head = target->parent;
	}

	bb* head;
	bb* cont;
	bb* brk;
};

enum looptype 
{
	lt_dowhile,
	lt_while,
	lt_for
};

struct spu_image_info
{
	uint32_t txt_off;
	uint32_t txt_len;
	uint32_t data_off;
	uint32_t data_len;
};

const string OSFolder = "F:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\";

vector<uint8_t> ReadFileBin( string path )
{
	ifstream iff( path.c_str(), ios::in | ios::binary | ios::ate );
	const size_t filesize = iff.tellg();
	vector<uint8_t> FileData(filesize);
	iff.seekg(0);
	iff.read( reinterpret_cast<char*>(FileData.data()), filesize );
	return FileData;
}

void DumpDisassembly( const vector<fn>& functions, const string& path );

class SPUImage
{
public:
	vector<uint32_t> SPUTextSection;
	vector<uint8_t> SPUDataSection;
	vector<uint8_t> SPULSImage;

public:
	SPUImage(vector<uint8_t>&& SPUBinary)
		: SPULSImage(SPUBinary)
	{
		ReverseLSQWords();
	}
private:
	void ReverseLSQWords()
	{
		const size_t DwordCount	= SPULSImage.size() / sizeof(uint64_t);
		uint64_t* LSDataBegin	= reinterpret_cast<uint64_t*>(SPULSImage.data());
		uint64_t* LSDataEnd		= reinterpret_cast<uint64_t*>(SPULSImage.data()) + DwordCount;

		while (LSDataBegin != LSDataEnd)
		{
			LSDataBegin[0] = _byteswap_uint64(LSDataBegin[0]);
			LSDataBegin[1] = _byteswap_uint64(LSDataBegin[1]);
			swap(LSDataBegin[0], LSDataBegin[1]);
			LSDataBegin += 2;
		}
	}
};

struct SPUDecoder
{
	typedef spu_insn InsnType;

	vector<InsnType> Decode(vector<uint32_t>& IList, size_t VBase);
};

template<class InsnType, class BinaryDecoder>
class ModuleEnv
{
public:

};

// A 128 bit byteswap for LE systems
void PreprocessLS( vector<uint8_t>& SPULSImage )
{
	const size_t DwordCount	= SPULSImage.size() / sizeof(uint64_t);
	uint64_t* LSDataBegin	= reinterpret_cast<uint64_t*>(SPULSImage.data());
	uint64_t* LSDataEnd		= reinterpret_cast<uint64_t*>(SPULSImage.data()) + DwordCount;

	while (LSDataBegin != LSDataEnd)
	{
		LSDataBegin[0] = _byteswap_uint64(LSDataBegin[0]);
		LSDataBegin[1] = _byteswap_uint64(LSDataBegin[1]);
		swap(LSDataBegin[0], LSDataBegin[1]);
		LSDataBegin += 2;
	}
}


int main( int /*argc*/, char** /*argv*/ )
{
	//auto internalELF = elf::EnumEmbeddedSPUOffsets(ELFFile);

	//void symbol_test();
	//symbol_test();

	//return 0;
	//
	vector<uint32_t> SPUTextSection;
	vector<uint8_t> SPUDataSection;
	vector<uint8_t> SPULSImage;
	uint32_t VirtualBase = 0;

#if 1
	{
		// bootldr.elf
		SPULSImage = ReadFileBin(OSFolder + "bootldr.elf");

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

	PreprocessLS(SPULSImage);

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

	//
	//

	//spu_vm spuVM;
	//spuVM.next = &ilist[0];
	//spuVM.LS = SPULSImage.data();
	//spuVM.vbase = VirtualBase;
	//memset(spuVM.GPR, 0, 128 * 16);

	//while (spuVM.next)
	//{
	//	void spu_vm_dostep(spu_vm* vm);
	//	spu_vm_dostep(&spuVM);
	//}

	//string spu_make_pseudo( const spu_insn* insn );
	//ostringstream oss;


	//for (auto insn : ilist)
	//{
	//	oss << spu_make_pseudo( &insn ) << endl;
	//}

	//ofstream off("F:\\boot_src.cpp");
	//off << oss.str();
	//off.close();


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

	/*vector<size_t> bb_leads = spu_find_basicblock_leader_offsets(
	Distrib_new, ilist );*/

	vector<bb> blocks = bb_genblocks( ilist, Distrib_new );

	bb_calctypes( blocks );

	vector<fn> functions = bb_genfn(blocks, ilist, brsl_targets, VirtualBase);

//	assert( functions.size() == 0x1d6);

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

	//DumpDisassembly(functions, "F:\\bootldr.dis");


	// look for blocks that jump out of functions and are not exits
	//for (auto& fun : functions)
	//{
	//	auto fn_begin = fun.entry->ibegin;
	//	auto fn_end = fun.exit->iend;

	//	for (auto block = fun.entry; block != fun.exit; ++block)
	//	{
	//		for (auto insn = block->ibegin; insn != block->iend; ++insn)
	//		{
	//			if (is_jump(insn))
	//			{
	//				auto to = jump_target(insn);

	//				if (to < fn_begin 
	//					|| to >= fn_end)
	//					cout << hex << insn->vaddr << "->" << hex << to->vaddr << endl;
	//			}
	//		}
	//	}
	//}
	
	
	
	fn_calc_argcount(functions);
		
	//set<const spu_insn*> unused_insn;
	//set<bool> dumper;

	//transform(functions.begin(), functions.end()-1,
	//	functions.begin()+1,
	//	inserter(dumper, dumper.end()),
	//	[&](const fn& a, const fn& b)
	//{
	//	auto en = a.exit->iend;
	//	auto ex = b.entry->ibegin;
	//	while (en != ex)
	//		unused_insn.insert(en++);

	//	return false;
	//});

	
	// static jumps at function exits are TCOs
	for ( auto& fun : functions )
	{
		if ( fun.exit->branch->op == spu_op::M_BR )
		{
			fun.exit->type = bbtype::ret_tco;
		}
	}

	// mid-function static jumps to known function entries are TCOs
	for ( auto* insn : Distrib_new[spu_op::M_BR] )
	{
		const size_t to_vaddr = (insn->vaddr + insn->comps.IMM * 4) & 0x3ffff;
		spu_insn* target = &ilist[(to_vaddr - ilist[0].vaddr) / 4];
		for ( auto& fun : functions )
		{
			if ( target == fun.entry->ibegin)
			{
				insn->parent->type = bbtype::ret_tco;
				break;
			}
		}
	}


	
	ostringstream oss;
	for ( auto& fun : functions )
	{
		oss << "function " << fun.entry->ibegin->vaddr << endl;

		vector<loop_dowhile> loops;

		for ( auto& block : ptr_range<bb>(fun.entry, fun.exit + 1) )
		{
			if ( bbtype::cjumpb == block.type
				|| bbtype::sjumpb == block.type)
			{
				loops.push_back(loop_dowhile(ilist, &block));
			}
		}

		for ( auto& block : ptr_range<bb>(fun.entry, fun.exit + 1) )
		{
			oss << "\t";

			switch (block.type)
			{
			case bbtype::cjumpf:
				{
					const size_t to_vaddr = (block.branch->vaddr + block.branch->comps.IMM * 4) & 0x3ffff;
					spu_insn* target = &ilist[(to_vaddr - ilist[0].vaddr) / 4];
					oss << "IF\t\t" << hex << block.branch->vaddr << " - " << hex << target->vaddr;
					break;
				}
			case bbtype::cjumpb:
				{
					const size_t to_vaddr = (block.branch->vaddr + block.branch->comps.IMM * 4) & 0x3ffff;
					spu_insn* target = &ilist[(to_vaddr - ilist[0].vaddr) / 4];
					oss << "DO-WHILE\t";
					oss << "head: " << hex << target->vaddr;
					oss << " step: " << hex << (&block - 1)->ibegin->vaddr;
					oss << " continue: " << hex << block.ibegin->vaddr;
					oss << " break: " << hex << (block.branch + 1)->vaddr;
					break;
				}
			case bbtype::sjumpf:
				{
					const size_t to_vaddr = (block.branch->vaddr + block.branch->comps.IMM * 4) & 0x3ffff;
					spu_insn* target = &ilist[(to_vaddr - ilist[0].vaddr) / 4];

					loop_dowhile dw(ilist, &block);
					oss << "GOTO\t\t";
					oss << hex << target->vaddr;
					break;
				}
			case bbtype::sjumpb:
				{
					const size_t to_vaddr = (block.branch->vaddr + block.branch->comps.IMM * 4) & 0x3ffff;
					spu_insn* target = &ilist[(to_vaddr - ilist[0].vaddr) / 4];
					oss << "FOREVER\t\t";
					oss << "head: " << hex << target->vaddr;
					oss << " step: " << hex << (&block - 1)->ibegin->vaddr;
					oss << " continue: " << hex << block.ibegin->vaddr;
					oss << " break: " << hex << (block.branch + 1)->vaddr;
					break;
				}
			case bbtype::scall:
				{
					oss << "CALL";
					break;
				}
			default:
				{
					oss << "CODE";
					break;
				}
			}
			oss << endl;
		}
	}

	ofstream("F:\\bootldr_branches.txt") << oss.str();


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

void DumpDisassembly( const vector<fn>& functions, const string& path )
{
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

	ofstream off(path.c_str());
	off << oss.str();
	off.close();
}
