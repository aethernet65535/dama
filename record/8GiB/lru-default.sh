#!/bin/bash

# DAMA (DAMon Assistant) Alpha v7 Test
# ====================================
#
# This test is only for auto-tune min_age.

if [ "$(id -u)" -ne 0 ]; then
  echo "please run as root"
  exit 1
fi

pkill -x dama
pkill stress-ng
pkill sar

# DAMON will run when the free is in 40-1%, will stop if it is higher
# than 50%. Just like kswapd will run when free is lower than 'low', and
# stop when free is higher than 'high'.
echo N > /sys/module/damon_lru_sort/parameters/enabled
echo 800 > /sys/module/damon_lru_sort/parameters/wmarks_high
echo 700 > /sys/module/damon_lru_sort/parameters/wmarks_mid
echo 10 > /sys/module/damon_lru_sort/parameters/wmarks_low
echo 120000000 > /sys/module/damon_lru_sort/parameters/cold_min_age

rm -rf ./report/lru-default-2026-05-24-0001/
mkdir -p ./report/lru-default-2026-05-24-0001/

cat /proc/vmstat | rg "refault" >> ./report/lru-default-2026-05-24-0001/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/lru-default-2026-05-24-0001/pgsteal.txt
cat /sys/module/damon_lru_sort/parameters/bytes_lru_sorted_cold_regions >> ./report/lru-default-2026-05-24-0001/damon_passed.txt
sar -r 5 $(((2*60*60)/5)) >> ./report/lru-default-2026-05-24-0001/memu.txt &
sar -q CPU 5 $(((2*60*60)/5)) >> ./report/lru-default-2026-05-24-0001/cpu.txt &
sar -q IO 5 $(((2*60*60)/5)) >> ./report/lru-default-2026-05-24-0001/io.txt &
sar -q MEM 5 $(((2*60*60)/5)) >> ./report/lru-default-2026-05-24-0001/memo.txt &
sar -B 5 $(((2*60*60)/5)) >> ./report/lru-default-2026-05-24-0001/fault.txt &

echo Y > /sys/module/damon_lru_sort/parameters/enabled

# My device memory
# ================
#
# Due to 'stress-ng' '--vm-bytes N%' is % of total available memory
# instead of total memory, I have to noted this :
#
#   Total Memory: 7720 KiB
#
#   Total Block: 20
#   Block Storage: 386 MiB

stress-ng --vm 1 --vm-bytes $((5 * 386))M --vm-hang $((10 * 60)) --vm-keep --timeout=2h &
stress-ng --vm 1 --vm-bytes $((5 * 386))M --vm-hang $((5 * 60)) --vm-keep --timeout=2h &
stress-ng --vm 1 --vm-bytes $((5 * 386))M --vm-hang $((3 * 60)) --vm-keep --timeout=2h &
sleep 1h

stress-ng --vm 1 --vm-bytes $((5 * 386))M --vm-hang 5 --vm-keep --timeout=1h

cat /proc/vmstat | rg "refault" >> ./report/lru-default-2026-05-24-0001/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/lru-default-2026-05-24-0001/pgsteal.txt
cat /sys/module/damon_lru_sort/parameters/bytes_lru_sorted_cold_regions >> ./report/lru-default-2026-05-24-0001/damon_passed.txt
echo N > /sys/module/damon_lru_sort/parameters/enabled
