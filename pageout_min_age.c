#include "core.h"
#include "sysfs.h"
#include "damon.h"
#include "util.h"
#include "log.h"

extern unsigned long ticks;

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

	const char *sz_applied = "/sys/kernel/mm/damon/admin/kdamonds/0/"
				 "contexts/0/schemes/0/stats/sz_applied";

	ctx->state.anon_weight = ANON_REFAULT_WEIGHT;
	ctx->state.file_weight = FILE_REFAULT_WEIGHT;

	if (read_refault(&total_refault_anon, &total_refault_file))
		goto err;
	if (read_steal(&total_kswapd_reclaimed, &total_direct_reclaimed))
		goto err;
	if (sysfs_read_ulong(sz_applied, &total_nr_damon_applied))
		goto err;

	if (!ctx->state.init)
		goto update;

	d_kswapd_reclaimed =
		total_kswapd_reclaimed - ctx->last.kswapd_reclaimed;
	d_direct_reclaimed =
		total_direct_reclaimed - ctx->last.direct_reclaimed;
	d_pgsteal = d_kswapd_reclaimed + d_direct_reclaimed;
	d_damon_reclaimed = (total_nr_damon_applied / PAGE_SIZE) -
			    ctx->last.nr_damon_applied;
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

int reclaim_step_calc(struct mas_calc *ctx, int action)
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
	bool fade = false;

	refault_weighted = anon_weighted + file_weighted;

	if (damon_read_min_age(&min_age))
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
			goto done;
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
		fade = true;
	}
done:
	ctx->result.nr_inc = nr_inc;
	ctx->result.nr_dec = nr_dec;
	ctx->result.fade = fade;

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
	bool fade;

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
	if (damon_read_min_age(&min_age))
		goto err;
	if (reclaim_step_calc(ctx, PAGEOUT))
		goto err;

	nr_inc = ctx->result.nr_inc;
	nr_dec = ctx->result.nr_dec;
	fade = ctx->result.fade;

	if (!nr_inc && !nr_dec && !fade)
		goto record;
	if (!nr_inc && !nr_dec)
		goto fade;

	next_min_age = ctx->result.next_min_age;
	if (nr_dec) {
		ret = dec_min_age(nr_dec, &next_min_age);
	} else if (nr_inc) {
		ret = inc_min_age(nr_inc, &next_min_age);
	}
	if (ret) {
		goto err;
	}
	ctx->result.next_min_age = next_min_age;
fade:
	fade_mas(ctx);
record:
	record_mas(ctx);
	return 0;
err:
	return -1;
}

int min_age_calc(struct mas_calc *ctx, unsigned int action)
{
	if (action == PAGEOUT)
		return reclaim_min_age_calc(ctx);
	else if (action == LRU_PRIO)
		return -1;
	else
		return -1;
}
