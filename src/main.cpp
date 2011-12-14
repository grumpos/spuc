//
//struct node_t
//{
//	int data;
//	node_t* next;
//};
//
//void _slist_append( node_t* node, int val )
//{
//	node_t* n = new node_t;
//	n->next = nullptr;
//	n->data = val;
//	node->next = n;
//}
//
//node_t*  _slist_prepend( node_t* node, int val )
//{
//	node_t* n = new node_t;
//	n->next = node;
//	n->data = val;
//	return n;
//}
//
//void slist_insert( node_t** head, int val )
//{
//	*head = _slist_prepend( *head, val );
//}
//
//void slist_insert_sorted( node_t** head, int val )
//{
//	if ( !*head || val < (*head)->data )
//	{
//		*head = _slist_prepend( *head, val );
//	}
//	else if ( !(*head)->next )
//	{
//		if ( (*head)->data < val )
//		{
//			_slist_append( (*head), val );
//		}
//		else
//		{
//			*head = _slist_prepend( *head, val );	
//		}
//	}
//	else
//	{
//		node_t* n = *head;
//		while ( n->next && n->next->data < val )
//		{
//			n = n->next;
//		}
//
//		n->next = _slist_prepend( n->next, val );
//	}	
//}
//
//void slist_destroy( node_t** head )
//{	
//	while( *head )
//	{
//		node_t* n = *head;
//		*head = (*head)->next;
//		delete n;
//		n = 0;		
//	}
//}
//
//void slist_delete( node_t** head, int val )
//{
//	if ( !*head )
//		return;
//
//	if ( (*head)->data == val )
//	{
//		node_t* n = *head;
//		*head = (*head)->next;
//		delete n;			
//		return;
//	}
//
//	node_t* h = *head;
//	while ( h->next && h->next->data != val )
//	{
//		h = h->next;
//	}
//
//	node_t* n = h->next;
//	h->next = n->next;
//	delete n;
//}
//
//void slist_print( node_t* head )
//{
//	while ( head )
//	{
//		std::cout << head->data << ", ";
//		head = head->next;
//	}
//}
//
//void srev( char* begin, char* end )
//{
//	--end;
//
//	while ( begin < end )
//	{
//		char c = *begin;
//		*begin = *end;
//		*end = c;
//		++begin;
//		--end;
//	}
//}
//
//void wrev( char* begin, char* end )
//{
//	srev( begin, end );
//	char* wb = begin;
//	char* we = begin;
//
//	do
//	{
//		while ( wb != end && !isalpha(*wb) )
//			++wb;
//		we = wb;
//		while ( we != end && isalpha(*we) )
//			++we;
//		srev( wb, we );
//		wb = we;
//	} while ( we != end );
//}
//
//void remw( char* text )
//{
//	char* getp = text;
//	char* putp = text;
//
//	while ( *getp )
//	{
//		while ( isspace(*getp) )
//			++getp;
//		*putp = *getp;
//		++putp;
//		++getp;
//	}
//	*putp = '\0';
//}
//
//void remdup( char* text )
//{
//	char* getp = text;
//	char* putp = text;
//	
//	while ( *getp )
//	{		
//		*putp = *getp;
//		char last = *getp;
//		while ( last == *getp )
//			++getp;
//		++putp;
//		//++getp;
//	}
//	*putp = '\0';
//}
//
//char ffu( const char* str )
//{
//	char c = *str;
//	const char* p = str;
//
//	while ( *str )
//	{
//		while ( *str && *p != *str )
//			++str;
//
//		if ( !*str )
//			return *p;
//
//		char c = *p;
//
//		while ( c == *p )
//			++p;
//
//		str = p;
//
//		++str;
//	}
//
//	return *p;
//}

#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
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

//const char* ppc_decode_mnem( uint32_t i );

//#include "spu_internals_x86.h"
//#include "spu_unittest.h"

using namespace std;

//vector<string> StringTableA16( const char* begin, const char* end )
//{
//	vector<string> Res;
//
//	const char* b = begin;
//
//	while ( b != end )
//	{
//		if ( *b < 0x20 )
//		{
//			break;
//		}
//
//		string NewString(b);		
//
//		Res.push_back(NewString);
//
//		b += NewString.size();
//
//		while ( !*b )
//			++b;
//	}	
//
//	return Res;
//
//}

//#include <regex>
//#include <algorithm>
//#include <Windows.h>

//map<string, vector<string>> Calls;
//
//void f(const string& FnName, size_t IndentDepth, ostream& out)
//{
//	auto& FnList = Calls[FnName];
//
//	const string Indent(IndentDepth, ' ');
//
//	for ( size_t i = 0; i < FnList.size(); ++i )
//	{
//		out << Indent << FnList[i] << endl;
//		if ( FnList[i] != FnName ) 
//			f( FnList[i], IndentDepth + 1, out );
//	}
//}

