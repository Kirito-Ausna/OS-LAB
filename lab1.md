# Lab 1: RV64 内核引导

## 1 实验目的

学习RISC-V相关知识，了解OpenSBI平台，实现sbi调用函数，封装打印函数，并利用Makefile来完成对整个工程的管理。	

为降低实验难度，我们选择使用OpenSBI作为BIOS来进行机器启动时m模式下的硬件初始化与寄存器设置，并使用OpenSBI所提供的接口完成诸如字符打印等的操作。

## 2 实验内容及要求

* 阅读RISC-V中文手册，学习RISC-V相关知识
* 学习Makefile编写规则，补充Makefile文件使得项目成功运行
* 了解OpenSBI的运行原理，编写代码通过sbi调用实现字符串的打印

请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。请查看文档尾部附录部分的背景知识介绍，跟随实验步骤完成实验，以截图的方式记录命令行的输入与输出，注意进行适当的代码注释。如有需要，请对每一步的命令以及结果进行必要的解释。

在最终提交的实验报告中，**请自行删除附录部分内容**，并命名为“**学号_姓名\_lab1.pdf**"，代码文件请根据给出的结构整理好后打包命名为“**学号_姓名\_lab1**"，分开上传至学在浙大平台。
## 3 实验步骤

### 3.1 搭建实验环境（10%）

