#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int sysfs_write_ulong(const char *path, unsigned long val);
int sysfs_write_int(const char *path, int val);
int sysfs_write_bool(const char *path, bool val);
int sysfs_write_str(const char *path, const char *val);
int sysfs_read_ulong(const char *path, unsigned long *val);
int sysfs_read_int(const char *path, int *val);
int sysfs_read_char(const char *path, char *val);
int sysfs_read_str(const char *path, char **val, size_t max_len);
int read_meminfo(unsigned long *memtotal, unsigned long *memfree,
		 unsigned long *inactive_anon, unsigned long *inactive_file);
int read_psi_cpu(struct psi *psi);
int read_psi_io(struct psi *psi);
int read_psi_mem(struct psi *psi);
int read_refault(unsigned long *anon, unsigned long *file);
int read_steal(unsigned long *kswapd, unsigned long *direct);
