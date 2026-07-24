#include <stdbool.h>

char *action_to_str(unsigned int action);
int damon_close(void);
int damon_init(void);
int damon_write_state(const char *state);
int damon_write_operation(const char *op);
int damon_write_pid(unsigned long pid);
int damon_write_action(unsigned int action);
int damon_is_enabled(bool *enabled);
int damon_set_enabled(bool on);
int damon_commit_params(void);
int damon_update_stats(void);
int damon_read_wmarks(struct wmarks *wmarks);
int damon_write_wmarks(struct wmarks wmarks);
int damon_read_min_age(unsigned long *min_age);
int damon_write_min_age(unsigned long min_age);
int __damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz);
int damon_read_quota(unsigned long *quota_ms, unsigned long *quota_sz);
int damon_write_quota(unsigned long quota_ms, unsigned long quota_sz);
int damos_init(void);
