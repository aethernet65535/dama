#include "core.h"
#include "damon.h"
#include "sysfs.h"
#include "log.h"
#include "util.h"

unsigned long ticks = 0;

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

void *alloc_damon_info(void)
{
	struct damon_info *damon_info = malloc(sizeof(struct damon_info));
	void *ret = NULL;

	if (!damon_info)
		return ret;

	damon_info->param = zalloc(sizeof(struct damos_param));
	if (!damon_info->param)
		return ret;

	damon_info->mas_calc = zalloc(sizeof(struct mas_calc));
	if (!damon_info->mas_calc)
		return ret;

	return damon_info;
}

int inc_min_age(unsigned int damon_module, unsigned long step,
		unsigned long *min_age)
{
	if (!step)
		return -1;
	if (get_min_age(damon_module, min_age))
		return -1;

	if (*min_age + step < *min_age)
		*min_age = MAX_MIN_AGE;
	else
		*min_age = min(MAX_MIN_AGE, *min_age + step);

	pr_time("[%lu] inc_min_age(): %lu\n", ticks, *min_age);

	return 0;
}

int dec_min_age(unsigned int damon_module, unsigned long step,
		unsigned long *min_age)
{
	if (!step)
		return -1;
	if (get_min_age(damon_module, min_age))
		return -1;

	/* Underflow */
	if (*min_age - step > *min_age)
		*min_age = MIN_MIN_AGE;
	else
		*min_age = max(MIN_MIN_AGE, *min_age - step);

	pr_time("[%lu] dec_min_age(): %lu\n", ticks, *min_age);

	return 0;
}

unsigned long update_mas(struct mas_calc *ctx)
{
	unsigned long total_refault_anon = 0;
	unsigned long total_refault_file = 0;
	unsigned long total_kswapd_reclaimed = 0;
	unsigned long total_direct_reclaimed = 0;
	unsigned long total_nr_damon_applied = 0;

	unsigned long d_damon_reclaimed = 0;
	unsigned long d_pgsteal = 0;
	unsigned long d_kswapd_reclaimed = 0;
	unsigned long d_direct_reclaimed = 0;

	ctx->state.anon_weight = ANON_REFAULT_WEIGHT;
	ctx->state.file_weight = FILE_REFAULT_WEIGHT;

	if (read_refault(&total_refault_anon, &total_refault_file))
		goto err;
	if (read_steal(&total_kswapd_reclaimed, &total_direct_reclaimed))
		goto err;
	if (read_damon_nr_applied(DAMON_RECLAIM, &total_nr_damon_applied))
		goto err;

	if (!ctx->state.init)
		goto update;

	d_kswapd_reclaimed =
		total_kswapd_reclaimed - ctx->last.kswapd_reclaimed;
	d_direct_reclaimed =
		total_direct_reclaimed - ctx->last.direct_reclaimed;
	d_pgsteal = d_kswapd_reclaimed + d_direct_reclaimed;
	d_damon_reclaimed = total_nr_damon_applied - ctx->last.nr_damon_applied;
update:
	ctx->total.nr_damon_applied = total_nr_damon_applied / PAGE_SIZE;
	ctx->total.kswapd_reclaimed = total_kswapd_reclaimed;
	ctx->total.direct_reclaimed = total_direct_reclaimed;
	ctx->total.refault_anon = total_refault_anon;
	ctx->total.refault_file = total_refault_file;

	ctx->metric.nr_damon_applied += d_damon_reclaimed;
	ctx->metric.kswapd_reclaimed += d_kswapd_reclaimed;
	ctx->metric.direct_reclaimed += d_direct_reclaimed;
	ctx->metric.refault_anon += total_refault_anon - ctx->last.refault_anon;
	ctx->metric.refault_file += total_refault_file - ctx->last.refault_file;

	ctx->remaining.damon += d_damon_reclaimed;
	ctx->remaining.pgsteal += d_pgsteal;

	if (!ctx->state.init)
		return MAS_NEED_INIT;
	return 0;
err:
	return MAS_ERR;
}

