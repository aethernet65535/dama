#include <stdio.h>
#include "config.h"

#if CONFIG_LOG_ENABLED
#define pr_info(fmt, ...) printf("[INFO]" fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) printf("[ERROR]" fmt, ##__VA_ARGS__)
#else
#define pr_info(fmt, ...) ((void)0)
#define pr_err(fmt, ...) ((void)0)
#endif

#if CONFIG_DEBUG_ENABLED
#define pr_debug(fmt, ...) printf("[DEBUG]" fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) ((void)0)
#endif
