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
    echo "${progname}: Images directory ${img_dir} does not exist. Bye."
    exit 1
fi

if [ ! -d ${db_dir} ] ; then
    echo "${progname}: SQLite DB directory ${db_dir} does not exist. Bye."
    exit 1
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
if [ ! -s ${dbfile} ] ; then
    echo "Error: ${dbfile} not found. Bye"
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

gnuplot <<EOF
set xdata time
set timefmt "%Y-%m-%d"
set xlabel 'Day in $start_month_name $start_year'
set xrange ['$start_year-$start_month-01':]
set xtics 24*3600
set xtics format "%d"
set ylabel 'total_size, MiB'
set logscale y
set yrange [0.09:]  ### Not quite right, but tables are rounded to 0.1MB, so this ensures there will always be a finite y-range.

set key top left
set grid
set style data linespoints

set terminal svg font "arial,14" linewidth 2 size 960,480   # even "giant" is not enough font.
set output 'out.svg'

# Round ball markers, black line by default.
plot 'days3.dat' using 1:2 linestyle 7 title 'All EIDA nodes'

#set terminal dumb
#set output
#replot
EOF

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

gnuplot <<EOF
set xlabel 'Day in $start_month_name $start_year'

set ylabel 'total_size, MiB'
set yrange [0:]

set key top left
set grid

set style data histograms
set style histogram rowstacked
set boxwidth 0.5 relative
set style fill solid 1.0 border -1

set terminal svg font "arial,14" size 960,480
set output 'out.svg'

# Default for ls 6 is dark blue, too close to pure blue for GFZ:
set style line 3 linecolor rgb "#00589C"
set style line 5 linecolor rgb "skyblue"
set style line 6 linecolor rgb "violet"
set style line 10 linecolor rgb "magenta"

plot '<cut -c9- days3.dat' using 3:xtic(1) title 'BGR' ls 2, \
     '' using  4 title 'ETHZ' ls 1, \
     '' using  5 title 'GFZ' ls 3, \
     '' using  6 title 'INGV' ls 4, \
     '' using  7 title 'IPGP' ls 6, \
     '' using  8 title 'KOERI' ls 1, \
     '' using  9 title 'LMU' ls 7, \
     '' using 10 title 'NIEP' ls 10, \
     '' using 11 title 'NOA' ls 5, \
     '' using 12 title 'ODC' ls 9, \
     '' using 13 title 'RESIF' ls 8

#set terminal dumb
#set output
#replot
EOF

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
