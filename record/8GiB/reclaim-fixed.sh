#!/bin/bash

# DAMA (DAMon Assistant) Alpha v7 Test
# ====================================
#
# This test is only for auto-tune min_age.

DATE="2026-06-04-0001"
TEST_MINS=60

if [ "$(id -u)" -ne 0 ]; then
  echo "please run as root"
  exit 1
fi

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
echo $((10 * 1000000)) > /sys/module/damon_reclaim/parameters/min_age

rm -rf ./report/rec-fixed-$DATE/
mkdir -p ./report/rec-fixed-$DATE/

cat /proc/vmstat | rg "refault" >> ./report/rec-fixed-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/rec-fixed-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/rec-fixed-$DATE/damon_passed.txt
sar -r 5 $((($TEST_MINS*60)/5)) >> ./report/rec-fixed-$DATE/memu.txt &
sar -q CPU 5 $((($TEST_MINS*60)/5)) >> ./report/rec-fixed-$DATE/cpu.txt &
sar -q IO 5 $((($TEST_MINS*60)/5)) >> ./report/rec-fixed-$DATE/io.txt &
sar -q MEM 5 $((($TEST_MINS*60)/5)) >> ./report/rec-fixed-$DATE/memo.txt &
sar -B 5 $((($TEST_MINS*60)/5)) >> ./report/rec-fixed-$DATE/fault.txt &

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

/home/user/workspace/github/masim/masim /home/user/workspace/github/dama/record/8GiB/masim/foobar_full.cfg

cat /proc/vmstat | rg "refault" >> ./report/rec-fixed-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/rec-fixed-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/rec-fixed-$DATE/damon_passed.txt
echo N > /sys/module/damon_reclaim/parameters/enabled
