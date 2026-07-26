#include "core.h"
#include "log.h"
#include "util.h"
#include "sysfs.h"
#include "damon.h"

int damon_close(void)
{
	char *nr_kdamonds = "/sys/kernel/mm/damon/admin/kdamonds/nr_kdamonds";

	damon_write_state("off");
	return sysfs_write_ulong(nr_kdamonds, 0);
}

int damon_init(void)
{
	char *nr_kdamonds = "/sys/kernel/mm/damon/admin/kdamonds/nr_kdamonds";
	char *nr_contexts = "/sys/kernel/mm/damon/admin/kdamonds/0/"
			    "contexts/nr_contexts";
	char *nr_targets = "/sys/kernel/mm/damon/admin/kdamonds/0/"
			   "contexts/0/targets/nr_targets";
	char *nr_schemes = "/sys/kernel/mm/damon/admin/kdamonds/0/"
			   "contexts/0/schemes/nr_schemes";
	int ret;

	ret = sysfs_write_ulong(nr_kdamonds, 1);
	if (ret)
		return ret;
	pr_info("nr_kdamonds: OK\n");

	ret = sysfs_write_ulong(nr_contexts, 1);
	if (ret)
		return ret;
	pr_info("nr_contexts: OK\n");

	ret = sysfs_write_ulong(nr_targets, 1);
	if (ret)
		return ret;
	pr_info("nr_targets: OK\n");

	ret = sysfs_write_ulong(nr_schemes, 1);
	if (ret)
		return ret;
	pr_info("nr_schemes: OK\n");

	return 0;
}

int damon_write_state(const char *state)
{
	const char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	return sysfs_write_str(path, state);
}

int damon_write_operation(const char *op)
{
	const char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/"
			   "contexts/0/operations";
	return sysfs_write_str(path, op);
}

int damon_write_pid(unsigned long pid)
{
	const char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/"
			   "contexts/0/targets/0/pid_target";
	return sysfs_write_ulong(path, pid);
}

int damon_write_action(char *action)
{
	const char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/"
			   "contexts/0/schemes/0/action";
	return sysfs_write_str(path, action);
}

int damon_is_enabled(bool *enabled)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	char buf[8];
	char *on = buf;

	if (sysfs_read_str(path, &on, sizeof(buf)))
		return -1;

	if (!strncmp(buf, "on", 2))
		*enabled = true;
	else
		*enabled = false;

	return 0;
}

int damon_set_enabled(bool on)
{
	if (on)
		return damon_write_state("on");
	else
		return damon_write_state("off");
}

int damon_commit_params(void)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	return sysfs_write_str(path, "commit");
}

int damon_update_stats(void)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	return sysfs_write_str(path, "update_schemes_stats");
}

int damon_read_wmarks(struct wmarks *wmarks)
{
	char *high = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		     "contexts/0/schemes/0/watermarks/high";
	char *mid = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		    "contexts/0/schemes/0/watermarks/mid";
	char *low = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		    "contexts/0/schemes/0/watermarks/low";

	if (sysfs_read_ulong(high, &wmarks->high))
		return -1;
	if (sysfs_read_ulong(mid, &wmarks->mid))
		return -1;
	if (sysfs_read_ulong(low, &wmarks->low))
		return -1;

	return 0;
}

int damon_write_wmarks(struct wmarks wmarks)
{
	char *high = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		     "contexts/0/schemes/0/watermarks/high";
	char *mid = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		    "contexts/0/schemes/0/watermarks/mid";
	char *low = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		    "contexts/0/schemes/0/watermarks/low";

	if (sysfs_write_ulong(high, wmarks.high))
		return -1;
	if (sysfs_write_ulong(mid, wmarks.mid))
		return -1;
	if (sysfs_write_ulong(low, wmarks.low))
		return -1;

	return 0;
}

int damon_read_min_age(unsigned long *min_age)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		     "contexts/0/schemes/0/access_pattern/age/min";
	return sysfs_read_ulong(path, min_age);
}

int damon_write_min_age(unsigned long min_age)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		     "contexts/0/schemes/0/access_pattern/age/min";

	if (sysfs_write_ulong(path, min_age))
		return -1;
	if (damon_commit_params())
		return -1;

	return 0;
}

int __damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz)
{
	char *ms = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		   "contexts/0/schemes/0/quotas/ms";
	char *sz = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		   "contexts/0/schemes/0/quotas/bytes";

	if (sysfs_read_ulong(ms, quota_ms))
		return -1;
	if (sysfs_read_ulong(sz, quota_sz))
		return -1;

	return 0;
}

int damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz)
{
	unsigned long tmp_ms, tmp_sz;
	int ret;

	ret = __damon_read_quota(&tmp_ms, &tmp_sz);
	if (ret)
		return ret;

	if (tmp_ms)
		*quota_ms = tmp_ms;
	if (tmp_sz)
		*quota_sz = tmp_sz;

	return 0;
}

int damon_write_quota(unsigned long quota_ms, unsigned long quota_sz)
{
	char *ms = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		   "contexts/0/schemes/0/quotas/ms";
	char *sz = "/sys/kernel/mm/damon/admin/kdamonds/0/"
		   "contexts/0/schemes/0/quotas/bytes";

	if (sysfs_write_ulong(ms, quota_ms))
		return -1;
	if (sysfs_write_ulong(sz, quota_sz))
		return -1;

	return 0;
}

int damos_init(void)
{
	unsigned long min_age = MIN_AGE;
	struct wmarks wmarks;
	unsigned long quota_ms = QUOTA_MS;
	unsigned long quota_sz = QUOTA_SZ;

	if (DAMOS_ONLY_WATERMARKS) {
		wmarks.high = 1000;
		wmarks.mid = 1000;
		wmarks.low = 0;
	} else {
		wmarks.high = WMARKS_HIGH;
		wmarks.mid = WMARKS_MID;
		wmarks.low = WMARKS_LOW;
	}

	if (damon_write_min_age(min_age))
		return -1;
	pr_info("write min_age: OK\n");

	if (damon_write_wmarks(wmarks))
		return -1;
	pr_info("write wmarks: OK\n");

	if (damon_write_quota(quota_ms, quota_sz))
		return -1;
	pr_info("write quota: OK\n");

	if (damon_commit_params())
		return -1;
	pr_info("commit: OK\n");

	return 0;
}
