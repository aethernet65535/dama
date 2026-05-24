#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
  echo "please run as root"
  exit 1
fi

# DAMON_RECLAIM
echo 400 > /sys/module/damon_reclaim/parameters/wmarks_high
echo 300 > /sys/module/damon_reclaim/parameters/wmarks_mid
echo 100 > /sys/module/damon_reclaim/parameters/wmarks_low

./dama
