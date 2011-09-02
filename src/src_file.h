#include <vector>
#include <cstdint>


namespace spu 
{
	using namespace std;

	void MakeSPUSrcFile( const vector<uint32_t>& Binary, const vector<vector<pair<size_t, size_t>>>& FnRanges,
		size_t SPUOffset, size_t VirtualBase, size_t EntryAddr );
}