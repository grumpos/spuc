#include "fn.h"
#include "spu_idb.h"
#include "basicblock.h"

using namespace std;

void fn_calc_argcount(vector<fn>& functions)
{
	// scan function insns for register usage
	// registers 3-74 are used for argument passing
	// any register that is used as source operand before it was
	// used as destination operand must be a function argument
	for (auto& fun : functions)
	{
		// unused operands have index SPU_OP_INVALID_GPR
		// allocating enough space for this index removes lots of ifs
		uint8_t Registers[SPU_OP_INVALID_GPR + 1] = {0};
		uint8_t ArgCount = 0;

		auto check_gpr = [&Registers, &ArgCount](uint8_t gpr)
		{
			const bool is_arg_reg = gpr > 2 && gpr < 75;

			if (is_arg_reg && !Registers[gpr])
			{
				ArgCount = max<uint8_t>( gpr - 2, ArgCount );
			}
		};

		for (spu_insn* insn = fun.entry->ibegin; insn != fun.exit->iend; ++insn)
		{
			SPU_OP_COMPONENTS& OPComp = insn->comps;

			if (insn->op == spu_op::M_STOP)
			{
				continue;
			}

			if (insn->op == spu_op::M_WRCH)
			{
				Registers[OPComp.RA] = 1;
				continue;
			}

			if (insn->op != spu_op::M_RDCH
				&& insn->op != spu_op::M_RCHCNT)
			{
				check_gpr(OPComp.RA);				
			}	
			check_gpr(OPComp.RB);
			check_gpr(OPComp.RC);

			// flag register as written
			Registers[OPComp.RT] = 1;
		}

		fun.argcnt = ArgCount;
	}
}