//#include <iostream>
//#include <string>
//#include <algorithm>
//#include <sstream>
//#include <fstream>
//#include <cassert>
//#include <vector>
//#include <regex>
//#include <string>
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//
//#include "spu_emu.h"
//#include "spu_unittest.h"
////
//#include "spu_internals_x86.h"

//uint32_t spu_assemble( const std::string& istr_text_raw );
//std::string spu_disassemble( uint32_t instr_raw );

//template <class FieldT>
//void print_GPR( SPU_t* SPU, size_t offset, size_t count, std::ostream& out = std::cout )
//{
//	if ( offset + count > 128 )
//	{
//		out << "print_GPR error: offset + count greater than 127" << std::endl;
//		return;
//	}
//
//	const size_t FieldSize = sizeof(FieldT);
//	const size_t FieldCount = sizeof(GPR_t) / FieldSize;
//	//const char SignedDigitMaxCntLTB[] = { 0, 4, 6, 0, 11, 0, 0, 0, 21 };
//
//	for ( size_t i = offset; i < offset + count; ++i )
//	{
//		out << "GPR";
//		out.width( 3 );
//		out.fill( '0' );
//		out << i << ": [ ";
//		out.fill( ' ' );
//
//		const FieldT* fp = (FieldT*)&SPU->GPR[i] + FieldCount - 1;
//		for ( int j = 0; j < FieldCount; ++j, --fp ) 
//		{
//			out.width( std::numeric_limits<FieldT>::max_digits10 );
//
//			out << *fp;
//
//			if ( 0 != j )
//				out << ", ";
//		}
//
//		out << " ]" << std::endl;
//	}	
//}
//
//
//std::string XMMToHexString( __m128 xmm )
//{
//	std::string res;
//	res += "[ ";
//	for ( ptrdiff_t j = 15; j >= 0; --j ) 
//	{
//		res += "0123456789ABCDEF"[0xF & (xmm.m128_u8[j] >> 4)];
//		res += "0123456789ABCDEF"[0xF & xmm.m128_u8[j]];
//		res += " ";
//	}
//	res += "]";
//
//	return res;
//}
//
//void print_GPR_hex( SPU_t* SPU, size_t offset, size_t count, std::ostream& dump = std::cout )
//{
//	const size_t offset_end = offset + count;
//
//	std::ostringstream out;
//
//	if ( offset_end <= 128 )
//	{
//		for ( size_t i = offset; i < offset_end; ++i )
//		{
//			out << "GPR";
//			out.width( 3 );
//			out.fill( '0' );
//			out << i << ": ";
//			
//			out << XMMToHexString( SPU->GPR[i] );
//			
//			out << std::endl;
//		}
//	}
//	else
//	{
//		out << "print_GPR error: offset + count greater than 127" << std::endl;
//	}		
//
//	dump << out.str();
//}


//struct FileWatch
//{
//	HANDLE hFile;
//	FILETIME LastWriteTime;
//
//	FileWatch( TCHAR* path )
//	{
//		hFile = CreateFile( path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr );
//		GetFileTime( hFile, nullptr, nullptr, &LastWriteTime );
//	}
//
//	~FileWatch()
//	{
//		CloseHandle(hFile);
//	}
//
//	bool IsDirty()
//	{
//		FILETIME Temp;
//		GetFileTime( hFile, nullptr, nullptr, &Temp );
//		LONG res = CompareFileTime( &LastWriteTime, &Temp );
//		LastWriteTime = Temp;
//		return res == -1;
//	}
//};

/*
template<class FN1_t, class FN2_t>
void testXMM( FN1_t fn1, FN2_t fn2 )
{
	const __m128 testArray[] = 
	{ 
		_mm_castsi128_ps( _mm_set1_epi32(-16)),
		_mm_castsi128_ps( _mm_set1_epi32(-1)),
		_mm_castsi128_ps( _mm_set1_epi32(0)),
		_mm_castsi128_ps( _mm_set1_epi32(1)),
		_mm_castsi128_ps( _mm_set1_epi32(15)),
	};

	std::string name = "addx";
	std::for_each(testArray, testArray + 5, 
		[&](__m128 RA)
	{		
		std::cout << std::endl;
		std::for_each(testArray, testArray + 5, 
			[&, RA](__m128 RB)
		{
			std::cout << printXMMHex( RA );
			std::cout << " " << name << std::endl;
			std::cout << printXMMHex( RB );
			std::cout << std::endl << std::string(3*16, '-') << std::endl;
			__m128 RT = fn1(RA, RB);
			__m128 testRT = fn2(RA, RB);
			std::cout << printXMMHex( RT ) << " >>> " << ((0 == memcmp(&RT, &testRT, sizeof(__m128))) ? "PASS" : "FAIL");
			std::cout << std::endl << std::endl;
		});
	});
}*/

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
#include "raw.h"
#include "elf_helper.h"
#include "basic_blocks.h"
#include "src_file.h"
#include "spu_pseudo.h"

const char* ppc_decode_mnem( uint32_t i );

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


