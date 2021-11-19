# Lab 2: 时钟中断处理

## 1 实验目的

以时钟中断为例学习在RISC-V上的异常处理相关机制。

## 2 实验内容及要求

* 理解RISC-V异常委托与异常处理机制
* 利用OpenSBI平台的SBI调用触发时钟中断，并通过代码设计实现定时触发时钟中断的效果

请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。请查看文档尾部附录部分的背景知识介绍，跟随实验步骤完成实验，以截图的方式记录命令行的输入与输出，注意进行适当的代码注释。如有需要，请对每一步的命令以及结果进行必要的解释。

最终提交的实验报告请删除附录后命名为“**学号_姓名\_lab2.pdf**"，代码文件请根据给出的结构整理好后打包命名为“**学号_姓名\_lab2**"，分开上传至学在浙大平台。

## 3 实验步骤

### 3.1 搭建实验环境，理解实验执行流程

请同学们认真阅读**RISC-V中文手册中的【第十章 RV32/64 特权架构】**，对异常及特权模式进行一个系统性的了解，与本实验相关的内容已总结至附录中，可按顺序阅读。

本次实验的目标是**定时触发时钟中断并在相应的中断处理函数中输出相关信息**，最终运行结果可见【3.5 编译及测试】。代码实现逻辑如下：

* 在初始化阶段，设置CSR寄存器以允许S模式的时钟中断发生，利用SBI调用触发第一次时钟中断。
* SBI调用触发时钟中断后，OpenSBI平台自动完成了M模式的中断处理。由于设置了S模式的中断使能，随后触发了S模式下的时钟中断，相关异常处理CSR进行自动转换，需要保存寄存器现场后调用相应的时钟中断处理函数。
* 在时钟中断处理函数中打印相关信息，并设置下一次时钟中断，从而实现定时（每隔一秒执行一次）触发时钟中断并打印相关信息的效果。函数返回后恢复寄存器现场，调用监管者异常返回指令sret回到发生中断的指令。

为了完成实验，需要同学们在`init.c`中设置CSR寄存器允许中断发生，在`clock.c`中触发时钟中断事件，在`entry.S`及`trap.c`中编写中断处理函数。

#### 1. 创建容器，映射文件夹

同lab1的文件夹映射方法，目录名为`lab2`。

![image-20211101135018535](C:\Users\欸？\AppData\Roaming\Typora\typora-user-images\image-20211101135018535.png)

#### 2. 理解组织文件结构

```
lab2
├── arch
│   └── riscv
│       ├── kernel
│       │   ├── clock.c
│       │   ├── head.S
│       │   ├── entry.S
│       │   ├── init.c
│       │   ├── main.c
│       │   ├── Makefile
│       │   ├── print.c
│       │   ├── sbi.c
│       │   ├── trap.c
│       │   └── vmlinux.lds
│       └── Makefile
├── include
│   ├── defs.h
│   ├── riscv.h
│   └── test.h
├── Makefile
└── README.md
```

下载相关代码移动至`lab2`文件夹中，项目代码结构如上所示，**请同学们根据lab1中自己写的代码填写`print.c`以及`sbi.c`中的`sbi_call()`函数。**

附录【B.gef插件】提供了一款gdb插件`gef`的下载安装方式，该插件能够实时显示各个寄存器的值，**方便调试**，有需要的同学可以按照提示进行安装，实验中对此不做要求。

### 3.修改必要文件

由于裸机程序需要在`.text`段起始位置执行，所以需要利用`vmlinux.lds`中`.text`段的定义来确保`head.S`中的`.text`段被放置在其他`.text`段之前，首先修改`head.S`中的`.text`命名为`.text.init`：

```asm
<<<<< before
.section .text
============
.section .text.init
>>>>> after
```

修改`entry.S`中的`.text`命名为`.text.entry`：

```asm
<<<<< before
.section .text
============
.section .text.entry
>>>>> after
```

然后修改`vmlinux.lds`文件中的`.text`展开方式：

```asm
<<<<< before 
.text : {
		*(.text)
		*(.text.*)
	 }
============
.text : {
		*(.text.init)
		*(.text.entry)
		*(.text)
		*(.text.*)
	 }
>>>>> after
```

### 3.2 编写init.c中的相关函数（20%）

我们将M模式的事务交由OpenSBI进行处理，从而只需要关注S模式下的处理。为了满足时钟中断在S模式下触发的使能条件，我们需要对以下寄存器进行设置：

