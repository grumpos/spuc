#ifndef __FN_H__
#define __FN_H__

struct bb;

struct fn
{
	bb* entry;
	bb* exit;
	uint8_t argcnt;
};

#endif // __FN_H__