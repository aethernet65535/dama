#include "core.h"
#include "damon.h"
#include "sysfs.h"
#include "log.h"

int main(void)
{
	struct damon_info *info = alloc_damon_info();
	bool enabled = false;

	setvbuf(stdout, NULL, _IONBF, 0);

	if (!info) {
		pr_err("ENOMEM failed\n");
		return -1;
	}

	info->param->action = PAGEOUT;

	if (damon_init())
		goto err;

	if (damon_write_operation("paddr"))
		goto err;

	if (damon_write_action(info->param->action))
		goto err;

	if (damon_set_enabled(true))
		goto err;

	if (damon_is_enabled(&enabled))
		goto err;

	if (!enabled) {
		pr_err("damon is not enabled\n");
		goto err;
	}

	if (damos_init())
		goto err;

	udamond_fn(info);
	return 0;
err:
	pr_err("main: error\n");
	return -1;
}
