#pragma once

/*
  Just some fun with mem tracking
*/

typedef struct
{
    void* addr;
    size_t size;
}memloc_t;

#define ENABLE_VERBOSE_MEM_LOGGING FALSE

struct memory_stats_t
{
  memloc_t pos[10000];
  unsigned index;
  unsigned malloc_free;
  unsigned log;
  unsigned memuse;
}static memstats = 
{ 
  {0}
  , 0
  , 0
  , ENABLE_VERBOSE_MEM_LOGGING
  , 0
};

void * crapto_malloc(size_t n);

void dump_memloc();

memloc_t * find_memloc(unsigned addr);

void crapto_free(void *p);