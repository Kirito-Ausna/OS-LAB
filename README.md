# OS-LAB

2021-2022  操作系统实验

## 附录

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

