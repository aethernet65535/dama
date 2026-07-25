ADMIN=/sys/kernel/mm/damon/admin
STATS=$ADMIN/kdamonds/0/contexts/0/schemes/0/stats

echo update_schemes_stats > $ADMIN/kdamonds/0/state

echo "NR_TRIED          $(cat $STATS/nr_tried)"
echo "BYTES_TRIED       $(cat $STATS/sz_tried)"
echo "REGIONS_APPLIED   $(cat $STATS/nr_applied)"
echo "BYTES_APPLIED     $(cat $STATS/sz_applied)"
echo "QUOTA_EXCEEDS     $(cat $STATS/qt_exceeds)"
