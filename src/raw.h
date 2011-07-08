#pragma once
#ifndef __MEMMAP_H__
#define __MEMMAP_H__

struct memmap_t;

memmap_t*	open(const char* path);
void		close(memmap_t*);
size_t		size(memmap_t*);
void*		begin(memmap_t*);
const void*	begin(const memmap_t*);
void*		end(memmap_t*);
const void*	end(const memmap_t*);

#endif