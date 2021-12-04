# Lab 7: A Toy RISC-V OS

## 1 实验目的

利用给定的文件系统和Shell程序，实现从命令行运行给定的用户程序。

## 2 实验目标

- 了解文件系统的实现，学会使用给定的文件系统读取文件内容。
- 理解ELF文件格式，实现简单的ELF Loader，能够解析ELF文件，并将需要的segment装载到内存。
- 实现exec，wait，exit系统调用。
- 使用给定的Shell，能够解析命令行输入，获取需要执行的用户态程序名，运行给定的用户态程序。

## 3 实验环境

- Docker Image

## 4 背景知识

### 4.1 initramfs

文件系统是一套实现了数据的存储、分级组织、存取和获取等操作的抽象数据类型，其底层的操作的文件可能存储在物理的光盘、硬盘上，也可能是来自于网络、内存。在本实验中，我们使用的是initranfs临时文件系统，进一步的，是使用了newc规范cpio格式的镜像。
在Linux启动的过程中，所有配置信息、工具、库等文件会被存入initramfs中，内核将创建一个tmpfs文件系统访问其中的内容，主要是使用/init来完成对内核的初始化，同学们可以观察自己当前使用的Linux中使用了哪个程序/脚本作为init：

```
$ sudo cat /var/log/kern.log | grep "/init"
Jan  3 17:08:55 phantom-workstation kernel: [    0.954328] Run /init as init process

### 若使用了 /init，且根目录下没有init文件，可以检查/boot/grub/grub.cfg中使用了哪个initrd镜像
$ sudo unmkinitramfs -v /boot/initrd.img-5.4.0-59-generic  .

$ tail main/init
...
exec run-init ${drop_caps} "${rootmnt}" "${init}" "$@" <"${rootmnt}/dev/console" >"${rootmnt}/dev/console" 2>&1
...
```

