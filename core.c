#include <sys/param.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "core.h"
#include "sysfs.h"
#include "log.h"

unsigned long times = 0;

unsigned long gettime_us(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_usec;
}

bool strtobool(const char *str)
{
	if (!str)
		return false;

	return (str[0] == 'Y' || str[0] == 'y' || str[0] == '1');
}

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

bool is_supported_module(unsigned int damon_module)
{
	if (damon_module == DAMON_RECLAIM)
		return true;
	else if (damon_module == DAMON_LRU_SORT)
		return false;
	else
		return false;
}

int module_to_name(unsigned int damon_module, char **module_name)
{
	if (!module_name)
		return -1;

	if (damon_module == DAMON_RECLAIM)
		*module_name = "damon_reclaim";
	else if (damon_module == DAMON_LRU_SORT)
		*module_name = "damon_lru_sort";
	else
		return -1;

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

	if (wmarks->high)
		wmarks->high /= 10;
	if (wmarks->mid)
		wmarks->mid /= 10;
	if (wmarks->low)
		wmarks->low /= 10;

	return 0;
}

void *alloc_damon_info(void)
{
	struct damon_info *damon_info = malloc(sizeof(struct damon_info));
	void *ret = NULL;

	if (!damon_info)
		return ret;

	damon_info->stats = malloc(sizeof(struct sysfs_param));
	if (!damon_info->stats)
		return ret;

	damon_info->ma_calc = malloc(sizeof(struct ma_calc));
	if (!damon_info->ma_calc)
		return ret;

	return damon_info;
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

unsigned long inc_min_age(unsigned int damon_module, unsigned long step)
{
	unsigned long min_age;

	if (!step)
		return -1;
	if (get_min_age(damon_module, &min_age))
		return -1;

	if (min_age + step < min_age)
		min_age = MAX_MIN_AGE;
	else
		min_age = min(MAX_MIN_AGE, min_age + step);

	if (write_min_age(damon_module, min_age))
		return -1;

	pr_time("[%lu] inc_min_age(): %lu\n", times, min_age);

	return 0;
}

unsigned long dec_min_age(unsigned int damon_module, unsigned long step)
{
	unsigned long min_age;

	if (!step)
		return -1;
	if (get_min_age(damon_module, &min_age))
		return -1;

	/* Underflow */
	if (min_age - step > min_age)
		min_age = MIN_MIN_AGE;
	else
		min_age = max(MIN_MIN_AGE, min_age - step);

	if (write_min_age(damon_module, min_age))
		return -1;

	pr_time("[%lu] dec_min_age(): %lu\n", times, min_age);

	return 0;
}

/* 
 * reclaim_min_age_calc - calculate 'min_age'.
 * @inc: address to get the step of increase.
 * @dec: address to get the step of decrease.
 * @ctx: address to save the context.
 *
 * Note that, refault is not only caused by DAMON/Direct/Kswapd reclaimation.
 */
int reclaim_min_age_calc(unsigned long *inc, unsigned long *dec,
			 struct ma_calc *ctx)
{
	unsigned long delta_kswapd_reclaimed = 0;
	unsigned long delta_direct_reclaimed = 0;
	unsigned long total_kswapd_reclaimed = 0;
	unsigned long total_direct_reclaimed = 0;
	unsigned long total_nr_damon_applied = 0;
	unsigned long delta_damon_reclaimed = 0;
	unsigned long total_refault_anon = 0;
	unsigned long total_refault_file = 0;
	unsigned long refault_weighted = 0;
	unsigned long damon_reclaimed = 0;
	unsigned long delta_pgsteal = 0;
	unsigned long percentage = 0;
	unsigned long pgsteal = 0;
	unsigned long min_age = 0;
	unsigned long nr_inc = 0;
	unsigned long nr_dec = 0;
	unsigned long diff = 0;

	ctx->anon_weight = ANON_REFAULT_WEIGHT;
	ctx->file_weight = FILE_REFAULT_WEIGHT;

	if (!inc || !dec)
		goto done;
	if (read_refault(&total_refault_anon, &total_refault_file))
		goto done;
	if (read_steal(&total_kswapd_reclaimed, &total_direct_reclaimed))
		goto done;
	if (read_damon_nr_applied(DAMON_RECLAIM, &total_nr_damon_applied))
		goto done;

	total_nr_damon_applied = total_nr_damon_applied / PAGE_SIZE;

	if (!ctx->init)
		goto update;

	ctx->refault_anon += total_refault_anon - ctx->last_refault_anon;
	ctx->refault_file += total_refault_file - ctx->last_refault_file;

	delta_kswapd_reclaimed =
		total_kswapd_reclaimed - ctx->last_kswapd_reclaimed;
	delta_direct_reclaimed =
		total_direct_reclaimed - ctx->last_direct_reclaimed;
	delta_pgsteal = delta_kswapd_reclaimed + delta_direct_reclaimed;
	ctx->kswapd_reclaimed += delta_kswapd_reclaimed;
	ctx->direct_reclaimed += delta_direct_reclaimed;
	pgsteal = ctx->kswapd_reclaimed + ctx->direct_reclaimed;
	ctx->remaining_pgsteal += delta_pgsteal;

	delta_damon_reclaimed =
		total_nr_damon_applied - ctx->last_nr_damon_applied;
	ctx->nr_damon_applied += delta_damon_reclaimed;
	damon_reclaimed = ctx->nr_damon_applied;
	ctx->remaining_damon += delta_damon_reclaimed;

	refault_weighted = (ctx->refault_anon * ctx->anon_weight) +
			   (ctx->refault_file * ctx->file_weight);

	if (!refault_weighted)
		goto update;
	if (!pgsteal && !damon_reclaimed)
		goto update;
	if (get_min_age(DAMON_RECLAIM, &min_age))
		goto done;

	/* 
	 * DAMON - Increase
	 *
	 * To ensure that DAMA can determine the cause of most
	 * refaults as accurately as possible, DAMA will decay the
	 * 'ctx->remaining_{*}' of another "reclaim group" as quickly
	 * as possible, and slowly decay its own 'ctx->remaining_{*}'.
	 */
	if (ctx->remaining_damon >= DAMON_THRESHOLD && damon_reclaimed) {
		ctx->remaining_damon =
			PERCENT(ctx->remaining_damon, WORKING_FACTOR);
		ctx->remaining_pgsteal =
			min(PGSTEAL_THRESHOLD, PERCENT(ctx->remaining_pgsteal,
						       NOT_WORKING_FACTOR));
		percentage = (refault_weighted * 100) / damon_reclaimed;
		if (percentage > INCREASE_THRESHOLD) {
			pr_time("[%lu] percentage-damon: %lu\n", times,
				percentage);
			diff = (percentage - INCREASE_THRESHOLD);
			nr_inc = PERCENT(min_age, diff);
			goto fade;
		}
	}

	/* KSWAPD+DIRECT - Decrase */
	if (ctx->remaining_pgsteal >= PGSTEAL_THRESHOLD && pgsteal) {
		ctx->remaining_pgsteal =
			PERCENT(ctx->remaining_pgsteal, WORKING_FACTOR);
		ctx->remaining_damon =
			min(DAMON_THRESHOLD,
			    PERCENT(ctx->remaining_damon, NOT_WORKING_FACTOR));
		percentage = (refault_weighted * 100) / pgsteal;
		if (percentage < DECREASE_THRESHOLD) {
			pr_time("[%lu] percentage-system: %lu\n", times,
				percentage);
			diff = (DECREASE_THRESHOLD - percentage);
			nr_dec = PERCENT(min_age, diff);
			goto fade;
		}
	}

	goto update;
fade:
	/*
	 * Keep the amount of reclaim and the amount of refault
	 * decaying at the same rate.
	 */
	while (true) {
		refault_weighted = ctx->refault_anon + ctx->refault_file;
		damon_reclaimed = ctx->nr_damon_applied;
		pgsteal = ctx->kswapd_reclaimed + ctx->direct_reclaimed;

		if (!refault_weighted || !pgsteal || !damon_reclaimed)
			break;

		ctx->nr_damon_applied =
			PERCENT(ctx->nr_damon_applied, FADE_FACTOR);
		ctx->kswapd_reclaimed =
			PERCENT(ctx->kswapd_reclaimed, FADE_FACTOR);
		ctx->direct_reclaimed =
			PERCENT(ctx->direct_reclaimed, FADE_FACTOR);
		ctx->refault_anon = PERCENT(ctx->refault_anon, FADE_FACTOR);
		ctx->refault_file = PERCENT(ctx->refault_file, FADE_FACTOR);
	}

	/* 
	 * DAMA should discard refaults data as much as possible
	 * to avoid inaccurate interference from this data in
	 * the next calculation.
	 */
	ctx->refault_anon = 0;
	ctx->refault_file = 0;
update:
	ctx->last_kswapd_reclaimed = total_kswapd_reclaimed;
	ctx->last_direct_reclaimed = total_direct_reclaimed;
	ctx->last_nr_damon_applied = total_nr_damon_applied;
	ctx->last_refault_anon = total_refault_anon;
	ctx->last_refault_file = total_refault_file;
	*inc = nr_inc;
	*dec = nr_dec;
	ctx->init = true;
	return 0;
done:
	return -1;
}

int min_age_calc(unsigned long *inc, unsigned long *dec, struct ma_calc *ctx,
		 unsigned int damon_module)
{
	if (damon_module == DAMON_RECLAIM)
		return reclaim_min_age_calc(inc, dec, ctx);
	else if (damon_module == DAMON_LRU_SORT)
		return -1;
	else
		return -1;
}

/*
 * The Heart of the DAMA.
 */
int udamond_fn(struct damon_info *info)
{
	bool is_in_kdamond_watermark;
	char *module_name = NULL;
	unsigned long nr_inc = 0;
	unsigned long nr_dec = 0;
	unsigned long no_dec = 0;
	unsigned long memtotal;
	unsigned long memfree;

	unsigned long start_us;
	unsigned long end_us;

	if (!info)
		goto done;
	if (module_to_name(info->damon_module, &module_name))
		goto done;
	if (!is_supported_module(info->damon_module))
		goto done;

	pr_time("udamond start %s\n", module_name);

	while (true) {
		times++;
		start_us = gettime_us();

		if (read_meminfo(&memtotal, &memfree, NULL, NULL))
			goto done;
		if (damon_read_wmarks(module_name, &info->wmarks))
			goto done;

		is_in_kdamond_watermark =
			PERCENT(memtotal, info->wmarks.high) >= memfree;

		if (!is_in_kdamond_watermark)
			goto rest;

		if (min_age_calc(&nr_inc, &nr_dec, info->ma_calc,
				 info->damon_module))
			goto done;

		no_dec = 0;

		if (nr_inc) { /* Increase 'min_age' */
			if (inc_min_age(info->damon_module, nr_inc))
				goto done;
		} else if (!no_dec && nr_dec) { /* Decrease 'min_age' */
			if (dec_min_age(info->damon_module, nr_dec))
				goto done;
		}

	rest:
		no_dec = no_dec ? no_dec - 1 : 0;

		end_us = gettime_us();
		info->ma_calc->remaining_time_us += end_us - start_us;
		usleep(UDAMOND_SLEEP_US);
	}
done:
	pr_time("udamond stop %s\n", module_name);
	free(info);
	return 0;
}
