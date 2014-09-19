#!/bin/bash

# Tests which requre a meaningful database - only run on open1/open2
if [ 1 -eq 1 ] ; then

# make_month_graph

echo "SELECT distinct(networkCode),count(*) FROM ArcStatsNetwork GROUP BY networkCode;" | sqlite3 var/reqlogstats-2014.db

./make_month_graph.sh --code FN   # Probably has nothing, so no output.
./make_month_graph.sh --code GE
./make_month_graph.sh --code G
./make_month_graph.sh --code CH

./make_month_graph.sh 12 2013


./make_month_graph.sh 11 2013 var/reqlogstats-2013.db



fi


if [ 1 -eq 1 ] ; then

	php www/reqlog.php | wc -l  # expect 0
	php www/reqlogdisplay.php | file -  # expect XML
	php www/reqlognetwork.php | file -  # expect XML

fi

