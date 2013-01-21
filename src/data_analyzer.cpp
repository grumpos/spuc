#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include "spu_idb.h"

using namespace std;

// Experimental stuff

void FindTextSectionEnd(const void* data, size_t len)
{
	vector<uint32_t> possibleTextSectionEnd;

	for (const char* str = (const char*)data; ; str += 4)
	{

		const uint32_t* op = (uint32_t*)str;
		const uint32_t spu_bi = 0x00000035;
		if (*op == spu_bi)
		{
			const size_t padLen = 16 - ((int)op & 0xF) - sizeof(uint32_t);
			if (string(str + 4, str + 4 + padLen) == string(padLen, '\0'))
			{
				possibleTextSectionEnd.push_back(((str - (const char*)data) + 0x22400)&0x3FFF0);
			}
		}

		if (str == (const char*)data + len)
			break;
	}
}

void PossibleStrings(const void* data, size_t len)
{
	vector<string> possibleStrings;

	for (const char* str = (const char*)data;;)
	{
		// assume we are pointing to a null terminated string
		const size_t max_len = (const char*)data + len - str;
		// TODO: non-portable. needed because we can't guarantee the binary ends with 0
		size_t len = 0;//strnlen(str, max_len);
		//const size_t len = strlen(str);
		for (const char* s = str, * s_max = str + max_len; s < s_max && *s; ++s, ++len)
		{
			;
		}

		if (0 == len)
		{
			str += 16;
			continue;
		}

		// that is 16 byte aligned and padded to be a multiple of 16
		const size_t qwordCount = len / 16 + 1;
		const size_t zeroPadLen = 16 - len % 16;
		const char* str_begin	= str;
		const char* str_end		= str_begin + len;
		const char* pad_begin	= str_end;
		const char* pad_end		= pad_begin + zeroPadLen;

		// printable characters only until the null terminator
		bool isPrintable = true;
		for (const char* c = str_begin; isPrintable && c != str_end; ++c)
		{
			isPrintable = isPrintable 
				&& *c >= 0
				&& *c <= 255
				&& (isprint(*c) || *c == '\t' || *c == '\n');
		}

		// must end with null padding to multiple of 16
		const bool isZeroPadded = string(pad_begin, pad_end) == string(zeroPadLen, '\0');

		if (isPrintable && isZeroPadded)
		{
			possibleStrings.push_back(string(str_begin, str_end));
			str += qwordCount * 16;
		}
		else
		{
			str += 16;
		}		

		if (str == (const char*)data + len)
			break;
	}
}

vector<spu_insn> remove_nops(vector<spu_insn>& ilist)
{
	// Remove all NOP/LNOP

	// Store how many NOP/LNOP will be removed before each insn
	vector<uint32_t> Offsets(ilist.size());

	uint32_t NOPCount = 0;

	for (size_t ii = 0; ii < ilist.size(); ++ii)
	{
		Offsets[ii] = NOPCount; 

		if (spu_op::M_NOP == ilist[ii].op
			|| spu_op::M_LNOP == ilist[ii].op)
		{
			++NOPCount;
		}
	}

	for (size_t ii = 0; ii < ilist.size(); ++ii)
	{
		spu_insn* Last = ilist.data() + ilist.size();
		spu_insn* Current = &ilist[ii];
		const size_t TargetOffset = 0xffff & ((int64_t)ii + ilist[ii].comps.IMM);
		spu_insn* Target = ilist.data() + TargetOffset;

		if (spu_op::M_LQR == ilist[ii].op
			|| spu_op::M_BR == ilist[ii].op
			|| spu_op::M_BRSL == ilist[ii].op
			|| spu_op::M_BRZ == ilist[ii].op
			|| spu_op::M_BRNZ == ilist[ii].op
			|| spu_op::M_BRHZ == ilist[ii].op
			|| spu_op::M_BRHNZ == ilist[ii].op
			|| spu_op::M_STQR == ilist[ii].op)
		{
			if (Target < Current)
			{
				ilist[ii].comps.IMM -= Offsets[ii];

				if (&ilist[ii] <= Target && Target < Last)
				{
					ilist[ii].comps.IMM += (Offsets[ii] - Offsets[TargetOffset]);
				}
			}
			else if (Target > Current)
			{
				ilist[ii].comps.IMM += Offsets[ii];

				if (&ilist[ii] <= Target && Target < Last)
				{
					ilist[ii].comps.IMM -= (Offsets[TargetOffset] - Offsets[ii]);
				}
			}			
		}
	}

	vector<spu_insn> ilist_nopless(ilist.size() - NOPCount);

	// Compact it, skipping NOP/LNOP
	//vector<spu_insn> ilist_nopless;
	copy_if(ilist.begin(), ilist.end(), ilist_nopless.begin(), 
		[](const spu_insn& insn)
	{
		return !(spu_op::M_NOP == insn.op
			|| spu_op::M_LNOP == insn.op);
	});

	return ilist_nopless;
}