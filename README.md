# dama
dama has two names, the more formal one is DAMON Assistant; The less
formal one is "大吗？" (dà ma, meaning "is is big?").

# What dama do?
The intention of dama is to make DAMON easier to use, by adjusting all
parameters before starting the DAMON module (e.g., DAMON_RECLAIM,
DAMON_LRU_SORT).

## Auto-tune temperature ([cold_]min_age)
There's many execellent auto-tunning machanism in DAMON core. However,
most of them only control the quota, rather than more accurately
determining hot and cold memory.

I want to use this repository to achieve more precise dynamic
determination of cold memory. This is just a user program, not a
modification to the DAMON core.

### Others
This won't be a very big tool, just one that I personally use. :>
