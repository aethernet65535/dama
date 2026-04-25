#include <stdbool.h>
#include "log.h"
#include "sysfs.h"
#include "core.h"
#include "config.h"
#include "test.h"

int main(void)
{
	test_all();
	return 0;
}
