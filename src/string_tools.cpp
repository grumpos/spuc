#include "string_tools.h"

using namespace std;

namespace str_tools
{
	template <class T>
	string BitsFromScalar( T val )
	{
		const char* bitrep[] = 
		{
			"0000", "0001", "0010", "0011", 
			"0100", "0101", "0110", "0111", 
			"1000", "1001", "1010", "1011", 
			"1100", "1101", "1110", "1111"
		};

		const size_t nibbles = sizeof(T) * 2;

		string res;

		for ( size_t i = 0; i < nibbles; ++i )
		{
			res += bitrep[(val>>28)&0xF]; 
			val <<= 4;
		}
		return res;
	}
}