1. 设置`stvec`寄存器。`stvec`寄存器中存储着S模式下发生中断时跳转的地址，我们需要编写相关的中断处理函数，并将地址存入`stvec`中。
2. 将`sie`寄存器中的stie位 置位。`sie[stie]`为1时，s模式下的时钟中断开启。
3. 将`sstatus`寄存器中的sie位 置位。`sstatus[sie]`位为s模式下的中断总开关，这一位为1时，才能响应中断。

在`init.c`中设置CSR寄存器以允许中断发生，`idt_init()`函数用于设置`stvec`寄存器，`intr_enable()`与`intr_disable()`通过设置`sstatus[sie]`打开s模式的中断开关。

#### 1.编写`intr_enable()`/`intr_disable()`

使用CSR命令设置`sstatus[sie]`的值，可以使用`riscv.h`文件中的相应函数，也可以自行通过内联汇编实现。

```c
void intr_enable(void) { 
    //设置sstatus[sie]=1,打开s模式的中断开关
    //your code
    set_csr(sstatus,1<<1);
}

void intr_disable(void) {
    //设置sstatus[sie]=0,关闭s模式的中断开关
    //your code
    clear_csr(sstatus, 0x1D);
 }
```

**Q1.解释所填写的代码。**

**答：**利用set_csr函数，借助csrrsi指令设置sstatus[1] = sstatus[sie] = 1,打开s模式的中断开关

利用clear_csr函数，借助csrrci指令设置sstatus[1] = sstatus[sie] = 0，关闭s模式下的中断开关



#### 2.编写`idt_init()`

使用`riscv.h`文件中的相应函数向`stvec`寄存器中写入中断处理后跳转函数`trap_s`（entry.S)的**地址**。

```c
void idt_init(void) {
    extern void trap_s(void);
    //向stvec寄存器中写入中断处理后跳转函数的地址
    //your code
	write_csr(stvec, trap_s);
}
```

### 3.3 编写`clock.c`中的相关函数（30%）

我们的"时钟中断"实际上就是"每隔若干个时钟周期执行一次的程序"，可以利用OpenSBI提供的`sbi_set_timer()`接口实现，**向该函数传入一个时刻，函数将在那个时刻触发一次时钟中断**（见`lab1`中对sbi调用的相关说明）。使用SBI调用完成了M模式下的中断触发与处理，不需要考虑`mtime`、`mtimecmp`等相关寄存器。

我们需要“**每隔若干时间就发生一次时钟中断**”，但是OpenSBI提供的接口一次只能设置一个时钟中断事件。我们采用的方式是：一开始只设置一个时钟中断，之后每次发生时钟中断的时候，在相应的中断处理函数中设置下一次的时钟中断。

在`clock.c`中，我们需要定时触发时钟中断事件，`clock_init()`用于启用时钟中断并设置第一个时钟中断；`clock_set_next_event()`用于调用SBI函数`set_sbi_timer()`触发时钟中断。`get_cycle()`函数通过`rdtime`伪指令读取一个叫做`time`的CSR的数值，表示CPU启动之后经过的真实时间。

qemu中外设晶振的频率为10mhz，即每秒钟`time`的值将会增大$10^7$。我们可以据此来计算每次`time`的增加量，以控制时钟中断的间隔。

**Q2.为了使得每次时钟中断的间隔为1s，`timebase`（即`time`的增加量）需要设置为___10^7___\__\_\__。若需要时钟中断间隔为0.1s，`timebase`需要设置为_______10^6_________________。**

```c
static uint64_t timebase =1e7;//需要自行修改timebase为合适的值，使得时钟中断间隔为1s
```



#### 1. 编写`clock_init()`

```c
void clock_init(void) {
    puts("ZJU OS LAB 2             Student_ID:123456\n");
    //对sie寄存器中的时钟中断位设置（sie[stie]=1）以启用时钟中断。
    //设置第一个时钟中断
    //your code
    set_csr(sie,1<<5);
    ticks = 1;
    uint64_t time = get_cycles();
    trigger_time_interrupt(time);
}
```

#### 2.编写`clock_set_next_event()`

```C
void clock_set_next_event(void) { 
    //获取当前cpu cycles数并计算下一个时钟中断的发生时刻，利用sbi.c中相关函数触发时钟中断。
    //your code
    uint64_t time = get_cycles() + timebase;
    trigger_time_interrupt(time);
    ticks++;
}
```

**Q3.解释`sbi.c`中`trigger_time_interrupt()`函数。**

答：该函数利用lab1所写的sbi_call函数，传入sbi指令类型为0，即set_sbi_timer()函数，传入中断时刻，系统到达该时刻时会触发中断。



### 3.4 编写并调用中断处理函数（50%）

#### 1.在`entry.S`中调用中断处理函数（30%）

