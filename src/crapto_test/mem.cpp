#include "stdafx.h"


void * crapto_malloc(size_t n)
{
  
  memstats.malloc_free++;
  void* p = malloc(n);
  
  if(memstats.log)
    printf("[crapto_mem] consume | 0x%08X | %p\n",n,p);

  memstats.memuse+=n;
  memloc_t* loc = &memstats.pos[memstats.index++];
  loc->addr = p;
  loc->size = n;
  return p;
}

void dump_memloc()
{
  unsigned n;
  for(n = 0;n< 10000;n++)
  {
    if((unsigned)memstats.pos[n].addr)
      printf("[!MEM] leaked 0x%08X | %p\n",memstats.pos[n].size, memstats.pos[n].addr);
  }
}
memloc_t * find_memloc(unsigned addr)
{
  unsigned n;
  for(n = 0;n< 10000;n++)
  {
    if((unsigned)memstats.pos[n].addr == addr)
      return &memstats.pos[n];
  }
  return 0;
}

void crapto_free(void *p)
{
  memloc_t* loc;
  memstats.malloc_free--;
  loc = find_memloc((unsigned)p);
  
  if(!loc)
  {
    printf("[error] Cannot find memloc %p\n",p);

  }
  else{
    memstats.memuse-=loc->size;
    if(memstats.log)
      printf("[crapto_mem] release | 0x%08X | %p\n",loc->size,p);
    loc->addr = 0;
    loc->size = 0;
  }
  free(p);
}