可以看到，init进程一旦挂载了根文件系统和其他重要文件系统，就会将根切换到真实的根文件系统，并最终在该系统上调用/sbin/init（${init}）二进制文件以继续启动过程，有关cpio newc的相关介绍参考[此处](http://zju.phvntom.tech/markdown/md2html.php?id=md/2020-12/cpio.md)。

在本实验中我们提供了一个简单的cpio读取demo，同学们需要在使用时需要自行完成映射，initramfs的物理位置与lab5中一致，位于0x84000000，完成映射后使用namei定位文件，readi读取文件。
namei和readi函数的声明如下：

```C
struct inode *namei(char *path);
int readi(struct inode *ip, int user_dst, void *dst, uint off, uint n);
```

### 4.2 ELF文件格式与Loader

- 本部分有关ELF和Loader的内容参考了[《程序员的自我修养--链接、装载与库》](https://item.jd.com/10067200.html)，[【man elf】](https://man7.org/linux/man-pages/man5/elf.5.html)以及[【Executable and Linkable Format】](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)

#### 4.2.1 什么是ELF文件格式

可执行与可链接格式，即Executable and Linkable Format，缩写为ELF，常被称为ELF格式，在计算机科学中，是一种用于可执行文件、目标文件、共享库和核心转储(core dump)的标准文件格式。1999年，被86open项目选为x86架构上的类Unix操作系统的二进制文件格式标准，用来取代COFF。因其可扩展性与灵活性，也可应用在其它处理器、计算机系统架构的操作系统上。

#### 4.2.2 ELF Header

下图描述了ELF文件的总体结构，我们省去了ELF一些繁琐的结构，把最重要的结构提取出来。

```
         +--------------------+
         |                    |
         |     ELF Header     |
         |                    |
         +--------------------+
         |                    |
+--------+Program Header Table|
|        |                    |
|     +-----------------------+
|     |  |                    |
|     |  |       .text        |  <-----------+
|     |  |                    |              |
+---> |  +--------------------+              |
|     |  |                    |              |
|     |  |      .rodata       |  <-----------+
|     |  |                    |              |
|     +-----------------------+              |
|        |                    |              |
|     +-----------------------+              |
|     |  |                    |              |
+---> |  |       .data        |  <-----------+
      |  |                    |              |
      +-----------------------+              |
         |                    |              |
         |        .bss        |  <-----------+
         |                    |              |
         +--------------------+              |
         |                    |              |
         |Section Header Table+--------------+
         |                    |
         +--------------------+
```

ELF文件格式的最前部是ELF文件头（ELF Header），它描述了整个文件的基本属性，比如ELF文件版本，目标机器型号，程序入口地址等。紧接着是ELF文件各个section。 我们可以用readelf命令来详细查看ELF文件：

```bash
# 查看ELF文件头
$ riscv64-unknown-elf-readelf -h hello.elf
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           RISC-V
  Version:                           0x1
  Entry point address:               0x0
  Start of program headers:          64 (bytes into file)
  Start of section headers:          13576 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         1
  Size of section headers:           64 (bytes)
  Number of section headers:         22
  Section header string table index: 21
```

从上面的输出结果可以看到，在ELF的文件头中定义了ELF魔数（0x464C457F），文件机器字节长度（64bit），数据存储方式（小端），文件类型（EXEC），机器类型（RISC-V），ELF程序的入口虚拟地址（0x0），program header table的位置、长度、其中包含的program header的数量，section header table的位置、长度、其中包含的sectio header的数量等信息。有关section header与program header的区别我们将在下一小节介绍。
我们用以下的结构体 `elfhdr`来解析ELF header：

```c
/* elf.h */
// ELF file header struct
struct elfhdr {
    uint magic;            // must equal ELF_MAGIC, i.e., 0x464C457F
    uchar elf[12];
    ushort type;
    ushort machine;
    uint version;
    uint64 entry;
    uint64 phoff;        // program header table距离ELF文件开始的偏移(单位：字节)
    uint64 shoff;        // section header table距离ELF文件开始的偏移(单位：字节)
    uint flags;
    ushort ehsize;
    ushort phentsize;    // 单个program header的大小(单位：字节)
    ushort phnum;        // program header数量
    ushort shentsize;    // 单个section header的大小(单位：字节)
    ushort shnum;        // section header数量
    ushort shstrndx;
};
```

#### 4.2.3 ELF文件的链接视图和执行视图

我们可以从链接视图和执行视图来分别理解一个ELF文件。

##### 链接视图

ELF文件的section header table包含了ELF文件中的sections。在链接的时候，链接器会合并相同性质的section，比如将所有输入文件的".text"合并到输出文件的".text"段，接着是".rodata"段，".data"段，".bss"段等。我们可以用readelf查看一个ELF文件的section信息，例如：

```bash
# 查看ELF文件的链接视图
$ riscv64-unknown-elf-readelf -S hello
There are 15 section headers, starting at offset 0x4d38:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         00000000000100b0  000000b0
       00000000000025fc  0000000000000000  AX       0     0     2
  [ 2] .rodata           PROGBITS         00000000000126b0  000026b0
       0000000000000012  0000000000000000   A       0     0     8
  [ 3] .eh_frame         PROGBITS         00000000000136c4  000026c4
       0000000000000004  0000000000000000  WA       0     0     4
  [ 4] .init_array       INIT_ARRAY       00000000000136c8  000026c8
       0000000000000010  0000000000000008  WA       0     0     8
  [ 5] .fini_array       FINI_ARRAY       00000000000136d8  000026d8
       0000000000000008  0000000000000008  WA       0     0     8
  [ 6] .data             PROGBITS         00000000000136e0  000026e0
       0000000000000f58  0000000000000000  WA       0     0     8
  [ 7] .sdata            PROGBITS         0000000000014638  00003638
       0000000000000040  0000000000000000  WA       0     0     8
  [ 8] .sbss             NOBITS           0000000000014678  00003678
       0000000000000028  0000000000000000  WA       0     0     8
  [ 9] .bss              NOBITS           00000000000146a0  00003678
       0000000000000060  0000000000000000  WA       0     0     8
  [10] .comment          PROGBITS         0000000000000000  00003678
       0000000000000012  0000000000000001  MS       0     0     1
  [11] .riscv.attributes RISCV_ATTRIBUTE  0000000000000000  0000368a
       0000000000000035  0000000000000000           0     0     1
  [12] .symtab           SYMTAB           0000000000000000  000036c0
       0000000000000f60  0000000000000018          13    79     8
  [13] .strtab           STRTAB           0000000000000000  00004620
       0000000000000698  0000000000000000           0     0     1
  [14] .shstrtab         STRTAB           0000000000000000  00004cb8
       000000000000007e  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  p (processor specific)
```

##### 执行视图

当可执行文件被装载到内存空间时，如果操作系统以ELF文件的section为单位，为每个section分配单独的内存，那么会造成内存的浪费。【思考题，为什么会造成内存浪费】
其实操作系统装载可执行文件时，并不关心可执行文件各个段所包含的实际内容，操作系统只关心一些跟装载相关的问题，最主要的是section的权限（可读、可写、可执行），而section的权限往往只有为数不多的几种组合，即：

- 可读可执行，以.text段为代表
- 可读可写，以.data, .bss段为代表
- 只读数据段，以.rodata段为代表

因此，为了节省空间，操作系统会把权限相同的sections合并到一起当作一个**segment**进行映射。我们同样可以用readelf查看ELF文件的segment信息：

```bash
# 查看ELF文件的执行视图
$ riscv64-unknown-elf-readelf -l hello

Elf file type is EXEC (Executable file)
Entry point 0x100c2
There are 2 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000000000 0x0000000000010000 0x0000000000010000
                 0x00000000000026c2 0x00000000000026c2  R E    0x1000
  LOAD           0x00000000000026c4 0x00000000000136c4 0x00000000000136c4
                 0x0000000000000fb4 0x000000000000103c  RW     0x1000

 Section to Segment mapping:
  Segment Sections...
   00     .text .rodata
   01     .eh_frame .init_array .fini_array .data .sdata .sbss .bss
```

可执行文件 `hello`有2个segment，第1个segment包含了.data .sdata .sbss .bss等这些可读可写的section。【思考题：为什么第0个segment会将只读的.rodata赋给执行权限？】

描述segment的结构体叫做程序头（program header），它描述了ELF文件该如何被操作系统映射到进程的虚拟虚拟空间。我们可以用结构体 `proghdr`来解析program header。结构体每个成员的含义也已给出：

```c
/* elf.h */
// ELF program header struct
struct proghdr {
    uint32 type;    // type的值决定了segment的类型，在本实验中我们只需要关注LOAD类型的segment，即type值为1的segment
    uint32 flags;   // segment的权限属性，包括R,W,X
    uint64 off;     // segment在ELF文件中的偏移(单位：字节)
    uint64 vaddr;   // segment的第一个字节在进程虚拟地址空间的起始位置。整个program header table中，所有“LOAD”类型的segment按照vaddr从小到大排列
    uint64 paddr;   // segment的物理装载地址，一般与vaddr一样
    uint64 filesz;  // segment在ELF文件中所占空间的长度(单位：字节)
    uint64 memsz;   // segment在进程虚拟地址空间所占用的长度(单位：字节)
    uint64 align;   // segment的对齐属性。实际对齐字节等于2的align次。例如当align等于10，那么实际的对齐属性就是2的10次方，即1024字节
};
```

#### 4.2.4 可执行文件的装载

可执行文件需要装载到内存中才能成为进程，并被CPU执行。可执行文件装载的流程如下：
(1)创建一个独立的虚拟地址空间
(2)读取可执行文件头，为进程分配物理内存，读取并复制可执行文件的LOAD类型segment到进程虚拟地址空间中
(3)将PC寄存器设置为可执行文件的入口地址，启动运行

【注意】本实验与Linux的实现有所不同，在读取并复制可执行文件的segment之前分配好了物理内存

### 4.3 系统调用

沿用Lab5中添加syscall的方法，本次实验中要求实现的系统调用函数原型以及具体功能如下：

#### 4.3.1 实现281号系统调用: exec

```c
// 同学们可直接复制sys_exec()以及exec()
uint64 sys_exec(const char *path) {
    int ret = exec(path);
    return ret;
}

int exec(const char *path) {
    int ret = proc_exec(current, path);
    return ret;
}

// 需要同学们实现proc_exec函数
int proc_exec(struct task_struct *proc, const char *path) {
    ...
}
```

- exec的功能是将当前进程正在执行的可执行文件 替换为 一个新的可执行文件。在Shell中，先执行fork，复制父进程的虚拟内存空间到子进程；然后子进程再执行exec，替换为新的可执行文件。
- 函数参数`proc`表示当前进程的task_struct, 参数`path`表示可执行文件的路径
- 执行成功返回0，执行失败则返回-1

#### 4.3.2 实现260号系统调用:wait, 93号系统调用exit

- 考虑到本次实验没有添加信号机制，wait以及exit的实现由我们自行设计，满足简单的功能。同时需要同学们自行分析所需参数及返回值，只要实现父进程可以等待子进程的功能即可。
- wait系统调用的简单的伪代码描述如下：

```c
int sys_wait(uintptr_t *regs) {
    set self as pending state;
     schedule;
}
```

- wait系统调用的功能是将当前进程的state设置为 `TASK_SLEEPING`
- 执行成功返回0，执行失败则返回-1
- exit系统调用的简单的伪代码描述如下：

```c
void sys_exit() {
     while(true) {
          if( parent is pending )
               break;
          else
               schedule to give up cpu;
     }
     set parent as running state;
     schedule to give up cpu;
}
```

- 更多内容可以参考[【Linux系统调用号】](https://elixir.bootlin.com/linux/latest/source/include/uapi/asm-generic/unistd.h)

### 4.4 Shell原理

本次使用的shell为一个简单的user program，实现简单的shell功能，并进行简单的交互。其简单的伪代码描述如下：

```C
int main(void)
{
	while(1) {
		printf("$ ");
		if( gets(buf) < 0)
			break;
		if(fork() == 0) {
			exec(buf);
		}
		wait(0);
	}
	exit(0);
}
```

## 5 实验步骤

### 5.1 环境搭建

#### 5.1.1 建立映射

同lab5的文件夹映射方法，目录名为lab7

#### 5.1.2 组织文件结构

```
.
├── arch
│   └── riscv
│       ├── include
│       │   ...
│       │   └── elf.h
│       ├── kernel
│       │   ...
│       │   └── exec.c
│       └── Makefile
├── driver
│   ...
├── fs
│   ├── cpio.c
│   └── Makefile
├── include
|   ...
│   └── fs.h
├── init
│   ...
├── lib
│   ...
├── Makefile
└── user
    ├── *.h
    ├── Makefile
    └── shell.c
```

### 5.2 实现简单的ELF Loader

实现简单的ELF Loader实际上就是实现一个loadseg函数，将 `LOAD`类型的segment的内容复制到内存的某一个位置，其伪代码描述如下：

```c
int loadseg(pagetable_t pagetable, uint64 va, struct inode *ip, uint offset, uint filesz) {
    check whether va is aligned to PAGE_SIZE;
    for(i=0; i<filesz; i+=PAGE_SIZE) {
        walk the pagetable, get the corresponding pa of va;
        use readi() to read the content of segment to address pa;
    }
}
```

参数说明：

- pagetable: page table to map the va of the segments
- va: proghdr.vaddr
- ip: point to the inode of the elf file
- offset: proghdr.offset
- filesize: proghdr.filesz

### 5.3 实现exec系统调用

函数proc_exec简单的伪代码描述如下

```C
int proc_exec(struct task_struct *proc, const char *path) {
    inode = namei(path);
    create a new page table newpgtbl, copy kernel page table from kpgtbl (same as lab5);
    readi() read the elf header(denoted as elf_header) from elf file;
    check whether elf_header.magic == ELF_MAGIC;
    for(i=0; i<elf_header.phnum; i++) {
        readi() read the program header(denoted as prog_header) of each segment;
        check whether prog_header.type == LOAD;
        parse_ph_flags() parse the prog_header.flags to permissions;
        uvmalloc() allocate user pages for [prog_header.vaddr, prog_header.vaddr+prog_header.memsz] of this segment in newpgtbl, and set proper permissions;
        loadseg() copy the content of this segment from prog_header.off to its just allocated memory space;
    }
    allocate a page for user stack and update the newpgtbl for it. The user stack va range: [USER_END-PAGE_SIZE, USER_END];
    set proc's sstatus, sscratch, and sepc like task_init() in Lab5;
    set proc->pgtbl to newpgtbl;
}
```

### 5.4 实现exit, wait系统调用

考虑到本次实验没有添加信号机制，wait以及exit的实现由我们自行设计，满足简单的功能。简单的伪代码描述如下：

```c
sys_exit() {
     while(true) {
          if( parent is pending )
               break;
          else
               schedule to give up cpu;
     }
     set parent as running state;
     schedule to give up cpu;
}
sys_wait() {
     set self as pending state;
     schedule;
}
```

同时需要同学们自行分析所需参数及返回值，只要实现父进程可以等待子进程的功能即可。

### 5.5 修改init.c，调用给定的Shell

启动后内核运行在S-mode，需要切换到U-mode来执行用户程序。本实验我们实现了fork和exec系统调用，但是这些系统调用需要在用户态调用，那么如何创建第一个用户态进程呢？

与**lab 5**相近，我们先分配一个task struct，建立用户态程序的映射，根据用户态程序的入口设置好task struct里保存的sepc、准备好寄存器状态使得切换上下文后能返回到用户态程序入口处继续执行，把进程状态设为ready后由调度器调度到用户态程序执行。

在**lab 5**中，我们使用objcopy来跳过了ELF loader的部分，直接建立到binary的映射来实现用户态进程的创建。本实验与**lab 5**不同的是，映射的建立要求通过ELF loader来实现。具体来说需要如下的流程：

```text
elf_loader(task, path)
{
    read file (path) in cpio
    load elf file to page table of task
    set sepc to entry
    set sstatus, sscratch ...
}

task_init()
{
    allocate task struct
    elf_loader(task, "shell") // "shell" is the path of shell
    set task status to READY
}
```

### 5.6 参考输出

```bash
$ hello
[PID = 2] Process fork from [PID = 1] Successfully! counter = 2
[!] Switch from task 1[ffffffff80009180] to task 2[ffffffff800092b0], prio: 3, counter: 2
[User] pid: 2, sp is fffffffeffffffe0
[User] pid: 2, sp is fffffffeffffffe0
[!] Switch from task 2[ffffffff800092b0] to task 0[ffffffff80009050], prio: 5, counter: 1
[!] Switch from task 0[ffffffff80009050] to task 2[ffffffff800092b0], prio: 3, counter: 3
[User] pid: 2, sp is fffffffeffffffe0
[User] pid: 2, sp is fffffffeffffffe0
```

*注：输出的地址信息、打印数量仅供参考*
*由于没有实现回显，在输入 `hello`命令的时候Shell并没有显示，但实际上已经输入了命令*

## 6 实验任务与要求

  请仔细阅读背景知识，完成 5.1 至 5.6 的内容，撰写实验报告, 需提交实验报告以及整个工程的压缩包。

- 实验报告：
  * 各实验步骤截图以及结果分析
  * 回答思考题(2个)
  * 实验结束后心得体会
  * 对实验指导的建议（可选）
- 工程文件
  * 所有source code（确保make clean）
  * 将lab7_319010XXXX目录压缩成zip格式（若分组，则一组只需要一位同学提交）
- 最终提交
  * 将报告和工程压缩包**分别**提交至学在浙大
