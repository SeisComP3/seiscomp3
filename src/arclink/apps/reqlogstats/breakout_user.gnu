#!/bin/sh

year=$(date +"%Y")
today=$(date +"%Y-%m-%d")

echo "Year = $year ; today = $today"

awk '{print $1,$2}' /srv/www/webdc/eida/data/total-${year}.txt > a
awk '{print $1,$2}' /srv/www/webdc/eida/data/total-user-${year}_fdsnws.txt > f

cat <<END | gnuplot
set timefmt "%Y-%m-%d"

set xdata time
year='$year'
today='$today'
#today='2017-01-31'
set xlabel 'Date in '.year
set xtics format '%d %b'
set xrange [year.'-01-01':today]
set ylabel 'MiB'
### set logscale y

set title 'Total megabytes requested and requests by fdsnws-dataselect to '.today
set key box top left horizontal height 1.5

set terminal svg font "arial,14" size 960,480 dynamic
set output 'out.svg'

n=2
plot '/srv/www/webdc/eida/data/total-${year}.txt' u 1:n with linespoints title 'All EIDA nodes total', \
     '/srv/www/webdc/eida/data/total-user-${year}_fdsnws.txt' u 1:2 with impulses title 'All EIDA fdsnws only', \
      '' u 1:2 with lines notitle ls 2
## Marker for FDSNWS too:     '' u 1:2 with points notitle ls 2

set output 'fdsnws_ratio.svg'
unset ylabel
unset logscale y
set yrange [0:1.1]
set style data linespoints
set title 'Fraction of megabytes requested by fdsnws-dataselect to '.today
plot '<paste a f' using 1:(\$4/\$2) title 'Fraction using fdsnws-dataselect', \
	'<paste a f' using 1:(((\$4/\$2)>0.5)?0.5:-NaN) with lines notitle, 1
END
ls -l out.svg fdsnws_ratio.svg
rm a f
