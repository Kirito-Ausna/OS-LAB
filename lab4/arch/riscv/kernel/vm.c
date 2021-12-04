#include"vm.h"
#include"put.h"
  
extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;


void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm)
{
	/*your code*/
}

void paging_init()
{
   uint64_t *pgtbl = &_end;
    /*your code*/
}
