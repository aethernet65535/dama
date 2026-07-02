# dama [alpha]
dama has two names, the more formal one is DAMos Autotune; The less
formal one is "大吗？" (dà ma, meaning "is it big?").

# What dama do?
DAMA's only current feature is 'min_age' auto-tune, and it is only
intended to serve DAMON_RECLAIM.

## Result
```txt
# DAMON RECLAIM
2026-06-04 Thu --- 2026-06-05 Fri

- Default: 120s (Fixed)
- Custom: 60s (Fixed)
- Dama: 10s (Initial)
- System: 0s (Kswapd and Direct reclaim only)

Note that the unit of RECLAIMED is _pages_.
|-------------------------------------------------------------|
|           | DEFAULT   | CUSTOM      | DAMA      | SYSTEM    |
|-------------------------------------------------------------|
| RECLAIMED | --------- | ----------- | --------- | --------- |
| DAMON     | 0         | 27 648      | 669 274   | 0         |
| KSWAPD    | 4 876 306 | 5 259 842   | 1 817 669 | 5 341 815 |
| DIRECT    | 12 670    | 19 697      | 1 479     | 20 114    |
| PSI       | --------- | ----------- | --------- | --------- |
| CPU       | 0.06      | 0.06        | 0.06      | 0.06      |
| I/O       | 0.00      | 0.00        | 0.00      | 0.00      |
| MEM       | 0.22      | 0.23        | 0.08      | 0.23      |
| REFAULT   | --------- | ----------- | --------- | --------- |
| ANON      | 4 462 273 | 4 858 679   | 2 015 817 | 4 915 695 |
| FAULT     | --------- | ----------- | --------- | --------- |
| PGFAULT   | 1 317.48  | 1 428.33    | 575.10    | 1 443.96  |
| MAJFAULT  | 1 239.70  | 1 349.85    | 560.25    | 1 365.70  |
|-------------------------------------------------------------|
```
```

# Build
```sh
$ make clean
$ make
```
```
```
