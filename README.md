# OS-LAB

2021-2022  操作系统实验

## 附录 LAB0

### A.Linux 使用基础

#### Linux简介

Linux 是一套免费使用和自由传播的类 Unix 操作系统，是一个基于 POSIX 和 UNIX 的多用户、多任务、支持多线程和多 CPU 的操作系统。 

在Linux环境下，人们通常使用命令行接口来完成与计算机的交互。终端（Terminal）是用于处理该过程的一个应用程序，通过终端你可以运行各种程序以及在自己的计算机上处理文件。在类Unix的操作系统上，终端可以为你完成一切你所需要的操作。我们仅对实验中涉及的一些概念进行介绍，你可以通过下面的链接来对命令行的使用进行学习：

1. [The Missing Semester of Your CS Education](https://missing-semester-cn.github.io/2020/shell-tools)[&gt;&gt;Video&lt;&lt;](https://www.bilibili.com/video/BV1x7411H7wa?p=2)
2. [GNU/Linux Command-Line Tools Summary](https://tldp.org/LDP/GNU-Linux-Tools-Summary/html/index.html)
3. [Basics of UNIX](https://github.com/berkeley-scf/tutorial-unix-basics)

#### Linux中的环境变量

当我们在终端输入命令时，终端会找到对应的程序来运行。我们可以通过 `which`命令来做一些小的实验：

```bash
$ which gcc
/usr/bin/gcc
$ ls -l /usr/bin/gcc
lrwxrwxrwx 1 root root 5 5月  21  2019 /usr/bin/gcc -> gcc-7
```

可以看到，当我们在输入 `gcc`命令时，终端实际执行的程序是 `/usr/bin/gcc`。实际上，终端在执行命令时，会从 `PATH`环境变量所包含的地址中查找对应的程序来执行。我们可以将 `PATH`变量打印出来来检查一下其是否包含 `/usr/bin`。

```bash
$ echo $PATH
/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/phantom/.local/bin
```

如果你想直接访问 `riscv64-unknown-linux-gnu-gcc`、`qemu-system-riscv64`等程序，那么你需要把他们所在的目录添加到目录中，镜像中已完成了这步操作，可在相应容器中使用`echo $PATH`命令检查是否包含`/opt/riscv/bin`目录。

```bash
$ export PATH=$PATH:/opt/riscv/bin
```

### B. Docker 使用基础

#### Docker 基本介绍

在生产开发环境中，常常会遇到应用程序和系统环境变量以及一些依赖的库文件不匹配，导致应用无法正常运行的问题，因此出现了Docker。Docker 是一种利用容器（container）来进行创建、部署和运行应用的工具。Docker把一个应用程序运行需要的二进制文件、运行需要的库以及其他依赖文件打包为一个包（package），然后通过该包创建容器并运行，由此被打包的应用便成功运行在了Docker容器中。

比如，你在本地用Python开发网站后台，开发测试完成后，就可以将Python3及其依赖包、Flask及其各种插件、Mysql、Nginx等打包到一个容器中，然后部署到任意你想部署到的环境。

Docker中有三个重要概念：

1. 镜像（Image）：类似于虚拟机中的镜像，是一个包含有文件系统的面向Docker引擎的只读模板。任何应用程序运行都需要环境，而镜像就是用来提供这种运行环境的。
2. 容器（Container）：Docker引擎利用容器来运行、隔离各个应用。**容器是镜像创建的应用实例**，可以创建、启动、停止、删除容器，各个容器之间是是相互隔离的，互不影响。镜像本身是只读的，容器从镜像启动时，Docker在镜像的上层创建一个可写层，镜像本身不变。
3. 仓库（Repository）：镜像仓库是Docker用来集中存放镜像文件的地方，一般每个仓库存放一类镜像，每个镜像利用tag进行区分，比如Ubuntu仓库存放有多个版本（12.04、14.04等）的Ubuntu镜像。

#### Docker基本命令

实验中主要需要了解docker中容器生命周期管理及容器操作的相关命令及选项，可以通过[Docker 命令大全 | 菜鸟教程 (runoob.com)](https://www.runoob.com/docker/docker-command-manual.html)进行初步了解。

```shell
#列出所有镜像
docker image ls

#创建/启动容器
docker run -it oslab:2020 /bin/bash    
#启动容器时，使用-v参数指定挂载宿主机目录，启动一个centos容器将宿主机的/test目录挂载到容器的/soft目录
docker run -it -v /test:/soft centos /bin/bash
#进入运行中的容器，执行bash:
docker exec -it 9df70f9a0714 /bin/bash
#停止容器
docker stop ID
#启动处于停止状态的容器
docker start ID

#重命名容器
docker rename oldname newname
#列出所有的容器
docker ps -a
#删除容器
docker rm ID
```

### C. QEMU 使用基础

#### 什么是QEMU

QEMU最开始是由法国程序员Fabrice Bellard等人开发的可执行硬件虚拟化的开源托管虚拟机。

QEMU主要有两种运行模式。**用户模式下，QEMU能够将一个平台上编译的二进制文件在另一个不同的平台上运行。**如一个ARM指令集的二进制程序，通过QEMU的TCG（Tiny Code Generator）引擎的处理之后，ARM指令被转化为TCG中间代码，然后再转化为目标平台（如Intel x86）的代码。

**系统模式下，QEMU能够模拟一个完整的计算机系统，该虚拟机有自己的虚拟CPU、芯片组、虚拟内存以及各种虚拟外部设备。**它使得为跨平台编写的程序进行测试及除错工作变得容易。

#### 如何使用 QEMU（常见参数介绍）

以该命令为例，我们简单介绍QEMU的参数所代表的含义

```bash
$ qemu-system-riscv64 -nographic -machine virt -kernel build/linux/arch/riscv/boot/Image  \
 -device virtio-blk-device,drive=hd0 -append "root=/dev/vda ro console=ttyS0"   \
 -bios default -drive file=rootfs.ext4,format=raw,id=hd0 \
 -netdev user,id=net0 -device virtio-net-device,netdev=net0 -S -s
```

* **-nographic**: 不使用图形窗口，使用命令行
* **-machine**: 指定要emulate的机器，可以通过命令 `qemu-system-riscv64 -machine help`查看可选择的机器选项
* **-kernel**: 指定内核image
* **-append cmdline**: 使用cmdline作为内核的命令行
* **-device**: 指定要模拟的设备，可以通过命令 `qemu-system-riscv64 -device help`查看可选择的设备，通过命令 `qemu-system-riscv64 -device <具体的设备>,help`查看某个设备的命令选项
* **-drive, file=<file_name>**: 使用'file'作为文件系统
* **-netdev user,id=str**: 指定user mode的虚拟网卡, 指定ID为str
* **-S**: 启动时暂停CPU执行(使用'c'启动执行)
* **-s**: -gdb tcp::1234 的简写
* **-bios default**: 使用默认的OpenSBI firmware作为bootloader

更多参数信息可以参考官方文档[System Emulation — QEMU documentation (qemu-project.gitlab.io)](https://qemu-project.gitlab.io/qemu/system/index.html)

### D. GDB 使用基础

#### 什么是 GDB

GNU调试器（英语：GNU Debugger，缩写：gdb）是一个由GNU开源组织发布的、UNIX/LINUX操作系统下的、基于命令行的、功能强大的程序调试工具。借助调试器，我们能够查看另一个程序在执行时实际在做什么（比如访问哪些内存、寄存器），在其他程序崩溃的时候可以比较快速地了解导致程序崩溃的原因。被调试的程序可以是和gdb在同一台机器上，也可以是不同机器上。

总的来说，gdb可以有以下4个功能：

- 启动程序，并指定可能影响其行为的所有内容
- 使程序在指定条件下停止
- 检查程序停止时发生了什么
- 更改程序中的内容，以便纠正一个bug的影响

#### GDB 基本命令介绍

在gdb命令提示符“(gdb)”下输入“help”可以查看所有内部命令及使用说明。

* (gdb) start：单步执行，运行程序，停在第一执行语句
* (gdb) next：单步调试（逐过程，函数直接执行）,简写n
* (gdb) run：重新开始运行文件（run-text：加载文本文件，run-bin：加载二进制文件），简写r
* (gdb) backtrace：查看函数的调用的栈帧和层级关系，简写bt
* (gdb) break 设置断点。比如断在具体的函数就break func；断在某一行break filename:num
* (gdb) finish：结束当前函数，返回到函数调用点
* (gdb) frame：切换函数的栈帧，简写f
* (gdb) print：打印值及地址，简写p
* (gdb) info：查看函数内部局部变量的数值，简写i；查看寄存器的值i register xxx
* (gdb) display：追踪查看具体变量值

**学会调试将在后续实验中为你提供帮助，推荐同学们跟随[GDB调试入门指南](https://zhuanlan.zhihu.com/p/74897601)教程完成相应基础练习，熟悉gdb调试的使用。**

### E. LINUX 内核编译基础

#### 交叉编译

交叉编译指的是在一个平台上编译可以在另一个平台运行的程序，例如在x86机器上编译可以在arm平台运行的程序，交叉编译需要交叉编译工具链的支持。

#### 内核配置

内核配置是用于配置是否启用内核的各项特性，内核会提供一个名为 `defconfig`(即default configuration) 的默认配置，该配置文件位于各个架构目录的 `configs` 文件夹下，例如对于RISC-V而言，其默认配置文件为 `arch/riscv/configs/defconfig`。使用 `make ARCH=riscv defconfig` 命令可以在内核根目录下生成一个名为 `.config` 的文件，包含了内核完整的配置，内核在编译时会根据 `.config` 进行编译。配置之间存在相互的依赖关系，直接修改defconfig文件或者 `.config` 有时候并不能达到想要的效果。因此如果需要修改配置一般采用 `make ARCH=riscv menuconfig` 的方式对内核进行配置。

#### 常见参数

**ARCH** 指定架构，可选的值包括arch目录下的文件夹名，如x86,arm,arm64等，不同于arm和arm64，32位和64位的RISC-V共用 `arch/riscv` 目录，通过使用不同的config可以编译32位或64位的内核。

**CROSS_COMPILE** 指定使用的交叉编译工具链，例如指定 `CROSS_COMPILE=aarch64-linux-gnu-`，则编译时会采用 `aarch64-linux-gnu-gcc` 作为编译器，编译可以在arm64平台上运行的kernel。

**CC** 指定编译器，通常指定该变量是为了使用clang编译而不是用gcc编译，Linux内核在逐步提供对clang编译的支持，arm64和x86已经能够很好的使用clang进行编译。

#### 常用编译选项

```shell
$ make defconfig	        ### 使用当前平台的默认配置，在x86机器上会使用x86的默认配置
$ make -j$(nproc)	        ### 编译当前平台的内核，-j$(nproc)为以机器硬件线程数进行多线程编译

$ make ARCH=riscv defconfig	### 使用RISC-V平台的默认配置
$ make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc)     ### 编译RISC-V平台内核

$ make clean	                ### 清除所有编译好的object文件
$ make mrproper	                ### 清除编译的配置文件，中间文件和结果文件

$ make init/main.o	        ### 编译当前平台的单个object文件init/main.o（会同时编译依赖的文件）
```

## 附录 LAB1

### A. Makefile介绍

Makefile是一种实现关系整个工程编译规则的文件，通过自动化编译极大提高了软件开发的效率。一个工程中源文件，按类型，功能，模块分别放在若干个目录中，Makefile定义了一系列规则来指定哪些文件需要先编译，哪些文件需要后编译，哪些文件需要重新编译，甚至可以执行操作系统的命令。

以C为例，源文件首先会生成中间目标文件，再由中间目标文件生成执行文件。在编译时，编译器只检测程序语法和函数、变量是否被声明。如果函数未被声明，编译器会给出一个警告，但可以生成Object File。而在链接程序时，链接器会在所有的Object File中找寻函数的实现，如果找不到，那到就会报告链接错误码。

Lab0中我们已经使用了make工具利用Makefile文件来管理整个工程。请阅读[Makefile介绍 ](https://seisman.github.io/how-to-write-makefile/introduction.html)，根据工程文件夹里Makefile的代码与注释来掌握一些基本的使用技巧。

### B. RISC-V指令集

请**下载并认真阅读[RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)，**掌握基础知识、基本命令及特权架构相关内容。

实验中涉及了特权相关的重要寄存器，在中文手册中进行了简要介绍，其具体布局以及详细介绍可以参考[The RISC-V Instruction Set Manual Volume II: Privileged Architecture Version 1.9.1](https://riscv.org/wp-content/uploads/2016/11/riscv-privileged-v1.9.1.pdf)。

#### RISC-V汇编指令

常用的汇编指令包括`la`、`li`、`j` 、`ld`等，可以自行在 [RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)附录中了解其用法。

RISC-V指令集中有一类**特殊寄存器CSRs(Control and Status Registers)**，这类寄存器存储了CPU的相关信息，只有特定的控制状态寄存器指令 (csrrc、csrrs、csrrw、csrrci、csrrsi、csrrwi等)才能够读写CSRs。例如，保存`sepc`的值至内存时需要先使用相应的CSR指令将其读入寄存器，再通过寄存器保存该值，写入sepc时同理。

```asm
csrr t0, sepc
sd t0, 0(sp)
```

#### RISC-V特权模式

RISC-V有三个特权模式：U（user）模式、S（supervisor）模式和M（machine）模式。它通过设置不同的特权级别模式来管理系统资源的使用。其中M模式是最高级别，该模式下的操作被认为是安全可信的，主要为对硬件的操作；U模式是最低级别，该模式主要执行用户程序，操作系统中对应于用户态；S模式介于M模式和U模式之间，操作系统中对应于内核态，当用户需要内核资源时，向内核申请，并切换到内核态进行处理。

本实验主要在S模式运行，通过调用运行在M模式的OpenSBI提供的接口操纵硬件。

| Level | Encoding | Name             | Abbreviation |
| ----- | -------- | ---------------- | ------------ |
| 0     | 00       | User/Application | U            |
| 1     | 01       | Supervisor       | S            |
| 2     | 10       | Reserved         |              |
| 3     | 11       | Machine          | M            |

### B. OpenSBI介绍

SBI (Supervisor Binary Interface)是 S-Mode 的 kernel 和 M-Mode 执行环境之间的标准接口，而OpenSBI项目的目标是为在M模式下执行的平台特定固件提供RISC-V SBI规范的开源参考实现。为了使操作系统内核可以适配不同硬件，OpenSBI提出了一系列规范对m-mode下的硬件进行了抽象，运行在s-mode下的内核可以按照标准对这些硬件进行操作。

**为降低实验难度，我们将选择OpenSBI作为bios来进行机器启动时m模式下的硬件初始化与寄存器设置，并使用OpenSBI所提供的接口完成诸如字符打印的操作。**

![](https://raw.githubusercontent.com/riscv/riscv-sbi-doc/master/riscv-sbi-intro1.png)

#### RISC-V启动过程

![启动过程](https://mianbaoban-assets.oss-cn-shenzhen.aliyuncs.com/2020/12/eU3yIz.png) 
上图是RISC-V架构计算机的启动过程：

- ZSBL(Zeroth Stage Boot Loader)：片上ROM程序，烧录在硬件上，是芯片上电后最先运行的代码。它的作用是加载FSBL到指定位置并运行。
- FSBL(First Stage Boot Loader ）：启动PLLs和初始化DDR内存，对硬件进行初始化，加载下一阶段的bootloader。
- OpenSBI：运行在m模式下的一套软件，提供接口给操作系统内核调用，以操作硬件，实现字符输出及时钟设定等工作。OpenSBI就是一个开源的RISC-V虚拟化二进制接口的通用的规范。
- Bootloader：OpenSBI初始化结束后会通过mret指令将系统特权级切换到s模式，并跳转到操作系统内核的初始化代码。这一阶段，将会完成中断地址设置等一系列操作。之后便进入了操作系统。

更多内容可参考[An Introduction to RISC-V Boot Flow](https://riscv.org/wp-content/uploads/2019/12/Summit_bootflow.pdf)。

为简化实验，从ZSBL到OpenSBI运行这一阶段的工作已通过QEMU模拟器完成。运行QEMU时，我们使用-bios default选项将OpenSBI代码加载到0x80000000起始处。OpenSBI初始化完成后，会跳转到0x80200000处。因此，我们所编译的代码需要放到0x80200000处。


### C. 内联汇编

我们在用c语言写代码的时候会遇到直接对寄存器进行操作的情况，这时候就需要用的内联汇编了。内联汇编的详细使用方法可参考[**GCC内联汇编**](https://mp.weixin.qq.com/s/Ln4qBYvSsgRvdiK1IJqI6Q)。

在此给出一个简单示例。其中，三个`:`将汇编部分分成了四部分。

第一部分是汇编指令，指令末尾需要添加'\n'。指令中以`%[name]`形式与输入输出操作数中的同名操作符`[name]`绑定，也可以通过`%数字`的方式进行隐含指定。假设输出操作数列表中有1个操作数，“输入操作数”列表中有2个操作数，则汇编指令中`%0`表示第一个输出操作数，`%1`表示第一个输入操作数，`%2`表示第二个输入操作数。

第二部分是输出操作数，第三部分是输入操作数。输入或者输出操作符遵循`[name] "CONSTRAINT" (variable)`格式，由三部分组成：

* `[name]`：符号名用于同汇编指令中的操作数通过同名字符绑定。

* `"constraint"`：限制字符串，用于约束此操作数变量的属性。字母`r`代表使用编译器自动分配的寄存器来存储该操作数变量，字母`m`代表使用内存地址来存储该操作数变量，字母`i`代表立即数。

  对于输出操作数而言，等号“=”代表输出变量用作输出，原来的值会被新值替换；加号“+”代表输出变量不仅作为输出，还作为输入。此约束不适用于输入操作数。

* `(variable)`：C/C++变量名或者表达式。

示例中，输出操作符`[ret_val] "=r" (ret_val)`代表将汇编指令中`%[ret_val]`的值更新到变量ret_val中，输入操作符`[type] "r" (type)`代表着将()中的变量`type`放入寄存器中。

第四部分是可能影响的寄存器或存储器，用于告知编译器当前内联汇编语句可能会对某些寄存器或内存进行修改，使得编译器在优化时将其因素考虑进去。

```asm
unsigned long long s_example(unsigned long long type,unsigned long long arg0) {
    unsigned long long ret_val;
    __asm__ volatile (
    	#汇编指令
        "mv x10, %[type]\n"
        "mv x11,%[arg0]\n"
        "mv %[ret_val], x12"
        #输出操作数
        : [ret_val] "=r" (ret_val)
        #输入操作数
        : [type] "r" (type), [arg0] "r" (arg0)
        #可能影响的寄存器或存储器
        : "memory"
    );
    return ret_val;
}
```

示例二定义了一个宏，其中`%0`代表着输入输出部分的第一个符号，即`val`。其中`#reg`是c语言的一个特殊宏定义语法，相当于将`reg`进行宏替换并用双引号包裹起来。例如`write_csr(sstatus,val)`宏展开会得到：
`({asm volatile ("csrw " "sstatus" ", %0" :: "r"(val)); })`

```C
#define write_csr(reg, val) ({
    asm volatile ("csrw " #reg ", %0" :: "r"(val)); })
```

### D. Linux Basic

#### vmlinux

vmlinux通常指Linux Kernel编译出的可执行文件（Executable and Linkable Format， ELF），特点是未压缩的，带调试信息和符号表的。在本实验中，vmlinux通常指将你的代码进行编译，链接后生成的可供QEMU运行的RISC-V 64-bit架构程序。如果对vmlinux使用**file**命令，你将看到如下信息：

```shell
$ file vmlinux 
vmlinux: ELF 64-bit LSB executable, UCB RISC-V, version 1 (SYSV), statically linked, not stripped
```

#### System.map

System.map是内核符号表（Kernel Symbol Table）文件，是存储了所有内核符号及其地址的一个列表。使用System.map可以方便地读出函数或变量的地址，为Debug提供了方便。“符号”通常指的是函数名，全局变量名等等。使用`nm vmlinux`命令即可打印vmlinux的符号表，符号表的样例如下：

```asm
0000000000000800 A __vdso_rt_sigreturn
ffffffe000000000 T __init_begin
ffffffe000000000 T _sinittext
ffffffe000000000 T _start
ffffffe000000040 T _start_kernel
ffffffe000000076 t clear_bss
ffffffe000000080 t clear_bss_done
ffffffe0000000c0 t relocate
ffffffe00000017c t set_reset_devices
ffffffe000000190 t debug_kernel
```

#### vmlinux.lds

GNU ld即链接器，用于将\*.o文件（和库文件）链接成可执行文件。在操作系统开发中，为了指定程序的内存布局，ld使用链接脚本（Linker Script）来控制，在Linux Kernel中链接脚本被命名为vmlinux.lds。更多关于ld的介绍可以使用`man ld`命令。

下面给出一个vmlinux.lds的例子：

```asm
/* 目标架构 */
OUTPUT_ARCH( "riscv" )
/* 程序入口 */
ENTRY( _start )
/* 程序起始地址 */
BASE_ADDR = 0x80000000;
SECTIONS
{
  /* . 代表当前地址 */
  . = BASE_ADDR;
  /* code 段 */
  .text : { *(.text) }
  /* data 段 */
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) }
  . += 0x8000;
  /* 栈顶 */
  stack_top = .;
  /* 程序结束地址 */
  _end = .;
}
```

首先我们使用OUTPUT_ARCH指定了架构为RISC-V，之后使用ENTRY指定程序入口点为`_start`函数，程序入口点即程序启动时运行的函数，经过这样的指定后在head.S中需要编写`_start`函数，程序才能正常运行。

链接脚本中有`.` `*`两个重要的符号。单独的`.`在链接脚本代表当前地址，它有赋值、被赋值、自增等操作。而`*`有两种用法，其一是`*()`在大括号中表示将所有文件中符合括号内要求的段放置在当前位置，其二是作为通配符。

链接脚本的主体是SECTIONS部分，在这里链接脚本的工作是将程序的各个段按顺序放在各个地址上，在例子中就是从0x80000000地址开始放置了.text，.rodata，.data和.bss段。各个段的作用可以简要概括成：

| 段名    | 主要作用                             |
| ------- | ------------------------------------ |
| .text   | 通常存放程序执行代码                 |
| .rodata | 通常存放常量等只读数据               |
| .data   | 通常存放已初始化的全局变量、静态变量 |
| .bss    | 通常存放未初始化的全局变量、静态变量 |

在链接脚本中可以自定义符号，例如stack_top与_end都是我们自己定义的，其中stack_top与程序调用栈有关。

更多有关链接脚本语法可以参考[这里](https://sourceware.org/binutils/docs/ld/Scripts.html)。

#### Image

在Linux Kernel开发中，为了对vmlinux进行精简，通常使用objcopy丢弃调试信息与符号表并生成二进制文件，这就是Image。Lab0中QEMU正是使用了Image而不是vmlinux运行。

```shell
$ objcopy -O binary vmlinux Image --strip-all
```

此时再对Image使用**file**命令时：

```shell
$ file Image 
image: data
```



## 附录 LAB2

### A. RISC-V中的异常

异常(trap)是指是不寻常的运行时事件，由硬件或软件产生，当异常产生时控制权将会转移至异常处理程序。异常是操作系统最基础的概念，一个没有异常的操作系统无法进行正常交互。

RISC-V将异常分为两类。一类是硬件中断(interrupt)，它是与指令流异步的外部事件，比如鼠标的单击。另外一类是同步异常(exception)，这类异常在指令执行期间产生，如访问了无效的存储器地址或执行了具有无效操作码的指令。

这里我们用异常(trap)作为硬件中断(interrupt)和同步异常(exception)的集合，另外trap指的是发生硬件中断或者同步异常时控制权转移到handler的过程。

> 后文统一用异常指代trap，中断/硬件中断指代interrupt，同步异常指代exception。

### B. M模式下的异常

#### 1. 硬件中断的处理（以时钟中断为例）

简单地来说，中断处理经过了三个流程：中断触发、判断处理还是忽略、可处理时调用处理函数。

* 中断触发：时钟中断的触发条件是这个hart（硬件线程）的时间比较器`mtimecmp`小于实数计数器`mtime`。

* 判断是否可处理：

  当时钟中断触发时，并不一定会响应中断信号。M模式只有在全局中断使能位`mstatus[mie]`置位时才会产生中断，如果在S模式下触发了M模式的中断，此时无视`mstatus[mie]`直接响应，即运行在低权限模式下，高权限模式的全局中断使能位一直是enable状态。

  此外，每个中断在控制状态寄存器`mie`中都有自己的使能位，对于特定中断来说，需要考虑自己对应的使能位，而控制状态寄存器`mip`中又指示目前待处理的中断。

  **以时钟中断为例，只有当`mstatus[mie]`=1，`mie[mtie]`=1，且`mip[mtip]`=1时，才可以处理机器的时钟中断。**其中`mstatus[mie]`以及`mie[mtie]`需要我们自己设置，而`mip[mtip]`在中断触发时会被硬件自动置位。

* 调用处理函数：

  当满足对应中断的处理条件时，硬件首先会发生一些状态转换，并跳转到对应的异常处理函数中，在异常处理函数中我们可以通过分析异常产生的原因判断具体为哪一种，然后执行对应的处理。

  为了处理异常结束后不影响hart正常的运行状态，我们首先需要保存当前的状态即上下文切换。我们可以先用栈上的一段空间来把**全部**寄存器保存，保存完之后执行到我们编写的异常处理函数主体，结束后退出。

#### 2. M模式下的异常相关寄存器

M模式异常需要使用的寄存器首先有lab1提到的`mstatus`，`mip`，`mie`，`mtvec`寄存器，这些寄存器需要我们操作；剩下还有`mepc`，`mcause`寄存器，这些寄存器在异常发生时**硬件会自动置位**，它们的功能如下：

* `mepc`：存放着中断或者异常发生时的指令地址，当我们的代码没有按照预期运行时，可以查看这个寄存器中存储的地址了解异常处的代码。通常指向异常处理后应该恢复执行的位置。

* `mcause`：存储了异常发生的原因。

* `mstatus`：Machine Status Register，其中m代表M模式。此寄存器中保持跟踪以及控制hart(hardware thread)的运算状态。通过对`mstatus`进行位运算，可以实现对不同bit位的设置，从而控制不同运算状态。

* `mie`、`mip`：`mie`以及`mip`寄存器是Machine Interrup Registers，用来保存中断相关的一些信息，通过`mstatus`上mie以及mip位的设置，以及`mie`和`mip`本身两个寄存器的设置可以实现对硬件中断的控制。注意mip位和`mip`寄存器并不相同。

* `mtvec`：Machine Trap-Vector Base-Address Register，主要保存M模式下的trap vector（可理解为中断向量）的设置，包含一个基地址以及一个mode。 

与时钟中断相关的还有`mtime`和`mtimecmp`寄存器，它们的功能如下：

* `mtime`：Machine Time Register。保存时钟计数，这个值会由硬件自增。
* `mtimecmp`：Machine Time Compare Register。保存需要比较的时钟计数，当`mtime`的值大于或等于`mtimecmp`的值时，触发时钟中断。

需要注意的是，`mtime`和`mtimecmp`寄存器需要用MMIO的方式即使用内存访问指令（sd，ld等）的方式交互，可以将它们理解为M模式下的一个外设。

事实上，异常还与`mideleg`和`medeleg`两个寄存器密切相关，它们的功能将在**S模式下的异常**部分讲解，主要用于将M模式的一些异常处理委托给S模式。

#### 3. 同步异常的处理

同步异常的触发条件是当前指令执行了未经定义的行为，例如：

* Illegal instruction：跳过判断可以处理还是忽略的步骤，硬件会直接经历一些状态转换，然后跳到对应的异常处理函数。
* 环境调用同步异常ecall：主要在低权限的mode需要高权限的mode的相关操作时使用的，比如系统调用时U-mode call S-mode ，在S-mode需要操作某些硬件时S-mode call M-mode。

需要注意的是，不管是中断还是同步异常，都会经历相似的硬件状态转换，并跳到**同一个**异常处理地址（由`mtvec`/`stvec`寄存器指定），异常处理函数根据`mcause`寄存器的值判断异常出现原因，针对不同的异常进行不同的处理。

### B. S模式下的异常

由于hart位于S模式，我们需要在S模式下处理异常。这时首先要提到委托（delegation）机制。

#### 1. 委托机制

**RISC-V架构所有mode的异常在默认情况下都跳转到M模式处理**。为了提高性能，RISC-V支持将低权限mode产生的异常委托给对应mode处理，该过程涉及了`mideleg`和`medeleg`这两个寄存器。

* `mideleg`：Machine Interrupt Delegation。该寄存器控制将哪些中断委托给S模式处理，它的结构可以参考`mip`寄存器，如`mideleg[5]`对应于 S模式的时钟中断，如果把它置位， S模式的时钟中断将会移交 S模式的异常处理程序，而不是 M模式的异常处理程序。
* `medeleg`：Machine Exception Delegation。该寄存器控制将哪些同步异常委托给对应mode处理，它的各个位对应`mcause`寄存器的返回值。

#### 2. S模式下时钟中断处理流程

事实上，即使在`mideleg`中设置了将S模式产生的时钟中断委托给S模式，委托仍未完成，因为硬件产生的时钟中断仍会发到M模式（`mtime`寄存器是M模式的设备），所以我们需要**手动触发S模式下的时钟中断**。

此前，假设设置好`[m|s]status`以及`[m|s]ie`，即我们已经满足了时钟中断在两种mode下触发的使能条件。接下来一个时钟中断的委托流程如下：

1. 当`mtimecmp`小于`mtime`时，**触发M模式时钟中断**，硬件**自动**置位`mip[mtip]`。

2. 此时`mstatus[mie]`=1，`mie[mtie]`=1，且`mip[mtip]`=1 表示可以处理M模式的时钟中断。

3. 此时hart发生了异常，硬件会自动经历状态转换，其中`pc`被设置为`mtvec`的值，即程序将跳转到我们设置好的M模式处理函数入口。

   （注：`pc`寄存器是用来存储指向下一条指令的地址，即下一步要执行的指令代码。）

4. M模式处理函数将分析异常原因，判断为时钟中断，为了将时钟中断委托给S模式，于是将`mip[stip]`置位，并且为了防止在S模式处理时钟中断时继续触发M模式时钟中断，于是同时将`mie[mtie]`清零。

5. M模式处理函数处理完成并退出，此时`sstatus[sie]`=1，`sie[stie]`=1，且`sip[stip]`=1(由于sip是mip的子集，所以第4步中令`mip[stip]`置位等同于将`sip[stip]`置位)，于是**触发S模式的时钟中断**。

6. 此时hart发生了异常，硬件自动经历状态转换，其中`pc`被设置为stvec，即跳转到我们设置好的S模式处理函数入口。

7. S模式处理函数分析异常原因，判断为时钟中断，于是进行相应的操作，然后利用`ecall`触发异常，跳转到M模式的异常处理函数进行最后的收尾。

8. M模式异常处理函数分析异常原因，发现为ecall from S-mode，于是设置`mtimecmp`+=100000，将`mip[stip]`清零，表示S模式时钟中断处理完毕，并且设置`mie[mtie]`恢复M模式的中断使能，保证下一次时钟中断可以触发。

9. 函数逐级返回，整个委托的时钟中断处理完毕。

#### 3. 中断前后硬件的自动转换

当`mtime`寄存器中的的值大于`mtimecmp`时，`sip[stip]`会被置位。此时，如果`sstatus[sie]`与`sie[stie]`也都是1，硬件会自动经历以下的状态转换（这里只列出S模式下的变化）：

* 发生异常的时`pc`的值被存入`sepc`，且`pc`被设置为`stvec`。
* `scause`按图 10.3根据异常类型设置，`stval`被设置成出错的地址或者其它特定异
  常的信息字。
* `sstatus` CSR中的 SIE 位置零，屏蔽中断，且中断发生前的`sstatus[sie]`会被存入`sstatus[spie]`。
* 发生异常时的权限模式被保存在`sstatus[spp]`，然后设置当前模式为 S模式。 

在我们处理完中断或异常，并将寄存器现场恢复为之前的状态后，**我们需要用`sret`指令回到之前的任务中**。`sret`指令会做以下事情：

- 将`pc`设置为`sepc`。
- 通过将 `sstatus`的 SPIE域复制到`sstatus[sie]`来恢复之前的中断使能设置。
- 并将权限模式设置为`sstatus[spp]`。

### B. gef插件

1. 新建gdbtool文件夹（方便使用其他插件）

   ```shell
   $ cd ~
   $ mkdir gdbtool
   ```

2. 新建gef.py插件

   ```shell
   $ cd ~/gdbtool
   $ vim gef.py
   (此时将https://github.com/hugsy/gef-legacy/blob/master/gef.py的代码粘贴过来)
   (然后使用:wq保存并退出vim)
   ```

3. 编写gdb配置文件

   ```shell
   $ cd ~
   $ echo "source ~/gdbtool/gef.py" > .gdbinit
   ```

4. 此时已经成功添加插件

   ```shell
   $ riscv64-unknown-linux-gnu-gdb
   (可以查看对应显示为gef)
   ```

5. 若要使用其他插件

   * 首先在 ~/gdbtool目录下添加对应插件的py文件
   * 然后使用修改~/.gdbinit文件
