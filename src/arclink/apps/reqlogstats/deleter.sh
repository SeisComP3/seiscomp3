#
# Use this to remove all objects for a particular source AND cut off day.
#
#
set -u
start_day="2016-01-24"
src=16

echo 'SELECT * FROM ArcStatsSummary WHERE src >= '$src';'
echo 'SELECT * FROM ArcStatsSource WHERE id >= '$src';'
echo "Are you sure? Sleeping a bit while you think..."
sleep 5

date_constr="start_day = '$start_day'"
src_constr="(src = '$src')"
echo 'DELETE FROM ArcStatsVolume WHERE '$date_constr' AND '$src_constr';'
echo 'DELETE FROM ArcStatsClientIP WHERE '$date_constr' AND '$src_constr';'
echo 'DELETE FROM ArcStatsMessages WHERE '$date_constr' AND '$src_constr';'
echo 'DELETE FROM ArcStatsNetwork WHERE '$date_constr' AND '$src_constr';' 
echo 'DELETE FROM ArcStatsRequest WHERE '$date_constr' AND '$src_constr';' 
echo 'DELETE FROM ArcStatsStation WHERE '$date_constr' AND '$src_constr';'
echo 'DELETE FROM ArcStatsSummary WHERE '$date_constr' AND '$src_constr';'
echo 'DELETE FROM ArcStatsUser WHERE '$date_constr' AND '$src_constr';' 
echo 'DELETE FROM ArcStatsUserIP WHERE '$date_constr' AND '$src_constr';'      
echo 'DELETE FROM ArcStatsVolume WHERE '$date_constr' AND '$src_constr';'
