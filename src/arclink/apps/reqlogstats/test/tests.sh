#!/bin/bash

# Tests which requre a meaningful database - small.db is derived from
# the active db on geofon-open2, with data for October-December 2014.

if [ 1 -eq 1 ] ; then

# make_month_graph

echo "SELECT distinct(networkCode),count(*) FROM ArcStatsNetwork GROUP BY networkCode;" | sqlite3 small.db

../make_month_graph.sh --code FN  8 2014 small.db  # No content in the db, so no output.
../make_month_graph.sh --code GE 12 2014 small.db
../make_month_graph.sh --code G  11 2014 small.db
../make_month_graph.sh --code CH 11 2014 small.db  # Present at two nodes, ODC and ETH

# Test with default database file: no file so no output.
../make_month_graph.sh 12 2014


../make_month_graph.sh 11 2014 small.db


fi


# make_year_graph

../make_year_graph.sh


# web content

if [ 1 -eq 1 ] ; then

	php ../www/reqlog.php | wc -l  # expect 0
	php ../www/reqlogdisplay.php | file -  # expect XML
	php ../www/reqlognetwork.php | file -  # expect XML

fi

