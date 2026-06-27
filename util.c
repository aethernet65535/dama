#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include "core.h"

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

void *zalloc(size_t size)
{
	return calloc(1, size);
}

void pct(unsigned long *val, unsigned long percentage)
{
	*val = PERCENT(*val, percentage);
}
