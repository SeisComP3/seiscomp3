set timefmt "%Y-%m-%d"

set xdata time
set xlabel 'Date in 2016'
set xtics format '%b'
##set xrange ['2016-01-01':'2016-12-31']
set xrange ['2016-01-01':'2016-03-01']
set ylabel 'MiB'
set logscale y

set title 'Total megabytes requested and requests by fdsnws-dataselect '
set key box outside right bottom horizontal height 1.5

set terminal svg font "arial,14" size 960,480
set output 'out.svg'

n=2
plot '/srv/www/webdc/eida/data/total-2016.txt' u 1:n with linespoints title 'All EIDA nodes total', \
     '/srv/www/webdc/eida/data/total-user-2016_fdsnws.txt' u 1:2 with impulses title 'All EIDA fdsnws only', \
     '' u 1:2 with points notitle ls 2

