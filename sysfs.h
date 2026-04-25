#include <stdbool.h>
#include "core.h"

#define PATH_DAMON_RECLAIM "/sys/module/damon_reclaim/parameters"
#define PATH_DAMON_LRU_SORT "/sys/module/damon_lru_sort/parameters"

int write_sysfs_ulong(const char *path, unsigned long val);
int write_sysfs_int(const char *path, int val);
int write_sysfs_bool(const char *path, bool val);
int read_sysfs_ulong(const char *path, unsigned long *val);
int read_sysfs_int(const char *path, int *val);
int read_sysfs_char(const char *path, char *val);

int read_meminfo(unsigned long *mem_total_kb, unsigned long *inactive_anon_kb,
		 unsigned long *inactive_file_kb);
int read_psi_mm(struct psi *psi);
