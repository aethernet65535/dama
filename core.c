#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sysfs.h"

bool strtobool(const char *str)
{
	if (!str)
		return false;

	return (str[0] == 'Y' || str[0] == 'y' || str[0] == '1');
}

int damon_read_bool(const char *module, const char *param, bool *val)
{
	char path[512];
	int ret, len;
	char c;

	len = snprintf(path, sizeof(path), "/sys/module/%s/parameters/%s",
		       module, param);
	if (len < 0 || len >= (int)sizeof(path)) {
		return -ENAMETOOLONG;
	}

	ret = read_sysfs_char(path, &c);
	if (ret)
		return ret;

	*val = strtobool(&c);

	return 0;
}

int damon_write_bool(const char *module, const char *param, bool val)
{
	char path[512];
	int len;

	len = snprintf(path, sizeof(path), "/sys/module/%s/parameters/%s",
		       module, param);
	if (len < 0 || len >= (int)sizeof(path)) {
		return -ENAMETOOLONG;
	}

	return write_sysfs_bool(path, val);
}

int damon_is_enabled(const char *module, bool *enabled)
{
	int ret;
	bool val;

	ret = damon_read_bool(module, "enabled", &val);
	if (ret)
		return ret;

	*enabled = val ? true : false;
	return 0;
}

int damon_set_enabled(const char *module, bool on)
{
	return damon_write_bool(module, "enabled", on);
}
