#ifndef __FN_H__
#define __FN_H__

#include <vector>
#include <cstdint>

struct bb;

struct fn
{
	bb* entry;
	bb* exit;
	uint8_t argcnt;
	uint8_t retcnt;
};

void fn_calc_argcount(std::vector<fn>& functions);

#endif // __FN_H__