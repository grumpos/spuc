#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <string>
#include "spu_emu.h"
#include "spu_idb.h"

static void spu_preprocess_strip_clutter( std::string& s )
{
	std::replace( s.begin(), s.end(), ',', ' ' );
	std::replace( s.begin(), s.end(), '$', ' ' );
	std::replace( s.begin(), s.end(), '(', ' ' );
	std::replace( s.begin(), s.end(), ')', ' ' );
}

static void spu_preprocess_strip_comment( std::string& s ) 
{
	s = s.substr( 0, s.find( ';' ) );
}

static void spu_preprocess_resolve_aliases( std::string& s )
{
	auto LR_alias = s.find( "LR" );
	if ( std::string::npos != LR_alias )
	{
		s.replace( LR_alias, 2, "0" );
	}	

	auto SP_alias = s.find( "SP" );
	if ( std::string::npos != SP_alias )
	{
		s.replace( SP_alias, 2, "1" );	
	}

	auto lr_alias = s.find( "lr" );
	if ( std::string::npos != lr_alias )
	{
		std::string temp = s;
		spu_preprocess_strip_comment( temp );
		temp = std::string("ori ") + temp.substr( temp.find( "lr" ) + 2 ) + std::string(" 0");
		s = temp;
	}	
}

uint32_t spu_assemble( const std::string& istr_text_raw )
{
	std::string istr_text = istr_text_raw;

	spu_preprocess_strip_comment( istr_text );	
	spu_preprocess_strip_clutter( istr_text );
	spu_preprocess_resolve_aliases( istr_text );

	std::istringstream spu_command_stream( istr_text );
	std::string mnemonic;
	spu_command_stream >> mnemonic;

	if ( !mnemonic.empty() )
	{
		uint32_t opcode = 0;
		while ( opcode < SPU_MAX_INSTRUCTION_COUNT && mnemonic != spu_decode_op_mnemonic( opcode << 21 ) )
		{
			++opcode;
		}

		if ( opcode == SPU_MAX_INSTRUCTION_COUNT )
		{
			return 0x7FF;
		}

		const size_t optype = spu_decode_op_type(opcode << 21);

		

		// Assuming the text instruction is correct, read the arguments in order into the
		// slots of the matching instruction type
		// e.g.: RR's first argument is a target register so read it into args[A_RT]

		int64_t args[32] = {0};
		for ( size_t i = 0; i < db_op_arglist[opcode].size(); ++i )
		{
			SPU_ARGLIST al = db_op_arglist[opcode][i];
			int64_t temp = 0;
			for ( size_t j = 0; j < al.ArgCount; ++j )
			{
				const ptrdiff_t argt = al.ArgType[j];
				spu_command_stream >> temp;

				if (argt == A_P)
					continue;

				args[argt] = temp;

				if ( argt == A_S18 )
				{
					args[A_S16] = args[argt] >> 2;
				}
				else if ( argt == A_S14 )
				{
					args[A_S10] = args[argt] >> 4;
				}
			}		
		}

		//==

		// Based on instruction type, just assemble the binary form using the argument list.

		SPU_INSTRUCTION result;
		result.Instruction = uint32_t(-1);

		switch ( optype )
		{
		case SPU_OP_TYPE_RRR:
			{
				result.RRR.OP = opcode >> 7;
				result.RRR.RT = args[A_T];
				result.RRR.RA = args[A_A];
				result.RRR.RB = args[A_B];
				result.RRR.RC = args[A_C];
				break;
			}
		case SPU_OP_TYPE_RR:
			{
				result.RR.OP = opcode;
				result.RR.RT = args[A_T];
				result.RR.RA = args[A_A];
				result.RR.RB = args[A_B];
				break;
			}
		case SPU_OP_TYPE_RI7:
			{
				result.RI7.OP = opcode;
				result.RI7.RT = args[A_T];
				result.RI7.RA = args[A_A];
				result.RI7.I7 = args[A_S7];
				break;
			}
		case SPU_OP_TYPE_RI8:
			{
				result.RI8.OP = opcode >> 1;
				result.RI8.RT = args[A_T];
				result.RI8.RA = args[A_A];
				result.RI8.I8 = args[A_U7A];
				break;
			}
		case SPU_OP_TYPE_RI10:
			{
				result.RI10.OP = opcode >> 3;
				result.RI10.RT = args[A_T];
				result.RI10.RA = args[A_A];
				result.RI10.I10 = args[A_S10];
				break;
			}
		case SPU_OP_TYPE_RI16:
			{
				result.RI16.OP = opcode >> 2;
				result.RI16.RT = args[A_T];
				result.RI16.I16 = args[A_S16];
				break;
			}
		case SPU_OP_TYPE_RI18:
			{
				result.RI18.OP = opcode >> 4;
				result.RI18.RT = args[A_T];
				result.RI18.I18 = args[A_U18];
				break;
			}
		case SPU_OP_TYPE_LBT:
			{
				result.LBT.OP = opcode >> 4;
				result.LBT.I16 = args[A_S16];
				result.LBT.ROL = (args[A_S11] & 0x7F);
				result.LBT.ROH = ((args[A_S11] >> 7) & 3);
				break;
			}
		case SPU_OP_TYPE_LBTI:
			{
				result.LBTI.OP = opcode;
				result.LBTI.RA = args[A_S16];
				result.LBTI.ROL = (args[A_S11I] & 0x7F);
				result.LBTI.ROH = ((args[A_S11I] >> 7) & 3);
				result.LBTI.unused = 0;
				break;
			}
		};	

		return result.Instruction;
	}
	else	// if ( !mnemonic.empty() )
	{
		//std::cout << "ERROR: Unknown Command: " << istr_text << std::endl;
		return 0x7FF;
	}	
}