在【3.1】中，我们向stvec寄存器存入了中断处理函数的地址，中断发生后将自动进行硬件状态转换，程序将读取`stvec`的地址并进行跳转，运行`trap_s`函数。该函数需要在栈中保存`调用者保存寄存器`及`sepc`寄存器，读取`scause`CSR寄存器设置参数`a0`调用`handle_s`函数进行中断处理，调用返回后需要恢复寄存器并使用`sret`命令回到发生中断的指令。

**提示**：可参考[RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)3.3节相关内容完成实验；寄存器大小为8字节；CSR命令的特殊使用方式（见lab1附录)；若不清楚`调用者保存寄存器`，可将寄存器全都保存；对汇编语言不是特别了解的建议把中文手册读一遍，用时也不长的。

```asm
trap_s:
	#save caller-saved registers and spec
    addi sp, sp, -32
	sd ra, 24(sp)
	sd t0, 16(sp)
	csrr t0, sepc
	sd t0, 8(sp)
	sd a0, 0(sp) 
	# call handler_s(scause)
	csrr a0, scause
	call handler_s
	# load sepc and caller-saved registers
	ld ra, 24(sp)
	ld t0, 8(sp)
	csrw sepc, t0
	ld t0, 16(sp)
	ld a0, 0(sp)
	addi sp, sp, 32
	sret

```

**Q4.为什么要保存`sepc`寄存器：**

答：发生异常时，sepc保存了pc寄存器的值，指示下一句应该执行的指令，在异常处理完成后，sret会将sepc的值写回pc，让程序继续执行，所以需要对sepc寄存器进行保护。



#### 2.在`trap.c`中编写中断处理函数（20%）

正常情况下，异常处理函数需要根据`[m|s]cause`寄存器的值判断异常的种类后分别处理不同类型的异常，但在本次实验中简化为只判断并处理时钟中断。

【3.3】中提到，为了实现"定时触发中断"，我们需要在中断处理函数中设置下一次的时钟中断。此外，为了进行测试，中断处理函数中还需要打印时钟中断发生的次数，需要在`clock.c`中利用`ticks`变量进行统计，请更新【3.3】中相关代码。

```c
void handler_s(uint64_t cause){
    // interrupt	
	if (cause) {
		// supervisor timer interrupt
		if (ticks >= 1) {	
			//设置下一个时钟中断，打印当前的中断数目。
    		//your code
			puts("[S] Supervisor Timer Interrupt ");
			put_num(ticks);
			puts("\n");
			clock_set_next_event();
		}
	}
}
```

**Q5.触发时钟中断时，`[m|s]cause`寄存器的值是？据此填写代码中的条件判断语句。**

答：1，表示发生了中断。



**Q6.【同步异常与中断的区别】当处理同步异常时应该在退出前给`epc`寄存器+4（一条指令的长度），当处理中断时则不需要，请解释为什么要这样做。**

答：同步异常在指令执行后进入中断(trap)，所以需要epc+4以恢复执行下一条语句，而中断可以在指令之间发生，所以无需+4，继续执行当前语句即可。

### 3.5 编译及测试

请修改`clock.c`中的`ID:123456`，确保输出自己的学号。仿照`lab1`进行调试，预期的实验结果如下，**每隔一秒触发一次时钟中断，打印一次信息**。请将样例截图替换为自己的截图。

![31910339416d2a587728c2ae912809d](C:\Users\欸？\AppData\Local\Temp\WeChat Files\31910339416d2a587728c2ae912809d.png)

## 4 讨论和心得

1. 请在此处填写实验过程中遇到的问题及相应的解决方式。

RISCV中文文档附录A的指令集，关于csr的指令那里有不少相互矛盾的地方，特别是csrrw csrrc csrrsi那里，比较混乱，给我一开始写实验造成了很大困扰，以及发现代码有一些和我们上一个实验不太一致的地方，比如trap.c里面，put_num变成了string_num，不过如果理解了代码的整体逻辑，这些问题都很容易被解决。

2. 本实验本意是使用OpenSBI平台避免编写过多的汇编代码以简化实验，但是这样做之后省去了实际时钟中断处理时的复杂跳转，把整个过程简化得过于简单了。欢迎同学们对本实验提出建议。

我觉得本次实验难度适中，一开始因为RISCV和相应汇编掌握不好，所以一头雾水，但按照助教的建议稍微花了点时间去仔细阅读了相应文档，顿时有醍醐灌顶之感，实验也很顺利地写出来。如果本实验需要写过多的汇编代码的话，可能我学了之后也要花过多时间，对于一次实验来说，我觉得这是不太合适。不过时钟中断的复杂跳转可以以思考题的形式给出。让大家去了解整个过程，但不必写代码以降低难度。
