#include <stdbool.h>

struct psi {
	unsigned long some_avg10, some_avg60, some_avg300;
	unsigned long full_avg10, full_avg60, full_avg300;
	unsigned long long some_total, full_total;
};

int damon_read_bool(const char *module, const char *param, bool *val);
int damon_write_bool(const char *module, const char *param, bool val);
int damon_is_enabled(const char *module, bool *enabled);
int damon_set_enabled(const char *module, bool on);
