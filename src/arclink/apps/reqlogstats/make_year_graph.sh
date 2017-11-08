#!/bin/sh
#
# Make graph of bytes per day or month over a whole year.
#
# Begun by Peter L. Evans, May 2014.
# Quick hack which may need rethinking.
#
# Input: reqlogstats-*.db SQLite database
# Parameters: network code [optional]
# Output: PNG plot - total
#         PNG plot - break-out by source.
#         text total  
#
# Copyright (C) 2017 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
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
  echo "Usage: ${progname} [--code {net} ] [ {month} [ {year} [ {db file} ]]]"
  echo
  echo "Create usage images from {db file} for the given date."
  echo "If {net} is not given, all networks are included."
  echo "If {month} or {year} are not given, use today's date."
  echo "If {db file} is not given, use a default."
}

code=
code_constr=""
table=

if [ $# -gt 0 ] ; then
    first=$1
    if [ "$first" = "--code" ] ; then
	code=$2
	code_constr="AND X.networkCode = '${code}'"
	shift 2;
    fi
fi

echo "Restricted to code=${code}; setting constraint: '${code_constr}'"
#code2=$(echo $code | sed -e "s:\/:_:")  # used in file names, so no slashes.

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


if [ -z "${code}" ] ; then
    cmd="SELECT start_day, dcid, total_size FROM ArcStatsSummary as X JOIN ArcStatsSource as Y WHERE X.src = Y.id ${code_constr} AND substr(X.start_day, 1, 4) = '${start_year}' ORDER BY start_day, dcid;"
else
    cmd="SELECT start_day, dcid, size/1024.0/1024.0 FROM ArcStatsNetwork as X JOIN ArcStatsSource as Y WHERE X.src = Y.id ${code_constr} ORDER BY start_day, dcid;"
fi
echo ${cmd}

# Select; normalise units (GiB -> MiB); make the table
echo ${cmd} \
    | sqlite3 ${dbfile} | sed -e 's/|/  /g' \
    | python ${dirname}/t1.py \
    | python ${dirname}/t2.py > days3.dat

if [ $(wc -l days3.dat | awk '{print $1}') -le 1 ] ; then
    echo "Nothing in db with '${code_constr}'."
    exit 0
fi

head -1 days3.dat
tail -5 days3.dat

start_month_name=$(date +%B -d "$start_year-$start_month-01")

xtic_density=14
sed -e "s/\#year\#/${start_year}/g" \
    -e "s/\#xtic_density\#/${xtic_density}/g" \
    total.gnu | gnuplot

if [ -z "${code}" ] ; then
    out_dir="${img_dir}"
    outfile="${out_dir}/total-${start_year}.svg"
else
    echo "Sorry, can't do it yet. Bye."
    exit 22
    #out_dir="${img_dir}/${start_year}/${start_month}"
    #outfile="${out_dir}/graph-${code2}-total.svg"
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

sed -e "s/\#year\#/$start_year/" \
    sources_year.gnu | gnuplot

if [ -z "${code}" ] ; then
    out_dir="${img_dir}"
    outfile="${out_dir}/sources-${start_year}.svg"
    txtfile="${out_dir}/total-${start_year}.txt"
else
    echo "Sorry, can't do it yet. Bye."
    exit 22
 
    #out_dir="${img_dir}/${start_year}/${start_month}"
    #outfile="${out_dir}/graph-${code2}-sources.svg"
    #txtfile="${out_dir}/graph-${code2}-total.txt"
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