int main( int /*argc*/, char** /*argv*/ )
{
	vector<uint8_t> ELFFile;
	{
		//memmap_t* ELFFileMapped = open( "D:\\PS3\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf" );
		memmap_t* ELFFileMapped = open( "D:\\Downloads\\fail0verflow_ps3tools_win32_\\355\\update_files\\CORE_OS_PACKAGE\\lv1ldr.elf" );

		ELFFile.resize( size(ELFFileMapped) );

		copy( (uint8_t*)begin(ELFFileMapped), (uint8_t*)end(ELFFileMapped), ELFFile.begin() );

		close(ELFFileMapped);
	}
	{
		if ( ELFFile.empty() )
		{
			return 1;
		}
	}

	

	//vector<string> tbl = StringTableA16( (const char*)(ELFFile.data() + 0xD0FA8), (const char*)(ELFFile.data() + 0xD3460) );

	//elf::HeadersToSystemEndian( ELFFile.data() );

	//vector<uint32_t> PPUBinary = elf::spu::LoadExecutable( ELFFile.data() );

	//auto XSections = elf::PPUExecutables( ELFFile.data() );

	//ofstream off("PPU bindump.txt");

	//for ( size_t i = 0x200/4; i != 0x230/4; ++i )//elf::EntryPointIndex( ELFFile.data() )
	//{
	//	auto z = _byteswap_ulong(((uint32_t*)ELFFile.data())[i]);
	//	off << string(ppc_decode_mnem( z )) << endl;
	//}

	/*vector<vector<string>> code;
	code.resize(XSections.size());*/



	/*transform( XSections.cbegin(), XSections.cend(), code.begin(), 
		[]( const vector<uint32_t>& raws ) -> vector<string>
	{
		vector<string> InstrText;
		InstrText.resize(raws.size());

		for ( size_t i = 0; i != raws.size(); ++i )
		{
			InstrText[i] = ppc_decode_mnem(_byteswap_ulong(raws[i]));
		}

		return InstrText;
	} );*/

	//return 0;

	vector<size_t> SPUELFOffsets;
	{
		SPUELFOffsets = elf::EnumEmbeddedSPUOffsets( ELFFile );
	}	

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

	spu::op_distrib_t Distrib;
	{
		Distrib = spu::GatherOPDistribution( SPUBinary );
	}
	
	auto FnRanges = spu::BuildInitialBlocks( SPUBinary, Distrib, elf::VirtualBaseAddr(SPU0), EntryIndex );

	spu::MakeSPUSrcFile( SPUBinary, FnRanges, 0, 
		elf::VirtualBaseAddr(SPU0), elf::EntryPointIndex(SPU0)*4 );

	/*uint8_t LS[0x40000];
	elf::spu::LoadImage( LS, SPU0 );

	ofstream off("sad", ios::out | ios::binary);
	off.write( (const char*)LS, 0x40000 );*/

	

	
	/*
	__m128i asdasd = _mm_set_epi32( 0xFF, 0xFF00, 0xFF0000, 0xFF000000 );
	volatile auto xx = si_orx((__m128&)asdasd);*/
	/*std::ifstream fin("temp_ppc_ilist_raw.txt");
	std::ofstream fout("ppc_ilist.h");

	if (fin.is_open() && fout.is_open())
	{
		std::string line;
		
		while (std::getline(fin, line))
		{
			std::istringstream is(line);

			std::string ifmt, op, xop, mode, page, mnem;

			is >> ifmt >> op;

			if ( ifmt != "D" && ifmt != "B" && ifmt != "SC" && ifmt != "I" && ifmt != "M" )
			{
				is >> xop;
			}
			else
			{
				xop = "-1";
			}

			is >> mode >> page >> mnem;

			if ( !page.empty() && isalpha(page[0]) )
			{
				mnem = page;
				page = mode;
				mode.clear();
}

fout << "DEFINST( \"" << mnem.substr( 0, mnem.find('[')) << "\",\t IFORM_" 
	<< ifmt << ",\t " << op << ", " << xop << " )" << std::endl;
		}
	}*/

	/*testXMM(si_mpy, [](__m128 RA, __m128 RB) -> __m128
	{
		__m128 RT;
		for ( int i = 0; i < 4; ++i )
		{
			RT.m128_i32[i] = (int32_t)(int16_t)RA.m128_i32[i] * (int32_t)(int16_t)RB.m128_i32[i];
		}
		return RT;
	});

	return 0;*/

	

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

//void ClearScreen()
//{
//	HANDLE                     hStdOut;
//	CONSOLE_SCREEN_BUFFER_INFO csbi;
//	DWORD                      count;
//	DWORD                      cellCount;
//	COORD                      homeCoords = { 0, 0 };
//
//	hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
//	if (hStdOut == INVALID_HANDLE_VALUE) return;
//
//	/* Get the number of cells in the current buffer */
//	if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
//	cellCount = csbi.dwSize.X *csbi.dwSize.Y;
//
//	/* Fill the entire buffer with spaces */
//	if (!FillConsoleOutputCharacter(
//		hStdOut,
//		(TCHAR) ' ',
//		cellCount,
//		homeCoords,
//		&count
//		)) return;
//
//	/* Fill the entire buffer with the current colors and attributes */
//	if (!FillConsoleOutputAttribute(
//		hStdOut,
//		csbi.wAttributes,
//		cellCount,
//		homeCoords,
//		&count
//		)) return;
//
//	/* Move the cursor home */
//	SetConsoleCursorPosition( hStdOut, homeCoords );
//}