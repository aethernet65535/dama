#!/bin/bash

# DAMA (DAMon Assistant) Alpha Test
# =================================

MIN_SEC=60

DIR_NAME="rec-system"
DATE="2026-07-02-0001"
TEST_SECS=$((720))
TOTAL_MEM=$((15991316))
INTERVAL_SECS=$((5))
SAMPLING_TIMES=$((($TEST_MINS*MIN_SEC)/INTERVAL_SECS))

if [ "$(id -u)" -ne 0 ]; then
  echo "please run as root"
  exit 1
fi

pkill -x dama
pkill stress-ng
pkill sar

echo N > /sys/module/damon_reclaim/parameters/enabled

rm -rf ./report/$DIR_NAME\-$DATE/
mkdir -p ./report/$DIR_NAME\-$DATE/

cat /proc/vmstat | rg "refault" >> ./report/$DIR_NAME\-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/$DIR_NAME\-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/$DIR_NAME\-$DATE/damon_passed.txt
sar -r $INTERVAL_SECS $SAMPLING_TIMES >> ./report/$DIR_NAME\-$DATE/memu.txt &
sar -q CPU $INTERVAL_SECS $SAMPLING_TIMES >> ./report/$DIR_NAME\-$DATE/cpu.txt &
sar -q IO $INTERVAL_SECS $SAMPLING_TIMES >> ./report/$DIR_NAME\-$DATE/io.txt &
sar -q MEM $INTERVAL_SECS $SAMPLING_TIMES >> ./report/$DIR_NAME\-$DATE/memo.txt &
sar -B $INTERVAL_SECS $SAMPLING_TIMES >> ./report/$DIR_NAME\-$DATE/fault.txt &

/home/user/workspace/dcperf/benchmark_cli.py run tao_bench_standalone

cat /proc/vmstat | rg "refault" >> ./report/$DIR_NAME\-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/$DIR_NAME\-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/$DIR_NAME\-$DATE/damon_passed.txt
