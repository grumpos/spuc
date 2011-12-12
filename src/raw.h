#pragma once
#ifndef __MEMMAP_H__
#define __MEMMAP_H__

struct memmap_t;

memmap_t*	mmopen(const char* path, const char* openmode = "r");
void		mmclose(memmap_t*);
size_t		mmsize(memmap_t*);
void*		mmbegin(memmap_t*);
const void*	mmbegin(const memmap_t*);
void*		mmend(memmap_t*);
const void*	mmend(const memmap_t*);

void		mmcreate(const char* path, size_t size);

#endif