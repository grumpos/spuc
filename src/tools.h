#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>

#define BE16(h) _byteswap_ushort(h)
#define BE32(w) _byteswap_ulong(w)
#define BE64(d) _byteswap_uint64(d)

void ReverseBytesInRange( void* data, const char* range_map );

template<class T>
struct ByteSwapHelper : public std::unary_function<T, void>
{
	std::string LayoutString_;
	ByteSwapHelper( const std::string& LayoutString )
		: LayoutString_(LayoutString)
	{}

	void operator()( argument_type& Data ) const
	{
		ReverseBytesInRange( &Data, LayoutString_.c_str() );
	}
};


#endif