int reclaim_step_calc(struct mas_calc *ctx, int damon_module)
{
	unsigned long damon_reclaimed = ctx->metric.nr_damon_applied;
	unsigned long kswapd_reclaimed = ctx->metric.kswapd_reclaimed;
	unsigned long direct_reclaimed = ctx->metric.direct_reclaimed;
	unsigned long pgsteal = kswapd_reclaimed + direct_reclaimed;
	unsigned long refault_anon = ctx->metric.refault_anon;
	unsigned long refault_file = ctx->metric.refault_file;
	unsigned long anon_weight = ctx->state.anon_weight;
	unsigned long file_weight = ctx->state.file_weight;
	unsigned long anon_weighted = refault_anon * anon_weight;
	unsigned long file_weighted = refault_file * file_weight;
	unsigned long nr_inc = 0;
	unsigned long nr_dec = 0;
	unsigned long refault_weighted, min_age;

	refault_weighted = anon_weighted + file_weighted;

	if (get_min_age(damon_module, &min_age))
		goto err;

	/* 
	 * DAMON - Increase
	 *
	 * To ensure that DAMA can determine the cause of most
	 * refaults as accurately as possible, DAMA will decay the
	 * 'ctx->remaining_{*}' of another "reclaim group" as quickly
	 * as possible, and slowly decay its own 'ctx->remaining_{*}'.
	 */
	if (ctx->remaining.damon >= DAMON_THRESHOLD && damon_reclaimed) {
		unsigned long percentage, diff;

		pct(&ctx->remaining.damon, WORKING_FACTOR);
		pct(&ctx->remaining.pgsteal, NOT_WORKING_FACTOR);
		if (ctx->remaining.pgsteal > PGSTEAL_THRESHOLD) {
			ctx->remaining.pgsteal = PGSTEAL_THRESHOLD;
		}

		percentage = (refault_weighted * 100) / damon_reclaimed;
		if (percentage > INCREASE_THRESHOLD) {
			pr_time("[%lu] percentage-damon: %lu\n", ticks,
				percentage);
			diff = (percentage - INCREASE_THRESHOLD);
			nr_inc = PERCENT(min_age, diff);
		}
	}

	/* KSWAPD+DIRECT - Decrase */
	if (ctx->remaining.pgsteal >= PGSTEAL_THRESHOLD && pgsteal) {
		unsigned long percentage, diff;

		pct(&ctx->remaining.pgsteal, WORKING_FACTOR);
		pct(&ctx->remaining.damon, NOT_WORKING_FACTOR);
		if (ctx->remaining.damon > DAMON_THRESHOLD) {
			ctx->remaining.damon = DAMON_THRESHOLD;
		}

		percentage = (refault_weighted * 100) / pgsteal;
		if (percentage < DECREASE_THRESHOLD) {
			pr_time("[%lu] percentage-system: %lu\n", ticks,
				percentage);
			diff = (DECREASE_THRESHOLD - percentage);
			nr_dec = PERCENT(min_age, diff);
		}
	}

	ctx->result.nr_inc = nr_inc;
	ctx->result.nr_dec = nr_dec;

	return 0;
err:
	return -1;
}

int fade_mas(struct mas_calc *ctx)
{
	/*
	 * Keep the amount of reclaim and the amount of refault
	 * decaying at the same rate.
	 */
	while (true) {
		unsigned long refault_weighted, damon_reclaimed, pgsteal;
		unsigned long refault_anon = ctx->metric.refault_anon;
		unsigned long refault_file = ctx->metric.refault_file;
		unsigned long nr_damon_applied = ctx->metric.nr_damon_applied;
		unsigned long kswapd_reclaimed = ctx->metric.kswapd_reclaimed;
		unsigned long direct_reclaimed = ctx->metric.direct_reclaimed;

		refault_weighted = refault_anon + refault_file;
		damon_reclaimed = nr_damon_applied;
		pgsteal = kswapd_reclaimed + direct_reclaimed;

		if (!refault_weighted || !pgsteal || !damon_reclaimed)
			break;

		pct(&ctx->metric.nr_damon_applied, FADE_FACTOR);
		pct(&ctx->metric.kswapd_reclaimed, FADE_FACTOR);
		pct(&ctx->metric.direct_reclaimed, FADE_FACTOR);
		pct(&ctx->metric.refault_anon, FADE_FACTOR);
		pct(&ctx->metric.refault_file, FADE_FACTOR);
	}

	/* 
	 * DAMA should discard refaults data as much as possible
	 * to avoid inaccurate interference from this data in
	 * the next calculation.
	 */
	ctx->metric.refault_anon = 0;
	ctx->metric.refault_file = 0;

	return 0;
}