按照以下方法创建新的容器，并建立volume映射([参考资料](https://kebingzao.com/2019/02/25/docker-volume/))，在本地编写代码，并在容器内进行编译检查。未来的实验中同样需要用该方法搭建相应的实验环境，但不再作具体指导，请理解每一步的命令并自行更新相关内容。**注意及时关闭运行中的容器**。

#### 1.创建容器并建立映射关系

```shell
### 首先新建自己本地的工作目录(比如lab1)并进入
$ mkdir lab1
$ cd lab1
$ pwd
~/.../lab1

### 查看docker已有镜像(与lab0同一个image)
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
oslab               2020                678605140682        46 hours ago        2.89GB

### 创建新的容器，同时建立volume映射
$ docker run -it -v `pwd`:/home/oslab/lab1 -u oslab -w /home/oslab 6786 /bin/bash
oslab@3c1da3906541:~$ 
```

**【请将该步骤中命令的输入及运行结果截图附在此处，接下来每一步同理。】**

![image-20211005154714817](C:\Users\欸？\AppData\Roaming\Typora\typora-user-images\image-20211005154714817.png)

![image-20211005160511430](C:\Users\欸？\AppData\Roaming\Typora\typora-user-images\image-20211005160511430.png)

#### 2.测试映射关系

为测试映射关系是否成功，再打开一个终端窗口，并进入`lab1`目录下。

```shell
###Terminal2
$ touch testfile
$ ls
testfile
```

![image-20211005160709100](C:\Users\欸？\AppData\Roaming\Typora\typora-user-images\image-20211005160709100.png)

在第一个终端窗口，即容器中确认是否挂载成功。确认映射关系建立成功后，可以在本地`lab1`目录下编写实验代码，文件将映射到容器中，因此可以直接在容器中进行实验。

```shell
###Terminal1
### 在docker中确认是否挂载成功
oslab@3c1da3906541:~$ pwd
/home/oslab
oslab@3c1da3906541:~$ cd lab1
oslab@3c1da3906541:~$ ls
testfile
```

![image-20211005160852284](C:\Users\欸？\AppData\Roaming\Typora\typora-user-images\image-20211005160852284.png)

### 3.2 了解项目框架，编写MakeFile（20%）

#### 1.编写Makefile文件

```
.
├── arch
│   └── riscv
│       ├── boot
│       ├── include
│       │   ├── print.h
│       │   ├── sbi.h
│       │   └── test.h
│       ├── kernel
│       │   ├── head.S
│       │   ├── main.c(需修改数字为学号)
│       │   ├── Makefile
│       │   ├── sbi.c（需通过内联汇编实现sbi调用）
│       │   ├── test.c
│       │   └── vmlinux.lds
│       ├── libs
│       │   ├── MAKEFILE（需编写字符串打印函数及数字打印函数）
│       │   └── print.c
│       └── Makefile
├── cp.bat
├── include
│   └── defs.h
└── Makefile
```

下载相关代码移动至`lab1`文件夹中。

项目代码结构由上图所示，通过最外层的Makefile中的"make run"命令进行编译与运行。请参考【附录A.Makefile介绍】中的基础知识，了解项目各结构中Makefile每一行命令的作用，编写`./arch/riscv/libs/Makefile`文件使得整个项目能够顺利编译，并将其填写至下述代码块中。其他步骤中的代码编写同理。

```makefile
#lab1/arch/riscv/libs/Makefile 
# YOUR MAKEFILE CODE
all: print.o

%.o:%.c
	${CC}  ${CFLAG}  -c $<

clean:
	$(shell rm *.o 2>/dev/null)

```

#### 2.解释Makefile命令

**请解释`lab1/Makefile`中的下列命令：**

```makefile
${LD} -T arch/riscv/kernel/vmlinux.lds arch/riscv/kernel/*.o arch/riscv/libs/*.o -o vmlinux
```

含义：`${LD}`被展开为`riscv64-unknown-elf-ld`，即链接命令。`-T arch/riscv/kernel/vmlinux.lds`选项表示使用`vmlinux.lds`作为链接器脚本。`-o vmlinux`选项指定输出文件的名称为`vmlinux`，整行命令的意思是将`arch/riscv/kernel/*.o`和`arch/riscv/libs/*.o`根据`vmlinux.lds`链接形成`vmlinux`。

```shell
${OBJCOPY} -O binary vmlinux arch/riscv/boot/Image
```

含义：`${OBJCOPY}`被展开为riscv64-unknown-elf-objcopy，即内容拷贝命令，-O指定output即输出文件的bfdname为binary(二进制)，实现将`arch/riscv/boot/Image`中的内容拷贝到vmlinux文件中，并转为二进制。

```makefile
${MAKE} -C arch/riscv all
```

含义：
`${MAKE}`的含义可以参见[这里](https://stackoverflow.com/questions/38978627/what-is-the-variable-make-in-a-makefile)

 `${MAKE}`是嵌套执行make命令的意思，-C表示进入文件`arch/riscv`中执行make, all表示编译目标，整个语句的意思是进入文件夹`arch/riscv`中执行make all指令，完成all目标的编译

### 3.3 学习RISC-V相关知识及特权架构（10%）

后续实验中将持续使用RISC-V指令集相关的内容，请参考【附录B.RISC-V指令集】了解相关知识，**下载并认真阅读[RISC-V中文手册](crva.ict.ac.cn/documents/RISC-V-Reader-Chinese-v2p1.pdf)，**掌握基础知识、基本命令及特权架构相关内容。

#### 1.基础命令掌握情况

请填写命令的含义。

```assembly
#1.加载立即数，t0=0x40000
li t0,0x40000

#2.将to值写入satp CSR寄存器
csrw satp, t0

#3.计算to-t1并把值保存在to中
sub t0,t0,t1

#4.将x1值保存在寄存器8(sp)中
sd x1, 8(sp)

#5.获取stack_top定义的地址数据，传递给sp寄存器
la sp,stack_top
```

### 3.3 通过OpenSBI接口实现字符串打印函数（60%）

#### 1.程序执行流介绍

对于本次实验，我们选择使用OpenSBI作为bios来进行机器启动时m模式下的硬件初始化与寄存器设置，并使用OpenSBI所提供的接口完成诸如字符打印等的操作。

请参考【附录B.OpenSBI介绍】了解OpenSBI平台的功能及启动方式，参考【附录D. Linux Basic】了解`vmlinux.lds`、`vmlinux`的作用，理解执行`make run`命令时程序的执行过程。

```shell
#make run 实际为执行lab1/Makefile中的该行命令
@qemu-system-riscv64 -nographic --machine virt -bios default -device loader,file=vmlinux,addr=0x80200000 -D log
```

QEMU模拟器完成从ZSBL到OpenSBI阶段的工作，使用-bios default选项将OpenSBI代码加载到0x80000000起始处。OpenSBI初始化完成后，跳转到0x80200000处。我们将`vmlinux`程序这一RISC-V 64-bit架构程序加载至0x80200000处运行。

`vmlinux.lds`链接脚本指定了程序的内存布局，最先加载的`.text.init`段代码为`head.S`文件的内容，顺序执行调用`main()`函数。`main()`函数调用了两个打印函数，通过`sbi_call()`向OpenSBI发起调用，完成字符的打印。

#### 2.编写sbi_call()函数（20%）

当系统处于m模式时，对指定地址进行写操作便可实现字符的输出。但我们编写的内核运行在s模式，需要使用OpenSBI提供的接口，让运行在m模式的OpenSBI帮我们实现输出。此时，运行在s模式的内核通过 `ecall` 发起sbi 调用请求，RISC-V CPU 会从 s 态跳转到 m 态的 OpenSBI 固件。

执行 `ecall` 前需要指定 sbi调用的编号，传递参数。一般而言，`a7(x17)` 为 SBI 调用编号，`a0(x10)`、`a1(x11)` 和 `a2(x12)` 寄存器为sbi调用参数。**其中`a7`是寄存器的ABI Name，在RISC-V命令中需要使用`x17`进行标识，详见RISC-V中文手册中寄存器-接口名称相关的对照表。**

我们需要编写内联汇编语句以使用OpenSBI接口，给出函数定义如下：

```C
typedef unsigned long long uint64_t;
uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2)
```

在该函数中，我们需要完成以下内容：

* 将sbi_type放到寄存器a7中，将arg0-arg2放入a0-a2中。

* 使用ecall指令发起sbi调用请求。

* OpenSBI的返回结果会放到a0中，我们需要将其取出，作为sbi_call的返回值。

请参考【附录C.内联汇编】相关知识，以内联汇编形式实现`lab1/arch/riscv/kernel/sbi.c`中的`sbi_call()`函数。

```c
#lab1/arch/riscv/kernel/sbi.c
uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint64_t ret_val;
    __asm__ volatile ( 
   //Your code
   //Asembly code
        "mv a7, %[sbi_type]\n"// Assign value to the registers
        "mv a0, %[arg0]\n"
        "mv a1, %[arg1]\n"
        "mv a2, %[arg2]\n"
        "ecall\n"
        "mv %[ret_val], a0\n"
        //Output
        : [ret_val] "=r" (ret_val) //Update the variable ret_val
        //Input
        : [sbi_type] "r" (sbi_type), [arg0] "r" (arg0), [arg1] "r" (arg1), [arg2] "r" (arg2) //Pass the c variable to registers
        : "memory"
   );
    return ret_val;
}
```

向`sbi_call()`传入不同的`sbi_type`调用不同功能的OpenSBI接口，其对应关系如下表所示，其中每个函数的具体使用方法可参考[OpenSBI 文档](https://github.com/riscv/riscv-sbi-doc/blob/master/riscv-sbi.adoc#function-listing-1)。 为了利用OpenSBI接口打印字符，我们需要向`sbi_call()`函数传入`sbi_type=1`以调用`sbi_console_putchar(int ch)`函数，第一个参数`arg0`需要传入待打印字符的ASCII码，第二、三个参数的值可直接设为0。测试时，调用`sbi_call(1,0x30,0,0)`将会输出字符'0'。其中1代表输出，0x30代表'0'的ascii值，arg1-2不需要输入，可以用0填充。

<img src="https://mianbaoban-assets.oss-cn-shenzhen.aliyuncs.com/2020/12/RfMJBr.png" style="zoom:50%;" />

#### 3.编写字符串打印函数（40%）

需要在`./arch/riscv/libs/print.c`文件中通过调用`sbi_call()`实现字符串打印函数`int puts(char* str)`及数字打印函数`int put_num(uint64_t n)`，后者可将数字转换为字符串后调用前者执行。

```c
#./arch/riscv/libs/print.c
#include "defs.h"
extern sbi_call();
int puts(char* str){
	// your code
    int i = 0;
	while (str[i] != '\0')
	{
		sbi_call(1,(int)str[i],0,0);
		i++;
	}
    return 0;
}

int put_num(uint64_t n){
	// your code
    if(n != 0){
		char c = (int)(n%10) + '0';
		char cstring[2];
		cstring[0] = c;
		cstring[1] = '\0';
		n = n/10;
		put_num(n);
		puts(cstring);
	}
	return 0;
}
```

### 3.4 编译及测试

在`lab1/arch/riscv/kernel/main.c`中将`21922192`修改为你的学号，在项目最外层输入`make run`命令调用Makefile文件完成整个工程的编译及执行。

**如果编译失败，及时使用`make clean`命令清理文件后再重新编译。**

如果程序能够正常执行并打印出相应的字符串及你的学号，则实验成功。预期的实验结果如下，请附上相应的命令行及实验结果截图。

```shell
oslab@fa503ace2456:~/lab1$ make run
Hello riscv
3180100055
```

![image-20211013211329309](C:\Users\欸？\AppData\Roaming\Typora\typora-user-images\image-20211013211329309.png)

## 4 讨论和心得

本次实验遇到的最大问题是RISCV相关指令的学习，由于之前没有接触过RISCV，并且实验指导只有一本书，让我望而生畏，实验停滞了很久。但是后来硬着头皮学了一下，发现收获很多，了解了指令集架构和汇编、操作系统等的关系，而且幸好完成本次实验本身并不太难。

随后就是遇到了一些小bug，都是C语言写的时候不仔细，很快就解决了。

有一个小建议是改变做实验的顺序，先让大家填充代码，再编写makefile文件，这样子可以直绩测试，不会让人引起困惑。另外一次让看一本书，有些不是很友好，希望能够指出本次实验所需要的部分，因为只用使用过的，才是记得最熟悉的，看过的也就看过了，再每一次实验中一点点学，会比一次性让大家看一本书要友好得多。
