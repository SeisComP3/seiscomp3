set -u
start_day="2016-01-24"
src=15
echo 'DELETE FROM ArcStatsVolume where start_day >= "'$start_day'" and src >= '$src';'
echo 'DELETE FROM ArcStatsClientIP where start_day >= "'$start_day'" and src >= '$src';'
echo 'DELETE FROM ArcStatsMessages where start_day >= "'$start_day'" and src >= '$src';'                 
echo 'DELETE FROM ArcStatsNetwork where start_day >= "'$start_day'" and src >= '$src';' 
echo 'DELETE FROM ArcStatsRequest where start_day >= "'$start_day'" and src >= '$src';' 
echo 'DELETE FROM ArcStatsStation where start_day >= "'$start_day'" and src >= '$src';'                  
echo 'DELETE FROM ArcStatsSummary where start_day >= "'$start_day'" and src >= '$src';'                   
echo 'DELETE FROM ArcStatsUser where start_day >= "'$start_day'" and src >= '$src';' 
echo 'DELETE FROM ArcStatsUserIP where start_day >= "'$start_day'" and src >= '$src';'      
echo 'DELETE FROM ArcStatsVolume where start_day >= "'$start_day'" and src >= '$src';'
