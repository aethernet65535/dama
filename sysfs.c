#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "core.h"
#include "sysfs.h"

int write_sysfs_ulong(const char *path, unsigned long val)
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

int write_sysfs_int(const char *path, int val)
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

int write_sysfs_bool(const char *path, bool val)
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

int read_sysfs_ulong(const char *path, unsigned long *val)
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

int read_sysfs_int(const char *path, int *val)
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

int read_sysfs_char(const char *path, char *val)
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

int read_meminfo(unsigned long *mem_total_kb, unsigned long *inactive_anon_kb,
		 unsigned long *inactive_file_kb)
{
	char line[256];
	FILE *fp = fopen("/proc/meminfo", "r");

	if (!fp) {
		pr_err("open failed\n");
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!strncmp(line, "MemTotal:", 9))
			sscanf(line, "MemTotal: %lu kB", mem_total_kb);
		else if (!strncmp(line, "Inactive(anon):", 14))
			sscanf(line, "Inactive(anon): %lu kB",
			       inactive_anon_kb);
		else if (!strncmp(line, "Inactive(file):", 14))
			sscanf(line, "Inactive(file): %lu kB",
			       inactive_file_kb);
	}
	pr_debug("Meminfo: OK\n");

	fclose(fp);
	return 0;
}

int read_psi_mm(struct psi *psi)
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
			psi->some_avg10 = (unsigned long)(avg10 * 10000);
			psi->some_avg60 = (unsigned long)(avg60 * 10000);
			psi->some_avg300 = (unsigned long)(avg300 * 10000);
			psi->some_total = total;
			pr_debug("PSI some: OK\n");
		} else if (!strncmp(line, "full", 4)) {
			sscanf(line,
			       "full avg10=%lf avg60=%lf avg300=%lf total=%llu",
			       &avg10, &avg60, &avg300, &total);
			psi->full_avg10 = (unsigned long)(avg10 * 10000);
			psi->full_avg60 = (unsigned long)(avg60 * 10000);
			psi->full_avg300 = (unsigned long)(avg300 * 10000);
			psi->full_total = total;
			pr_debug("PSI full: OK\n");
		}
	}

	fclose(fp);
	return 0;
}
