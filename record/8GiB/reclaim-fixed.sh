#!/bin/bash

# DAMA (DAMon Assistant) Alpha v7 Test
# ====================================
#
# This test is only for auto-tune min_age.

pkill -x dama
pkill stress-ng
pkill sar

# DAMON will run when the free is in 40-2%, will stop if it is higher
# than 50%. Just like kswapd will run when free is lower than 'low', and
# stop when free is higher than 'high'.
echo N > /sys/module/damon_reclaim/parameters/enabled
echo 800 > /sys/module/damon_reclaim/parameters/wmarks_high
echo 700 > /sys/module/damon_reclaim/parameters/wmarks_mid
echo 20 > /sys/module/damon_reclaim/parameters/wmarks_low
echo 120000000 > /sys/module/damon_reclaim/parameters/min_age

rm -rf ./report/rec-fixed-2026-05-24-0002/
mkdir -p ./report/rec-fixed-2026-05-24-0002/

cat /proc/vmstat | rg "refault" >> ./report/rec-fixed-2026-05-24-0002/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/rec-fixed-2026-05-24-0002/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/rec-fixed-2026-05-24-0002/damon_passed.txt
sar -r 5 $(((2*60*60)/5)) >> ./report/rec-fixed-2026-05-24-0002/memu.txt &
sar -q CPU 5 $(((2*60*60)/5)) >> ./report/rec-fixed-2026-05-24-0002/cpu.txt &
sar -q IO 5 $(((2*60*60)/5)) >> ./report/rec-fixed-2026-05-24-0002/io.txt &
sar -q MEM 5 $(((2*60*60)/5)) >> ./report/rec-fixed-2026-05-24-0002/memo.txt &
sar -B 5 $(((2*60*60)/5)) >> ./report/rec-fixed-2026-05-24-0002/fault.txt &

# Comment to disable:
# [DAMA]
echo $((10 * 1000000)) > /sys/module/damon_reclaim/parameters/min_age

echo Y > /sys/module/damon_reclaim/parameters/enabled

# My device memory
# ================
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

cat /proc/vmstat | rg "refault" >> ./report/rec-fixed-2026-05-24-0002/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/rec-fixed-2026-05-24-0002/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/rec-fixed-2026-05-24-0002/damon_passed.txt
echo N > /sys/module/damon_reclaim/parameters/enabled
