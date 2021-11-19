#include "defs.h" 
#include "riscv.h"

uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint64_t ret_val;
    __asm__ volatile ( 
         //Your code
   );
    return ret_val;
}


void trigger_time_interrupt(unsigned long long stime_value) {
    sbi_call(0, stime_value, 0, 0);
}
