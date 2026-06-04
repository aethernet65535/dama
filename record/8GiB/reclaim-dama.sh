#!/bin/bash

# DAMA (DAMon Assistant) Alpha v7 Test
# ====================================
#
# This test is only for auto-tune min_age.

DATE="2026-06-04-0001"
TEST_MINS=60
DEFAULT_MIN_AGE_SEC=10

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
echo 120000000 > /sys/module/damon_reclaim/parameters/min_age

rm -rf ./report/rec-dama-$DATE/
mkdir -p ./report/rec-dama-$DATE/

cat /proc/vmstat | rg "refault" >> ./report/rec-dama-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/rec-dama-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/rec-dama-$DATE/damon_passed.txt
sar -r 5 $((($TEST_MINS*60)/5)) >> ./report/rec-dama-$DATE/memu.txt &
sar -q CPU 5 $((($TEST_MINS*60)/5)) >> ./report/rec-dama-$DATE/cpu.txt &
sar -q IO 5 $((($TEST_MINS*60)/5)) >> ./report/rec-dama-$DATE/io.txt &
sar -q MEM 5 $((($TEST_MINS*60)/5)) >> ./report/rec-dama-$DATE/memo.txt &
sar -B 5 $((($TEST_MINS*60)/5)) >> ./report/rec-dama-$DATE/fault.txt &

echo $(($DEFAULT_MIN_AGE_SEC * 1000000)) > /sys/module/damon_reclaim/parameters/min_age
/home/user/workspace/github/dama/dama >> ./report/rec-dama-$DATE/dama.txt &

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

cat /proc/vmstat | rg "refault" >> ./report/rec-dama-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/rec-dama-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/rec-dama-$DATE/damon_passed.txt
echo N > /sys/module/damon_reclaim/parameters/enabled
