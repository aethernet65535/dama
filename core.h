#include <stdbool.h>

#define pow2(x) (x * x)
#define PERCENT(x, p) (((x) * (p)) / 100)
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min3(x, y, z) (min((x), min((y), (z))))
#define max3(x, y, z) (max((x), max((y), (z))))

#define PAGE_SIZE (4096)

#define DAMON_RECLAIM (0)
#define DAMON_LRU_SORT (1)

struct ma_calc {
	unsigned long kswapd_reclaimed;
	unsigned long direct_reclaimed;
	unsigned long nr_damon_applied;

	unsigned long remaining_pgsteal;
	unsigned long remaining_damon;

	unsigned long refault_anon;
	unsigned long refault_file;
	unsigned long anon_weight;
	unsigned long file_weight;

	unsigned long last_kswapd_reclaimed;
	unsigned long last_direct_reclaimed;
	unsigned long last_nr_damon_applied;
	unsigned long last_refault_anon;
	unsigned long last_refault_file;

	unsigned long remaining_time_us;

	bool init;
};

struct sysfs_param {
	unsigned long min_age;
};

struct wmarks {
	unsigned long high, mid, low;
};

struct psi {
       unsigned long some_avg10, some_avg60, some_avg300;
       unsigned long full_avg10, full_avg60, full_avg300;
       unsigned long long some_total, full_total;
};

struct damon_info {
	unsigned int damon_module;
	struct sysfs_param *stats;
	struct ma_calc *ma_calc;
	struct wmarks wmarks;
};

int module_to_name(unsigned int damon_module, char **module_name);
int damon_is_enabled(const char *module_name, bool *enabled);
int damon_set_enabled(const char *module_name, bool on);
void *alloc_damon_info(void);
int udamond_fn(struct damon_info *info);
