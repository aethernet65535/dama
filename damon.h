char *action_to_str(unsigned int action);
int damon_init(void);
int write_sample_us(unsigned long sample);
int write_aggr_us(unsigned long aggr);
int write_intervals(unsigned long sample, unsigned long aggr);
int write_operation(const char *op);
int damon_write_action(unsigned int action);
int write_ds_pid(unsigned long pid);
int write_ds_state(const char *state);
int damon_is_enabled(bool *enabled);
int damon_set_enabled(bool on);
int damon_commit_params(void);
int read_damon_sz_applied(unsigned long *damon_sz_applied);
int damon_read_wmarks(struct wmarks *wmarks);
int damon_write_wmarks(struct wmarks wmarks);
int get_min_age(unsigned long *min_age);
int write_min_age(unsigned long min_age);
int __damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz);
int damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz);
int damon_write_quota(unsigned long quota_ms, unsigned long quota_sz);
int damos_init(void);

/*
 * NOTE: Tree Of DAMON_SYSFS
 * =========================
 *
 * /sys/kernel/mm/damon/admin/
 *└── kdamonds/
 *    ├── nr_kdamonds                        # write N to create N kdamond directories
 *    └── 0/
 *        ├── state                          # write: on|off|commit|update_schemes_stats|...
 *        ├── pid                            # read: PID of the kdamond thread
 *        ├── refresh_ms                     # auto-refresh interval for sysfs reads
 *        └── contexts/
 *            ├── nr_contexts                # currently only 1 context per kdamond
 *            └── 0/
 *                ├── avail_operations       # read: available ops (vaddr fvaddr paddr)
 *                ├── operations             # write: vaddr | fvaddr | paddr
 *                ├── addr_unit              # scale factor for address values
 *                ├── monitoring_attrs/
 *                │   ├── intervals/
 *                │   │   ├── sample_us      # sampling interval in µs (default 5000)
 *                │   │   ├── aggr_us        # aggregation interval in µs (default 100000)
 *                │   │   ├── update_us      # ops update interval in µs (default 60000000)
 *                │   │   └── intervals_goal/
 *                │   │       ├── access_bp
 *                │   │       ├── aggrs
 *                │   │       ├── min_sample_us
 *                │   │       └── max_sample_us
 *                │   └── nr_regions/
 *                │       ├── min            # minimum number of regions (default 10)
 *                │       └── max            # maximum number of regions (default 1000)
 *                ├── targets/
 *                │   ├── nr_targets
 *                │   └── 0/
 *                │       ├── pid_target     # PID to monitor (vaddr/fvaddr only)
 *                │       ├── obsolete_target
 *                │       └── regions/
 *                │           ├── nr_regions # for fvaddr: number of fixed regions
 *                │           └── 0/
 *                │               ├── start  # region start address
 *                │               └── end    # region end address
 *                └── schemes/
 *                    ├── nr_schemes
 *                    └── 0/
 *                        ├── action         # willneed|cold|pageout|hugepage|nohugepage|
 *                        │                  # lru_prio|lru_deprio|migrate_hot|migrate_cold|stat
 *                        ├── target_nid     # destination NUMA node for migrate actions
 *                        ├── apply_interval_us  # override aggr_interval for this scheme
 *                        ├── access_pattern/
 *                        │   ├── sz/
 *                        │   │   ├── min
 *                        │   │   └── max
 *                        │   ├── nr_accesses/
 *                        │   │   ├── min
 *                        │   │   └── max
 *                        │   └── age/
 *                        │       ├── min
 *                        │       └── max
 *                        ├── quotas/
 *                        │   ├── ms
 *                        │   ├── bytes
 *                        │   ├── reset_interval_ms
 *                        │   ├── effective_bytes    # read-only: current effective quota
 *                        │   ├── weights/
 *                        │   │   ├── sz_permil
 *                        │   │   ├── nr_accesses_permil
 *                        │   │   └── age_permil
 *                        │   └── goals/
 *                        │       ├── nr_goals
 *                        │       └── 0/
 *                        │           ├── target_metric
 *                        │           ├── target_value
 *                        │           ├── current_value
 *                        │           ├── nid
 *                        │           └── path
 *                        ├── watermarks/
 *                        │   ├── metric         # none | free_mem_rate
 *                        │   ├── interval_us
 *                        │   ├── high
 *                        │   ├── mid
 *                        │   └── low
 *                        ├── filters/           # core + ops filters combined
 *                        │   ├── nr_filters
 *                        │   └── 0/
 *                        │       ├── type        # anon|active|memcg|young|hugepage_size|
 *                        │       │               # unmapped|addr|target
 *                        │       ├── matching
 *                        │       ├── allow
 *                        │       ├── memcg_path
 *                        │       ├── addr_start
 *                        │       ├── addr_end
 *                        │       ├── min         # for hugepage_size filter
 *                        │       ├── max
 *                        │       └── damon_target_idx
 *                        ├── dests/             # for migrate_hot/cold actions
 *                        │   ├── nr_dests
 *                        │   └── 0/
 *                        │       ├── id         # destination NUMA node id
 *                        │       └── weight
 *                        ├── stats/             # read-only counters
 *                        │   ├── nr_tried
 *                        │   ├── sz_tried
 *                        │   ├── nr_applied
 *                        │   ├── sz_applied
 *                        │   ├── sz_ops_filter_passed
 *                        │   ├── qt_exceeds
 *                        │   ├── nr_snapshots
 *                        │   └── max_nr_snapshots
 *                        └── tried_regions/     # regions that matched this scheme
 *                            ├── total_bytes
 *                            └── 0/
 *                                ├── start
 *                                ├── end
 *                                ├── nr_accesses
 *                                ├── age
 *                                └── sz_filter_passed
 */

