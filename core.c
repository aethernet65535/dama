#include "core.h"
#include "damon.h"
#include "sysfs.h"
#include "log.h"
#include "util.h"
#include "pageout_min_age.h"

unsigned long ticks = 0;
bool running = false;

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

int inc_min_age(unsigned long step, unsigned long *min_age)
{
	if (!step)
		return -1;
	if (damon_read_min_age(min_age))
		return -1;

	if (*min_age + step < *min_age)
		*min_age = MAX_MIN_AGE;
	else
		*min_age = min(MAX_MIN_AGE, *min_age + step);

	pr_time("[%lu] inc_min_age(): %lu\n", ticks, *min_age);

	return 0;
}

int dec_min_age(unsigned long step, unsigned long *min_age)
{
	if (!step)
		return -1;
	if (damon_read_min_age(min_age))
		return -1;

	/* Underflow */
	if (*min_age - step > *min_age)
		*min_age = MIN_MIN_AGE;
	else
		*min_age = max(MIN_MIN_AGE, *min_age - step);

	pr_time("[%lu] dec_min_age(): %lu\n", ticks, *min_age);

	return 0;
}

/*
 * The Heart of the DAMA.
 */
int udamond_fn(struct damon_info *info)
{
	unsigned long memtotal;
	unsigned long memfree;
	char *action = info->param->action;
	unsigned long quota_ms = 0;
	unsigned long quota_sz = 0;
	bool in_wmarks = false;
	bool executed_kdamond = false;
	bool stop_kdamond = false;

	if (!info)
		goto done;
	if (damon_read_quota(&quota_ms, &quota_sz))
		goto done;

	pr_time("udamond start %s\n", action);

	while (true) {
		ticks++;

		if (!running) {
			damon_close();
			goto done;
		}

		if (read_meminfo(&memtotal, &memfree, NULL, NULL))
			goto done;
		executed_kdamond =
			(PERCENT(memtotal, WMARKS_LOW / 10) <= memfree &&
			 memfree <= PERCENT(memtotal, WMARKS_MID / 10));

		if (!executed_kdamond && !in_wmarks)
			goto rest;

		in_wmarks = true;
		stop_kdamond = PERCENT(memtotal, WMARKS_HIGH / 10) < memfree ||
			       PERCENT(memtotal, WMARKS_LOW / 10) > memfree;

		if (stop_kdamond) {
			executed_kdamond = false;
			in_wmarks = false;
			stop_kdamond = false;
			goto rest;
		}

		if (info->udamond_action((void*)info))
			goto done;

	rest:
		if (DAMOS_ONLY_WATERMARKS) {
			if (!in_wmarks) {
				damon_write_action("stats");
			} else {
				if (damon_read_quota(&quota_ms, &quota_sz))
					goto done;
				damon_write_quota(quota_ms, quota_sz);
			}
		}
		usleep(UDAMOND_SLEEP_US);
	}
done:
	pr_time("udamond stop %s\n", action);
	free(info);
	return 0;
}
