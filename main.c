#include <stdbool.h>
#include <stdlib.h>
#include "core.h"
#include "sysfs.h"
#include "log.h"
#include "config.h"

int main(void)
{
	struct damon_info *info = alloc_damon_info();
	bool enabled = false;
	char *module_name;

	setvbuf(stdout, NULL, _IONBF, 0);

	if (!info) {
		pr_err("ENOMEM failed\n");
		return -1;
	}

	info->damon_module = DAMON_RECLAIM;
	if (module_to_name(info->damon_module, &module_name))
		return -1;

	damon_set_enabled(module_name, true);
	damon_is_enabled(module_name, &enabled);
	if (!enabled) {
		pr_err("damon is not enabled\n");
		return -1;
	}

	udamond_fn(info);
	return 0;
}
