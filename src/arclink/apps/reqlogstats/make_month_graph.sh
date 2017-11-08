#!/bin/sh
#
# Make a graph of bytes per day or month.
#
# Begun by Peter L. Evans, December 2013/January 2014
#
# Input: reqlogstats-*.db SQLite database
# Parameters: network code [optional]
# Output: two plots - total, and break-out by source.
#
# Copyright (C) 2013-7 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ
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
code2=$(echo $code | sed -e "s:\/:_:")  # used in file names, so no slashes.

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
	table="ArcStatsSummary as X JOIN ArcStatsSource as Y WHERE X.src = Y.id"
	cmd="SELECT start_day, dcid, total_size FROM ${table} ${code_constr}"
else
	table="ArcStatsNetwork as X JOIN ArcStatsSource as Y WHERE X.src = Y.id"
	cmd="SELECT start_day, dcid, size FROM ${table} ${code_constr}"
fi
cmd="${cmd} AND X.start_day > '$start_year-$start_month-00' AND X.start_day < '$start_year-$start_month-99' ORDER BY start_day, dcid;"

#echo ${cmd}
echo ${cmd} \
    | sqlite3 ${dbfile} | sed -e 's/|/  /g' \
    | python ${dirname}/t1.py \
    | python ${dirname}/t2.py > days3.dat

if [ $(wc -l days3.dat | awk '{print $1}') -le 1 ] ; then
    echo "Nothing in db with '${code_constr}'."
    rm days3.dat
    exit 0
fi

head -1 days3.dat
tail -5 days3.dat

start_month_name=$(date +%B -d "$start_year-$start_month-01")

sed -e "s/\#month_name\#/${start_month_name}/" \
    -e "s/\#month\#/${start_month}/" \
    -e "s/\#year\#/${start_year}/" \
    total-month.gnu | gnuplot

if [ -z "${code}" ] ; then
    out_dir="${img_dir}"
    outfile="${out_dir}/total-${start_year}-${start_month}.svg"
else
    out_dir="${img_dir}/${start_year}/${start_month}"
    outfile="${out_dir}/graph-${code2}-total.svg"
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

sed -e "s/\#month\#/${start_month_name}/g" \
    -e "s/\#year\#/${start_year}/g" \
    sources.gnu | gnuplot

if [ -z "${code}" ] ; then
    out_dir="${img_dir}"
    outfile="${out_dir}/sources-${start_year}-${start_month}.svg"
    txtfile="${out_dir}/total-${start_year}-${start_month}.txt"
else
    out_dir="${img_dir}/${start_year}/${start_month}"
    outfile="${out_dir}/graph-${code2}-sources.svg"
    txtfile="${out_dir}/graph-${code2}-total.txt"
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