int record_mas(struct mas_calc *ctx)
{
	unsigned long total_nr_damon_applied = ctx->total.nr_damon_applied;
	unsigned long total_kswapd_reclaimed = ctx->total.kswapd_reclaimed;
	unsigned long total_direct_reclaimed = ctx->total.direct_reclaimed;
	unsigned long total_refault_anon = ctx->total.refault_anon;
	unsigned long total_refault_file = ctx->total.refault_file;

	ctx->last.nr_damon_applied = total_nr_damon_applied;
	ctx->last.kswapd_reclaimed = total_kswapd_reclaimed;
	ctx->last.direct_reclaimed = total_direct_reclaimed;
	ctx->last.refault_anon = total_refault_anon;
	ctx->last.refault_file = total_refault_file;

	ctx->state.init = true;

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
int reclaim_min_age_calc(struct mas_calc *ctx)
{
	unsigned long ret, damon_reclaimed, pgsteal, min_age;
	unsigned long next_min_age, nr_inc, nr_dec;

	ret = update_mas(ctx);
	if (ret & MAS_ERR)
		goto err;
	if (ret & MAS_NEED_INIT)
		goto record;

	damon_reclaimed = ctx->metric.nr_damon_applied;
	pgsteal = ctx->metric.kswapd_reclaimed + ctx->metric.direct_reclaimed;

	if (!(ctx->metric.refault_anon + ctx->metric.refault_file))
		goto record;
	if (!pgsteal && !damon_reclaimed)
		goto record;
	if (get_min_age(DAMON_RECLAIM, &min_age))
		goto err;
	if (reclaim_step_calc(ctx, DAMON_RECLAIM))
		goto err;

	nr_inc = ctx->result.nr_inc;
	nr_dec = ctx->result.nr_dec;

	if (!nr_inc && !nr_dec)
		goto record;

	next_min_age = ctx->result.next_min_age;
	if (nr_dec) {
		ret = dec_min_age(DAMON_RECLAIM, nr_dec, &next_min_age);
	} else if (nr_inc) {
		ret = inc_min_age(DAMON_RECLAIM, nr_inc, &next_min_age);
	}
	if (ret) {
		goto err;
	}
	ctx->result.next_min_age = next_min_age;

	fade_mas(ctx);
record:
	record_mas(ctx);
	return 0;
err:
	return -1;
}

int min_age_calc(struct mas_calc *ctx, unsigned int damon_module)
{
	if (damon_module == DAMON_RECLAIM)
		return reclaim_min_age_calc(ctx);
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
	char *module_name = NULL;
	unsigned long next_min_age;
	unsigned long memtotal;
	unsigned long memfree;
	unsigned int damon_module = info->damon_module;
	unsigned long quota_ms = 0;
	unsigned long quota_sz = 0;
	bool in_wmarks = false;
	bool executed_kdamond = false;
	bool stop_kdamond = false;

	if (!info)
		goto done;
	if (module_to_name(damon_module, &module_name))
		goto done;
	if (!is_supported_module(damon_module))
		goto done;
	if (damos_init())
		goto done;
	if (damon_read_quota(module_name, &quota_ms, &quota_sz))
		goto done;

	pr_time("udamond start %s\n", module_name);

	while (true) {
		ticks++;

		if (read_meminfo(&memtotal, &memfree, NULL, NULL))
			goto done;
		if (damon_read_wmarks(module_name, &info->wmarks))
			goto done;
		executed_kdamond =
			(PERCENT(memtotal, info->wmarks.low / 10) <= memfree &&
			 memfree <= PERCENT(memtotal, info->wmarks.mid / 10));

		if (!executed_kdamond && !in_wmarks)
			goto rest;

		in_wmarks = true;
		stop_kdamond =
			PERCENT(memtotal, info->wmarks.high / 10) < memfree ||
			PERCENT(memtotal, info->wmarks.low / 10) > memfree;

		if (stop_kdamond) {
			executed_kdamond = false;
			in_wmarks = false;
			stop_kdamond = false;
			goto rest;
		}

		if (min_age_calc(info->mas_calc, damon_module))
			goto done;

		next_min_age = info->mas_calc->result.next_min_age;
		if (next_min_age)
			write_min_age(damon_module, next_min_age);

rest:
		if (SCHEME_WATERMARKS) {
			if (!in_wmarks) {
				damon_write_quota(module_name, 0, 0);
			} else {
				if (damon_read_quota(module_name, &quota_ms, &quota_sz))
					goto done;
				damon_write_quota(module_name, quota_ms, quota_sz);
			}
		}
		usleep(UDAMOND_SLEEP_US);
	}
done:
	pr_time("udamond stop %s\n", module_name);
	free(info);
	return 0;
}
