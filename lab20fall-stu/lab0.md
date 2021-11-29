# Lab 0: RV64 内核调试

## 1 实验目的

通过在QEMU上运行Linux来熟悉如何从源代码开始将内核运行在QEMU模拟器上，并且掌握使用GDB跟QEMU进行联合调试，为后续实验打下基础。

## 2 实验环境

- Ubuntu 18.04.5 LTS
- Docker[下载地址](https://magiclink.teambition.com/shares/5f6ee504674e61d264aa0ef8)[备用地址（内网访问）](ftp://party:party@zju.phvntom.tech/Samsung_T5/Jinyan/tmp/oslab.tar)

## 3 实验基础知识介绍

### 3.1 Linux 使用基础

在Linux环境下，人们通常使用命令行接口来完成与计算机的交互。终端（Terminal）是用于处理该过程的一个应用程序，通过终端你可以运行各种程序以及在自己的计算机上处理文件。在类Unix的操作系统上，终端可以为你完成一切你所需要的操作。下面我们仅对实验中涉及的一些概念进行介绍，你可以通过下面的链接来对命令行的使用进行学习：

1. [The Missing Semester of Your CS Education](https://missing-semester-cn.github.io/2020/shell-tools)[&gt;&gt;Video&lt;&lt;](https://www.bilibili.com/video/BV1x7411H7wa?p=2)
2. [GNU/Linux Command-Line Tools Summary](https://tldp.org/LDP/GNU-Linux-Tools-Summary/html/index.html)
3. [Basics of UNIX](https://github.com/berkeley-scf/tutorial-unix-basics)

#### 环境变量

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

在后面的实验中，如果你想直接访问 `riscv64-unknown-linux-gnu-gcc`、`qemu-system-riscv64`等程序，那么你需要把他们所在的目录添加到目录中。

```bash
$ export PATH=$PATH:/opt/riscv/bin
```

### 3.2 Docker 使用基础

#### Docker 基本介绍

Docker 是一种利用容器（container）来进行创建、部署和运行应用的工具。Docker把一个应用程序运行需要的二进制文件、运行需要的库以及其他依赖文件打包为一个包（package），然后通过该包创建容器并运行，由此被打包的应用便成功运行在了Docker容器中。之所以要把应用程序打包，并以容器的方式运行，主要是因为在生产开发环境中，常常会遇到应用程序和系统环境变量以及一些依赖的库文件不匹配，导致应用无法正常运行的问题。Docker带来的好处是只要我们将应用程序打包完成（组装成为Docker imgae），在任意安装了Docker的机器上，都可以通过运行容器的方式来运行该应用程序，因而将依赖、环境变量等带来的应用部署问题解决了。
Docker和虚拟机功能上有共同点，但是和虚拟机不同，Docker不需要创建整个操作系统，只需要将应用程序的二进制和有关的依赖文件打包，因而容器内的应用程序实际上使用的是容器外Host的操作系统内核。这种共享内核的方式使得Docker的移植和启动非常的迅速，同时由于不需要创建新的OS，Docker对于容器物理资源的管理也更加的灵活，Docker用户可以根据需要动态的调整容器使用的计算资源（通过cgroups）。

#### Docker 安装

如果你在 Ubuntu 发行版上安装 Docker，请参考[这里](https://docs.docker.com/engine/install/ubuntu/)，其余平台请自行查找。
你可以从**2**中获得实验所需的环境，我们已经为你准备好了RISC-V工具链，以及QEMU模拟器，使用方法请参见**4.1**。

### 3.3 QEMU 使用基础

#### 什么是QEMU

<!-- QEMU 模拟器所提供的功能 -->

QEMU最开始是由法国程序员Fabrice Bellard开发的模拟器。QEMU能够完成用户程序模拟和系统虚拟化模拟。用户程序模拟指的是QEMU能够将为一个平台编译的二进制文件运行在另一个不同的平台，如一个ARM指令集的二进制程序，通过QEMU的TCG（Tiny Code Generator）引擎的处理之后，ARM指令被转化为TCG中间代码，然后再转化为目标平台（比如Intel x86）的代码。系统虚拟化模拟指的是QEMU能够模拟一个完整的系统虚拟机，该虚拟机有自己的虚拟CPU，芯片组，虚拟内存以及各种虚拟外部设备，能够为虚拟机中运行的操作系统和应用软件呈现出与物理计算机完全一致的硬件视图。

#### 如何使用 QEMU（常见参数介绍）

<!-- 如何使用QEMU，所涉及的参数介绍 -->

以以下命令为例，我们简单介绍QEMU的参数所代表的含义

```bash
$ qemu-system-riscv64 -nographic -machine virt -kernel build/linux/arch/riscv/boot/Image  \
 -device virtio-blk-device,drive=hd0 -append "root=/dev/vda ro console=ttyS0"   \
 -bios default -drive file=rootfs.ext4,format=raw,id=hd0 \
 -netdev user,id=net0 -device virtio-net-device,netdev=net0 -S -s
```

**-nographic**: 不使用图形窗口，使用命令行
**-machine**: 指定要emulate的机器，可以通过命令 `qemu-system-riscv64 -machine help`查看可选择的机器选项
**-kernel**: 指定内核image
**-append cmdline**: 使用cmdline作为内核的命令行
**-device**: 指定要模拟的设备，可以通过命令 `qemu-system-riscv64 -device help`查看可选择的设备，通过命令 `qemu-system-riscv64 -device <具体的设备>,help`查看某个设备的命令选项
**-drive, file=<file_name>**: 使用'file'作为文件系统
**-netdev user,id=str**: 指定user mode的虚拟网卡, 指定ID为str
**-S**: 启动时暂停CPU执行(使用'c'启动执行)
**-s**: -gdb tcp::1234 的简写
**-bios default**: 使用默认的OpenSBI firmware作为bootloader

更多参数信息可以参考[这里](https://www.qemu.org/docs/master/system/index.html)

### 3.4 GDB 使用基础

#### 什么是 GDB

GNU调试器（英语：GNU Debugger，缩写：gdb）是一个由GNU开源组织发布的、UNIX/LINUX操作系统下的、基于命令行的、功能强大的程序调试工具。借助调试器，我们能够查看另一个程序在执行时实际在做什么（比如访问哪些内存、寄存器），在其他程序崩溃的时候可以比较快速地了解导致程序崩溃的原因。
被调试的程序可以是和gdb在同一台机器上（本地调试，or native debug），也可以是不同机器上（远程调试， or remote debug）。

<!-- GDB 功能介绍 -->

总的来说，gdb可以有以下4个功能

- 启动程序，并指定可能影响其行为的所有内容
- 使程序在指定条件下停止
- 检查程序停止时发生了什么
- 更改程序中的内容，以便纠正一个bug的影响

#### GDB 基本命令介绍

(gdb) start：单步执行，运行程序，停在第一执行语句
(gdb) next：单步调试（逐过程，函数直接执行）,简写n
(gdb) run：重新开始运行文件（run-text：加载文本文件，run-bin：加载二进制文件），简写r
(gdb) backtrace：查看函数的调用的栈帧和层级关系，简写bt
(gdb) break 设置断点。比如断在具体的函数就break func；断在某一行break filename:num
(gdb) finish：结束当前函数，返回到函数调用点
(gdb) frame：切换函数的栈帧，简写f
(gdb) print：打印值及地址，简写p
(gdb) info：查看函数内部局部变量的数值，简写i；查看寄存器的值i register xxx
(gdb) display：追踪查看具体变量值

更多命令可以参考[100个gdb小技巧](https://wizardforcel.gitbooks.io/100-gdb-tips/content/)

#### 3.4.3 GDB 插件使用（不做要求）

单纯使用gdb比较繁琐不是很方便，我们可以使用gdb插件让调试过程更有效率。推荐各位同学使用gef，由于当前工具链还不支持 `python3`，请使用旧版本的[gef-legacy](https://github.com/hugsy/gef-legacy)。
该仓库中已经取消的原有的安装脚本，同学们可以把 `gef.py` 脚本拷贝下来，直接在 `.gdbinit` 中引导，感兴趣的同学可以参考这篇[文章（内网访问）](http://zju.phvntom.tech/markdown/md2html.php?id=md/gef.md)。

### 3.5 LINUX 内核编译基础

#### 交叉编译

交叉编译指的是在一个平台上编译可以在另一个平台运行的程序，例如在x86机器上编译可以在arm平台运行的程序，交叉编译需要交叉编译工具链的支持。

#### 内核配置

内核配置是用于配置是否启用内核的各项特性，内核会提供一个名为 `defconfig`(即default configuration) 的默认配置，该配置文件位于各个架构目录的 `configs` 文件夹下，例如对于RISC-V而言，其默认配置文件为 `arch/riscv/configs/defconfig`。使用 `make ARCH=riscv defconfig` 命令可以在内核根目录下生成一个名为 `.config` 的文件，包含了内核完整的配置，内核在编译时会根据 `.config` 进行编译。配置之间存在相互的依赖关系，直接修改defconfig文件或者 `.config` 有时候并不能达到想要的效果。因此如果需要修改配置一般采用 `make ARCH=riscv menuconfig` 的方式对内核进行配置。

#### 常见参数

**ARCH** 指定架构，可选的值包括arch目录下的文件夹名，如x86,arm,arm64等，不同于arm和arm64，32位和64位的RISC-V共用 `arch/riscv` 目录，通过使用不同的config可以编译32位或64位的内核。

**CROSS_COMPILE** 指定使用的交叉编译工具链，例如指定 `CROSS_COMPILE=aarch64-linux-gnu-`，则编译时会采用 `aarch64-linux-gnu-gcc` 作为编译器，编译可以在arm64平台上运行的kernel。

**CC** 指定编译器，通常指定该变量是为了使用clang编译而不是用gcc编译，Linux内核在逐步提供对clang编译的支持，arm64和x86已经能够很好的使用clang进行编译。

#### 常用编译选项

```
$ make defconfig	        ### 使用当前平台的默认配置，在x86机器上会使用x86的默认配置
$ make -j$(nproc)	        ### 编译当前平台的内核，-j$(nproc)为以机器硬件线程数进行多线程编译

$ make ARCH=riscv defconfig	### 使用RISC-V平台的默认配置
$ make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc)     ### 编译RISC-V平台内核

$ make clean	                ### 清除所有编译好的object文件
$ make mrproper	                ### 清除编译的配置文件，中间文件和结果文件

$ make init/main.o	        ### 编译当前平台的单个object文件init/main.o（会同时编译依赖的文件）
```

## 4 实验步骤

通常情况下，`$` 提示符表示当前运行的用户为普通用户，`#` 代表当前运行的用户为特权用户。
但注意，在下文的示例中，以 `###` 开头的行代表注释，`$` 开头的行代表在你的宿主机/虚拟机上运行的命令，`#` 开头的行代表在 `docker` 中运行的命令，`(gdb)` 开头的行代表在 `gdb` 中运行的命令。
**在执行每一条命令前，请你对将要进行的操作进行思考，给出的命令不需要全部执行，并且不是所有的命令都可以无条件执行，请不要直接复制粘贴命令去执行。**

### 4.1 搭建docker环境

```
### docker安装可使用下面命令，或者参考给出的官方链接
$ curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun

### 将用户加入docker组，免 sudo
$ sudo usermod -aG docker $USER   ### 注销后重新登陆生效
$ sudo chmod a+rw /home/$USER/.docker/config.json

### 导入docker镜像
$ cat oslab.tar | docker import - oslab:2020
### 执行命令后若出现以下错误提示
### ERROR: Got permission denied while trying to connect to the Docker daemon socket at unix:///var/run/docker.sock
### 可以使用下面命令为该文件添加权限来解决
### $ sudo chmod a+rw /var/run/docker.sock

### 查看docker镜像
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
oslab               2020                d7046ea68221        5 seconds ago       2.89GB

### 从镜像创建一个容器
$ docker run -it oslab:2020 /bin/bash    ### -i:交互式操作 -t:终端
root@368c4cc44221:/#                     ### 提示符变为 '#' 表明成功进入容器 后面的字符串根据容器而生成，为容器id
root@368c4cc44221:/# exit (或者CTRL+D）   ### 从容器中退出 此时运行docker ps，运行容器的列表为空

### 查看当前运行的容器
$ docker ps 
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
### 查看所有存在的容器
$ docker ps -a 
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS                      PORTS               NAMES
368c4cc44221        oslab:2020          "/bin/bash"         54 seconds ago      Exited (0) 30 seconds ago                       relaxed_agnesi

### 启动处于停止状态的容器
$ docker start 368c     ### 368c 为容器id的前四位，id开头的几位便可标识一个容器
$ docker ps             ### 可看到容器已经启动
CONTAINER ID        IMAGE               COMMAND             CREATED              STATUS              PORTS               NAMES
368c4cc44221        oslab:2020          "/bin/bash"         About a minute ago   Up 16 seconds                           relaxed_agnesi

### 进入已经运行的容器 oslab的密码为2020
$ docker exec -it -u oslab -w /home/oslab 36 /bin/bash
oslab@368c4cc44221:~$

### 进入docker后，按4.2-4.5指导进行下一步实验
```

### 4.2 编译 linux 内核

```
### 进入实验目录并设置环境变量
# pwd
/home/oslab
# cd lab0
# export TOP=`pwd`
# export RISCV=/opt/riscv
# export PATH=$PATH:$RISCV/bin

# mkdir -p build/linux
# make -C linux O=$TOP/build/linux \
          CROSS_COMPILE=riscv64-unknown-linux-gnu- \
          ARCH=riscv CONFIG_DEBUG_INFO=y \
          defconfig all -j$(nproc)
```

### 4.3 使用QEMU运行内核

```
### 用户名root，没有密码
# qemu-system-riscv64 -nographic -machine virt -kernel build/linux/arch/riscv/boot/Image  \
 -device virtio-blk-device,drive=hd0 -append "root=/dev/vda ro console=ttyS0"   \
 -bios default -drive file=rootfs.ext4,format=raw,id=hd0 \
 -netdev user,id=net0 -device virtio-net-device,netdev=net0
```

### 4.4 使用 gdb 对内核进行调试

```
### Terminal 1
# qemu-system-riscv64 -nographic -machine virt -kernel build/linux/arch/riscv/boot/Image  \
 -device virtio-blk-device,drive=hd0 -append "root=/dev/vda ro console=ttyS0"   \
 -bios default -drive file=rootfs.ext4,format=raw,id=hd0 \
 -netdev user,id=net0 -device virtio-net-device,netdev=net0 -S -s

### Terminal 2
# riscv64-unknown-linux-gnu-gdb build/linux/vmlinux
(gdb) target remote localhost:1234  ### 连接 qemu
(gdb) b start_kernel                ### 设置断点
(gdb) continue                      ### 继续执行
(gdb) quit                          ### 退出 gdb
```

## 5 实验任务与要求

- 请各位同学独立完成实验，任何抄袭行为都将使本次实验判为0分。
- 编译内核并用 gdb + QEMU 调试，在内核初始化过程中（用户登录之前）设置断点，对内核的启动过程进行跟踪，并尝试使用gdb的各项命令（如backtrace、finish、frame、info、break、display、next等）。
- 在学在浙大中提交pdf格式的实验报告，记录实验过程并截图（4.1-4.4），对每一步的命令以及结果进行必要的解释，记录遇到的问题和心得体会。

#### 本文贡献者以及负责答疑内容

> 周侠 (gdb qemu riscv-toolchain)
> 管章辉 (docker)
> 徐金焱 (gdb qemu riscv-toolchain)
> 张文龙 (gdb qemu riscv-toolchain)
> 刘强 孙家栋 周天昱
