/* - Unconfigable - */

#define MS_US (1000UL)
#define SEC_US (1000UL * MS_US)
#define MIN_US (60U * SEC_US)

#define SEC_MS (1000UL)
#define MIN_MS (60 * SEC_MS)

#define KiB (1024 * 1)
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

#define DAMON_RECLAIM (0)
#define DAMON_LRU_SORT (1)

/* - End - */
/* --------------------------------------- */
/* - Configable - but better don't touch - */

#define FADE_FACTOR (95)
#define NOT_WORKING_FACTOR (45)
#define WORKING_FACTOR (90)

/* - End - */
/* --------------------------------------- */
/* - Configable - */

#define CONFIG_LOG_ENABLED (1)
#define CONFIG_DEBUG_ENABLED (1)
#define SCHEME_WATERMARKS (1)

/* -- Sysfs Param -- */

#define DAMON_MODULE (DAMON_RECLAIM)

#define WMARKS_HIGH (500)
#define WMARKS_MID (400)
#define WMARKS_LOW (0)

#define MIN_AGE (10UL * MIN_US)

#define QUOTA_MS (10)
#define QUOTA_SZ (128 * MiB)

/* -- End -- */

#define UDAMOND_SLEEP_US (5UL * MS_US)

#define MIN_MIN_AGE (10UL * MIN_US)
#define MAX_MIN_AGE (60UL * MIN_US)
#define MAX_NO_DECREASE (10)

#define ANON_REFAULT_WEIGHT (3)
#define FILE_REFAULT_WEIGHT (1)

#define INCREASE_THRESHOLD (5)
#define DECREASE_THRESHOLD (10)

#define DAMON_THRESHOLD (16 * KiB)
#define PGSTEAL_THRESHOLD (16 * MiB)

/* - End - */
/* --------------------------------------- */
