/* Unconfigable */
#define MS_US (1000UL)
#define SEC_US (1000UL * MS_US)
#define MIN_US (60U * SEC_US)

#define KiB (1024 * 1)
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

#define REFAULT_HIGH (1 << 0)
#define DAMON_HIGH (1 << 1)
#define PGSTEAL_HIGH (1 << 2)

/* Configable */
#define CONFIG_LOG_ENABLED (1)
#define CONFIG_DEBUG_ENABLED (1)

#define UDAMOND_SLEEP_US (5UL * SEC_US)

#define MIN_MIN_AGE (10UL * SEC_US)
#define MAX_MIN_AGE (10UL * MIN_US)
#define MAX_NO_DECREASE (10)

#define ANON_REFAULT_WEIGHT (3)
#define FILE_REFAULT_WEIGHT (1)

#define INCREASE_THRESHOLD (5)
#define DECREASE_THRESHOLD (10)

#define FADE_FACTOR (95)
#define NOT_WORKING_FACTOR (45)
#define WORKING_FACTOR (90)

#define DAMON_THRESHOLD (16 * KiB)
#define PGSTEAL_THRESHOLD (16 * MiB)
#define TIME_THRESHOLD (5 * MIN_US)
