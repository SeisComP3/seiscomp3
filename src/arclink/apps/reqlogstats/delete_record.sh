#!/bin/sh
#
# Helper script to manually remove a day from the EIDA reqlogstats
# database. For instance, due to a bad report, or when there are
# problems adding a new DCID or a change in source.
#
# First shows what should be done, leaving the actual DELETEs to you.
#
# Begun by Peter L. Evans, May 2014?
# <pevans@gfz-potsdam.de>
#
# Copyright (C) Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
#
# ----------------------------------------------------------------------
set -u

progname=`basename $0`

show_usage() {
  echo "Usage: ${progname} {db} {DCID | id} {date}"
  cat <<EOF
Remove a record from all tables for the report.
 {db}   - the SQLite3 database containing Arclink statistics summaries
 {DCID} - string e.g. "GFZ"
 {id}   - numeric internal id used inside the database; see ArcStatsSource.
 {date} - start day of the record to be remove, as YYYY-MM-DD

EOF
}


examine() {
        echo "Looking into DB for relevant records..."

	echo "Don't remove this row in ArcStatsSource:"
	sqlite3 ${db} <<EOF
SELECT * FROM ArcStatsSource where id='${src}'; /* Don't delete this! */
EOF

	echo "Do remove all of these:"
	sqlite3 ${db} <<EOF | tee list
SELECT * FROM ArcStatsClientIP where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsMessages where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsNetwork where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsReport where start_day = '${day}';
SELECT * FROM ArcStatsRequest where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsStation where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsSummary where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsUser where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsUserIP where start_day = '${day}' and src='${src}';
SELECT * FROM ArcStatsVolume where start_day = '${day}' and src='${src}';

EOF
	lines=$(cat list | wc -l)
	rm list
	echo "Number of records found: ${lines}"

	if [ ${lines} -gt 0 ] ; then
		echo
		echo "Now carefully cut and paste from the following lines:"
		echo "sqlite3 ${db}"
		for table in ArcStatsClientIP ArcStatsMessages ArcStatsNetwork ArcStatsRequest ArcStatsStation ArcStatsSummary ArcStatsUser ArcStatsUserIP ArcStatsVolume ; do
			echo "DELETE FROM ${table} WHERE start_day = '${day}' AND src='${src}';"
		done
		echo "## DELETE FROM ArcStatsReport WHERE start_day = '${day}';"
		echo "SELECT * FROM ArcStatsReport where start_day = '${day}';"
		echo "SELECT * FROM ArcStatsSummary where start_day = '${day}';"
		echo "## (the offending report should be gone from the summary table)"
	fi
}

if [ $# -ne 3 ] ; then
    show_usage
    exit 2
fi

db=$1 # var/reqlogstats-2014.db
src=$2  # 4, RESIF
day=$3  # "2014-03-03"

if [ ! -r ${db} ] ; then
    echo "Error, database file ${db} isn't readable" >&2
    exit 1
fi

if [[ "$src" =~ ^[0-9]*$ ]] ; then
    echo "Removing record for id=$src"
else
    echo "Looking up source DCID..."
    id=$(echo "SELECT id FROM ArcStatsSource WHERE dcid='$src';" | sqlite3 $db)
    src=$id
fi
echo "Found source:"
echo "SELECT * FROM ArcStatsSource WHERE id='$src';" | sqlite3 $db

if [[ "${day}" =~ ^[1-9][0-9][0-9][0-9]-[0-1][0-9]-[0-3][0-9]$ ]] ; then
    echo "Start day: $day"
else
    echo "Error, not a proper day, use YYYY-MM-DD format." >&2
    exit 1
fi

examine

# Then:
# sqlite> DELETE FROM ArcStatsSummary where start_day = '${day}' and src="${src}";
# and so on...

# Make sure the offending e-mail isn't in ~/eida_stats, otherwise
# it might just get reloaded. Then run reqlogstats.sh again.


