#include <stdbool.h>
#include <sys/param.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#define pow2(x) (x * x)
#define PERCENT(x, p) (((x) * (p)) / 100)
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min3(x, y, z) (min((x), min((y), (z))))
#define max3(x, y, z) (max((x), max((y), (z))))

#define BIT(x) (1 << x)

#define PAGE_SIZE (4096)

#define MAS_ERR BIT(0)
#define MAS_NEED_INIT BIT(1)

struct wmarks {
	unsigned long high, mid, low;
};

struct damos_param {
	unsigned int action;
	unsigned long min_age;
	struct wmarks wmarks;
};

struct mas_calc {
	struct {
		unsigned long nr_inc;
		unsigned long nr_dec;
		unsigned long next_min_age;
		bool fade;
	} result;

	/* State */
	struct {
		unsigned long damon;
		unsigned long pgsteal;
	} remaining;

	struct {
		unsigned long anon_weight;
		unsigned long file_weight;
		bool init;
	} state;

	/* History/Last */
	struct {
		unsigned long kswapd_reclaimed;
		unsigned long direct_reclaimed;
		unsigned long nr_damon_applied;
		unsigned long refault_anon;
		unsigned long refault_file;
	} last;

	struct {
		unsigned long kswapd_reclaimed;
		unsigned long direct_reclaimed;
		unsigned long nr_damon_applied;
		unsigned long refault_anon;
		unsigned long refault_file;
	} metric;

	struct {
		unsigned long kswapd_reclaimed;
		unsigned long direct_reclaimed;
		unsigned long nr_damon_applied;
		unsigned long refault_anon;
		unsigned long refault_file;
	} total;
};

struct psi {
	unsigned long some_avg10, some_avg60, some_avg300;
	unsigned long full_avg10, full_avg60, full_avg300;
	unsigned long long some_total, full_total;
};

struct damon_info {
	struct damos_param *param;
	struct mas_calc *mas_calc;
};

bool is_supported_action(unsigned int action);
void *alloc_damon_info(void);
int inc_min_age(unsigned long step, unsigned long *min_age);
int dec_min_age(unsigned long step, unsigned long *min_age);
int udamond_fn(struct damon_info *info);
