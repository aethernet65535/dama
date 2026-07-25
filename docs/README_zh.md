# DAMA (DAMon Augment) 中文文档
> [!NOTE]
> 该项目目前处于 **Alpha 阶段**，可能会有许多意想不到的问题，请谨慎在生产环境中运行。

DAMA 是一个基于 DAMON 的用户空间 Daemon，旨在**在不修改内核的情况下**，为用户提供一些基于用户空间的实验性功能。

## 原理简述
DAMA **几乎**所有功能都是通过操作 [DAMON_SYSFS](https://docs.kernel.org/admin-guide/mm/damon/usage.html#sysfs-targets) 实现的，部分简单的功能可能不涉及 DAMON_SYSFS。

## 环境要求
由于 DAMA 深度依赖 Linux 内核的 DAMON 机制以及特定的文件系统路径：
* **操作系统：** 仅支持 **Linux**（Windows、macOS 以及部分非主流发行版无法运行）。
* **内核版本：** 需要你的 Linux 已启用 DAMON 并支持 DAMON_SYSFS。

## 快速开始
DAMA 并没有使用过多复杂的工具链，可以简单的使用 `make` 构建。
```sh
$ git clone https://github.com/aethernet65535/dama.git
$ cd dama
$ make
$ sudo ./dama
```

### 配置
大部分配置都需要在 `config.h` 中修改。大部分常用的参数会在之后支持作为命令行参数传入 DAMA。

#### 目前支持的命令行参数
**`--operation`：**指定 DAMON 监控的地址空间类型。
- **选项：** `paddr` 和 `vaddr`。
- **注：**目前暂不支持 `fvaddr`。
**`--target_pid`:** 指定 VADDR 的 DAMON 要监控的目标进程 PID。

##### 使用例子
以下为监控一个 `stress-ng-vm` 的例子：
```sh
# stress-ng --vm 1 --vm-bytes 1G --vm-hang 3600 --vm-keep --timeout=5m &
# ./dama --operation=vaddr --target_pid=$(pidof stress-ng-vm)
```
