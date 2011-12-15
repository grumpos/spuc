
//
///*
//struct Ticker
//{
//	std::vector<uint64_t> ticks_;
//	uint64_t freq_;
//	uint64_t t0_;
//
//	Ticker() 
//		: freq_(0),
//		t0_(0)
//	{
//		ticks_.reserve(32);
//		QueryPerformanceFrequency((LARGE_INTEGER*)&freq_);
//		QueryPerformanceCounter((LARGE_INTEGER*)&t0_);
//	}
//
//	void Tick()
//	{
//		uint64_t t1 = 0;
//		QueryPerformanceCounter((LARGE_INTEGER*)&t1);
//		ticks_.push_back(t1 - t0_);
//		t0_ = t1;
//	}
//
//	void Reset()
//	{
//		ticks_.clear();
//		QueryPerformanceCounter((LARGE_INTEGER*)&t0_);
//	}
//
//	string Stats()
//	{
//		std::ostringstream oss;
//		std::for_each( ticks_.cbegin(), ticks_.cend(), [&oss](uint64_t t){ oss << t << ", "; });
//		return oss.str();
//	}
//};*/
//
//struct spu_program_t
//{
//	size_t EntryPoint;
//	size_t VirtualBaseAddress;
//	std::vector<uint32_t> Binary;
//};
//
//struct spu_data_t
//{
//	size_t virtual_offset;
//	vector<uint8_t> data;
//};
//
//struct spu_branch_t
//{
//	//uint32_t BranchFrom;
//	int32_t  AdvanceAmount;
//	uint32_t BranchBlockBegin;
//	uint32_t BranchBlockEnd;
//};
//
//struct basic_block_t
//{
//	size_t first, last;
//};
//
//struct spu_function_t
//{
//	vector<basic_block_t> BasicBlocks_;
//};
//
//bool operator<(const spu_branch_t& lhs, const spu_branch_t& rhs)
//{
//	return lhs.BranchBlockBegin < rhs.BranchBlockBegin;
//}
//
//struct spu_info_t
//{
//	std::map<string, vector<size_t>>	heuristics;
//	vector<spu_branch_t>				staticBranches;
//	vector<spu_function_t>				FunctionRanges;
//	std::map<size_t, string>			functionSymbols;
//	std::map<size_t, size_t>			jumps;
//	std::map<size_t, string>			jumpSymbols;
//};
//
//vector<uint32_t> spuGatherStaticCallTargets( const spu_program_t* program, spu_info_t* info );
//
//spu_program_t spuExtractProgram( ElfFile<ELF_32_traits_t>& elfFile )
//{
//	const size_t ELFPD_EXECUTABLE_OFFSET = 0;
//
//	assert( 0 != elfFile.PData_.size() );	
//	assert( 0 != elfFile.ProgramHeaders_.size() );
//	const memrange_t RawData = elfFile.PData_[ELFPD_EXECUTABLE_OFFSET];
//	assert( 0 == RawData.size()%4 );
//
//	spu_program_t result = 
//	{ 
//		elfFile.HeaderLE_.e_entry,
//		elfFile.ProgramHeaders_[ELFPD_EXECUTABLE_OFFSET].p_vaddr, 		
//		vector<uint32_t>( (const uint32_t*)RawData.begin(), (const uint32_t*)RawData.end() )
//	};
//	return result;
//}
//
//spu_data_t spuExtractData( ElfFile<ELF_32_traits_t>& ElfFile )
//{
//	const size_t ELFPD_DATA_OFFSET = 1;
//
//	assert( 0 != ElfFile.PData_.size() );	
//	assert( 0 != ElfFile.ProgramHeaders_.size() );
//	if (ElfFile.PData_.size() == 3)
//	{
//		const memrange_t RawData = ElfFile.PData_[ELFPD_DATA_OFFSET];
//		assert( 0 == RawData.size()%4 );
//
//		spu_data_t result = 
//		{			
//			ElfFile.ProgramHeaders_[ELFPD_DATA_OFFSET].p_vaddr,
//			vector<uint8_t>((const uint8_t*)RawData.begin(), (const uint8_t*)RawData.end())
//		};
//		return result;
//	}
//
//	spu_data_t result;
//	result.virtual_offset = 0;
//	return result;
//}
//
//
//
//
//
//struct pflow_node_t
//{
//	pflow_node_t() : next(nullptr), branch(nullptr) {}
//
//	pflow_node_t* next;
//	pflow_node_t* branch;
//	basic_block_t bb;
//};
//
////vector<uint32_t> spuGatherStaticCallTargets( const spu_program_t* program, spu_info_t* info )
////{
////	auto& program_local = program;
////
////	const auto& FunCallInstr = info->heuristics["brsl"];
////
////	std::set<uint32_t> StaticCallTargets;
////
////	StaticCallTargets.insert( (program->EntryPoint - program->VirtualBaseAddress) / 4 ); // main()
////
////	std::transform( 
////		FunCallInstr.cbegin(), FunCallInstr.cend(), 
////		std::inserter(StaticCallTargets, StaticCallTargets.end()),
////		[program](uint32_t IOffset)->uint32_t
////	{
////		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[IOffset]);
////		return IOffset + (int16_t)OPComponents.IMM;
////	});
////
////	/*std::transform( StaticCallTargets.cbegin(), StaticCallTargets.cend(), std::back_inserter(info->FunctionRanges),
////		[](uint32_t FnBeginOffset)->spu_function_t
////	{
////		const spu_function_t result = { FnBeginOffset, 0 };
////		return result;
////	});
////
////	if ( !info->FunctionRanges.empty() )
////	{
////		std::for_each( info->FunctionRanges.begin(), info->FunctionRanges.end() - 1,
////			[]( spu_function_t& FnRange )
////		{
////			auto Next = (&FnRange + 1);
////			FnRange.end = Next->begin;
////		});
////		info->FunctionRanges.back().end = program->Binary.size();
////	}	*/
////
////	//////////////////////////////////////////////////////////////////////////
////
////
////	//auto IsNOP = [](uint32_t op)->bool
////	//{
////	//	return 1 == (op >> 21) || 513 == (op >> 21);
////	//};
////
////	//auto IsSTOP = [](uint32_t op)->bool
////	//{
////	//	return 0 == (op >> 21) || 320 == (op >> 21);
////	//};
////
////	//auto IsUnconditionalReturn = [](uint32_t op)->bool
////	//{
////	//	if ( 0x1a8 == spu_decode_op_opcode(op) ) // bi
////	//	{
////	//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
////	//		if ( 0 == OPComponents.RA )
////	//		{
////	//			return true;
////	//		}
////	//	}
////	//	return false;
////	//};
////
////	//auto IsConditionalReturn = [](uint32_t op)->bool
////	//{
////	//	/*const uint32_t opcode = op >> 21;
////	//	if ( 296 == opcode || 297 == opcode || 298 == opcode || 299 == opcode || 424 == opcode ) // bi*
////	//	{
////	//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
////	//		if ( 0 == OPComponents.RA )
////	//		{
////	//			return true;
////	//		}
////	//	}*/
////
////	//	/* Indirect conditional jumps to the link register is equivalent to:
////
////	//		if( condition )
////	//			return;
////
////	//		Indirect unconditional jumps to the link register is equivalent to:
////
////	//		return;
////	//	*/
////
////	//	switch(spu_decode_op_opcode(op))
////	//	{
////	//	case 0x128: // biz
////	//	case 0x129: // binz
////	//	case 0x12a: // bihz
////	//	case 0x12b: // bihnz
////	//		{
////	//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
////	//			if ( 0 == OPComponents.RA )
////	//			{
////	//				return true;
////	//			}
////	//		}
////	//	default:
////	//		return false;
////	//	}
////	//};
////
////	//auto GetJumpAmount = [](uint32_t op) -> int32_t
////	//{
////	//	switch(spu_decode_op_opcode(op))
////	//	{
////	//	case 0x100: // brz
////	//	case 0x108: // brnz
////	//	case 0x110: // brhz
////	//	case 0x118: // brhnz
////	//	case 0x190: // br
////	//		{
////	//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(op);
////	//			return (int32_t)OPComponents.IMM;
////	//		}
////	//	default:
////	//		return 0;
////	//	}
////	//};
////
////	auto IsBrach = [](uint32_t op)->bool
////	{
////		//uint32_t opcode = spu_decode_op_opcode(op);
////		//const string BR_I16[] = { 0x100, 0x108, 0x110, 0x118, 0x180, /*0x188,*/ 0x190, /*0x198*/ };
////		//auto a = std::count(BR_I16, BR_I16 + _countof(BR_I16), op >> 23 );
////
////		//const string BR_RR[] = { 0x1a8, /*0x1a9,*/ 0x1aa, /*0x1ab,*/ 0x128, 0x129, 0x12a, 0x12b };
////		//auto b = std::count(BR_RR, BR_RR + _countof(BR_RR), op >> 21 );
////
////		string mnem = spu_decode_op_mnemonic( op );
////
////		const string BR_I16[] = { "br", /*"brsl",*/ "bra", /*"brasl",*/ "brz", "brnz", "brhz", "brhnz" };
////		auto a = std::count(BR_I16, BR_I16 + _countof(BR_I16), mnem );
////
////		const string BR_RR[] = { "bi", /*"bisl",*/ /*"bisled",*/ "iret", "biz", "binz", "bihz", "bihnz" };
////		auto b = std::count(BR_RR, BR_RR + _countof(BR_RR), mnem );
////
////		const string STOP[] = { "stop" };
////		auto c = std::count(STOP, STOP + _countof(STOP), mnem );
////
////		return 0 != (a + b + c);
////	};
////
////	vector<basic_block_t> blocks;
////
////	
////	enum spu_basic_block_type_t
////	{
////		function_end,
////		unconditional_static_jump,
////
////	};
////	
////
////	auto FindBasicBlockInRange = [&blocks, program_local, IsBrach]( size_t begin, size_t end )
////	{
////		while ( begin != end )
////		{
////			basic_block_t NewBlock = FindBasicBlock( begin, end, program_local, IsBrach );
////
////			if ( NewBlock.first != NewBlock.last )
////			{
////				blocks.push_back(NewBlock);
////				const uint32_t LastOp = program_local->Binary[NewBlock.last - 1];
////
////				if ( LastOp == 0 )
////					break;
////
////				begin = NewBlock.last;
////				/*begin += GetJumpAmount(LastOp);
////
////				if ( begin == NewBlock.first )
////				{
////				begin = NewBlock.last;
////				}*/
////			}
////			else
////				break;
////		}
////	};
////
////	//pflow_node_t* p = FindBasicBlock2( &program->Binary[4], &program->Binary[16], program, IsBrach );
////	
////	// skip SPU GUID
////	FindBasicBlockInRange( 66, 193 );
////
////	/*auto MakeFunctionText = [&]( const vector<basic_block_t>& blocks ) -> string
////	{
////		std::ostringstream oss;
////
////		for ( auto i = blocks.cbegin(); i != blocks.cend(); ++i )
////		{
////			for ( auto j = &program->Binary[i->first]; j != &program->Binary[i->last]; ++j )
////			{
////				oss << spu_make_pseudo( (SPU_INSTRUCTION&)(*j), 0 ) << std::endl;
////			}
////			oss << std::endl;
////		}
////
////		return oss.str();
////	};*/
////
////
////	//std::cout << MakeFunctionText( blocks );
////
////	auto& info_local = info;
////
////	vector<basic_block_t> blocks2;
////
////	
////	return vector<uint32_t>( StaticCallTargets.begin(), StaticCallTargets.end() );
////}
//
//void spuGatherStaticBranches( const spu_program_t* program, spu_info_t* info )
//{
//	auto spuGatherStaticBranchesAux = [program, info](const string& branchMnem)
//	{
//		const auto& branches = info->heuristics[branchMnem];
//		std::transform( branches.cbegin(), branches.cend(), std::back_inserter(info->staticBranches),
//			[=](const uint32_t InstructionOffset)->spu_branch_t
//		{
//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[InstructionOffset]);
//			const uint32_t			BranchFrom = InstructionOffset;
//			const uint32_t			BranchBlockBegin	= (OPComponents.IMM > 0) ? BranchFrom : (BranchFrom + (int32_t)OPComponents.IMM);
//			const uint32_t			BranchBlockEnd		= (OPComponents.IMM > 0) ? (BranchFrom + (int32_t)OPComponents.IMM) : (BranchFrom + 1);
//			const spu_branch_t		br = { /*BranchFrom,*/ (int32_t)OPComponents.IMM, BranchBlockBegin, BranchBlockEnd };
//			return br;
//		});
//	};
//
//	spuGatherStaticBranchesAux("brz");
//	spuGatherStaticBranchesAux("brnz");
//	spuGatherStaticBranchesAux("brhz");
//	spuGatherStaticBranchesAux("brhnz");
//
//	std::sort( info->staticBranches.begin(), info->staticBranches.end() );
//
//	auto Overlaps = []( const spu_branch_t& lhs, const spu_branch_t& rhs ) -> bool
//	{
//		//if ( lhs.BranchBlockBegin <= rhs.BranchBlockBegin )
//			return (lhs.BranchBlockBegin <= rhs.BranchBlockBegin) && (rhs.BranchBlockBegin < lhs.BranchBlockEnd);
//		//else
//			//return rhs.BranchBlockEnd < lhs.BranchBlockBegin;
//	};
//
//	/*decltype(info->staticBranches) BrIfVector;
//	auto first1 = info->staticBranches.begin();
//	auto first2 = info->staticBranches.begin() + 1;
//	auto last = info->staticBranches.end() - 1;
//
//	do
//	{
//	if ( (first1->AdvanceAmount > 0) && !Overlaps( *first1, *first2 ) )
//	{
//	BrIfVector.push_back(*first1);
//	}
//	++first1;
//	++first2;
//	} while ( first1 != last );
//
//	if ( info->staticBranches.back().AdvanceAmount > 0 )
//	{
//	BrIfVector.push_back(*first1);
//	}*/
//
//	/*decltype(info->staticBranches) BrIfComplex;
//	std::set_difference( info->staticBranches.begin(), info->staticBranches.end(), 
//		BrIfVector.begin(), BrIfVector.end(), std::inserter( BrIfComplex, BrIfComplex.end() ) );*/
//}
//
//void spuDumpInstrTextToFile( const string& path, const vector<string>& Prefix, const spu_program_t& program, const vector<string>& Postfix )
//{
//	assert( Prefix.size() == program.Binary.size() && program.Binary.size() == Postfix.size() );
//
//	std::ofstream fout_intrin( path );
//	vector<string> FinalText(program.Binary.size(), "");
//	
//	if ( fout_intrin.is_open() )
//	{
//		const ptrdiff_t Count = program.Binary.size();
//		
//		for ( ptrdiff_t i = 0; i < Count; ++i )
//		{
//			FinalText[i] = Prefix[i] + spu_make_pseudo((SPU_INSTRUCTION&)program.Binary[i], program.VirtualBaseAddress + i*4) + Postfix[i];
//		}
//		std::copy( FinalText.cbegin(), FinalText.cend()-1, std::ostream_iterator<string>(fout_intrin, "\n") );
//		fout_intrin << FinalText.back();
//		/*std::transform( program.Binary.cbegin(), program.Binary.cend(), std::ostream_iterator<string>(fout_intrin, "\n"),
//			[&](uint32_t op)->string
//		{
//			string result = Prefix[i] + spu_make_pseudo((SPU_INSTRUCTION&)op, 0) + Postfix[i];
//			++i;
//			return result;			
//		});*/
//	}
//}
//
//void spuBuildFunctionSymbols( spu_info_t* info, const vector<uint32_t>& fn_begins )
//{
//	std::ostringstream oss;
//	std::for_each( fn_begins.cbegin(), fn_begins.cend(), 
//		[info](uint32_t off)
//	{
//		std::ostringstream oss;
//		oss << "sub" << std::hex << std::setw(8) << std::setfill('0') << off*4;
//		info->functionSymbols[off] = oss.str();
//	} );
//}
//
//void spuGatherJumps( const spu_program_t* program, spu_info_t* info )
//{
//	const auto& JumpsRel = info->heuristics["br"];
//
//	std::transform( JumpsRel.cbegin(), JumpsRel.cend(), std::inserter(info->jumps, info->jumps.end()),
//		[=](const uint32_t InstructionOffset)->std::pair<uint32_t, uint32_t>
//	{
//		const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program->Binary[InstructionOffset]);
//		const uint32_t			JumpFrom	 = InstructionOffset;
//		const uint32_t			JumpTo		 = InstructionOffset + (int32_t)OPComponents.IMM;		
//		return std::make_pair( JumpFrom, JumpTo );
//	});
//
//	/*std::set<uint32_t> JumpTargets;
//	std::transform( info->jumps.cbegin(), info->jumps.cend(), std::inserter(JumpTargets, JumpTargets.end()),
//		[](const std::pair<uint32_t, uint32_t>& it)->uint32_t
//	{
//		return it.second;
//	});
//
//	std::transform( JumpTargets.begin(), JumpTargets.end(), std::inserter(info->jumpSymbols, info->jumpSymbols.end()),
//		[](uint32_t JumpTo)->std::pair<uint32_t, string>
//	{
//		std::ostringstream oss;
//		oss << "LB" << std::hex << std::setw(8) << std::setfill('0') << (JumpTo*4);
//		return std::make_pair( JumpTo, oss.str() );
//	});	*/
//}
//
//
//void spuGatherLoads( const spu_program_t* program, spu_info_t* info )
//{	
//	auto GatherAbsAddresses = [program, info]( const string& mnem, std::set<uint32_t>& Addresses )
//	{
//		const auto& AbsMemOPs = info->heuristics[mnem];
//		auto program_local = program;
//
//		std::transform( 
//			AbsMemOPs.begin(), AbsMemOPs.end(), 
//			std::inserter(Addresses, Addresses.end()), 
//			[program_local]( uint32_t Offset )->uint32_t
//		{
//			const uint32_t LSLR = 0x3ffff & -16;
//
//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program_local->Binary[Offset]);
//
//			return ((uint32_t)OPComponents.IMM << 2) & LSLR;
//		});
//	};
//
//	auto GatherRelAddresses = [program, info]( const string& mnem, std::set<uint32_t>& Addresses )
//	{
//		const auto& RelMemOPs = info->heuristics[mnem];
//		auto program_local = program;
//
//		std::transform( RelMemOPs.begin(), RelMemOPs.end(), std::inserter(Addresses, Addresses.end()), 
//			[program_local]( uint32_t Offset )->uint32_t
//		{
//			const uint32_t LSLR = 0x3ffff & -16;
//
//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program_local->Binary[Offset]);
//
//			return (program_local->VirtualBaseAddress + (Offset*4) + ((int32_t)OPComponents.IMM << 2)) & LSLR;
//		});
//	};
//
//	std::set<uint32_t> AbsLoadTargets;
//	std::set<uint32_t> RelLoadTargets;
//	std::set<uint32_t> AbsStoreTargets;
//	std::set<uint32_t> RelStoreTargets;
//
//	GatherAbsAddresses( "lqa", AbsLoadTargets );
//	GatherRelAddresses( "lqr", RelLoadTargets );
//	GatherAbsAddresses( "stqa", AbsStoreTargets );
//	GatherRelAddresses( "stqr", RelStoreTargets );
//}
//
//void PrintInstrAdresses( uint32_t BaseAddress, vector<string>& instrInfoText )
//{
//	std::for_each( instrInfoText.begin(), instrInfoText.end(), 
//		[&BaseAddress]( string& s )
//	{
//		std::ostringstream oss;
//		oss << std::hex << std::setw(8) << std::setfill('0') << BaseAddress << " ";
//		BaseAddress += 4;
//		s = oss.str();
//	} );
//}
//
//	
//
//
//	
//
//#define ALL( _Container ) (_Container).begin(), (_Container).end()
//	
//	vector<size_t> ELF_offsets = spuFindSPUOffsetsInELFFile(fraw);
//
//
//	vector<ElfFile<ELF_32_traits_t>> SPUList;
//
//	std::transform(
//		ALL(ELF_offsets),
//		std::back_inserter(SPUList),
//		[]( size_t Offset ) -> ElfFile<ELF_32_traits_t>
//	{
//		return ElfFile<ELF_32_traits_t>("D:\\Torrents\\BLES00945\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf", Offset);
//		//return ElfFile<ELF_32_traits_t>("D:\\Dev\\segata-crossedgeu\\BLUS30348\\PS3_GAME\\USRDIR\\eboot.elf", Offset);
//	});
//
//	
//
//	//int i = 0;
//
//	std::for_each( ELF_offsets.cbegin(), ELF_offsets.cbegin() +1 ,
//		[](uint32_t off)
//	{
//		ElfFile<ELF_32_traits_t> SPU = ElfFile<ELF_32_traits_t>("D:\\Torrents\\BLES00945\\BLES00945\\PS3_GAME\\USRDIR\\eboot.elf", off);
//
//		spu_program_t program = spuExtractProgram( SPU );
//		spu_data_t data = spuExtractData( SPU );
//		spuSwitchEndianness( &program );
//		//spuSwitchEndianness( &data );
//		spu_info_t info;
//		spuGatherProgramHeuristics( &info, &program );
//		//spuGatherStaticBranches( &program, &info );		
//		//spuGatherJumps( &program, &info );
//		//spuGatherLoads( &program, &info );
//		const vector<uint32_t> fn_begins = spuGatherStaticCallTargets( &program, &info );
//		//spuBuildFunctionSymbols( &info, fn_begins );
//		
//		// write out extra info
//		vector<string> instrInfoTextPrefix( program.Binary.size(), "" );
//		vector<string> instrInfoTextPostfix( program.Binary.size(), "" );		
//		vector<string> instrInfoText( program.Binary.size(), "" );
//
//		// instruction addresses
//		//PrintInstrAdresses( (uint32_t)program.VirtualBaseAddress, instrInfoText );
//		
//		// function calls
//		/*std::for_each( info.functionSymbols.cbegin(), info.functionSymbols.cend(), 
//			[&]( const std::pair<uint32_t, string>& it )
//		{
//			if (it.first < instrInfoText.size())
//			{
//				instrInfoText[it.first] += string("begin ") + it.second;
//			}
//		} );*/
//
//		// labels
//		/*std::for_each( info.jumpSymbols.cbegin(), info.jumpSymbols.cend(), 
//			[&instrInfoText]( const std::pair<size_t, string>& it )
//		{
//			instrInfoText[it.first] += it.second + ":";
//		} );*/
//
//		// gotos
//		/*std::for_each( info.jumps.cbegin(), info.jumps.cend(), 
//			[&instrInfoText, &info]( const std::pair<size_t, size_t>& it )
//		{
//			instrInfoText[it.first].append("goto ").append( info.jumpSymbols[it.second] ).append(":");
//		} );*/
//
//		// comment out stack load/store
//		/*auto HideStackOPs = [&info, &program, &instrInfoTextPrefix](const std::string& mnem)
//		{
//			const auto& instr = info.heuristics[mnem];
//			auto program_local = &program;
//			auto instrInfoTextPrefix_local = &instrInfoTextPrefix;
//
//			std::for_each( instr.cbegin(), instr.cend(),
//				[program_local, instrInfoTextPrefix_local]( uint32_t fn_call_instr_offset )
//			{
//				const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program_local->Binary[fn_call_instr_offset]);
//				if ( 1 == OPComponents.RA )
//				{
//					instrInfoTextPrefix_local->at(fn_call_instr_offset) = string("//") + instrInfoTextPrefix_local->at(fn_call_instr_offset);
//				}
//			} );
//		};
//
//		HideStackOPs("lqd");
//		HideStackOPs("lqx");
//		HideStackOPs("stqd");
//		HideStackOPs("stqx");*/
//		
//		// indent functions
//		/*std::for_each( info.FunctionRanges.begin(), info.FunctionRanges.end(),
//			[&instrInfoTextPrefix]( const spu_function_t& fn )
//		{
//			auto beg = instrInfoTextPrefix.begin() + fn.begin + 1;
//			auto end = instrInfoTextPrefix.begin() + fn.end;		
//
//			while ( beg != end )
//			{
//				beg->append("\t");
//				++beg;
//			}
//
//		});*/
//
//		// indent static branches
//		/*std::for_each( info.staticBranches.begin(), info.staticBranches.end(),
//			[&instrInfoTextPrefix, &instrInfoTextPostfix]( const spu_branch_t& br )
//		{
//			if (br.AdvanceAmount > 0)
//			{
//				auto beg = &instrInfoTextPrefix[br.BranchBlockBegin];
//				auto end = &instrInfoTextPrefix[br.BranchBlockEnd];		
//
//				while ( beg != end )
//				{
//					beg->append("\t");
//					++beg;
//				}
//			}
//		});*/
//
//		/*
//		// indent jumps
//		std::for_each( info.jumps.cbegin(), info.jumps.cend(), 
//			[&instrInfoTextPrefix]( const std::pair<uint32_t, uint32_t>& it )
//		{
//			if ( it.first > it.second )
//			{
//				auto beg = &instrInfoTextPrefix[it.second];
//				auto end = &instrInfoTextPrefix[it.first];
//				while ( beg != end )
//				{
//					beg->append("\t");
//					++beg;
//				}
//			}			
//		} );*/
//
//		/*const auto& fn_calls = info.heuristics["brsl"];
//		std::for_each( fn_calls.cbegin(), fn_calls.cend(), 
//			[&instrInfoText, &info, &program]( uint32_t fn_call_instr_offset )
//		{
//			const SPU_OP_COMPONENTS OPComponents = spu_decode_op_components(program.Binary[fn_call_instr_offset]);
//			instrInfoText[fn_call_instr_offset].append( "call " ).append( info.functionSymbols[fn_call_instr_offset + (int32_t)OPComponents.IMM] );
//		} );*/
//
//		static uint32_t counter = 0;
//		
//		std::ostringstream path;
//		path << "spu_code_" << counter++;
//		std::ofstream fout_info((path.str() + ".info").c_str());
//		spuDumpInstrTextToFile((path.str() + ".intr"), instrInfoTextPrefix, program, instrInfoTextPostfix );
//		
//
//		if ( fout_info.is_open() )
//		{
//			std::copy( instrInfoText.cbegin(), instrInfoText.cend(), std::ostream_iterator<string>(fout_info, "\n") );
//		}
//	});
//}
//
//void spu_parse_file( SPU_t* targetSPU, const char* /*path */)
//{		
//	spu_execute( targetSPU );
//}
//
////string BytestreamToHexString( const void* data, size_t length, uint32_t stride )
////{
////	string out, hex_dump, txt_dump;
////
////	for ( size_t i = 0; i < length; ++i )
////	{	
////		if ( 0 != i
////			&& 0 == (i % stride) )
////		{
////			out += hex_dump;
////			out += " | ";
////			out += txt_dump;
////			out += "\n";
////			hex_dump.clear();
////			txt_dump.clear();
////		}
////
////		uint8_t byte = ((const uint8_t*)data)[i];
////		hex_dump.push_back( "0123456789ABCDEF"[0xF & (byte >> 4)] );
////		hex_dump.push_back( "0123456789ABCDEF"[0xF & byte] );
////		hex_dump.push_back( ' ' );
////		txt_dump.push_back( byte >= 0x20 ? (char)byte : '.' );
////
////	}
////
////	if ( !hex_dump.empty() )
////	{
////		hex_dump.resize( 16*3, ' ' );
////		txt_dump.resize( 16*3, ' ' );
////		out += hex_dump;
////		out += " | ";
////		out += txt_dump;
////		out += "\n";
////	}
////
////	return out;
////}