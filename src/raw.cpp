#define WIN32_LEAN_AND_MEAN
#include <cassert>
#include <Windows.h>
#include <map>
#include <string>
#include <algorithm>
#include "raw.h"

struct memmap_t
{
	void* begin_;
	void* end_;
};

static std::map<std::string, std::pair<size_t, memmap_t*>> MappedFiles;

memmap_t* mmopen( const char* path, const char* openmode )
{
	auto& MemmapRecord = MappedFiles[std::string(path)];

	if ( MemmapRecord.second )
	{
		++MemmapRecord.first;
	}
	else
	{
		DWORD access = 0;
		DWORD share = 0;
		DWORD mapprotect = 0;
		DWORD mapaccess = 0;
		DWORD open = 0;

		const char* mode = openmode;

		while ( *mode )
		{
			switch( *mode )
			{
			case 'r':
				{
					open		= OPEN_EXISTING;
					access		|= GENERIC_READ;
					//share		|= FILE_SHARE_READ;
					mapprotect	|= FILE_MAP_READ;
					mapaccess	|= PAGE_READONLY;
					break;
				}
			case 'w':
				{
					open		= OPEN_ALWAYS;
					access		|= GENERIC_READ | GENERIC_WRITE;
					share		|= FILE_SHARE_WRITE;
					mapprotect	|= FILE_MAP_WRITE;
					mapaccess	|= PAGE_READWRITE;
					break;
				}
			};

			++mode;
		}		

		HANDLE FileHandle = CreateFileA( path, access, share, 0, open, FILE_ATTRIBUTE_NORMAL, 0 );
		HANDLE FileMapHandle = CreateFileMappingA( FileHandle, 0, mapaccess, 0, 0, 0 );		
		LARGE_INTEGER FileSize = {0};
		GetFileSizeEx( FileHandle, &FileSize );
		memmap_t* rawmap_handle = new memmap_t;
		rawmap_handle->begin_	= MapViewOfFile( FileMapHandle, mapprotect, 0, 0, 0 );
		rawmap_handle->end_		= (char*)rawmap_handle->begin_ + FileSize.QuadPart;
		CloseHandle( FileMapHandle );
		CloseHandle( FileHandle );

		MemmapRecord.first = 1;
		MemmapRecord.second = rawmap_handle;
	}

	return MemmapRecord.second;
}

void mmclose(memmap_t* rawmap_handle)
{
	auto rawmap_handle_local = rawmap_handle;
	auto MemmapRecord = std::find_if( MappedFiles.begin(), MappedFiles.end(),
		[rawmap_handle_local]( const std::pair<std::string, std::pair<size_t, memmap_t*>>& rec ) -> bool
	{
		return rec.second.second == rawmap_handle_local;
	});

	if ( MemmapRecord != MappedFiles.end() )
	{
		if ( 1 == MemmapRecord->second.first )
		{
			memmap_t* m = MemmapRecord->second.second;
			UnmapViewOfFile( m->begin_ );
			delete m;
			MappedFiles.erase(MemmapRecord);
		}
		else
		{
			--MemmapRecord->second.first;
		}
	}
}

size_t	mmsize(memmap_t* rm)
{ 
	return (char*)rm->end_ - (char*)rm->begin_; 
}

void*	mmbegin(memmap_t* rm) 
{ 
	return  rm->begin_; 
}

const void*	mmbegin(const memmap_t* rm) 
{ 
	return  rm->begin_; 
}

void*	mmend(memmap_t* rm) 
{ 
	return  rm->end_; 
}

const void*	mmend(const memmap_t* rm) 
{ 
	return  rm->end_; 
}

void		mmcreate(const char* path, size_t size)
{
	HANDLE FileHandle = CreateFileA( path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	LARGE_INTEGER sizeLL;
	sizeLL.QuadPart = size;
	SetFilePointer( FileHandle, sizeLL.LowPart, &sizeLL.HighPart, FILE_BEGIN );
	SetEndOfFile( FileHandle );
	CloseHandle( FileHandle );
}