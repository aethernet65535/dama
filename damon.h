int damon_read_ulong(const char *module_name, const char *param,
		     unsigned long *val);
int damon_read_bool(const char *module_name, const char *param, bool *val);
int damon_write_ulong(const char *module_name, const char *param,
		      unsigned long val);
int damon_write_bool(const char *module_name, const char *param, bool val);
int damon_is_enabled(const char *module_name, bool *enabled);
int damon_set_enabled(const char *module_name, bool on);
int damon_commit_params(const char *module_name);
int read_damon_nr_applied(unsigned int damon_module,
			  unsigned long *damon_nr_applied);
int damon_read_wmarks(char *module_name, struct wmarks *wmarks);
int damon_write_wmarks(char *module_name, struct wmarks wmarks);
int module_to_min_age(unsigned int damon_module, char **min_age);
int get_min_age(unsigned int damon_module, unsigned long *min_age);
int write_min_age(unsigned int damon_module, unsigned long min_age);
int __damon_read_quota(char *module_name, unsigned long *quota_ms,
		       unsigned long *quota_sz);
int damon_read_quota(char *module_name, unsigned long *quota_ms,
		     unsigned long *quota_sz);
int damon_write_quota(char *module_name, unsigned long quota_ms,
		      unsigned long quota_sz);
int damos_init(void);