struct min_max {
	int (*write_min)(unsigned long);
	int (*read_min)(unsigned long*);

	int (*write_max)(unsigned long);
	int (*read_max)(unsigned long*);
};

struct access_pattern {
	struct min_max sz;
	struct min_max nr_accesses;
	struct min_max age;
};

struct scheme {
	int (*write_action)(const char*);
	int (*read_action)(const char**);

	int (*write_target_nid)(unsigned long);
	int (*read_target_nid)(unsigned long*);

	int (*write_apply_interval_us)(unsigned long);
	int (*read_apply_interval_us)(unsigned long*);

	struct access_pattern access_pattern;
};

struct schemes {
	int (*write_nr_schemes)(unsigned long);
	int (*read_nr_schemes)(unsigned long*);

	struct scheme scheme[];
};

struct region {
	int (*write_start)(unsigned long);
	int (*read_start)(unsigned long*);

	int (*write_end)(unsigned long);
	int (*read_end)(unsigned long*);
};

struct regions {
	int (*write_nr_regions)(unsigned long);
	int (*read_nr_regions)(unsigned long*);

	struct region region[];
};

struct target {
	int (*write_pid_target)(unsigned long);
	int (*read_pid_target)(unsigned long*);

	int (*write_obsolete_target)(unsigned long);
	int (*read_obsolete_target)(unsigned long*);

	struct regions *regions;
};

struct targets {
	int (*write_nr_targets)(unsigned long);
	int (*read_nr_targets)(unsigned long*);

	struct target target[];
};

struct intervals_goal {
	int (*write_access_bp)(unsigned long);
	int (*read_access_bp)(unsigned long*);

	int (*write_aggrs)(unsigned long);
	int (*read_aggrs)(unsigned long*);

	int (*write_min_sample_us)(unsigned long);
	int (*read_min_simple_us)(unsigned long*);

	int (*write_max_sample_us)(unsigned long);
	int (*read_max_simple_us)(unsigned long*);
};

struct intervals {
	int (*write_sample_us)(unsigned long);
	int (*read_sample_us)(unsigned long*);

	int (*write_aggr_us)(unsigned long);
	int (*read_aggr_us)(unsigned long*);

	int (*write_update_us)(unsigned long);
	int (*read_update_us)(unsigned long*);

	struct intervals_goal *intervals_goal;
};

struct monitoring_attrs {
	struct intervals *intervals;
	struct min_max *nr_regions;
};

struct context {
	int (*read_avail_operation)(const char**);

	int (*write_operations)(const char*);
	int (*read_operations)(const char**);

	int (*write_addr_unit)(unsigned long);
	int (*read_addr_unit)(unsigned long*);

	struct monitoring_attrs *monitoring_attrs;
	struct targets *targets;
	struct schemes *schemes;
};

struct contexts {
	int (*write_nr_contexts)(unsigned long);
	int (*read_nr_contexts)(unsigned long*);
	
	struct context context[];
};

struct kdamond {
	int (*write_state)(const char*);
	int (*read_state)(const char**);

	int (*write_pid)(unsigned long);
	int (*read_pid)(unsigned long*);

	int (*write_refresh_ms)(unsigned long);
	int (*read_refresh_ms)(unsigned long*);

	struct contexts *context;
};

struct kdamonds {
	unsigned long nr_kdamonds;
	struct kdamond kdamond[];
};
