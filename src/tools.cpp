#include <cstdint>


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