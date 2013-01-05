#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>

#define BE16(h) _byteswap_ushort(h)
#define BE32(w) _byteswap_ulong(w)
#define BE64(d) _byteswap_uint64(d)

void ReverseBytesInRange( void* data, const char* range_map );

#endif



