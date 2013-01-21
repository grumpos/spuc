

#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>

struct Streamer
{
	struct DataChunk
	{
		struct FileSource
		{
			std::string Name;
			size_t Offset;
			size_t Length;
		};

		void Load()
		{
			FileHandle = CreateFileA( Source.Name.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0 );
			LARGE_INTEGER FileSize = {0};
			GetFileSizeEx( FileHandle, &FileSize );
			Data.resize( FileSize.QuadPart );
			ZeroMemory( &AsyncInfo, sizeof(OVERLAPPED) );
			BOOL res = ReadFileEx( FileHandle, &Data[0], static_cast<DWORD>(Data.size()), &AsyncInfo, 0 );
			DWORD err = GetLastError();
			//FormatMessageA()
			//while ( !Done());
		}

		size_t Done()
		{
			DWORD BytesDone = 0;
			GetOverlappedResult( FileHandle, &AsyncInfo, &BytesDone, FALSE );
			return (Data.size() * 100) / BytesDone;
		}

		void Stop()
		{
			CancelIo( FileHandle );
		}

		FileSource Source;
		std::vector<uint8_t> Data;
		HANDLE FileHandle;
		OVERLAPPED AsyncInfo;
	};

	void Process()
	{

	}
};


struct tstamp3
{
	uint64_t t0;
	uint64_t* r; 
	tstamp3( uint64_t* result ) 
		: t0(0), r( result )
	{
		QueryPerformanceCounter( (LARGE_INTEGER*)&t0 );
	}
	~tstamp3()
	{
		uint64_t t1;
		QueryPerformanceCounter( (LARGE_INTEGER*)&t1 );
		*r = t1 - t0;
	}
};

#include <list>
#include <algorithm>

class RangePlanner
{
public:
	typedef std::pair<size_t, size_t> range_t;

	RangePlanner( size_t begin, size_t end )
	{
		if ( begin < end )
		{
			RangeList_.push_back(std::make_pair(begin, end));
		}
	}

	void clip_back( range_t& old, const range_t& overlapped )
	{
		old.second = overlapped.first;
	}

	void clip_front( range_t& old, const range_t& overlapped )
	{
		old.first = overlapped.second;
	}

	bool overlaps_back( const range_t& old, const range_t& overlapped ) const
	{
		return (old.first <= overlapped.first) && (overlapped.first < old.second);
	}

	bool overlaps_front( const range_t& old, const range_t& overlapped ) const
	{
		return (old.first < overlapped.second) && (overlapped.second <= old.second);
	}

	void Remove( size_t begin, size_t end )
	{
		if ( begin == end )
		{
			return;
		}

		range_t overlapped = std::make_pair( begin, end );

		auto b = std::find_if( RangeList_.begin(), RangeList_.end(), 
			[&]( const range_t& r) -> bool
		{
			return overlaps_back( r, overlapped );
		});

		if ( b != RangeList_.end() )
		{
			range_t front = std::make_pair( (*b).first, begin );
			if ( front.first != front.second )
			{
				RangeList_.insert( b, front );
			}

			auto e = std::find_if( b, RangeList_.end(), 
				[&]( const range_t& r) -> bool
			{
				return overlaps_front( r, overlapped );
			});

			if ( e != RangeList_.end() )
			{
				range_t back = std::make_pair( end, (*e).second );
				if ( back.first != back.second )
				{
					RangeList_.insert( b, back );				
				}
				++e;
			}

			while ( b != e )
			{
				auto iter = b;
				++b;
				RangeList_.erase(iter);
			}
		}		
	}
private:
	std::list<range_t> RangeList_;
};