template<class Container, class Formatter> 
void ConsoleDisplay( Container C, Formatter F );

int main( int /*argc*/, char** /*argv*/ )
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
		//memmap_t* ELFFileMapped = mmopen( "D:\\PS3\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf" );//aim_spu_module
		memmap_t* ELFFileMapped = mmopen( "D:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\aim_spu_module.elf" );
		//memmap_t* ELFFileMapped = mmopen( "D:\\cell\\host-win32\\spu\\bin\\a.out" );

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

	vector<size_t> SPUELFOffsets;
	{
		SPUELFOffsets = elf::EnumEmbeddedSPUOffsets( ELFFile );
	}

	for_each( SPUELFOffsets.begin(), SPUELFOffsets.end(),
		[&ELFFile](size_t Offset)
	{
		ostringstream oss;
		oss << "spu" << hex << setw(8) << Offset << ".elf";

		ofstream ofs(oss.str().c_str(), ios::out | ios::binary);
		
		const uint8_t* b = ELFFile.data() + Offset;
		const Elf32_Ehdr* eh = (Elf32_Ehdr*)b;
		const uint8_t* e = b;
		if ( _byteswap_ushort(eh->e_shnum) )
		{
			e = b + _byteswap_ulong(eh->e_shoff) + (_byteswap_ushort(eh->e_shnum) * _byteswap_ushort(eh->e_shentsize));

		}
		else
		{
			Elf32_Phdr* LastPH = (Elf32_Phdr*)(b + _byteswap_ulong(eh->e_phoff));
			LastPH += _byteswap_ushort(eh->e_phnum) - 1;
			e = b + _byteswap_ulong(LastPH->p_offset) + (_byteswap_ulong(LastPH->p_filesz));
		}		

		ofs.write((const char*)b, e-b);
	});

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

	spu::op_distrib_t OPDistrib;
	{
		OPDistrib = spu::GatherOPDistribution( SPUBinary );
	}

	vector<uint64_t> OPFlags;
	{
		OPFlags = spu::BuildOPFlags( SPUBinary, OPDistrib );
	}

	/*vector<spu::basic_block_t> bb;
	{
		size_t lead = 0;
		size_t term = 0;

		for ( size_t i = 0; i < SPUBinary.size(); ++i )
		{
			if ( OPFlags[i] & BB_TERM )
			{
				spu::basic_block_t block = { lead, i + 1 };
				lead = i + 1;
				bb.push_back(block);
				continue;
			}
			else if ( OPFlags[i] & BB_LEAD )
			{
				spu::basic_block_t block = { lead, i };
				lead = i;
				bb.push_back(block);
				continue;
			}
		}
	}*/

	vector<uint32_t> LS(0x40000);
	elf::spu::LoadImage( (uint8_t*)&LS[0], SPU0 );
	
	auto FnRanges = spu::BuildInitialBlocks( LS, OPDistrib, elf::VirtualBaseAddr(SPU0), EntryIndex );

	

	spu::MakeSPUSrcFile( SPUBinary, FnRanges, OPFlags, 0, 
		elf::VirtualBaseAddr(SPU0), elf::EntryPointIndex(SPU0)*4 );

	//ConsoleDisplay( SPUBinary, [](uint32_t i){ cout << hex << setw(8) << i << endl;} );
	

	//void ClearScreen();

	//FileWatch fw(TEXT("spu_commands.spu"));

	//

	//SPU_t mySPU;

	//spu_parse_file( &mySPU, "spu_commands.spu" );

	////return 0;
	//

	//size_t gpr_offset = 0;
	//bool input_dirty = true;

	//for(;;)
	//{
	//	if ( GetAsyncKeyState(VK_ESCAPE) )
	//	{
	//		break;
	//	}

	//	if ( fw.IsDirty() )
	//	{
	//		spu_parse_file( &mySPU, "spu_commands.spu" );
	//	}

	//	if ( input_dirty )
	//	{
	//		ClearScreen();


	//		print_GPR<uint16_t>( &mySPU, gpr_offset, 10 );
	//		std::cout << "---------------------" << std::endl;
	//		print_GPR_hex( &mySPU, gpr_offset, 10 );
	//		std::cout << std::endl;

	//		input_dirty = false;
	//	}

	//	if ( GetAsyncKeyState(VK_UP) && gpr_offset != 0 ) 
	//	{
	//		--gpr_offset;
	//		input_dirty = true;
	//	}
	//	if ( GetAsyncKeyState(VK_DOWN) && gpr_offset != (128 - 11) ) 
	//	{
	//		++gpr_offset;
	//		input_dirty = true;
	//	}
	//}

	return 0;
}
