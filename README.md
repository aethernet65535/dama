# dama [alpha]
dama has two names, the more formal one is DAMon Autotune; The less
formal one is "大吗？" (dà ma, meaning "is it big?").

# What dama do?
DAMA's only current feature is 'min_age' auto-tune, and it is only
intended to serve DAMON_RECLAIM and DAMON_LRU_SORT.

## Result
```txt
# DAMON_RECLAIM
# Note that DAMON is nr_regions.
|---------------------------------------------|
|           | DEFAULT   | DAMA      | CHANGES |
|---------------------------------------------|
| RECLAIMED | --------- | --------- | ------- |
| DAMON     | 271       | 433       | +37.4%  |
| KSWAPD    | 3 426 476 | 1 069 295 | -68.8%  |
| DIRECT    | 212 735   | 127 887   | -39.9%  |
| PSI       | --------- | --------- | ------- |
| CPU       | 0.16      | 0.16      | +/- 0%  |
| I/O       | 0.09      | 0.09      | +/- 0%  |
| MEM       | 0.14      | 0.05      | -64.3%  |
| REFAULT   | --------- | --------- | ------- |
| ANON      | 3 238 857 | 1 213 354 | -62.5%  |
| FAULT     | --------- | --------- | ------- |
| PGFAULT   | 649.04    | 349.64    | -46.1%  |
| MAJFAULT  | 342.01    | 46.15     | -81.5%  |
|---------------------------------------------|
```
