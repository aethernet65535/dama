#include "core.h"
#include "sysfs.h"
#include "log.h"

int sysfs_write_ulong(const char *path, unsigned long val)
{
	FILE *fp = fopen(path, "w");
	int ret;

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	ret = fprintf(fp, "%lu", val);
	if (ret < 0) {
		pr_err("write failed\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int sysfs_write_int(const char *path, int val)
{
	FILE *fp = fopen(path, "w");
	int ret;

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	ret = fprintf(fp, "%d", val);
	if (ret < 0) {
		pr_err("write failed\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int sysfs_write_bool(const char *path, bool val)
{
	FILE *fp = fopen(path, "w");
	int ret;

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	ret = fprintf(fp, "%d", val);
	if (ret < 0) {
		pr_err("write failed\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int sysfs_write_str(const char *path, const char *val)
{
	FILE *fp = fopen(path, "w");
	int ret;

	if (!val) {
		pr_err("not valid string");
		return -1;
	}

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	ret = fputs(val, fp);
	if (ret < 0) {
		pr_err("write failed\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);

	return 0;
}

int sysfs_read_ulong(const char *path, unsigned long *val)
{
	FILE *fp = fopen(path, "r");
	int ret;

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	ret = fscanf(fp, "%lu", val);
	if (ret != 1) {
		pr_err("read failed\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int sysfs_read_int(const char *path, int *val)
{
	FILE *fp = fopen(path, "r");
	int ret;

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	ret = fscanf(fp, "%d", val);
	if (ret != 1) {
		pr_err("read failed\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

int sysfs_read_char(const char *path, char *val)
{
	FILE *fp = fopen(path, "r");
	int c;

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	c = fgetc(fp);
	if (c == EOF) {
		pr_err("read failed\n");
		fclose(fp);
		return -1;
	}

	*val = (char)c;

	fclose(fp);
	return 0;
}

int sysfs_read_str(const char *path, char **val, size_t max_len)
{
	FILE *fp = fopen(path, "r");
	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	if (!fgets(*val, max_len, fp)) {
		pr_err("read failed\n");
		fclose(fp);
		return -1;
	}

	size_t len = strlen(*val);
	if (len > 0 && (*val)[len - 1] == '\n') {
		(*val)[len - 1] = '\0';
	}

	fclose(fp);
	return 0;
}

/* Read meminfo in KiB. */
int read_meminfo(unsigned long *memtotal, unsigned long *memfree,
		 unsigned long *inactive_anon, unsigned long *inactive_file)
{
	char line[256];
	FILE *fp = fopen("/proc/meminfo", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "MemTotal:", 9) && memtotal)
			sscanf(line, "MemTotal: %lu kB", memtotal);
		else if (!strncmp(line, "MemFree:", 8) && memfree)
			sscanf(line, "MemFree: %lu kB", memfree);
		else if (!strncmp(line, "Inactive(anon):", 15) && inactive_anon)
			sscanf(line, "Inactive(anon): %lu kB", inactive_anon);
		else if (!strncmp(line, "Inactive(file):", 15) && inactive_file)
			sscanf(line, "Inactive(file): %lu kB", inactive_file);
	}

	fclose(fp);
	return 0;
}

int read_psi_cpu(struct psi *psi)
{
	double avg10, avg60, avg300;
	unsigned long long total;
	char line[256];
	FILE *fp = fopen("/proc/pressure/cpu", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "some", 4)) {
			sscanf(line,
			       "some avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->some_avg10 = (unsigned long)(avg10 * 100);
			psi->some_avg60 = (unsigned long)(avg60 * 100);
			psi->some_avg300 = (unsigned long)(avg300 * 100);
			psi->some_total = total;
		} else if (!strncmp(line, "full", 4)) {
			sscanf(line,
			       "full avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->full_avg10 = (unsigned long)(avg10 * 100);
			psi->full_avg60 = (unsigned long)(avg60 * 100);
			psi->full_avg300 = (unsigned long)(avg300 * 100);
			psi->full_total = total;
		}
	}

	fclose(fp);
	return 0;
}

int read_psi_io(struct psi *psi)
{
	double avg10, avg60, avg300;
	unsigned long long total;
	char line[256];
	FILE *fp = fopen("/proc/pressure/io", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "some", 4)) {
			sscanf(line,
			       "some avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->some_avg10 = (unsigned long)(avg10 * 100);
			psi->some_avg60 = (unsigned long)(avg60 * 100);
			psi->some_avg300 = (unsigned long)(avg300 * 100);
			psi->some_total = total;
		} else if (!strncmp(line, "full", 4)) {
			sscanf(line,
			       "full avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->full_avg10 = (unsigned long)(avg10 * 100);
			psi->full_avg60 = (unsigned long)(avg60 * 100);
			psi->full_avg300 = (unsigned long)(avg300 * 100);
			psi->full_total = total;
		}
	}

	fclose(fp);
	return 0;
}

int read_psi_mem(struct psi *psi)
{
	double avg10, avg60, avg300;
	unsigned long long total;
	char line[256];
	FILE *fp = fopen("/proc/pressure/memory", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "some", 4)) {
			sscanf(line,
			       "some avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->some_avg10 = (unsigned long)(avg10 * 100);
			psi->some_avg60 = (unsigned long)(avg60 * 100);
			psi->some_avg300 = (unsigned long)(avg300 * 100);
			psi->some_total = total;
		} else if (!strncmp(line, "full", 4)) {
			sscanf(line,
			       "full avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->full_avg10 = (unsigned long)(avg10 * 100);
			psi->full_avg60 = (unsigned long)(avg60 * 100);
			psi->full_avg300 = (unsigned long)(avg300 * 100);
			psi->full_total = total;
		}
	}

	fclose(fp);
	return 0;
}

int read_refault(unsigned long *anon, unsigned long *file)
{
	char line[256];
	FILE *fp = fopen("/proc/vmstat", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "workingset_refault_anon", 23))
			sscanf(line, "workingset_refault_anon %lu", anon);
		if (!strncmp(line, "workingset_refault_file", 23))
			sscanf(line, "workingset_refault_file %lu", file);
	}

	fclose(fp);
	return 0;
}

int read_steal(unsigned long *kswapd, unsigned long *direct)
{
	char line[256];
	FILE *fp = fopen("/proc/vmstat", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "pgsteal_kswapd", 14))
			sscanf(line, "pgsteal_kswapd %lu\n", kswapd);
		else if (!strncmp(line, "pgsteal_direct", 14))
			sscanf(line, "pgsteal_direct %lu\n", direct);
	}

	fclose(fp);
	return 0;
}
