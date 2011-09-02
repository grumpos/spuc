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

static string MakeFnSignature( size_t BaseAddr, size_t EntryOffset )
{
	return string("void") + MakeFnName(BaseAddr, EntryOffset) + string("()");
}

static string MakeLabel( size_t BaseAddr, size_t EntryOffset )
{
	ostringstream oss;
	oss << "loc_" << hex << (BaseAddr + EntryOffset);
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
			NewBytecode.insert( NewBytecode.end(), &Binary[r.first], &Binary[r.second] );
		});

		return NewBytecode;
	}

	void MakeSPUSrcFile( const vector<uint32_t>& Binary, const vector<vector<range_t>>& FnRanges,
		size_t SPUOffset, size_t VirtualBase, size_t EntryAddr )
	{		
		vector<vector<uint32_t>> Bytecodes;
		{
			transform( FnRanges.cbegin(), FnRanges.cend()-1, 
				inserter( Bytecodes, Bytecodes.end() ),
				[&Bytecodes, Binary]( vector<range_t> FnRange ) -> vector<uint32_t>
			{
				return BytecodeFromRanges( Binary, FnRange );
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
		/*for_each( FunctionSignatures.cbegin(), FunctionSignatures.cend(),
			[&off]( const string& FnSig )
		{
			off << FnSig << ";" << endl;
		});*/

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

		auto i = FunctionSignatures.cbegin();
		auto j = FnRanges.cbegin();

		for ( ; i != FunctionSignatures.cend(); ++i, ++j )
		{
			off << *i << endl;

			off << "{" << endl;

			/*vector<size_t> JumpTargets;
			JumpTargets.resize((*j).second - (*j).first);
			{
				for ( size_t k = (*j).first; k != (*j).second; ++k )
				{
					const string mnem = spu_decode_op_mnemonic(Binary[k]);

					if ( "brnz" == mnem || "brz" == mnem || "brhnz" == mnem || "brhz" == mnem || "br" == mnem )
					{
						SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[k]);

						const size_t target = k + OPComponents.IMM;

						if ( ((*j).first <= target) && (target < (*j).second) )
						{
							JumpTargets[target - (*j).first] = 1;
						}
					}
				}
			}*/

			for ( auto FnIter = Bytecodes.cbegin(); FnIter != Bytecodes.cend(); ++FnIter )
			{
				for ( size_t i = 0; i != (*FnIter).size(); ++i )
				{
					//const size_t IP = VirtualBase + (i * 4);

					off << "\t" << spu_make_pseudo((SPU_INSTRUCTION&)((*FnIter)[i]), 0) << endl;
				}
			}

			/*for ( size_t k = (*j).first; k != (*j).second; ++k )
			{
				if ( 1 == JumpTargets[k - (*j).first] )
				{
					ostringstream oss;

					oss << "loc_" << hex << (VirtualBase + (k) * 4) << ":";

					off << oss.str() << endl;
				}

				const size_t IP = VirtualBase + (k * 4);

				off << "\t" << spu_make_pseudo((SPU_INSTRUCTION&)(Binary[k]), IP) << endl;
			}*/

			off << "}" << endl << endl;
		}

		off << "#define ENTRY " << MakeFnName(VirtualBase, EntryAddr) << endl << endl;

		off << "#include \"spu_entry.h\"" << endl << endl;
	}
}