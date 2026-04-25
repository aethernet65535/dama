#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "sysfs.h"
#include "core.h"
#include "test.h"

#ifdef CONFIG_UNIT_TEST
void test_meminfo(void)
{
	unsigned long total_mem, inactive_anon, inactive_file;

	read_meminfo(&total_mem, &inactive_anon, &inactive_file);

	pr_debug("Total Memory: %lu KiB\n", total_mem);
	pr_debug("Inactive Anon Memory: %lu KiB\n", inactive_anon);
	pr_debug("Inactive File Memory: %lu KiB\n", inactive_file);

	pr_debug("Meminfo: OK\n");
}

void test_psi(void)
{
	struct psi *psi = malloc(sizeof(struct psi));

	if (!psi) {
		pr_err("PSI: NOMEM failed\n");
		return;
	}

	read_psi_mm(psi);

	pr_debug("some avg10=%lu\n", psi->some_avg10);
	pr_debug("some avg60=%lu\n", psi->some_avg60);
	pr_debug("some avg300=%lu\n", psi->some_avg300);
	pr_debug("some total=%llu\n", psi->some_total);
	pr_debug("full avg10=%lu\n", psi->full_avg10);
	pr_debug("full avg60=%lu\n", psi->full_avg60);
	pr_debug("full avg300=%lu\n", psi->full_avg300);
	pr_debug("full total=%llu\n", psi->full_total);

	pr_debug("PSI: OK\n");
}

void test_damon_rw(void)
{
	bool enabled = false;

	damon_set_enabled("damon_reclaim", true);
	damon_is_enabled("damon_reclaim", &enabled);
	if (!enabled)
		goto err;

	damon_set_enabled("damon_reclaim", false);
	damon_is_enabled("damon_reclaim", &enabled);
	if (enabled)
		goto err;

	pr_debug("DAMON read/write: OK\n");
	return;
err:
	pr_err("DAMON read/write: failed\n");
}

void test_all(void)
{
	test_damon_rw();
	test_meminfo();
	test_psi();
}
#else
void test_all(void)
{
	return;
}
#endif
