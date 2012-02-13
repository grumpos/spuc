
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <iomanip>
#include <intrin.h>
#include <string>
#include <fstream>
#include <sstream>
#include "spu_idb.h"
#include "elf.h"
#include "raw.h"
#include "elf_helper.h"
#include "basic_blocks.h"
#include "src_file.h"
#include "spu_pseudo.h"

using namespace std;

void spuGatherLoads( const vector<uint32_t>& Binary, spu::op_distrib_t& OPDistrib,
	size_t VirtBase )
{	
	auto GatherAbsAddresses = [&]( const string& mnem, std::set<uint32_t>& Addresses )
	{
		const auto& AbsMemOPs = OPDistrib[mnem];
		//auto program_local = Binary;

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
		//auto program_local = program;

		std::transform( RelMemOPs.begin(), RelMemOPs.end(), std::inserter(Addresses, Addresses.end()), 
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

template<class Container, class Formatter> 
void ConsoleDisplay( Container C, Formatter F );

int main( int argc, char** argv )
{
	/*memmap_t* metldr = mmopen("D:\\Downloads\\fail0verflow_ps3tools_win32_\\NOR\\asecure_loader\\metldr");
	unsigned char* buf = new unsigned char[mmsize(metldr)];
	memmap_t* metldr_key = mmopen("C:\\Users\\Grupos\\ps3keys\\ldr-key-retail");

	AES_KEY key;
	AES_set_decrypt_key((const unsigned char*)mmbegin(metldr_key),mmsize(metldr_key)*8, &key);

	for ( size_t i = 0; i < (mmsize(metldr)/16);++i)
	{
	AES_decrypt((const unsigned char*)mmbegin(metldr) + (16*i), buf + (16*i), &key);
	}



	ofstream off("mt", ios::binary | ios::out );
	off.write( (const char*)buf, mmsize(metldr));*/

	/*ifstream iff("D:\\PS3\\spe_reg.txt");
	ofstream off("D:\\PS3\\spe_reg.dump");

	regex prnth("\\((.*)\\)");

	if (iff.is_open())
	{
		string line;

		while (getline(iff, line))
		{
			smatch res;

			regex_search( line, res, prnth);

			string rname = res[1].str();

			if ( !rname.empty() && "CBEA Architected Registers" != rname && "Implementation-Specific Registers" != rname )
				off << "uint64_t " << rname << ";" << endl;
		}
	}*/

	/*ifstream iff("D:\\PS3\\spu_0.cpp");
	ofstream off("D:\\PS3\\tree.txt");

	if (iff.is_open())
	{
	regex fn_sig("static void (sub_.*)\\(\\)");
	regex fn_call("(sub_[0-9a-fA-F]*)");

	string line;



	while (getline(iff, line))
	if ( "static void sub_12c00()" == line )
	break;

	string CurrentFn = "sub_12c00";
	smatch match1, match2;

	while (getline(iff, line))
	{
	if ( regex_search( line, match1, fn_sig) )
	{
	Calls[CurrentFn];
	CurrentFn = match1[1].str();

	continue;
	}
	else if ( regex_search( line, match2, fn_call) )
	{
	if ( Calls[CurrentFn].end() == find(Calls[CurrentFn].begin(), Calls[CurrentFn].end(), match2[1].str() ) )
	Calls[CurrentFn].push_back(match2[1].str());
	}
	}

	}

	f(Calls.begin()->first, 0, off);*/

	vector<uint8_t> ELFFile;
	{
		const std::string ModuleName = (argc > 1) ? argv[1] : "F:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\lv1ldr.elf";
		//memmap_t* ELFFileMapped = mmopen( "F:\\PS3\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf" );//aim_spu_module
		memmap_t* ELFFileMapped = mmopen( ModuleName.c_str() );
		//memmap_t* ELFFileMapped = mmopen( "F:\\cell\\host-win32\\spu\\bin\\a.out" );

		ELFFile.resize( mmsize(ELFFileMapped) );

		memcpy( ELFFile.data(), mmbegin(ELFFileMapped), mmsize(ELFFileMapped) );

		mmclose(ELFFileMapped);
	}
	{
		if ( ELFFile.empty() )
		{
			return 1;
		}
	}

// 	vector<size_t> SPUELFOffsets;
// 	{
// 		SPUELFOffsets = elf::EnumEmbeddedSPUOffsets( ELFFile );
// 	}

// 	for_each( SPUELFOffsets.begin(), SPUELFOffsets.end(),
// 		[&ELFFile](size_t Offset)
// 	{
// 		ostringstream oss;
// 		oss << "spu" << hex << setw(8) << Offset << ".elf";
// 
// 		ofstream ofs(oss.str().c_str(), ios::out | ios::binary);
// 		
// 		const uint8_t* b = ELFFile.data() + Offset;
// 		const Elf32_Ehdr* eh = (Elf32_Ehdr*)b;
// 		const uint8_t* e = b;
// 		if ( _byteswap_ushort(eh->e_shnum) )
// 		{
// 			e = b + _byteswap_ulong(eh->e_shoff) + (_byteswap_ushort(eh->e_shnum) * _byteswap_ushort(eh->e_shentsize));
// 
// 		}
// 		else
// 		{
// 			Elf32_Phdr* LastPH = (Elf32_Phdr*)(b + _byteswap_ulong(eh->e_phoff));
// 			LastPH += _byteswap_ushort(eh->e_phnum) - 1;
// 			e = b + _byteswap_ulong(LastPH->p_offset) + (_byteswap_ulong(LastPH->p_filesz));
// 		}		
// 
// 		ofs.write((const char*)b, e-b);
// 	});

	vector<uint32_t> SPUBinary;
	size_t EntryIndex = 0;
	uint8_t* SPU0 = (uint8_t*)ELFFile.data();// + SPUELFOffsets[0];
	{
		elf::HeadersToSystemEndian( SPU0 );

		SPUBinary = elf::spu::LoadExecutable( SPU0 );

		for ( size_t i = 0; i != SPUBinary.size(); ++i )
		{
			SPUBinary[i] = _byteswap_ulong(SPUBinary[i]);
		}

		EntryIndex = elf::EntryPointIndex( SPU0 );
	}

	

	//spuGatherLoads( SPUBinary, OPDistrib, elf::VirtualBaseAddr(SPU0) );
	vector<uint64_t> OPFlags;


	spu_img_regions_t regs = elf::spu::ReadLoadRegions( SPU0 );

	// find first invalid op
	size_t invalid = 0;
	for ( size_t i = 0; i < SPUBinary.size(); ++i )
	{
		if ( -1 == spu_decode_op_type( SPUBinary[i] ) )
		{
			invalid = i;
			break;
		}
	}

	SPUBinary.resize( 0x190C0 / 4 ); // hardcode for now

	spu::op_distrib_t OPDistrib;
	{
		OPDistrib = spu::GatherOPDistribution( SPUBinary );
	}	

	set<uint32_t> FnCallTargets;
	{
		auto& FnCalls = OPDistrib["brsl"];

		for_each( FnCalls.begin(), FnCalls.end(), [&]
		( uint32_t op )
		{
			const SPU_OP_COMPONENTS OC = spu_decode_op_components( SPUBinary[op] );

			FnCallTargets.insert( elf::VirtualBaseAddr(SPU0) + (4 * op) + (OC.IMM << 2) );
		} );
	}

	set<uint32_t> SHUFMasks;
	{
		for ( size_t i = 0; i < SPUBinary.size(); i += 4 )
		{
			if ( PossibleShufbMask(&SPUBinary[i]))
			{
				SHUFMasks.insert( elf::VirtualBaseAddr(SPU0) + (4 * i) );
			}
		}
	}

	vector<uint32_t> LS(0x40000/4);
	elf::spu::LoadImage( (uint8_t*)&LS[0], SPU0 );

	set<uint32_t> Ctors;
	std::vector<uint32_t> FnCalls;
	set<uint32_t> Text;
	{
		for ( size_t i = 0; i < LS.size(); i += 4 )
		{
			if ( PossibleCtorDtorList( i, LS, FnCalls ) )
			{
				Ctors.insert( (4 * i) );
			}

			if ( PossibleString( i, LS ) )
			{
				Text.insert( (4 * i) );
			}
		}
	}	

	//FnCallTargets.insert( FnCalls.begin(), FnCalls.end() );
	
	auto FnRanges = spu::BuildInitialBlocks( 
		SPUBinary, OPDistrib, elf::VirtualBaseAddr(SPU0), 
		EntryIndex, FnCalls );

	OPFlags = spu::BuildOPFlags( SPUBinary, OPDistrib );

	

	std::ofstream bbdump("d");

// 	for ( size_t i = 0; i < SPUBinary.size(); ++i )
// 	{
// 		bbdump << spu_decode_op_components( SPUBinary[i] );
// 	}

	for_each( FnCalls.begin(), FnCalls.end(), [&]
	( uint32_t op )
	{
		const SPU_OP_COMPONENTS OC = spu_decode_op_components( SPUBinary[op] );

		FnCallTargets.insert( elf::VirtualBaseAddr(SPU0) + (4 * op) + (OC.IMM << 2) );
	} );

	spu::MakeSPUSrcFile( SPUBinary, FnRanges, OPFlags, 0, 
		elf::VirtualBaseAddr(SPU0), elf::EntryPointIndex(SPU0)*4 );

	//ConsoleDisplay( SPUBinary, [](uint32_t i){ cout << hex << setw(8) << i << endl;} );
	
	return 0;
}
