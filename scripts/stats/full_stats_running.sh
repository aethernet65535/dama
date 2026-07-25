#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo "please run as root"
  exit 1
fi

REFAULT_ANON=$(awk '/workingset_refault_anon/ {print $2}' /proc/vmstat)
REFAULT_FILE=$(awk '/workingset_refault_file/ {print $2}' /proc/vmstat)

STEAL_KSWAPD=$(awk '/pgsteal_kswapd/ {print $2}' /proc/vmstat)
STEAL_DIRECT=$(awk '/pgsteal_direct/ {print $2}' /proc/vmstat)

ADMIN=/sys/kernel/mm/damon/admin
STATS=$ADMIN/kdamonds/0/contexts/0/schemes/0/stats

echo "--- REFAULT ---"
echo "REFAULT_ANON $REFAULT_ANON"
echo "REFAULT_FILE $REFAULT_FILE"
echo ""

echo "--- PGSTEAL ---"
echo "STEAL_KSWAPD $STEAL_KSWAPD"
echo "STEAL_DIRECT $STEAL_DIRECT"
echo ""

echo "--- DAMON ---"

echo "NR_TRIED          $(cat $STATS/nr_tried)"
echo "BYTES_TRIED       $(cat $STATS/sz_tried)"
echo "REGIONS_APPLIED   $(cat $STATS/nr_applied)"
echo "BYTES_APPLIED     $(cat $STATS/sz_applied)"
echo "QUOTA_EXCEEDS     $(cat $STATS/qt_exceeds)"
echo ""
