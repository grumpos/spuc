#include <cstdint>
#include <string>
#include <intrin.h>

void ReverseBytesInRange( void* data, const char* range_map )
{
	uint8_t* raw = (uint8_t*)data;

	while ( *range_map )
	{
		uint8_t*  first = raw;
		uint8_t*  last = raw + *range_map - 1;

		while ( first < last )
		{
			uint8_t tmp = *first;
			*first = *last;
			*last = tmp;
			++first;
			--last;			
		}

		raw += *range_map;
		++range_map;
	}	
}

void BitvecSet( void* bits, size_t bitno )
{
	((uint8_t*)bits)[bitno/8] |= ((uint8_t)0x80 >> (bitno % 8));
}

void BitvecUnset( void* bits, size_t bitno )
{
	((uint8_t*)bits)[bitno/8] &= ~((uint8_t)0x80 >> (bitno % 8));
}

void BitvecFlip( void* bits, size_t bitno )
{
	((uint8_t*)bits)[bitno/8] ^= ((uint8_t)0x80 >> (bitno % 8));
}

bool BitvecTest( void* bits, size_t bitno )
{
	return 0 != (((uint8_t*)bits)[bitno/8] & ((uint8_t)0x80 >> (bitno % 8)));
}


template<class Container, class Formatter> 
void ConsoleDisplay( Container C, Formatter F )
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;



	size_t offset = 0;
	bool input_dirty = true;
	bool key_lock = false;

	for (;;)
	{
		if ( !key_lock && GetAsyncKeyState(VK_ESCAPE) ) 
		{
			break;
		}
		if ( !key_lock && GetAsyncKeyState(VK_UP) && (offset != 0) ) 
		{
			--offset;
			input_dirty = true;
			key_lock = true;
		}
		if ( !key_lock && GetAsyncKeyState(VK_DOWN) && (offset != (C.size() - 1)) ) 
		{
			++offset;
			input_dirty = true;
			key_lock = true;
		}
		if ( !GetAsyncKeyState(VK_UP) && !GetAsyncKeyState(VK_DOWN) )
		{
			key_lock = false;
		}
		if ( false == input_dirty )
		{
			continue;
		}

		/* Fill the entire buffer with spaces */
		if (!FillConsoleOutputCharacter(
			hStdOut,
			(TCHAR) ' ',
			cellCount,
			homeCoords,
			&count
			)) return;

		/* Fill the entire buffer with the current colors and attributes */
		if (!FillConsoleOutputAttribute(
			hStdOut,
			csbi.wAttributes,
			cellCount,
			homeCoords,
			&count
			)) return;

		/* Move the cursor home */
		SetConsoleCursorPosition( hStdOut, homeCoords );

		for_each( C.begin() + offset, C.begin() + offset + 20, F );

		input_dirty = false;
	}
}
//
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
//
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


std::string XMMToHexString( __m128 xmm )
{
	std::string res;
	res += "[ ";
	for ( ptrdiff_t j = 15; j >= 0; --j ) 
	{
		res += "0123456789ABCDEF"[0xF & (xmm.m128_u8[j] >> 4)];
		res += "0123456789ABCDEF"[0xF & xmm.m128_u8[j]];
		res += " ";
	}
	res += "]";

	return res;
}/*

void print_GPR_hex( SPU_t* SPU, size_t offset, size_t count, std::ostream& dump = std::cout )
{
	const size_t offset_end = offset + count;

	std::ostringstream out;

	if ( offset_end <= 128 )
	{
		for ( size_t i = offset; i < offset_end; ++i )
		{
			out << "GPR";
			out.width( 3 );
			out.fill( '0' );
			out << i << ": ";

			out << XMMToHexString( SPU->GPR[i] );

			out << std::endl;
		}
	}
	else
	{
		out << "print_GPR error: offset + count greater than 127" << std::endl;
	}		

	dump << out.str();
}*/