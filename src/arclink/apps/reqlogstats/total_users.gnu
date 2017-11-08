set xdata time
set timefmt "%Y-%m-%d"
set xlabel 'Date in #year#'
set xrange ['#year#-01-01':]
set xtics #xtic_density# * 24 * 3600
set xtics format "%d\n%b"
set ylabel 'Distinct users'
#set logscale y
set yrange [0:260]  # For GFZ in 2014: [0:130] is good.

set key top left
set grid x
set style data linespoints

set terminal svg font "arial,14" size 960,480   # even "giant" is not enough font.
set output 'out.svg'

plot 'days3.dat' using 1:2 title 'All EIDA nodes', \
  '' using 1:5 title 'GFZ'

#set terminal dumb
#set output
#replot
