
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <cstdint>
#include <string>
#include <set>
#include <iterator>
#include <cassert>
#include <iostream>
#include <functional>
#include <deque>

#include "spu_idb.h"
#include "elf_helper.h"
#include "spu_pseudo.h"

#include "basic_blocks.h"

namespace spu
{
	using namespace std;

	typedef pair<size_t, size_t> range_t;

	struct CodeBlock
	{
		vector<string> LOC;
	};

	struct CFlowNode
	{
		CFlowNode* Parent;
		CFlowNode* Branch;
		CFlowNode* Follow;

		CodeBlock Block;

		string BROP;
	};

	void DumpLOC( const vector<string>& Binary )
	{
		for_each( Binary.cbegin(), Binary.cend(),
			[](string OP)
		{
			cout << OP << endl;
		});

		cout << endl;
	}

	bool IsNOP(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "nop" == mnem || "lnop" == mnem;
	}

	bool IsBranch(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"br", /*"brsl",*/ "bra", /*"brasl",*/ "brz", "brnz", "brhz", "brhnz",
			"bi", /*"bisl",*/ /*"bisled",*/ "iret", "biz", "binz", "bihz", "bihnz"
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );
		
		return IsBranch;
	};

	bool IsAnyBranch(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"br", "brsl", "bra", "brasl", "brz", "brnz", "brhz", "brhnz",
			"bi", "bisl", "bisled", "iret", "biz", "binz", "bihz", "bihnz"
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		return IsBranch;
	};
	
	bool IsScopedBrach(uint32_t op)
	{		
		string mnem = spu_decode_op_mnemonic( op );

		const string BranchOPs[] =
		{
			"brz", "brnz", "brhz", "brhnz",
		};

		const bool IsBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		return IsBranch;
	}

	bool IsSTOP(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "stop" == mnem;
	}

	bool IsFnCall(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "brsl" == mnem;
	}

	bool IsReturn(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic(op);

		const string BranchOPs[] =
		{
			"bi", "biz", "binz", "bihz", "bihnz",
		};

		const bool IsIndirectBranch = (BranchOPs + _countof(BranchOPs)) != find( BranchOPs, BranchOPs + _countof(BranchOPs), mnem );

		SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);

		return IsIndirectBranch && 0 == OPComponents.RA;
	}

	bool IsUncondReturn(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic(op);

		const bool IsIndirectBranch = "bi" == mnem;

		SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);

		return IsIndirectBranch && 0 == OPComponents.RA;
	}

	bool IsJump(uint32_t op)
	{
		string mnem = spu_decode_op_mnemonic( op );

		return "br" == mnem;
	}

	bool IsCFlowOP(uint32_t op)
	{
		return IsReturn(op) || IsSTOP(op) || IsFnCall(op) || IsJump(op);
	}	

	static set<size_t> Visited;
	static map<size_t, size_t> FnBnd;

	//CFlowNode* CFlowBuilder( const vector<uint32_t>& Binary, size_t begin, size_t end )
	//{
	//	/*static size_t counter = 5;
	//	if ( 0 == --counter )
	//	{
	//		return nullptr; 
	//	}*/
	//	

	//	assert( begin < Binary.size() );
	//	assert( end <= Binary.size() );

	//	if ( begin == end )
	//	{
	//		return nullptr; 
	//	}

	//	CodeBlock NewBlock;

	//	/*if (  begin != end && !IsCFlowOP(Binary[begin]) )
	//	{
	//		NewBlock.LOC.push_back( spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin++], 0 ) );
	//	}*/
	//	

	//	while ( begin != end && !IsCFlowOP(Binary[begin]) )
	//	{
	//		NewBlock.LOC.push_back( spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin++], 0 ) );
	//		//++begin;
	//	}

	//	if ( IsFnCall(Binary[begin]) )
	//	{
	//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[begin]);

	//		const size_t Target = begin + (int16_t)OPComponents.IMM;

	//		CFlowNode* Node = new CFlowNode;
	//		Node->Parent = nullptr;
	//		Node->Block = NewBlock;
	//		Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
	//		if ( Target < begin )
	//		{
	//			Node->Branch = CFlowBuilder( Binary, Target, begin );
	//		}
	//		else
	//		{
	//			Node->Branch = CFlowBuilder( Binary, Target, FnBnd[Target] > 0 ? FnBnd[Target] : Binary.size() );
	//		}
	//		Node->Follow = CFlowBuilder( Binary, begin + 1, FnBnd[Target] > 0 ? FnBnd[Target] : Binary.size() );

	//		return Node;
	//	}
	//	else if ( IsJump(Binary[begin]) )
	//	{
	//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[begin]);

	//		const size_t Target = begin + (int16_t)OPComponents.IMM;

	//		CFlowNode* Node = new CFlowNode;
	//		Node->Parent = nullptr;
	//		Node->Block = NewBlock;
	//		Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
	//		if ( Target < begin )
	//		{
	//			Node->Branch = CFlowBuilder( Binary, Target, begin );
	//		}
	//		else
	//		{
	//			Node->Branch = CFlowBuilder( Binary, Target, FnBnd[Target] > 0 ? FnBnd[Target] : Binary.size() );
	//		}
	//		Node->Follow = nullptr;

	//		return Node;
	//	}
	//	else if ( IsReturn(Binary[begin]) || IsSTOP(Binary[begin]) )
	//	{
	//		CFlowNode* Node = new CFlowNode;
	//		Node->Parent = nullptr;
	//		Node->Block = NewBlock;
	//		Node->BROP = spu_make_pseudo((SPU_INSTRUCTION&)Binary[begin], 0 );
	//		Node->Branch = nullptr;
	//		Node->Follow = nullptr;

	//		return Node;
	//	}

	//	return nullptr;
	//}

	//ofstream off("calltree.txt");

	//void WalkCFlowTree( CFlowNode* Root, size_t Depth )
	//{
	//	if ( !Root )
	//	{
	//		return;
	//	}

	//	const string Indent( Depth*2, ' ' );

	//	for ( int i = 0; i != Root->Block.LOC.size() ; ++i )
	//	{
	//		off << Indent << Root->Block.LOC[i] << endl;
	//	}

	//	//copy( Root->Block.LOC.cbegin(), Root->Block.LOC.cend(), ostream_iterator<string>(cout, "\n" ) );
	//	off << Indent << Root->BROP << endl;
	//	//system("color 0F");
	//	//cout << Indent << "----------" << endl;

	//	WalkCFlowTree( Root->Branch, Depth + 1 );
	//	WalkCFlowTree( Root->Follow, Depth );
	//}

	template<class T, class U>
	bool contains(const T& Container, U Element)
	{
		return Container.end() != find(Container.begin(), Container.end(), Element);
	}

	vector<basic_block_t> BuildInitialBlocks( const vector<uint32_t>& Binary, op_distrib_t& Distrib, size_t EntryIndex )
	{	
		set<size_t> FnEntryByStaticCall;
		{
			const auto& FunCallInstr = Distrib["brsl"];

			transform( 
				FunCallInstr.cbegin(), FunCallInstr.cend(), 
				std::inserter(FnEntryByStaticCall, FnEntryByStaticCall.end()),
				[Binary](size_t IOffset)->size_t
			{
				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

				return IOffset + OPComponents.IMM;
			});

			// main()
			FnEntryByStaticCall.insert( EntryIndex );
		}

		vector<size_t> ScopeDepth;			
		{
			auto BranchExtent = []( uint32_t BROP, size_t IOffset ) -> range_t
			{
				SPU_OP_COMPONENTS OPComponents = spu_decode_op_components( BROP );

				if ( OPComponents.IMM > 0 )
				{
					return make_pair( IOffset, IOffset + OPComponents.IMM );
				}
				else
				{
					return make_pair( IOffset + OPComponents.IMM, IOffset );
				}
			};

			ScopeDepth.resize( Binary.size() );

			for ( size_t i = 0; i < Binary.size(); ++i )
			{
				if ( IsScopedBrach( Binary[i] ) )
				{
					range_t BrExt = BranchExtent( Binary[i], i );

					for( size_t j = BrExt.first; j != BrExt.second; ++j )
					{
						++ScopeDepth[j];
					}
				}
			}
		}

		set<size_t> FnEntryAfterReturn;
		{
			set<size_t> Returns;
			{
				auto& bi = Distrib["bi"];

				for_each( 
					bi.cbegin(), bi.cend(),
					[&Returns, &ScopeDepth, &Binary]( size_t IOffset )
				{
					if ( 0 == ScopeDepth[IOffset] && IsReturn(Binary[IOffset]) )
					{
						Returns.insert(IOffset);
					}
				});
			}

			transform( 
				Returns.cbegin(), Returns.cend(),
				std::inserter(FnEntryAfterReturn, FnEntryAfterReturn.end()),
				[]( size_t IOffset ) -> size_t
			{
				const bool EvenIP = 0 == (IOffset % 2);
				return IOffset + (EvenIP ? 2 : 1);
			});
		}

		set<size_t> FnEntryAfterJumps;
		{
			set<size_t> Jumps;
			{
				auto& br = Distrib["br"];

				for_each( 
					br.cbegin(), br.cend(),
					[&Jumps, &ScopeDepth, &Binary]( size_t IOffset )
				{
					if ( 0 == ScopeDepth[IOffset] )
					{
						SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(Binary[IOffset]);

						if ( OPComponents.IMM < 0 )
						{
							Jumps.insert(IOffset);
						}
						else
						{
							size_t b = IOffset + 1;
							const size_t e = IOffset + OPComponents.IMM;

							for ( ; b != e; ++b )
							{
								if ( 0 != ScopeDepth[b] )
								{
									break;
								}
							}

							if ( b == e )
							{
								Jumps.insert(IOffset);
							}
						}
					}
				});

				transform( 
					Jumps.cbegin(), Jumps.cend(),
					std::inserter(FnEntryAfterJumps, FnEntryAfterJumps.end()),
					[]( size_t IOffset ) -> size_t
				{
					const bool EvenIP = 0 == (IOffset % 2);
					return IOffset + (EvenIP ? 2 : 1);
				});
			}
		}		

		{			
			auto FnEntries = FnEntryByStaticCall;
			FnEntries.insert( FnEntryAfterReturn.begin(), FnEntryAfterReturn.end() );
			FnEntries.insert( FnEntryAfterJumps.begin(), FnEntryAfterJumps.end() );
			deque<size_t> Entries(FnEntries.begin(), FnEntries.end());

			ofstream off("spu_code_0.intr");
			ofstream ioff("spu_code_0.info");

			for ( size_t i = 1; i < Binary.size(); ++i ) // start from 1, notepad++ indexes lines from 1
			{
				ioff << hex << (0x3000+(i*4)) << endl;

				/*if ( 0 != ScopeDepth[i] )
				{
					off << endl;
					continue;
				}*/
				if ( !Entries.empty() && i == Entries.front() )
				{
					Entries.pop_front();
					off << "\t\t" << spu_make_pseudo((SPU_INSTRUCTION&)(Binary[i]), 0) << endl;
					continue;
				}

				off << spu_make_pseudo((SPU_INSTRUCTION&)(Binary[i]), 0) << endl;
			}

			vector<range_t> FnRanges;

			transform(
				FnEntries.cbegin(), --FnEntries.cend(), ++FnEntries.cbegin(),
				back_inserter( FnRanges ),
				[](size_t b, size_t e) -> range_t
			{
				return make_pair( b, e );
			});

			//CFlowNode* Root = CFlowBuilder( Binary, EntryIndex, Binary.size() );

			//WalkCFlowTree( Root, 0 );

		}

		return vector<basic_block_t>();
	}

	op_distrib_t GatherOPDistribution( const vector<uint32_t>& Binary )
	{
		op_distrib_t Distrib;
		{
			size_t i = 0;

			for_each( Binary.cbegin(), Binary.cend(),
				[&Distrib, &i](uint32_t Instr)
			{		
				Distrib[spu_decode_op_mnemonic(Instr)].push_back(i++);
			});
		}	

		return Distrib;
	}
};