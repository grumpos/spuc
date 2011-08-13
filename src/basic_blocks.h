#ifndef __BASIC_BLOCK_H__
#define __BASIC_BLOCK_H__

#include <vector>
#include <map>

namespace spu
{
	using namespace std;

	typedef map<string, vector<size_t>> op_distrib_t;


	struct basic_block_t
	{
		size_t begin;
		size_t end;
	};

	struct function_t
	{
		std::vector<basic_block_t> blocks;
	};

	vector<pair<size_t, size_t>> BuildInitialBlocks( 
		vector<uint32_t>& Binary, op_distrib_t& Distrib, size_t VirtualBase, size_t EntryIndex );

	op_distrib_t GatherOPDistribution( const vector<uint32_t>& Binary );
}



#endif