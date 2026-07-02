#!/bin/bash

# DAMA (DAMon Assistant) Alpha Test
# =================================

MIN_SEC=60

DIR_NAME="rec-default"
DATE="2026-07-02-0001"
TEST_SECS=$((720))
TOTAL_MEM=$((15991316))
INTERVAL_SECS=$((5))
SAMPLING_TIMES=$(($TEST_SECS/$INTERVAL_SECS))

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

pushd /home/user/workspace/dcperf/
sudo ./benchpress_cli.py run tao_bench_standalone -i '{"num_servers": 1, "memsize": 16, "num_clients": 2, "warmup_time": 160, "test_time": 300\}'
popd

cat /proc/vmstat | rg "refault" >> ./report/$DIR_NAME\-$DATE/refault.txt
cat /proc/vmstat | rg "pgsteal" >> ./report/$DIR_NAME\-$DATE/pgsteal.txt
cat /sys/module/damon_reclaim/parameters/bytes_reclaimed_regions >> ./report/$DIR_NAME\-$DATE/damon_passed.txt
echo N > /sys/module/damon_reclaim/parameters/enabled
