set xdata time
set timefmt "%Y-%m-%d"
set xlabel 'Date in #year#'

year = ('#' . 'year' . '#' eq '#year#')?2017:#year#
set xrange [year . '-01-01':]

xtic_density = ('#' . 'xtic_density' . '#' eq '#xtic_density#')?7:#xtic_density#
set xtics #xtic_density# * 24 * 3600

set xtics format "%d\n%b"
set ylabel 'Total bytes for "#patt#"'
set logscale y

set key top left
set grid x
set style data impulses

set terminal svg font "arial,14" size 960,480   # even "giant" is not enough font.
set output 'out.svg'

plot 'days3.dat' using 1:2 title 'All EIDA nodes', \
  '' using 1:5 title 'GFZ', \
  '/srv/www/webdc/eida/data/total-2017.txt' using 1:(1024*1024*$2) title 'All requests' w l

#set terminal dumb
#set output
#replot
