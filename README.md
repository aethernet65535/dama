# DAMA (DAMon Augment)

> [!NOTE]
> This project is currently in the **Alpha stage** and may contain many unexpected issues. Please exercise caution when running it in a production environment.

DAMA is a user-space daemon based on DAMON, designed to provide users with experimental user-space features **without modifying the kernel**.

## Brief Principle

**Almost** all of DAMA's features are implemented by operating on [DAMON_SYSFS](https://docs.kernel.org/admin-guide/mm/damon/usage.html#sysfs-targets). Some simpler features may not involve DAMON_SYSFS.

## Prerequisites

Since DAMA deeply relies on the Linux kernel's DAMON mechanism and specific file system paths:
* **Operating System:** **Linux** (Windows, macOS, and some niche distributions are likely unable to run it).
* **Kernel Version:** Requires your Linux kernel to have DAMON enabled and support DAMON_SYSFS.

## Quick Start

DAMA does not use overly complex toolchains and can be simply built using `make`.
```sh
$ git clone [https://github.com/aethernet65535/dama.git](https://github.com/aethernet65535/dama.git)
$ cd dama
$make$ sudo ./dama

```

### Configuration

Most configurations need to be modified in `config.h`. Most commonly used parameters will be supported as command-line arguments passed to DAMA in the future.

#### Currently Supported Command-Line Arguments

**`--operation`:** Specifies the address space type monitored by DAMON.

* **Options:** `paddr` and `vaddr`.
* **Note:** `fvaddr` is currently not supported.

**`--target_pid`:** Specifies the target process PID to be monitored by DAMON for VADDR.

##### Usage Example

The following is an example of monitoring a `stress-ng-vm` process:

```sh
# stress-ng --vm 1 --vm-bytes 1G --vm-hang 3600 --vm-keep --timeout=5m &
# ./dama --operation=vaddr --target_pid=$(pidof stress-ng-vm)
```
