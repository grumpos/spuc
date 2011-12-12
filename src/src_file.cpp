#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iterator>

#include "src_file.h"
#include "spu_pseudo.h"
#include "spu_idb.h"

using namespace std;


static string MakeFnName( size_t BaseAddr, size_t EntryOffset )
{
	ostringstream oss;
	oss << "sub_" << hex << (BaseAddr + EntryOffset);
	return oss.str();
}

// static string MakeFnSignature( size_t BaseAddr, size_t EntryOffset )
// {
// 	return string("void") + MakeFnName(BaseAddr, EntryOffset) + string("()");
// }

static string MakeLabel( size_t BaseAddr, size_t EntryOffset )
{
	ostringstream oss;
	oss << "loc_" << hex << (BaseAddr + EntryOffset) << ":";
	return oss.str();
}

namespace spu
{
	typedef pair<size_t, size_t> range_t;

	vector<uint32_t> BytecodeFromRanges( const vector<uint32_t>& Binary, const vector<range_t>& FnRanges )
	{
		vector<uint32_t> NewBytecode;

		for_each( FnRanges.cbegin(), FnRanges.cend(),
			[Binary, &NewBytecode]( range_t r )
		{
			NewBytecode.insert( NewBytecode.end(), Binary.cbegin() + r.first, Binary.cbegin() + r.second );
			//NewBytecode.insert( NewBytecode.end(), &Binary[r.first], &Binary[r.second] );
		});

		return NewBytecode;
	}

	vector<uint32_t> IPFromRanges( const vector<range_t>& FnRanges )
	{
		vector<uint32_t> NewIPList;

		for_each( FnRanges.cbegin(), FnRanges.cend(),
			[&NewIPList]( range_t r )
		{
			NewIPList.reserve( NewIPList.size() + r.second - r.first );

			for ( uint32_t i = r.first; i != r.second; ++i )
			{
				NewIPList.push_back(i);
			}
		});

		return NewIPList;
	}

	void MakeSPUSrcFile( const vector<uint32_t>& Binary, const vector<vector<range_t>>& FnRanges,
		const vector<uint64_t>& OPFlags,
		size_t SPUOffset, size_t VirtualBase, size_t EntryAddr )
	{		
		vector<vector<uint32_t>> Bytecodes;
		{
			transform( FnRanges.cbegin(), FnRanges.cend(), 
				back_inserter( Bytecodes ),
				[&Bytecodes, Binary]( const vector<range_t>& FnRange ) -> vector<uint32_t>
			{
				return BytecodeFromRanges( Binary, FnRange );
			});
		}

		vector<vector<uint32_t>> IPs;
		{
			transform( FnRanges.cbegin(), FnRanges.cend(), 
				back_inserter( IPs ),
				[&Bytecodes, Binary]( const vector<range_t>& FnRange ) -> vector<uint32_t>
			{
				return IPFromRanges( FnRange );
			});
		}

		vector<string> FunctionNames;
		{
			for_each( FnRanges.cbegin(), FnRanges.cend(),
				[&FunctionNames, VirtualBase]( vector<range_t> FnRange )
			{
				FunctionNames.push_back( MakeFnName( VirtualBase, FnRange[0].first * 4 ) );
			});
		}

		vector<string> FunctionSignatures;
		{
			transform( FunctionNames.cbegin(), FunctionNames.cend(),
				back_inserter(FunctionSignatures),
				[]( const string& FnName ) -> string
			{
				return string("static void ") + FnName + string("()");
			});
		}

		// Create the source file
		ostringstream oss;
		oss << "spu_" << hex << SPUOffset << ".cpp";

		ofstream off(oss.str().c_str());

		if ( !off.is_open() )
			return;

		// Includes
		off << "#include \"spu_common.h\"" << endl;
		//off << "#include \"../../../spu_intrin/src/spu_internals_x86.h\"" << endl;

		off << endl << endl;

		// Function declarations

		copy( FunctionSignatures.cbegin(), FunctionSignatures.cend(), ostream_iterator<string>(off, ";\n") );

		off << endl << endl;

		// Function address -> function pointer map
		off << "static const std::pair<size_t, void(*)()> VTABLEArray[] =" << endl;
		off << "{" << endl;

		transform( FunctionNames.cbegin(), FunctionNames.cend(), FnRanges.cbegin(),
			ostream_iterator<string>(off, "\n"),
			[&off, VirtualBase]( const string& FnName, vector<range_t> FnRange ) -> string
		{
			ostringstream oss;

			oss << "\t";

			oss << "std::make_pair( 0x" << hex << (VirtualBase + FnRange[0].first * 4) << dec;
			oss << ", ";
			oss << FnName;

			oss << " ),";

			return oss.str();
		});

		off << "};" << endl << endl;

		off << "static std::map<size_t, void(*)()> VTABLE( VTABLEArray, VTABLEArray + _countof(VTABLEArray) );";

		off << endl << endl;

		auto FnSigIter = FunctionSignatures.cbegin();
		auto BytecodeIter = Bytecodes.cbegin();
		auto IPIter = IPs.cbegin();


		for ( ; FnSigIter != FunctionSignatures.cend(); ++FnSigIter, ++BytecodeIter, ++IPIter )
		{
			off << *FnSigIter << endl;

			off << "{" << endl;

			for ( size_t k = 0; k != (*BytecodeIter).size(); ++k )
			{
				const size_t IP = VirtualBase + ((*IPIter)[k] * 4);

				if ( OPFlags[(*IPIter)[k]] & SPU_IS_BRANCH_TARGET )
				{
					off << MakeLabel( VirtualBase, (*IPIter)[k] * 4 ) << endl;
				}

				if ( OPFlags[(*IPIter)[k]] & SPU_IS_ASSIGNMENT )
				{
					const SPU_OP_COMPONENTS OPC = spu_decode_op_components( (*BytecodeIter)[k] );

					off << "\tGPR(" << (uint16_t)OPC.RT << ") = GPR(" << (uint16_t)OPC.RA << ");" << endl;
				}
				else
				{
					off << "\t" << spu_make_pseudo((SPU_INSTRUCTION&)((*BytecodeIter)[k]), IP) << endl;
				}				
			}

			off << "}" << endl << endl;
		}

		off << "#define ENTRY " << MakeFnName(VirtualBase, EntryAddr) << endl << endl;

		off << "#include \"spu_entry.h\"" << endl << endl;
	}
}