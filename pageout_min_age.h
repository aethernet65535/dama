unsigned long update_mas(struct mas_calc *ctx);
int reclaim_step_calc(struct mas_calc *ctx);
int fade_mas(struct mas_calc *ctx);
int record_mas(struct mas_calc *ctx);
int reclaim_min_age_calc(struct mas_calc *ctx);
int min_age_calc(struct mas_calc *ctx, unsigned int action);
int pageout_min_age_autotune(void *ctx);
