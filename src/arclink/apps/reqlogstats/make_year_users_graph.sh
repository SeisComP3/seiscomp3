#!/bin/sh
#
# Make graph of number of users per day or month over a whole year.
#
# Begun by Peter L. Evans, Oct 2014.
# Quick hack of make_year_graph.sh
#
# Input: reqlogstats-*.db SQLite database
# Parameters: network code [optional]
# Output: PNG plot - total
#         PNG plot - break-out by source.
#         text total  
#
# Copyright (C) 2014-7 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
#
# This software is free software and comes with ABSOLUTELY NO WARRANTY.
#
# ----------------------------------------------------------------------
set -u

progname=`basename $0`
dirname=`dirname $0`
today=`date +%F`

start_year=`date +%Y`
start_month=`date +%m`
img_dir='/srv/www/webdc/eida/data'
db_dir="${HOME}/reqlogstats/var"

if [ ! -d ${img_dir} ] ; then
    echo "${progname}: Images directory ${img_dir} does not exist. Using local var."
    img_dir=var
fi

if [ ! -d ${db_dir} ] ; then
    echo "${progname}: SQLite DB directory ${db_dir} does not exist. Using local var."
    db_dir=var
fi

show_usage() {
  echo "Usage: ${progname} [--dcid {dcid} ] [ {month} [ {year} [ {db file} ]]]"
  echo
  echo "Create usage images from {db file} for the given date."
  echo "If {dcid} is not given, all DCIDs are included."
  echo "If {month} or {year} are not given, use today's date."
  echo "If {db file} is not given, use a default."
}

dcid=
dcid_constr=""
table=

if [ $# -gt 0 ] ; then
    first=$1
    if [ "$first" = "--dcid" ] ; then
	dcid=$2
	dcid_constr="AND Y.dcid = '${dcid}'"
	shift 2;
    fi
fi

echo "Restricted to dcid=${dcid}; setting constraint: '${dcid_constr}'"

if [ $# -ge 1 ] ; then
    start_month=$1  # Should be a two-digit number
fi
if [ $# -ge 2 ] ; then
    start_year=$2  # Should be a four-digit number
fi
if [ $# -ge 3 ] ; then
    dbfile=$3
else
    dbfile="${db_dir}/reqlogstats-${start_year}.db"
fi
echo "Looking in ${dbfile} for ${start_year} month ${start_month}" 
if [ ! -s "${dbfile}" ] ; then
    echo "Error: ${dbfile} not found or is empty. Bye"
    exit 1
fi


if [ -z "${dcid}" ] ; then
    cmd="SELECT start_day, dcid, count(userID) FROM ArcStatsUser as X JOIN ArcStatsSource as Y WHERE X.src = Y.id ${dcid_constr} AND substr(X.start_day, 1, 4) = '${start_year}' GROUP BY start_day, dcid ORDER BY start_day, dcid;"
else
    cmd="SELECT start_day, dcid, count(userID) FROM ArcStatsUser as X JOIN ArcStatsSource as Y WHERE X.src = Y.id ${dcid_constr} GROUP BY start_day, dcid ORDER BY start_day, dcid;"
fi
echo ${cmd}

echo ${cmd} \
    | sqlite3 ${dbfile} | tee raw.sql | sed -e 's/|/  /g' \
    | python ${dirname}/t2.py > days3.dat

if [ $(wc -l days3.dat | awk '{print $1}') -le 1 ] ; then
    echo "Nothing in db with '${dcid_constr}'."
    rm days3.dat
    exit 0
fi

head -1 days3.dat
tail -5 days3.dat

start_month_name=$(date +%B -d "$start_year-$start_month-01")

xtic_density=14
sed -e "s/\#year\#/${start_year}/g" \
    -e "s/\#xtic_density\#/${xtic_density}/g" \
    total_users.gnu | gnuplot

if [ -z "${dcid}" ] ; then
    out_dir="${img_dir}"
    outfile="${out_dir}/total-users-${start_year}.svg"
else
    echo "Sorry, can't do it yet. Bye."
    exit 22
fi

if [ -s out.svg ] ; then
    mkdir -p ${out_dir}
    mv out.svg $outfile
    echo "Wrote $outfile"
else
    echo "No output!"
    rm -f out.svg
    exit 0
fi

# ----------------------------------------------------------------------

sed -e "s/\#year\#/${start_year}/g" \
    -e "s/\#xtic_density\#/${xtic_density}/g" \
    sources_users.gnu | gnuplot

if [ -z "${dcid}" ] ; then
    out_dir="${img_dir}"
    outfile="${out_dir}/sources-users-${start_year}.svg"
    txtfile="${out_dir}/total-users-${start_year}.txt"
else
    echo "Sorry, can't do it yet. Bye."
    exit 22
 
    #out_dir="${img_dir}/${start_year}/${start_month}"
fi

if [ -s out.svg ] ; then
    mkdir -p "${out_dir}"
    mv out.svg $outfile
    echo "Wrote $outfile"
else
    rm -f out.svg
    echo "No SVG output!"
fi

if [ -s days3.dat ] ; then
    mv days3.dat $txtfile
else
    echo "No text file output!"
fi

rm -f days3.dat
