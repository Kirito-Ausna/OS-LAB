#include "riscv.h"

extern void clock_init(void);

void intr_enable(void) { 
    //设置sstatus[sie]=1,打开s模式的中断开关
    //your code
}

void intr_disable(void) {
    //设置sstatus[sie]=0,关闭s模式的中断开关
    //your code
 }

void idt_init(void) {
    extern void trap_s(void);
    //向stvec寄存器中写入中断处理后跳转函数的地址
    //your code

  
}

void init(void) {
    idt_init();
    intr_enable();
    clock_init();
    task_init();
}
