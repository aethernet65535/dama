ADMIN=/sys/kernel/mm/damon/admin
STATS=$ADMIN/kdamonds/0/contexts/0/schemes/0/stats

echo update_schemes_stats > $ADMIN/kdamonds/0/state

echo "Regions tried:   $(cat $STATS/nr_tried)"
echo "Bytes tried:     $(cat $STATS/sz_tried)"
echo "Regions applied: $(cat $STATS/nr_applied)"
echo "Bytes applied:   $(cat $STATS/sz_applied)"
echo "Quota exceeded:  $(cat $STATS/qt_exceeds)"
