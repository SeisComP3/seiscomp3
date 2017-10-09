#!/bin/sh
#
# Prepare summary graphs for all networks.
#
# Begun by Peter L. Evans, 2014
# <pevans@gfz-potsdam.de>
#
# ----------------------------------------------------------------------

set -u

year=$(date +%Y)
month=$(date +%m)
if [ $# -gt 1 ] ; then
	month=$1   # Use leading zero, e.g. "03" for March
fi
if [ $# -ge 2 ] ; then
    year=$2  # Should be a four-digit number
fi

# FIXME: Needs to only query networks with data for the current month.

dbfile="var/reqlogstats-${year}.db"
if [ ! -r ${dbfile} ] ; then
	echo " *** Error: SQLite DB file ${dbfile} was not found."
	exit 1
fi
netlist=`echo "SELECT distinct(networkCode) FROM ArcStatsNetwork GROUP BY networkCode;" | sqlite3 ${dbfile}`
for code in ${netlist} ; do
    ./make_month_graph.sh --code ${code} ${month} ${year} ${dbfile}
done
