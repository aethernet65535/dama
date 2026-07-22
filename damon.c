#include "core.h"
#include "log.h"
#include "util.h"
#include "sysfs.h"

int write_sample_us(unsigned long sample)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/monitoring_attrs/intervals/sample_us";
	return write_sysfs_ulong(path, sample);
}

int write_aggr_us(unsigned long aggr)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/monitoring_attrs/intervals/aggr_us";
	return write_sysfs_ulong(path, aggr);
}

int write_intervals(unsigned long sample, unsigned long aggr)
{
	if (write_sample_us(sample))
		return -1;
	if (write_aggr_us(aggr))
		return -1;

	return 0;
}

char *action_to_str(unsigned int action)
{
	if (action == PAGEOUT)
		return "pageout";
	else if (action == LRU_PRIO)
		return "lru_prio";
	else
		return "none";
}

int damon_init(void)
{
	char *nr_kdamonds = "/sys/kernel/mm/damon/admin/kdamonds/nr_kdamonds";
	char *nr_contexts =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/nr_contexts";
	char *nr_targets =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/targets/nr_targets";
	char *nr_schemes =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/nr_schemes";
	int ret;

	ret = write_sysfs_ulong(nr_kdamonds, 1);
	if (ret)
		return ret;
	pr_info("nr_kdamonds: OK\n");

	ret = write_sysfs_ulong(nr_contexts, 1);
	if (ret)
		return ret;
	pr_info("nr_contexts: OK\n");

	ret = write_sysfs_ulong(nr_targets, 1);
	if (ret)
		return ret;
	pr_info("nr_targets: OK\n");

	ret = write_sysfs_ulong(nr_schemes, 1);
	if (ret)
		return ret;
	pr_info("nr_schemes: OK\n");

	return 0;
}

int write_access_pattern_min_sz(unsigned long min)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/sz/min";
	return write_sysfs_ulong(path, min);
}

int write_access_pattern_max_sz(unsigned long max)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/sz/max";
	return write_sysfs_ulong(path, max);
}

int write_access_pattern_sz(unsigned long min, unsigned long max)
{
	if (write_access_pattern_min_sz(min))
		return -1;
	if (write_access_pattern_max_sz(max))
		return -1;

	return 0;
}

int write_access_pattern_nr_accesses_min(unsigned long min)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/nr_accesses/min";
	return write_sysfs_ulong(path, min);
}

int write_access_pattern_nr_accesses_max(unsigned long max)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/nr_accesses/min";
	return write_sysfs_ulong(path, max);
}

int write_access_pattern_nr_accesses(unsigned long min, unsigned long max)
{
	if (write_access_pattern_nr_accesses_min(min))
		return -1;
	if (write_access_pattern_nr_accesses_max(max))
		return -1;

	return 0;
}

int write_operation(const char *op)
{
	char *path =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/operations";

	if (!op)
		return -1;

	if (!strncmp(op, "vaddr", 5))
		goto write;
	else if (!strncmp(op, "paddr", 5))
		goto write;
	else
		return -1;
write:
	return write_sysfs_str(path, op);
}

int damon_write_action(unsigned int action)
{
	char *path =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/action";
	char *str = action_to_str(action);

	if (!strncmp(str, "none", 4))
		return -1;

	return write_sysfs_str(path, str);
}

int write_ds_pid(unsigned long pid)
{
	char *path =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/targets/0/pid_target";

	return write_sysfs_ulong(path, pid);
}

/*
 * write_ds_state - Write 'state' to DAMON_SYSFS.
 *
 * @state: state of kdamond [on, off, commit].
 */
int write_ds_state(const char *state)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	return write_sysfs_str(path, state);
}

int damon_is_enabled(bool *enabled)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	char buf[8];
	char *on = buf;

	if (read_sysfs_str(path, &on, sizeof(buf)))
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
		return write_ds_state("on");
	else
		return write_ds_state("off");
}

int damon_commit_params(void)
{
	char *path = "/sys/kernel/mm/damon/admin/kdamonds/0/state";
	return write_sysfs_str(path, "commit");
}

int read_damon_sz_applied(unsigned long *damon_sz_applied)
{
	char *path =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/stats/sz_applied";

	return read_sysfs_ulong(path, damon_sz_applied);
}

int damon_read_wmarks(struct wmarks *wmarks)
{
	char *high =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/high";
	char *mid =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/mid";
	char *low =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/low";

	if (read_sysfs_ulong(high, &wmarks->high))
		return -1;
	if (read_sysfs_ulong(mid, &wmarks->mid))
		return -1;
	if (read_sysfs_ulong(low, &wmarks->low))
		return -1;

	return 0;
}

int damon_write_wmarks(struct wmarks wmarks)
{
	char *high =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/high";
	char *mid =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/mid";
	char *low =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/watermarks/low";

	if (write_sysfs_ulong(high, wmarks.high))
		return -1;
	if (write_sysfs_ulong(mid, wmarks.mid))
		return -1;
	if (write_sysfs_ulong(low, wmarks.low))
		return -1;

	return 0;
}

int get_min_age(unsigned long *min_age)
{
	char *path =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/age/min";
	return read_sysfs_ulong(path, min_age);
}

int write_min_age(unsigned long min_age)
{
	char *path =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/access_pattern/age/min";

	if (write_sysfs_ulong(path, min_age))
		return -1;
	if (damon_commit_params())
		return -1;

	return 0;
}

int __damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz)
{
	char *ms =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/ms";
	char *sz =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/bytes";

	if (read_sysfs_ulong(ms, quota_ms))
		return -1;
	if (read_sysfs_ulong(sz, quota_sz))
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
	char *ms =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/ms";
	char *sz =
		"/sys/kernel/mm/damon/admin/kdamonds/0/contexts/0/schemes/0/quotas/bytes";

	if (write_sysfs_ulong(ms, quota_ms))
		return -1;
	if (write_sysfs_ulong(sz, quota_sz))
		return -1;

	return 0;
}

int damos_init(void)
{
	unsigned long min_age = MIN_AGE;
	struct wmarks wmarks;
	unsigned long quota_ms = QUOTA_MS;
	unsigned long quota_sz = QUOTA_SZ;

	if (SCHEME_WATERMARKS) {
		wmarks.high = 1000;
		wmarks.mid = 1000;
		wmarks.low = 0;
	} else {
		wmarks.high = WMARKS_HIGH;
		wmarks.mid = WMARKS_MID;
		wmarks.low = WMARKS_LOW;
	}

	if (write_min_age(min_age))
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
