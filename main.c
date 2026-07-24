#include "core.h"
#include "damon.h"
#include "pageout_min_age.h"
#include "sysfs.h"
#include "log.h"
#include "arg.h"

int main(int argc, char *argv[])
{
	struct damon_info *info = alloc_damon_info();
	bool enabled = false;

	setvbuf(stdout, NULL, _IONBF, 0);

	if (!info) {
		pr_err("ENOMEM failed\n");
		return -1;
	}

	info->param->action = PAGEOUT;
	info->udamond_action = pageout_min_age_autotune;

	damon_close();
	if (damon_init())
		goto err;

	/* Access Pattern */
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/monitoring_attrs/intervals/sample_us", 5000))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/monitoring_attrs/intervals/aggr_us", 100000))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/sz/min", 4096))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/sz/max", 18446744073709551615UL))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/nr_accesses/min", 0))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/nr_accesses/max", 0))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/age/min", 120))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/age/max", 18446744073709551615UL))
		goto err;

	/* Quota */
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/ms", 10))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/bytes", 134217728))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/reset_interval_ms", 1000))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/weights/sz_permil", 0))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/weights/nr_accesses_permil", 0))
		goto err;
	if (sysfs_write_ulong("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/weights/age_permil", 1000))
		goto err;

	/* Watermark */
	if (sysfs_write_str("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/metric", "free_mem_rate"))
		goto err;
	if (sysfs_write_str("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/interval_us", "5000000"))
		goto err;
	if (sysfs_write_str("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/high", "600"))
		goto err;
	if (sysfs_write_str("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/mid", "500"))
		goto err;
	if (sysfs_write_str("/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/low", "10"))
		goto err;

	if (damon_write_operation("paddr"))
		goto err;
	if (damon_write_action(info->param->action))
		goto err;
	if (arg_exec(argc, argv))
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
