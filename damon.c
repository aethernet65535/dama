#include <errno.h>
#include "core.h"
#include "log.h"
#include "util.h"
#include "sysfs.h"

int damon_read_ulong(const char *module_name, const char *param,
		     unsigned long *val)
{
	char path[512];
	int ret, len;

	len = snprintf(path, sizeof(path), "/sys/module/%s/parameters/%s",
		       module_name, param);
	if (len < 0 || len >= (int)sizeof(path)) {
		return -ENAMETOOLONG;
	}

	ret = read_sysfs_ulong(path, val);
	if (ret)
		return ret;

	return 0;
}

int damon_read_bool(const char *module_name, const char *param, bool *val)
{
	char path[512];
	int ret, len;
	char c;

	len = snprintf(path, sizeof(path), "/sys/module/%s/parameters/%s",
		       module_name, param);
	if (len < 0 || len >= (int)sizeof(path)) {
		return -ENAMETOOLONG;
	}

	ret = read_sysfs_char(path, &c);
	if (ret)
		return ret;

	*val = strtobool(&c);

	return 0;
}

int damon_write_ulong(const char *module_name, const char *param,
		      unsigned long val)
{
	char path[512];
	int len;

	len = snprintf(path, sizeof(path), "/sys/module/%s/parameters/%s",
		       module_name, param);
	if (len < 0 || len >= (int)sizeof(path)) {
		return -ENAMETOOLONG;
	}

	return write_sysfs_ulong(path, val);
}

int damon_write_bool(const char *module_name, const char *param, bool val)
{
	char path[512];
	int len;

	len = snprintf(path, sizeof(path), "/sys/module/%s/parameters/%s",
		       module_name, param);
	if (len < 0 || len >= (int)sizeof(path)) {
		return -ENAMETOOLONG;
	}

	return write_sysfs_bool(path, val);
}

int damon_is_enabled(const char *module_name, bool *enabled)
{
	int ret;
	bool val;

	ret = damon_read_bool(module_name, "enabled", &val);
	if (ret)
		return ret;

	*enabled = val ? true : false;
	return 0;
}

int damon_set_enabled(const char *module_name, bool on)
{
	return damon_write_bool(module_name, "enabled", on);
}

int damon_commit_params(const char *module_name)
{
	return damon_write_bool(module_name, "commit_inputs", true);
}

int read_damon_nr_applied(unsigned int damon_module,
			  unsigned long *damon_nr_applied)
{
	char *module_name;

	if (module_to_name(damon_module, &module_name))
		return -1;

	if (damon_module == DAMON_RECLAIM) {
		if (damon_read_ulong(module_name, "bytes_reclaimed_regions",
				     damon_nr_applied))
			return -1;
	} else if (damon_module == DAMON_LRU_SORT) {
		if (damon_read_ulong(module_name,
				     "bytes_lru_sorted_cold_regions",
				     damon_nr_applied))
			return -1;
	}

	return 0;
}

int damon_read_wmarks(char *module_name, struct wmarks *wmarks)
{
	if (!module_name)
		return 0;

	if (damon_read_ulong(module_name, "wmarks_high", &wmarks->high))
		return -1;
	if (damon_read_ulong(module_name, "wmarks_mid", &wmarks->mid))
		return -1;
	if (damon_read_ulong(module_name, "wmarks_low", &wmarks->low))
		return -1;

	return 0;
}

int damon_write_wmarks(char *module_name, struct wmarks wmarks)
{
	if (!module_name)
		return 0;

	if (damon_write_ulong(module_name, "wmarks_high", wmarks.high))
		return -1;
	if (damon_write_ulong(module_name, "wmarks_mid", wmarks.mid))
		return -1;
	if (damon_write_ulong(module_name, "wmarks_low", wmarks.low))
		return -1;

	return 0;
}

int module_to_min_age(unsigned int damon_module, char **min_age)
{
	if (!min_age)
		return -1;

	if (damon_module == DAMON_RECLAIM)
		*min_age = "min_age";
	else if (damon_module == DAMON_LRU_SORT)
		*min_age = "cold_min_age";
	else
		return -1;

	return 0;
}

int get_min_age(unsigned int damon_module, unsigned long *min_age)
{
	char *min_age_name = NULL;
	char *module_name = NULL;

	if (module_to_name(damon_module, &module_name))
		return -1;
	if (module_to_min_age(damon_module, &min_age_name))
		return -1;
	if (damon_read_ulong(module_name, min_age_name, min_age))
		return -1;

	return 0;
}

int write_min_age(unsigned int damon_module, unsigned long min_age)
{
	char *min_age_name = NULL;
	char *module_name = NULL;

	if (module_to_name(damon_module, &module_name))
		return -1;
	if (module_to_min_age(damon_module, &min_age_name))
		return -1;
	if (damon_write_ulong(module_name, min_age_name, min_age))
		return -1;
	if (damon_commit_params(module_name))
		return -1;

	return 0;
}

int __damon_read_quota(char *module_name, unsigned long *quota_ms,
		       unsigned long *quota_sz)
{
	if (damon_read_ulong(module_name, "quota_ms", quota_ms))
		return -1;
	if (damon_read_ulong(module_name, "quota_sz", quota_sz))
		return -1;

	return 0;
}

int damon_read_quota(char *module_name, unsigned long *quota_ms,
		     unsigned long *quota_sz)
{
	unsigned long tmp_ms, tmp_sz;
	int ret;

	ret = __damon_read_quota(module_name, &tmp_ms, &tmp_sz);
	if (ret)
		return ret;

	if (tmp_ms)
		*quota_ms = tmp_ms;
	if (tmp_sz)
		*quota_sz = tmp_sz;

	return 0;
}

int damon_write_quota(char *module_name, unsigned long quota_ms,
		      unsigned long quota_sz)
{
	if (damon_write_ulong(module_name, "quota_ms", quota_ms))
		return -1;
	if (damon_write_ulong(module_name, "quota_sz", quota_sz))
		return -1;

	return 0;
}

int damos_init(void)
{
	unsigned int damon_module = DAMON_MODULE;
	unsigned long min_age = MIN_AGE;
	struct wmarks wmarks;
	unsigned long quota_ms = QUOTA_MS;
	unsigned long quota_sz = QUOTA_SZ;
	char *module_name = NULL;

	if (SCHEME_WATERMARKS) {
		wmarks.high = 1000;
		wmarks.mid = 1000;
		wmarks.low = 0;
	} else {
		wmarks.high = WMARKS_HIGH;
		wmarks.mid = WMARKS_MID;
		wmarks.low = WMARKS_LOW;
	}

	if (module_to_name(damon_module, &module_name))
		return -1;
	if (write_min_age(damon_module, min_age))
		return -1;
	if (damon_write_wmarks(module_name, wmarks))
		return -1;
	if (damon_write_quota(module_name, quota_ms, quota_sz))
		return -1;
	if (damon_commit_params(module_name))
		return -1;

	return 0;
}
