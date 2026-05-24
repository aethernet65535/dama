#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include "core.h"
#include "sysfs.h"

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

int read_damon_nr_passed(unsigned int damon_module,
			 unsigned long *damon_nr_passed)
{
	char *module_name;

	if (module_to_name(damon_module, &module_name))
		return -1;

	if (damon_module == DAMON_RECLAIM) {
		if (damon_read_ulong(module_name, "bytes_reclaimed_regions",
				     damon_nr_passed))
			return -1;
	} else if (damon_module == DAMON_LRU_SORT) {
		if (damon_read_ulong(module_name,
				     "bytes_lru_sorted_cold_regions",
				     damon_nr_passed))
			return -1;
	}

	return 0;
}

bool is_supported_module(unsigned int damon_module)
{
	if (damon_module == DAMON_RECLAIM)
		return true;
	else if (damon_module == DAMON_LRU_SORT)
		return true;
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

	damon_info->psi_cpu = malloc(sizeof(struct psi));
	if (!damon_info->psi_cpu)
		return ret;

	damon_info->psi_io = malloc(sizeof(struct psi));
	if (!damon_info->psi_io)
		return ret;

	damon_info->psi_mem = malloc(sizeof(struct psi));
	if (!damon_info->psi_mem)
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

unsigned long inc_min_age(unsigned int damon_module, unsigned long step)
{
	unsigned long min_age;
	unsigned long limit = MAX_MIN_AGE;
	char *module_name;
	char *min_age_name;

	if (module_to_name(damon_module, &module_name))
		return -1;
	if (module_to_min_age(damon_module, &min_age_name))
		return -1;
	if (!step)
		return -1;
	if (damon_read_ulong(module_name, min_age_name, &min_age))
		return -1;

	/* Overflow */
	if (min_age + step > limit)
		min_age = limit;
	else
		min_age += step;

	if (damon_write_ulong(module_name, min_age_name, min_age))
		return -1;
	if (damon_commit_params(module_name))
		return -1;

	pr_time("inc_min_age(): %lu\n", min_age);

	return 0;
}

unsigned long dec_min_age(unsigned int damon_module, unsigned long step)
{
	unsigned long min_age;
	unsigned long limit = MIN_MIN_AGE;
	char *module_name;
	char *min_age_name;

	if (module_to_name(damon_module, &module_name))
		return -1;
	if (module_to_min_age(damon_module, &min_age_name))
		return -1;
	if (!step)
		return -1;
	if (damon_read_ulong(module_name, min_age_name, &min_age))
		return -1;

	/* Underflow */
	if (min_age - step > limit)
		min_age = limit;
	else
		min_age -= step;

	if (damon_write_ulong(module_name, min_age_name, min_age))
		return -1;
	if (damon_commit_params(module_name))
		return -1;

	pr_time("dec_min_age(): %lu\n", min_age);

	return 0;
}

/* 
 * reclaim_min_age_calc - calculate 'min_age'.
 * @inc: address to get the step of increase.
 * @dec: address to get the step of decrease.
 * @ctx: address to save the context.
 *
 * Algorithm (TLDR)
 * ================
 *
 * Decrease:
 * When the refault is lower than DECREASE_THRESHOLD, the 'min_age' will
 * decrease until 'pgsteal' or 'refault' equals zero.
 *
 * Increase:
 * When the refault is greater than INCREASE_THRESHOLD, the 'min_age' will
 * increase until 'ctx->nr_damon_passed' or 'refault' equals zero.
 *
 * Note that, refault is not only caused by DAMON/Direct/Kswapd reclaimation.
 * 
 */
int reclaim_min_age_calc(unsigned long *inc, unsigned long *dec, struct ma_calc *ctx)
{
	unsigned long total_kswapd_reclaimed = 0;
	unsigned long total_direct_reclaimed = 0;
	unsigned long total_nr_damon_passed = 0;
	unsigned long total_refault_anon = 0;
	unsigned long total_refault_file = 0;
	unsigned long pgsteal;
	unsigned long refault = 0;
	unsigned long factor, nr_dec = 0, nr_inc = 0;
	unsigned long percentage = 0;
	int ret = -1;

	factor = MIN_AGE_FACTOR;
	ctx->anon_weight = ANON_REFAULT_WEIGHT;
	ctx->file_weight = FILE_REFAULT_WEIGHT;

	if (!inc || !dec)
		goto done;
	if (read_refault(&total_refault_anon, &total_refault_file))
		goto done;
	if (read_steal(&total_kswapd_reclaimed, &total_direct_reclaimed))
		goto done;
	if (read_damon_nr_passed(DAMON_RECLAIM, &total_nr_damon_passed))
		goto done;

	total_nr_damon_passed = total_nr_damon_passed / PAGE_SIZE;

	if (!ctx->init)
		goto update;

	ctx->refault_anon += total_refault_anon - ctx->last_refault_anon;
	ctx->refault_file += total_refault_file - ctx->last_refault_file;

	ctx->kswapd_reclaimed +=
		total_kswapd_reclaimed - ctx->last_kswapd_reclaimed;
	ctx->direct_reclaimed +=
		total_direct_reclaimed - ctx->last_direct_reclaimed;
	ctx->nr_damon_passed +=
		total_nr_damon_passed - ctx->last_nr_damon_passed;

	refault = (ctx->refault_anon * ctx->anon_weight) +
		  (ctx->refault_file * ctx->file_weight);

	if (!refault)
		goto update;
	if (!ctx->kswapd_reclaimed && !ctx->direct_reclaimed &&
	    !ctx->nr_damon_passed)
		goto update;

	/* DAMON - Increase */
	do {
		if (!ctx->nr_damon_passed)
			break;

		refault = (ctx->refault_anon * ctx->anon_weight) +
			  (ctx->refault_file * ctx->file_weight);
		percentage = (refault * 100) / ctx->nr_damon_passed;
		if (!(percentage >= INCREASE_THRESHOLD))
			break;

		ctx->nr_damon_passed = PERCENT(ctx->nr_damon_passed, factor);
		ctx->kswapd_reclaimed = PERCENT(ctx->kswapd_reclaimed, factor);
		ctx->direct_reclaimed = PERCENT(ctx->direct_reclaimed, factor);
		ctx->refault_anon = PERCENT(ctx->refault_anon, factor);
		ctx->refault_file = PERCENT(ctx->refault_file, factor);
		nr_inc += INCREASE_BASE;
	} while (percentage >= INCREASE_THRESHOLD);

	if (nr_inc)
		goto update;

	/* KSWAPD + DIRECT - Decrease */
	do {
		pgsteal = ctx->kswapd_reclaimed + ctx->direct_reclaimed;
		if (!pgsteal)
			break;

		refault = (ctx->refault_anon * ctx->anon_weight) +
			  (ctx->refault_file * ctx->file_weight);
		percentage = (refault * 100) / pgsteal;
		if (!(percentage <= DECREASE_THRESHOLD))
			break;

		ctx->nr_damon_passed = PERCENT(ctx->nr_damon_passed, factor);
		ctx->kswapd_reclaimed = PERCENT(ctx->kswapd_reclaimed, factor);
		ctx->direct_reclaimed = PERCENT(ctx->direct_reclaimed, factor);
		ctx->refault_anon = PERCENT(ctx->refault_anon, factor);
		ctx->refault_file = PERCENT(ctx->refault_file, factor);
		nr_dec += DECREASE_BASE;
	} while (percentage <= DECREASE_THRESHOLD);

update:
	ctx->last_kswapd_reclaimed = total_kswapd_reclaimed;
	ctx->last_direct_reclaimed = total_direct_reclaimed;
	ctx->last_nr_damon_passed = total_nr_damon_passed;
	ctx->last_refault_anon = total_refault_anon;
	ctx->last_refault_file = total_refault_file;
	*inc = nr_inc;
	*dec = nr_dec;
	ctx->init = true;
	ret = 0;
done:
	return ret;
}

/* Testing... */
int lru_min_age_calc(unsigned long *inc, unsigned long *dec, struct ma_calc *ctx)
{
	unsigned long total_kswapd_reclaimed = 0;
	unsigned long total_direct_reclaimed = 0;
	unsigned long total_nr_damon_passed = 0;
	unsigned long total_refault_anon = 0;
	unsigned long total_refault_file = 0;
	unsigned long pgsteal;
	unsigned long refault = 0;
	unsigned long factor, nr_dec = 0, nr_inc = 0;
	unsigned long percentage_refault = 0, percentage_passed = 0;
	int ret = -1;

	factor = MIN_AGE_FACTOR;
	ctx->anon_weight = ANON_REFAULT_WEIGHT;
	ctx->file_weight = FILE_REFAULT_WEIGHT;

	if (!inc || !dec)
		goto done;
	if (read_refault(&total_refault_anon, &total_refault_file))
		goto done;
	if (read_steal(&total_kswapd_reclaimed, &total_direct_reclaimed))
		goto done;
	if (read_damon_nr_passed(DAMON_LRU_SORT, &total_nr_damon_passed))
		goto done;

	total_nr_damon_passed = total_nr_damon_passed / PAGE_SIZE;

	if (!ctx->init)
		goto update;

	ctx->refault_anon += total_refault_anon - ctx->last_refault_anon;
	ctx->refault_file += total_refault_file - ctx->last_refault_file;

	ctx->kswapd_reclaimed +=
		total_kswapd_reclaimed - ctx->last_kswapd_reclaimed;
	ctx->direct_reclaimed +=
		total_direct_reclaimed - ctx->last_direct_reclaimed;
	ctx->nr_damon_passed +=
		total_nr_damon_passed - ctx->last_nr_damon_passed;

	refault = (ctx->refault_anon * ctx->anon_weight) +
		  (ctx->refault_file * ctx->file_weight);

	if (!refault)
		goto update;
	if (!ctx->kswapd_reclaimed && !ctx->direct_reclaimed &&
	    !ctx->nr_damon_passed)
		goto update;

	ctx->nr_damon_passed = PERCENT(ctx->nr_damon_passed, factor);

	do {
		pgsteal = ctx->kswapd_reclaimed + ctx->direct_reclaimed;
		if (!pgsteal)
			break;

		refault = (ctx->refault_anon * ctx->anon_weight) +
			  (ctx->refault_file * ctx->file_weight);
		percentage_refault = (refault * 100) / pgsteal;
		if (!(percentage_refault >= INCREASE_THRESHOLD))
			break;

		percentage_passed = (ctx->nr_damon_passed * 100) / pgsteal;
		if (percentage_passed >= INCREASE_THRESHOLD)
			nr_inc += INCREASE_BASE;
		else
			nr_dec += DECREASE_BASE;

		ctx->nr_damon_passed = PERCENT(ctx->nr_damon_passed, factor);
		ctx->kswapd_reclaimed = PERCENT(ctx->kswapd_reclaimed, factor);
		ctx->direct_reclaimed = PERCENT(ctx->direct_reclaimed, factor);
		ctx->refault_anon = PERCENT(ctx->refault_anon, factor);
		ctx->refault_file = PERCENT(ctx->refault_file, factor);
	} while (percentage_refault >= INCREASE_THRESHOLD);

update:
	ctx->last_kswapd_reclaimed = total_kswapd_reclaimed;
	ctx->last_direct_reclaimed = total_direct_reclaimed;
	ctx->last_nr_damon_passed = total_nr_damon_passed;
	ctx->last_refault_anon = total_refault_anon;
	ctx->last_refault_file = total_refault_file;
	*inc = nr_inc;
	*dec = nr_dec;
	ctx->init = true;
	ret = 0;
done:
	return ret;
}

int min_age_calc(unsigned long *inc, unsigned long *dec, struct ma_calc *ctx,
		 unsigned int damon_module)
{
	if (damon_module == DAMON_RECLAIM)
		return reclaim_min_age_calc(inc, dec, ctx);
	else if (damon_module == DAMON_LRU_SORT)
		return lru_min_age_calc(inc, dec, ctx);
	else
		return -1;
}

/*
 * The Heart of the DAMA.
 */
int udamond_fn(struct damon_info *info)
{
	unsigned int sleep_us;
	bool is_in_kdamond_watermark;
	unsigned long nr_dec = 0, nr_inc = 0, no_dec = 0;
	unsigned long memtotal, memfree;
	struct ma_calc ma_calc = {};
	char *module_name;

	if (!info)
		goto done;
	if (module_to_name(info->damon_module, &module_name))
		goto done;
	if (!is_supported_module(info->damon_module))
		goto done;

	pr_time("udamond start %s\n", module_name);

	while (true) {
		sleep_us = UDAMOND_SLEEP_US;

		if (read_meminfo(&memtotal, &memfree, NULL, NULL))
			goto done;
		if (damon_read_wmarks(module_name, &info->wmarks))
			goto done;

		is_in_kdamond_watermark =
			PERCENT(memtotal, info->wmarks.high) >= memfree;

		if (!is_in_kdamond_watermark)
			goto rest;

		if (min_age_calc(&nr_inc, &nr_dec, &ma_calc,
				 info->damon_module))
			goto done;

		no_dec += min(nr_inc, MAX_NO_DECREASE);

		if (nr_inc) { /* Increase 'min_age' */
			if (inc_min_age(info->damon_module, nr_inc * STEP_BASE))
				goto done;
		} else if (!no_dec && nr_dec) { /* Decrease 'min_age' */
			if (dec_min_age(info->damon_module, nr_dec * STEP_BASE))
				goto done;
		}

	rest:
		no_dec = no_dec ? no_dec - 1 : 0;

		usleep(sleep_us);
	}
done:
	pr_time("udamond stop %s\n", module_name);
	free(info);
	return 0;
}
